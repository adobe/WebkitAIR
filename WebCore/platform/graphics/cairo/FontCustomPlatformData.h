/*
 * Copyright (C) 2008 Alp Toker <alp@atoker.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef FontCustomPlatformData_h
#define FontCustomPlatformData_h

#include "FontRenderingMode.h"
#include <wtf/Noncopyable.h>

#if PLATFORM(APOLLO)
#include "SharedBuffer.h"
#endif

typedef struct _cairo_font_face cairo_font_face_t;

namespace WebCore {

class FontPlatformData;
class SharedBuffer;

struct FontCustomPlatformData : Noncopyable {
    FontCustomPlatformData(cairo_font_face_t* fontFace)
    : m_fontFace(fontFace)
#if PLATFORM(APOLLO)
      , m_buffer(0)
#endif
    {}

#if PLATFORM(APOLLO)
    FontCustomPlatformData(cairo_font_face_t* fontFace, SharedBuffer* buffer)
    : m_fontFace(fontFace),
      m_buffer(buffer)
    {}
#endif

    ~FontCustomPlatformData();

    FontPlatformData fontPlatformData(int size, bool bold, bool italic, FontRenderingMode = NormalRenderingMode);

    cairo_font_face_t* m_fontFace;
    
#if PLATFORM(APOLLO)
private:
    SharedBuffer* m_buffer;
#endif
};

FontCustomPlatformData* createFontCustomPlatformData(SharedBuffer* buffer);

}

#endif
