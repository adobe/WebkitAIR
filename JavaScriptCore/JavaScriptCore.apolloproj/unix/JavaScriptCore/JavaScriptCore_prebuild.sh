#
# Copyright (C) 2011 Adobe Systems Incorporated.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the Adobe Systems Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY ADOBE SYSTEMS INCORPORATED "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ADOBE SYSTEMS INCORPORATED
# OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 

#This script file assumes the current folder as the component's build folder

if [ -z "${BUILD_SHARE_DIR}" ]
    then
    echo "BUILD_SHARE_DIR not provided"
    exit 1
fi

if [ -z "${WEBKIT_ROOT_DIR}" ]
    then
    echo "WEBKIT_ROOT_DIR not provided"
    exit 1
fi

#Included to make use of the GenerateRelativePath function
source "${BUILD_SHARE_DIR}/Common.sh"

PERL_CMD="/usr/bin/perl"
PERL_INC="-I${WEBKIT_ROOT_DIR}/SDK/unix/i80386linux/modules/perl"
PYTHON_CMD="/usr/bin/python"
CREATE_HASH_TABLE="${WEBKIT_ROOT_DIR}/JavaScriptCore/create_hash_table"
CREATE_REGEX_TABLES="${WEBKIT_ROOT_DIR}/JavaScriptCore/create_regex_tables"
DFTABLES_CMD="${WEBKIT_ROOT_DIR}/JavaScriptCore/pcre/dftables"

if [ ! -f "${PERL_CMD}" ]
    then
    echo "Missing perl - ${PERL_CMD}"
    exit 1
fi
if [ ! -f "${PYTHON_CMD}" ]
    then
    echo "Missing python - ${PTYHON_CMD}"
    exit 1
fi

if [ ! -f "${DFTABLES_CMD}" ]
    then
    echo "Missing dftables - ${DFTABLES_CMD}"
    exit 1
fi

if [ ! -f "${CREATE_HASH_TABLE}" ]
    then
    echo "Missing create_hash_table - ${CREATE_HASH_TABLE}"
    exit 1
fi

if [ ! -f "${CREATE_REGEX_TABLES}" ]
    then
    echo "Missing create_regex_tables - ${CREATE_REGEX_TABLES}"
    exit 1
fi

do_create_hash_table()
{
    if [ ! -f "$2" ]
	then
	${PERL_CMD} ${PERL_INC} ${CREATE_HASH_TABLE} $1 -i > $2.tmp
	if [ $? -ne 0 ]
	    then
	    echo "Failure in command :----"
	    echo "${PERL_CMD} ${PERL_INC} ${CREATE_HASH_TABLE} $1 -i > $2.tmp"
	    exit 1
	fi
	
	cp $2.tmp $2
	if [ $? -ne 0 ]
	    then
	    echo "Error copying $2.tmp onto $2"
	fi
    fi
}

do_create_regex_tables()
{
    if [ ! -f "$1" ]
    then
    ${PYTHON_CMD} ${CREATE_REGEX_TABLES} > $1.tmp
    if [ $? -ne 0 ]
        then
        echo "Failure in command :----"
        ${PYTHON_CMD} ${CREATE_REGEX_TABLES} > $1.tmp
        exit 1
    fi

    cp $1.tmp $1
    if [ $? -ne 0 ]
        then
        echo "Error copying $1.tmp onto $1"
    fi
    fi
}

do_grammar_create()
{
    if [ ! -f "Grammar.cpp" ] || [ ! -f "Grammar.h" ]
	then
	if [ -f Grammar.tmp.cpp ]
	    then
	    rm -f Grammar.tmp.cpp
	fi
	
	if [ -f Grammar.tmp.hpp ]
	    then
	    rm -f Grammar.tmp.hpp
	fi
	
	bison -l -d -p jscyy ${WEBKIT_ROOT_DIR}/JavaScriptCore/parser/Grammar.y -o Grammar.tmp.cpp
	if [ $? -ne 0 ]
	    then
	    echo "Failure in command :----"
	    echo "bison -l -d -p jscyy ${WEBKIT_ROOT_DIR}/JavaScriptCore/parser/Grammar.y -o Grammar.tmp.cpp"
	    exit 1
	fi
	
	cp Grammar.tmp.cpp Grammar.cpp
	if [ $? -ne 0 ]
	    then
	    echo "Error copying Grammar.tmp.cpp onto Grammar.cpp"
	fi
	
	cp Grammar.tmp.hpp Grammar.h
	if [ $? -ne 0 ]
	    then
	    echo "Error copying Grammar.tmp.hpp onto Grammar.h"
	fi
    fi
}

