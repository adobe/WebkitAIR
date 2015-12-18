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
#include "Interpreter.h"
#include "CallFrame.h"
#include "CodeBlock.h"
#include "ObjectPrototype.h"

#if PLATFORM(APOLLO)
namespace JSC{
    static const char linePropertyName[] = "line";
    static const char sourceURLPropertyName[] = "sourceURL";
    static const char stackTracePropertyName[] = "stackTrace";
    static const char functionNamePropertyName[] = "functionName";
		
    void Interpreter::updateExceptionStackTraceIfPossible(CallFrame*& callFrame
                                                           , JSValue const exceptionValue
                                                           , unsigned bytecodeOffset
                                                           , CodeBlock* const codeBlock)
    {
        if(exceptionValue.isObject()) {
            JSObject* exception = asObject(exceptionValue);
            JSObject* stackTraceArray = 0;
            
            if(!exception->hasProperty(callFrame, Identifier(callFrame, stackTracePropertyName))) {
               //don't have a stack trace yet and we can set one on the object, so create the stack trace array
               ASSERT(callFrame->dynamicGlobalObject());
               
               stackTraceArray = constructEmptyArray(callFrame);
               PutPropertySlot slot;
               exception->put(callFrame, Identifier(callFrame, stackTracePropertyName), stackTraceArray, slot);
               if(callFrame->hadException())
                    stackTraceArray = 0;               
            }
            else {
               //already have a stack trace array
               JSValue const stackTracePropertyValue = exception->get(callFrame, Identifier(callFrame, stackTracePropertyName));
               if(stackTracePropertyValue.isObject())
                    stackTraceArray = asObject(stackTracePropertyValue);
            }
               
            if(stackTraceArray) {
               ASSERT(callFrame->dynamicGlobalObject());
               JSValue pushPropertyValue = stackTraceArray->get(callFrame, Identifier(callFrame, "push"));
               CallData callData;
               CallType callType = pushPropertyValue.getCallData(callData);
               if(callType != CallTypeNone) {
                    JSObject* const currStackFrameObject = new (callFrame) JSObject(JSObject::createStructure(callFrame->lexicalGlobalObject()->objectPrototype()));
                    PutPropertySlot slot;
                    currStackFrameObject->put(callFrame, Identifier(callFrame, linePropertyName), jsNumber(callFrame, codeBlock->lineNumberForBytecodeOffset(callFrame, bytecodeOffset)), slot);
                    currStackFrameObject->put(callFrame, Identifier(callFrame, sourceURLPropertyName), jsOwnedString(callFrame, codeBlock->ownerExecutable()->sourceURL()), slot);
               
                    //if we are not in the global call frame
                    if(callFrame->codeBlock()->codeType() != GlobalCode) {
                        JSFunction* function = callFrame->callee();
                        if(function) {
                            PutPropertySlot slot;
                            currStackFrameObject->put(callFrame, 
                                                      Identifier(callFrame, functionNamePropertyName), 
                                                      jsOwnedString(callFrame, function->name(callFrame)),
                                                      slot);
                        }
               
                        MarkedArgumentBuffer argList;
                        argList.append(currStackFrameObject);
                        call(callFrame, pushPropertyValue, callType, callData, stackTraceArray, argList);
                        //asFunction(pushPropertyValue)->call(callFrame, stackTraceArray, argList);
                    }
               }
            }
        }
	}

    // copied from Interpreter.cpp
    static ALWAYS_INLINE unsigned bytecodeOffsetForPC(CallFrame* callFrame, CodeBlock* codeBlock, void* pc)
    {
    #if ENABLE(JIT)
        return codeBlock->getBytecodeIndex(callFrame, ReturnAddressPtr(pc));
    #else
        UNUSED_PARAM(callFrame);
        return static_cast<Instruction*>(pc) - codeBlock->instructions().begin();
    #endif
    }

    NEVER_INLINE bool Interpreter::previousCallFrame(CallFrame*& callFrame, JSValue& /*exceptionValue*/, unsigned& bytecodeOffset, CodeBlock*& codeBlock)
    {
        if(codeBlock->codeType() == GlobalCode)
            return false;
    	
        void* returnPC = callFrame->returnPC();
        //get the previous call frame
        callFrame = callFrame->callerFrame();	
        if (callFrame->hasHostCallFrameFlag())
            return false;

        codeBlock = callFrame->codeBlock();
        if(!codeBlock)
            return false;

        bytecodeOffset = bytecodeOffsetForPC(callFrame, codeBlock, returnPC);
        return true;
    }
	
    NEVER_INLINE bool Interpreter::exceptionWillBeCaught(CallFrame* callFrame, JSValue& exceptionValue, unsigned bytecodeOffset)	
    {
        CodeBlock* codeBlock = callFrame->codeBlock();
        while(!codeBlock->handlerForBytecodeOffset(bytecodeOffset)) {
            if(!previousCallFrame(callFrame, exceptionValue, bytecodeOffset, codeBlock))
                return false;
        }
    	
        return true;
    }
		
}	
#endif

