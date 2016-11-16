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
import sys
import stat
import tempfile
import shutil

from redhawk.codegen import utils
from redhawk.codegen import versions

from environment import CodegenEnvironment

class Generator(object):
    def __init__(
            self,
            outputdir,
            overwrite=False,
            crcs={},
            variant="",
            header=None,
            **options):

        self.outputdir = outputdir
        self.overwrite = overwrite
        if variant != "":
            variant = "_" + variant
        self.variant = variant
        self.parseopts(**options)

        # Read MD5 sums for testing file changes.
        self.md5file = os.path.join(self.outputdir, '.md5sums')
        self.md5sums = {}
        if os.path.exists(self.md5file):
            for line in open(self.md5file, 'r'):
                try:
                    digest, filename = line.rstrip().split(None, 1)
                    self.md5sums[filename] = digest
                except:
                    pass

        # Migrate legacy CRCs to MD5 sums. If the file is unchanged, calculate
        # the MD5; otherwise, continue to check the old CRC32.
        self.crcs = {}
        for filename in crcs:
            if filename in self.md5sums:
                # Assume MD5 sum is more recent
                continue
            # Check if the file has changed since the last CRC32.
            pathname = os.path.join(self.outputdir, filename)
            if not os.path.exists(pathname):
                # The file has been removed, and can be regenerated.
                continue
            if crcs[filename] == str(utils.fileCRC(pathname, stripnewlines=True)):
                # File is unchanged, calculate MD5 sum.
                self.md5sums[filename] = utils.fileMD5(pathname)
            else:
                self.crcs[filename] = crcs[filename]

        # Save the header (if given)
        self.header = header

    def parseopts(self):
        """
        Parse additional options passed to the constructor. Subclasses should
        implement this method if they have template-specific options.
        """
        pass

    def loader(self, component):
        raise NotImplementedError, 'CodeGenerator.loader'

    def templates(self, component):
        raise NotImplementedError, 'CodeGenerator.templates'

    def filenames(self, softpkg):
        component = self.map(softpkg)
        return [t.filename for t in self.templates(component)]

    def trackedFiles(self):
        """
        Returns the list of files tracked by MD5 sums or CRC32s (for legacy
        projects).
        """
        return self.md5sums.keys() + self.crcs.keys()

    def fileChanged(self, filename, userfile=False):
        if userfile:
            return True
        pathname = os.path.join(self.outputdir, filename)
        if not os.path.exists(pathname):
            return False
        lastHash = self.md5sums.get(filename, None)
        if lastHash is None and filename in self.crcs:
            lastHash = self.crcs[filename]
            currentHash = str(utils.fileCRC(pathname, stripnewlines=True))
        else:
            currentHash = utils.fileMD5(pathname)
        return lastHash != currentHash

    def fileExists(self, filename):
        pathname = os.path.join(self.outputdir, filename)
        return os.path.exists(pathname)

    def fileinfo(self, softpkg):
        """
        Return information about what files would be affected by generation.
        """
        component = self.map(softpkg)
        stale = set(self.trackedFiles())
        files = []
        # Check state of files that would be generated
        for template in self.templates(component):
            if template.filename in stale:
                stale.remove(template.filename)
            info = { 'filename': template.filename,
                     'user':     template.userfile,
                     'modified': self.fileChanged(template.filename, template.userfile),
                     'new':      not self.fileExists(template.filename),
                     'remove':   False }
            files.append(info)

        # Check for files that can be deleted
        for filename in stale:
            if self.fileExists(filename):
                info = { 'filename': filename,
                         'user':     False,
                         'modified': self.fileChanged(filename),
                         'new':      False,
                         'remove':   True}
                files.append(info)

        return files

    def _addHeader(self, gen, header):
        """
        """

    def generate(self, softpkg, *filenames):
        loader = self.loader(softpkg)

        # Map the component model into a language-specific version
        component = self.map(softpkg)

        generated = []
        skipped = []

        # Note all tracked files, for cleaning up stale files; if a file list
        # was given, only consider those files for deletion.
        stale = set(self.trackedFiles())
        if filenames:
            stale.intersection_update(filenames)

        for template in self.templates(component):
            # Mark the current template as seen so it will not be deleted,
            # regardless of whether it gets generated
            if template.filename in stale:
                stale.remove(template.filename)

            # If a file list was given, skip files not explicitly listed.
            if filenames and template.filename not in filenames:
                continue

            filename = os.path.join(self.outputdir, template.filename)

            if os.path.exists(filename):
                # Check if the file has been modified since last generation.
                if self.fileChanged(template.filename, template.userfile) and not self.overwrite:
                    skipped.append((template.filename, 'overwrite'))
                    continue
                action = ''
            else:
                action = '(added)'

            # Attempt to ensure that the full required path exists
            parentdir = os.path.dirname(filename)
            if parentdir and not os.path.isdir(parentdir):
                os.makedirs(parentdir)

            env = CodegenEnvironment(loader=loader, **template.options())
            env.filters.update(template.filters())
            tmpl = env.get_template(template.template)

            # Initially, write the output to a temporary file to avoid trashing
            # the original file if the template is malformed
            with tempfile.NamedTemporaryFile() as outfile:
                # Start with the template-specific context, then add the mapped
                # component and a reference to this generator with known names.
                context = template.context()
                context['component'] = component
                context['generator'] = self
                context['versions'] = versions

                # Evaluate the template in streaming mode (rather than all at
                # once)
                gen = tmpl.generate(**context)
                if self.header:
                    # Define a generator function to insert the header at the
                    # top of the file
                    def generate(gen, header):
                        first = True
                        for chunk in gen:
                            if first:
                                # Take "shebang" into account for executable
                                # scripts
                                if chunk.startswith('#!'):
                                    line, chunk = chunk.split('\n', 1)
                                    yield line + '\n'
                                yield header
                            first = False
                            yield chunk

                    # Wrap the template's generator with the header insertion
                    # generator
                    gen = generate(gen, template.comment(self.header))

                # Write the stream to the output file
                for chunk in gen:
                    outfile.write(chunk)

                # Add a trailing newline to work around a Jinja bug.
                outfile.write('\n')

                # Now that generation has succeeded, flush the temporary file
                # to ensure the contents are completer, and copy to the target
                # location
                outfile.file.flush()
                shutil.copy(outfile.name, filename)

                # Set the executable bit, if requested by the template.
                if template.executable:
                    st = os.stat(filename)
                    os.chmod(filename, st.st_mode|stat.S_IEXEC)

            generated.append((template.filename, action))

            # Update the MD5 digest
            self.md5sums[template.filename] = utils.fileMD5(filename)

        # Remove old files that were not (and would not have been) generated on
        # this pass, and are unchanged.
        for existing in stale:
            filename = os.path.join(self.outputdir, existing)
            if not os.path.exists(filename):
                continue

            # Check for changes, and require explicit action to remove.
            if self.fileChanged(existing) and not self.overwrite:
                skipped.append((existing, 'delete'))
                continue

            # Delete the file, and remove its MD5 sum (if it has one).
            os.unlink(filename)
            generated.append((existing, '(deleted)'))
            if existing in self.md5sums:
                del self.md5sums[existing]

        # Save updated MD5 digests
        # NB: To work with "md5sum -c", there must be two spaces between the
        #     MD5 digest and the filename.
        md5out = open(self.md5file, 'w')
        for name, digest in self.md5sums.items():
            print >>md5out, "%s  %s" % (digest, name)
        md5out.close()

        return generated, skipped

    def getOutputDir(self):
        return self.outputdir

    def relativeBasePath(self):
        return os.path.relpath('.', self.getOutputDir()) + '/'

