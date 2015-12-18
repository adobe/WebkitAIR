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

@rem This batch file uses the active state perl distribute to run the create_hash_table perl script.

@setlocal

@set perlPathRel=..\..\..\tools\platform\win\ActivePerl\bin\perl.exe
@call :ABSPATH "%~dp0%perlPathRel%"
@set perlPath=%ABSPATH%
@if not exist "%perlPath%" @goto ERROR_NO_PERL

@set scriptPathRel=create_hash_table
@call :ABSPATH "%~dp0%scriptPathRel%"
@set scriptPath=%ABSPATH%
@if not exist "%scriptPath%" @goto ERROR_NO_SCRIPT

"%perlPath%" "%scriptPath%" %*

@goto END

:ERROR_NO_PERL
@echo Can not find perl, "%perlPath%" does not exist!
@goto ERROR_END

:ERROR_NO_SCRIPT
@echo Can not find perl script, "%scriptPath%" does not exist!
@goto ERROR_END

:ERROR_END
@endlocal
@exit /b 1

:END
@endlocal
@goto :EOF

:ABSPATH
@set ABSPATH=%~f1
@goto :EOF

:SHORTPATH
@set SHORTPATH=%~s1
@goto :EOF