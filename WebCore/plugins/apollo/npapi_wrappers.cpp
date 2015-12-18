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
#include "npapi_wrappers.h"
#include "PluginView.h"
#include "npruntime_impl.h"

using namespace WebCore;

// This function is copied from WebCore\plugins\npapi.cpp and should be mantained if that changes.
static PluginView* pluginViewForInstance(NPP instance)
{
    if (instance && instance->ndata)
        return static_cast<PluginView*>(instance->ndata);
    return PluginView::currentPluginView();
};

/*
*	There are two kind of wrappers NPProc_Wrapper (3 versions, for different arguments count) 
*   and NPFun_Wrapper (7 versions, for different arguments count).
*	NPProc_Wrapper wraps functions that have a void return type.
*	NPFun_Wrapper wraps functions that have a non-void return type.
*	Each wrapper packs the arguments in a NPProc_Message (resp. NPFun_Message) and calls
*	PluginView::enterPlayer passing it the packed arguments and a callback implemented as
*	NPProc_Callback (resp. NPFun_Callback).
*	PluginView::enterPlayer basically calls ENTER_PLAYER and if that succeeds it calls the 
*	passed callback with the corresponding packed arguments.
*	The callbacks, NPProc_Callback (resp. NPFun_Callback), unpack the arguments and call the 
*	original functions. 
*	The whole process ensures there is an ENTER_PLAYER on the stack when executing the NPAPI functions.
*/

namespace WebKitApollo
{

template <typename T>
struct DefaultValues
{
	static T failValue;
};

template<> NPObject* DefaultValues< NPObject* >::failValue = NULL;
template<> jref DefaultValues< jref >::failValue = NULL;
template<> bool DefaultValues<bool>::failValue = false;
template<> int32_t DefaultValues<int32_t>::failValue = -1;
template<> const char* DefaultValues<const char*>::failValue = NULL;
template<> NPError DefaultValues<NPError>::failValue = NPERR_GENERIC_ERROR;
#if OS(DARWIN)
template<> NPBool DefaultValues<NPBool>::failValue = false;
#endif

struct NOTYPE{};


//
//	Procedure wrappers
//

template <typename TArg1 = NOTYPE, typename TArg2 = NOTYPE, typename TArg3 = NOTYPE, 
						typename TArg4 = NOTYPE, typename TArg5 = NOTYPE, typename TArg6 = NOTYPE>

struct NPProc_Message
{
	NPP arg0;
	TArg1 arg1;
	TArg2 arg2;
};


// Procedure wrapper 1+0 arguments

template <void (*func)(NPP)>
void NPProc_Callback(void* arg)
{
	NPProc_Message<>* msg = reinterpret_cast< NPProc_Message<>* >(arg);
	NPP instance = msg->arg0;
		
	func(instance);
}

template <void (*func)(NPP)>
void	NPProc_Wrapper(NPP instance)
{
	
	PluginView* view = pluginViewForInstance(instance);

	if(view)
	{
		NPProc_Message<> msg;
		msg.arg0 = instance;

#if ENABLE(NETSCAPE_PLUGIN_API)
		view->enterPlayer(NPProc_Callback<func>,&msg);
#endif
		
	}
	else
	{
		func(instance);
	}
}




// Procedure wrapper 1+1 arguments

template <typename TArg1,
		void (*func)(NPP,TArg1)>
void NPProc_Callback(void* arg)
{
	NPProc_Message<TArg1>* msg = reinterpret_cast< NPProc_Message<TArg1>* >(arg);
	NPP instance = msg->arg0;
	TArg1 arg1 = msg->arg1;
	
	func(instance, arg1);
}

template <typename TArg1, void (*func)(NPP,TArg1)>
void	NPProc_Wrapper(NPP instance, TArg1 arg1)
{
	
	PluginView* view = pluginViewForInstance(instance);

	if(view)
	{
		NPProc_Message<TArg1> msg;
		msg.arg0 = instance;
		msg.arg1 = arg1;

#if ENABLE(NETSCAPE_PLUGIN_API)
		view->enterPlayer(NPProc_Callback<TArg1,func>,&msg);
#endif
		
	}
	else
	{
		func(instance, arg1);
	}
}




// Procedure wrapper 1+2 arguments

template <typename TArg1, typename TArg2, void (*func)(NPP,TArg1,TArg2)>
void NPProc_Callback(void* arg)
{
	NPProc_Message<TArg1,TArg2>* msg = reinterpret_cast< NPProc_Message<TArg1,TArg2>* >(arg);
	NPP instance = msg->arg0;
	TArg1 arg1 = msg->arg1;
	TArg2 arg2 = msg->arg2;
	
	func(instance, arg1, arg2);
}

template <typename TArg1, typename TArg2, void (*func)(NPP,TArg1,TArg2)>
void	NPProc_Wrapper(NPP instance, TArg1 arg1, TArg2 arg2)
{
	

	PluginView* view = pluginViewForInstance(instance);

	if(view)
	{
		NPProc_Message<TArg1,TArg2> msg;
		msg.arg0 = instance;
		msg.arg1 = arg1;
		msg.arg2 = arg2;

#if ENABLE(NETSCAPE_PLUGIN_API)
		view->enterPlayer(NPProc_Callback<TArg1,TArg2,func>,&msg);
#endif
	
	}
	else
	{
		func(instance, arg1, arg2);
	}
}




//
//	Function wrappers
//



template <typename TRes, typename TArg1 = NOTYPE, typename TArg2 = NOTYPE, typename TArg3 = NOTYPE, 
						typename TArg4 = NOTYPE, typename TArg5 = NOTYPE, typename TArg6 = NOTYPE>
struct NPFunc_Message
{
	TRes result;
	
