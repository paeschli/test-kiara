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

import os, sys, re, types
import traceback
import getopt

if sys.platform == 'win32':
    dirname = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'winpexpect')
    sys.path = [dirname] + sys.path
    import winpexpect

    spawn_process = winpexpect.winspawn
    TIMEOUT = winpexpect.TIMEOUT
    EOF = winpexpect.EOF

else:
    import pexpect

    spawn_process = pexpect.spawn
    TIMEOUT = pexpect.TIMEOUT
    EOF = pexpect.EOF

ERROR_LINE = re.compile('Error:.*$')
EXPECT_TIMEOUT = 60

class TestFailed(Exception):
    pass

def ReceiveText(receiveText):
    return (None ,receiveText)

def SendText(sendText):
    return (sendText, None)

def SendAndReceiveText(sendText, receiveText):
    return (sendText, receiveText)

def startServer(cmdnameTemplate, port=8080, protocol='jsonrpc'):
    endPort = port + 10
    while True:
        cmdname = cmdnameTemplate % {'port' : port, 'protocol' : protocol}
        print "Starting: %s ..." % cmdname,
        p = spawn_process(cmdname)
        i = p.expect(['Starting server...', TIMEOUT, EOF], timeout=EXPECT_TIMEOUT)
        if i == 0:
            print "Starting server ...",
        elif i == 1 or i == 2:
            print
            p.terminate()
            raise TestFailed('Could not start %s' % cmdname)
        i = p.expect([ERROR_LINE, TIMEOUT], timeout=1)
        if i == 0:
            print p.after
            p.terminate()
            if port > endPort:
                raise TestFailed('Could not start %s' % cmdname)
            print "FAILED"
            port += 1
        else:
            break
    print "DONE"
    return p, port

def checkSending(spawnedProcess, sendText):
    if sendText is not None:
        spawnedProcess.sendline(sendText)
        print 'Sent %s' % repr(sendText)

def checkReceiving(spawnedProcess, receiveText):
    if receiveText is not None:
        i = spawnedProcess.expect([receiveText, ERROR_LINE, TIMEOUT, EOF], timeout=EXPECT_TIMEOUT)
        if i == 0:
            print 'Received %s' % repr(receiveText)
        elif i == 1:
            print spawnedProcess.after
            raise TestFailed('Expected: %s' % repr(receiveText))
        elif i == 2:
            print "BEFORE",spawnedProcess.before
            print "AFTER",spawnedProcess.after
            print 'Not Received %s : TIMEOUT' % repr(receiveText)
            raise TestFailed('Expected: %s' % repr(receiveText))
        elif i == 3:
            print 'Not Received %s : EOF (received: %s)' % (repr(receiveText), repr(spawnedProcess.before))
            raise TestFailed('Expected: %s' % repr(receiveText))


def runTests(name, serverCmd, clientCmd, protocol, testList):
    print "-- Running test: %s --" % name
    s, port = startServer(serverCmd, protocol=protocol)
    try:
        c = spawn_process(clientCmd % {'port' : port, 'protocol' : protocol})
        try:
            for t in testList:
                if isinstance(t, types.StringTypes):
                    sendText = None
                    receiveText = t
                else:
                    sendText, receiveText = t

                checkSending(c, sendText)
                checkReceiving(c, receiveText)

        finally:
            c.terminate()
    finally:
        s.terminate()
        print ""

def run_basetypetest(protocol):
    runTests('basetypetest',
             'kiara_basetypetest_server %(port)i %(protocol)s',
             'kiara_basetypetest "http://localhost:%(port)i/service"',
             protocol,
             ['basetype.send_boolean: result = 1',
              'basetype.send_i8: result = -128',
              'basetype.send_u8: result = 255',
              'basetype.send_i16: result = -32768',
              'basetype.send_u16: result = 65535',
              'basetype.send_i32: result = -2147483648',
              'basetype.send_u32: result = 4294967295',
              'basetype.send_i64: result = -9223372036854775808',
              'basetype.send_u64: result = 18446744073709551615',
              'basetype.send_float: result = 64.132004',
              'basetype.send_double: result = 21164.103021',
              'basetype.send_string: result = test string'])

