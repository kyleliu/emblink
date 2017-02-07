// Copyright (c) 2014 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/osr/render_widget_host_view_osr.h"

#include "libcef/browser/browser_host_impl.h"
#include "third_party/WebKit/public/platform/WebCursorInfo.h"

void CefRenderWidgetHostViewOSR::PlatformCreateCompositorWidget() {
}

void CefRenderWidgetHostViewOSR::PlatformResizeCompositorWidget(const gfx::Size&) {
}

void CefRenderWidgetHostViewOSR::PlatformDestroyCompositorWidget() {
}

ui::PlatformCursor CefRenderWidgetHostViewOSR::GetPlatformCursor(
    blink::WebCursorInfo::Type type) {
  return 0;
}
