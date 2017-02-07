// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/pepper/browser_pepper_host_factory.h"

#include "build/build_config.h"
#include "base/memory/ptr_util.h"
#include "content/public/browser/browser_ppapi_host.h"
#include "ppapi/host/message_filter_host.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/host/resource_host.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/shared_impl/ppapi_permissions.h"

using ppapi::host::MessageFilterHost;
using ppapi::host::ResourceHost;
using ppapi::host::ResourceMessageFilter;

CefBrowserPepperHostFactory::CefBrowserPepperHostFactory(
    content::BrowserPpapiHost* host)
    : host_(host) {}

CefBrowserPepperHostFactory::~CefBrowserPepperHostFactory() {}

std::unique_ptr<ResourceHost> CefBrowserPepperHostFactory::CreateResourceHost(
    ppapi::host::PpapiHost* host,
    PP_Resource resource,
    PP_Instance instance,
    const IPC::Message& message) {
  DCHECK(host == host_->GetPpapiHost());

  // Make sure the plugin is giving us a valid instance for this resource.
  if (!host_->IsValidInstance(instance))
    return nullptr;

  NOTREACHED() << "Unhandled message type: " << message.type();
  return nullptr;
}
