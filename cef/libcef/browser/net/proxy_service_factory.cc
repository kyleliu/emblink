// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/net/proxy_service_factory.h"

#include <stddef.h>
#include <string>
#include <utility>

#include "base/command_line.h"
#include "base/metrics/field_trial.h"
#include "base/strings/string_number_conversions.h"
#include "base/threading/thread.h"
#include "build/build_config.h"
// #include "chrome/browser/browser_process.h"
// #include "chrome/browser/io_thread.h"
// #include "chrome/common/chrome_switches.h"
#include "components/proxy_config/pref_proxy_config_tracker_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/content_switches.h"
#include "net/proxy/dhcp_proxy_script_fetcher_factory.h"
#include "net/proxy/proxy_config_service.h"
#include "net/proxy/proxy_resolver_v8.h"
#include "net/proxy/proxy_script_fetcher_impl.h"
#include "net/proxy/proxy_service.h"
#include "net/proxy/proxy_service_v8.h"
#include "net/url_request/url_request_context.h"

#if !defined(OS_ANDROID)
#include "chrome/browser/net/utility_process_mojo_proxy_resolver_factory.h"
#include "net/proxy/proxy_service_mojo.h"
#endif

using content::BrowserThread;

// static
std::unique_ptr<net::ProxyConfigService>
ProxyServiceFactory::CreateProxyConfigService(PrefProxyConfigTracker* tracker) {
  // The linux gconf-based proxy settings getter relies on being initialized
  // from the UI thread.
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  std::unique_ptr<net::ProxyConfigService> base_service;

  // On ChromeOS, base service is NULL; chromeos::ProxyConfigServiceImpl
  // determines the effective proxy config to take effect in the network layer,
  // be it from prefs or system (which is network shill on chromeos).

  // For other platforms, create a baseline service that provides proxy
  // configuration in case nothing is configured through prefs (Note: prefs
  // include command line and configuration policy).

  // TODO(port): the IO and FILE message loops are only used by Linux.  Can
  // that code be moved to chrome/browser instead of being in net, so that it
  // can use BrowserThread instead of raw MessageLoop pointers? See bug 25354.
  base_service = net::ProxyService::CreateSystemProxyConfigService(
      BrowserThread::GetTaskRunnerForThread(BrowserThread::IO),
      BrowserThread::GetTaskRunnerForThread(BrowserThread::FILE));

  return tracker->CreateTrackingProxyConfigService(std::move(base_service));
}

// static
PrefProxyConfigTracker*
ProxyServiceFactory::CreatePrefProxyConfigTrackerOfProfile(
    PrefService* profile_prefs,
    PrefService* local_state_prefs) {
  return new PrefProxyConfigTrackerImpl(
      profile_prefs, BrowserThread::GetTaskRunnerForThread(BrowserThread::IO));
}

// static
PrefProxyConfigTracker*
ProxyServiceFactory::CreatePrefProxyConfigTrackerOfLocalState(
    PrefService* local_state_prefs) {
  return new PrefProxyConfigTrackerImpl(
      local_state_prefs,
      BrowserThread::GetTaskRunnerForThread(BrowserThread::IO));
}

// static
std::unique_ptr<net::ProxyService> ProxyServiceFactory::CreateProxyService(
    net::NetLog* net_log,
    net::URLRequestContext* context,
    net::NetworkDelegate* network_delegate,
    std::unique_ptr<net::ProxyConfigService> proxy_config_service,
    const base::CommandLine& command_line,
    bool quick_check_enabled,
    bool pac_https_url_stripping_enabled) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  bool use_v8 = true;
  size_t num_pac_threads = 0u;  // Use default number of threads.

  std::unique_ptr<net::ProxyService> proxy_service;
  if (use_v8) {
    std::unique_ptr<net::DhcpProxyScriptFetcher> dhcp_proxy_script_fetcher;
    net::DhcpProxyScriptFetcherFactory dhcp_factory;
    dhcp_proxy_script_fetcher = dhcp_factory.Create(context);

    if (!proxy_service) {
      proxy_service = net::CreateProxyServiceUsingV8ProxyResolver(
          std::move(proxy_config_service),
          new net::ProxyScriptFetcherImpl(context),
          std::move(dhcp_proxy_script_fetcher), context->host_resolver(),
          net_log, network_delegate);
    }
  } else {
    proxy_service = net::ProxyService::CreateUsingSystemProxyResolver(
        std::move(proxy_config_service), num_pac_threads, net_log);
  }

  proxy_service->set_quick_check_enabled(quick_check_enabled);
  proxy_service->set_sanitize_url_policy(
      pac_https_url_stripping_enabled
          ? net::ProxyService::SanitizeUrlPolicy::SAFE
          : net::ProxyService::SanitizeUrlPolicy::UNSAFE);

  return proxy_service;
}
