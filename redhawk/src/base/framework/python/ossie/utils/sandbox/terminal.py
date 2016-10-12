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

import commands

class Terminal(object):
    def __init__(self, command):
        status, self.__command = commands.getstatusoutput('which '+command)
        if status:
            raise RuntimeError, command + ' cannot be found'

    def _termOpts(self):
        return []

    def command(self, command, arguments, title=None):
        options = self._termOpts()
        if title:
            options += self._titleArgs(title)
        return self.__command, options + self._execArgs(command, arguments)

class XTerm(Terminal):
    def __init__(self):
        super(XTerm,self).__init__('xterm')

    def _titleArgs(self, title):
        return ['-T', title]

    def _execArgs(self, command, arguments):
        return ['-e', command] + arguments

class GnomeTerm(Terminal):
    def __init__(self):
        super(GnomeTerm,self).__init__('gnome-terminal')

    def _titleArgs(self, title):
        return ['-t', title]

    def _execArgs(self, command, arguments):
        return ['-x', command] + arguments
