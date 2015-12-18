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
#include "DragController.h"
#include "DragData.h"
#include <WebKitApollo/WebKit.h>

namespace WebKitApollo { extern WebKitAPIHostFunctions* g_HostFunctions; }

namespace WebCore {

const int DragController::LinkDragBorderInset = -2;
const int DragController::MaxOriginalImageArea = 1500 * 1500;
const int DragController::DragIconRightInset = 7;
const int DragController::DragIconBottomInset = 3;

const float DragController::DragImageAlpha = 0.75f;

bool DragController::isCopyKeyDown()
{
    ASSERT(WebKitApollo::g_HostFunctions);
    ASSERT(WebKitApollo::g_HostFunctions->getCopyKeyState);
    return WebKitApollo::g_HostFunctions->getCopyKeyState();
}

DragOperation DragController::dragOperation(DragData* dragData)
{
    ASSERT(dragData);
    DragOperation draggingSourceOperationMask = dragData->draggingSourceOperationMask();
    if (draggingSourceOperationMask & DragOperationCopy)
        return DragOperationCopy;
    else if (draggingSourceOperationMask & DragOperationMove)
        return DragOperationMove;
    else if (draggingSourceOperationMask & DragOperationLink)
        return DragOperationLink;
    return DragOperationNone;
}

const IntSize &DragController::maxDragImageSize()
{
    static const IntSize maxDragImageSize(400, 400);

    return maxDragImageSize;
}
	
void DragController::cleanupAfterSystemDrag()
{
}	

DragOperation DragController::operationForLoad(DragData*)
{
    return DragOperationNone;
}    

}
