// Copyright 2016 The Chromium Embedded Framework Authors. Portions copyright
// 2016 The Chromium Authors. All rights reserved. Use of this source code is
// governed by a BSD-style license that can be found in the LICENSE file.

#include "include/cef_crash_util.h"
#include "libcef/common/cef_switches.h"

#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/debug/crash_logging.h"
#include "base/strings/string_util.h"

bool CefCrashReportingEnabled() {
  return false;
}

void CefSetCrashKeyValue(const CefString& key, const CefString& value) {
  base::debug::SetCrashKeyValue(key.ToString(), value.ToString());
}
