// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/common/resource_request_body.h"

#include "content/common/resource_request_body_impl.h"

namespace content {

ResourceRequestBody::ResourceRequestBody() {}

ResourceRequestBody::~ResourceRequestBody() {}

// static
scoped_refptr<ResourceRequestBody> ResourceRequestBody::CreateFromBytes(
    const char* bytes,
    size_t length) {
  scoped_refptr<ResourceRequestBodyImpl> result = new ResourceRequestBodyImpl();
  result->AppendBytes(bytes, length);
  return result;
}

}  // namespace content
