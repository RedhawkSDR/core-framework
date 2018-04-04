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
import threading
from _unitTestHelpers import scatest

from ossie.events import Subscriber
from ossie.cf import CF, StandardEvent

class ComponentTerminationTest(scatest.CorbaTestCase):
    def setUp(self):
        self.launchDomainManager()
        self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")

        self._event = threading.Event()
        self._subscriber = Subscriber(self._domainManager, 'IDM_Channel', self._idmMessageReceived)

    def tearDown(self):
        # Remove the subscriber
        self._subscriber.terminate()

        # Continue normal teardown
        scatest.CorbaTestCase.tearDown(self)

    def _idmMessageReceived(self, message):
        if message.typecode().equal(StandardEvent._tc_AbnormalComponentTerminationEventType):
            self._event.set()

    def _test_UnhandledException(self, lang):
        waveform_name = 'svc_fn_error_' + lang + '_w'
        sad_file = os.path.join('/waveforms', waveform_name, waveform_name + '.sad.xml')
        app = self._domainManager.createApplication(sad_file, waveform_name, [], [])

        try:
            app.start()
        except CF.Resource.StartError:
            # Python in particular will throw a CORBA exception if the start()
            # call has not finished when the exception handler terminates the
            # component. It doesn't matter for this test, so ignore it.
            pass

        self._event.wait(1)
        self.failUnless(self._event.isSet(), 'No unexpected termination message received')

    def test_UnhandledExceptionCpp(self):
        """
        Test that unhandled exceptions in C++ service function cause components
        to terminate abnormally.
        """
        self._test_UnhandledException('cpp')

    def test_UnhandledExceptionPy(self):
        """
        Test that unhandled exceptions in Python service function cause
        components to terminate abnormally.
        """
        self._test_UnhandledException('py')

    @scatest.requireJava
    def test_UnhandledExceptionJava(self):
        """
        Test that unhandled exceptions in Java service function cause
        components to terminate abnormally.
        """
        self._test_UnhandledException('java')
