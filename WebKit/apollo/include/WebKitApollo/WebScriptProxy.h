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

/**
	This header file defines the script variant interface that various script engines can implement to bridge object
	references into WebKitApollo.

	An implementation of any of the functions defined in this file MUST never throw a C++ exception out of the function being implemented.
	An implementation of any of the functions defined in this file MUST never call longjmp such that the longjmp would result in the
	exiting of the function's scopy.
	Functions defined in this file MUST ALWAYS return in the normal way.  The functions that can fail either use a return code or have
	parameter passed in that can be filled with error information.

	** NOTHROW **


	Dead objects are handled by having the GetProperty, SetProperty, and Invoke functions
	set the type member of the WebScriptProxyInvokeGetSetResult to
	WebScriptProxyVariantInvokeGetSetResultObjectDead when any of those
	functions is called on a variant that references an object that is dead.

*/

#ifndef WebScriptProxy_h
#define WebScriptProxy_h

#include <WebKitApollo/WebConfig.h>
#include <stddef.h>

enum WebScriptProxyVariantType {
	WebScriptProxyVariantTypeFirst = 0,
	WebScriptProxyVariantTypeNumber = 0,
	WebScriptProxyVariantTypeBoolean = 1,
	WebScriptProxyVariantTypeUndefined = 2,
	WebScriptProxyVariantTypeNull = 3,
	WebScriptProxyVariantTypeString = 4,
	WebScriptProxyVariantTypeObjectRef = 5,

	/* *** NEW TYPES GO ABOVE THIS COMMENT *** */
	WebScriptProxyVariantTypeLast
};

struct WebScriptProxyVariant;
struct WebScriptProxyInvokeGetSetResult;
struct WebScriptProxyPropertyEnumClient;


/**
	Increments reference count of variant by one.

	Implementation may not re-enter caller - must be a simple operation.

	@param pVariant The variant whose reference count will be incremented. pVariant may not be NULL.
 */
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantAcquireFunction )( const struct WebScriptProxyVariant* pVariant );

/**
	Decrements reference count of variant by one.  If reference count goes to zero any resources associated with the variant may be freed.

	Implementation may not re-enter caller - must be a simple operation.

	@param pVariant The variant whose reference count will be decremented. pVariant may not be NULL.
 */
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantReleaseFunction )( const struct WebScriptProxyVariant* pVariant );

/**
	Accessor for getting the type of a variant. The type is immutable for a given variant instance.

	Implementation may not re-enter caller - must be a simple operation.

	@param pVariant The variant whose type is to be returned. pVariant may not be NULL.

	@return The type of pVariant.
 */
typedef WEBKIT_APOLLO_PROTO1 WebScriptProxyVariantType ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantGetTypeFunction )( const struct WebScriptProxyVariant* pVariant );

/**
	If the type of a variant is WebScriptProxyVariantBoolean, then this function may be used to extract the boolean value from the variant.
	This function may be called if and only if the GetType function above returned WebScriptProxyVariantTypeBoolean.  If the variant
	is not of type WebScriptProxyVariantTypeBoolean, then the implementation must ASSERT in the debug build, and behavior is undefined in the
	release build.

	Implementation may not re-enter caller - must be a simple operation.

	@param pVariant The variant whose boolean value is to be extracted. pVariant may not be NULL.

	@return 0 if the variant has a value of false, 1 if the variant has a value of true.
*/
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantGetBoolFunction )( const struct WebScriptProxyVariant* pVariant );
// xxx - **TODO - should comment whether bool value is invariant for a given variant. I *think* it is, except that if variant goes dead it can change


/**
	If the type of a variant is WebScriptProxyVariantNumber, then this function may be used to extract the number value from the variant.
	This function may be called if and only if the GetType function above returned WebScriptProxyVariantTypeNumber.  If the variant
	is not of type WebScriptProxyVariantTypeNumber, then the implementation must ASSERT in the debug build, and behavior is undefined in the
	release build.

	Implementation may not re-enter caller - must be a simple operation.

	@param pVariant The variant whose number value is to be extracted. pVariant may not be NULL.

	@return The number value of the variant.
*/
typedef WEBKIT_APOLLO_PROTO1 double ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantGetNumberFunction )( const struct WebScriptProxyVariant* pVariant );
// xxx - **TODO - should comment whether number value is invariant for a given variant. I *think* it is, except that if variant goes dead it can change


