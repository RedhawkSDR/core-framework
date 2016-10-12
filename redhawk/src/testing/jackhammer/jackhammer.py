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

import threading
import time
import sys
import os
import getopt

from omniORB import CORBA, URI
import CosNaming

from ossie.cf import CF
import ossie.parsers.dmd

class Jackhammer(object):
    def __init__ (self):
        args = ["-ORBoneCallPerConnection", "0"]
        self.orb = CORBA.ORB_init(args, CORBA.ORB_ID)
        self.inc = self.orb.resolve_initial_references('NameService')._narrow(CosNaming.NamingContext)
        self.runThreads = threading.Event()
        self.threads = []

    def setup (self, domainName, *args):
        self.domainName = domainName
        try:
            self.domMgr = self.inc.resolve(URI.stringToName('/'.join([domainName]*2)))
        except:
            print >>sys.stderr, "Unable to connect to domain manager for domain", domainName
            sys.exit(1)

        print 'Arguments: ', ' '.join(args)
        self.initialize(*args)

    def createThreads (self, numThreads):
        print "Creating %d threads" % (numThreads,)
        for ii in xrange(numThreads):
            t = threading.Thread(target=self.run, args=(ii,))
            t.start()
            self.threads.append(t)

        self.numThreads = len(self.threads)

    def jackhammer (self):
        print 'Starting threads'
        self.runThreads.set()

        while self.runThreads.isSet() and self.numThreads > 0:
            try:
                time.sleep(1)
            except KeyboardInterrupt:
                print 'Terminating'
                self.runThreads.clear()

    def run (self, id):
        self.runThreads.wait()
        
        while self.runThreads.isSet():
            try:
                self.test()
            except:
                excType = sys.exc_info()[0]
                excStr = sys.exc_info()[1]
                print "Error (thread %d): %s %s" % (id, excType, excStr)
                break
        self.numThreads -= 1

    def initialize (self, *args):
        """
        Override in subclasses to handle test-specific initialization.
        """
        pass

    def test (self):
        """
        Override in subclasses to implement tested behavior.
        """
        pass

    def options(self):
        """
        Override in subclasses to add command line options. Must return a tuple
        of short and long options, in getopt format.
        """
        return "", []

    def setOption(self, key, value):
        """
        Override in subclasses to handle command line options.
        """
        pass


def run(TestClass):
    hammer = TestClass()
    shortopts, longopts = hammer.options()
    shortopts = "" + shortopts
    longopts = [ "domainname=", "threads=" ] + longopts
    opts, args = getopt.getopt(sys.argv[1:], shortopts, longopts)

    try:
        dmdFile = os.path.join(os.environ["SDRROOT"], "dom", "domain", "DomainManager.dmd.xml")
        dmd = ossie.parsers.dmd.parse(dmdFile)
        domainName = str(dmd.get_name())
    except:
        domainName = None

    nThreads = 1
    for key, value in opts:
        if key == "--threads":
            nThreads = int(value)
        elif key == "--domainname":
            domainName = value
        else:
            hammer.setOption(key, value)

    if domainName is None:
        print >>sys.stderr, "Unable to determine domain name; use --domainname=<name>"
        sys.exit(1)

    hammer.setup(domainName, *args)
    hammer.createThreads(nThreads)
    hammer.jackhammer()