def run_calctest(protocol):
    runTests('calctest',
             'kiara_calctest_server %(port)i %(protocol)s',
             'kiara_calctest "http://localhost:%(port)i/service"',
             protocol,
             ['calc.add: result = 53',
              'calc.addf: result = 53.000000',
              'calc.stringToInt32: result = -125',
              'calc.int32ToString: result = -42',
              'calc.stringToInt32: result = 521',
              'calc.int32ToString: result = 142'])

def run_structtest(protocol):
    runTests('structtest',
             'kiara_structtest_server %(port)i %(protocol)s',
             'kiara_structtest "http://localhost:%(port)i/service2"',
             protocol,
             ['StructTest.pack: result = { ival : 21, sval : "test21" }',
              'StructTest.getInteger: result = 25',
              'StructTest.getString: result = test72',
              'StructTest.setLocation: DONE',
              'StructTest.getLocation: result = position 0.500000 10.500000 8.000000 rotation 0.707107 0.000000 0.000000 0.707107',
              'Server exception raised: 1234 error'])

def run_remote_chat(protocol):
    runTests('remote_chat',
             'kiara_remote_chat_server %(port)i %(protocol)s',
             'kiara_remote_chat_client localhost %(port)i',
             protocol,
             [ReceiveText('Connecting with'),
              ReceiveText('Enter user name:'),
              SendText('daada'),
              ReceiveText('Enter password:'),
              SendText('dadad'),
              ReceiveText('Enter ":q" for exiting'),
              ReceiveText('daada:'),
              SendText('uia'),
              ReceiveText('From: echo \\(id=2\\) Message: uia'), # From: echo (id=2) Message: uia
              SendText('duia'),
              ReceiveText('From: echo \\(id=2\\) Message: duia'), # From: echo (id=2) Message: duia
              SendText(':q'),
              ReceiveText('Exiting.')
              ])

def run_enctest(protocol):
    runTests('enctest',
             'kiara_enctest_server %(port)i %(protocol)s',
             'kiara_enctest "http://localhost:%(port)i/service"',
             protocol,
             ['enc.sendInt: result = 321',
              'enc.sendString: result = data321'])

def run_enctest_forward(protocol):
    name = "enctest_forward"
    print "-- Running test: %s --" % name
    s1, port1 = startServer('kiara_enctest_server %(port)i %(protocol)s', protocol=protocol)
    try:
        s2, port2 = startServer('kiara_enctest_forward_server http://localhost:'+str(port1)+'/service %(port)i %(protocol)s', port=port1+1, protocol=protocol)
        try:
            cmd = ('kiara_enctest http://localhost:%(port)i/service' % {'port' : port2})
            print 'Starting %s' % cmd
            c = spawn_process(cmd)
            try:
                checkReceiving(c, 'enc.sendInt: result = 321')
                checkReceiving(c, 'enc.sendString: result = data321')
            finally:
                c.terminate()
        finally:
            s2.terminate()
    finally:
        s1.terminate()
        print ""

