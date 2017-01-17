// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/metrics/statistics_recorder.h"
#include "base/test/launcher/unit_test_launcher.h"
#include "build/build_config.h"
#include "crypto/nss_util.h"
#include "net/socket/client_socket_pool_base.h"
#include "net/socket/ssl_server_socket.h"
#include "net/spdy/spdy_session.h"
#include "net/test/net_test_suite.h"
#include "url/url_features.h"

#if !defined(OS_ANDROID) && !defined(OS_IOS)
#include "mojo/edk/embedder/embedder.h"  // nogncheck
#endif

using net::internal::ClientSocketPoolBaseHelper;
using net::SpdySession;

int main(int argc, char** argv) {
  // Record histograms, so we can get histograms data in tests.
  base::StatisticsRecorder::Initialize();

  NetTestSuite test_suite(argc, argv);
  ClientSocketPoolBaseHelper::set_connect_backup_jobs_enabled(false);

  // Enable support for SSL server sockets, which must be done while
  // single-threaded.
  net::EnableSSLServerSockets();

#if !defined(OS_ANDROID) && !defined(OS_IOS)
  mojo::edk::Init();
#endif

  return base::LaunchUnitTests(
      argc, argv, base::Bind(&NetTestSuite::Run,
                             base::Unretained(&test_suite)));
}
