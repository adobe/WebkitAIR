#prebuild gets the configuration name and the output dir

if [ -z "${WEBKIT_ROOT_DIR}" ]
    then
    echo "WEBKIT_ROOT_DIR not provided"
    exit 1
fi

if [ -z "${BUILD_SHARE_DIR}" ]
    then
    echo "BUILD_SHARE_DIR not provided"
    exit 1
fi

#Included to make use of the GenerateRelativePath function
source "${BUILD_SHARE_DIR}/Common.sh"

if [ ! -d "./DerivedSources" ]
    then
    mkdir -p DerivedSources
    if [ $? -ne 0 ]
	then
	echo "Could not create the DerivedSources folder."
	exit 1
    fi
fi

cd DerivedSources
if [ $? -ne 0 ]
    then
    echo "Could not switch folder to DerivedSources."
    exit 1
fi

export SRCROOT="${WEBKIT_ROOT_DIR}/WebCore"
export SOURCE_ROOT="${SRCROOT}"
export BUILT_PRODUCTS_DIR="../"
export CREATE_HASH_TABLE="${WEBKIT_ROOT_DIR}/JavaScriptCore/create_hash_table"
export WebCore="${SRCROOT}"
export ENCODINGS_FILE="${WebCore}/platform/win/win-encodings.txt";
export ENCODINGS_PREFIX="\" \""
export FEATURE_DEFINES="ENABLE_XPATH ENABLE_XSLT PLATFORM_APOLLO ENABLE_JAVASCRIPT_DEBUGGER"

if [ ! -f "${CREATE_HASH_TABLE}" ]
    then
    echo "Missing create_hash_table - ${CREATE_HASH_TABLE}"
    exit 1
fi

make -f "${WebCore}/DerivedSources.make" APOLLO=1 || exit 1


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
	cd "${OLD_DIR}"

	cd "`dirname $2`"
	ABSOLUTE_DST="`pwd`"
	cd "${OLD_DIR}"

	RELATIVE_SRC="`GetRelativePath ${ABSOLUTE_DST} ${ABSOLUTE_SRC}`"
	ln -s "${RELATIVE_SRC}/`basename $1`" "${ABSOLUTE_DST}/`basename $2`"
	if [ $? -ne 0 ]
	    then
	    echo "Could not create link $2 to $1"
	fi
    fi
}

cd ..
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
