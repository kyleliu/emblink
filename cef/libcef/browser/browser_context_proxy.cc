// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/browser_context_proxy.h"

#include "libcef/browser/content_browser_client.h"
#include "libcef/browser/download_manager_delegate.h"
#include "libcef/browser/net/url_request_context_getter_proxy.h"
#include "libcef/browser/storage_partition_proxy.h"
#include "libcef/browser/thread_util.h"

#include "base/logging.h"
#include "components/visitedlink/browser/visitedlink_master.h"
#include "content/browser/blob_storage/chrome_blob_storage_context.h"
#include "content/browser/resource_context_impl.h"
#include "content/browser/streams/stream_context.h"
#include "content/browser/webui/url_data_manager.h"
#include "content/public/browser/storage_partition.h"

CefBrowserContextProxy::CefBrowserContextProxy(
    CefRefPtr<CefRequestContextHandler> handler,
    scoped_refptr<CefBrowserContextImpl> parent)
    : CefBrowserContext(true),
      handler_(handler),
      parent_(parent) {
  DCHECK(handler_.get());
  DCHECK(parent_.get());
  parent_->AddProxy(this);
}

CefBrowserContextProxy::~CefBrowserContextProxy() {
  Shutdown();

  parent_->RemoveProxy(this);
}

void CefBrowserContextProxy::Initialize() {
  CefBrowserContext::Initialize();

  // This object's CefResourceContext needs to proxy some UserData requests to
  // the parent object's CefResourceContext.
  resource_context()->set_parent(parent_->resource_context());
}

base::FilePath CefBrowserContextProxy::GetPath() const {
  return parent_->GetPath();
}

std::unique_ptr<content::ZoomLevelDelegate>
    CefBrowserContextProxy::CreateZoomLevelDelegate(
        const base::FilePath& partition_path) {
  return parent_->CreateZoomLevelDelegate(partition_path);
}

bool CefBrowserContextProxy::IsOffTheRecord() const {
  return parent_->IsOffTheRecord();
}

content::DownloadManagerDelegate*
    CefBrowserContextProxy::GetDownloadManagerDelegate() {
  DCHECK(!download_manager_delegate_.get());

  content::DownloadManager* manager = BrowserContext::GetDownloadManager(this);
  download_manager_delegate_.reset(new CefDownloadManagerDelegate(manager));
  return download_manager_delegate_.get();
}

content::BrowserPluginGuestManager* CefBrowserContextProxy::GetGuestManager() {
  return parent_->GetGuestManager();
}

storage::SpecialStoragePolicy*
    CefBrowserContextProxy::GetSpecialStoragePolicy() {
  return parent_->GetSpecialStoragePolicy();
}

content::PushMessagingService*
    CefBrowserContextProxy::GetPushMessagingService() {
  return parent_->GetPushMessagingService();
}

content::SSLHostStateDelegate*
    CefBrowserContextProxy::GetSSLHostStateDelegate() {
  return parent_->GetSSLHostStateDelegate();
}

content::PermissionManager* CefBrowserContextProxy::GetPermissionManager() {
  return parent_->GetPermissionManager();
}

content::BackgroundSyncController*
    CefBrowserContextProxy::GetBackgroundSyncController() {
  return parent_->GetBackgroundSyncController();
}

net::URLRequestContextGetter* CefBrowserContextProxy::CreateRequestContext(
    content::ProtocolHandlerMap* protocol_handlers,
    content::URLRequestInterceptorScopedVector request_interceptors) {
  // CefBrowserContextImpl::GetOrCreateStoragePartitionProxy is called instead
  // of this method.
  NOTREACHED();
  return nullptr;
}

net::URLRequestContextGetter*
    CefBrowserContextProxy::CreateRequestContextForStoragePartition(
        const base::FilePath& partition_path,
        bool in_memory,
        content::ProtocolHandlerMap* protocol_handlers,
        content::URLRequestInterceptorScopedVector request_interceptors) {
  return nullptr;
}

PrefService* CefBrowserContextProxy::GetPrefs() {
  return parent_->GetPrefs();
}

const PrefService* CefBrowserContextProxy::GetPrefs() const {
  return parent_->GetPrefs();
}

const CefRequestContextSettings& CefBrowserContextProxy::GetSettings() const {
  return parent_->GetSettings();
}

CefRefPtr<CefRequestContextHandler> CefBrowserContextProxy::GetHandler() const {
  return handler_;
}

HostContentSettingsMap* CefBrowserContextProxy::GetHostContentSettingsMap() {
  return parent_->GetHostContentSettingsMap();
}

void CefBrowserContextProxy::AddVisitedURLs(const std::vector<GURL>& urls) {
  parent_->AddVisitedURLs(urls);
}

content::StoragePartition*
CefBrowserContextProxy::GetOrCreateStoragePartitionProxy(
    content::StoragePartition* partition_impl) {
  CEF_REQUIRE_UIT();

  if (!storage_partition_proxy_) {
    scoped_refptr<CefURLRequestContextGetterProxy> url_request_getter =
        new CefURLRequestContextGetterProxy(handler_,
                                            parent_->request_context());
    resource_context()->set_url_request_context_getter(
        url_request_getter.get());
    storage_partition_proxy_.reset(
        new CefStoragePartitionProxy(partition_impl,
                                     url_request_getter.get()));

    // Associates UserData keys with the ResourceContext.
    // Called from StoragePartitionImplMap::Get() for CefBrowserContextImpl.
    content::InitializeResourceContext(this);
  }

  // There should only be one CefStoragePartitionProxy for this
  // CefBrowserContextProxy.
  DCHECK_EQ(storage_partition_proxy_->parent(), partition_impl);
  return storage_partition_proxy_.get();
}
