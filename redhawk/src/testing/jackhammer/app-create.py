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

from ossie.cf import CF
import ossie.parsers.sad

import jackhammer

class CreateApp(jackhammer.Jackhammer):
    def initialize (self, sadFile):
        try:
            self.domMgr.installApplication(sadFile)
        except CF.DomainManager.ApplicationAlreadyInstalled:
            pass
        domRoot = os.path.join(os.environ["SDRROOT"], "dom")
        sad = ossie.parsers.sad.parse(domRoot + sadFile)
        id = sad.get_id()
        for appFact in self.domMgr._get_applicationFactories():
            if appFact._get_identifier() == id:
                self.appFact = appFact
                return

        raise KeyError, "Couldn't find app factory"

    def test (self):
        app = self.appFact.create(self.appFact._get_name(), [], [])
        app.stop()
        app.releaseObject()


if __name__ == '__main__':
    jackhammer.run(CreateApp)
