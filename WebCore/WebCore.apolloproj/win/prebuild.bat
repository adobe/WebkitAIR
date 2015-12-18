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

@rem This batch file uses the gnu win32 tools to run the DerivedSources.make file in the root of WebCore

@setlocal

@set outDir=%~f1

@set webCoreDirRel=..\..
@call :ABSPATH "%~dp0%webCoreDirRel%"
@set webCoreDir=%ABSPATH%

@call :SHORTPATH "%webCoreDir%"
@set webCoreDirShort=%SHORTPATH%
@set webCoreDirShortFS=%webCoreDirShort:\=/%

@set gnuWin32PathRel=..\..\..\..\..\tools\platform\win\gnuwin32
@call :ABSPATH "%~dp0%gnuWin32PathRel%"
@set gnuWin32Path=%ABSPATH%
@set gnuWin32BinDir=%gnuWin32Path%\bin

@set makeExe=%gnuWin32BinDir%\make.exe
@if not exist "%makeExe%" @goto ERROR_NO_MAKE

@set mkdirExe=%gnuWin32BinDir%\mkdir.exe
@call :SHORTPATH "%mkdirExe%"
@set mkdirExeShort=%SHORTPATH%
@if not exist "%mkdirExe%" @goto ERROR_NO_MKDIR

@set echoExe=%gnuWin32BinDir%\echo.exe
@call :SHORTPATH "%echoExe%"
@set echoExeShort=%SHORTPATH%
@if not exist "%echoExe%" @goto ERROR_NO_ECHO


@set perlBinDirRel=..\..\..\..\..\tools\platform\win\ActivePerl\bin
@call :ABSPATH "%~dp0%perlBinDirRel%"
@set perlBinDir=%ABSPATH%
@set perlPath=%perlBinDir%\perl.exe
@if not exist "%perlPath%" @goto ERROR_NO_PERL

@set createHashTableRel=..\..\..\JavaScriptCore\create_hash_table.bat
@call :ABSPATH "%~dp0%createHashTableRel%"
@set createHashTable=%ABSPATH%
@if not exist "%createHashTable%" @goto ERROR_NO_CREATE_HASH_TABLE

@set featureDefines=ENABLE_XPATH ENABLE_XSLT PLATFORM_APOLLO ENABLE_JAVASCRIPT_DEBUGGER

@set makeFile=%webCoreDir%\DerivedSources.make
@if not exist "%makeFile%" @goto ERROR_NO_MAKEFILE

@set webKitOutputConfigDir=%outDir%
@call :SHORTPATH "%webKitOutputConfigDir%"
@set webKitOutputConfigDirShort=%SHORTPATH%
@set webKitOutputConfigDirShortFS=%webKitOutputConfigDirShort:\=/%
@set derivedSourcesDir=%webKitOutputConfigDir%\DerivedSources
@call :SHORTPATH "%derivedSourcesDir%"
@set derivedSourcesDirShort=%SHORTPATH%
@if not exist %derivedSourcesDirShort%\nul @mkdir "%derivedSourcesDir%"
@if not exist %derivedSourcesDirShort%\nul @goto ERROR_NO_DERIVED_SOURCES_DIR

@set PATH=%gnuWin32BinDir%;%perlBinDir%;%PATH%
@pushd "%derivedSourcesDir%"
@"%makeExe%" -f "%makeFile%" "MKDIR=%mkdirExeShort%" "ECHO=%echoExeShort%" "APOLLO=1" "SRCROOT=%webCoreDirShortFS%" "SOURCE_ROOT=%webCoreDirShortFS%" "BUILD_PRODUCTS_DIR=%webKitOutputConfigDirShortFS%" "CREATE_HASH_TABLE=%createHashTable%" "WebCore=%webCoreDirShortFS%" "FEATURE_DEFINES=%featureDefines%"
@popd

@goto END

:ERROR_NO_MAKE
@echo Can not find make, "%makeExe%" does not exist!
@goto ERROR_END

:ERROR_NO_MKDIR
@echo Can not find mkdir, "%mkdirExe%" does not exist!
@goto ERROR_END

:ERROR_NO_ECHO
@echo Can not find echo, "%echoExe%" does not exist!
@goto ERROR_END

:ERROR_NO_PERL
@echo Can not find perl, "%perlPath%" does not exist!
@goto ERROR_END

:ERROR_NO_CREATE_HASH_TABLE
@echo Can not find create_hash_table, "%createHashTable%" does not exist!
@goto ERROR_END

:ERROR_NO_MAKEFILE
@echo Can not find make file, "%makeFile%" does not exist!
@goto ERROR_END

:ERROR_NO_DERIVED_SOURCES_DIR
@echo Unable to find or create derived sources directory, "%derivedSourcesDir%" is not a directory
@echo or such a directory can not be created.
@goto ERROR_END

:ERROR_END
@endlocal
@exit /b 1

:END
@endlocal
@goto :EOF


:FIND_IN_PATH
@set FIND_IN_PATH=%~$PATH:1
@goto :EOF

:ABSPATH
@set ABSPATH=%~f1
@goto :EOF

:SHORTPATH
@set SHORTPATH=%~s1
@goto :EOF
