// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/renderer/pepper/renderer_pepper_host_factory.h"

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "content/public/renderer/renderer_ppapi_host.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/host/resource_host.h"
#include "ppapi/proxy/ppapi_message_utils.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/shared_impl/ppapi_permissions.h"

using ppapi::host::ResourceHost;

CefRendererPepperHostFactory::CefRendererPepperHostFactory(
    content::RendererPpapiHost* host)
    : host_(host) {}

CefRendererPepperHostFactory::~CefRendererPepperHostFactory() {}

std::unique_ptr<ResourceHost> CefRendererPepperHostFactory::CreateResourceHost(
    ppapi::host::PpapiHost* host,
    PP_Resource resource,
    PP_Instance instance,
    const IPC::Message& message) {
  DCHECK_EQ(host_->GetPpapiHost(), host);

  // Make sure the plugin is giving us a valid instance for this resource.
  if (!host_->IsValidInstance(instance))
    return nullptr;

  NOTREACHED() << "Unhandled message type: " << message.type();
  return nullptr;
}
