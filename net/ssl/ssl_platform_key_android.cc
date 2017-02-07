// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/cert/x509_certificate.h"
#include "net/ssl/openssl_client_key_store.h"
#include "net/ssl/ssl_platform_key.h"
#include "net/ssl/ssl_private_key.h"

namespace net {

scoped_refptr<SSLPrivateKey> FetchClientCertPrivateKey(
    X509Certificate* certificate) {
  return OpenSSLClientKeyStore::GetInstance()->FetchClientCertPrivateKey(
      certificate);
}

}  // namespace net
