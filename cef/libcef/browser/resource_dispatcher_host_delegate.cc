// Copyright 2015 The Chromium Embedded Framework Authors.
// Portions copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/resource_dispatcher_host_delegate.h"

#include <stdint.h>

#include <utility>

#include "libcef/browser/browser_host_impl.h"
#include "libcef/browser/origin_whitelist_impl.h"
#include "libcef/browser/resource_context.h"
#include "libcef/browser/thread_util.h"

#include "base/guid.h"
#include "base/memory/scoped_vector.h"
#include "build/build_config.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/browser/stream_info.h"
#include "content/public/common/resource_response.h"
#include "content/public/common/webplugininfo.h"
#include "net/http/http_response_headers.h"
#include "net/url_request/url_request.h"

// namespace {

// void SendExecuteMimeTypeHandlerEvent(
//     std::unique_ptr<content::StreamInfo> stream,
//     int64_t expected_content_size,
//     const std::string& extension_id,
//     const std::string& view_id,
//     bool embedded,
//     int frame_tree_node_id,
//     int render_process_id,
//     int render_frame_id) {
//   CEF_REQUIRE_UIT();

//   content::WebContents* web_contents = nullptr;
//   if (frame_tree_node_id != -1) {
//     web_contents =
//         content::WebContents::FromFrameTreeNodeId(frame_tree_node_id);
//   } else {
//     web_contents = content::WebContents::FromRenderFrameHost(
//         content::RenderFrameHost::FromID(render_process_id, render_frame_id));
//   }

//   CefRefPtr<CefBrowserHostImpl> browser =
//       CefBrowserHostImpl::GetBrowserForContents(web_contents);
//   if (!browser.get())
//     return;
// }

// }  // namespace

CefResourceDispatcherHostDelegate::CefResourceDispatcherHostDelegate() {
}

CefResourceDispatcherHostDelegate::~CefResourceDispatcherHostDelegate() {
}

bool CefResourceDispatcherHostDelegate::HandleExternalProtocol(
    const GURL& url,
    int child_id,
    const content::ResourceRequestInfo::WebContentsGetter& web_contents_getter,
    bool is_main_frame,
    ui::PageTransition page_transition,
    bool has_user_gesture,
    content::ResourceContext* resource_context) {
  CEF_POST_TASK(CEF_UIT,
      base::Bind(base::IgnoreResult(&CefResourceDispatcherHostDelegate::
                      HandleExternalProtocolOnUIThread),
                  base::Unretained(this), url,
                  web_contents_getter));
  return false;
}

// Implementation based on
// ChromeResourceDispatcherHostDelegate::ShouldInterceptResourceAsStream.
bool CefResourceDispatcherHostDelegate::ShouldInterceptResourceAsStream(
    net::URLRequest* request,
    const base::FilePath& plugin_path,
    const std::string& mime_type,
    GURL* origin,
    std::string* payload) {
  return false;
}

// Implementation based on
// ChromeResourceDispatcherHostDelegate::OnStreamCreated.
void CefResourceDispatcherHostDelegate::OnStreamCreated(
    net::URLRequest* request,
    std::unique_ptr<content::StreamInfo> stream) {
}

void CefResourceDispatcherHostDelegate::OnRequestRedirected(
    const GURL& redirect_url,
    net::URLRequest* request,
    content::ResourceContext* resource_context,
    content::ResourceResponse* response) {
  const GURL& active_url = request->url();
  if (active_url.is_valid() && redirect_url.is_valid() &&
      active_url.GetOrigin() != redirect_url.GetOrigin() &&
      HasCrossOriginWhitelistEntry(active_url, redirect_url)) {
    if (!response->head.headers.get())
      response->head.headers = new net::HttpResponseHeaders(std::string());

    // Add CORS headers to support XMLHttpRequest redirects.
    response->head.headers->AddHeader("Access-Control-Allow-Origin: " +
        active_url.scheme() + "://" + active_url.host());
    response->head.headers->AddHeader("Access-Control-Allow-Credentials: true");
  }
}

std::unique_ptr<net::ClientCertStore>
    CefResourceDispatcherHostDelegate::CreateClientCertStore(
        content::ResourceContext* resource_context) {
  return static_cast<CefResourceContext*>(resource_context)->
      CreateClientCertStore();
}

void CefResourceDispatcherHostDelegate::HandleExternalProtocolOnUIThread(
    const GURL& url,
    const content::ResourceRequestInfo::WebContentsGetter&
        web_contents_getter) {
  CEF_REQUIRE_UIT();
  content::WebContents* web_contents = web_contents_getter.Run();
  if (web_contents) {
    CefRefPtr<CefBrowserHostImpl> browser =
        CefBrowserHostImpl::GetBrowserForContents(web_contents);
    if (browser.get())
      browser->HandleExternalProtocol(url);
  }
}
