echo on

@rem Copyright 2007 Adobe Systems Incorporated. All Rights Reserved.
@rem 
@rem Redistribution and use in source and binary forms, with or without
@rem modification, are permitted provided that the following conditions
@rem are met:
@rem 
@rem 1.  Redistributions of source code must retain the above copyright
@rem     notice, this list of conditions and the following disclaimer. 
@rem 2.  Redistributions in binary form must reproduce the above copyright
@rem     notice, this list of conditions and the following disclaimer in the
@rem     documentation and/or other materials provided with the distribution. 
@rem 3.  Neither the name of Adobe Systems, Inc. ("Adobe") nor the names of
@rem     its contributors may be used to endorse or promote products derived
@rem     from this software without specific prior written permission. 
@rem 
@rem THIS SOFTWARE IS PROVIDED BY ADOBE AND ITS CONTRIBUTORS "AS IS" AND ANY
@rem EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
@rem WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
@rem DISCLAIMED. IN NO EVENT SHALL ADOBE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
@rem DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
@rem (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
@rem LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
@rem ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
@rem (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
@rem THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

rem This batch file uses the gnu win32 tools to run the DerivedSources.make file in the root of JavaScriptCore

echo Beginning of file PATH = 
echo %PATH%
echo.

setlocal

set outDir=%~f1

set jsCoreDirRel=..\..
call :ABSPATH "%~dp0%jsCoreDirRel%"
set jsCoreDir=%ABSPATH%

call :SHORTPATH "%jsCoreDir%"
set jsCoreDirShort=%SHORTPATH%
set jsCoreDirShortFS=%jsCoreDirShort:\=/%

set gnuWin32PathRel=..\..\..\..\..\tools\platform\win\gnuwin32
call :ABSPATH "%~dp0%gnuWin32PathRel%"
set gnuWin32Path=%ABSPATH%
set gnuWin32BinDir=%gnuWin32Path%\bin

set makeExe=%gnuWin32BinDir%\make.exe
if not exist "%makeExe%" @goto ERROR_NO_MAKE

set mkdirExe=%gnuWin32BinDir%\mkdir.exe
call :SHORTPATH "%mkdirExe%"
set mkdirExeShort=%SHORTPATH%
if not exist "%mkdirExe%" @goto ERROR_NO_MKDIR

set echoExe=%gnuWin32BinDir%\echo.exe
call :SHORTPATH "%echoExe%"
set echoExeShort=%SHORTPATH%
if not exist "%echoExe%" @goto ERROR_NO_ECHO

set perlBinDirRel=..\..\..\..\..\tools\platform\win\ActivePerl\bin
call :ABSPATH "%~dp0%perlBinDirRel%"
set perlBinDir=%ABSPATH%
set perlPath=%perlBinDir%\perl.exe
if not exist "%perlPath%" @goto ERROR_NO_PERL

set pythonBinDirRel=..\..\..\..\..\tools\platform\win\Python2.5
call :ABSPATH "%~dp0%pythonBinDirRel%"
set pythonBinDir=%ABSPATH%
set pythonPath=%pythonBinDir%\python.exe
if not exist "%pythonPath%" @goto ERROR_NO_PYTHON

set createHashTableRel=..\..\create_hash_table.bat
call :ABSPATH "%~dp0%createHashTableRel%"
set createHashTable=%ABSPATH%
if not exist "%createHashTable%" @goto ERROR_NO_CREATE_HASH_TABLE

set createRegexTablesRel=..\..\create_regex_tables
call :ABSPATH "%~dp0%createRegexTablesRel%"
set createRegexTables=%ABSPATH%
if not exist "%createRegexTables%" @goto ERROR_NO_CREATE_REGEX_TABLES

set makeFile=%jsCoreDirShort%\DerivedSources.make
if not exist "%makeFile%" @goto ERROR_NO_MAKEFILE

call :SHORTPATH "%outDir%"
set outDirShort=%SHORTPATH%
set outDirShortFS=%outDirShort:\=/%
set derivedSourcesDir=%outDirShort%\DerivedSources
call :SHORTPATH "%derivedSourcesDir%"
set derivedSourcesDirShort=%SHORTPATH%
if not exist %derivedSourcesDirShort%\nul @mkdir "%derivedSourcesDir%"
if not exist %derivedSourcesDirShort%\nul @goto ERROR_NO_DERIVED_SOURCES_DIR


set PATH=%gnuWin32BinDir%;%pythonBinDir%;%perlBinDir%;%PATH%
set TMPDIR=%outDir%
pushd "%derivedSourcesDir%"
"%makeExe%" -f "%makeFile%" "MKDIR=%mkdirExeShort%" "ECHO=%echoExeShort%" "APOLLO=1" "CREATE_HASH_TABLE=%createHashTable%" "CREATE_REGEX_TABLES=%createRegexTables%" "JavaScriptCore=%jsCoreDirShortFS%"
popd

set frameworkHeadersDir=%outDir%\include\JavaScriptCore
if not exist "%frameworkHeadersDir%" mkdir %frameworkHeadersDir%

call :COPY_IF_DIFFERENT "%jsCoreDir%\API\APICast.h" "%frameworkHeadersDir%\APICast.h"
call :COPY_IF_DIFFERENT "%jsCoreDir%\API\JavaScript.h" "%frameworkHeadersDir%\JavaScript.h"
call :COPY_IF_DIFFERENT "%jsCoreDir%\API\JSBase.h" "%frameworkHeadersDir%\JSBase.h"
call :COPY_IF_DIFFERENT "%jsCoreDir%\API\JSContextRef.h" "%frameworkHeadersDir%\JSContextRef.h"
call :COPY_IF_DIFFERENT "%jsCoreDir%\API\JSStringRef.h" "%frameworkHeadersDir%\JSStringRef.h"
call :COPY_IF_DIFFERENT "%jsCoreDir%\API\JSRetainPtr.h" "%frameworkHeadersDir%\JSRetainPtr.h"
call :COPY_IF_DIFFERENT "%jsCoreDir%\API\JSObjectRef.h" "%frameworkHeadersDir%\JSObjectRef.h"
call :COPY_IF_DIFFERENT "%jsCoreDir%\API\JSStringRef.h" "%frameworkHeadersDir%\JSStringRef.h"
call :COPY_IF_DIFFERENT "%jsCoreDir%\API\JSValueRef.h" "%frameworkHeadersDir%\JSValueRef.h"
call :COPY_IF_DIFFERENT "%jsCoreDir%\API\JavaScriptCore.h" "%frameworkHeadersDir%\JavaScriptCore.h"
call :COPY_IF_DIFFERENT "%jsCoreDir%\API\OpaqueJSString.h" "%frameworkHeadersDir%\OpaqueJSString.h"
call :COPY_IF_DIFFERENT "%jsCoreDir%\API\WebKitAvailability.h" "%frameworkHeadersDir%\WebKitAvailability.h"

goto END

:ERROR_NO_MAKE
echo Can not find make, "%makeExe%" does not exist!
goto ERROR_END

:ERROR_NO_MKDIR
echo Can not find mkdir, "%mkdirExe" does not exist!
goto ERROR_END

:ERROR_NO_ECHO
echo Can not find echo, "%echoExe" does not exist!
goto ERROR_END

:ERROR_NO_PERL
echo Can not find perl, "%perlPath%" does not exist!
goto ERROR_END

:ERROR_NO_PYTHON
echo Can not find python, "%pythonPath%" does not exist!
goto ERROR_END

:ERROR_NO_CREATE_HASH_TABLE
echo Can not find create_hash_table, "%createHashTable%" does not exist!
goto ERROR_END

:ERROR_NO_CREATE_REGEX_TABLES
echo Can not find create_regex_tables, "%createRegexTables%" does not exist!
goto ERROR_END

:ERROR_NO_MAKEFILE
echo Can not find make file, "%makeFile%" does not exist!
goto ERROR_END

:ERROR_NO_DERIVED_SOURCES_DIR
echo Unable to find or create derived sources directory, "%derivedSourcesDir%" is not a directory
echo or such a directory can not be created.
goto ERROR_END

:ERROR_END
endlocal
exit /b 1

:END
endlocal
goto :EOF


:FIND_IN_PATH
set FIND_IN_PATH=%~$PATH:1
goto :EOF

:ABSPATH
set ABSPATH=%~f1
goto :EOF

:SHORTPATH
set SHORTPATH=%~s1
goto :EOF

:COPY_IF_DIFFERENT
setlocal
set sourceFile=%~1
set destFile=%~2
if not exist "%destFile%" copy "%sourceFile%" "%destFile%" >NUL
fc /b "%sourceFile%" "%destFile%" | find "FC: no differences encountered" >NUL
if errorlevel 1 copy "%sourceFile%" "%destFileDir%" >NUL
endlocal
goto :EOF

echo End of file PATH = 
echo %PATH%
echo.