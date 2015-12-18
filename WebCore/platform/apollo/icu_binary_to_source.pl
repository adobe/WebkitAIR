#!/usr/bin/perl
# Copyright (C) 2008 Adobe Systems Incorporated.  All rights reserved.
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
#
#
# This script will generate a C++ source file that has the bytes of
# the file specified as a const unsigned char array.

use strict;
use IO::File;
use Fcntl;
use File::Basename;

if ($#ARGV != 2) {
    print "usage: " . $0 . " input output.cpp variable";
    exit(1);
}

my $inputFileName = $ARGV[0];
my $outputCPPFileName = $ARGV[1];
my $variableName = $ARGV[2];

my $inputFile = new IO::File $inputFileName, O_RDONLY | O_BINARY;

if (defined $inputFile) {
    my $inputFileLength = $inputFile->sysseek(0, Fcntl::SEEK_END);
    $inputFile->sysseek(0, Fcntl::SEEK_SET);
    
    my $outputFile = new IO::File $outputCPPFileName, O_CREAT | O_TRUNC | O_WRONLY;
    if (defined $outputFile) {
        $outputFile->print( "extern \"C\" const struct {\n\tunsigned short bogus[4]; // originally double bogus;\n\tunsigned char bytes\[" . $inputFileLength . "\];\n} " );
        $outputFile->print( $variableName . " = { {0,0,0,0}, {" );
        my $bytesRead = 0;
        my $readResult = 0;
        my $buffer;
        my $linePrefix = "\n";
        do {
          $readResult = $inputFile->sysread($buffer, 16);
          if ((defined $readResult) && ($readResult > 0 )) {
            my @bufferBytes = split(//,$buffer);
            $bytesRead += $#bufferBytes + 1;
            my @bufferStrings = map(sprintf("0x%02X", ord($_)), @bufferBytes);
            my $outputLine = $linePrefix . "  " . join(", ", @bufferStrings);
            $linePrefix = ",\n";
            $outputFile->print( $outputLine );
          }
        } while ((defined $readResult) && ($readResult > 0));
        $outputFile->print("} };");
        $outputFile->close();
    }
    else {
        print "Unable to open output file: " . $outputCPPFileName . "\n";
    }
    $inputFile->close();
}
else {
    print "Unable to open input file: " . $inputFileName . "\n";
}