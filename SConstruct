#-*-Python-*-

# SB Engine Bootstrap Loader
import os.path

# Check and execute SB Engine

BUILDER_REQUIRED_VERSION = '2.23'
BUILDER_ENGINE_REPO_NAME = 'scons-builder'
BUILDER_ENGINE_DEST_DIR = './builder'

BUILDER_ENGINE_DEST_DIR = os.path.abspath(BUILDER_ENGINE_DEST_DIR)
BUILDER_ENGINE_FILE = os.path.join(BUILDER_ENGINE_DEST_DIR, 'sbengine.py')

import subprocess

def _fatalError(msg):
    print 'Fatal error: %s' % msg
    Exit(1)

if not os.path.exists(BUILDER_ENGINE_FILE):

    print "SB Engine file %s not found" % BUILDER_ENGINE_FILE
    print "I will try to fetch SB Engine"

    import re
    import SCons.Util

    def _normalizeGitURL(url):
        url = url.strip()
        if url.endswith('/.git'):
            url = url[:-5]
        url = url.rstrip('/')
        return url

    def _splitURL(url):
        url = url.rstrip('/')
        i = url.rfind('/')
        if i == -1:
            i = url.rfind(':')
        i+=1
        head, tail = url[:i], url[i:]
        if head and head != '/'*len(head):
            head = head.rstrip('/')
        return head, tail

    def _baseURL(url):
        return _splitURL(url)[0]

    # Slightly modified version of backtick function
    # from SCons's engine/SCons/Environment.py

    def _runCommand(command, env=None):
        """returns triple (returncode, stdout, stderr)"""
        myenv = {}
        for k, v in env.items():
            myenv[str(k)] = str(v)
        env = myenv
        if SCons.Util.is_List(command):
            p = subprocess.Popen(command,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE,
                                 env=env,
                                 universal_newlines=False)
        else:
            p = subprocess.Popen(command,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE,
                                 env=env,
                                 universal_newlines=False,
                                 shell=True)
        out = p.stdout.read()
        p.stdout.close()
        err = p.stderr.read()
        p.stderr.close()
        status = p.wait()

        return (status, out, err)

    status, out, err = _runCommand('git config --get remote.origin.url',
                                   os.environ)
    if status != 0:
        _fatalError("Could not get git origin URL, maybe git is not in PATH")

    GIT_REMOTE_URL = _normalizeGitURL(out)
    GIT_BASE_URL = _baseURL(GIT_REMOTE_URL)

    # Create list of URLs to check
    #GIT_REMOTE_URL = "http://uni/home/rubinste/proj/rtsg2-dev/.git"

    GIT_BUILDER_URLS = [GIT_REMOTE_URL+'/'+BUILDER_ENGINE_REPO_NAME,
                        GIT_BASE_URL+'/'+BUILDER_ENGINE_REPO_NAME]

    for url in GIT_BUILDER_URLS:
        print 'Trying to clone builder engine from "%s"' % url
        #status, out, err = _runCommand('git clone "%s" %s' % \
        #                               (url, BUILDER_ENGINE_DEST_DIR),
        #                                os.environ)
        status = subprocess.call(["git","clone",url,BUILDER_ENGINE_DEST_DIR])
        if status == 0:
            print 'Successfully cloned builder engine from "%s"' % url
            break
        else:
            print 'Could not clone builder from "%s"' % url
        if not url.endswith('.git'):
            url += '.git'
            print 'Trying to clone builder engine from "%s"' % url
            #status, out, err = _runCommand('git clone "%s" %s' % \
            #                               (url, BUILDER_ENGINE_DEST_DIR),
            #                                os.environ)
            status = subprocess.call(["git","clone",url,BUILDER_ENGINE_DEST_DIR])
            if status == 0:
                print 'Successfully cloned builder engine from "%s"' % url
                break
            else:
                print 'Could not clone builder from "%s"' % url

    if status != 0:
        _fatalError("Could not clone builder from URLs: %s." % (', '.join(GIT_BUILDER_URLS)))

    del _normalizeGitURL
    del _splitURL
    del _baseURL
    del _runCommand

if not os.path.exists(BUILDER_ENGINE_FILE):
    print 'Fatal error: Could not found SB Engine : %s' % BUILDER_ENGINE_FILE
    Exit(1)

# -----------------

import sys
sys.path.append(BUILDER_ENGINE_DEST_DIR)

# this check is required as we need to have "*.py" files
# and not only "*.pyc"
if os.path.exists(os.path.join(BUILDER_ENGINE_DEST_DIR, "butils.py")) and \
       os.path.exists(os.path.join(BUILDER_ENGINE_DEST_DIR, "bversion.py")):
    import butils
    import bversion
    _hasVersion = True
    _requiredVersion = butils.Version(BUILDER_REQUIRED_VERSION)
    if bversion.SB_ENGINE_VERSION < _requiredVersion:
        print "Current SB Engine version is %s" % bversion.SB_ENGINE_VERSION
        print "Required SB Engine version is %s" % _requiredVersion
        _updateBuilder = True
    else:
        _updateBuilder = False
        #print "WILL NOT UPDATE BUILDER",bversion.SB_ENGINE_VERSION,_requiredVersion
else:
    # Current builder version is too old !
    #print "BUILDER TOO OLD"
    _updateBuilder = True
    _requiredVersion = None
    _hasVersion = False

if _updateBuilder:
    import os
    cwd = os.getcwd()
    os.chdir(BUILDER_ENGINE_DEST_DIR)
    status = subprocess.call(["git", "pull"])
    os.chdir(cwd)
    if status == 0:
        print 'Successfully updated builder engine'
    else:
        _fatalError('Could not update builder engine. Please do this manually.')

    # reload butils and bversion from the updated engine
    if _hasVersion:
        reload(butils)
        reload(bversion)
    else:
        import butils
        import bversion

    # We need to recreate required version representation because
    # we reloaded parts of builder (butils and bversion)
    _requiredVersion = butils.Version(BUILDER_REQUIRED_VERSION)

    # recheck versions
    if bversion.SB_ENGINE_VERSION < _requiredVersion:
        _fatalError('After update engine version %s is less than required %s' \
                    % (bversion.SB_ENGINE_VERSION, _requiredVersion))

# cleanup

del _requiredVersion
del _hasVersion
del _updateBuilder
del bversion
del butils
sys.path.pop()
del sys
del _fatalError
del subprocess

# -----------------

if 'builder-check' in COMMAND_LINE_TARGETS:
    print 'Builder check finished, exiting.'
    Exit(0)

execfile(BUILDER_ENGINE_FILE)