/**
	If the type of a variant is WebScriptProxyVariantTypeString, then this function may be used to extract the number of code units ( 16-bit words )
	needed to store the string value in the UTF-16 encoding.  Use this function to determine the size of the buffer you need to allocate
	before calling the GetStringUTF16 method below.  The number of code units returned by this function will not include space needed for
	a null terminator.
	This function may be called if and only if the GetType function above returned WebScriptProxyVariantTypeString.  If the variant
	is not of type WebScriptProxyVariantTypeString, then the implementation must ASSERT in the debug build, and behavior is undefined in the
	release build.

	Implementation may not re-enter caller - must be a simple operation.

	@param pVariant The variant whose string length is to be extracted. pVariant may not be NULL.

	@return The number code units needed to store a copy of the string value of the variant. Note that for an empty string, 0 is returned.
*/
typedef WEBKIT_APOLLO_PROTO1 unsigned long ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantGetStringUTF16LengthFunction )( const struct WebScriptProxyVariant* pVariant );
// xxx - **TODO - should comment whether string value is invariant for a given variant. I *think* it is, except that if variant goes dead it can change


/**
	If the type of a variant is WebScriptProxyVariantTypeString, then this function may be used to copy the string value of the specified
	variant to the specified buffer in the UTF-16 encoding.  The copied string will NOT be null terminated.
	This function may be called if and only if the GetType function above returned WebScriptProxyVariantTypeString. If the variant
	is not of type WebScriptProxyVariantTypeString, then the implementation must ASSERT in the debug build, and behavior is undefined in the
	release build.

	Implementation may not re-enter caller - must be a simple operation.

	@param pVariant The variant whose string value is to be copied. pVariant may not be NULL.
	@param pDest The buffer the string value of the variant should be copied to. May not be NULL.
*/
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantGetStringUTF16Function )( const struct WebScriptProxyVariant* pVariant
                                                                                                       , uint16_t* pDest );
// xxx - **TODO - should comment whether string value is invariant for a given variant. I *think* it is, except that if variant goes dead it can change


/**
	Determines whether or not the object referenced by a variant has a specified property.  This function may be called if and only if the
	GetType function above returned WebScriptProxyVariantTypeObjectRef.  If the variant is not of type
	WebScriptProxyVariantTypeObjectRef, then the implementation must ASSERT in the debug build, and behavior is undefined in the release build.
	For non-enumerable properties, this function must still return 1.

	Caller should NOT assume that it will not be re-entered from this function.

	@param pVariant The variant whose referenced object is to be queried for the specified property. pVariant may not be NULL.
	@param pUTF16PropertyName The name of the property encoded in UTF-16.  The name does not have to be null terminated.
	@param utf16PropertyNameLength The length of the name in code units ( 16-bit words ). Do not include null terminator (if any).

	@return 0 if the object does not have the specified property, 1 if the object does have the specified property.
 */
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantHasPropertyFunction )( const struct WebScriptProxyVariant* pVariant
                                                                                                             , const uint16_t* pUTF16PropertyName
                                                                                                             , unsigned long utf16PropertyNameLength
																											 , void* interpreterContext );



/**
	Gets the value of a property of the object referenced by the variant.  This function may be called
	if and only if the GetType function above returned WebScriptProxyVariantTypeObjectRef.  If the variant is not of type
	WebScriptProxyVariantTypeObjectRef, then the implementation must ASSERT in the debug build, and behavior is undefined in the release build.

	In general most callers assume that a call to HasProperty immediately followed by a call to this function is atomic.  In the future
	this may not be the case and if an implementor can not ensure that those two function calls in sequence is atomic, they should
	consider removing this assumption from all callers of this function.  If HasProperty returns 1 then the implementation of this
	function should return either a property value or an exception indicating any error status other than property not found.
	Note that it is not necessary to call HasProperty before calling this method. If HasProperty is not called (or returns 0) then
	calling this method yields behavior appropriate to the variant implementation. This may result in an exception being returned,
	an undefined variant being returned, etc. Note that it is explicitly legal to call this method without calling HasProperty.

	Caller should NOT assume that it will not be re-entered from this function.

	@param pVariant The variant whose referenced object is to be queried for the specified property. pVariant may not be NULL.
	@param pUTF16PropertyName The name of the property encoded in UTF-16.  The name does not have to be null terminated.
	@param utf16PropertyNameLength The length of the name in code units ( 16-bit words ). Do not include null terminator (if any).
	@param pPropertyGetResult	Pointer to a WebScriptProxyInvokeGetSetResult struct that will be filled in by this function
								to indicate whether the value of the property was retrieved successfully,
								there was a script exception thrown in the process of retrieving the property value,
								or this function just failed for some reason. pPropertyGetResult may not be NULL. The callee must
								assume that the contents of the WebScriptProxyInvokeGetSetResult are random - should not assume
								any initialization nor use the contents.
*/
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantGetPropertyFunction )( struct WebScriptProxyVariant* pVariant
                                                                                                    , const uint16_t* pUTF16PropertyName
                                                                                                    , unsigned long utf16PropertyNameLength
																									, void* interpreterContext
                                                                                                    , WebScriptProxyInvokeGetSetResult* pPropertyGetResult );


