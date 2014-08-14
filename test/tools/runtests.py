#!/usr/bin/env python

#
#  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
#
#  Copyright (C) 2013  German Research Center for Artificial Intelligence (DFKI)
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
#

# KIARA Testing Tool
# Author: Dmitri Rubinstein

import sys
import os
import os.path
import subprocess
import easyprocess
import signal
import colorama
from optparse import OptionParser
from types import *

colorama.init()

def is_a_tty(stream):
    return hasattr(stream, 'isatty') and stream.isatty()

def isString(s):
    return type(s) in StringTypes

def isList(l):
    return type(l) in (ListType, TupleType)

def shellQuote(s):
    r = '"'
    for c in s:
        if c in ('$', '`', '\\', '"'):
            r+='\\'
        r+=c
    return r+'"'

def shellQuoteList(l):
    if type(l) in StringTypes:
        return shellQuote(l)
    return ' '.join([shellQuote(e) for e in l])

def dblShellQuote(s):
    return shellQuote(shellQuote(s))

# def runCommand(command, env=None):
#     """returns triple (returncode, stdout, stderr)"""
#     if env is None:
#         env = os.environ
#     myenv = {}
#     for k, v in env.items():
#         myenv[str(k)] = str(v)
#     env = myenv
#     if isList(command):
#         p = subprocess.Popen(command,
#                              stdout=subprocess.PIPE,
#                              stderr=subprocess.PIPE,
#                              env=env,
#                              universal_newlines=False)
#     else:
#         p = subprocess.Popen(command,
#                              stdout=subprocess.PIPE,
#                              stderr=subprocess.PIPE,
#                              env=env,
#                              universal_newlines=False,
#                              shell=True)
#     out = p.stdout.read()
#     p.stdout.close()
#     err = p.stderr.read()
#     p.stderr.close()
#     status = p.wait()

#     return (status, out, err)

# Exit Status
EXIT_ON_TIMEOUT  = 'Timeout'
EXIT_ON_SEGFAULT = 'Segfault'
EXIT_ON_ABORT    = 'Abort'
EXIT_ON_SUCCESS  = 'Success'
EXIT_ON_FAILURE  = 'Failure'

def runCommand(command, env=None, cwd=None, timeout=None):
    """returns triple (returncode, stdout, stderr, exitstatus)"""
    if env is None:
        env = os.environ
    tmpenv = {}
    for k, v in env.items():
        tmpenv[str(k)] = str(v)
    env = tmpenv

    if len(cwd) == 0:
        cwd = None

    p = easyprocess.EasyProcess(command,
                                cwd=cwd,
                                env=env,
                                universal_newlines=False)
    p.call(timeout)

    exitstatus = EXIT_ON_FAILURE
    if p.timeout_happened:
        exitstatus = EXIT_ON_TIMEOUT
    elif p.return_code == 0:
        exitstatus = EXIT_ON_SUCCESS
    elif p.return_code in (134, -signal.SIGABRT):
        exitstatus = EXIT_ON_ABORT
    elif p.return_code in (139, -signal.SIGSEGV):
        exitstatus = EXIT_ON_SEGFAULT

    return (p.return_code, p.stdout, p.stderr, exitstatus)

