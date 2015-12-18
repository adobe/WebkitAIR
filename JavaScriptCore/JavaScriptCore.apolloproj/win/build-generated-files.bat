@rem
@rem Batch file to generate files needed by KJS library
@rem
@rem (c) 2006 by Adobe Systems Inc.  All Rights Reserved.
@rem
@rem Removes JavaScriptCore's dependency on cygwin.

@setlocal
@set derivedSourcesDir=%~1

@if not exist "%derivedSourcesDir%" mkdir "%derivedSourcesDir%"
@if not exist "%derivedSourcesDir%" goto ERROR_NO_DERIVED_SOURCES_DIR

@set SDKDirRel=..\..\..\..\..\SDK
@call :ABSPATH "%~dp0%SDKDirRel%"
@set SDKDir=%ABSPATH%
if not exist "%SDKDir%" goto ERROR_NO_SDK_DIR

@set perlEXE=%SDKDir%\perl\win\ActivePerl\bin\perl.exe
@if not exist "%perlEXE%" goto ERROR_NO_PERL

@set gnuWin32BinDir=%SDKDir%\win\gnuwin32\bin

@set bisonEXE=%gnuWin32BinDir%\bison.exe
@if not exist "%bisonEXE%" goto ERROR_NO_BISON

@set createHashTableScript=%~dp0..\..\create_hash_table
@if not exist "%createHashTableScript%" goto ERROR_NO_CREATE_HASH_TABLE

@rem Invoke the create_hash_table perl script to create all of our lookup tables

@set hashTableFiles=ArrayPrototype.cpp DatePrototype.cpp MathObject.cpp
@set hashTableFiles=%hashTableFiles% NumberConstructor.cpp RegExpConstructor.cpp RegExpObject.cpp StringPrototype.cpp

@set sourceFilesDir=%~dp0..\..\runtime
@set parserFilesDir=%~dp0..\..\parser
@if not exist "%sourceFilesDir%" goto ERROR_NO_SOURCE_FILES_DIR
@if not exist "%parserFilesDir%" goto ERROR_NO_PARSER_FILES_DIR

@for %%i in ( %hashTableFiles% ) do @(
	@call :CREATE_HASH_TABLE "%perlEXE%" "%createHashTableScript%" "%sourceFilesDir%\%%~i" "%derivedSourcesDir%\%%~ni.lut.tmp" "%derivedSourcesDir%\%%~ni.lut.h"
)

@call :CREATE_HASH_TABLE "%perlEXE%" "%createHashTableScript%" "%parserFilesDir%\Keywords.table" "%derivedSourcesDir%\Lexer.lut.tmp" "%derivedSourcesDir%\Lexer.lut.h"

@if exist "%derivedSourcesDir%\Grammar.tmp.cpp" del "%derivedSourcesDir%\Grammar.tmp.cpp"
@if exist "%derivedSourcesDir%\Grammar.tmp.hpp" del "%derivedSourcesDir%\Grammar.tmp.hpp"

@rem Bison requires that its bin directory in the path. 
@set PATH=%gnuWin32BinDir%;%PATH%

@"%bisonEXE%" -d -p kjsyy "%parserFilesDir%\Grammar.y" -o "%derivedSourcesDir%\Grammar.tmp.cpp"

@call :COPY_IF_DIFFERENT "%derivedSourcesDir%\Grammar.tmp.cpp" "%derivedSourcesDir%\Grammar.cpp"
@call :COPY_IF_DIFFERENT "%derivedSourcesDir%\Grammar.tmp.hpp" "%derivedSourcesDir%\Grammar.h"
@if exist "%derivedSourcesDir%\Grammar.tmp.cpp" del "%derivedSourcesDir%\Grammar.tmp.cpp"
@if exist "%derivedSourcesDir%\Grammar.tmp.hpp" del "%derivedSourcesDir%\Grammar.tmp.hpp"

@set dfTables=%~dp0..\..\pcre\dftables

@setlocal
@set TMPDIR=%derivedSourcesDir%
@"%perlEXE%" "%dfTables%" "%derivedSourcesDir%\chartables.c.tmp"
@endlocal


@call :COPY_IF_DIFFERENT "%derivedSourcesDir%\chartables.c.tmp" "%derivedSourcesDir%\chartables.c"



@goto END

:ERROR_NO_DERIVED_SOURCES_DIR
@echo "%derivedSourcesDir%" does not exist and could not be created.
@goto ERROR_END

:ERROR_NO_SDK_DIR
@echo "%SDKDir%" does not exist!!!
goto ERROR_END

:ERROR_NO_PERL
@echo "%perlEXE%" does not exist!!!
@goto ERROR_END

:ERROR_NO_BISON
@echo "%bisonEXE%" does not exist!!!
@goto ERROR_END

:ERROR_NO_CREATE_HASH_TABLE
@echo "%createHashTableScript%" does not exist!!!
@goto ERROR_END

:ERROR_NO_SOURCE_FILES_DIR
@echo "%sourceFilesDir%" does not exist!!!
@goto ERROR_END

:ERROR_NO_PARSER_FILES_DIR
@echo "%parserFilesDir%" does not exist!!!
@goto ERROR_END

:ERROR_END
@endlocal
@exit /b 1

:END
@endlocal
@goto :EOF

:CREATE_HASH_TABLE
@setlocal
@set perlEXE=%~1
@set createHashTableScript=%~2
@set createHashTableCmd="%perlEXE%" "%createHashTableScript%"
@set sourceFile=%~3
@set derivedSourceTemp=%~4
@set derivedSource=%~5
@if exist "%derivedSourceTemp%" del "%derivedSourceTemp%"
@%createHashTableCmd% "%sourceFile%" -i > "%derivedSourceTemp%"
@call :COPY_IF_DIFFERENT "%derivedSourceTemp%" "%derivedSource%"
@del "%derivedSourceTemp%"
@endlocal
@goto :EOF

:COPY_IF_DIFFERENT
@setlocal
@set sourceFile=%~1
@set destFile=%~2
@if not exist "%destFile%" copy /Y "%sourceFile%" "%destFile%" >NUL
@fc /b "%sourceFile%" "%destFile%" | find "FC: no differences encountered" >NUL
@if errorlevel 1 copy /Y "%sourceFile%" "%destFile%" >NUL
@endlocal
@goto :EOF

:ABSPATH
@set ABSPATH=%~f1
@goto :EOF