/**
	Sets the value of a property of the object referenced by the variant.  This function may be called
	if and only if the GetType function above returned WebScriptProxyVariantTypeObjectRef.  If the variant is not of type
	WebScriptProxyVariantTypeObjectRef, then the implementation must ASSERT in the debug build, and behavior is undefined in the release build.

	Caller should NOT assume that it will not be re-entered from this function.

	@param pVariant The variant whose referenced object is to have the specified property set to the specified value. pVariant may not be NULL.
	@param pUTF16PropertyName The name of the property encoded in UTF-16.  The name does not have to be null terminated.
	@param utf16PropertyNameLength The length of the name in code units ( 16-bit words ). Do not include null terminator (if any).
	@param pValue	The new value for the specified property.  This function will increment the reference count of the specified variant,
					by calling the Acquire function above, if the variant needs to survive the call to this function. pValue may not be NULL.
	@param pPropertySetResult	Pointer to a WebScriptProxyInvokeGetSetResult struct that will be filled in by this function
								to indicate whether the value of the property was set successfully,
								there was a script exception thrown in the process of setting the property value,
								or this function just failed for some reason. pPropertySetResult may not be NULL. The callee must
								assume that the contents of the WebScriptProxyInvokeGetSetResult are random - should not assume
								any initialization nor use the contents.

*/
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantSetPropertyFunction )( struct WebScriptProxyVariant* pVariant
                                                                                                    , const uint16_t* pUTF16PropertyName
                                                                                                    , unsigned long utf16PropertyNameLength
                                                                                                    , WebScriptProxyVariant* pValue
																									, void* interpreterContext
                                                                                                    , WebScriptProxyInvokeGetSetResult* pPropertySetResult );

/**
	Deletes a property on the object referenced by the variant.  This function may be called
	if and only if the GetType function above returned WebScriptProxyVariantTypeObjectRef.  If the variant is not of type
	WebScriptProxyVariantTypeObjectRef, then the implementation must ASSERT in the debug build, and behavior is undefined in the release build.

	Return values a sort of screwy because in AS3 if the object was a sealed non-dynamic object then
	delete property will always return false and true otherwise.

	Caller should NOT assume that it will not be re-entered from this function.

	@param pVariant The variant whose referenced object is to have the specified property deleted. pVariant may not be NULL.
	@param pUTF16PropertyName The name of the property encoded in UTF-16.  The name does not have to be null terminated.
	@param utf16PropertyNameLength The length of the name in code units ( 16-bit words ). Do not include null terminator (if any).
	@param ppDeleteException	Pointer to a pointer that is set my this function to point at a variant that represents an exception
								object thrown as a result of the delete property operation.  On entry implementations of this
								function should ASSERT that *ppDeleteException == NULL.  On exit of this function if *ppDeleteException
								is not equal to NULL then the caller owns a refernence to the variant pointed at by *ppDeleteException
								and must eventually call the Release function on that variant to reclaim the resources used by that
								variant.

	@return		0	If the property exists but could not be deleted
				0	If the property did not exist and could not have been deleted if it did exist.
				1	If the property was deleted
				1	If the property did not exist and could have been deleted if it did exist.
*/
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantDeletePropertyFunction )( struct WebScriptProxyVariant* pVariant
                                                                                                                , const uint16_t* pUTF16PropertyName
                                                                                                                , unsigned long utf16PropertyNameLength
																												, void* interpreterContext
                                                                                                                , struct WebScriptProxyVariant** ppDeleteException );

