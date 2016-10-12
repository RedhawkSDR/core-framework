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

from ossie import parsers
from _unitTestHelpers import scatest
import commands
import os
import tempfile

class PythonParserTestCase(scatest.OssieTestCase):
    def _xmllint(self, fPath, fType):
        os.environ['SGML_CATALOG_FILES'] = os.path.abspath("../xml/dtd/catalog.xml")
        docType = "-//JTRS//DTD SCA V2.2.2 %s//EN" % (fType.upper())
        cmd = "xmllint --nowarning --nonet --catalogs --noout --dropdtd --dtdvalidfpi '%s' %s" % (docType, fPath)
        status = commands.getstatusoutput(cmd)
        if status[0] != 0:
            print status[1]
        return status[0]

    def test_SPDParser(self):
        # Verify the input is valid
        spdPath = os.path.abspath("sdr/dom/components/CommandWrapper/CommandWrapper.spd.xml")
        status = self._xmllint(spdPath, "SPD")
        self.assertEqual(status, 0, "Input XML isn't DTD valid")

        spd = parsers.SPDParser.parse(spdPath)
        self.assertEqual(spd.get_id(), "DCE:458872f6-a316-4082-b1eb-ce5704f5c49d")
        self.assertEqual(spd.get_name(), "CommandWrapper")
        self.assertEqual(str(spd.get_author()[0].get_name()[0]), "REDHAWK test author")
        self.assertEqual(spd.get_propertyfile().get_type(), "PRF")
        self.assertEqual(spd.get_propertyfile().get_localfile().get_name(), "CommandWrapper.prf.xml")

        # Verify that we can write the output and still be DTD valid
        tmpfile = tempfile.mktemp()
        try:
            tmp = open(tmpfile, "w")
            spd.export(tmp, 0, name_="softpkg")
            tmp.close()
            status = self._xmllint(tmpfile, "SPD")
            self.assertEqual(status, 0, "Python parser did not emit DTD compliant XML")
        finally:
            try:
                os.remove(tmpfile)
            except OSError:
                pass

    def test_PRFParser(self):
        prf = parsers.PRFParser.parse("sdr/dom/components/CommandWrapper/CommandWrapper.prf.xml")
        props = {}
        for property in prf.get_simple():
            props[property.get_id()] = property
        for property in prf.get_simplesequence():
            props[property.get_id()] = property
        self.assertEqual(props["DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e"].get_mode(), "readwrite")
        self.assertEqual(props["DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e"].get_name(), "command")
        self.assertEqual(props["DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e"].get_type(), "string")
        self.assertEqual(props["DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e"].get_value(), "/bin/echo")
        self.assertEqual(props["DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e"].get_kind()[0].get_kindtype(), "configure")

        self.assertEqual(props["DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53"].get_mode(), "readwrite")
        self.assertEqual(props["DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53"].get_name(), "args")
        self.assertEqual(props["DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53"].get_type(), "string")
        self.assertEqual(props["DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53"].get_values().get_value()[0], "Hello World")
        self.assertEqual(props["DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53"].get_kind()[0].get_kindtype(), "configure")

        # Verify that we can write the output and still be DTD valid
        tmpfile = tempfile.mktemp()
        try:
            tmp = open(tmpfile, "w")
            prf.export(tmp, 0, name_="properties")
            tmp.close()
            status = self._xmllint(tmpfile, "PRF")
            self.assertEqual(status, 0, "Python parser did not emit DTD compliant XML")
        finally:
            try:
                os.remove(tmpfile)
            except OSError:
                pass

    def test_SCDParser(self):
        scd = parsers.SCDParser.parse("sdr/dom/components/CommandWrapper/CommandWrapper.scd.xml")
        self.assertEqual(scd.get_corbaversion(), "2.2")
        self.assertEqual(scd.get_componentrepid().get_repid(), "IDL:CF/Resource:1.0")
        self.assertEqual(scd.get_componenttype(), "resource")
        self.assertEqual(scd.get_componentfeatures().get_supportsinterface()[0].get_repid(), "IDL:CF/Resource:1.0")
        self.assertEqual(scd.get_componentfeatures().get_supportsinterface()[0].get_supportsname(), "Resource")
        self.assertEqual(scd.get_interfaces().get_interface()[0].get_name(), "Resource")
        self.assertEqual(scd.get_interfaces().get_interface()[0].get_inheritsinterface()[0].get_repid(), "IDL:CF/LifeCycle:1.0")

        # Verify that we can write the output and still be DTD valid
        tmpfile = tempfile.mktemp()
        try:
            tmp = open(tmpfile, "w")
            scd.export(tmp, 0)
            tmp.close()
            status = self._xmllint(tmpfile, "SCD")
            self.assertEqual(status, 0, "Python parser did not emit DTD compliant XML")
        finally:
            try:
                os.remove(tmpfile)
            except OSError:
                pass

    def test_DCDParser(self):
        dcd = parsers.DCDParser.parse("sdr/dev/nodes/test_MultipleExecutableDevice_node/DeviceManager.dcd.xml")
        self.assertEqual(dcd.get_id(), "DCE:d68b588e-5223-11db-9069-000d56d8556e")
        self.assertEqual(dcd.get_name(), "MultipleExecutableDevice_node")
        self.assertEqual(len(dcd.componentfiles.get_componentfile()), 1)
        self.assertEqual(len(dcd.partitioning.get_componentplacement()), 4)
        self.assertEqual(dcd.partitioning.get_componentplacement()[0].get_componentfileref().get_refid(), "ExecutableDevice1_c6e250b8-5223-11db-9fc4-000d56d8556e")
        self.assertEqual(dcd.partitioning.get_componentplacement()[0].get_componentinstantiation()[0].get_id(), "DCE:c21a39ba-6464-4cd6-92b4-cf462cfd4f16")
        self.assertEqual(dcd.partitioning.get_componentplacement()[0].get_componentinstantiation()[0].get_usagename(), "ExecutableDevice1")
        self.assertEqual(dcd.domainmanager.get_namingservice().get_name(), "DomainName1/DomainManager")

        # Verify that we can write the output and still be DTD valid
        tmpfile = tempfile.mktemp()
        try:
            tmp = open(tmpfile, "w")
            dcd.export(tmp, 0)
            tmp.close()
            status = self._xmllint(tmpfile, "DCD")
            self.assertEqual(status, 0, "Python parser did not emit DTD compliant XML")
        finally:
            try:
                os.remove(tmpfile)
            except OSError:
                pass


    def test_SADParser(self):
        sad = parsers.SADParser.parse("sdr/dom/waveforms/CommandWrapperWithPropertyOverride/CommandWrapper.sad.xml")
        self.assertEqual(sad.get_id(), "DCE:d206ab51-6342-4976-bac3-55e6902f3489")
        self.assertEqual(sad.get_name(), "CommandWrapperWithPropertyOverride")
        self.assertEqual(len(sad.componentfiles.get_componentfile()), 1)
        self.assertEqual(len(sad.partitioning.get_componentplacement()), 1)
        self.assertEqual(sad.partitioning.get_componentplacement()[0].componentfileref.refid, "CommandWrapper_592b8bd6-b011-4468-9417-705af45e907b")
        self.assertEqual(sad.partitioning.get_componentplacement()[0].get_componentinstantiation()[0].id_, "DCE:8c129782-a6a4-4095-8212-757f01de0c09")
        self.assertEqual(sad.partitioning.get_componentplacement()[0].get_componentinstantiation()[0].get_usagename(), "CommandWrapper1")
        self.assertEqual(sad.partitioning.get_componentplacement()[0].get_componentinstantiation()[0].componentproperties.get_simpleref()[0].refid, "DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e")
        self.assertEqual(sad.partitioning.get_componentplacement()[0].get_componentinstantiation()[0].componentproperties.get_simpleref()[0].value, "/bin/date")

        # Verify that we can write the output and still be DTD valid
        tmpfile = tempfile.mktemp()
        try:
            tmp = open(tmpfile, "w")
            sad.export(tmp, 0)
            tmp.close()
            status = self._xmllint(tmpfile, "SAD")
            self.assertEqual(status, 0, "Python parser did not emit DTD compliant XML")
        finally:
            try:
                os.remove(tmpfile)
            except OSError:
                pass

    def test_startorder(self):
        sad = parsers.SADParser.parse("sdr/dom/waveforms/CommandWrapperStartOrderTests/CommandWrapperWithOrder.sad.xml")
        self.assertEqual(sad.get_id(), "DCE:e6b136d5-6bf2-48ee-b2ec-52ceb9b80194")
        self.assertEqual(sad.get_name(), "CommandWrapperWithOrder")
        self.assertEqual(len(sad.componentfiles.get_componentfile()), 1)
        self.assertEqual(len(sad.partitioning.get_componentplacement()), 4)
        self.assertEqual(sad.partitioning.get_componentplacement()[0].componentfileref.refid, "CommandWrapperStartCounter_bfbfc18d-206a-4432-8a71-e219882abff2")
        self.assertEqual(sad.partitioning.get_componentplacement()[0].get_componentinstantiation()[0].id_, "myid1")
        self.assertEqual(sad.partitioning.get_componentplacement()[0].get_componentinstantiation()[0].startorder, None)
        self.assertEqual(sad.partitioning.get_componentplacement()[1].componentfileref.refid, "CommandWrapperStartCounter_bfbfc18d-206a-4432-8a71-e219882abff2")
        self.assertEqual(sad.partitioning.get_componentplacement()[1].get_componentinstantiation()[0].id_, "myid2")
        self.assertEqual(sad.partitioning.get_componentplacement()[1].get_componentinstantiation()[0].startorder, "1")
        self.assertEqual(sad.partitioning.get_componentplacement()[2].componentfileref.refid, "CommandWrapperStartCounter_bfbfc18d-206a-4432-8a71-e219882abff2")
        self.assertEqual(sad.partitioning.get_componentplacement()[2].get_componentinstantiation()[0].id_, "myid3")
        self.assertEqual(sad.partitioning.get_componentplacement()[2].get_componentinstantiation()[0].startorder, "2")
        self.assertEqual(sad.partitioning.get_componentplacement()[3].componentfileref.refid, "CommandWrapperStartCounter_bfbfc18d-206a-4432-8a71-e219882abff2")
        self.assertEqual(sad.partitioning.get_componentplacement()[3].get_componentinstantiation()[0].id_, "myid4")
        self.assertEqual(sad.partitioning.get_componentplacement()[3].get_componentinstantiation()[0].startorder, "3")


