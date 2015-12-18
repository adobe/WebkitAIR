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
#include <BitmapImage.h>
#include <algorithm>
#include <cstring>

#include <DerivedSources/res/checkBoxStates.png.h>
#include <DerivedSources/res/deleteButton.png.h>
#include <DerivedSources/res/deleteButtonPressed.png.h>
#include <DerivedSources/res/htmlButtonStates.png.h>
#include <DerivedSources/res/nullPlugin.png.h>
#include <DerivedSources/res/missingImage.png.h>
#include <DerivedSources/res/textAreaResizeCorner.png.h>
#include <DerivedSources/res/popupStates.png.h>
#include <DerivedSources/res/radioButtonStates.png.h>
#include <DerivedSources/res/scrollBarStatesHorz.png.h>
#include <DerivedSources/res/scrollBarStatesVert.png.h>
#include <DerivedSources/res/focusRing.png.h>

#include "SharedBuffer.h"

namespace WebCoreResource {
struct ResourceTableEntry
{
    const char* const name;
    const unsigned char* const bytes;
    const unsigned long size;
};

static const ResourceTableEntry resourceTable[] = {
	{ "checkBoxStates", checkBoxStates, checkBoxStatesSize },
    { "deleteButton", deleteButton, deleteButtonSize },
    { "deleteButtonPressed", deleteButtonPressed, deleteButtonPressedSize },
    { "focusRing", focusRing, focusRingSize },
    { "htmlButtonStates", htmlButtonStates, htmlButtonStatesSize },
    { "missingImage", missingImage, missingImageSize },
    { "nullPlugin", nullPlugin, nullPluginSize },
    { "popupStates", popupStates, popupStatesSize },
    { "radioButtonStates", radioButtonStates, radioButtonStatesSize },
    { "scrollBarStatesHorz", scrollBarStatesHorz, scrollBarStatesHorzSize },
    { "scrollBarStatesVert", scrollBarStatesVert, scrollBarStatesVertSize },
    { "textAreaResizeCorner", textAreaResizeCorner, textAreaResizeCornerSize }
};

static const ResourceTableEntry* const resourceTableStart = resourceTable;
static const ResourceTableEntry* const resourceTableEnd = resourceTableStart + (sizeof(resourceTable) / sizeof(ResourceTableEntry));

namespace
{

class CompareResourceTableEntries
{
public:
    
    inline bool operator()(const ResourceTableEntry& first, const ResourceTableEntry& second)
    {
        int const cmpRet = std::strcmp(first.name, second.name);
        return cmpRet < 0;
    }
    
};

}

static const ResourceTableEntry* findResourceEntry(const char* const name)
{
    const ResourceTableEntry* result = NULL;
    CompareResourceTableEntries compareFunctor;
    ResourceTableEntry const entryToFind = { name, NULL, 0 };
    result = std::lower_bound(resourceTableStart, resourceTableEnd, entryToFind, compareFunctor);
    ASSERT(result >= resourceTableStart);
    if ((result >= resourceTableEnd) || (std::strcmp(result->name, name) != 0))
        result = 0;
    return result;
}

}


namespace WebCore
{
void BitmapImage::initPlatformData()
{
}

void BitmapImage::invalidatePlatformData()
{
    for (unsigned i = 0; i < m_frames.size(); ++i) {
        if (m_frames[i].m_frame) {
            //mihnea integrate -> no need to clear the metadata
            m_frames[i].clear(false);
        }
    }
}

PassRefPtr<Image> Image::loadPlatformResource(char const* resourceName)
{
    const WebCoreResource::ResourceTableEntry* const resourceEntry = WebCoreResource::findResourceEntry(resourceName);
    if (resourceEntry) {
        RefPtr<BitmapImage> const image = BitmapImage::create();
        WTF::RefPtr<SharedBuffer> imageBuffer = SharedBuffer::create(reinterpret_cast<const char*>(resourceEntry->bytes), resourceEntry->size);
        image->setData(imageBuffer.release(), true);
        return image;
    }
    else
        return 0;
}

}

