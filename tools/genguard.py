#!/usr/bin/python

#  genguard : C/C++ Guard Macro Generator
#
#  Author: Dmitri Rubinstein <rubinstein@cs.uni-saarland.de>
#
#  Copyright (C) 2009, 2010, 2011, 2012 German Research Center for
#  Artificial Intelligence (DFKI)
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

GUARD1_PATTERN = r"""^#ifndef ([A-Za-z_][A-Za-z_0-9]*)"""
GUARD2_PATTERN_TEMPLATE = r"""^#define %s"""

GUARD1_CMPLPAT = re.compile(GUARD1_PATTERN, re.MULTILINE)

class App:

    def __init__(self):
        self.parser = OptionParser(usage="%prog [options] files",
                                   description="genguard : C/C++ Guard Macro Generator")

        self.parser.set_defaults(sourcedir="src")
    
        self.parser.add_option("-s", "--sourcedir", dest="sourcedir",
                               type="string", action="store",
                               help="Directory relative to which the guards should be generated [default: %default]")

        self.sourcedir = None

    def genGuardName(self, f):
        f = os.path.abspath(f)

        # Delete prefix
        prefix = os.path.commonprefix([f, self.sourcedir])
        f = f[len(prefix):]

        guardParts = []

        head = f
        while True:
            # Split filename by path separator
            head, tail = os.path.split(head)
            if tail:
                # Split filename part by '.'
                tailParts = tail.split('.')
                tailParts.reverse()
                for tailPart in tailParts:
                    guardParts.insert(0, tailPart.upper())
            else:
                break
        guardParts.append('INCLUDED')
        return '_'.join(guardParts)

    def processFile(self, f, newGuard):
        print "Processing file %s" % f
        print "New guard will be %s" % newGuard

        # load file
        fd = open(f, 'r')
        data = fd.read()
        fd.close()

        # Replace guard macros
        match = GUARD1_CMPLPAT.search(data)
        if match:
            # Replace first guard macro
            oldGuard = match.group(1)
            print "Replacing first guard part '#ifndef %s'" % oldGuard
            replacement = "#ifndef %s" % newGuard
            data = GUARD1_CMPLPAT.sub(replacement, data, count=1)

            # Find second guard macro
            pattern = GUARD2_PATTERN_TEMPLATE % oldGuard
            guard2CmplPat = re.compile(pattern, re.MULTILINE)

            match = guard2CmplPat.search(data)
            if match:
                # Replace second guard macro
                print "Replacing second guard part '#define %s'" % oldGuard
                replacement = "#define %s" % newGuard
                data = guard2CmplPat.sub(replacement, data, count=1)
            else:
                print "No guard macro definition '%s' found in file %s" % \
                      (pattern, f)
                return False
            
        else:
            print "No guard macro located in file %s" % f
            return False

        # Replace file with new contents

        backupFileBase = f + '.bak'
        backupFile = backupFileBase
        counter = 1
        while os.path.exists(backupFile):
            backupFile = backupFileBase + ('.%i' % counter)
            counter+=1
        
        shutil.copyfile(f, backupFile)
        print "Backed up file %s to %s" % (f, backupFile)

        fd = open(f, 'w')
        fd.write(data)
        fd.close()

        print "Done processing of file %s" % f

        return True
        

    def run(self):
        (options, args) = self.parser.parse_args()

        if len(args) == 0:
            print "Error: No files specified"
            self.parser.print_usage()
            sys.exit(0)

        self.sourcedir = os.path.abspath(options.sourcedir)

        failed = []
        
        for f in args:
            guardName = self.genGuardName(f)
            if not self.processFile(f, guardName):
                failed.append(f)

        for f in failed:
            print >>sys.stderr, 'Error: Processing of file %s failed' % f
            

if __name__ == '__main__':
    App().run()
