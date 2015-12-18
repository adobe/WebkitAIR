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
#include <ResourceResponse.h>
#include <PluginInfoStore.h>
#include <WebKitApollo/WebKit.h>
#include "MIMETypeRegistry.h"
#include "runtime/DateInstance.h"
#include "wtf/DateMath.h"
#include "HTTPParsers.h"

namespace WebCore {

namespace {
    static const char contentTypeHeaderName[] = "Content-Type";
    static const char contentLengthHeaderName[] = "Content-Length";
    static const char contentTypeCharsetParameterName[] = "charset";

    static const char cacheControlHeaderName[] = "Cache-Control";
    static const char dateHeaderName[] = "Date";
    static const char ageHeaderName[] = "Age";
    static const char pragmaHeaderName[] = "Pragma";
    static const char expiresHeaderName[] = "Expires";
	static const char noCacheName[]	= "no-cache";
	static const char maxAgeName[] = "max-age";
}

namespace {

    double getExpiredTime() {
        time_t now = time(0);
        // Subtract 10 years worth of seconds from the date/time
        return (now - (double)315360000L);
    }

    //
    // The following code is stolen from Acrobat's EFS plug-in
    //

    typedef bool (*EnumFieldValuesCB) ( const char* Value, const size_t Len, void* pClientData );

    /* Struct used to pass info through CacheControlFieldEnumCB */
    typedef struct _CacheControlEnumRec {
	    bool	NoCacheFound;
	    bool	MaxAgeFound;
	    int64_t	MaxAge;	
    } CacheControlEnumRec;

    /* Internal helper function:
       Given a string and its length, call the passed callback if the string (sans leading whitespace)
       is non-empty.  String and Len passed to callback do not include leading whitespace.
       If IgnoreTrailingWS is true, Len passed to callback is trimmed to ignore trailing whitespace. */
    bool ProcessSection( const char* Section, size_t SectionLen, bool IgnoreTrailingWS, 
						          EnumFieldValuesCB CallBackFunc, void* pClientData )
    {
	    ASSERT( Section != NULL );
	    ASSERT( CallBackFunc != NULL );

	    /* Scan past any leading whitespace in the section */
	    size_t NumLeadingWS;
	    for ( NumLeadingWS = 0; ( NumLeadingWS < SectionLen ) && isSpaceOrNewline( Section[ NumLeadingWS ] ); NumLeadingWS++ )
		    ;

	    /* Keep enumerating if subsection empty or just contained whitespace */
	    if ( NumLeadingWS >= SectionLen )
		    return true;
	    else {

		    size_t TrimmedSectionLen = ( SectionLen - NumLeadingWS ); 
		    ASSERT( TrimmedSectionLen > 0 );	/* We do not get here if entirely whitespace */


		    /* Does the caller want to ignore trailing whitespace? */
		    if ( IgnoreTrailingWS ) {
    		
			    /* Ignore any trailing whitespace by shortening the length of the string which 
			       we're passing to the callback.  Because TrimmedSectionLen is unsigned (size_t), it is important
			       that we use >0 rather than >= 0.  This is fine, because it should only hit 0 if the entire section was
			       whitespace.  If this was the case, we would not get here. */
			    ASSERT( !isSpaceOrNewline( Section[ NumLeadingWS ] ) );
			    for ( ; ( TrimmedSectionLen > 0 ) && isSpaceOrNewline( Section[ NumLeadingWS + TrimmedSectionLen - 1 ] ); 
				    TrimmedSectionLen-- )
				    ;
		    }

		    /* Call the callback with the subsection */
		    return CallBackFunc( Section + NumLeadingWS, TrimmedSectionLen, pClientData );
	    }
    }

