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

import platform
import struct
import warnings

def _deferred_imports():
    # The GStreamer (or bulkio) modules may not be installed; defer imports
    # until creation of a SoundSink to report the error only when it's relevant
    def _deferred_imports():
        pass

    has_pygst = False
    has_gi = False
    try:
        import pygst
        has_pygst = True
    except ImportError:
        pass
    try:
        import gi
        has_gi = True
    except ImportError:
        pass

    try:
        # Prefer gstreamer v1 over v0.
        if has_gi:
            gi.require_version('Gst', '1.0')
            from gi.repository import GObject, Gst

            Gst.init(None)
        elif has_pygst:
            pygst.require('0.10')
            import gst
        else:  # Cause the ImportError
            os_major_version = _get_os_major_version()
            if os_major_version < 7:  # evaluates as True if os_major_version is None
                import pygst
            else:
                import gi

        from bulkio.bulkioInterfaces import BULKIO__POA

        globals().update(locals())
    except ImportError, e:
        raise RuntimeError("Missing required package for sandbox audio: '%s'" % e)

from io_helpers import _SinkBase

__all__ = ('SoundSink',)


def _get_os_major_version():
    distname, version, id_ = platform.linux_distribution()
    os_major_version = None
    if version and version[0].isdigit():
        os_major_version = int(version[0])
    return os_major_version


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
        if has_gi:  # use gstreamer v1.0
            self._create_pipeline = self._create_pipeline_v1
            self.play = self._play_v1
            self.pause = self._pause_v1
            self._pushPacket = self._push_packet_v1
            self._get_caps = self._get_caps_v1
        else:  # use gstreamer v0.10
            self._create_pipeline = self._create_pipeline_v0
            self.play = self._play_v0
            self.pause = self._pause_v0
            self._pushPacket = self._push_packet_v0
            self._get_caps = self._get_caps_v0
        self._create_pipeline()
        if has_gi:
            self.start()

    def _set_format_gstreamer(self, format_in):
        # Gst.Buffer.new_wrapped() raises an Exception when the caps string specifies S8
        # and the input data contains a negative number.  So, we translate S8 to S16.
        self.translate_char_to_short = False
        if format_in in ['char', 'charIn']:
            self.format_caps = 'S16'
            self.translate_char_to_short = True
        elif format_in in ['octet', 'octetIn']:
            self.format_caps = 'U8'
        elif format_in in ['short', 'shortIn']:
            self.format_caps = 'S16'
        elif format_in in ['ushort', 'ushortIn']:
            self.format_caps = 'U16'
        elif format_in in ['long', 'longIn']:
            self.format_caps = 'S32'
        elif format_in in ['ulong', 'ulongIn']:
            self.format_caps = 'U32'
        elif format_in in ['float', 'floatIn']:
            self.format_caps = 'F32'
        elif format_in in ['double', 'doubleIn']:
            self.format_caps = 'F64'

        if format_in not in ['octet', 'octetIn']:
            if self.ENDIANNESS == '1234':
                self.format_caps += 'LE'
            elif self.ENDIANNESS == '4321':
                self.format_caps += 'BE'

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
            self._set_format_gstreamer(port['portDict']['Port Name'])
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
        self.play()

    def stop(self):
        super(SoundSink,self).stop()
        self.pause()

    def _play_v0(self):
        self.pipeline.set_state(gst.STATE_PLAYING)

    def _play_v1(self):
        self.pipeline.set_state(Gst.State.PLAYING)

    def _pause_v0(self):
        self.pipeline.set_state(gst.STATE_PAUSED)

    def _pause_v1(self):
        self.pipeline.set_state(Gst.State.PAUSED)

    def _pushSRI(self, sri):
        if sri.xdelta <= 0:
            warnings.warn('sri.xdelta must be > 0; got:  {0}.  Setting sri.xdelta = 1.0.'.format(sri.xdelta))
            sri.xdelta = 1.0
        sample_rate = int(1/sri.xdelta)
        if sample_rate != self.sample_rate:
            # Sample rate changed, pause playback and update caps
            self.sample_rate = sample_rate
            self.pause()
            self.source.set_property('caps', self._get_caps())
            self.play()

    def _create_pipeline_v0(self):
        self.sample_rate = None

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

    def _create_pipeline_v1(self):
        self.sample_rate = None

        self.pipeline = Gst.Pipeline()

        # Create an AppSrc to push data in; the format will be determined later
        self.source = Gst.ElementFactory.make('appsrc', 'source')
        self.pipeline.add(self.source)

        # Create sample rate and format converters to help with connecting
        converter = Gst.ElementFactory.make('audioconvert', 'converter')
        resampler = Gst.ElementFactory.make('audioresample', 'resampler')
        self.pipeline.add(converter)
        self.pipeline.add(resampler)

        # Create an ALSA sink for output
        sink = Gst.ElementFactory.make("alsasink", "sink")
        self.pipeline.add(sink)

        self.source.link(converter)
        converter.link(resampler)
        resampler.link(sink)

    def _get_caps_v0(self):
        bits = struct.calcsize(self.datatype) * 8
        if self.datatype in ('f', 'd'):
            return gst.Caps('audio/x-raw-float, endianness=%s, width=%d, rate=%d, channels=1'
                            '' % (self.ENDIANNESS, bits, self.sample_rate))
        else:
            # In struct module, unsigned types are uppercase, signed are lower
            if self.datatype.isupper():
                signed = 'false'
            else:
                signed = 'true'
            return gst.Caps('audio/x-raw, endianness=%s, signed=%s, width=%d, depth=%d, rate=%d, channels=1'
                            '' % (self.ENDIANNESS, signed, bits, bits, self.sample_rate))

    def _get_caps_v1(self):
        caps_str = ('audio/x-raw, format={0}, channels=1, layout=interleaved, rate={1}'
                    ''.format(self.format_caps, self.sample_rate))
        caps = Gst.Caps.from_string(caps_str)
        return caps

    def _push_packet_v0(self, data, timestamp, EOS, streamId):
        if self.breakBlock:
            return

        if self.datatype not in ('b', 'B'):
            # dataChar and dataOctet provide data as string; other formats require packing
            format = self.datatype
            data = struct.pack('%d%s' % (len(data), format), *data)
        buf = gst.Buffer(data)
        self.source.emit('push_buffer', buf)

    def _push_packet_v1(self, data, timestamp, EOS, streamId):
        if self.breakBlock or not data:
            return

        if self.translate_char_to_short:
            data = struct.unpack('%d%s' % (len(data), self.datatype), data)
            S16_MAX = 2 ** (16 - 1) - 1
            S8_MAX = 2 ** (8 - 1) - 1
            mult = (S16_MAX + 0.0) / S8_MAX  # force floating point division
            data = [int(round(d * mult)) for d in data]

        dtype = self.datatype
        if dtype in ('b'):
            # char gets translated to short to accommodate Gst.Buffer.new_wrapped()
            dtype = 'h'
        if dtype not in ('B'):
            # dataChar and dataOctet provide data as string; other formats require packing
            data = struct.pack('%d%s' % (len(data), dtype), *data)

        buf = Gst.Buffer.new_wrapped(data)
        self.source.emit('push_buffer', buf)

