#!/usr/bin/python


#  make_ptr_uids : Converts hexadecimal numbers (like 0xABCDEF) to unique
#                  object IDs
#  Copyright (C) 2009  Dmitri Rubinstein
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import sys
import os.path
import re
import shutil
from optparse import Option, OptionGroup, OptionParser, OptionError

HEX_PATTERN = r"""(0x[0-9a-fA-F]+)"""

HEX_CMPLPAT = re.compile(HEX_PATTERN)

def processFile(f):
    print >>sys.stderr, "Processing file %s" % f

    # load file
    fd = open(f, 'r')
    data = fd.read()
    fd.close()

    idMap = {}
    idCounter = 0

    destData = ''

    startPos = 0

    while True:
        match = HEX_CMPLPAT.search(data, startPos)
        if match:
            hexPtr = match.group(1)

            id = idMap.get(hexPtr)
            if id is None:
                id = idCounter
                idCounter += 1
                idMap[hexPtr] = id

            replacement = "ID_%i" % id

            destData += data[startPos : match.start(1)]
            destData += replacement
            startPos = match.end(1)
        else:
            destData += data[startPos:]
            break

    # Replace file with new contents

    backupFileBase = f + '.bak'
    backupFile = backupFileBase
    counter = 1
    while os.path.exists(backupFile):
        backupFile = backupFileBase + ('.%i' % counter)
        counter+=1

    shutil.copyfile(f, backupFile)
    print >>sys.stderr, "Backed up file %s to %s" % (f, backupFile)

    fd = open(f, 'w')
    fd.write(destData)
    fd.close()

    print >>sys.stderr, "Done processing of file %s" % f

    return True

def main():
    parser = OptionParser(usage="%prog [options] files",
                          description="make_ptr_uids : Unique object ID filter")

    (options, args) = parser.parse_args()

    if len(args) == 0:
        print >>sys.stderr, "Error: No files specified"
        parser.print_usage()
        sys.exit(0)

    failed = []

    for f in args:
        if not processFile(f):
            failed.append(f)

    for f in failed:
        print >>sys.stderr, 'Error: Processing of file %s failed' % f

if __name__ == '__main__':
    main()
