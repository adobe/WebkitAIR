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
#include "Icon.h"
#include "PlatformString.h"


namespace WebCore {

// apollo - compile - integrate - 58803
// olaru:
// Icon::createIconForFile was removed from the header. Its behavior was put directly into
// createIconForFiles.
// Commenting out createIconForFile
// Copying functionality from IconMac.cpp to createIconForFiles, commenting out the bits
// that were similarly commented in our createIconForFile 
/*
PassRefPtr<Icon> Icon::createIconForFile(const String& filename)
{
    // Don't pass relative filenames -- we don't want a result that depends on the current directory.
    // Need 0U here to disambiguate String::operator[] from operator(NSString*, int)[]
    if (filename.isEmpty() || filename[0U] != '/')
        return 0;
	
    //NSImage* image = [[NSWorkspace sharedWorkspace] iconForFile:filename];
    //if (!image)
    //    return 0;
	//
    //return adoptRef(new Icon(image));
	return 0;
}
*/

PassRefPtr<Icon> Icon::createIconForFiles(const Vector<String>& filenames)
{
    if (filenames.isEmpty())
        return 0;
    
    bool useIconFromFirstFile;
#ifdef BUILDING_ON_TIGER
    // FIXME: find a better image for multiple files to use on Tiger.
    useIconFromFirstFile = true;
#else
    useIconFromFirstFile = filenames.size() == 1;
#endif
    if (useIconFromFirstFile) {
        // Don't pass relative filenames -- we don't want a result that depends on the current directory.
        // Need 0U here to disambiguate String::operator[] from operator(NSString*, int)[]
        if (filenames[0].isEmpty() || filenames[0][0U] != '/')
            return 0;
        
        //NSImage* image = [[NSWorkspace sharedWorkspace] iconForFile:filenames[0]];
        //if (!image)
        //    return 0;
        
        //return adoptRef(new Icon(image));
        return 0;
    }
#ifdef BUILDING_ON_TIGER
    return 0;
#else
    //NSImage* image = [NSImage imageNamed:NSImageNameMultipleDocuments];
    //if (!image)
    //    return 0;
    
    //return adoptRef(new Icon(image));
    return 0;
#endif
}

}
