/*
 * Copyright (C) 2009 Adobe Systems Incorporated.  All rights reserved.
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

#include "TextBreakIterator.h"
#include <WebKitApollo/WebKit.h>

namespace WebKitApollo {
	extern WebKitAPIHostFunctions* g_HostFunctions;
}

namespace WebCore {
	class TextBreakIterator;

	static TextBreakIterator* setUpIterator(bool& createdIterator, TextBreakIterator*& iterator,
		BreakIteratorType type, const UChar* string, int length)
	{
        // newBreakIterator assumes the length > 0
		if(!string || !length)
			return 0;

		if(!createdIterator)
		{
			iterator = reinterpret_cast<TextBreakIterator*>
				(WebKitApollo::g_HostFunctions->newBreakIterator(type, reinterpret_cast<const uint16_t*>(string), length));
			createdIterator = true;
		}

		if (!iterator)
			return 0;

		WebKitApollo::g_HostFunctions->textBreakSetText(reinterpret_cast<void*>(iterator), reinterpret_cast<const uint16_t*>(string), length);

		return iterator;
	}

	TextBreakIterator* characterBreakIterator(const UChar* string, int length)
	{
		static bool createdCharacterBreakIterator = false;
		static TextBreakIterator* staticCharacterBreakIterator;
		return setUpIterator(createdCharacterBreakIterator,
			staticCharacterBreakIterator, CharacterBreakIteratorType, string, length);
	}

	TextBreakIterator* wordBreakIterator(const UChar* string, int length)
	{
		static bool createdWordBreakIterator = false;
		static TextBreakIterator* staticWordBreakIterator;
		return setUpIterator(createdWordBreakIterator,
			staticWordBreakIterator, WordBreakIteratorType, string, length);
	}

	TextBreakIterator* lineBreakIterator(const UChar* string, int length)
	{
		static bool createdLineBreakIterator = false;
		static TextBreakIterator* staticLineBreakIterator;
		return setUpIterator(createdLineBreakIterator,
			staticLineBreakIterator, LineBreakIteratorType, string, length);
	}

	TextBreakIterator* sentenceBreakIterator(const UChar* string, int length)
	{
		static bool createdSentenceBreakIterator = false;
		static TextBreakIterator* staticSentenceBreakIterator;
		return setUpIterator(createdSentenceBreakIterator,
			staticSentenceBreakIterator, SentenceBreakIteratorType, string, length);
	}

    TextBreakIterator* cursorMovementIterator(const UChar* string, int length)
    {
        return characterBreakIterator(string, length);
    }

	int textBreakFirst(TextBreakIterator* bi)
	{
		return WebKitApollo::g_HostFunctions->textBreakFirst(reinterpret_cast<void*>(bi));
	}

	int textBreakNext(TextBreakIterator* bi)
	{
		return WebKitApollo::g_HostFunctions->textBreakNext(reinterpret_cast<void*>(bi));
	}

	int textBreakPreceding(TextBreakIterator* bi, int pos)
	{
		return WebKitApollo::g_HostFunctions->textBreakPreceding(reinterpret_cast<void*>(bi), pos);
	}

	int textBreakFollowing(TextBreakIterator* bi, int pos)
	{
		return WebKitApollo::g_HostFunctions->textBreakFollowing(reinterpret_cast<void*>(bi), pos);
	}

	int textBreakCurrent(TextBreakIterator* bi)
	{
		return WebKitApollo::g_HostFunctions->textBreakCurrent(reinterpret_cast<void*>(bi));
	}

	bool isTextBreak(TextBreakIterator* bi, int pos)
	{
		return WebKitApollo::g_HostFunctions->isTextBreak(reinterpret_cast<void*>(bi), pos);
	}

}
