// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/frame_host/render_frame_message_filter.h"

#include "base/command_line.h"
#include "base/macros.h"
#include "base/metrics/field_trial.h"
#include "base/strings/string_util.h"
#include "build/build_config.h"
#include "content/browser/bad_message.h"
#include "content/browser/blob_storage/chrome_blob_storage_context.h"
#include "content/browser/child_process_security_policy_impl.h"
#include "content/browser/download/download_stats.h"
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/browser/gpu/gpu_data_manager_impl.h"
#include "content/browser/renderer_host/render_widget_helper.h"
#include "content/browser/resource_context_impl.h"
#include "content/common/content_constants_internal.h"
#include "content/common/frame_messages.h"
#include "content/common/frame_owner_properties.h"
#include "content/common/view_messages.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/download_url_parameters.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/content_switches.h"
#include "gpu/GLES2/gl2extchromium.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "net/cookies/cookie_options.h"
#include "net/cookies/cookie_store.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"
#include "storage/browser/blob/blob_storage_context.h"
#include "url/gurl.h"

#if !defined(OS_MACOSX)
#include "third_party/khronos/GLES2/gl2.h"
#include "third_party/khronos/GLES2/gl2ext.h"
#endif

namespace content {

namespace {

const char kEnforceStrictSecureExperiment[] = "StrictSecureCookies";

void CreateChildFrameOnUI(int process_id,
                          int parent_routing_id,
                          blink::WebTreeScopeType scope,
                          const std::string& frame_name,
                          const std::string& frame_unique_name,
                          blink::WebSandboxFlags sandbox_flags,
                          const FrameOwnerProperties& frame_owner_properties,
                          int new_routing_id) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  RenderFrameHostImpl* render_frame_host =
      RenderFrameHostImpl::FromID(process_id, parent_routing_id);
  // Handles the RenderFrameHost being deleted on the UI thread while
  // processing a subframe creation message.
  if (render_frame_host) {
    render_frame_host->OnCreateChildFrame(new_routing_id, scope, frame_name,
                                          frame_unique_name, sandbox_flags,
                                          frame_owner_properties);
  }
}

void DownloadUrlOnUIThread(std::unique_ptr<DownloadUrlParameters> parameters) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  RenderProcessHost* render_process_host =
      RenderProcessHost::FromID(parameters->render_process_host_id());
  if (!render_process_host)
    return;

  BrowserContext* browser_context = render_process_host->GetBrowserContext();
  DownloadManager* download_manager =
      BrowserContext::GetDownloadManager(browser_context);
  RecordDownloadSource(INITIATED_BY_RENDERER);
  download_manager->DownloadUrl(std::move(parameters));
}

// Common functionality for converting a sync renderer message to a callback
// function in the browser. Derive from this, create it on the heap when
// issuing your callback. When done, write your reply parameters into
// reply_msg(), and then call SendReplyAndDeleteThis().
class RenderMessageCompletionCallback {
 public:
  RenderMessageCompletionCallback(RenderFrameMessageFilter* filter,
                                  IPC::Message* reply_msg)
      : filter_(filter),
        reply_msg_(reply_msg) {
  }

  virtual ~RenderMessageCompletionCallback() {
    if (reply_msg_) {
      // If the owner of this class failed to call SendReplyAndDeleteThis(),
      // send an error reply to prevent the renderer from being hung.
      reply_msg_->set_reply_error();
      filter_->Send(reply_msg_);
    }
  }

  RenderFrameMessageFilter* filter() { return filter_.get(); }
  IPC::Message* reply_msg() { return reply_msg_; }

  void SendReplyAndDeleteThis() {
    filter_->Send(reply_msg_);
    reply_msg_ = nullptr;
    delete this;
  }

 private:
  scoped_refptr<RenderFrameMessageFilter> filter_;
  IPC::Message* reply_msg_;
};

}  // namespace

RenderFrameMessageFilter::RenderFrameMessageFilter(
    int render_process_id,
    PluginServiceImpl* plugin_service,
    BrowserContext* browser_context,
    net::URLRequestContextGetter* request_context,
    RenderWidgetHelper* render_widget_helper)
    : BrowserMessageFilter(FrameMsgStart),
      BrowserAssociatedInterface<mojom::RenderFrameMessageFilter>(this, this),
      request_context_(request_context),
      resource_context_(browser_context->GetResourceContext()),
      render_widget_helper_(render_widget_helper),
      incognito_(browser_context->IsOffTheRecord()),
      render_process_id_(render_process_id) {
}

RenderFrameMessageFilter::~RenderFrameMessageFilter() {
  // This function should be called on the IO thread.
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
}