    bool EnumCommaSeparatedFieldValue( const char* FieldValue, unsigned int FieldLen, bool IgnoreTrailingWS, 
										        EnumFieldValuesCB CallBackFunc, void* pClientData )
    {
	    size_t NextSectionStartIndex = 0;
	    bool InQuotedSection = false;
	    bool RetVal = true;

	    ASSERT( FieldValue != NULL );
 	    ASSERT( CallBackFunc != NULL );

	    /* See HTTP 1.1 spec section 2.1 which describes the #rule for the syntax
	       we're following */

	    /* Look for each comma-delimited subsection. */
	    for ( size_t i = 0; i < FieldLen; i++ ) {
    		
		    /* Look for the next comma or quotation mark (").  We need to find quotation marks
		       because commas inside of them are NOT considered to be sub-section delimiters.  We
		       need to ignore escaped quotation marks which are part of an escape sequence 
		       (quoted-pair in HTTP parlance) inside of a quoted section (ie: "bleh \" blah") */
    		
		    /* If the current character is a comma, and we're not inside a quoted section, we've
		       potentially found the end of a subsection (unless it is empty or only has whitespace
		       as per HTTP 1.1 section 2.1 */
		    if ( ( FieldValue[ i ] == ',' ) && ( !InQuotedSection ) ) {

			    /* Call helper function.  Calls the callback routine if the section isn't empty or
			       all whitespace.  Also handles trailing whitespace trimming if desired by client.  
			       If callback returned false, stop enumerating */
			    if ( !ProcessSection( &( FieldValue[ NextSectionStartIndex ] ), 
				    ( i - NextSectionStartIndex ), IgnoreTrailingWS, CallBackFunc, pClientData ) )
				    return false;

			    /* Reset the start of the next section to begin right after the comma */
			    NextSectionStartIndex = i + 1;
		    }
		    /* Otherwise, if the current character is a quotation mark: */
		    else if ( FieldValue[ i ] == '"' ) {
    			
			    /* Are we in a quoted section already? */
			    if ( InQuotedSection ) {
    			
				    /* If the quote isn't preceded by a slash (if it is, we ignore it because
				       it is being escaped), then the quoted section is ending */
				    if ( ( i > 0 ) && ( FieldValue[ i - 1 ] != '\\' ) )
					    InQuotedSection = false;
			    }
			    else {
				    /* We're not yet in a quoted section, so this quotation mark begins 
				       a quoted section */
				    InQuotedSection = true;
			    }
		    }
	    }

	    /* Process last section if there is one (stuff after the last comma).  The only way there 
	       isn't one is if the field we're parsing ends in a comma. */
	    if ( NextSectionStartIndex < FieldLen ) {

		    /* Call helper function.  Calls the callback routine if the section isn't empty or
		       all whitespace.  Also handles trailing whitespace trimming if desired by client */
		    RetVal = ProcessSection( &( FieldValue[ NextSectionStartIndex ] ), 
			    ( FieldLen - NextSectionStartIndex ), IgnoreTrailingWS, CallBackFunc, pClientData );
	    }

	    return RetVal;
    }

    // Called for each pragma field value section
    bool PragmaFieldEnumCB( const char* Value, const size_t Len, void* pClientData )
    {
	    ASSERT( (void*)Value != NULL );
 	    ASSERT( Len > 0 );
	    ASSERT( pClientData != NULL );

	    /* Is it a no-cache directive? We have to be careful with the compare, because Value doesn't 
	       have a trailing NULL */
	    if ( 0 == strncasecmp( Value, noCacheName, Len ) ) {

		    /* We found a no-cache directive, so set the ASBool which pClientData points to to true */
		    *( (bool*)pClientData ) = true;

		    /* Stop enumerating */
		    return false;
	    }

	    /* Keep enumerating */
	    return true;
    }

