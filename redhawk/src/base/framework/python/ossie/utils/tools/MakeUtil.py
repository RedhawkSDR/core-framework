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


'''
Created on May 18, 2011

'''
import time, os, sys, pprint

__DEBUG__ = True
__PATTERN__ = "Makefile"

def usage():
    txt = """ 
    Usage: MakeUtil.py [options]
    
    Options:
      -h, --help        show this help message and exit
      -l LOCATION, --location=LOCATION
                        The directory or Makefile to find the targets
      -r, --recursive   if location is a directory, searches for all Makefiles
                        (default is False)
    """
    print txt
    
class MakeUtil:
    
    def __init__(self, location=None, is_recursive=False):
        """
        Instantiates a new object with the given arguments.  If the location is
        not provided, it uses the current directory and attempts to either find
        a Makefile there (if not recursive) or all the Makefiles in all the 
        directories underneath.
        
        The results are printed using pretty print using the following format:
        
            {<filename> [target 1,
                       target 2,
                       target 3,
                       .....
                       target n]}
        
        Inputs:
            <location>        Either a directory or a Makefile
            <is_recursive>    Flag determining to recurse through all the 
                              directories or not
        """        
        self.__tgt = location
        self.__recurse = is_recursive
        self.__make_files = []
        
        # determines whether to process a single file or multiple files
        if os.path.isdir(self.__tgt):
            self.__processDir()
        else:
            self.__processFile(self.__tgt)
        
        tgts_dict = {}
        # it checks every line on every Makefile found looking for targets
        for item in self.__make_files:
            self.__log("Parsing %s" % item)
            targets = []  
            obj = open(item, 'r')
            # read every line in the file
            for line in obj.readlines():
                line = line.strip()
                if (not line.startswith('.') and line.endswith(':') and 
                    line.find('=') < 0 and line.find('@') < 0):
                    targets.append(line[:-1])
            obj.close()
            tgts_dict[item] = targets
        # prints the result using pretty print
        print pprint.pformat(tgts_dict)
            
    def __log(self, txt):
        """ 
        Simple function used to print debug statements if the __DEBUG__ flag 
        is set.
        
        Input:
            <txt>    The message to print to stdout
        """
        if __DEBUG__:
            print "%s" % txt
          
    def __processDir(self):
        """
        Looks for Makefiles in the given directory and all the sub-directories
        if recursive is set to true
        """
        self.__log("Processing directory %s" % self.__tgt)  
        
        # if recurse, then use walk otherwise do current directory only
        if self.__recurse:
            for (path, dirs, files) in os.walk(self.__tgt):
                for curr_file in files:
                    # if the file is a Makefile added to process
                    if curr_file == __PATTERN__:
                        fname = os.path.join(path, curr_file)
                        self.__make_files.append(fname)
                        self.__log("Adding %s to list" % fname)
        else:
            # just care to find Makefiles in this directory
            files = os.listdir(self.__tgt)
            if __PATTERN__ in files:
                fname = os.path.join(self.__tgt, __PATTERN__)
                self.__log("Appending %s to the list" % fname)
                self.__make_files.append(fname)
                
    
    def __processFile(self, filename):
        """
        Process a single file if the filename is valid and if is a Makefile
        
        Input:
            <filename>    The name of the Makefile to process
        """
        self.__log("Processing file %s" % filename)        
        path, fname = os.path.split(self.__tgt)
        # making sure the file is valid
        if os.path.isfile(self.__tgt) and fname == __PATTERN__:
            self.__log("Appending %s to the list" % self.__tgt)
            self.__make_files.append(self.__tgt)
        else:
            self.__log("\n\tERROR: File %s is not a %s file\n" % 
                       (self.__tgt, __PATTERN__))
    


if __name__ == "__main__":
    from optparse import OptionParser
    # adding all the options
    parser = OptionParser()
    parser.add_option("-l", "--location", dest="location", 
                      help="The directory or Makefile to find the targets",
                      action="store")
    parser.add_option("-r", "--recursive", dest="recursive", 
                      help="if location is a directory, searches for all Makefiles (default is False)",
                      action="store_true")
    
    (options, args) = parser.parse_args()
    
    loc = None
    is_recursive = False
    # parsing all the options passed by the user
    if options.location == None:
        loc = os.getcwd()
        is_recursive = options.recursive
        
        print "\n\tNo location was provided, using current directory: %s" % loc
        print ""
    else:
        loc = options.location
        is_recursive = options.recursive

    if is_recursive == None:
        is_recursive = False
            
    MakeUtil(location=loc, is_recursive=is_recursive)
    
    
    