/**
	Determines whether or not the object referenced by the variant can be invoked as a function.  This function may be called if and only if the
	GetType function above returned WebScriptProxyVariantTypeObjectRef.  If the variant is not of type
	WebScriptProxyVariantTypeObjectRef, then the implementation must ASSERT in the debug build, and behavior is undefined in the release build.

	Note that it is legal for CanInvoke to return different values at different times for the same variant instance. Obviously, this behavior must
	not be capricious, as otherwise a CanInvoke/Invoke sequence could fail.

	Caller should NOT assume that it will not be re-entered from this function.

	@param pVariant The variant whose referenced object is to be queried for the ability to be called as a function. pVariant may not be NULL.

	@return 0 if the variant's referenced object CANNOT be called as a function, 1 if the variant's referenced object CAN be called as a function.
*/
typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantCanInvokeFunction )( const struct WebScriptProxyVariant* pVariant );


/**
	Calls the object referenced by the specified variant as a function.  This function may be called if and only if the
	GetType function above returned WebScriptProxyVariantTypeObjectRef.  If the variant is not of type
	WebScriptProxyVariantTypeObjectRef, then the implementation must ASSERT in the debug build, and behavior is undefined in the release build.

	In general most callers assume that a call to CanInvoke immediately followed by a call to this function is atomic.  In future
	this may not be the case and if an implementor can not ensure that those two function calls in sequence is atomic, they should
	consider removing this assumption from all callers of this function.  If CanInvoke returns 1 then the implementation of this
	function should return either success or an exception indicating any error status other than "can't invoke". 
	Note that it is not necessary to call CanInvoke before calling this function. If CanInvoke is not called (or returns 0) then
	calling this function yields behavior appropriate to the variant implementation. This may result in an exception being returned,
	a NOP, etc. Note that it is explicitly legal to call this function without calling CanInvoke.

	Caller should NOT assume that it will not be re-entered from this function.

	@param pVariant The variant whose referenced object is to be called as a function. pVariant may not be NULL.
	@param pThisVariant	The variant whose referenced object is be used as the "this" pointer for the function call.
						Cannot be NULL and must be a variant of type WebScriptProxyVariantTypeObjectRef. Implementation
						of this function must ASSERT in the debug build that this argument is non-NULL and points to a
						variant of type WebScriptProxyVariantTypeObjectRef.  
	@param numArguments The number of arguments being passed to the function. 0 is legal.
	@param argumentArray	An array of variant pointers where each pointer points to an argument for the function call.
							The argument array and all off the references to the variants in that array are owned by the caller.
							This argument MUST be ignored by the caller in numArguments is zero.
							The caller MUST set this argument to NULL if numArguments is zero.
							No pointer within the array may be NULL.
	@param pInvokeResult	Pointer to a WebScriptProxyInvokeGetSetResult struct that will be filled in by this function
							to indicate whether the function returned a value,
							there was a script exception thrown,
							or this function just failed for some reason. pInvokeResult may not be NULL. The callee must
							assume that the contents of the WebScriptProxyInvokeGetSetResult are random - should not assume
							any initialization nor use the contents.

*/
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantInvokeFunction )( const struct WebScriptProxyVariant* pVariant
                                                                                               , struct WebScriptProxyVariant* pThisVariant
                                                                                               , unsigned long numArguments
                                                                                               , struct WebScriptProxyVariant * const * argumentArray
																							   , void* interpreterContext
                                                                                               , struct WebScriptProxyInvokeGetSetResult* pInvokeResult );

struct WebScriptProxyVariantPropertiesEnumData;
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantEnumPropertiesProcFunction )( struct WebScriptProxyVariantPropertiesEnumData* pClientData
                                                                                                           , const uint16_t* pUTF16PropertyName
                                                                                                           , unsigned long utf16PropertyNameLength );

// NOTE: Caller should NOT assume that it will not be re-entered from this function.
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantEnumPropertiesFunction )( const struct WebScriptProxyVariant* pVariant
                                                                                                       , WebScriptProxyVariantEnumPropertiesProcFunction enumProc
                                                                                                       , struct WebScriptProxyVariantPropertiesEnumData* pClientData
																									   , void* interpreterContext );

typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantCanConstructFunction )( const struct WebScriptProxyVariant* pVariant );
// NOTE: similar to CanInvoke, CanConstruct can return different results for same variant instance at different times

// NOTE: Caller should NOT assume that it will not be re-entered from this function.
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantConstructFunction )(	const struct WebScriptProxyVariant* pVariant
                                                                                                  , unsigned long numArguments
                                                                                                  , struct WebScriptProxyVariant * const * argumentArray
																								  , void* interpreterContext
                                                                                                  , struct WebScriptProxyInvokeGetSetResult* pConstructResult );

