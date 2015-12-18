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
#include <Cursor.h>



namespace WebCore {

Cursor::Cursor(Image*, const IntPoint& /*hotspot*/)
    : m_impl(WebCursorTypePointer)
{
}

Cursor::Cursor(WebCursorType cursorType)
    : m_impl(cursorType)
{
}

Cursor::Cursor(const Cursor& other)
    : m_impl(other.m_impl)
{
}

Cursor::~Cursor()
{
}

Cursor& Cursor::operator=(const Cursor& other)
{
    m_impl = other.m_impl;
	return *this;
}

const Cursor& pointerCursor()
{
    static Cursor const cursor(WebCursorTypePointer);
    return cursor;
}
const Cursor& crossCursor()
{
    static Cursor const cursor(WebCursorTypeCross);
    return cursor;
}
const Cursor& handCursor()
{
    static Cursor const cursor(WebCursorTypeHand);
    return cursor;
}
const Cursor& moveCursor()
{
    static Cursor const cursor(WebCursorTypeMove);
    return cursor;
}
const Cursor& iBeamCursor()
{
    static Cursor const cursor(WebCursorTypeIBeam);
    return cursor;
}
const Cursor& waitCursor()
{
    static Cursor const cursor(WebCursorTypeWait);
    return cursor;
}
const Cursor& helpCursor()
{
    static Cursor const cursor(WebCursorTypeHelp);
    return cursor;
}
const Cursor& eastResizeCursor()
{
    static Cursor const cursor(WebCursorTypeEastResize);
    return cursor;
}
const Cursor& northResizeCursor()
{
    static Cursor const cursor(WebCursorTypeNorthResize);
    return cursor;
}
const Cursor& northEastResizeCursor()
{
    static Cursor const cursor(WebCursorTypeNorthEastResize);
    return cursor;
}
const Cursor& northWestResizeCursor()
{
    static Cursor const cursor(WebCursorTypeNorthWestResize);
    return cursor;
}
const Cursor& southResizeCursor()
{
    static Cursor const cursor(WebCursorTypeSouthResize);
    return cursor;
}
const Cursor& southEastResizeCursor()
{
    static Cursor const cursor(WebCursorTypeSouthEastResize);
    return cursor;
}
const Cursor& southWestResizeCursor()
{
    static Cursor const cursor(WebCursorTypeSouthWestResize);
    return cursor;
}
const Cursor& westResizeCursor()
{
    static Cursor const cursor(WebCursorTypeWestResize);
    return cursor;
}
const Cursor& northSouthResizeCursor()
{
    static Cursor const cursor(WebCursorTypeNorthSouthResize);
    return cursor;
}
const Cursor& eastWestResizeCursor()
{
    static Cursor const cursor(WebCursorTypeEastWestResize);
    return cursor;
}
const Cursor& northEastSouthWestResizeCursor()
{
    static Cursor const cursor(WebCursorTypeNorthEastSouthWestResize);
    return cursor;
}
const Cursor& northWestSouthEastResizeCursor()
{
    static Cursor const cursor(WebCursorTypeNorthWestSouthEastResize);
    return cursor;
}
const Cursor& columnResizeCursor()
{
    static Cursor const cursor(WebCursorTypeColumnResize);
    return cursor;
}
const Cursor& rowResizeCursor()
{
    static Cursor const cursor(WebCursorTypeRowResize);
    return cursor;
}
const Cursor& verticalTextCursor()
{
    static Cursor const cursor(WebCursorTypeVerticalText);
    return cursor;
}
const Cursor& cellCursor()
{
    static Cursor const cursor(WebCursorTypeCell);
    return cursor;
}
const Cursor& contextMenuCursor()
{
    static Cursor const cursor(WebCursorTypeContextMenu);
    return cursor;
}
const Cursor& noDropCursor()
{
    static Cursor const cursor(WebCursorTypeNoDrop);
    return cursor;
}
const Cursor& notAllowedCursor()
{
    static Cursor const cursor(WebCursorTypeNotAllowed);
    return cursor;
}
const Cursor& progressCursor()
{
    static Cursor const cursor(WebCursorTypeProgress);
    return cursor;
}
const Cursor& aliasCursor()
{
    static Cursor const cursor(WebCursorTypeAlias);
    return cursor;
}

const Cursor& zoomInCursor()
{
    static Cursor const cursor(WebCursorTypeZoomIn);
    return cursor;
}

const Cursor& zoomOutCursor()
{
    static Cursor const cursor(WebCursorTypeZoomOut);
    return cursor;
}

const Cursor& copyCursor()
{
    static Cursor const cursor(WebCursorTypeCopy);
    return cursor;
}
const Cursor& noneCursor()
{
    static Cursor const cursor(WebCursorTypeNone);
    return cursor;
}
	
const Cursor& grabCursor()
{
	static Cursor const cursor(WebCursorTypeHand);
	return cursor;
}	
	
const Cursor& grabbingCursor()
{
	static Cursor const cursor(WebCursorTypeHand);
	return cursor;
}

const Cursor& middlePanningCursor()
{
    static Cursor const cursor(WebCursorTypeHand);
    return cursor;
}

const Cursor& eastPanningCursor()
{
    static Cursor const cursor(WebCursorTypeHand);
    return cursor;
}

const Cursor& northPanningCursor()
{
    static Cursor const cursor(WebCursorTypeHand);
    return cursor;
}

const Cursor& northEastPanningCursor()
{
    static Cursor const cursor(WebCursorTypeHand);
    return cursor;
}

const Cursor& northWestPanningCursor()
{
    static Cursor const cursor(WebCursorTypeHand);
    return cursor;
}

const Cursor& southPanningCursor()
{
    static Cursor const cursor(WebCursorTypeHand);
    return cursor;
}

const Cursor& southEastPanningCursor()
{
    static Cursor const cursor(WebCursorTypeHand);
    return cursor;
}

const Cursor& southWestPanningCursor()
{
    static Cursor const cursor(WebCursorTypeHand);
    return cursor;
}

const Cursor& westPanningCursor()
{
    static Cursor const cursor(WebCursorTypeHand);
    return cursor;
}


}
