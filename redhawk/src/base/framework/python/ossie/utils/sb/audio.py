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

import struct
import threading
import time

def _deferred_imports():
    # The GStreamer (or bulkio) modules may not be installed; defer imports
    # until creation of a SoundSink to report the error only when it's relevant
    try:
        import pygst
        pygst.require('0.10')
        import gst

        from bulkio.bulkioInterfaces import BULKIO__POA

        def _deferred_imports():
            pass

        globals().update(locals())
    except ImportError, e:
        raise RuntimeError("Missing required package for sandbox audio: '%s'" % e)

from ossie.utils.log4py import logging

from io_helpers import _SinkBase

__all__ = ('SoundSink',)

class SoundSink(_SinkBase):
    """
    Sink for audio playback of BULKIO streams. Supports a single source.

    Data is resampled to the preferred sample rate of the ALSA playback device.
    For proper playback, the content must be in the audio range (~20KHz).
    """

    # Determine GStreamer endianness by unpacking the little-endian 32-bit
    # string '1234' and repacking it into a native endian 32-bit integer. If the
    # native endianness is little, the packed value will be '1234', otherwise
    # it will be swapped to '4321'.
    ENDIANNESS = struct.pack('@i', struct.unpack('<i', '1234')[0])

    def __init__(self):
        """
        Create a new audio sink.
        """
        _deferred_imports()
        _SinkBase.__init__(self, formats=('float', 'double', 'char', 'octet', 'short', 'ushort', 'long', 'ulong'))
        self.sample_rate = None
        self.datatype = None

        self.pipeline = gst.Pipeline(self._instanceName)

        # Create an AppSrc to push data in; the format will be determined later
        self.source = gst.element_factory_make('appsrc', 'source')
        self.pipeline.add(self.source)

        # Create sample rate and format converters to help with connecting
        converter = gst.element_factory_make('audioconvert', 'converter')
        resampler = gst.element_factory_make('audioresample', 'resampler')
        self.pipeline.add(converter)
        self.pipeline.add(resampler)

        # Create an ALSA sink for output
        sink = gst.element_factory_make("alsasink", "sink")
        self.pipeline.add(sink)

        self.source.link(converter)
        converter.link(resampler)
        resampler.link(sink)

    def getPort(self, name):
        if self._sink:
            if name != self._sinkName:
                raise RuntimeError("SoundSink only supports one port at a time (using '%s')" % self._sinkName)
        else:
            port = self.getPortByName(name)

            # Find the POA class in the BULKIO__POA module (the namespace is
            # assumed, so split off the class name)
            poa_class = getattr(BULKIO__POA, port['portType'].split('.')[-1])

            # Create a correctly-typed POA adapter to forward CORBA calls to
            # this object
            self._sink = poa_class()
            self._sink.pushPacket = self._pushPacket
            self._sink.pushSRI = self._pushSRI

            self.datatype = port['format']
            self._sinkName = name

        return self._sink._this()

    def releaseObject(self):
        super(SoundSink,self).releaseObject()
        poa = self._sink._default_POA()
        try:
            poa.deactivate_object(poa.servant_to_id(self._sink))
        except:
            pass
        self._sink = None

    def start(self):
        super(SoundSink,self).start()
        self.breakBlock = False
        self.pipeline.set_state(gst.STATE_PLAYING)

    def stop(self):
        super(SoundSink,self).stop()
        self.pipeline.set_state(gst.STATE_PAUSED)

    def _getCaps(self):
        bits = struct.calcsize(self.datatype) * 8
        if self.datatype in ('f', 'd'):
            return gst.Caps('audio/x-raw-float,endianness=%s,width=%d,rate=%d,channels=1' % (self.ENDIANNESS, bits, self.sample_rate))
        else:
            # In struct module, unsigned types are uppercase, signed are lower
            if self.datatype.isupper():
                signed = 'false'
            else:
                signed = 'true'
            return gst.Caps('audio/x-raw-int,endianness=%s,signed=%s,width=%d,depth=%d,rate=%d,channels=1' % (self.ENDIANNESS, signed, bits, bits, self.sample_rate))

    def _pushSRI(self, sri):
        sample_rate = int(1/sri.xdelta)
        if sample_rate != self.sample_rate:
            # Sample rate changed, pause playback and update caps
            self.sample_rate = sample_rate
            self.pipeline.set_state(gst.STATE_PAUSED)
            self.source.set_property('caps', self._getCaps())
            self.pipeline.set_state(gst.STATE_PLAYING)

    def _pushPacket(self, data, timestamp, EOS, streamId):
        if self.breakBlock:
            return

        if self.datatype not in ('b', 'B'):
            # dataChar and dataOctet already provide data as string; all
            # other formats require packing
            format = self.datatype
            data = struct.pack('%d%s' % (len(data), format), *data)
        self.source.emit('push_buffer', gst.Buffer(data))