// NOTE: Caller should NOT assume that it will not be re-entered from this function.
typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantDefaultValueFunction )( const struct WebScriptProxyVariant* pVariant
                                                                                                     , struct WebScriptProxyInvokeGetSetResult* pDefaultValueResult
                                                                                                     , enum WebScriptProxyVariantType typeHint
																									 , void* interpreterContext );

typedef WEBKIT_APOLLO_PROTO1 unsigned char ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantIsDeadVariantFunction )( const struct WebScriptProxyVariant* pVariant );

typedef WEBKIT_APOLLO_PROTO1 void ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantGetObjectTypeNameFunction )( const struct WebScriptProxyVariant* pVariant
                                                                                                                , uint16_t* pUTF16TypeName
                                                                                                                , unsigned long* pTypeNameLength);

/**
	Enumeration for return value of GetObjectType function discussed below.
 */
enum WebScriptProxyVariantObjectType {
	WebScriptProxyVariantObjectTypeFirst = 0,
	
	/**
		Type of object is not known.
	 */
	WebScriptProxyVariantObjectTypeUnknown = 0,
	
	/**
		Object is an array object.
	 */
	WebScriptProxyVariantObjectTypeArray = 1,
	
	/**
		Object is a function object.
	 */
	WebScriptProxyVariantObjectTypeFunction = 2,
	
	/* *** NEW TYPES GO ABOVE THIS COMMENT *** */
	WebScriptProxyVariantObjectTypeLast,
};

/**
	Retrieves the type of the object referenced by the variant.  This function may be called if and only if the
	GetType function above returned WebScriptProxyVariantTypeObjectRef.  If the variant is not of type
	WebScriptProxyVariantTypeObjectRef, then the implementation must ASSERT in the debug build, and behavior is undefined in the release build.
	
	@param pVariant The variant whose referenced object is to be queried for its type. pVariant may not be NULL.

	@return The type of the object referenced by the specified variant.
*/
typedef WEBKIT_APOLLO_PROTO1 WebScriptProxyVariantObjectType ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantGetObjectTypeFunction )( const struct WebScriptProxyVariant* pVariant );


/**
	Retrieves the script engine identifier of the specified variant.  The implementation of this function should return a pointer to a
	global variable that is specific to a particular implementation of the interface defined in this file.  The return value of this
	function can be used to determine if a variant was created by a particular implementation.  If you want to know if a variant was
	created by your implementation of this interface, call this function and check the return value against the pointer to the global
	variable your implementation would return.  If they are the same value then you can safely cast the pointer to the opaque variant to
	a pointer to whatever type you use to implement the variant interface.

	@param pVariant The variant whose script engine identifier is to be returned. pVariant may not be NULL.	

	@return Script engine identifier for the specified variant.
*/
struct WebScriptProxyEngineId;
typedef WEBKIT_APOLLO_PROTO1 const WebScriptProxyEngineId* ( WEBKIT_APOLLO_PROTO2 *WebScriptProxyVariantGetScriptEngineIdFunction )( const struct WebScriptProxyVariant* pVariant );
//xxx - should return opaque type rather than void*?
//chrisb - Strictly speaking I could get rid of the entire function.  Implementations can check the m_pVTable to determine if a variant
//was created by a particular implementation.  

/**
	Enumeration used by the WebScriptProxyInvokeGetSetResult to indicate
	the type of result returned by the GetProperty function above.
*/
enum WebScriptProxyVariantInvokeGetSetResultType {
	WebScriptProxyVariantInvokeGetSetResultFirst = 0,
	
	/**
		The property get or function invoke was successful and the value member of
		the WebScriptProxyInvokeGetSetResult struct contains a reference
		to a variant representing the value of the property or the return value of the invoked
		function.  Ownership of the reference to the
		variant in the value member is passed to the caller of the GetProperty
		function or Invoke function.  The variant in the value member will not be disposed of until
		the caller of the GetProperty function or Invoke function calls the Release function at least once.
	*/
	WebScriptProxyVariantInvokeGetSetResultSuccess = 0,
	
	/**
		The property get, property set, or function invoke failed because the get operation resulted
		in a script exception being thrown.  The exception member of the WebScriptProxyInvokeGetSetResult
		struct contains a reference to a variant representing the exception that was thrown.
		Ownership of the reference to the variant in the exception member is passed to the
		caller of the GetProperty function, SetProperty function, or Invoke function.  The variant in
		the propertyGetException member will not be disposed of until the caller of the GetProperty
		function, SetProperty function, or Invoke function calls the Release function at least
		once.
	*/
	WebScriptProxyVariantInvokeGetSetResultException = 1,

