/*
    Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
                  2004, 2005 Rob Buis <buis@kde.org>
    Copyright (C) Research In Motion Limited 2010. All rights reserved.

    Based on khtml code by:
    Copyright (C) 2000-2003 Lars Knoll (knoll@kde.org)
              (C) 2000 Antti Koivisto (koivisto@kde.org)
              (C) 2000-2003 Dirk Mueller (mueller@kde.org)
              (C) 2002-2003 Apple Computer, Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef SVGRenderStyleDefs_h
#define SVGRenderStyleDefs_h

#if ENABLE(SVG)
#include "Color.h"
#include "PlatformString.h"
#include "ShadowData.h"
#include <wtf/OwnPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>

// Helper macros for 'SVGRenderStyle'
#define SVG_RS_DEFINE_ATTRIBUTE(Data, Type, Name, Initial) \
    void set##Type(Data val) { svg_noninherited_flags.f._##Name = val; } \
    Data Name() const { return (Data) svg_noninherited_flags.f._##Name; } \
    static Data initial##Type() { return Initial; }

#define SVG_RS_DEFINE_ATTRIBUTE_INHERITED(Data, Type, Name, Initial) \
    void set##Type(Data val) { svg_inherited_flags._##Name = val; } \
    Data Name() const { return (Data) svg_inherited_flags._##Name; } \
    static Data initial##Type() { return Initial; }

// "Helper" macros for SVG's RenderStyle properties
// FIXME: These are impossible to work with or debug.
#define SVG_RS_DEFINE_ATTRIBUTE_DATAREF(Data, Group, Variable, Type, Name) \
    Data Name() const { return Group->Variable; } \
    void set##Type(Data obj) { SVG_RS_SET_VARIABLE(Group, Variable, obj) }

#define SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL(Data, Group, Variable, Type, Name, Initial) \
    SVG_RS_DEFINE_ATTRIBUTE_DATAREF(Data, Group, Variable, Type, Name) \
    static Data initial##Type() { return Initial; }

#define SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL_REFCOUNTED(Data, Group, Variable, Type, Name, Initial) \
    Data* Name() const { return Group->Variable.get(); } \
    void set##Type(PassRefPtr<Data> obj) { \
        if (!(Group->Variable == obj)) \
            Group.access()->Variable = obj; \
    } \
    static Data* initial##Type() { return Initial; }

#define SVG_RS_DEFINE_ATTRIBUTE_DATAREF_WITH_INITIAL_OWNPTR(Data, Group, Variable, Type, Name, Initial) \
    Data* Name() const { return Group->Variable.get(); } \
    void set##Type(Data* obj) { \
        Group.access()->Variable.set(obj); \
    } \
    static Data* initial##Type() { return Initial; }

#define SVG_RS_SET_VARIABLE(Group, Variable, Value) \
    if (!(Group->Variable == Value)) \
        Group.access()->Variable = Value;

namespace WebCore {

    enum EBaselineShift {
        BS_BASELINE, BS_SUB, BS_SUPER, BS_LENGTH
    };

    enum ETextAnchor {
        TA_START, TA_MIDDLE, TA_END
    };

    enum EColorInterpolation {
        CI_AUTO, CI_SRGB, CI_LINEARRGB
    };

    enum EColorRendering {
        CR_AUTO, CR_OPTIMIZESPEED, CR_OPTIMIZEQUALITY
    };
    
    enum EImageRendering {
        IR_AUTO, IR_OPTIMIZESPEED, IR_OPTIMIZEQUALITY
    };

    enum EShapeRendering {
        SR_AUTO, SR_OPTIMIZESPEED, SR_CRISPEDGES, SR_GEOMETRICPRECISION
    };

    enum EWritingMode {
        WM_LRTB, WM_LR, WM_RLTB, WM_RL, WM_TBRL, WM_TB
    };

    enum EGlyphOrientation {
        GO_0DEG, GO_90DEG, GO_180DEG, GO_270DEG, GO_AUTO
    };

    enum EAlignmentBaseline {
        AB_AUTO, AB_BASELINE, AB_BEFORE_EDGE, AB_TEXT_BEFORE_EDGE,
        AB_MIDDLE, AB_CENTRAL, AB_AFTER_EDGE, AB_TEXT_AFTER_EDGE,
        AB_IDEOGRAPHIC, AB_ALPHABETIC, AB_HANGING, AB_MATHEMATICAL
    };

    enum EDominantBaseline {
        DB_AUTO, DB_USE_SCRIPT, DB_NO_CHANGE, DB_RESET_SIZE,
        DB_IDEOGRAPHIC, DB_ALPHABETIC, DB_HANGING, DB_MATHEMATICAL,
        DB_CENTRAL, DB_MIDDLE, DB_TEXT_AFTER_EDGE, DB_TEXT_BEFORE_EDGE
    };

    class CSSValue;
    class CSSValueList;
    class SVGPaint;

    // Inherited/Non-Inherited Style Datastructures
    class StyleFillData : public RefCounted<StyleFillData> {
    public:
        static PassRefPtr<StyleFillData> create() { return adoptRef(new StyleFillData); }
        PassRefPtr<StyleFillData> copy() const { return adoptRef(new StyleFillData(*this)); }
        
        bool operator==(const StyleFillData&) const;
        bool operator!=(const StyleFillData& other) const
        {
            return !(*this == other);
        }

        float opacity;
        RefPtr<SVGPaint> paint;

    private:
        StyleFillData();
        StyleFillData(const StyleFillData&);
    };

    class StyleStrokeData : public RefCounted<StyleStrokeData> {
    public:
        static PassRefPtr<StyleStrokeData> create() { return adoptRef(new StyleStrokeData); }
        PassRefPtr<StyleStrokeData> copy() const { return adoptRef(new StyleStrokeData(*this)); }

        bool operator==(const StyleStrokeData&) const;
        bool operator!=(const StyleStrokeData& other) const
        {
            return !(*this == other);
        }

        float opacity;
        float miterLimit;

        RefPtr<CSSValue> width;
        RefPtr<CSSValue> dashOffset;

        RefPtr<SVGPaint> paint;
        RefPtr<CSSValueList> dashArray;

    private:        
        StyleStrokeData();
        StyleStrokeData(const StyleStrokeData&);
    };

    class StyleStopData : public RefCounted<StyleStopData> {
    public:
        static PassRefPtr<StyleStopData> create() { return adoptRef(new StyleStopData); }
        PassRefPtr<StyleStopData> copy() const { return adoptRef(new StyleStopData(*this)); }

        bool operator==(const StyleStopData&) const;
        bool operator!=(const StyleStopData& other) const
        {
            return !(*this == other);
        }

        float opacity;
        Color color;

    private:        
        StyleStopData();
        StyleStopData(const StyleStopData&);
    };

    class StyleTextData : public RefCounted<StyleTextData> {
    public:
        static PassRefPtr<StyleTextData> create() { return adoptRef(new StyleTextData); }
        PassRefPtr<StyleTextData> copy() const { return adoptRef(new StyleTextData(*this)); }
        
        bool operator==(const StyleTextData& other) const;
        bool operator!=(const StyleTextData& other) const
        {
            return !(*this == other);
        }

        RefPtr<CSSValue> kerning;

    private:
        StyleTextData();
        StyleTextData(const StyleTextData&);
    };

    // Note: the rule for this class is, *no inheritance* of these props
    class StyleMiscData : public RefCounted<StyleMiscData> {
    public:
        static PassRefPtr<StyleMiscData> create() { return adoptRef(new StyleMiscData); }
        PassRefPtr<StyleMiscData> copy() const { return adoptRef(new StyleMiscData(*this)); }

        bool operator==(const StyleMiscData&) const;
        bool operator!=(const StyleMiscData& other) const
        {
            return !(*this == other);
        }

        Color floodColor;
        float floodOpacity;
        Color lightingColor;

        // non-inherited text stuff lives here not in StyleTextData.
        RefPtr<CSSValue> baselineShiftValue;

    private:
        StyleMiscData();
        StyleMiscData(const StyleMiscData&);
    };

    class StyleShadowSVGData : public RefCounted<StyleShadowSVGData> {
    public:
        static PassRefPtr<StyleShadowSVGData> create() { return adoptRef(new StyleShadowSVGData); }
        PassRefPtr<StyleShadowSVGData> copy() const { return adoptRef(new StyleShadowSVGData(*this)); }

        bool operator==(const StyleShadowSVGData&) const;
        bool operator!=(const StyleShadowSVGData& other) const
        {
            return !(*this == other);
        }

        OwnPtr<ShadowData> shadow;

    private:
        StyleShadowSVGData();
        StyleShadowSVGData(const StyleShadowSVGData&);
    };

    // Non-inherited resources
    class StyleResourceData : public RefCounted<StyleResourceData> {
    public:
        static PassRefPtr<StyleResourceData> create() { return adoptRef(new StyleResourceData); }
        PassRefPtr<StyleResourceData> copy() const { return adoptRef(new StyleResourceData(*this)); }

        bool operator==(const StyleResourceData&) const;
        bool operator!=(const StyleResourceData& other) const
        {
            return !(*this == other);
        }

        String clipper;
        String filter;
        String masker;

    private:
        StyleResourceData();
        StyleResourceData(const StyleResourceData&);
    };

    // Inherited resources
    class StyleInheritedResourceData : public RefCounted<StyleInheritedResourceData> {
    public:
        static PassRefPtr<StyleInheritedResourceData> create() { return adoptRef(new StyleInheritedResourceData); }
        PassRefPtr<StyleInheritedResourceData> copy() const { return adoptRef(new StyleInheritedResourceData(*this)); }

        bool operator==(const StyleInheritedResourceData&) const;
        bool operator!=(const StyleInheritedResourceData& other) const
        {
            return !(*this == other);
        }

        String markerStart;
        String markerMid;
        String markerEnd;

    private:
        StyleInheritedResourceData();
        StyleInheritedResourceData(const StyleInheritedResourceData&);
    };

} // namespace WebCore

#endif // ENABLE(SVG)
#endif // SVGRenderStyleDefs_h
