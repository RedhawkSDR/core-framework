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
import os
import imp
import sys
import commands

def loadModule(filename):
    if filename == '':
        raise RuntimeError, 'Empty filename cannot be loaded'
    print "Loading module %s" % (filename)
    searchPath, file = os.path.split(filename)
    if not searchPath in sys.path: 
        sys.path.append(searchPath)
        sys.path.append(os.path.normpath(searchPath+"/../"))
    moduleName, ext = os.path.splitext(file)
    fp, pathName, description = imp.find_module(moduleName, [searchPath,])

    try:
        module = imp.load_module(moduleName, fp, pathName, description)
    finally:
        if fp:
            fp.close()

    return module

def getUnitTestFiles(rootpath, testFileGlob="test_*.py"):
    rootpath = os.path.normpath(rootpath) + "/"
    print "Searching for files in %s with prefix %s" % (rootpath, testFileGlob)
    test_files = commands.getoutput("find %s -name '%s'" % (rootpath, testFileGlob))
    files = test_files.split('\n')
    if files == ['']:
        files = []
    files.sort()
    return files

