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

#include <config.h>
#include "WebEditorClient.h"

#include <EditCommand.h>
#include <KeyboardEvent.h>
#include <PlatformKeyboardEvent.h>
#include <Document.h>
#include <Frame.h>
#include <SelectionController.h>
#include <TypingCommand.h>
#include <WebKitApollo/WebKeyboardCodes.h>
#include <Editor.h>
#include <Frame.h>
#include <Page.h>
#include "WebFrameImpl.h"
#include <EventNames.h>

// Arbitrary depth limit for the undo stack, to keep it from using
// unbounded memory.  This is the maximum number of distinct undoable
// actions 
#define maximumUndoStackDepth 1000

namespace WebKitApollo {
WebEditorClient::WebEditorClient(WebViewImpl* webView)
    : m_webView(webView)
    , m_isInRedo(false)
{
}

WebEditorClient::~WebEditorClient()
{
}

void WebEditorClient::pageDestroyed()
{
    delete this;
}

bool WebEditorClient::shouldDeleteRange(WebCore::Range*)
{
    // The Safari WebEditorClient asks WebDefaultEditingDelegate returns YES
    return true;
}

bool WebEditorClient::shouldShowDeleteInterface(WebCore::HTMLElement*)
{
    // The Safari WebEditorClient asks WebDefaultEditingDelegate returns NO
    return false;
}

bool WebEditorClient::smartInsertDeleteEnabled()
{
    // The Safari WebEditorClient asks the webView which inits the smartInsertDeleteEnabled memeber to true.
    return true;
}

bool WebEditorClient::isContinuousSpellCheckingEnabled()
{
    // The Safari WebEditorClient asks the webView which by default 
    // returns YES, but we don't support spell checking so return false.
    return false;
}

void WebEditorClient::toggleContinuousSpellChecking()
{
}

bool WebEditorClient::isGrammarCheckingEnabled()
{
    // The Safari WebEditorClient asks the webView which by default 
    // returns YES, but we don't support grammar checking so return false.
    return false;
}

void WebEditorClient::toggleGrammarChecking()
{
}

int WebEditorClient::spellCheckerDocumentTag()
{
    return 0;
}

bool WebEditorClient::isEditable()
{
    // The Safari WebEditorClient asks the webView.
    // return false is correct here unless we want to make
    // the entire document editable ( an email client would do this for composing HTML mail ).
    return false;
}

bool WebEditorClient::shouldBeginEditing(WebCore::Range*)
{
    // Returns whether the user is allowed to edit a range of content in a document.
    return true;
}

bool WebEditorClient::shouldEndEditing(WebCore::Range*)
{
    // Returns whether the user should be allowed to end editing.
    return true;
}

bool WebEditorClient::shouldInsertNode(WebCore::Node*, WebCore::Range*, WebCore::EditorInsertAction)
{
    // Returns whether the user should be allowed to insert a node in place of a range of content.
    return true;
}

bool WebEditorClient::shouldInsertText(const WebCore::String&, WebCore::Range*, WebCore::EditorInsertAction)
{
    // Returns whether a user should be allowed to insert text in place of a range of content.
    return true;
}

bool WebEditorClient::shouldChangeSelectedRange(WebCore::Range* fromRange, WebCore::Range* toRange, WebCore::EAffinity, bool stillSelecting)
{
    // Returns whether the user should be allowed to change the selected range.
    return true;
}

bool WebEditorClient::shouldApplyStyle(WebCore::CSSStyleDeclaration*, WebCore::Range*)
{
    // Returns whether the user should be allowed to apply a style to a range of content.
    return true;
}


bool WebEditorClient::shouldMoveRangeAfterDelete(WebCore::Range*, WebCore::Range*)
{
    return true;
}

void WebEditorClient::didBeginEditing()
{
}

void WebEditorClient::respondToChangedContents()
{
}

void WebEditorClient::respondToChangedSelection()
{
    m_webView->selectionChanged();
}

void WebEditorClient::didEndEditing()
{
}

void WebEditorClient::didWriteSelectionToPasteboard()
{
}

void WebEditorClient::didSetSelectionTypesForPasteboard()
{
}

void WebEditorClient::registerCommandForUndo(WTF::PassRefPtr<WebCore::EditCommand> command)
{
    if (undoStack.size() == maximumUndoStackDepth)
        undoStack.removeFirst();
    if (!m_isInRedo)
        redoStack.clear();
    undoStack.append(command);    
}

void WebEditorClient::registerCommandForRedo(WTF::PassRefPtr<WebCore::EditCommand> command)
{
    redoStack.append(command);
}

void WebEditorClient::clearUndoRedoOperations()
{
    undoStack.clear();
    redoStack.clear();    
}

bool WebEditorClient::canUndo() const
{
    return !undoStack.isEmpty();
}

bool WebEditorClient::canRedo() const
{
    return !redoStack.isEmpty();
}

void WebEditorClient::undo()
{
    if (canUndo()) {
        RefPtr<WebCore::EditCommand> command(*(--undoStack.end()));
        undoStack.remove(--undoStack.end());
        // unapply will call us back to push this command onto the redo stack.
        command->unapply();
    }    
}

void WebEditorClient::redo()
{
    if (canRedo()) {
        RefPtr<WebCore::EditCommand> command(*(--redoStack.end()));
        redoStack.remove(--redoStack.end());
        
        ASSERT(!m_isInRedo);
        m_isInRedo = true;
        // reapply will call us back to push this command onto the undo stack.
        command->reapply();
        m_isInRedo = false;
    }    
}

static const unsigned s_CtrlKey  = 1 << 0;
static const unsigned s_AltKey   = 1 << 1;
static const unsigned s_ShiftKey = 1 << 2;
static const unsigned s_MetaKey  = 1 << 3;


struct KeyDownEntry {
    unsigned virtualKey;
    unsigned modifiers;
    const char* name;
};

struct KeyPressEntry {
    unsigned charCode;
    unsigned modifiers;
    const char* name;
};

static const KeyDownEntry keyDownEntries[] = {
    { Web_VK_LEFT,   0,                      "MoveLeft"                                    },
    { Web_VK_LEFT,   s_ShiftKey,             "MoveLeftAndModifySelection"                  },
    { Web_VK_RIGHT,  0,                      "MoveRight"                                   },
    { Web_VK_RIGHT,  s_ShiftKey,             "MoveRightAndModifySelection"                 },
    { Web_VK_UP,     0,                      "MoveUp"                                      },
    { Web_VK_UP,     s_ShiftKey,             "MoveUpAndModifySelection"                    },
    { Web_VK_DOWN,   0,                      "MoveDown"                                    },
    { Web_VK_DOWN,   s_ShiftKey,             "MoveDownAndModifySelection"                  },
    { Web_VK_HOME,   0,                      "MoveToBeginningOfLine"                       },
    { Web_VK_HOME,   s_ShiftKey,             "MoveToBeginningOfLineAndModifySelection"     },
    { Web_VK_END,    0,                      "MoveToEndOfLine"                             },
    { Web_VK_END,    s_ShiftKey,             "MoveToEndOfLineAndModifySelection"           },
    { Web_VK_PRIOR,  s_ShiftKey,             "MoveUpAndModifySelection"                    },
    { Web_VK_NEXT,   s_ShiftKey,             "MoveDownAndModifySelection"                  },
#if OS(DARWIN)
    { Web_VK_LEFT,   s_MetaKey,              "MoveToBeginningOfLine"                       },
    { Web_VK_LEFT,   s_MetaKey | s_ShiftKey, "MoveToBeginningOfLineAndModifySelection"     },
    { Web_VK_LEFT,   s_AltKey,               "MoveWordLeft"                                },
    { Web_VK_LEFT,   s_AltKey | s_ShiftKey,  "MoveWordLeftAndModifySelection"              },
    { Web_VK_RIGHT,  s_MetaKey,              "MoveToEndOfLine"                             },
    { Web_VK_RIGHT,  s_MetaKey | s_ShiftKey, "MoveToEndOfLineAndModifySelection"           },
    { Web_VK_RIGHT,  s_AltKey,               "MoveWordRight"                               },
    { Web_VK_RIGHT,  s_AltKey | s_ShiftKey,  "MoveWordRightAndModifySelection"             },
    { Web_VK_UP,     s_MetaKey,              "MoveToBeginningOfDocument"                   },
    { Web_VK_UP,     s_MetaKey | s_ShiftKey, "MoveToBeginningOfDocumentAndModifySelection" },
    { Web_VK_UP,     s_AltKey,               "MoveUpByPageAndModifyCaret"                  },
    { Web_VK_UP,     s_AltKey | s_ShiftKey,  "MoveToBeginningOfDocumentAndModifySelection" },
    { Web_VK_DOWN,   s_MetaKey,              "MoveToEndOfDocument"                         },
    { Web_VK_DOWN,   s_MetaKey | s_ShiftKey, "MoveToEndOfDocumentAndModifySelection"       },
    { Web_VK_DOWN,   s_AltKey,               "MoveDownByPageAndModifyCaret"                },
    { Web_VK_DOWN,   s_AltKey | s_ShiftKey,  "MoveToEndOfDocumentAndModifySelection"       },
    { Web_VK_PRIOR,  0,                      "MoveUpByPage"                                },
    { Web_VK_NEXT,   0,                      "MoveDownByPage"                              },
    { Web_VK_HOME,   s_MetaKey,              "MoveToBeginningOfLine"                       },
    { Web_VK_HOME,   s_MetaKey | s_ShiftKey, "MoveToBeginningOfLineAndModifySelection"     },
    { Web_VK_END,    s_MetaKey,              "MoveToEndOfLine"                             },
    { Web_VK_END,    s_MetaKey | s_ShiftKey, "MoveToEndOfLineAndModifySelection"           },
#else
    { Web_VK_INSERT, s_CtrlKey,              "Copy"                                        },
    { Web_VK_DELETE, s_ShiftKey,             "Cut"                                         },
    { Web_VK_INSERT, s_ShiftKey,             "Paste"                                       },
    { Web_VK_LEFT,   s_CtrlKey,              "MoveWordLeft"                                },
    { Web_VK_LEFT,   s_CtrlKey | s_ShiftKey, "MoveWordLeftAndModifySelection"              },
    { Web_VK_RIGHT,  s_CtrlKey,              "MoveWordRight"                               },
    { Web_VK_RIGHT,  s_CtrlKey | s_ShiftKey, "MoveWordRightAndModifySelection"             },
    { Web_VK_HOME,   s_CtrlKey,              "MoveToBeginningOfDocument"                   },
    { Web_VK_HOME,   s_CtrlKey | s_ShiftKey, "MoveToBeginningOfDocumentAndModifySelection" },
    { Web_VK_END,    s_CtrlKey,              "MoveToEndOfDocument"                         },
    { Web_VK_END,    s_CtrlKey | s_ShiftKey, "MoveToEndOfDocumentAndModifySelection"       },
    { Web_VK_PRIOR,  0,                      "MoveUpByPageAndModifyCaret"                  },
    { Web_VK_NEXT,   0,                      "MoveDownByPageAndModifyCaret"                },
#endif

    { Web_VK_BACK,   0,                  "BackwardDelete"                              },
    { Web_VK_BACK,   s_ShiftKey,           "BackwardDelete"                              },
    { Web_VK_DELETE, 0,                  "ForwardDelete"                               },
    { Web_VK_DELETE, s_ShiftKey,           "ForwardDelete"                               },
    { Web_VK_BACK,   s_CtrlKey,            "DeleteWordBackward"                          },
    { Web_VK_DELETE, s_CtrlKey,            "DeleteWordForward"                           },
    
    { Web_VK_ESCAPE, 0,                  "Cancel"                                      },
    { Web_VK_OEM_PERIOD, s_CtrlKey,        "Cancel"                                      },
    { Web_VK_TAB,    0,                  "InsertTab"                                   },
    { Web_VK_TAB,    s_ShiftKey,           "InsertBacktab"                               },
    { Web_VK_RETURN, 0,                  "InsertNewline"                               },
    { Web_VK_RETURN, s_CtrlKey,            "InsertNewline"                               },
    { Web_VK_RETURN, s_AltKey,             "InsertNewline"                               },
    { Web_VK_RETURN, s_AltKey | s_ShiftKey,  "InsertNewline"                               },

	// these may cause problems with if we allow apps to override what these key combinations do.
	//
#if OS(DARWIN)
    { 'B',       s_MetaKey,            "ToggleBold"                                  },
    { 'I',       s_MetaKey,            "ToggleItalic"                                },

    { 'C',       s_MetaKey,            "Copy"                                        },
    { 'V',       s_MetaKey,            "Paste"                                       },
    { 'X',       s_MetaKey,            "Cut"                                         },
    { 'A',       s_MetaKey,            "SelectAll"                                   },
#else
    { 'B',       s_CtrlKey,            "ToggleBold"                                  },
    { 'I',       s_CtrlKey,            "ToggleItalic"                                },

    { 'C',       s_CtrlKey,            "Copy"                                        },
    { 'V',       s_CtrlKey,            "Paste"                                       },
    { 'X',       s_CtrlKey,            "Cut"                                         },
    { 'A',       s_CtrlKey,            "SelectAll"                                   },
#endif
};

static const KeyPressEntry keyPressEntries[] = {
    { Web_VK_TAB,       0,                      "InsertTab"                                   },
    { Web_VK_TAB,       s_ShiftKey,             "InsertBacktab"                               },
    { Web_VK_RETURN,    0,                      "InsertNewline"                               },
    { Web_VK_RETURN,    s_CtrlKey,              "InsertNewline"                               },
    { Web_VK_RETURN,    s_AltKey,               "InsertNewline"                               },
    { Web_VK_RETURN,    s_AltKey | s_ShiftKey,  "InsertNewline"                               },
};

static const char* interpretKeyEvent(const WebCore::KeyboardEvent* evt)
{
    ASSERT(evt->type() == WebCore::eventNames().keydownEvent || evt->type() == WebCore::eventNames().keypressEvent);

    static HashMap<int, const char*>* keyDownCommandsMap = 0;
    static HashMap<int, const char*>* keyPressCommandsMap = 0;

    if (!keyDownCommandsMap) {
        keyDownCommandsMap = new HashMap<int, const char*>;
        keyPressCommandsMap = new HashMap<int, const char*>;

        unsigned numberOfKeyDownItems = sizeof(keyDownEntries)/sizeof(keyDownEntries[0]);
        for (unsigned i = 0; i < numberOfKeyDownItems; i++)
            keyDownCommandsMap->set(keyDownEntries[i].modifiers << 16 | keyDownEntries[i].virtualKey, keyDownEntries[i].name);

        unsigned numberOfKeyPressItems = sizeof(keyPressEntries)/sizeof(keyPressEntries[0]);
        for (unsigned i = 0; i < numberOfKeyPressItems; i++)
            keyPressCommandsMap->set(keyPressEntries[i].modifiers << 16 | keyPressEntries[i].charCode, keyPressEntries[i].name);
    }

    unsigned modifiers = 0;
    if (evt->shiftKey())
        modifiers |= s_ShiftKey;
    if (evt->altKey())
        modifiers |= s_AltKey;
    if (evt->ctrlKey())
        modifiers |= s_CtrlKey;
    if(evt->metaKey())
        modifiers |= s_MetaKey;
    
    if (evt->type() == WebCore::eventNames().keydownEvent) {
        int mapKey = modifiers << 16 | evt->keyCode();
        return mapKey ? keyDownCommandsMap->get(mapKey) : 0;
    }

    int mapKey = modifiers << 16 | evt->charCode();
    return mapKey ? keyPressCommandsMap->get(mapKey) : 0;
}

void WebEditorClient::handleKeyboardEvent(WebCore::KeyboardEvent* keyEvent)
{
    ASSERT(keyEvent);    
    ASSERT(keyEvent->target());
    ASSERT(keyEvent->target()->toNode());

    WebCore::Node* node = keyEvent->target()->toNode();
    WebCore::Document* const doc = node->document();
    ASSERT(doc);
    WebCore::Frame* const frame = doc->frame();
    ASSERT(frame);
    
    const WebCore::PlatformKeyboardEvent* const platformKeyEvent = keyEvent->keyEvent();
    if(!platformKeyEvent)
        return;
    
    bool eventHandled = false;

    const char* cmdStr = interpretKeyEvent(keyEvent);
    WebCore::Editor::Command command = frame->editor()->command(cmdStr);

    if (platformKeyEvent->type() == WebCore::PlatformKeyboardEvent::RawKeyDown) {
        // WebKit doesn't have enough information about mode to decide how commands that just insert text if executed via Editor should be treated,
        // so we leave it upon WebCore to either handle them immediately (e.g. Tab that changes focus) or let a keypress event be generated
        // (e.g. Tab that inserts a Tab character, or Enter).
        eventHandled = !command.isTextInsertion() && command.execute( keyEvent );
    }
    else if(command.execute(keyEvent)) {
        eventHandled = true;
    }
    else {
        eventHandled = m_webView->doDefaultKeyPressActionForEditor(frame->editor(), keyEvent);
    }
    if (eventHandled)
        keyEvent->setDefaultHandled();
}

void WebEditorClient::handleInputMethodKeydown(WebCore::KeyboardEvent*)
{
}

void WebEditorClient::textFieldDidBeginEditing(WebCore::Element*)
{
}

void WebEditorClient::textFieldDidEndEditing(WebCore::Element*)
{
}

void WebEditorClient::textDidChangeInTextField(WebCore::Element*)
{
}

bool WebEditorClient::doTextFieldCommandFromEvent(WebCore::Element* elem, WebCore::KeyboardEvent* keyEvt)
{
//	m_webView->interpretKeyEvent( keyEvt );
    return false;
}

void WebEditorClient::textWillBeDeletedInTextField(WebCore::Element*)
{
}

void WebEditorClient::textDidChangeInTextArea(WebCore::Element*)
{
}

void WebEditorClient::ignoreWordInSpellDocument(const WebCore::String&)
{
}

void WebEditorClient::learnWord(const WebCore::String&)
{
}

void WebEditorClient::checkSpellingOfString(const UChar*, int length, int* misspellingLocation, int* misspellingLength)
{
}
    
WebCore::String WebEditorClient::getAutoCorrectSuggestionForMisspelledWord(const WebCore::String& misspelledWord)    
{
    // Safari 4.0.3 integration
    return WebCore::String();
}
    
void WebEditorClient::checkGrammarOfString(const UChar*, int length, WTF::Vector<WebCore::GrammarDetail>&, int* badGrammarLocation, int* badGrammarLength)
{
}

void WebEditorClient::updateSpellingUIWithGrammarString(const WebCore::String&, const WebCore::GrammarDetail& detail)
{
}

void WebEditorClient::updateSpellingUIWithMisspelledWord(const WebCore::String&)
{
}

void WebEditorClient::showSpellingUI(bool show)
{
}

bool WebEditorClient::spellingUIIsShowing()
{
    return false;
}

void WebEditorClient::getGuessesForWord(const WebCore::String&, Vector<WebCore::String>& guesses)
{
}

void WebEditorClient::setInputMethodState(bool enable)
{
    // this is called when a HTML input is focused
    // enable = false when a password input or non-editable HTML content is focused 
    m_webView->webHost()->m_pVTable->setInputMethodState(m_webView->webHost(), enable);
}

bool WebEditorClient::isSelectTrailingWhitespaceEnabled()
{
// RHU TODO "Implement isSelectTrailingWhitespaceEnabled"
    return false;
}

}