	/**
		The property get, property set or function invoke failed and no script exception is available.
	*/
	WebScriptProxyVariantInvokeGetSetResultFailed = 2,

	/**
		The property get, property set or function invoke failed because the object on
		which the operation was requested is a dead object.  Dead objects are objects
		that have been destroyed even though there is a variant that still points at them.
		This situation can arise when a script engine is shutdown before all the variants
		that point into that script engine have been destroyed.
	*/
	WebScriptProxyVariantInvokeGetSetResultObjectDead = 3,

	/* *** NEW TYPES GO ABOVE THIS COMMENT *** */
	WebScriptProxyVariantInvokeGetSetResultLast
};

static inline unsigned char webScriptProxyGetSetResultIsSuccess(WebScriptProxyVariantInvokeGetSetResultType const type)
{
	return type == WebScriptProxyVariantInvokeGetSetResultSuccess ? 1 : 0;
}

static inline unsigned char webScriptProxyGetSetResultHasException(WebScriptProxyVariantInvokeGetSetResultType const type)
{
	return type == WebScriptProxyVariantInvokeGetSetResultException ? 1 : 0;
}

struct WebScriptProxyInvokeGetSetResult
{
	/**
		Specifies the type of the get/set/invoke result.  See enumeration above
		for the possible values.  Set by the implementation of the
		GetProperty/SetProperty/Invoke functions.
	*/
	enum WebScriptProxyVariantInvokeGetSetResultType type;

	/**
		Set to the value of the property requested by the GetProperty call or the return value of the function call
		requested by the Invoke call by the implementation of the GetProperty function or of the Invoke function
		if and only if the type member of this struct is set to	WebScriptProxyVariantInvokeGetSetResultSuccess.
		Otherwise this member should be set to NULL by the callee and ignored by the caller.

		If this member is non-NULL then the caller of the GetProperty function or of the Invoke function owns a reference
		to the variant pointed at by this member and the variant will not be disposed until
		the caller of the GetProperty function or of the Invoke function calls the Release function at least once for the
		variant.
	*/
	struct WebScriptProxyVariant* value;
	
	/**
		Set to a variant of type WebScriptProxyVariantTypeObjectRef by the implementation of the GetProperty
		function, the SetProperty function, or the Invoke function whose value is the script exception object thrown
		by the get operation if and only if the type member of this struct is set to kWebScriptProxyVariantGetResult_Exception.
		Otherwise this member should be set to NULL by the callee and ignored by the caller.

		If this member is non-NULL then the caller of the GetProperty function, of the SetProperty function, or
		of the Invoke function owns a reference	to the variant pointed at by this member and the variant will not be disposed until
		the caller of the GetProperty function, of the SetProperty function, or of the Invoke function calls the Release function
		at least once for the variant.
	*/
	struct WebScriptProxyVariant* exception;
};



struct WebScriptProxyVariantVTable {
	unsigned long m_vTableSize;
	WebScriptProxyVariantAcquireFunction acquire;
	WebScriptProxyVariantReleaseFunction release;
	WebScriptProxyVariantGetTypeFunction getType;
	WebScriptProxyVariantGetBoolFunction getBool;
	WebScriptProxyVariantGetNumberFunction getNumber;
	WebScriptProxyVariantGetStringUTF16LengthFunction getStringUTF16Length;
	WebScriptProxyVariantGetStringUTF16Function getStringUTF16;
	WebScriptProxyVariantHasPropertyFunction hasProperty;
	WebScriptProxyVariantGetPropertyFunction getProperty;
	WebScriptProxyVariantSetPropertyFunction setProperty;
	WebScriptProxyVariantDeletePropertyFunction deleteProperty;
	WebScriptProxyVariantCanInvokeFunction canInvoke;
	WebScriptProxyVariantInvokeFunction invoke;
	WebScriptProxyVariantGetScriptEngineIdFunction getScriptEngineId;
	WebScriptProxyVariantEnumPropertiesFunction enumProperties;
	WebScriptProxyVariantCanConstructFunction canConstruct;
	WebScriptProxyVariantConstructFunction construct;
	WebScriptProxyVariantDefaultValueFunction defaultValue;
	WebScriptProxyVariantGetObjectTypeFunction getObjectType;
	WebScriptProxyVariantIsDeadVariantFunction isDeadVariant;
    WebScriptProxyVariantGetObjectTypeNameFunction getObjectTypeName;
};

struct WebScriptProxyVariant {
	const struct WebScriptProxyVariantVTable* m_pVTable;
};

#endif