class TestCase(object):

    SUCCESS = 'Succeed'
    FAILURE = 'Failed'
    TIMEOUT = 'Timeout'
    CRASH = 'Crash'
    ABORT = 'Abort/Assertion'
    WRONG_OUTPUT = 'Wrong Output'

    def __init__(self, name, timeout=None):
        self.name = str(name)
        self.errors = []
        self.command = None
        self.cwd = None
        self.expectedOutput = None
        self.removeLinesFromStdout = []
        self.removeLinesFromStderr = []
        self.subst = {}
        self.setTimeout(timeout)

        self.out = None
        self.err = None
        self.exitCode = None
        self.exitStatus = None
        self.status = None

    def setTimeout(self, timeout):
        if timeout is not None:
            timeout = float(timeout)
        self.timeout = timeout

    def getName(self):
        return self.name

    def message(self, msg, newline=True, color=None):
        if color is not None and is_a_tty(sys.stdout):
            sys.stdout.write(color)
        sys.stdout.write(msg)
        if newline:
            sys.stdout.write('\n')
        if color is not None and is_a_tty(sys.stdout):
            sys.stdout.write(colorama.Fore.RESET + colorama.Back.RESET)
        sys.stdout.flush()

    def newline(self):
        sys.stdout.write('\n')
        sys.stdout.flush()

    def printErrors(self, out):
        cmd = self.getCommandLine()
        if self.cwd:
            out.write("> cd %s; %s\n" % (self.cwd, self.getCommandLineStr()))
        else:
            out.write("> %s\n" % self.getCommandLineStr())

        outputColors = is_a_tty(out)
        if outputColors:
            out.write(colorama.Fore.RED)

        if self.errors:
            out.write("Errors of test %s: [\n" % self.name)
            for e in self.errors:
                out.write(str(e))
                out.write('\n')
            out.write("]\n")

        if self.err:
            out.write("Stderr of test %s: [\n%s]\n" % (self.name, self.err))

        if outputColors:
            out.write(colorama.Fore.RESET)

    def error(self, msg):
        self.errors.append(msg)
        if self.status in (self.SUCCESS, None):
            self.status = self.FAILURE

    def isFailed(self):
        return self.status not in (self.SUCCESS, None)

    def isTimeout(self):
        return self.status == self.TIMEOUT

    def getStatus(self):
        return self.status

    def getCommandLine(self):
        if isString(self.command):
            cmd = self.command % self.subst
        else:
            cmd = self.command
        return cmd

    def getCommandLineStr(self):
        cl = self.getCommandLine()
        if cl is None:
            return '<no command>'
        return ' '.join(cl)

    def run(self):
        if self.isFailed():
            return False
        if self.command is None:
            raise Exception('no command specified')

        cmd = self.getCommandLine()

        if self.cwd:
            msg = 'Executing: cd %s; %s' % (self.cwd, ' '.join(cmd))
        else:
            msg = 'Executing: %s' % ' '.join(cmd)

        self.message(msg, newline=False)
        timeout = self.timeout
        if timeout is not None and timeout < 0:
            timeout = None
        self.exitCode, self.out, self.err, self.exitStatus = \
                       runCommand(cmd, env=os.environ, cwd=self.cwd,
                                  timeout=timeout)

        self.out = self.out.replace('\r', '')
        self.err = self.err.replace('\r', '')

        if self.removeLinesFromStdout:
            out = ''
            for l in self.out.splitlines(True):
                for lineToRemove in self.removeLinesFromStdout:
                    if lineToRemove in l:
                        continue
                    out += l
            self.out = out

        if self.removeLinesFromStderr:
            err = ''
            for l in self.err.splitlines(True):
                for lineToRemove in self.removeLinesFromStderr:
                    if lineToRemove in l:
                        continue
                    err += l
            self.err = err

        self.status = self.SUCCESS
        color = colorama.Fore.GREEN
        if self.exitCode != 0 or self.exitStatus != EXIT_ON_SUCCESS:
            color = colorama.Fore.RED
            if self.exitStatus == EXIT_ON_SEGFAULT:
                self.status = self.CRASH
            elif self.exitStatus == EXIT_ON_TIMEOUT:
                self.status = self.TIMEOUT
            elif self.exitStatus == EXIT_ON_ABORT:
                self.status = self.ABORT
            else:
                self.status = self.FAILURE
        else:
            if self.expectedOutput is not None:
                if self.expectedOutput != self.out:
                    self.error('Unexpected output: [\n%s]' % self.out)
                    self.status = self.WRONG_OUTPUT
                    color = colorama.Fore.RED

        self.message('\t[%s]' % self.status, color=color)

        return not self.isFailed()

    @staticmethod
    def createFromKLOutput(outfilename, timeout=None):
        suffix = '.kl.out'
        assert outfilename.endswith(suffix)
        suffixLen = len(suffix)

        basename = os.path.basename(outfilename)
        cwd = os.path.dirname(outfilename)

        result = open(outfilename, 'rb').read()
        result = result.replace('\r', '')
        testname = outfilename[:-suffixLen]

        klfile = basename[:-suffixLen] + '.kl'
        klfileFullPath = os.path.join(cwd, klfile)
        testcase = TestCase(testname, timeout=timeout)
        if not os.path.exists(klfileFullPath):
            testcase.error('No such file %s' % klfileFullPath)
        else:
            testcase.expectedOutput = result
            #testcase.removeLinesFromStdout = ['==> Runtime']
            #testcase.removeLinesFromStderr = ['Warning assertion']
            #testcase.command = 'anydsl -l deepcps -r %(klfile)s'
            testcase.command = ['kiara-lang', klfile]
            testcase.cwd = cwd
            testcase.subst = {'klfile' : shellQuote(klfile)}
        return testcase

