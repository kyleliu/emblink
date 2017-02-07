// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/renderer/media/cef_key_systems.h"

#include <string>
#include <vector>

#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/string_split.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/renderer/render_thread.h"
#include "libcef/common/cef_messages.h"
#include "media/base/eme_constants.h"
#include "media/base/key_system_properties.h"

using media::KeySystemProperties;
using media::SupportedCodecs;

void AddCefKeySystems(
    std::vector<std::unique_ptr<KeySystemProperties>>* key_systems_properties) {
}
