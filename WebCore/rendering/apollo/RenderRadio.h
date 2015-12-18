/*
 * This file is part of the html renderer for KDE.
 *
 * Copyright 2007 Adobe Systems Incorporated. All Rights Reserved.
 * Copyright (C) 2005 Apple Computer
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#ifndef RenderRadio_h
#define RenderRadio_h

#include "RenderBlock.h"

namespace WebCore {

class RenderRadio : public RenderBlock
{
public:
    RenderRadio(Node*);
    virtual void paintObject(PaintInfo&, int tx, int ty);
    virtual bool hasControlClip() const;

    virtual const char* renderName() const { return "RenderRadio"; }
};

}

#endif
