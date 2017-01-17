// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// ---------------------------------------------------------------------------
//
// This file was generated by the CEF translator tool. If making changes by
// hand only do so within the body of existing method and function
// implementations. See the translator.README.txt file in the tools directory
// for more information.
//

#include "libcef_dll/ctocpp/navigation_entry_ctocpp.h"
#include "libcef_dll/ctocpp/sslstatus_ctocpp.h"


// VIRTUAL METHODS - Body may be edited by hand.

bool CefNavigationEntryCToCpp::IsValid() {
  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, is_valid))
    return false;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->is_valid(_struct);

  // Return type: bool
  return _retval?true:false;
}

CefString CefNavigationEntryCToCpp::GetURL() {
  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_url))
    return CefString();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_string_userfree_t _retval = _struct->get_url(_struct);

  // Return type: string
  CefString _retvalStr;
  _retvalStr.AttachToUserFree(_retval);
  return _retvalStr;
}

CefString CefNavigationEntryCToCpp::GetDisplayURL() {
  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_display_url))
    return CefString();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_string_userfree_t _retval = _struct->get_display_url(_struct);

  // Return type: string
  CefString _retvalStr;
  _retvalStr.AttachToUserFree(_retval);
  return _retvalStr;
}

CefString CefNavigationEntryCToCpp::GetOriginalURL() {
  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_original_url))
    return CefString();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_string_userfree_t _retval = _struct->get_original_url(_struct);

  // Return type: string
  CefString _retvalStr;
  _retvalStr.AttachToUserFree(_retval);
  return _retvalStr;
}

CefString CefNavigationEntryCToCpp::GetTitle() {
  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_title))
    return CefString();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_string_userfree_t _retval = _struct->get_title(_struct);

  // Return type: string
  CefString _retvalStr;
  _retvalStr.AttachToUserFree(_retval);
  return _retvalStr;
}

CefNavigationEntry::TransitionType CefNavigationEntryCToCpp::GetTransitionType(
    ) {
  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_transition_type))
    return TT_EXPLICIT;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_transition_type_t _retval = _struct->get_transition_type(_struct);

  // Return type: simple
  return _retval;
}

bool CefNavigationEntryCToCpp::HasPostData() {
  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, has_post_data))
    return false;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->has_post_data(_struct);

  // Return type: bool
  return _retval?true:false;
}

CefTime CefNavigationEntryCToCpp::GetCompletionTime() {
  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_completion_time))
    return CefTime();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_time_t _retval = _struct->get_completion_time(_struct);

  // Return type: simple
  return _retval;
}

int CefNavigationEntryCToCpp::GetHttpStatusCode() {
  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_http_status_code))
    return 0;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->get_http_status_code(_struct);

  // Return type: simple
  return _retval;
}

CefRefPtr<CefSSLStatus> CefNavigationEntryCToCpp::GetSSLStatus() {
  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_sslstatus))
    return NULL;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_sslstatus_t* _retval = _struct->get_sslstatus(_struct);

  // Return type: refptr_same
  return CefSSLStatusCToCpp::Wrap(_retval);
}


// CONSTRUCTOR - Do not edit by hand.

CefNavigationEntryCToCpp::CefNavigationEntryCToCpp() {
}

template<> cef_navigation_entry_t* CefCToCpp<CefNavigationEntryCToCpp,
    CefNavigationEntry, cef_navigation_entry_t>::UnwrapDerived(
    CefWrapperType type, CefNavigationEntry* c) {
  NOTREACHED() << "Unexpected class type: " << type;
  return NULL;
}

#if DCHECK_IS_ON()
template<> base::AtomicRefCount CefCToCpp<CefNavigationEntryCToCpp,
    CefNavigationEntry, cef_navigation_entry_t>::DebugObjCt = 0;
#endif

template<> CefWrapperType CefCToCpp<CefNavigationEntryCToCpp,
    CefNavigationEntry, cef_navigation_entry_t>::kWrapperType =
    WT_NAVIGATION_ENTRY;