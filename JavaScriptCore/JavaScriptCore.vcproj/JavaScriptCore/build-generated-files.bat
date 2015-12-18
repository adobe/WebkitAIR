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

@set SDKDir=%~dp0..\..\..\..\..\SDK
if not exist "%SDKDir%" goto ERROR_NO_SDK_DIR

@set perlEXE=%SDKDir%\perl\win\ActivePerl\bin\perl.exe
@if not exist "%perlEXE%" goto ERROR_NO_PERL

@set bisonBinDir=%SDKDir%\bison\win\bison2.1\bin
@set bisonEXE=%bisonBinDir%\bison.exe
@if not exist "%bisonEXE%" goto ERROR_NO_BISON

@set createHashTableScript=%~dp0..\..\kjs\create_hash_table
@if not exist "%createHashTableScript%" goto ERROR_NO_CREATE_HASH_TABLE


@rem Invoke the create_hash_table perl script to create all of our lookup tables

@set hashTableFiles=array_object.cpp bool_object.cpp date_object.cpp error_object.cpp function_object.cpp math_object.cpp
@set hashTableFiles=%hashTableFiles% number_object.cpp object_object.cpp regexp_object.cpp string_object.cpp

@set sourceFilesDir=%~dp0..\..\kjs
@if not exist "%sourceFilesDir%" goto ERROR_NO_SOURCE_FILES_DIR

@for %%i in ( %hashTableFiles% ) do @(
	@call :CREATE_HASH_TABLE "%perlEXE%" "%createHashTableScript%" "%sourceFilesDir%\%%~i" "%derivedSourcesDir%\%%~ni.lut.tmp" "%derivedSourcesDir%\%%~ni.lut.h"
)

@call :CREATE_HASH_TABLE "%perlEXE%" "%createHashTableScript%" "%sourceFilesDir%\keywords.table" "%derivedSourcesDir%\lexer.lut.tmp" "%derivedSourcesDir%\lexer.lut.h"

@if exist "%derivedSourcesDir%\grammar.tmp.cpp" del "%derivedSourcesDir%\grammar.tmp.cpp"
@if exist "%derivedSourcesDir%\grammar.tmp.hpp" del "%derivedSourcesDir%\grammar.tmp.hpp"

@rem Bison requires that its bin directory in the path.  So we temporarily change PATH
@set pathTemp=%PATH%
@set PATH=%bisonBinDir%;%PATH%
"%bisonEXE%" -d -p kjsyy "%sourceFilesDir%\grammar.y" -o "%derivedSourcesDir%\grammar.tmp.cpp"
@set PATH=%pathTemp%


@call :COPY_IF_DIFFERENT "%derivedSourcesDir%\grammar.tmp.cpp" "%derivedSourcesDir%\grammar.cpp"
@call :COPY_IF_DIFFERENT "%derivedSourcesDir%\grammar.tmp.hpp" "%derivedSourcesDir%\grammar.h"
@if exist "%derivedSourcesDir%\grammar.tmp.cpp" del "%derivedSourcesDir%\grammar.tmp.cpp"
@if exist "%derivedSourcesDir%\grammar.tmp.hpp" del "%derivedSourcesDir%\grammar.tmp.hpp"
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

:ERROR_END
@pause
@goto END

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