	NPP arg0;
	TArg1 arg1;
	TArg2 arg2;
	TArg3 arg3;
	TArg4 arg4;
	TArg5 arg5;
	TArg6 arg6;
};


// Function wrapper 1+0 arguments

template <typename TRes, 
		TRes (*func)(NPP)>
void NPFunc_Callback(void* arg)
{
	NPFunc_Message<TRes>* msg = reinterpret_cast< NPFunc_Message<TRes>* >(arg);
	NPP instance = msg->arg0;
	
	TRes funcResult = func(instance);

	msg->result = funcResult;
}

template <typename TRes, TRes (*func)(NPP)>
TRes	NPFunc_Wrapper(NPP instance)
{
	TRes funcResult;

	PluginView* view = pluginViewForInstance(instance);

	if(view)
	{
		NPFunc_Message<TRes> msg;
		msg.arg0 = instance;
		
				
#if ENABLE(NETSCAPE_PLUGIN_API)
		if(view->enterPlayer(NPFunc_Callback<TRes,func>,&msg))
		{
			funcResult = msg.result;
		}
		else
#endif
		{
			funcResult = DefaultValues<TRes>::failValue;
		}
		
	}
	else
	{
		funcResult = func(instance);
	}

	return funcResult;
}

// Function wrapper 1+1 arguments

template <typename TRes, typename TArg1,
		TRes (*func)(NPP,TArg1)>
void NPFunc_Callback(void* arg)
{
	NPFunc_Message<TRes,TArg1>* msg = reinterpret_cast< NPFunc_Message<TRes,TArg1>* >(arg);
	NPP instance = msg->arg0;
	TArg1 arg1 = msg->arg1;
	
	TRes funcResult = func(instance, arg1);

	msg->result = funcResult;
}

template <typename TRes, typename TArg1, TRes (*func)(NPP,TArg1)>
TRes	NPFunc_Wrapper(NPP instance, TArg1 arg1)
{
	TRes funcResult;

	PluginView* view = pluginViewForInstance(instance);

	if(view)
	{
		NPFunc_Message<TRes,TArg1> msg;
		msg.arg0 = instance;
		msg.arg1 = arg1;
				
#if ENABLE(NETSCAPE_PLUGIN_API)
		if(view->enterPlayer(NPFunc_Callback<TRes,TArg1,func>,&msg))
		{
			funcResult = msg.result;
		}
		else
#endif
		{
			funcResult = DefaultValues<TRes>::failValue;
		}
		
	}
	else
	{
		funcResult = func(instance, arg1);
	}

	return funcResult;
}

// Function wrapper 1+2 arguments

template <typename TRes, typename TArg1, typename TArg2, TRes (*func)(NPP,TArg1,TArg2)>
void NPFunc_Callback(void* arg)
{
	NPFunc_Message<TRes,TArg1,TArg2>* msg = reinterpret_cast< NPFunc_Message<TRes,TArg1,TArg2>* >(arg);
	NPP instance = msg->arg0;
	TArg1 arg1 = msg->arg1;
	TArg2 arg2 = msg->arg2;
	
	TRes funcResult = func(instance, arg1, arg2);

	msg->result = funcResult;
}

template <typename TRes, typename TArg1, typename TArg2, TRes (*func)(NPP,TArg1,TArg2)>
TRes	NPFunc_Wrapper(NPP instance, TArg1 arg1, TArg2 arg2)
{
	TRes funcResult;

	PluginView* view = pluginViewForInstance(instance);

	if(view)
	{
		NPFunc_Message<TRes,TArg1,TArg2> msg;
		msg.arg0 = instance;
		msg.arg1 = arg1;
		msg.arg2 = arg2;
		
#if ENABLE(NETSCAPE_PLUGIN_API)
		if(view->enterPlayer(NPFunc_Callback<TRes,TArg1,TArg2,func>,&msg))
		{
			funcResult = msg.result;
		}
		else
#endif
		{
			funcResult = DefaultValues<TRes>::failValue;
		}
		
	}
	else
	{
		funcResult = func(instance, arg1, arg2);
	}

	return funcResult;
}


// Function wrapper 1+3 arguments

template <typename TRes, typename TArg1, typename TArg2, typename TArg3, TRes (*func)(NPP,TArg1,TArg2,TArg3)>
void NPFunc_Callback(void* arg)
{
	NPFunc_Message<TRes,TArg1,TArg2,TArg3>* msg = reinterpret_cast< NPFunc_Message<TRes,TArg1,TArg2,TArg3>* >(arg);
	NPP instance = msg->arg0;
	TArg1 arg1 = msg->arg1;
	TArg2 arg2 = msg->arg2;
	TArg3 arg3 = msg->arg3;
	
	TRes funcResult = func(instance, arg1, arg2, arg3);

	msg->result = funcResult;
}

template <typename TRes, typename TArg1, typename TArg2, typename TArg3, TRes (*func)(NPP,TArg1,TArg2,TArg3)>
TRes	NPFunc_Wrapper(NPP instance, TArg1 arg1, TArg2 arg2, TArg3 arg3)
{
	TRes funcResult;

	PluginView* view = pluginViewForInstance(instance);

	if(view)
	{
		NPFunc_Message<TRes,TArg1,TArg2,TArg3> msg;
		msg.arg0 = instance;
		msg.arg1 = arg1;
		msg.arg2 = arg2;
		msg.arg3 = arg3;

#if ENABLE(NETSCAPE_PLUGIN_API)
		if(view->enterPlayer(NPFunc_Callback<TRes,TArg1,TArg2,TArg3,func>,&msg))
		{
			funcResult = msg.result;
		}
		else
#endif
		{
			funcResult = DefaultValues<TRes>::failValue;
		}
		
	}
	else
	{
		funcResult = func(instance, arg1, arg2, arg3);
	}

	return funcResult;
}

// Function wrapper 1+4 arguments

template <typename TRes, typename TArg1, typename TArg2, typename TArg3, typename TArg4, 
	TRes (*func)(NPP,TArg1,TArg2,TArg3,TArg4)>
void NPFunc_Callback(void* arg)
{
	NPFunc_Message<TRes,TArg1,TArg2,TArg3,TArg4>* msg = reinterpret_cast< NPFunc_Message<TRes,TArg1,TArg2,TArg3,TArg4>* >(arg);
	NPP instance = msg->arg0;
	TArg1 arg1 = msg->arg1;
	TArg2 arg2 = msg->arg2;
	TArg3 arg3 = msg->arg3;
	TArg4 arg4 = msg->arg4;
		
	TRes funcResult = func(instance, arg1, arg2, arg3, arg4);

	msg->result = funcResult;
}

template <typename TRes, typename TArg1, typename TArg2, typename TArg3, typename TArg4,  
	TRes (*func)(NPP,TArg1,TArg2,TArg3,TArg4)>
TRes	NPFunc_Wrapper(NPP instance, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4)
{
	TRes funcResult;

	PluginView* view = pluginViewForInstance(instance);

	if(view)
	{
		NPFunc_Message<TRes,TArg1,TArg2,TArg3,TArg4> msg;
		msg.arg0 = instance;
		msg.arg1 = arg1;
		msg.arg2 = arg2;
		msg.arg3 = arg3;
		msg.arg4 = arg4;

#if ENABLE(NETSCAPE_PLUGIN_API)
		if(view->enterPlayer(NPFunc_Callback<TRes,TArg1,TArg2,TArg3,TArg4,func>,&msg))
		{
			funcResult = msg.result;
		}
		else
#endif
		{
			funcResult = DefaultValues<TRes>::failValue;
		}
		
	}
	else
	{
		funcResult = func(instance, arg1, arg2, arg3, arg4);
	}

	return funcResult;
}


// Function wrapper 1+5 arguments

template <typename TRes, typename TArg1, typename TArg2, typename TArg3, typename TArg4, typename TArg5, 
	TRes (*func)(NPP,TArg1,TArg2,TArg3,TArg4,TArg5)>
void NPFunc_Callback(void* arg)
{
	NPFunc_Message<TRes,TArg1,TArg2,TArg3,TArg4,TArg5>* msg = reinterpret_cast< NPFunc_Message<TRes,TArg1,TArg2,TArg3,TArg4,TArg5>* >(arg);
	NPP instance = msg->arg0;
	TArg1 arg1 = msg->arg1;
	TArg2 arg2 = msg->arg2;
	TArg3 arg3 = msg->arg3;
	TArg4 arg4 = msg->arg4;
	TArg5 arg5 = msg->arg5;
		
	TRes funcResult = func(instance, arg1, arg2, arg3, arg4, arg5);

	msg->result = funcResult;
}

template <typename TRes, typename TArg1, typename TArg2, typename TArg3, typename TArg4, typename TArg5, 
	TRes (*func)(NPP,TArg1,TArg2,TArg3,TArg4,TArg5)>
TRes	NPFunc_Wrapper(NPP instance, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5)
{
	TRes funcResult;

	PluginView* view = pluginViewForInstance(instance);

	if(view)
	{
		NPFunc_Message<TRes,TArg1,TArg2,TArg3,TArg4,TArg5> msg;
		msg.arg0 = instance;
		msg.arg1 = arg1;
		msg.arg2 = arg2;
		msg.arg3 = arg3;
		msg.arg4 = arg4;
		msg.arg5 = arg5;

#if ENABLE(NETSCAPE_PLUGIN_API)
		if(view->enterPlayer(NPFunc_Callback<TRes,TArg1,TArg2,TArg3,TArg4,TArg5,func>,&msg))
		{
			funcResult = msg.result;
		}
		else
#endif
		{
			funcResult = DefaultValues<TRes>::failValue;
		}
		
	}
	else
	{
		funcResult = func(instance, arg1, arg2, arg3, arg4, arg5);
	}

	return funcResult;
}


// Function wrapper 1+6 arguments

template <typename TRes, typename TArg1, typename TArg2, typename TArg3, typename TArg4, typename TArg5, typename TArg6,
	TRes (*func)(NPP,TArg1,TArg2,TArg3,TArg4,TArg5,TArg6)>
void NPFunc_Callback(void* arg)
{
	NPFunc_Message<TRes,TArg1,TArg2,TArg3,TArg4,TArg5,TArg6>* msg = reinterpret_cast< NPFunc_Message<TRes,TArg1,TArg2,TArg3,TArg4,TArg5,TArg6>* >(arg);
	NPP instance = msg->arg0;
	TArg1 arg1 = msg->arg1;
	TArg2 arg2 = msg->arg2;
	TArg3 arg3 = msg->arg3;
	TArg4 arg4 = msg->arg4;
	TArg5 arg5 = msg->arg5;
	TArg6 arg6 = msg->arg6;
	
	TRes funcResult = func(instance, arg1, arg2, arg3, arg4, arg5, arg6);

	msg->result = funcResult;
}

template <typename TRes, typename TArg1, typename TArg2, typename TArg3, typename TArg4, typename TArg5, typename TArg6,
	TRes (*func)(NPP,TArg1,TArg2,TArg3,TArg4,TArg5,TArg6)>
TRes	NPFunc_Wrapper(NPP instance, TArg1 arg1, TArg2 arg2, TArg3 arg3, TArg4 arg4, TArg5 arg5, TArg6 arg6)
{
	TRes funcResult;

	PluginView* view = pluginViewForInstance(instance);

	if(view)
	{
		NPFunc_Message<TRes,TArg1,TArg2,TArg3,TArg4,TArg5,TArg6> msg;
		msg.arg0 = instance;
		msg.arg1 = arg1;
		msg.arg2 = arg2;
		msg.arg3 = arg3;
		msg.arg4 = arg4;
		msg.arg5 = arg5;
		msg.arg6 = arg6;

#if ENABLE(NETSCAPE_PLUGIN_API)
		if(view->enterPlayer(NPFunc_Callback<TRes,TArg1,TArg2,TArg3,TArg4,TArg5,TArg6,func>,&msg))
		{
			funcResult = msg.result;
		}
		else
#endif
		{
			funcResult = DefaultValues<TRes>::failValue;
		}
		
	}
	else
	{
		funcResult = func(instance, arg1, arg2, arg3, arg4, arg5, arg6);
	}

	return funcResult;
}


/*
*	InitializeBrowserFuncsWithWrappers initializes a NPNetscapeFuncs structure with wrappers 
*   that redirect into AIR, such that the NPAPI functions are called on stack that contains 
*   the ENTER_PLAYER markers. Only NPAPI functions that receive a NPP argument are wrapped,
*   the others being secure because they do not call into AIR (but that needs to be checked at integration).
*   The initialization sequence should contain all the functions used in WebCore\plugins\PluginPackage.cpp
*	PluginPackage::initializeBrowserFuncs()
*/
void InitializeBrowserFuncsWithWrappers(NPNetscapeFuncs& browserFuncs)
{
	typedef void (*FuncType)(void*);
	
#if ENABLE(NETSCAPE_PLUGIN_API)
	browserFuncs.geturl = NPFunc_Wrapper<NPError, const char* , const char* , NPN_GetURL>;
	browserFuncs.posturl =  NPFunc_Wrapper<NPError, const char*, const char*, uint32_t, const char*, NPBool, NPN_PostURL>;
	browserFuncs.newstream = NPFunc_Wrapper<NPError, NPMIMEType, const char*, NPStream**, NPN_NewStream>;
	browserFuncs.write = NPFunc_Wrapper<int32_t,NPStream*, int32_t, void*, NPN_Write>;
	browserFuncs.destroystream = NPFunc_Wrapper<NPError,  NPStream* , NPReason,	NPN_DestroyStream>;
	browserFuncs.status = NPProc_Wrapper<const char*, NPN_Status>;
	browserFuncs.uagent = NPFunc_Wrapper<const char*, NPN_UserAgent>;
	browserFuncs.memalloc = NPN_MemAlloc;
    browserFuncs.memfree = NPN_MemFree;
    browserFuncs.memflush = NPN_MemFlush;
    browserFuncs.reloadplugins = NPN_ReloadPlugins;
	browserFuncs.geturlnotify = NPFunc_Wrapper<NPError, const char*, const char*, void*, NPN_GetURLNotify>;
	browserFuncs.posturlnotify = NPFunc_Wrapper<NPError, const char*, const char* , uint32_t, const char*, NPBool, void*, NPN_PostURLNotify>;
	browserFuncs.getvalue = NPFunc_Wrapper<NPError, NPNVariable , void*, NPN_GetValue>;
	browserFuncs.setvalue = NPFunc_Wrapper<NPError, NPPVariable , void*, NPN_SetValue>;
	browserFuncs.invalidaterect = NPProc_Wrapper<NPRect*, NPN_InvalidateRect>;
	browserFuncs.invalidateregion = NPProc_Wrapper<NPRegion, NPN_InvalidateRegion>;
#if OS(DARWIN)
    browserFuncs.convertpoint = NPFunc_Wrapper<NPBool, double, double, NPCoordinateSpace, double*, double*, NPCoordinateSpace, NPN_ConvertPoint>;
#endif
	browserFuncs.forceredraw = NPProc_Wrapper<NPN_ForceRedraw>;
	browserFuncs.getJavaEnv = NPN_GetJavaEnv;
	browserFuncs.getJavaPeer = NPFunc_Wrapper<jref,	NPN_GetJavaPeer>;
	browserFuncs.pushpopupsenabledstate =  NPProc_Wrapper<NPBool, NPN_PushPopupsEnabledState>;
	browserFuncs.poppopupsenabledstate = NPProc_Wrapper<NPN_PopPopupsEnabledState>;
	browserFuncs.pluginthreadasynccall = NPProc_Wrapper<FuncType, void*, NPN_PluginThreadAsyncCall>;

	browserFuncs.releasevariantvalue = _NPN_ReleaseVariantValue;
    browserFuncs.getstringidentifier = _NPN_GetStringIdentifier;
    browserFuncs.getstringidentifiers = _NPN_GetStringIdentifiers;
    browserFuncs.getintidentifier = _NPN_GetIntIdentifier;
    browserFuncs.identifierisstring = _NPN_IdentifierIsString;
    browserFuncs.utf8fromidentifier = _NPN_UTF8FromIdentifier;
    browserFuncs.intfromidentifier = _NPN_IntFromIdentifier;
	browserFuncs.createobject = NPFunc_Wrapper<NPObject*, NPClass*, _NPN_CreateObject>;
	browserFuncs.retainobject = _NPN_RetainObject;
    browserFuncs.releaseobject = _NPN_ReleaseObject;
	browserFuncs.invoke = NPFunc_Wrapper<bool, NPObject*, NPIdentifier, const NPVariant* , uint32_t, NPVariant*, _NPN_Invoke>;
	browserFuncs.invokeDefault = NPFunc_Wrapper<bool, NPObject*, const NPVariant* , uint32_t, NPVariant*, _NPN_InvokeDefault>;
	browserFuncs.evaluate = NPFunc_Wrapper<bool, NPObject*, NPString* , NPVariant* , _NPN_Evaluate>;
	browserFuncs.getproperty = NPFunc_Wrapper<bool, NPObject*, NPIdentifier, NPVariant*, _NPN_GetProperty>;
	browserFuncs.setproperty = NPFunc_Wrapper<bool, NPObject*, NPIdentifier, const NPVariant*, _NPN_SetProperty>;
	browserFuncs.removeproperty = NPFunc_Wrapper<bool, NPObject*, NPIdentifier, _NPN_RemoveProperty>;
	browserFuncs.hasproperty = NPFunc_Wrapper<bool, NPObject*, NPIdentifier, _NPN_HasProperty>;
	browserFuncs.hasmethod = NPFunc_Wrapper<bool, NPObject*, NPIdentifier, _NPN_HasMethod>;
	browserFuncs.setexception = _NPN_SetException;
	browserFuncs.enumerate = NPFunc_Wrapper<bool, NPObject*, NPIdentifier**, uint32_t*, _NPN_Enumerate>;
	browserFuncs.construct = NPFunc_Wrapper<bool,  NPObject*, const NPVariant*, uint32_t, NPVariant*, _NPN_Construct>;
#else
    (void)browserFuncs;
#endif
}

}
