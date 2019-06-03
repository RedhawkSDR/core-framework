#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK code-generator.
#
# REDHAWK code-generator is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK code-generator is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import subprocess
import os
import uuid

from ossie.parsers import spd

class SoftPackage(object):

    def __init__(
            self,
            name,
            implementation,
            outputDir=".",
            ):

        self.name = name
        self.implementation = implementation
        self.outputDir = outputDir
        self.autotoolsDir = self.outputDir+'/'+self.name+'/'+self.implementation+'/'
        self.type = "";
        # Create empty objects that can be populated by classes inheriting
        # from SoftPackage
        self.spd = None
        self.scd = None
        self.prf = None
        self.wavedevContent = None

    def _setNameInSpd(self):
        self.spd.id_ = 'DCE:%s' % uuid.uuid4()
        self.spd.name = self.name

    def runCompileRpm(self):
        process = subprocess.Popen(
            './build.sh rpm',
            shell=True,
            cwd=self.outputDir+'/'+self.name)
        process.wait()

    def runInstall(self):
        '''
        Use subprocess.Popen() to call ./reconf; ./configure; make install.

        '''

        # Popen calls are non-blocking.  Need to call process.wait() to make
        # sure each step is done before continuing.
        process = subprocess.Popen(
            './reconf',
            shell=True,
            cwd=self.autotoolsDir)
        process.wait()
        process = subprocess.Popen(
            './configure',
            shell=True,
            cwd=self.autotoolsDir)
        process.wait()
        process = subprocess.Popen(
            'make install',
            shell=True,
            cwd=self.autotoolsDir)
        process.wait()

    def callCodegen(self, force = False, variant = ""):
        """
        Format command line arguments and call redhawk-codegen.

        For example:

            $ redhawk-codegen -m foo1.m -m foo2.m -f /home/user/bar.spd.xml

        """
        self._preCodegen()

        codegenArgs = ["redhawk-codegen"]

        if force:
            codegenArgs.append("-f")

        if variant != "":
            codegenArgs.append("--variant=" + variant)
        codegenArgs.append(self.outputDir+"/"+self.name+"/"+self.name+".spd.xml")
        subprocess.call(codegenArgs)

    def _preCodegen(self):
        """
        Override to perform additional tasks prior to code generation.
        """
        pass

    def _createWavedevContent(self, generator):
        # TODO: replace this with an XML template
        self.wavedevContent='<?xml version="1.0" encoding="ASCII"?>\n'
        self.wavedevContent+='<codegen:WaveDevSettings xmi:version="2.0" xmlns:xmi="http://www.omg.org/XMI" xmlns:codegen="http://www.redhawk.gov/model/codegen">\n'
        self.wavedevContent+='<implSettings key="__IMPLEMENTATION">\n'
        self.wavedevContent+='<value outputDir="__IMPLEMENTATION" template="redhawk.codegen.jinja.__GENERATOR" generatorId="redhawk.codegen.jinja.__GENERATOR" primary="true"/>\n'
        self.wavedevContent+='</implSettings>\n'
        self.wavedevContent+='</codegen:WaveDevSettings>\n'
        self.wavedevContent = self.wavedevContent.replace("__GENERATOR", generator)
        self.wavedevContent = self.wavedevContent.replace("__IMPLEMENTATION", self.implementation)

    def writeWavedev(self, outputDir="."):
        '''
        Write the hidden .resource.wavedev file.

        '''
        print self.name
        self.createOutputDirIfNeeded()
        outfile=open(self.outputDir+"/"+self.name+"/."+ self.name+".wavedev", 'w')
        outfile.write(self.wavedevContent)
        outfile.close()

    def createOutputDirIfNeeded(self):
        if not os.path.exists(self.outputDir + "/" + self.name):
            os.makedirs(self.outputDir + "/" + self.name)

    def writeXML(self):
        '''
        Call methods to Write resource.spd.xml, resource.prf.xml,
        resource.scd.xml, and .resource.wavedev.

        '''

        if self.spd:
            self.writeSPD()
        if self.scd:
            self.writeSCD()
        if self.prf:
            self.writePRF()
        if self.wavedevContent:
            self.writeWavedev()

    def _writeXMLwithHeader(self, xmlObject, fileType, dtdName, name_=None):
        '''
        The xml files contain two header lines that are outside of the primary
        file element.  Write the two header lines followed by the primary file
        element to output file.

        '''

        outFile = open(self.outputDir+"/"+self.name+"/"+self.name+"."+fileType+".xml", 'w')
        outFile.write('<?xml version="1.0" encoding="UTF-8"?>\n')
        outFile.write('<!DOCTYPE _DTDNAME_ PUBLIC "-//JTRS//DTD SCA V2.2.2 SPD//EN" "_DTDNAME_.dtd">\n'.replace("_DTDNAME_", dtdName))
        if name_ == None:
            name_ = dtdName
        xmlObject.export(
            outfile = outFile,
            level = 0,
            pretty_print = True,
            name_ = name_)
        outFile.close()

    def writeSPD(self):
        self.createOutputDirIfNeeded()
        self._writeXMLwithHeader(self.spd, "spd", "softpkg", name_="softpkg")

    def writeSCD(self):
        self.createOutputDirIfNeeded()
        self._writeXMLwithHeader(self.scd, "scd", "softwarecomponent")

    def writePRF(self):
        self.createOutputDirIfNeeded()
        self._writeXMLwithHeader(self.prf, "prf", "properties")

    def addSoftPackageDependency(self, dep, arch="noarch", resolve_implref=False):
        dep_impls=None
        if resolve_implref:
            dep_impls=self._get_impls_from_spd( dep )
            if dep_impls is None:
                warn_msg="Warning: Dependency ", dep, " contains no implementations, defaulting to ", arch
        else:
            warn_msg="Warning: No dependency implementation resolution, defaulting to ", arch

        if dep_impls is None:
            # no implementations found print out a warning and continue
            print warn_msg
            for index in range(len(self.spd.implementation)):
                self.spd.implementation[index].add_dependency( self._make_dep_ref(dep, arch) )
        else:
            # for each new spd implementation, add the matching impl from the dep param
            for index in range(len(self.spd.implementation)):
                spd_impl = self.spd.implementation[index]
                impl_ids = self._find_matching_impls( spd_impl, dep_impls )
                if len(impl_ids) == 0 :
                    print "Warning: No matching dependency implementations for implementation id: ", spd_impl.get_id(),  " defaulting to ", arch
                    spd_impl.add_dependency( self._make_dep_ref(dep,arch) )
                else:
                    if len(impl_ids) > 1 :
                        print "Warning: Multiple dependency implementations found, ", impl_ids, ", remove unwanted dependencies from SPD file"
                    for impl_id in impl_ids:
                        spd_impl.add_dependency(self._make_dep_ref(dep, impl_id) )

    def _make_dep_ref(self, localfile, refid='noarch'):
        softpkgref = spd.softPkgRef(localfile=spd.localFile(name=localfile),
                                    implref=spd.implRef(refid=refid))
        dependency = spd.dependency(type_="runtime_requirements",
                                    softpkgref=softpkgref)
        return dependency


    def _find_matching_impls(self, s_impl, add_deps ):
        # get spd's and deps matching implementation
        m_impls=[]
        spd_oss = [ o.get_name() for o in  s_impl.get_os() ]
        spd_procs = [ p.get_name() for p in  s_impl.get_processor() ]

        for k,dep in add_deps.iteritems():
            d_oss = [ o.get_name() for o in  dep.get_os() ]
            d_procs = [ p.get_name() for p in  dep.get_processor() ]

            m_os= False if len(spd_oss) > 0 else True
            m_proc= False if len(spd_procs) > 0 else True
            # match dep os value is in new spd
            for o in d_oss:
                if o in spd_oss:
                    m_os=True

            # match proc value is in new spd
            for p in d_procs:
                if p in spd_procs:
                    m_proc=True

            if m_os and m_proc:
                m_impls.append( k )
        return m_impls

    def _get_impls_from_spd(self, dep_file ):
        # try and get dependency spd file
        sdr_root='/var/redhawk/sdr'
        if 'SDRROOT' in os.environ:
            sdr_root= os.path.abspath(os.environ['SDRROOT'])
        else:
            print "Warning: SDRROOT undefined, defaulting to ", sdr_root

        fname = os.path.join(sdr_root, 'dom', dep_file)
        if dep_file.startswith('/'):
            fname = os.path.join(sdr_root, 'dom', dep_file[1:])

        impls={}
        try:
            parser = spd.parse( fname )
        except:
            print "Warning: Unable to open dependency file: ", fname
            return None
        for x in parser.get_implementation():
            impls[x.get_id()] = x
        return impls