bool RenderFrameMessageFilter::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(RenderFrameMessageFilter, message)
    IPC_MESSAGE_HANDLER(FrameHostMsg_CreateChildFrame, OnCreateChildFrame)
    IPC_MESSAGE_HANDLER(FrameHostMsg_CookiesEnabled, OnCookiesEnabled)
    IPC_MESSAGE_HANDLER(FrameHostMsg_DownloadUrl, OnDownloadUrl)
    IPC_MESSAGE_HANDLER(FrameHostMsg_SaveImageFromDataURL,
                        OnSaveImageFromDataURL)
    IPC_MESSAGE_HANDLER(FrameHostMsg_Are3DAPIsBlocked, OnAre3DAPIsBlocked)
    IPC_MESSAGE_HANDLER_GENERIC(FrameHostMsg_RenderProcessGone,
                                OnRenderProcessGone())
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void RenderFrameMessageFilter::OnDestruct() const {
  BrowserThread::DeleteOnIOThread::Destruct(this);
}

void RenderFrameMessageFilter::DownloadUrl(int render_view_id,
                                           int render_frame_id,
                                           const GURL& url,
                                           const Referrer& referrer,
                                           const base::string16& suggested_name,
                                           const bool use_prompt) const {
  if (!resource_context_)
    return;

  std::unique_ptr<DownloadUrlParameters> parameters(
      new DownloadUrlParameters(url, render_process_id_, render_view_id,
                                render_frame_id, request_context_.get()));
  parameters->set_content_initiated(true);
  parameters->set_suggested_name(suggested_name);
  parameters->set_prompt(use_prompt);
  parameters->set_referrer(referrer);

  if (url.SchemeIsBlob()) {
    ChromeBlobStorageContext* blob_context =
        GetChromeBlobStorageContextForResourceContext(resource_context_);
    parameters->set_blob_data_handle(
        blob_context->context()->GetBlobDataFromPublicURL(url));
    // Don't care if the above fails. We are going to let the download go
    // through and allow it to be interrupted so that the embedder can deal.
  }

  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&DownloadUrlOnUIThread, base::Passed(&parameters)));
}

void RenderFrameMessageFilter::OnCreateChildFrame(
    const FrameHostMsg_CreateChildFrame_Params& params,
    int* new_routing_id) {
  *new_routing_id = render_widget_helper_->GetNextRoutingID();
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&CreateChildFrameOnUI, render_process_id_,
                 params.parent_routing_id, params.scope, params.frame_name,
                 params.frame_unique_name, params.sandbox_flags,
                 params.frame_owner_properties, *new_routing_id));
}

void RenderFrameMessageFilter::OnCookiesEnabled(
    int render_frame_id,
    const GURL& url,
    const GURL& first_party_for_cookies,
    bool* cookies_enabled) {
  // TODO(ananta): If this render frame is associated with an automation
  // channel, aka ChromeFrame then we need to retrieve cookie settings from the
  // external host.
  *cookies_enabled = GetContentClient()->browser()->AllowGetCookie(
      url, first_party_for_cookies, net::CookieList(), resource_context_,
      render_process_id_, render_frame_id);
}

void RenderFrameMessageFilter::CheckPolicyForCookies(
    int render_frame_id,
    const GURL& url,
    const GURL& first_party_for_cookies,
    const GetCookiesCallback& callback,
    const net::CookieList& cookie_list) {
  net::URLRequestContext* context = GetRequestContextForURL(url);
  // Check the policy for get cookies, and pass cookie_list to the
  // TabSpecificContentSetting for logging purpose.
  if (context &&
      GetContentClient()->browser()->AllowGetCookie(
          url, first_party_for_cookies, cookie_list, resource_context_,
          render_process_id_, render_frame_id)) {
    callback.Run(net::CookieStore::BuildCookieLine(cookie_list));
  } else {
    callback.Run(std::string());
  }
}

void RenderFrameMessageFilter::OnDownloadUrl(
    int render_view_id,
    int render_frame_id,
    const GURL& url,
    const Referrer& referrer,
    const base::string16& suggested_name) {
  DownloadUrl(render_view_id, render_frame_id, url, referrer, suggested_name,
              false);
}

void RenderFrameMessageFilter::OnSaveImageFromDataURL(
    int render_view_id,
    int render_frame_id,
    const std::string& url_str) {
  // Please refer to RenderFrameImpl::saveImageFromDataURL().
  if (url_str.length() >= kMaxLengthOfDataURLString)
    return;

  GURL data_url(url_str);
  if (!data_url.is_valid() || !data_url.SchemeIs(url::kDataScheme))
    return;

  DownloadUrl(render_view_id, render_frame_id, data_url, Referrer(),
              base::string16(), true);
}

