#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK core.
#
# REDHAWK core is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import unittest, sys, os, shutil
import glob
import os.path
import time
import signal
import threading

from omniORB import CORBA, PortableServer
from omniORB import URI, any

from ossie.cf import CF, CF__POA
from ossie.parsers import DCDParser
from ossie.properties import simple_property
from ossie.resource import usesport
import ossie.parsers.prf as PRFParser
import ossie.utils
from omniORB import any
import subprocess

import CosNaming
import re
import tempfile
import buildconfig
import runtestHelpers
import commands
import traceback

_DCEUUID_RE = re.compile("DCE:[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{12}")

def uuidgen():
    return commands.getoutput('uuidgen')

def getSdrPath():
    try:
        return os.path.abspath(os.environ['SDRROOT'])
    except KeyError:
        return os.path.abspath(os.path.join(os.path.dirname(__file__), "../sdr"))

def getSdrCache():
    try:
        return os.environ['SDRCACHE']
    except KeyError:
        return os.path.join(getSdrPath(), 'dev')

def getDatPath():
   return os.path.join(os.getcwd(), "dat")

def hasJavaSupport():
    return runtestHelpers.haveJavaSupport('../Makefile')

def persistenceEnabled():
    for backend in ( "BDB", "GDBM", "SQLITE" ):
        if getBuildDefineValue("ENABLE_"+backend+"_PERSISTENCE") == "1":
            return True
    return False

def eventsEnabled():
    return getBuildDefineValue("ENABLE_EVENTS") == "1"

def getTestDomainName():
    # To avoid conflicts with others, spawnNodeBooter will use a special test domain name
    return os.environ['USER']+str(os.getpid())

def cleanDomainNameContext(root, name):
    pass

def getDomainMgrURI():
    domainName = getTestDomainName()
    return URI.stringToName("%s/%s" % (domainName, domainName))

def getExecDeviceNode():
    return "/nodes/test_GPP_node/DeviceManager.dcd.xml"

def getExecDevice():
    return "GPP_node"


def getLogConfig():
    return os.environ['OSSIEUNITTESTSLOGCONFIG']

def getBuildDefineValue(name):
    for define in buildconfig.DEFS.split():
        if not define.startswith("-D"):
            continue
        defname, value = define[2:].split("=")
        if defname == name:
            return value
    return None

def updateLink(source, target):
    if os.path.islink(target):
        # Remove old, possibly stale link.
        os.unlink(target)
    if not os.path.exists(target):
        # Do not replace existing files.
        os.symlink(source, target)


def setupDeviceAndDomainMgrPackage():
    # Point to the SDR source directory for the DomainManager and
    # DeviceManager softpkgs.
    sdrSrc = os.path.realpath('../control/sdr')

    # "Install" the DomainManager softpkg.
    domMgrSrc = os.path.join(sdrSrc, 'dommgr')
    domMgrDest = os.path.join(getSdrPath(), "dom", "mgr")
    updateLink(os.path.join(domMgrSrc, 'DomainManager'), os.path.join(domMgrDest, 'DomainManager'))
    for xmlFile in glob.glob(os.path.join(domMgrSrc, '*.xml')):
        updateLink(xmlFile, os.path.join(domMgrDest, os.path.basename(xmlFile)))

    # "Install" the ComponentHost softpkg
    compHostSrc = os.path.join(sdrSrc, 'ComponentHost')
    compHostDest = os.path.join(getSdrPath(), "dom/mgr/rh/ComponentHost")
    try:
        os.makedirs(compHostDest)
    except OSError:
        # Assume it failed because the directory already exists
        pass
    updateLink(os.path.join(compHostSrc, 'ComponentHost'), os.path.join(compHostDest, 'ComponentHost'))
    for xmlFile in glob.glob(os.path.join(compHostSrc, '*.xml')):
        updateLink(xmlFile, os.path.join(compHostDest, os.path.basename(xmlFile)))

    # "Install" the DeviceManager softpkg.
    devMgrSrc = os.path.join(sdrSrc, 'devmgr')
    devMgrDest = os.path.join(getSdrPath(), "dev", "mgr")
    updateLink(os.path.join(devMgrSrc, 'DeviceManager'), os.path.join(devMgrDest, 'DeviceManager'))
    for xmlFile in glob.glob(os.path.join(devMgrSrc, '*.xml')):
        updateLink(xmlFile, os.path.join(devMgrDest, os.path.basename(xmlFile)))