do_dftable()
{
    if [ ! -f "chartables.c" ]
	then
	${DFTABLES_CMD} chartables.c
    fi
}

do_create_link()
{
    if [ ! -L "$2" ]
	then
	
	if [ ! -e "$1" ]
	    then
	    echo "Could not create link $2 on $1. Source does not exist."
	    return 1
	fi

	OLD_DIR="`pwd`"

	cd "`dirname $1`"
	ABSOLUTE_SRC="`pwd`"
	cd "$OLD_DIR"

	cd "`dirname $2`"
	ABSOLUTE_DST="`pwd`"
	cd "$OLD_DIR"

	RELATIVE_SRC="`GetRelativePath ${ABSOLUTE_DST} ${ABSOLUTE_SRC}`"
	ln -s "${RELATIVE_SRC}/`basename $1`" "${ABSOLUTE_DST}/`basename $2`"
	if [ $? -ne 0 ]
	    then
	    echo "Could not create link $2 to $1"
	fi
    fi
}

do_create_hash_table ${WEBKIT_ROOT_DIR}/JavaScriptCore/runtime/ArrayPrototype.cpp ArrayPrototype.lut.h
do_create_hash_table ${WEBKIT_ROOT_DIR}/JavaScriptCore/runtime/DatePrototype.cpp DatePrototype.lut.h
do_create_hash_table ${WEBKIT_ROOT_DIR}/JavaScriptCore/runtime/JSONObject.cpp JSONObject.lut.h
do_create_hash_table ${WEBKIT_ROOT_DIR}/JavaScriptCore/runtime/MathObject.cpp MathObject.lut.h
do_create_hash_table ${WEBKIT_ROOT_DIR}/JavaScriptCore/runtime/NumberConstructor.cpp NumberConstructor.lut.h
do_create_hash_table ${WEBKIT_ROOT_DIR}/JavaScriptCore/runtime/RegExpConstructor.cpp RegExpConstructor.lut.h
do_create_hash_table ${WEBKIT_ROOT_DIR}/JavaScriptCore/runtime/RegExpObject.cpp RegExpObject.lut.h
do_create_hash_table ${WEBKIT_ROOT_DIR}/JavaScriptCore/runtime/StringPrototype.cpp StringPrototype.lut.h
do_create_hash_table ${WEBKIT_ROOT_DIR}/JavaScriptCore/parser/Keywords.table Lexer.lut.h

do_create_regex_tables RegExpJitTables.h

do_grammar_create
do_dftable

if [ ! -d "./JavaScriptCore" ]
    then
    mkdir JavaScriptCore
    if [ $? -ne 0 ]
	then
	echo "Could not create the JavaScriptCore folder for copying common include files."
	exit 1
    fi
fi

do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/API/APICast.h JavaScriptCore/APICast.h
do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/API/JavaScript.h JavaScriptCore/JavaScript.h
do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/API/JSBase.h JavaScriptCore/JSBase.h
do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/API/JSContextRef.h JavaScriptCore/JSContextRef.h
do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/API/JSObjectRef.h JavaScriptCore/JSObjectRef.h
do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/API/JSRetainPtr.h JavaScriptCore/JSRetainPtr.h
do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/API/JSStringRef.h JavaScriptCore/JSStringRef.h
do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/API/JSValueRef.h JavaScriptCore/JSValueRef.h
do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/API/JavaScriptCore.h JavaScriptCore/JavaScriptCore.h
do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/API/OpaqueJSString.h JavaScriptCore/OpaqueJSString.h
do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/API/WebKitAvailability.h JavaScriptCore/WebKitAvailability.h
#do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/bindings/runtime.h JavaScriptCore/runtime.h
#do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/bindings/NP_jsobject.h JavaScriptCore/NP_jsobject.h
#do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/bindings/npruntime_impl.h JavaScriptCore/npruntime_impl.h
#do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/bindings/runtime_root.h JavaScriptCore/runtime_root.h
#do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/bindings/npruntime.h JavaScriptCore/npruntime.h
#do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/bindings/runtime_object.h JavaScriptCore/runtime_object.h
#do_create_link ${WEBKIT_ROOT_DIR}/JavaScriptCore/bindings/npapi.h JavaScriptCore/npapi.h


exit 0
