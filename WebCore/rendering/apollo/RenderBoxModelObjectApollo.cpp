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
#include "RenderBoxModelObject.h"
#include "RenderBlock.h"

#if PLATFORM(APOLLO)

namespace WebCore {

int RenderBoxModelObject::relativePositionOffsetY() const
{
    // please take a look at webkit bug 26396
    if (!style()->top().isAuto()) {
        // if the containing block height is auto
        //      and the element is relatively positioned
        // then 
        //      if element top is percentage then
        //          resolve top as auto
        if (!(containingBlock()->style()->height().isAuto() && style()->top().isPercent()))
            return style()->top().calcValue(containingBlock()->availableHeight());
    } else if (!style()->bottom().isAuto()) {
        // if the containing block height is auto
        //      and the element is relatively positioned
        // then 
        //      if element bottom is percentage then
        //          resolve bottom as auto
        if (!(containingBlock()->style()->height().isAuto() && style()->bottom().isPercent()))
            return -style()->bottom().calcValue(containingBlock()->availableHeight());
    }

    return 0;
}

}

#endif //PLATFORM(APOLLO)

