/*
 * Copyright (C) 2011 Adobe Systems Incorporated.  All rights reserved.
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
#include "Console.h"

#if PLATFORM(APOLLO)

#include "FrameLoaderClient.h"
#include <FrameLoaderClientApollo.h>
#include "KURL.h"
#include "Frame.h"
#include "wtf/UnusedParam.h"

namespace WebCore {

static String printMessageSourceAndLevelPrefixToString(MessageSource source, MessageLevel level)
{
    const char* sourceString;
    switch (source) {
        case HTMLMessageSource:
            sourceString = "HTML";
            break;
        case XMLMessageSource:
            sourceString = "XML";
            break;
        case JSMessageSource:
            sourceString = "JS";
            break;
        case CSSMessageSource:
            sourceString = "CSS";
            break;
        default:
            ASSERT_NOT_REACHED();
            // Fall thru.
        case OtherMessageSource:
            sourceString = "OTHER";
            break;
    }

    const char* levelString;
    switch (level) {
        case TipMessageLevel:
            levelString = "TIP";
            break;
        default:
            ASSERT_NOT_REACHED();
            // Fall thru.
        case LogMessageLevel:
            levelString = "LOG";
            break;
        case WarningMessageLevel:
            levelString = "WARN";
            break;
        case ErrorMessageLevel:
            levelString = "ERROR";
            break;
    }

	return String::format("%s %s:", sourceString, levelString);
}

static String printSourceURLAndLineToString(const String& sourceURL, unsigned lineNumber)
{
    String result;
	
    if (!sourceURL.isEmpty()) {
        result.append ( sourceURL );
        result.append ( ':' );
        if (lineNumber > 0)
        {
            result.append ( String::number( lineNumber ) );
        }
        result.append ( ' ' );
    }
	
    return result;
}

static String printToString(MessageSource source, MessageLevel level, const String& message, const String& sourceURL, unsigned lineNumber)
{
    String result;
    result.append ( printSourceURLAndLineToString(sourceURL, lineNumber) );
    result.append ( printMessageSourceAndLevelPrefixToString(source, level) );

    result.append ( ' ' );
    result.append ( message );

    return result;
}

void Console::addMessage(MessageSource source, MessageType type, MessageLevel level, const String& message, unsigned lineNumber, const String& sourceURL)
{
    UNUSED_PARAM(type);
    // javascript exceptions/errors messages are sent via HTMLLoader.uncaughtException event
    if (source == JSMessageSource)
        return;

    String mappedSourceURL = sourceURL;
    if (!(mappedSourceURL.isNull() || mappedSourceURL.isEmpty()) && mappedSourceURL!="about:blank") {
        KURL sourceURL(ParsedURLString, mappedSourceURL);
        KURL mappedURL;
        // also trace messages for remote sandbox files
        if(FrameLoaderClientApollo::mapFrameUrl(m_frame, sourceURL, &mappedURL)){
            mappedSourceURL = mappedURL.string();
        }
        // the mappedURL will contain the initial url when the content is not from a 
        // remote sandbox iframe
        if (!(mappedURL.protocolIs("app")||mappedURL.protocolIs("file"))) {
            // we don't want to print messages from remote sites
            return;
        }
    }

    String msg = printToString(source, level, message, mappedSourceURL, lineNumber);
    ASSERT(m_frame->loader());
    if (FrameLoaderClient *loaderClient = m_frame->loader()->client())
        loaderClient->logWarning(msg);
}

void Console::addMessage(MessageType type, MessageLevel level, ScriptCallStack* callStack, bool acceptNoArguments)
{
    UNUSED_PARAM(type);
    UNUSED_PARAM(level);
    UNUSED_PARAM(callStack);
    UNUSED_PARAM(acceptNoArguments);
    // javascript exceptions/errors messages are sent via HTMLLoader.uncaughtException event
    return;
}

#if ENABLE(JAVASCRIPT_DEBUGGER)

void Console::profile(const String& title, ScriptCallStack* callStack)
{
    (void) title;
    (void) callStack;
}

void Console::profileEnd(const String& title, ScriptCallStack* callStack)
{
    (void) title;
    (void) callStack;
}

#endif

} //namespace WebCore

#endif