void RenderFrameMessageFilter::OnAre3DAPIsBlocked(int render_frame_id,
                                                  const GURL& top_origin_url,
                                                  ThreeDAPIType requester,
                                                  bool* blocked) {
  *blocked = GpuDataManagerImpl::GetInstance()->Are3DAPIsBlocked(
      top_origin_url, render_process_id_, render_frame_id, requester);
}

void RenderFrameMessageFilter::OnRenderProcessGone() {
  // FrameHostMessage_RenderProcessGone is a synthetic IPC message used by
  // RenderProcessHostImpl to clean things up after a crash (it's injected
  // downstream of this filter). Allowing it to proceed would enable a renderer
  // to fake its own death; instead, actually kill the renderer.
  bad_message::ReceivedBadMessage(
      this, bad_message::RFMF_RENDERER_FAKED_ITS_OWN_DEATH);
}

void RenderFrameMessageFilter::SetCookie(int32_t render_frame_id,
                                         const GURL& url,
                                         const GURL& first_party_for_cookies,
                                         const std::string& cookie) {
  ChildProcessSecurityPolicyImpl* policy =
      ChildProcessSecurityPolicyImpl::GetInstance();
  if (!policy->CanAccessDataForOrigin(render_process_id_, url)) {
    bad_message::ReceivedBadMessage(this,
                                    bad_message::RFMF_SET_COOKIE_BAD_ORIGIN);
    return;
  }

  net::CookieOptions options;
  bool experimental_web_platform_features_enabled =
      base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kEnableExperimentalWebPlatformFeatures);
  const std::string enforce_strict_secure_group =
      base::FieldTrialList::FindFullName(kEnforceStrictSecureExperiment);
  if (experimental_web_platform_features_enabled ||
      base::StartsWith(enforce_strict_secure_group, "Enabled",
                       base::CompareCase::INSENSITIVE_ASCII)) {
    options.set_enforce_strict_secure();
  }
  if (GetContentClient()->browser()->AllowSetCookie(
          url, first_party_for_cookies, cookie, resource_context_,
          render_process_id_, render_frame_id, options)) {
    net::URLRequestContext* context = GetRequestContextForURL(url);
    // Pass a null callback since we don't care about when the 'set' completes.
    context->cookie_store()->SetCookieWithOptionsAsync(
        url, cookie, options, net::CookieStore::SetCookiesCallback());
  }
}

void RenderFrameMessageFilter::GetCookies(int render_frame_id,
                                          const GURL& url,
                                          const GURL& first_party_for_cookies,
                                          const GetCookiesCallback& callback) {
  ChildProcessSecurityPolicyImpl* policy =
      ChildProcessSecurityPolicyImpl::GetInstance();
  if (!policy->CanAccessDataForOrigin(render_process_id_, url)) {
    bad_message::ReceivedBadMessage(this,
                                    bad_message::RFMF_GET_COOKIES_BAD_ORIGIN);
    callback.Run(std::string());
    return;
  }

  net::CookieOptions options;
  if (net::registry_controlled_domains::SameDomainOrHost(
          url, first_party_for_cookies,
          net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES)) {
    // TODO(mkwst): This check ought to further distinguish between frames
    // initiated in a strict or lax same-site context.
    options.set_same_site_cookie_mode(
        net::CookieOptions::SameSiteCookieMode::INCLUDE_STRICT_AND_LAX);
  } else {
    options.set_same_site_cookie_mode(
        net::CookieOptions::SameSiteCookieMode::DO_NOT_INCLUDE);
  }

  // If we crash here, figure out what URL the renderer was requesting.
  // http://crbug.com/99242
  char url_buf[128];
  base::strlcpy(url_buf, url.spec().c_str(), arraysize(url_buf));
  base::debug::Alias(url_buf);

  net::URLRequestContext* context = GetRequestContextForURL(url);
  context->cookie_store()->GetCookieListWithOptionsAsync(
      url, options,
      base::Bind(&RenderFrameMessageFilter::CheckPolicyForCookies, this,
                 render_frame_id, url, first_party_for_cookies, callback));
}

net::URLRequestContext* RenderFrameMessageFilter::GetRequestContextForURL(
    const GURL& url) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);

  net::URLRequestContext* context =
      GetContentClient()->browser()->OverrideRequestContextForURL(
          url, resource_context_);
  if (!context)
    context = request_context_->GetURLRequestContext();

  return context;
}

}  // namespace content