    /* Called for each Cache-Control field value section */
    bool CacheControlFieldEnumCB( const char* Value, const size_t Len, void* pClientData )
    {
	    CacheControlEnumRec* pCCER = (CacheControlEnumRec*)pClientData;

	    ASSERT( (void*)Value != NULL );
 	    ASSERT( Len > 0 );
	    ASSERT( pClientData != NULL );

	    /* Is it a no-cache directive? We have to be careful with the compare, because Value doesn't 
	       have a trailing NULL. */
	    if ( 0 == strncasecmp( Value, noCacheName, Len ) ) {

		    /* We found a no-cache directive.  We ignore the fact that HTTP 1.1 says that the 
		       no-cache directive can actually single out fields not to cache (14.9.1) and treat
		       all no-cache directives the same way. */

		    pCCER->NoCacheFound = true;

		    /* Stop enumerating, we now know all that we need to. */
		    return false;
	    }
	    /* Otherwise, is it a max-age directive?  We have to be careful with the compare, because Value 
	       doesn't have a trailing NULL. */ 
	    else if ( 0 == strncasecmp( Value, maxAgeName, sizeof( maxAgeName ) - 1 ) ) {

		    /* The value string definitely begins with "max-age", though it could begin
		       with "max-age-of-cheese" for all we know at this point. */
    	
		    /* Skip whitespace after "max-age" */
		    size_t i = sizeof( maxAgeName ) - 1;
		    for ( ; ( i < Len ) && isSpaceOrNewline( Value[ i ] ); i++ ) 
			    ;	

		    /* Is the first non-whitespace character after "max-age" an equal sign? */
		    if ( ( i < Len ) && ( Value[ i ] == '=' ) ) {
    	
			    /* Skip whitespace after the equal sign */
			    for ( i++; ( i < Len ) && isSpaceOrNewline( Value[ i ] ); i++ ) 
				    ;	
    		
			    /* Are there any non-whitespace characters left? */
			    if ( i < Len ) {
				    /* Try to convert the remaining characters to a long. */
                    // toInt64 returns 0 on error
                    pCCER->MaxAge = String(Value + i, Len - i).toInt64(&pCCER->MaxAgeFound);
			    }
		    }
    					
		    /* Don't stop enumerating, since a later no-cache directive could override.  This also
		       means that a later max-age directive (if there are more than one) will override
		       an earlier one. */
	    }
       
	    /* Keep enumerating */
	    return true;
    }
}

ResourceResponse::ResourceResponse(WebResourceResponse* pWebResourceResponse)
{
    set(pWebResourceResponse);
}

void ResourceResponse::set(WebResourceResponse* pWebResourceResponse)
{
    m_expectedContentLength = 0;
    m_textEncodingName = String();
    m_suggestedFilename = String();
    m_httpStatusText = String();
    m_httpHeaderFields.clear();
    m_lastModifiedDate = 0;
    m_isNull = false;
    
    ASSERT(pWebResourceResponse);
    ASSERT(pWebResourceResponse->m_pVTable);
    ASSERT(pWebResourceResponse->m_pVTable->getURL);
    ASSERT(pWebResourceResponse->m_pVTable->getStatusCode);
    ASSERT(pWebResourceResponse->m_pVTable->getHeaders);
    
    unsigned long numURLBytes = 0;
    const unsigned char* const pURLBytes = pWebResourceResponse->m_pVTable->getURL(pWebResourceResponse, &numURLBytes);
    ASSERT(pURLBytes);
    ASSERT(numURLBytes > 0);
    
    String const urlStr(reinterpret_cast<const char*>(pURLBytes), numURLBytes);
    m_url = KURL(ParsedURLString, urlStr);    
    
    m_httpStatusCode = pWebResourceResponse->m_pVTable->getStatusCode(pWebResourceResponse);

	bool PragmaNoCacheFound = false;
    int64_t	Age = 0;
    double Date = JSC::NaN;
    double Expires = JSC::NaN;

    CacheControlEnumRec	TheCCERec;
	TheCCERec.NoCacheFound	= false;
	TheCCERec.MaxAgeFound	= false;

    unsigned long httpNumResponseHeaders;
    const WebResourceResponseHeader* const pResponseHeaders = pWebResourceResponse->m_pVTable->getHeaders(pWebResourceResponse, &httpNumResponseHeaders);
    for(unsigned int i = 0; i < httpNumResponseHeaders; ++i) {
        const WebResourceResponseHeader* const pCurrHeader = pResponseHeaders + i;
        ASSERT(sizeof(UChar) == sizeof(uint16_t));
        WebCore::String const name(reinterpret_cast<const UChar *>(pCurrHeader->m_pUTF16HeaderName), pCurrHeader->m_numHeaderNameCodeUnits);
        WebCore::String const value(reinterpret_cast<const UChar *>(pCurrHeader->m_pUTF16HeaderValue), pCurrHeader->m_numHeaderValueCodeUnits);
        bool suppressHeader = false;

        if (equalIgnoringCase(contentTypeHeaderName, name)) {
            if (value.length() > 0) {
                Vector<WebCore::String> parameters;
                value.split(UChar(';'), parameters);
                ASSERT(parameters.size() > 0);
                
                m_mimeType = parameters[0];
                if (parameters.size() > 1) {
                    // Now we should check for a charset specification.
                    const bool caseSensitive = false;
                    Vector<WebCore::String>::iterator curIter(parameters.begin());
                    Vector<WebCore::String>::iterator endIter(parameters.end());
                    ++curIter; // skip over the first item
                    while(curIter != endIter) {
                        WebCore::String item = (*curIter).stripWhiteSpace();
                        int equalSignIndex = item.find( UChar( '=' ) );
                        if (equalSignIndex > 0 && item.startsWith(contentTypeCharsetParameterName, caseSensitive)) {
                            WebCore::String textEncodingName = item.substring(equalSignIndex + 1).stripWhiteSpace();
                            setTextEncodingName( textEncodingName );
                            break;
                        }
                        ++curIter;
                    }                    
                }
            }
            else {
                suppressHeader = true;
            }
        }
        else if(equalIgnoringCase(contentLengthHeaderName, name)) {
            m_expectedContentLength = value.toInt64();
        }
        else if(equalIgnoringCase(cacheControlHeaderName, name)) {
            CString valueStr = value.latin1();

            EnumCommaSeparatedFieldValue(valueStr.data(), valueStr.length(), true, CacheControlFieldEnumCB, &TheCCERec);
        }
        else if(equalIgnoringCase(dateHeaderName, name)) {
            Date = WebCore::parseDate(value.stripWhiteSpace()) / 1000;
        }
        else if(equalIgnoringCase(ageHeaderName, name)) {
            // toInt64 returns 0 on error
            Age = value.stripWhiteSpace().toInt64();
        }
        else if(equalIgnoringCase(pragmaHeaderName, name)) {
            CString valueStr = value.latin1();

            EnumCommaSeparatedFieldValue(valueStr.data(), valueStr.length(), true, PragmaFieldEnumCB, &PragmaNoCacheFound);
        }
        else if(equalIgnoringCase(expiresHeaderName, name)) {
            Expires = parseDate(value.stripWhiteSpace()) / 1000;
        }

        if (!suppressHeader)
            setHTTPHeaderField(name, value);
    }

    // set the expires value for the resource
    double expires = 0.0;

    if( TheCCERec.NoCacheFound || PragmaNoCacheFound ) {
        expires = getExpiredTime();
    }
    else if ( TheCCERec.MaxAgeFound ) {
        double ResponseTime = time(0); // we assume now is the current time
        double ResponseAge = ResponseTime - Date;
        
        if( isnan(ResponseAge) || ResponseAge < 0 )
            ResponseAge = 0;

        if( Age > ResponseAge )
            ResponseAge = Age;

        // APOLLO TODO: here we would add the delay between the request and the response to ResponseAge
        expires = ResponseTime + (TheCCERec.MaxAge - ResponseAge);
    }
    else if ( !isnan(Expires) ) {
        expires = Expires;
    }

    if( expires == 0) {
        expires = getExpiredTime();
    }

    setExpires(expires);
    
    if ((m_mimeType.length() == 0)) {
        // If we don't have a mime-type sniff for one using the extension of the last component of the path.
        m_mimeType = WebCore::MIMETypeRegistry::getMIMETypeForPath(m_url.path());
    }
}

}
