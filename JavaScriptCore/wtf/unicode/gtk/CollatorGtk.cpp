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


#include "config.h"
#include "unicode/Collator.h"

#if USE(GTK_UNICODE) && !UCONFIG_NO_COLLATION

#include <glib.h>
#if DEBUG
FILE *printencodinglog_file;
#include <sys/time.h>
#define PrintEncodingLog(X...)      \
                {                                                               \
                        if(!printencodinglog_file) printencodinglog_file = fopen("/tmp/enc_logs", "w"); \
                        if(printencodinglog_file)                                \
                        {                                                       \
                                timeval time;                                   \
                                gettimeofday(&time, NULL);                      \
                                int milli_seconds = ((time.tv_sec % 1000) * 1000) + (time.tv_usec / 1000); \
                                fprintf(printencodinglog_file, "%d - ", milli_seconds); \
                                fprintf(printencodinglog_file, X);               \
                                fflush(printencodinglog_file);                   \
                        }                                                       \
            }
#else
#define PrintEncodingLog(X...)
#endif

namespace WTF {

Collator::Collator(const char* locale)
    : m_locale(locale ? strdup(locale) : 0)
    , m_lowerFirst(false)
{
}

Collator::~Collator()
{
    free(m_locale);
}

void Collator::setOrderLowerFirst(bool lowerFirst)
{
    m_lowerFirst = lowerFirst;
}

PassOwnPtr<Collator> Collator::userDefault()
{
    return new Collator(0);
}

static Collator::Result ConvertToCollatorResult(gint gtkResult)
{
    if(gtkResult > 0)
        return Collator::Less;
    else if(gtkResult < 0)
        return Collator::Greater;
    else
        return Collator::Equal;
}


// A default implementation for platforms that lack Unicode-aware collation.
Collator::Result Collator::collate(const UChar* lhs, size_t lhsLength, const UChar* rhs, size_t rhsLength) const
{
    //This is Glib based implementation of Collation functions
    //Glib exposes these for it's collation functions for Utf-8. Using that.
    //

    //Use the locale info too
    //

    if (lhsLength == 0)
    {
        if (rhsLength == 0)
            return Collator::Equal;
        else
            return Collator::Less;
    }
    else if (rhsLength == 0)
            return Collator::Greater;

    GError* error = NULL;
    bool convertedLHS = true;;
    gchar* LeftHandSideUTF8 = g_utf16_to_utf8(lhs, lhsLength, NULL, NULL, &error);
    if(error) 
    {
	ASSERT(0);  
	PrintEncodingLog("Collation: LHS Error : %s\n", error->message); 	
        g_error_free(error);
    }

    gchar *LeftHandSideCaseFoldedUTF8 = NULL;
    if(LeftHandSideUTF8)
    {
    	LeftHandSideCaseFoldedUTF8 = g_utf8_casefold(LeftHandSideUTF8, -1);
    }
    else
    {
        ASSERT(0);
        PrintEncodingLog("Collation: LHS Error : No case folded LHS"); 	
    }




    gchar* RightHandSideUTF8 = g_utf16_to_utf8(rhs, rhsLength, NULL, NULL, &error);
    if(error) 
    {
	ASSERT(0);  
	PrintEncodingLog("Collation: RHS Error : %s\n", error->message); 	
        g_error_free(error);
    }

    gchar* RightHandSideCaseFoldedUTF8 = NULL;
    if(RightHandSideUTF8)
    {
	RightHandSideCaseFoldedUTF8 = g_utf8_casefold(RightHandSideUTF8, -1);
    }
    else
    {
        ASSERT(0);
        PrintEncodingLog("Collation: RHS Error : No case folded RHS"); 	
    }

    if(!LeftHandSideCaseFoldedUTF8)
    {
        ASSERT(0);
        //going to exit anyway so freeing the memory
        if(LeftHandSideUTF8)
            g_free(LeftHandSideUTF8);
        if(RightHandSideUTF8)
            g_free(RightHandSideUTF8);



        if(!RightHandSideCaseFoldedUTF8)
            return Collator::Equal;
        else
        {
            g_free(RightHandSideCaseFoldedUTF8);
            return Collator::Less;
        }
    }
    else
    {
        //LHS but no RHS
        if(!RightHandSideCaseFoldedUTF8)
        {
            //going to exit anyway so freeing the memory
            if(LeftHandSideUTF8)
                g_free(LeftHandSideUTF8);
            if(RightHandSideUTF8)
                g_free(RightHandSideUTF8);

            g_free(LeftHandSideCaseFoldedUTF8);
		    
            ASSERT(0);
            return Collator::Greater;
        }
    } 

    //Reaches here only if both UTF8's and corresponding CaseFolded variants are present;
    ASSERT (LeftHandSideCaseFoldedUTF8 && LeftHandSideUTF8 && RightHandSideCaseFoldedUTF8 && RightHandSideUTF8);

    gint CaseInsensitiveResult = g_utf8_collate(LeftHandSideCaseFoldedUTF8, RightHandSideCaseFoldedUTF8);

    if(CaseInsensitiveResult == 0)
    {
         gint CaseSensitiveOutput = g_utf8_collate(LeftHandSideUTF8, RightHandSideUTF8);
         Collator::Result caseSensitiveResult = ConvertToCollatorResult(CaseSensitiveOutput);

         if(!m_lowerFirst)
             return caseSensitiveResult;

         if (caseSensitiveResult == Collator::Less)
             return Collator::Greater;
         else if (caseSensitiveResult == Collator::Greater)
            return Collator::Less;
         return Collator::Equal;
     }
     return ConvertToCollatorResult(CaseInsensitiveResult);
}

}

#endif