class Tester(object):

    def __init__(self, options):
        self.options = options
        self.errors = []
        self.testCases = []

    def isFailed(self):
        for testCase in self.testCases:
            if testCase.isFailed():
                return True
        return False

    def error(self, msg):
        self.errors.append(msg)

    def processDirectory(self, dir):
        paths = []
        for root, dirs, files in os.walk(dir):
            for f in files:
                paths.append(os.path.join(root, f))
        paths.sort()
        for path in paths:
            self.processFile(path)

    def processFile(self, filename):
        if filename.endswith('.kl.out'):
            testCase = TestCase.createFromKLOutput(filename,
                                                   self.options.timeout)
            testCase.run()
            self.testCases.append(testCase)

    def printResults(self):
        numTests = len(self.testCases)
        numFailed = 0
        numTimeouts = 0
        numWrongOutput = 0
        numCrashed = 0
        outputColors = is_a_tty(sys.stdout)

        for testCase in self.testCases:
            if testCase.isFailed():
                numFailed += 1
                if testCase.isTimeout():
                    numTimeouts += 1
                elif testCase.getStatus() == testCase.WRONG_OUTPUT:
                    numWrongOutput += 1
                elif testCase.getStatus() == testCase.CRASH:
                    numCrashed += 1
                if outputColors:
                    sys.stdout.write(colorama.Fore.RED)
                print "*** Test %s failed [%s] ! ***" % \
                      (testCase.getName(), testCase.getStatus())
                if outputColors:
                    sys.stdout.write(colorama.Fore.RESET)
                testCase.printErrors(sys.stdout)
                print
        if outputColors:
            sys.stdout.write(colorama.Fore.GREEN)
        print "Tests succeed: %i / %i" % (numTests-numFailed, numTests)
        if outputColors:
            sys.stdout.write(colorama.Fore.RED)
        if numFailed:
            print "Tests failed:", numFailed
        if numTimeouts:
            print "Tests failed with timeout:", numTimeouts
        if numWrongOutput:
            print "Tests failed with wrong output:", numWrongOutput
        if numCrashed:
            print "Tests crashed:", numCrashed
        if outputColors:
            sys.stdout.write(colorama.Fore.RESET)

def main():

    usage="""
%prog [options] file/dir file/dir file/dir ...

Run unit tests and report errors.
"""

    op = OptionParser(usage=usage)
    op.add_option('-t', '--timeout', dest='timeout',
                  default=-1,
                  help='Set timeout of MAXTIME seconds for executing of a single test',
                  metavar='MAXTIME')

    if len(sys.argv) < 2:
        print >>sys.stderr, "No files or directories specified"
        op.print_help()
        return 1

    (options, args) = op.parse_args(sys.argv[1:])

    tester = Tester(options)

    for f in args:
        if os.path.isdir(f):
            tester.processDirectory(f)
        else:
            tester.processFile(f)

    tester.printResults()

    if tester.isFailed():
        return 1
    else:
        return 0

if __name__ == '__main__':
    sys.exit(main())
    #try:
    #    sys.exit(main())
    #except Exception, e:
    #    print >>sys.stderr, "Error:", e
    #    sys.exit(1)
