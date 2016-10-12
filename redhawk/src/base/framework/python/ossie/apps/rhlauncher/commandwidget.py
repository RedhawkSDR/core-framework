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
from PyQt4 import QtCore, QtGui

import ui

class CommandWidget(QtGui.QWidget):
    def __init__(self, *args, **kwargs):
        super(CommandWidget,self).__init__(*args, **kwargs)
        ui.load('commandwidget.ui', self)

        self._process = QtCore.QProcess(self)
        QtCore.QObject.connect(self._process, QtCore.SIGNAL('readyReadStandardOutput()'), self.readStandardOutput)
        QtCore.QObject.connect(self._process, QtCore.SIGNAL('readyReadStandardError()'), self.readStandardError)
        QtCore.QObject.connect(self._process, QtCore.SIGNAL('finished(int,QProcess::ExitStatus)'), self.processExited)
        QtCore.QObject.connect(self._process, QtCore.SIGNAL('stateChanged(QProcess::ProcessState)'), self.processStateChanged)

    def executeCommand(self, command, args):
        if self.processActive():
            return
        self._process.start(command, args)

    def setConsoleColor(self, color):
        fmt = self.consoleView.currentCharFormat()
        fmt.setForeground(QtGui.QBrush(QtGui.QColor(color)))
        self.consoleView.setCurrentCharFormat(fmt)

    def readStandardOutput(self):
        text = str(self._process.readAllStandardOutput()).rstrip()
        self.setConsoleColor('black')
        self.consoleView.appendPlainText(text)

    def readStandardError(self):
        text = str(self._process.readAllStandardError()).rstrip()
        self.setConsoleColor('red')
        self.consoleView.appendPlainText(text)

    def processExited(self, exitCode, exitStatus):
        self.setConsoleColor('gray')
        if exitStatus == QtCore.QProcess.NormalExit:
            self.consoleView.appendPlainText('<exited with status %d>' % exitCode)
        else:
            self.consoleView.appendPlainText('<terminated>')

    def processStateChanged(self, state):
        if state == QtCore.QProcess.Running:
            self.terminateButton.setEnabled(True)
        else:
            self.terminateButton.setEnabled(False)

    def processActive(self):
        return self._process.state() != QtCore.QProcess.NotRunning

    def terminateProcess(self):
        if not self.processActive():
            return
        self.terminateButton.setEnabled(False)
        self._process.terminate()
        QtCore.QTimer.singleShot(1000, self._checkTermination)

    def _checkTermination(self):
        if not self.processActive():
            return
        self._process.kill()

    def waitTermination(self, timeout=2000):
        if not self.processActive():
            return
        self._process.waitForFinished(timeout)