def run_arraytest(protocol):
    runTests('arraytest',
             'kiara_arraytest_server %(port)i %(protocol)s',
             'kiara_arraytest "http://localhost:%(port)i/service"',
             protocol,
             ['Opening connection',
              'arraytest.send: result = Data {',
              'array_boolean: \\[ 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0 \\]',
              'array_i8: \\[ 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 \\]',
              'array_u8: \\[ 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 \\]',
              'array_i16: \\[ 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 \\]',
              'array_u16: \\[ 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 \\]',
              'array_i32: \\[ 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 \\]',
              'array_u32: \\[ 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 \\]',
              'array_i64: \\[ 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 \\]',
              'array_u64: \\[ 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 \\]',
              'array_float: \\[ -49.000000 -48.000000 -47.000000 -46.000000 -45.000000 -44.000000 -43.000000 -42.000000 -41.000000 -40.000000 -39.000000 -38.000000 -37.000000 -36.000000 -35.000000 -34.000000 -33.000000 -32.000000 -31.000000 -30.000000 -29.000000 -28.000000 -27.000000 -26.000000 -25.000000 -24.000000 -23.000000 -22.000000 -21.000000 -20.000000 -19.000000 -18.000000 -17.000000 -16.000000 -15.000000 -14.000000 -13.000000 -12.000000 -11.000000 -10.000000 -9.000000 -8.000000 -7.000000 -6.000000 -5.000000 -4.000000 -3.000000 -2.000000 -1.000000 -0.000000 \\]',
              'array_double: \\[ -49.000000 -48.000000 -47.000000 -46.000000 -45.000000 -44.000000 -43.000000 -42.000000 -41.000000 -40.000000 -39.000000 -38.000000 -37.000000 -36.000000 -35.000000 -34.000000 -33.000000 -32.000000 -31.000000 -30.000000 -29.000000 -28.000000 -27.000000 -26.000000 -25.000000 -24.000000 -23.000000 -22.000000 -21.000000 -20.000000 -19.000000 -18.000000 -17.000000 -16.000000 -15.000000 -14.000000 -13.000000 -12.000000 -11.000000 -10.000000 -9.000000 -8.000000 -7.000000 -6.000000 -5.000000 -4.000000 -3.000000 -2.000000 -1.000000 -0.000000 \\]',
              '}'
              ])

def run_aostest(protocol):
    runTests('aostest',
             'kiara_aostest_server %(port)i %(protocol)s',
             'kiara_aostest "http://localhost:%(port)i/service"',
             protocol,
             ['Opening connection',
              'aostest.setLocations: DONE',
              'aostest.getLocations: LocationList {',
              '  locations: \\[',
              '    position 0.000000 0.000000 0.000000 rotation 0.707107 0.000000 0.000000 0.707107',
              '    position 1.000000 1.000000 1.000000 rotation 0.707107 0.000000 0.000000 0.707107',
              '    position 2.000000 2.000000 2.000000 rotation 0.707107 0.000000 0.000000 0.707107',
              '    position 3.000000 3.000000 3.000000 rotation 0.707107 0.000000 0.000000 0.707107',
              '    position 4.000000 4.000000 4.000000 rotation 0.707107 0.000000 0.000000 0.707107',
              '    position 5.000000 5.000000 5.000000 rotation 0.707107 0.000000 0.000000 0.707107',
              '    position 6.000000 6.000000 6.000000 rotation 0.707107 0.000000 0.000000 0.707107',
              '    position 7.000000 7.000000 7.000000 rotation 0.707107 0.000000 0.000000 0.707107',
              '    position 8.000000 8.000000 8.000000 rotation 0.707107 0.000000 0.000000 0.707107',
              '    position 9.000000 9.000000 9.000000 rotation 0.707107 0.000000 0.000000 0.707107',
              '  \\]',
              '}'
              ])

def usage():
    print "Usage: test_examples.py [options] protocol protocol ..."
    print "Where options are:"
    print " -h | --help       print this"
    print "-t | --timeout     set timeout in seconds"

def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "ht:", ["help", "timeout="])
    except getopt.GetoptError as err:
        # print help information and exit:
        print str(err) # will print something like "option -a not recognized"
        usage()
        sys.exit(2)
    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-t", "--timeout"):
            EXPECT_TIMEOUT = int(a)
            print "Timeout set to", EXPECT_TIMEOUT, "sec"
        else:
            assert False, "unhandled option"
    if not args:
        protocols = ('ortecdr', 'fastcdr', 'jsonrpc', 'tbp')
    else:
        protocols = args

    for protocol in protocols:
        print "Testing protocol %s..." % protocol
        run_basetypetest(protocol)
        run_calctest(protocol)
        run_structtest(protocol)
        run_remote_chat(protocol)
        run_enctest(protocol)
        run_enctest_forward(protocol)
        run_arraytest(protocol)
        run_aostest(protocol)
        print "Successfully tested protocol %s" % protocol
        print

if __name__ == '__main__':
    try:
        main()
        sys.exit(0)
    except TestFailed, e:
        print 'Error: Test failed:', e
        sys.exit(1)
    except SystemExit, e:
        raise e
    except Exception, e:
        print str(e)
        traceback.print_exc()
        os._exit(1)