def createTestDomain():
    domainName = getTestDomainName()
    print domainName

    domainPath = os.path.join(getSdrPath(), "dom", "domain")
    templatePath = os.path.join(getSdrPath(), "templates", "domain")
    # Create a test domain
    if os.path.isdir(domainPath):
        shutil.rmtree(domainPath)
    # Copy the template over
    shutil.copytree(templatePath, domainPath)
    # Open the DMD file and replace the name, using a very naive method
    dmd = open(os.path.join(domainPath, "DomainManager.dmd.xml"), "r")
    lines = dmd.read()
    dmd.close()
    lines = lines.replace("${DOMAINNAME}", domainName)
    dmd = open(os.path.join(domainPath, "DomainManager.dmd.xml"), "w+")
    dmd.write(lines)
    dmd.close()

    setupDeviceAndDomainMgrPackage()

def which(command):
    """
    Searches the path for the executable specified in 'command'.
    """
    for path in os.environ['PATH'].split(os.pathsep):
        execpath = os.path.join(path, command)
        if os.path.isfile(execpath) and os.access(execpath, os.X_OK):
            return execpath
    return None

def _skip(obj):
    return None

def _id(obj):
    return obj

def _skipUnless(condition, reason):
    if hasattr(unittest, 'skipUnless'):
        return unittest.skipUnless(condition, reason)
    elif not condition:
        return _skip
    else:
        return _id

def requireJava(obj):
    """
    Decorator to conditionally disable a test if Java support is not enabled.
    """
    return _skipUnless(hasJavaSupport(), 'Java is disabled')(obj)

def requireLog4cxx(obj):
    """
    Decorator to conditionally disable a test if log4cxx is not enabled.
    """
    log4cxx = runtestHelpers.haveLoggingSupport('../Makefile')
    return _skipUnless(log4cxx, 'log4cxx is disabled')(obj)

def requirePersistence(obj):
    """
    Decorator to conditionally disable a test if persistence is not enabled.
    """
    return _skipUnless(persistenceEnabled(), 'persistence is not enabled')(obj)

DEBUG_NODEBOOTER=False
GDB_CMD_FILE=None
def spawnNodeBooter(dmdFile=None, 
                    dcdFile=None, 
                    debug=-1, 
                    domainname=None, 
                    loggingURI=None, 
                    endpoint=None, 
                    dbURI=None, 
                    execparams="", 
                    nodeBooterPath="../../control/framework/nodeBooter",
                    stderr=None):
    args = []
    if dmdFile != None:
        args.extend(["-D", dmdFile])
    if dcdFile != None:
        args.extend(["-d", dcdFile])
    if domainname == None:
        # Always use the --domainname argument because
        # we don't want to have to read the DCD files or regnerate them
        args.extend(["--domainname", getTestDomainName()])
    else:
        args.extend(["--domainname", domainname])

    if endpoint is not None:
        args.extend(["-ORBendPoint", endpoint])

    if dbURI:
        args.extend(["--dburl", dbURI])

    if debug != -1:
        args.extend(["-debug", str(debug)])

    if loggingURI is not None:
        if loggingURI:
            args.extend(["-log4cxx", loggingURI])
    else:
        logconfig = getLogConfig()
        if os.path.exists(logconfig):
            args.extend(["-log4cxx", logconfig])
        else:
            print "Could not find Log Conf file <" + logconfig + '>'
            print "Not using a log configuration file"
    args.extend(execparams.split(" "))
    args.insert(0, nodeBooterPath)

    print '\n-------------------------------------------------------------------'
    print 'Launching nodeBooter', " ".join(args)
    print '-------------------------------------------------------------------'
    nb = ossie.utils.Popen(args, cwd=getSdrPath(), shell=False, preexec_fn=os.setpgrp, stderr=stderr)
    if DEBUG_NODEBOOTER:
        absNodeBooterPath = os.path.abspath("../control/framework/nodeBooter")
        if GDB_CMD_FILE != None:
            os.system("xterm -T '%s' -e 'gdb -p %d -x %s' &" % (argstr, nb.pid, GDB_CMD_FILE))
        else:
            os.system("xterm -T '%s' -e 'gdb -p %d' &" % (argstr, nb.pid))

    return nb

def  getProcessArgs( pname ):
     args=[]
     try:
        p=subprocess.Popen(['pgrep', '-f', pname ], stdout=subprocess.PIPE)
        o,e = p.communicate()
        if o != None and o != '':
            pid=int(o)
            cmdline = open('/proc/'+str(pid)+'/cmdline')
            args = cmdline.readlines()[0].split('\x00')
     except:
         traceback.print_exc()
         pass

     return args
               

