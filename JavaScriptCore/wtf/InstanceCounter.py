import struct;
import datetime;
import sys
import os.path
import os

structFormatStr = u'!LLL'
recordSizeInBytes = struct.calcsize(structFormatStr)

def doRead(fileObj, numBytesToRead):
	result = ''
	while (len(result) < numBytesToRead):
		currBytes = os.read(fileObj.fileno(), numBytesToRead - len(result))
		result = result + currBytes
	return result;

def dumpInstanceCounts(fileName):
	fileObj = file(fileName, u'rb')
	fileObj.seek(0, 2)
	fileSize = fileObj.tell()
	fileObj.seek(0, 0)
	print fileSize
	bytesRead = 0;

	
	while (bytesRead <= (fileSize - recordSizeInBytes)):
		recordBytes = doRead(fileObj, recordSizeInBytes)
		bytesRead = bytesRead + recordSizeInBytes
		#print u'len(recordBytes): ' + unicode(len(recordBytes)) + u' recordSizeInBytes: ' + unicode(recordSizeInBytes) + u' fileObj.tell(): ' + unicode(fileObj.tell()) + u' (fileSize - recordSizeInBytes): ' + unicode((fileSize - recordSizeInBytes))
		(timeStamp, dummyMethodAddr, maxCount) = struct.unpack(structFormatStr, recordBytes)
		timeStampObj = datetime.datetime.fromtimestamp(timeStamp)
		recordStr = timeStampObj.ctime() + u' ' + hex(dummyMethodAddr)[:-1] + u' ' + unicode(maxCount)
		print recordStr


def usage():
	print u'Usage:' + sys.argv[0] + u' file'

if (len(sys.argv) != 2):
	usage()
	sys.exit(-1)

if (not os.path.isfile(sys.argv[1])):
	usage()
	sys.exit(-2)

dumpInstanceCounts(sys.argv[1])
