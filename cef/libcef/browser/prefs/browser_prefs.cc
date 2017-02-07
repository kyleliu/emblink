// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "libcef/browser/prefs/browser_prefs.h"

#include "libcef/browser/browser_context.h"
#include "libcef/browser/media_capture_devices_dispatcher.h"
#include "libcef/browser/net/url_request_context_getter_impl.h"
#include "libcef/browser/prefs/renderer_prefs.h"
#include "libcef/common/cef_switches.h"

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"
#include "components/content_settings/core/browser/cookie_settings.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/prefs/json_pref_store.h"
#include "components/prefs/pref_filter.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_service_factory.h"
#include "components/prefs/testing_pref_store.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/proxy_config/pref_proxy_config_tracker_impl.h"
#include "components/proxy_config/proxy_config_dictionary.h"
#include "content/public/browser/browser_thread.h"
#include "ui/base/l10n/l10n_util.h"

namespace browser_prefs {

namespace {

// A helper function for registering localized values.
// Based on CreateLocaleDefaultValue from
// components/pref_registry/pref_registry_syncable.cc.
// void RegisterLocalizedValue(PrefRegistrySimple* registry,
//                             const char* path,
//                             base::Value::Type type,
//                             int message_id) {
//   const std::string resource_string = l10n_util::GetStringUTF8(message_id);
//   DCHECK(!resource_string.empty());
//   switch (type) {
//     case base::Value::TYPE_BOOLEAN: {
//       if ("true" == resource_string)
//         registry->RegisterBooleanPref(path, true);
//       else if ("false" == resource_string)
//         registry->RegisterBooleanPref(path, false);
//       return;
//     }

//     case base::Value::TYPE_INTEGER: {
//       int val;
//       base::StringToInt(resource_string, &val);
//       registry->RegisterIntegerPref(path, val);
//       return;
//     }

//     case base::Value::TYPE_DOUBLE: {
//       double val;
//       base::StringToDouble(resource_string, &val);
//       registry->RegisterDoublePref(path, val);
//       return;
//     }

//     case base::Value::TYPE_STRING: {
//       registry->RegisterStringPref(path, resource_string);
//       return;
//     }

//     default: {
//       NOTREACHED() <<
//           "list and dictionary types cannot have default locale values";
//     }
//   }
//   NOTREACHED();
// }

}  // namespace

const char kUserPrefsFileName[] = "UserPrefs.json";

std::unique_ptr<PrefService> CreatePrefService(
    CefBrowserContext* profile,
    const base::FilePath& cache_path,
    bool persist_user_preferences) {
  // const base::CommandLine* command_line =
  //     base::CommandLine::ForCurrentProcess();

  PrefServiceFactory factory;

  // Used to store command-line preferences, most of which will be evaluated in
  // the CommandLinePrefStore constructor. Preferences set in this manner cannot
  // be overridden by the user.
  // scoped_refptr<ChromeCommandLinePrefStore> command_line_pref_store(
  //     new ChromeCommandLinePrefStore(command_line));
  // renderer_prefs::SetCommandLinePrefDefaults(command_line_pref_store.get());
  // factory.set_command_line_prefs(command_line_pref_store);

  // True if preferences will be stored on disk.
  const bool store_on_disk = !cache_path.empty() && persist_user_preferences;

  scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner;
  if (store_on_disk) {
    // Get sequenced task runner for making sure that file operations of
    // this profile (defined by |cache_path|) are executed in expected order
    // (what was previously assured by the FILE thread).
    sequenced_task_runner = JsonPrefStore::GetTaskRunnerForFile(
        cache_path, content::BrowserThread::GetBlockingPool());
  }

  // Used to store user preferences.
  scoped_refptr<PersistentPrefStore> user_pref_store;
  if (store_on_disk) {
    const base::FilePath& pref_path =
        cache_path.AppendASCII(browser_prefs::kUserPrefsFileName);
    scoped_refptr<JsonPrefStore> json_pref_store =
        new JsonPrefStore(pref_path, sequenced_task_runner,
                          std::unique_ptr<PrefFilter>());
    factory.set_user_prefs(json_pref_store.get());
  } else {
    scoped_refptr<TestingPrefStore> testing_pref_store = new TestingPrefStore();
    testing_pref_store->SetInitializationCompleted();
    factory.set_user_prefs(testing_pref_store.get());
  }

  // Registry that will be populated with all known preferences. Preferences
  // are registered with default values that may be changed via a *PrefStore.
  scoped_refptr<user_prefs::PrefRegistrySyncable> registry(
      new user_prefs::PrefRegistrySyncable());

  // Default preferences.
  CefMediaCaptureDevicesDispatcher::RegisterPrefs(registry.get());
  PrefProxyConfigTrackerImpl::RegisterPrefs(registry.get());
  HostContentSettingsMap::RegisterProfilePrefs(registry.get());
  CefURLRequestContextGetterImpl::RegisterPrefs(registry.get());
  renderer_prefs::RegisterProfilePrefs(registry.get());
  content_settings::CookieSettings::RegisterProfilePrefs(registry.get());

  // Build the PrefService that manages the PrefRegistry and PrefStores.
  return factory.Create(registry.get());
}

}  // namespace browser_prefs