class OssieTestCase(unittest.TestCase):

    def assertMatch(self, first, re, msg=None):
        """Test that the first value matches the regular expression"""
        self.assertNotEqual(re.compile(re).match(first), None)

    def failUnlessMatch(self, first, re, msg=None):
        self.assertMatch(first, re, msg)

    def assertNotMatch(self, first, re, msg=None):
        """Test that the first value does not matches the regular expression"""
        self.assertEqual(re.compile(re).match(first), None)

    def failUnlessNotMatch(self, first, re, msg=None):
        self.assertNotMatch(first, re, msg)

    def assertIsDceUUID(self, uuid, msg=None):
        if msg == None:
            msg = "%s is not a DCE UUID" % uuid
        self.assertNotEqual(_DCEUUID_RE.match(uuid), None, msg)

    def failUnlessIsDceUUID(self, uuid, msg=None):
        self.assertIsDceUUID(uuid, msg)

    def assertFileExists(self, filename):
        self.assert_(os.path.exists(filename), "File %s does not exist" % filename)

    def assertFileNotExists(self, filename):
        self.failIf(os.path.exists(filename), "File %s exists" % filename)

    def assertAlomstEqual(self, a, b):
        self.assertEqual(round(a-b, 7), 0)

    def assertAlomstNotEqual(self, a, b):
        self.assertNotEqual(round(a-b, 7), 0)

    def promptToContinue(self):
        if sys.stdout.isatty():
            raw_input("Press enter to continue")
        else:
            pass # For non TTY just continue

    def promptUserInput(self, question, default):
        if sys.stdout.isatty():
            ans = raw_input("%s [%s]?" % (question, default))
            if ans == "":
                return default
            else:
                return ans
        else:
            return default

    def assertPrompt(self, question):
        """Prompt the user to answer the question [Y]/N.  Fail if they answer N.
        If stdin isn't a tty then assume the test passed."""
        if sys.stdout.isatty():
            ans = raw_input("%s [Y]/N?:" % question)
            if not ans.upper() in ("", "Y", "YES"):
                self.fail("No answer to %s" % question)
        else:
            pass # For non TTY just continue

    def assertNotPrompt(self, question):
        """Prompt the user to answer the question Y/[N].  Fail if they answer Y.
        If stdin isn't a tty then assume the test passed."""
        if sys.stdout.isatty():
            ans = raw_input("%s Y/[N]?:" % question)
            if not ans.upper() in ("", "N", "NO"):
                self.fail("Yes answer to %s" % question)
        else:
            pass # For non TTY just continue

    def waitPredicate (self, predicate, wait=5.0):
        """
        Waits up to wait seconds (default 5) for a predicate to become true.

        The predicate should be a callable object that takes no arguments;
        typically a lambda function. Calling the predicate should have no
        significant side effects.
        """
        end = time.time() + wait
        while not predicate() and time.time() < end:
            time.sleep(0.1)

    def assertPredicateWithWait (self, predicate, msg=None, wait=5.0):
        """
        Waits up to wait seconds (default 5) for a predicate to become true.
        If the wait period has elapsed and the predicate is still not true, the
        calling test fails.

        The predicate should be a callable object that takes no arguments;
        typically a lambda function. Calling the predicate should have no
        significant side effects.
        """
        self.waitPredicate(predicate, wait)
        self.assert_(predicate(), msg)

    def getProcessArgs(self,pname ):
        return getProcessArgs(pname)

