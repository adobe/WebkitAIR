/*
 * Copyright (C) 2007 Adobe Systems Incorporated.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Adobe Systems Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ADOBE SYSTEMS INCORPORATED "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ADOBE SYSTEMS INCORPORATED
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WebPopupWindow_h
#define WebPopupWindow_h

#include <WebKitApollo/WebConfig.h>

struct WebPopupWindow;
struct WebPopupWindowVTable;

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebPopupWindowAddOption )	( struct WebPopupWindow* pPopupWindow
																					, const uint16_t* const pUTF16OptionText
																					, unsigned long const numOptionTextCodeUnits
																					, int const itemIndex
																					, bool const isSelected
																					, bool const isDisabled
																					, bool const isChildOfGroup
																					, float const naturalTextWidth
																					);
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebPopupWindowAddGroupLabel )	( struct WebPopupWindow* pPopupWindow
																						, const uint16_t* const pUTF16OptionText
																						, unsigned long const numOptionTextCodeUnits
																						, int const itemIndex
																						, bool const isDisabled
																						, float const naturalTextWidth
																						);
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebPopupWindowAddSeparator )	( struct WebPopupWindow* pPopupWindow
																						, int const itemIndex
																						);
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebPopupWindowShow )	( struct WebPopupWindow* pPopupWindow
																				, int const windowX
                                                                                , int const windowY
                                                                                , int const popupWindowWidth
                                                                                );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebPopupWindowHide )( struct WebPopupWindow* pPopupWindow );
//typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebPopupWindowOnItemClicked )( struct WebPopupWindow* pPopupWindow, int const itemIndex );
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebPopupWindowUpdateSelectedItem)( struct WebPopupWindow* pPopupWindow, int const itemIndex );

struct WebPopupWindowVTable
{
	unsigned long m_vTableSize;
	WebPopupWindowAddOption addOption;
	WebPopupWindowAddGroupLabel addGroupLabel;
	WebPopupWindowAddSeparator addSeparator;
	WebPopupWindowShow show;
	WebPopupWindowHide hide;
//	WebPopupWindowOnItemClicked onItemClicked;
	WebPopupWindowUpdateSelectedItem updateSelectedItem;
};

struct WebPopupWindow
{
	const WebPopupWindowVTable* m_pVTable;
};

#endif
