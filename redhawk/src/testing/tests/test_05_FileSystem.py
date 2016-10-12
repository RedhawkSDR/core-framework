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

import unittest, os
import shutil
from _unitTestHelpers import scatest
from ossie.cf import CF

class FileManagerTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager(debug=self.debuglevel)
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml", debug=self.debuglevel)

    def test_BasicOperation(self):
        self.assertNotEqual(self._domMgr, None)
        fileMgr = self._domMgr._get_fileMgr()

        devfile = "/ExecutableDevice_node/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml"
        devlocalfile = "dev/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml"
        domfile = "/waveforms/CommandWrapper/CommandWrapper_DAS.xml"
        domlocalfile = "dom/waveforms/CommandWrapper/CommandWrapper_DAS.xml"
        addon = "foo.txt"
        devnewfile = "/ExecutableDevice_node/nodes/testdev.txt"
        devlocalnewfile = "dev/nodes/testdev.txt"
        domnewfile = "/waveforms/testdom.txt"
        domlocalnewfile = "dom/waveforms/testdom.txt"

        str_to_write_dev = "testing 1,2,3 dev"
        str_to_write_dom = "testing 1,2,3 dom"



        #################
        # Test exists
        #################
        try:
            fileMgr.exists(devfile)
        except:
            self.fail("Something bad happened in fileMgr.exists(devfile) for mountPoint lookup")

        try:
            fileMgr.exists(domfile)
        except:
            self.fail("Something bad happened in fileMgr.exists(domfile) for domain (local fs) lookup")

        try:
            fileMgr.exists(devfile + addon)
        except:
            self.fail("Something bad happened in fileMgr.exists(devfile + addon) for mountPoint lookup")

        try:
            fileMgr.exists(domfile + addon)
        except:
            self.fail("Something bad happened in fileMgr.exists(domfile + addon) for domain (local fs) lookup")

        self.assertEqual(fileMgr.exists(devfile), True)
        self.assertEqual(fileMgr.exists(domfile), True)

        self.assertEqual(fileMgr.exists(devfile+addon), False)
        self.assertEqual(fileMgr.exists(domfile+addon), False)

        #################
        # test open
        #################
        try:
            myfile = fileMgr.open(devfile, True)
        except:
            self.fail("Something bad happened in fileMgr.open(devfile, True) for mountPoint lookup")

        expectedSize = os.stat(os.path.join(scatest.getSdrPath(), devlocalfile))[6]
        self.assertEqual(myfile.sizeOf(), expectedSize)

        try:
            myfile.close()
        except:
            self.fail("Something bad happened in fileMgr.close(devfile, True) for mountPoint lookup")

        try:
            myfile = fileMgr.open(domfile, True)
        except:
            self.fail("Something bad happened in fileMgr.open(domfile, True) for domain (local fs) lookup")

        expectedSize = os.stat(os.path.join(scatest.getSdrPath(), domlocalfile))[6]
        self.assertEqual(myfile.sizeOf(), expectedSize)

        try:
            myfile.close()
        except:
            self.fail("Something bad happened in fileMgr.close(devfile, True) for mountPoint lookup")

        try:
            myfile = fileMgr.open(devfile+addon, True)
            self.fail("fileMgr.open(devfile+addon, True) should have thrown and exception and it didn't")
        except: pass

        try:
            myfile = fileMgr.open(domfile+addon, True)
            self.fail("fileMgr.open(domfile+addon, True) should have thrown and exception and it didn't")
        except: pass

        #################
        # test create
        #################
        if os.path.exists(os.path.join(scatest.getSdrPath(), devlocalnewfile)):
            os.remove(os.path.join(scatest.getSdrPath(), devlocalnewfile))

        try:
            myfile = fileMgr.create(devnewfile)
            myfile.write(str_to_write_dev)
            myfile.close()
        except:
            self.fail("Something bad happened in fileMgr.create(devnewfile) for mountPoint lookup")
        self.assertEqual(fileMgr.exists(devnewfile), True)

        if os.path.exists(os.path.join(scatest.getSdrPath(), domlocalnewfile)):
            os.remove(os.path.join(scatest.getSdrPath(), domlocalnewfile))

        try:
            myfile = fileMgr.create(domnewfile)
            myfile.write(str_to_write_dom)
            myfile.close()
        except:
            self.fail("Something bad happened in fileMgr.create(domnewfile) for domain (local fs) lookup")
        self.assertEqual(fileMgr.exists(domnewfile), True)

        #################
        # test remove
        #################
        try:
            fileMgr.remove(devnewfile)
        except:
            self.fail("Something bad happened in fileMgr.remove(devnewfile) for mountPoint lookup")
        self.assertEqual(fileMgr.exists(devnewfile), False)

        try:
            fileMgr.remove(domnewfile)
        except:
            self.fail("Something bad happened in fileMgr.remove(domnewfile) for domain (local fs) lookup")
        self.assertEqual(fileMgr.exists(domnewfile), False)

    def test_Copy(self):
        #################
        # test copy
        #################
        self.assertNotEqual(self._domMgr, None)
        fileMgr = self._domMgr._get_fileMgr()

        devfile = "/ExecutableDevice_node/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml"
        domfile = "/waveforms/CommandWrapper/CommandWrapper_DAS.xml"
        devcopyfiledst = "/ExecutableDevice_node/nodes/testcopy_dev_DeviceManager.dcd.xml"
        domcopyfiledst = "/waveforms/testcopy_dom_CommandWrapper_DAS.xml"
        domdevcopyfiledst = "/ExecutableDevice_node/nodes/testcopy_domdev_CommandWrapper_DAS.xml"
        devdomcopyfiledst = "/waveforms/testcopy_devdom_DeviceManager.dcd.xml"
        devbigfile = "/ExecutableDevice_node/nodes/bigfile_test.large"
        dombigfile = "/waveforms/bigfile_test.large"

        # test mounted fs to same mounted fs
        try:
            fileMgr.copy(devfile, devcopyfiledst)
        except:
            self.fail("Something bad happened in fileMgr.copy(devfile, devcopyfiledst) for mountPoint copy")
        self.assertEqual(fileMgr.exists(devcopyfiledst), True)
        self.assertEqual(self.filesEqual(devfile, devcopyfiledst, fileMgr), True)

        # test local fs to local fs
        try:
            fileMgr.copy(domfile, domcopyfiledst)
        except:
            self.fail("Something bad happened in fileMgr.copy(domfile, domcopyfiledst) for local copy")
        self.assertEqual(fileMgr.exists(domcopyfiledst), True)
        self.assertEqual(self.filesEqual(domfile, domcopyfiledst, fileMgr), True)

        # test local fs to mounted fs
        try:
            fileMgr.copy(domfile, domdevcopyfiledst)
        except:
            self.fail("Something bad happened in fileMgr.copy(domfile, domdevcopyfiledst) for local->mounted copy")
        self.assertEqual(fileMgr.exists(domdevcopyfiledst), True)
        self.assertEqual(self.filesEqual(domfile, domdevcopyfiledst, fileMgr), True)

        # test mounted fs to local fs
        try:
            fileMgr.copy(devfile, devdomcopyfiledst)
        except:
            self.fail("Something bad happened in fileMgr.copy(devfile, devdomcopyfiledst) for mounted->local copy")
        self.assertEqual(fileMgr.exists(devdomcopyfiledst), True)
        self.assertEqual(self.filesEqual(devfile, devdomcopyfiledst, fileMgr), True)

        # test overwrite of file within same fs
        self.assertEqual(self.filesEqual(domfile, devdomcopyfiledst, fileMgr), False) # make sure files are different
        try:
            fileMgr.copy(domfile, devdomcopyfiledst)
        except:
            self.fail("Something bad happened in fileMgr.copy(domfile, devdomcopyfiledst) for mounted->local copy (overwrite case)")
        self.assertEqual(self.filesEqual(domfile, devdomcopyfiledst, fileMgr), True) # make sure files are the same

        # test overwrite of file between two filesystems
        self.assertEqual(self.filesEqual(devdomcopyfiledst, devcopyfiledst, fileMgr), False) # make sure files are different
        try:
            fileMgr.copy(devdomcopyfiledst, devcopyfiledst)
        except:
            self.fail("Something bad happened in fileMgr.copy(devdomcopyfiledst, devcopyfiledst) for mounted->local copy (overwrite case)")
        self.assertEqual(self.filesEqual(devdomcopyfiledst, devcopyfiledst, fileMgr), True) # make sure files are the same

        # test large file transfers

        # create the large file
        bigfile = fileMgr.create(devbigfile)
        datastr = 'A'*1024*100
        for x in range(100):
            bigfile.write(datastr)
        bigfile.close()

        # now copy it
        try:
            fileMgr.copy(devbigfile, dombigfile)
        except:
            self.fail("Something bad happened in fileMgr.copy(devbigfile, dombigfile) for big file case")
        self.assertEqual(self.filesEqual(devdomcopyfiledst, devcopyfiledst, fileMgr), True) # make sure

        # Clean up
        fileMgr.remove(devcopyfiledst)
        fileMgr.remove(domcopyfiledst)
        fileMgr.remove(domdevcopyfiledst)
        fileMgr.remove(devdomcopyfiledst)
        fileMgr.remove(devbigfile)
        fileMgr.remove(dombigfile)

    def filesEqual(self, file1, file2, fileMgr):
        # read file1
        myfile1 = fileMgr.open(file1, True)
        file_data1 = myfile1.read(myfile1.sizeOf())
        # read file2
        myfile2 = fileMgr.open(file2, True)
        file_data2 = myfile2.read(myfile2.sizeOf())
        # close files
        myfile1.close()
        myfile2.close()

        return file_data1 == file_data2

    def test_fileIORCountDomain(self):
        # read file1
        domfile = "/waveforms/CommandWrapper/CommandWrapper_DAS.xml"
        fileMgr = self._domMgr._get_fileMgr()
        myfile1_1 = fileMgr.open(domfile, True)
        myfile1_2 = fileMgr.open(domfile, True)

        fileList = fileMgr.list(domfile)
        IORs = []
        for file in fileList:
            for prop in file.fileProperties:
                if prop.id == 'IOR_AVAILABLE':
                    IORs = prop.value._v
        self.assertEqual(len(IORs), 2)

        # close files
        myfile1_1.close()

        fileList = fileMgr.list(domfile)
        IORs = []
        for file in fileList:
            for prop in file.fileProperties:
                if prop.id == 'IOR_AVAILABLE':
                    IORs = prop.value._v
        self.assertEqual(len(IORs), 1)

        myfile1_2.close()

        fileList = fileMgr.list(domfile)
        IORs = []
        for file in fileList:
            for prop in file.fileProperties:
                if prop.id == 'IOR_AVAILABLE':
                    IORs = prop.value._v
        self.assertEqual(len(IORs), 0)

    def test_fileIORCountDevMgr(self):
        # read file1
        devfileFromDomain = "/ExecutableDevice_node/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml"
        devfile = "/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml"
        fileMgr = self._domMgr._get_fileMgr()
        fileSys = self._devMgr._get_fileSys()
        myfile1_1 = fileSys.open(devfile, True)
        myfile1_2 = fileMgr.open(devfileFromDomain, True)

        fileList = fileSys.list(devfile)
        IORs = []
        for file in fileList:
            for prop in file.fileProperties:
                if prop.id == 'IOR_AVAILABLE':
                    IORs = prop.value._v
        self.assertEqual(len(IORs), 2)

        myfile1_1.close()

        fileList = fileSys.list(devfile)
        IORs = []
        for file in fileList:
            for prop in file.fileProperties:
                if prop.id == 'IOR_AVAILABLE':
                    IORs = prop.value._v
        self.assertEqual(len(IORs), 1)

        myfile1_2.close()

        fileList = fileSys.list(devfile)
        IORs = []
        for file in fileList:
            for prop in file.fileProperties:
                if prop.id == 'IOR_AVAILABLE':
                    IORs = prop.value._v
        self.assertEqual(len(IORs), 0)

    def test_List(self):
        #################
        # test list
        #################

        self.assertNotEqual(self._domMgr, None)
        fileMgr = self._domMgr._get_fileMgr()

        devlistdir = "/ExecutableDevice_node/nodes/test_ExecutableDevice_node"
        domlistdir = "/waveforms/CommandWrapper"

        # use python to count number of regular files (non hidden) that are in the directory
        dir_file_list = [x for x in os.listdir(devlistdir.replace('/ExecutableDevice_node','sdr/dev')) if x[0] != '.']
        dir_file_list_xml = [x for x in dir_file_list if os.path.splitext(x)[1] == '.xml']
        self.assertEqual(len(dir_file_list_xml), 1)

        dir_file_list_hidden = [x for x in os.listdir(devlistdir.replace('/ExecutableDevice_node','sdr/dev')+'/.hidden_test') if x[0] != '.']
        dir_file_list_hidden.sort()

        #################
        # Test list
        #################

        # Test that lists of "" produce only a single entry to '/'
        rootFiles = self._devMgr._get_fileSys().list("")
        self.assertEqual(len(rootFiles), 1)
        self.assertEqual(rootFiles[0].name, "/")
        self.assertEqual(rootFiles[0].kind, CF.FileSystem.DIRECTORY)

        rootFiles = fileMgr.list("")
        self.assertEqual(len(rootFiles), 1)
        self.assertEqual(rootFiles[0].name, "/")
        self.assertEqual(rootFiles[0].kind, CF.FileSystem.DIRECTORY)


        # Test query of '/'.  It should show the contents of '/'
        rootFiles = self._devMgr._get_fileSys().list("/")
        # Compare the length against the local directory (ignoring hidden files)
        localFiles = filter(lambda x: not x.startswith('.'), os.listdir("sdr/dev"))
        self.assertEqual(len(rootFiles), len(localFiles))
        files = {}
        for fi in rootFiles:
            files[fi.name] = fi
        self.assert_(files.has_key("nodes"))
        self.assert_(files.has_key("devices"))
        self.assert_(files.has_key("mgr"))

        rootFiles = fileMgr.list("/")
        files = {}
        for fi in rootFiles:
            files[fi.name] = fi

        # Test a few expected directories, but don't be exhaustive
        self.assert_(files.has_key("waveforms"))
        self.assertEqual(files["waveforms"].kind, CF.FileSystem.DIRECTORY)
        self.assert_(files.has_key("components"))
        self.assertEqual(files["components"].kind, CF.FileSystem.DIRECTORY)

        # Check that the DeviceManager mount point is listed
        self.assert_(files.has_key("ExecutableDevice_node"))
        self.assertEqual(files["ExecutableDevice_node"].kind, CF.FileSystem.FILE_SYSTEM)

        os.system("chmod 444 "+devlistdir.replace('/ExecutableDevice_node','sdr/dev')+'/'+dir_file_list_xml[0])

        searchpattern = devlistdir + "/*.xml"
        try:
            filelist = fileMgr.list(searchpattern)
        except:
            self.fail("Something bad happened in fileMgr.list(searchpattern) for '/*.xml' case")
        self.assertEqual(len(filelist), len(dir_file_list_xml))
        new_file_list = [x.name for x in filelist]
        new_file_list.sort()
        self.assertEqual(dir_file_list_xml, new_file_list)

        foundMatch = False
        for entry in filelist:
            if entry.name == new_file_list[0]:
                for prop in entry.fileProperties:
                    if prop.id == "READ_ONLY":
                        self.assertEqual(prop.value._v, True)
                        foundMatch = True
                        break

        self.assertEqual(foundMatch, True)

        os.system("chmod 664 "+devlistdir.replace('/ExecutableDevice_node','sdr/dev')+'/'+dir_file_list_xml[0])

        searchpattern = devlistdir + "/"
        try:
            filelist = fileMgr.list(searchpattern)
        except:
            self.fail("Something bad happened in fileMgr.list(searchpattern) for '/' case")
        self.assertEqual(len(filelist), len(dir_file_list))
        new_file_list = [x.name for x in filelist]
        new_file_list.sort()
        dir_file_list.sort()
        self.assertEqual(dir_file_list, new_file_list)

        searchpattern = devlistdir
        try:
            filelist = fileMgr.list(searchpattern)
        except:
            self.fail("Something bad happened in fileMgr.list(searchpattern) for 'directory name' case")
        self.assertEqual(len(filelist), 1)
        self.assertEqual(os.path.basename(searchpattern), filelist[0].name)

        searchpattern = devlistdir + "/.hidden_test"
        try:
            filelist = fileMgr.list(searchpattern)
        except:
            self.fail("Something bad happened in fileMgr.list(searchpattern) for '/.hidden_test directory name' case")
        self.assertEqual(len(filelist), 1)
        self.assertEqual(os.path.basename(searchpattern), filelist[0].name)

        searchpattern = devlistdir + "/.hidden_test/"
        try:
            filelist = fileMgr.list(searchpattern)
        except:
            self.fail("Something bad happened in fileMgr.list(searchpattern) for '/.hidden_test/' case")
        self.assertEqual(len(filelist), len(dir_file_list_hidden))
        new_file_list = [x.name for x in filelist]
        new_file_list.sort()
        self.assertEqual(dir_file_list_hidden, new_file_list)

    def test_DirectoryOperation(self):
        #################
        # test mkdir
        #################

        self.assertNotEqual(self._domMgr, None)
        fileMgr = self._domMgr._get_fileMgr()

        domdir = '/mkdir_test_directory_tmp_local'
        localdomdir = 'dom/mkdir_test_directory_tmp_local'
        devdir = "/DeviceManager/mkdir_test_directory_tmp_mounted"
        localdevdir = "dev/mkdir_test_directory_tmp_mounted"
        domdir_nestedbase = '/mkdir_test_directory_tmp_nested1'
        localdomdir_nestedbase = 'dom/mkdir_test_directory_tmp_nested1'
        devdir_nestedbase = '/DeviceManager/mkdir_test_directory_tmp_nested1'
        localdevdir_nestedbase = 'dev/mkdir_test_directory_tmp_nested1'
        tmpdevfile = '/DeviceManager/mkdir_test_directory_tmp_nested1/foo.txt'
        tmpdevfilenested = '/DeviceManager/mkdir_test_directory_tmp_nested1/nested2/foo.txt'

        # test local mkdir
        if os.path.exists(os.path.join(scatest.getSdrPath(), localdomdir)):
            os.rmdir(os.path.join(scatest.getSdrPath(), localdomdir))
        try:
            fileMgr.mkdir(domdir)
        except:
            self.fail("Something bad happened in fileMgr.mkdir(newdir_local)")
        self.assertEqual(fileMgr.exists(domdir), True)

        # test mounted mkdir
        if os.path.exists(os.path.join(scatest.getSdrPath(), localdevdir)):
            os.rmdir(os.path.join(scatest.getSdrPath(), localdevdir))
        try:
            fileMgr.mkdir(devdir)
        except:
            self.fail("Something bad happened in fileMgr.mkdir(newdir_mounted)")
        self.assertEqual(fileMgr.exists(devdir), True)

        # test nested mkdir local
        if os.path.exists(os.path.join(scatest.getSdrPath(), localdomdir_nestedbase + '/nested2')):
            os.rmdir(os.path.join(scatest.getSdrPath(), localdomdir_nestedbase + '/nested2'))
        if os.path.exists(os.path.join(scatest.getSdrPath(), localdomdir_nestedbase)):
            os.rmdir(os.path.join(scatest.getSdrPath(), localdomdir_nestedbase))
        try:
            fileMgr.mkdir(domdir_nestedbase + '/nested2')
        except:
            self.fail("Something bad happened in fileMgr.mkdir(domdir_nestedbase)")
        self.assertEqual(fileMgr.exists(domdir_nestedbase + '/nested2'), True)

        # test nested mkdir mounted
        # also test trailing '/'
        if os.path.exists(os.path.join(scatest.getSdrPath(), localdevdir_nestedbase + '/nested2')):
            os.rmdir(os.path.join(scatest.getSdrPath(), localdevdir_nestedbase + '/nested2'))
        if os.path.exists(os.path.join(scatest.getSdrPath(), localdevdir_nestedbase)):
            os.rmdir(os.path.join(scatest.getSdrPath(), localdevdir_nestedbase))
        try:
            fileMgr.mkdir(devdir_nestedbase + '/nested2/')
        except:
            self.fail("Something bad happened in fileMgr.mkdir(devdir_nestedbase)")
        self.assertEqual(fileMgr.exists(devdir_nestedbase + '/nested2'), True)

        #################
        # test rmdir
        #################

        #remove empty directory
        try:
            fileMgr.rmdir(domdir)
            fileMgr.rmdir(devdir)
        except:
            self.fail("Something bad happened in fileMgr.rmdir()")
        self.assertEqual(fileMgr.exists(domdir), False)
        self.assertEqual(fileMgr.exists(devdir), False)

        #remove nested empty directories
        try:
            fileMgr.rmdir(domdir_nestedbase)
        except:
            self.fail("Something bad happened in fileMgr.rmdir(domdir_nestedbase)")
        self.assertEqual(fileMgr.exists(domdir_nestedbase + '/nested2'), False)
        self.assertEqual(fileMgr.exists(domdir_nestedbase), False)

        #make sure remove doesn't happen if files are in directory
        newfile = fileMgr.create(tmpdevfile)
        newfile.close()
        try:
            fileMgr.rmdir(devdir_nestedbase)
            self.fail("rmdir should not have worked. Directory not empty")
        except:
            self.assertEqual(fileMgr.exists(devdir_nestedbase + '/nested2'), True)
        fileMgr.remove(tmpdevfile)
        self.assertEqual(fileMgr.exists(tmpdevfile), False)

        #same thing as above, but in a nested directory
        newfile = fileMgr.create(tmpdevfilenested)
        newfile.close()
        try:
            fileMgr.rmdir(devdir_nestedbase)
            self.fail("rmdir should not have worked. Directory not empty")
        except:
            self.assertEqual(fileMgr.exists(devdir_nestedbase + '/nested2'), True)
        fileMgr.remove(tmpdevfilenested)
        self.assertEqual(fileMgr.exists(tmpdevfilenested), False)

        #remove the mounted nested directories
        try:
            fileMgr.rmdir(devdir_nestedbase)
        except:
            self.fail("Something bad happened in fileMgr.rmdir(devdir_nestedbase)")
        self.assertEqual(fileMgr.exists(devdir_nestedbase + '/nested2'), False)
        self.assertEqual(fileMgr.exists(devdir_nestedbase), False)

        # make sure error is thrown if trying to remove a directory that doesn't exists
        try:
            fileMgr.rmdir(domdir)
            self.fail("mdir should not have worked. Directory does not exist")
        except:
            self.assertEqual(fileMgr.exists(domdir), False)

    def test_Move(self):
        #################
        # test move
        #################
        self.assertNotEqual(self._domMgr, None)
        fileMgr = self._domMgr._get_fileMgr()

        # Test move on the same file system
        original = os.path.join(scatest.getSdrPath(), "dom/mgr/DomainManager.spd.xml")
        domoldfile = "/components/old.txt"
        domnewfile = "/waveforms/new.txt"
        devfile = "/ExecutableDevice_node/nodes/dev.txt"
        domlocaloldfile = os.path.join(scatest.getSdrPath(), 'dom' + domoldfile)
        domlocalnewfile = os.path.join(scatest.getSdrPath(), 'dom' + domnewfile)
        devlocalfile = os.path.join(scatest.getSdrPath(), "dev/nodes/dev.txt")

        shutil.copyfile(original, domlocaloldfile)
        if os.path.exists(domlocalnewfile):
            os.remove(domlocalnewfile)

        try:
            fileMgr.move(domoldfile, domnewfile)
        except:
            self.fail("Exception in local move")
        self.assertEqual(fileMgr.exists(domoldfile), False)
        self.assertEqual(fileMgr.exists(domnewfile), True)

        self.assertEqual(file(original).read(), file(domlocalnewfile).read())

        # Test move across file systems
        if os.path.exists(devlocalfile):
            os.remove(devlocalfile)
        try:
            fileMgr.move(domnewfile, devfile)
        except:
            self.fail("Exception in local->remote move")
        self.assertEqual(fileMgr.exists(domnewfile), False)
        self.assertEqual(fileMgr.exists(devfile), True)

        self.assertEqual(file(original).read(), file(devlocalfile).read())

        os.remove(devlocalfile)

    def test_EmptyFilename(self):
        #################
        # test move
        #################
        self.assertNotEqual(self._domMgr, None)
        fileMgr = self._domMgr._get_fileMgr()
        
        self.assertRaises(CF.InvalidFileName, fileMgr.open, '', True)

    def test_readException(self):
        # Makes sure that File_impl::read() throws correct exception and doesn't kill domain
        # Issue #533
        self.assertRaises(CF.DomainManager.ApplicationInstallationError, self._domMgr.installApplication, '/waveforms')