class TopLevelGenerator(Generator):
    def __init__(self, **opts):
        super(TopLevelGenerator,self).__init__(**opts)
        self.generators = {}

    def projectMapper(self):
        raise NotImplementedError, 'TopLevelGenerator.projectMapper'

    def map(self, softpkg):
        return self.projectMapper().mapProject(softpkg, self.generators)

    def addImplGenerator(self, generator):
        self.generators[generator.implId] = generator

class CodeGenerator(Generator):
    def __init__(self, implId, **opts):
        super(CodeGenerator,self).__init__(**opts)
        self.implId = implId

    def componentMapper(self):
        raise NotImplementedError, 'CodeGenerator.componentMapper'

    def propertyMapper(self):
        raise NotImplementedError, 'CodeGenerator.propertyMapper'

    def portMapper(self):
        raise NotImplementedError, 'CodeGenerator.portMapper'

    def portFactory(self):
        raise NotImplementedError, 'CodeGenerator.portFactory'

    def map(self, softpkg):
        # Apply template-specific mapping for component.
        impl = softpkg.getImplementation(self.implId)
        compmapper = self.componentMapper()
        component = compmapper.mapComponent(softpkg)
        component['impl'] = compmapper.mapImplementation(impl)

        # If generator has a mapping for properties, apply that.
        propmapper = self.propertyMapper()
        if propmapper:
            properties = propmapper.mapProperties(softpkg)
            component.update(properties)

        # If generator has a mapping for ports, apply that.
        portmapper = self.portMapper()
        if portmapper:
            portfactory = self.portFactory()
            ports = self.portMapper().mapPorts(softpkg, portfactory)
            component.update(ports)

        return component

    def rpmRequires(self):
        return []

    def rpmBuildRequires(self):
        return []
