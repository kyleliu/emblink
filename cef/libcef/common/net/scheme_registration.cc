// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "libcef/common/net/scheme_registration.h"

#include "libcef/common/content_client.h"

#include "content/public/common/url_constants.h"
#include "url/url_constants.h"

namespace scheme {

void AddInternalSchemes(std::vector<std::string>* standard_schemes,
                        std::vector<std::string>* savable_schemes) {
}

bool IsInternalHandledScheme(const std::string& scheme) {
  static const char* schemes[] = {
    url::kBlobScheme,
    content::kChromeDevToolsScheme,
    content::kChromeUIScheme,
    url::kDataScheme,
    url::kFileScheme,
    url::kFileSystemScheme,
  };

  for (size_t i = 0; i < sizeof(schemes) / sizeof(schemes[0]); ++i) {
    if (scheme == schemes[i])
      return true;
  }

  return false;
}

bool IsInternalProtectedScheme(const std::string& scheme) {
  // Some of these values originate from StoragePartitionImplMap::Get() in
  // content/browser/storage_partition_impl_map.cc and are modified by
  // InstallInternalProtectedHandlers().
  static const char* schemes[] = {
    url::kBlobScheme,
    content::kChromeUIScheme,
    url::kDataScheme,
    url::kFileScheme,
    url::kFileSystemScheme,
    url::kFtpScheme,
  };

  for (size_t i = 0; i < sizeof(schemes) / sizeof(schemes[0]); ++i) {
    if (scheme == schemes[i])
      return true;
  }

  return false;
}

}  // namespace scheme
