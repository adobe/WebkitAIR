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

#include "config.h"
#include "LocalizedStrings.h"
#include "PlatformString.h"
#include "IntSize.h"
#include <WebKitApollo/WebKit.h>
#include <wtf/text/CString.h>

namespace WebKitApollo { extern WebKitAPIHostFunctions* g_HostFunctions; }

namespace WebCore {

String contextMenuItemTagOpenLinkInNewWindow() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__NAVIGATION__OPEN_LINK_IN_NEW_WINDOW" ) );
}

String contextMenuItemTagDownloadLinkToDisk() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__NAVIGATION__DOWNLOAD_LINKED_FILE" ) );
}

String contextMenuItemTagCopyLinkToClipboard() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__EDIT__COPY_LINK" ) );
}

String contextMenuItemTagOpenImageInNewWindow() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__NAVIGATION__OPEN_IMAGE_IN_NEW_WINDOW" ) );
}

String contextMenuItemTagDownloadImageToDisk()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__NAVIGATION__DOWNLOAD_IMAGE" ) );
}

String contextMenuItemTagCopyImageToClipboard() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__EDIT__COPY_IMAGE" ) );
}

String contextMenuItemTagOpenFrameInNewWindow() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__NAVIGATION__OPEN_FRAME_IN_NEW_WINDOW" ) );
}

String contextMenuItemTagCopy() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "EDIT__COPY" ) );
}

String contextMenuItemTagGoBack() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__NAVIGATION__GO_BACK" ) );
}

String contextMenuItemTagGoForward() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__NAVIGATION__GO_FORWARD" ) );
}

String contextMenuItemTagStop() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__NAVIGATION__STOP" ) );
}

String contextMenuItemTagReload() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__NAVIGATION__RELOAD" ) );
}

String contextMenuItemTagCut() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "EDIT__CUT" ) );
}

String contextMenuItemTagPaste() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "EDIT__PASTE" ) );
}

String contextMenuItemTagNoGuessesFound() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__SPELLING__NO_GUESSES_FOUND" ) );
}

String contextMenuItemTagIgnoreSpelling() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__SPELLING__IGNORE_SPELLING" ) );
}

String contextMenuItemTagLearnSpelling() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__SPELLING__LEARN_SPELLING" ) );
}

String contextMenuItemTagSearchWeb() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__SEARCH__SEARCH_THE_WEB" ) );
}

String contextMenuItemTagLookUpInDictionary() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__SPELLING__SEARCH_IN_DICTIONARY" ) );
}

String contextMenuItemTagOpenLink() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__NAVIGATION__OPEN_LINK" ) );
}

String contextMenuItemTagIgnoreGrammar() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__SPELLING__IGNORE_GRAMMAR" ) );
}

String contextMenuItemTagSpellingMenu() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__SPELLING__SPELLING_AND_GRAMMAR" ) );
}

String contextMenuItemTagShowSpellingPanel(bool show) 
{
    return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( show ? "HTML_CONTEXTMENU__SPELLING__SHOW" : "HTML_CONTEXTMENU__SPELLING__HIDE" ) );
}

String contextMenuItemTagCheckSpelling() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__SPELLING__CHECK_SPELLING" ) );
}

String contextMenuItemTagCheckSpellingWhileTyping() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__SPELLING__CHECK_WHILE_TYPING" ) );
}

String contextMenuItemTagCheckGrammarWithSpelling() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__SPELLING__CHECK_GRAMMAR" ) );
}

String contextMenuItemTagFontMenu() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__FONTS" ) );
}

String contextMenuItemTagBold() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__STYLE__BOLD" ) );
}

String contextMenuItemTagItalic() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__STYLE__ITALICS" ) );
}

String contextMenuItemTagUnderline() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__STYLE__UNDERLINE" ) );
}

String contextMenuItemTagOutline()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__STYLE__OUTLINE" ) );
}

String contextMenuItemTagWritingDirectionMenu()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__STYLE__WRITING_DIRECTION" ) );
}
	
String contextMenuItemTagTextDirectionMenu()
{
#if PLATFORM(WIN)
#pragma message(TODO "HTML_CONTEXTMENU__STYLE__TEXT_DIRECTION must be defined")
#endif
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__STYLE__TEXT_DIRECTION" ) );
}	

String contextMenuItemTagDefaultDirection() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__STYLE__WRITING_DIRECTION__DEFAULT" ) );
}

String contextMenuItemTagLeftToRight() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__STYLE__WRITING_DIRECTION__LEFT_TO_RIGHT" ) );
}

String contextMenuItemTagRightToLeft() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__STYLE__WRITING_DIRECTION__RIGHT_TO_LEFT" ) );
}

String contextMenuItemTagInspectElement() 
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_CONTEXTMENU__INSPECT__ELEMENT" ) );
}