class CorbaTestCase(OssieTestCase):
    """A helper class for test cases which need a CORBA connection."""
    def __init__(self, methodName='runTest', orbArgs=[]):
        unittest.TestCase.__init__(self, methodName)
        args = sys.argv
        self.debuglevel = -1
        for arg in args:
            if '--debuglevel' in arg:
                self.debuglevel = arg.split('=')[-1]
        self._orb = CORBA.ORB_init(sys.argv + orbArgs, CORBA.ORB_ID)
        self._poa = self._orb.resolve_initial_references("RootPOA")
        self._poa._get_the_POAManager().activate()
        self._ns = self._orb.resolve_initial_references("NameService")
        self._root = self._ns._narrow(CosNaming.NamingContext)

        # Maintain a registry of the DomainManager (there should normally be just one)
        # and all spawned DeviceManagers, for easy cleanup.
        self._domainBooter = None
        self._domainManager = None

        self._deviceLock = threading.Lock()
        self._deviceBooters = []
        self._deviceManagers = []
        self._execparams = ""

    def _addDeviceBooter(self, devBooter):
        self._deviceLock.acquire()
        try:
            self._deviceBooters.append(devBooter)
        finally:
            self._deviceLock.release()

    def _getDeviceBooters(self):
        self._deviceLock.acquire()
        try:
            return self._deviceBooters[:]
        finally:
            self._deviceLock.release()

    def _addDeviceManager(self, devMgr):
        self._deviceLock.acquire()
        try:
            self._deviceManagers.append(devMgr)
        finally:
            self._deviceLock.release()

    def _getDeviceManagers(self):
        self._deviceLock.acquire()
        try:
            return self._deviceManagers[:]
        finally:
            self._deviceLock.release()

    def tearDown(self):
        for devMgr in self._getDeviceManagers():
            try:
                devMgr.shutdown()
            except CORBA.Exception:
                # Ignore all CORBA errors.
                pass

        for devBooter in self._getDeviceBooters():
            self.terminateChild(devBooter)

        if self._domainBooter:
            self.terminateChild(self._domainBooter)

        try:
            self._root.unbind(URI.stringToName(getTestDomainName()))
        except:
            pass

    def _getDomainManager(self):
        try:
            domMgr = self._root.resolve(getDomainMgrURI())._narrow(CF.DomainManager)
            if domMgr:
                return domMgr
        except:
            pass
        return None

    def _getDeviceManager(self, domMgr, id):
        for devMgr in domMgr._get_deviceManagers():
            try:
                if id == devMgr._get_identifier():
                    return devMgr
            except CORBA.Exception:
                # The DeviceManager being checked is unreachable.
                pass
        return None

    def _getDCDPath(self, dcdFile):
        # Validate that a real DCD file was given.
        dcdPath = dcdFile
        if dcdPath.startswith('/'):
            dcdPath = dcdPath[1:]
        dcdPath = os.path.join(getSdrPath(), 'dev', dcdPath)
        if not os.path.exists(dcdPath):
            raise IOError, "Invalid dcdPath %s" % (dcdPath,)
        return dcdPath

    def launchDomainManager(self, dmdFile="", *args, **kwargs):
        # Only allow one DomainManager, although this isn't a hard requirement.
        # If it has exited, allow a relaunch.
        if self._domainBooter and self._domainBooter.poll() == None:
            return (self._domainBooter, self._domainManager)

        # If debug level is not given, default to configured level
        if self.debuglevel != -1:
            kwargs.setdefault('debug', self.debuglevel)

        # Launch the nodebooter.
        self._domainBooter = spawnNodeBooter(dmdFile=dmdFile, execparams=self._execparams, *args, **kwargs)
        while self._domainBooter.poll() == None:
            self._domainManager = self._getDomainManager()
            if self._domainManager:
                try:
                    self._domainManager._get_identifier()
                    break
                except:
                    pass
            time.sleep(0.1)
        return (self._domainBooter, self._domainManager)

    def launchDeviceManager(self, dcdFile, domainManager=None, wait=True, *args, **kwargs):
        try:
            dcdPath = self._getDCDPath(dcdFile)
        except IOError:
            print "ERROR: Invalid DCD path provided to launchDeviceManager", dcdFile
            return (None, None)

        # If debug level is not given, default to configured level
        if self.debuglevel != -1:
            kwargs.setdefault('debug', self.debuglevel)

        # Launch the nodebooter.
        if domainManager == None:
            name = None
        else:
            name = domainManager._get_name()
        devBooter = spawnNodeBooter(dcdFile=dcdFile, domainname=name, execparams=self._execparams, *args, **kwargs)
        self._addDeviceBooter(devBooter)

        if wait:
            devMgr = self.waitDeviceManager(devBooter, dcdFile, domainManager)
        else:
            devMgr = None

        return (devBooter, devMgr)

    def waitDeviceManager(self, devBooter, dcdFile, domainManager=None):
        try:
            dcdPath = self._getDCDPath(dcdFile)
        except IOError:
            print "ERROR: Invalid DCD path provided to waitDeviceManager", dcdFile
            return None

        # Parse the DCD file to get the identifier and number of devices, which can be
        # determined from the number of componentplacement elements.
        dcd = DCDParser.parse(dcdPath)
        if dcd.get_partitioning():
            numDevices = len(dcd.get_partitioning().get_componentplacement())
        else:
            numDevices = 0

        # Allow the caller to override the DomainManager (assuming they have a good reason).
        if not domainManager:
            domainManager = self._domainManager

        # As long as the nodebooter process is still alive, keep checking for the
        # DeviceManager.
        devMgr = None
        while devBooter.poll() == None:
            devMgr = self._getDeviceManager(domainManager, dcd.get_id())
            if devMgr:
                break
            time.sleep(0.1)

        if devMgr:
            # Wait for all of the devices to register.
            self._waitRegisteredDevices(devMgr, numDevices)
            self._addDeviceManager(devMgr)
        return devMgr

    def waitForDeviceManager(self, node_dir):
        dcdPath = getSdrPath()+"/dev/nodes/"+node_dir+"/DeviceManager.dcd.xml"

        dcd = DCDParser.parse(dcdPath)
        if dcd.get_partitioning():
            numDevices = len(dcd.get_partitioning().get_componentplacement())
        else:
            numDevices = 0

        dm = self._getDomainManager()

        devMgr = None
        while devMgr == None:
            devMgr = self._getDeviceManager(dm, dcd.get_id())
            if devMgr:
                break
            time.sleep(0.1)

        if devMgr:
            self._waitRegisteredDevices(devMgr, numDevices)
            self._addDeviceManager(devMgr)
        return devMgr


    def _waitRegisteredDevices(self, devMgr, numDevices, timeout=5.0, pause=0.1):
        while timeout > 0.0:
            try: # when responding to error conditions during some tests, the Device Manager will trigger a TRANSIENT error
                if (len(devMgr._get_registeredDevices())+len(devMgr._get_registeredServices())) == numDevices:
                    return True
                else:
                    timeout -= pause
                    time.sleep(pause)
            except:
                break
        return False

    def waitTermination(self, child, timeout=5.0, pause=0.1):
        while child.poll() is None and timeout > 0.0:
            timeout -= pause
            time.sleep(pause)
        return child.poll() != None

    def terminateChild(self, child, signals=(signal.SIGINT, signal.SIGTERM)):
        if child.poll() != None:
           return
        try:
            #self.waitTermination(child)
            
            for sig in signals:
                #print "sending signal " + str(sig) + " to pid:" + str(child.pid)
                os.kill(child.pid, sig)
                if self.waitTermination(child):
                    break
            child.wait()
        except OSError, e:
            #print "terminateChild: pid:" + str(child.pid) + " OS ERROR:" + str(e)
            pass
        finally:
            pass


    def terminateChildrenPidOnly(self, pid, signals=(signal.SIGINT, signal.SIGTERM)):
        ls = commands.getoutput('ls /proc')
        entries = ls.split('\n')
        for entry in entries:
            filename = '/proc/'+entry+'/status'
            try:
                fp = open(filename,'r')
                stuff = fp.readlines()
                fp.close()
            except:
                continue
            ret = ''
            for line in stuff:
                if 'PPid' in line:
                    ret=line
                    break
            if ret != '':
                parentPid = ret.split('\t')[-1][:-1]
                if parentPid == pid:
                    self.terminateChildrenPidOnly(entry, signals)
        filename = '/proc/'+pid+'/status'
        for sig in signals:
            try:
                os.kill(int(pid), sig)
            except:
                continue
            done = False
            attemptCount = 0
            while not done:
                try:
                    fp = open(filename,'r')
                    fp.close()
                    attemptCount += 1
                    if attemptCount == 10:
                        break
                    time.sleep(0.1)
                except:
                    done = True
            if not done:
                continue

    def terminateChildren(self, child, signals=(signal.SIGINT, signal.SIGTERM)):
        ls = commands.getoutput('ls /proc')
        entries = ls.split('\n')
        for entry in entries:
            filename = '/proc/'+entry+'/status'
            try:
                fp = open(filename,'r')
                stuff = fp.readlines()
            except:
                continue
            for line in stuff:
                if 'PPid' in line:
                    ret=line
                    break
            if ret != '':
                parentPid = int(ret.split('\t')[-1][:-1])
                if parentPid == child.pid:
                    self.terminateChildrenPidOnly(entry, signals)

        if child.poll() != None:
            return
        try:
            for sig in signals:
                os.kill(child.pid, sig)
                if self.waitTermination(child):
                    break
            child.wait()
        except OSError:
            pass

