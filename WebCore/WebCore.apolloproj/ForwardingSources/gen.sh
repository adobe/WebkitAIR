#!/bin/bash

for derivedSourceFileName in `cat list.txt` ; do
  derivedSourceBaseName=`basename $derivedSourceFileName`
  sed -e "s,__file__,$derivedSourceFileName,g" template.txt > $derivedSourceBaseName
done