String inputElementAltText() 
{
    return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_INPUT_ELEMENT_ALT_TEXT" ) );
}

String resetButtonDefaultLabel()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_RESET_BUTTON_DEFAULT" ) );
}

String fileButtonChooseFileLabel()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_FILE_BUTTON_CHOOSE_FILE" ) );
}

String imageTitle(const String& filename, const IntSize& size)
{
	String str = String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_IMAGE_TITLE" ) );
    //mihnea integrate -> here we have a warning: format not a string literal, argument types not checked
    //i removed -Wformat=2 from the project and replaced with -Wformat -Wformat-security -Wformat-y2k
    //in fact Wformat=2 is equiv with -Wformat -Wformat-nonliteral -Wformat-security -Wformat-y2k
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
    return String::format(str.utf8().data(), filename.utf8().data(), size.width(), size.height());
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

String searchMenuClearRecentSearchesText()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_SEARCH_MENU__CLEAR_RECENT_SEARCHES" ) );
}

String searchMenuNoRecentSearchesText()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_SEARCH_MENU__NO_RECENT_SEARCHES" ) );
}

String searchMenuRecentSearchesText() 
{    
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_SEARCH_MENU__RECENT_SEARCHES" ) );
}

String searchableIndexIntroduction()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_SEARCHABLE_INDEX_INTRO" ) );
}

String submitButtonDefaultLabel()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_SUBMIT_BUTTON_DEFAULT" ) );
}

String AXWebAreaText()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_AX_WEB_AREA" ) );
}

String AXLinkText()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_AX_LINK" ) );
}

String AXListMarkerText()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_AX_LIST_MARKER" ) );
}

String AXImageMapText()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_AX_IMAGE_MAP" ) );
}

String AXHeadingText()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_AX_HEADING" ) );
}

String AXButtonActionVerb()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_AX_BUTTON_ACTION_VERB" ) );
}

String AXRadioButtonActionVerb()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_AX_RADIO_BUTTON_ACTION_VERB" ) );
}

String AXTextFieldActionVerb()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_AX_TEXT_FIELD_ACTION_VERB" ) );
}

String AXCheckedCheckBoxActionVerb()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_AX_CHECKED_CHECK_BOX_ACTION_VERB" ) );
}

String AXUncheckedCheckBoxActionVerb()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_AX_UNCHECKED_CHECK_BOX_ACTION_VERB" ) );
}

String AXLinkActionVerb()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_AX_LINK_ACTION_VERB" ) );
}


String unknownFileSizeText()
{
	return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_UNKNOWN_FILE_SIZE" ) );
}

String AXDefinitionListTermText()
{
//RHU "Localize: AXDefinitionListTermText")
    return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_AX_DEFINITION_LIST_TERM_TEXT" ) );
}

String AXDefinitionListDefinitionText()
{
//RHU "Localize: AXDefinitionListDefinitionText")
    return String::fromUTF8( WebKitApollo::g_HostFunctions->getLocalizedText( "HTML_AX_DEFINITION_LIST_DEFINITION_TEXT" ) );
}

//TODO Apollo integrate
String validationMessageValueMissingText() { 
    return "";
    //return String(LPCTSTR_UI_STRING("value missing", "Validation message for required form control elements that have no value")); 
}

String validationMessageTypeMismatchText() { 
    return "";
    //return String(LPCTSTR_UI_STRING("type mismatch", "Validation message for input form controls with a value not matching type")); 
}

String validationMessagePatternMismatchText() { 
    return "";
    //return String(LPCTSTR_UI_STRING("pattern mismatch", "Validation message for input form controls requiring a constrained value according to pattern")); 
}

String validationMessageTooLongText() { 
    return "";
    //return String(LPCTSTR_UI_STRING("too long", "Validation message for form control elements with a value longer than maximum allowed length")); 
}

String validationMessageRangeUnderflowText() { 
    return "";
    //return String(LPCTSTR_UI_STRING("range underflow", "Validation message for input form controls with value lower than allowed minimum")); 
}

String validationMessageRangeOverflowText() { 
    return "";
    //return String(LPCTSTR_UI_STRING("range overflow", "Validation message for input form controls with value higher than allowed maximum")); 
}

String validationMessageStepMismatchText() { 
    return "";
    //return String(LPCTSTR_UI_STRING("step mismatch", "Validation message for input form controls with value not respecting the step attribute")); 
}

String missingPluginText() { 
    return "";
    //return String(LPCTSTR_UI_STRING("Missing Plug-in", "Label text to be used when a plugin is missing")); 
}

String crashedPluginText() { 
    return "";
    //return String(LPCTSTR_UI_STRING("Plug-in Failure", "Label text to be used if plugin host process has crashed")); 
}

String AXMenuListActionVerb()
{
    return "";
}

String AXMenuListPopupActionVerb()
{
    return "";
}

}
