// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "include/cef_web_plugin.h"
#include "libcef/browser/context.h"
#include "libcef/browser/thread_util.h"

#include "base/bind.h"
#include "base/files/file_path.h"

// Global functions.

void CefVisitWebPluginInfo(CefRefPtr<CefWebPluginInfoVisitor> visitor) {
}

void CefRefreshWebPlugins() {
}

void CefUnregisterInternalWebPlugin(const CefString& path) {
}

void CefRegisterWebPluginCrash(const CefString& path) {
}

void CefIsWebPluginUnstable(
    const CefString& path,
    CefRefPtr<CefWebPluginUnstableCallback> callback) {
}

void CefRegisterWidevineCdm(const CefString& path,
                            CefRefPtr<CefRegisterCdmCallback> callback) {
}
