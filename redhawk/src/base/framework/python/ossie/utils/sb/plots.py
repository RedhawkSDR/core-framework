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

import threading
import time
import struct
import warnings

from omniORB.any import from_any

from ossie.utils.log4py import logging

def _deferred_imports():
    # Importing PyQt4 and matplotlib may take a long time--more than a second
    # worst case--but neither one is needed at import time (or possibly ever).
    # By deferring the import until required (at creation of a plot), the cost
    # is much less apparent.
    try:
        from PyQt4 import QtCore, QtGui

        import matplotlib
        matplotlib.use('Qt4Agg')
        from matplotlib import pyplot, mlab, pylab
        from matplotlib.backends.backend_agg import FigureCanvasAgg
        import numpy

        import bulkio
        from bulkio.bulkioInterfaces import BULKIO

        # Rebind the function to do nothing in future calls
        def _deferred_imports():
            pass

        globals().update(locals())
    except ImportError, e:
        import platform
        if 'el5' in platform.release() and 'PyQt4' in str(e):
            raise RuntimeError("matplotlib-based plots are not available by default on Red Hat Enterprise Linux 5 (missing PyQt4 dependency)")
        else:
            raise RuntimeError("Missing required package for sandbox plots: '%s'" % e)

from ossie.utils.model import PortSupplier
from ossie.utils.model.connect import PortEndpoint
from ossie.utils.sb import domainless

from ossie.utils.sandbox.helper import ThreadedSandboxHelper, ThreadStatus

__all__ = ('LinePlot', 'LinePSD', 'RasterPlot', 'RasterPSD', 'XYPlot')

log = logging.getLogger(__name__)

class PlotBase(ThreadedSandboxHelper):
    """
    Abstract base class for all matplotlib-based plots. Manages the provides
    port dictionary, the matplotlib figure, and the plot update thread.
    """
    def __init__(self):
        _deferred_imports()
        ThreadedSandboxHelper.__init__(self)

        # Use 1/10th of a second for sleeping when there are no updates
        self.setThreadDelay(0.1)

        # Create provides port dictionary.
        self._addProvidesPort('charIn', 'IDL:BULKIO/dataChar:1.0', bulkio.InCharPort)
        self._addProvidesPort('octetIn', 'IDL:BULKIO/dataOctet:1.0', bulkio.InOctetPort)
        self._addProvidesPort('shortIn', 'IDL:BULKIO/dataShort:1.0', bulkio.InShortPort)
        self._addProvidesPort('ushortIn', 'IDL:BULKIO/dataUshort:1.0', bulkio.InUShortPort)
        self._addProvidesPort('longIn', 'IDL:BULKIO/dataLong:1.0', bulkio.InLongPort)
        self._addProvidesPort('ulongIn', 'IDL:BULKIO/dataUlong:1.0', bulkio.InULongPort)
        self._addProvidesPort('longlongIn', 'IDL:BULKIO/dataLongLong:1.0', bulkio.InLongLongPort)
        self._addProvidesPort('ulonglongIn', 'IDL:BULKIO/dataUlongLong:1.0', bulkio.InULongLongPort)
        self._addProvidesPort('floatIn', 'IDL:BULKIO/dataFloat:1.0', bulkio.InFloatPort)
        self._addProvidesPort('doubleIn', 'IDL:BULKIO/dataDouble:1.0', bulkio.InDoublePort)
        self._addProvidesPort('bitIn', 'IDL:BULKIO/dataBit:1.0', bulkio.InBitPort)

        # Create a new figure and axes.
        self._figure = pyplot.figure()
        self._plot = self._figure.add_subplot(1, 1, 1)
        self._canvas = self._figure.canvas
        self._figure.show(False)

        # Let subclasses set up the axes.
        self._configureAxes()
        self._updateAxes()

        # Matplotlib 0.99 does not give an easy way to listen for window close
        # events; as a result, we have to get the underlying backend-specific
        # window. This is course, somewhat fragile. Furthermore, QMainWindow
        # does not provide a signal on close that would allow us to stop the
        # rendering loop before the objects are destroyed, so we dynamically
        # override the closeEvent method on this particular instance.
        window = self._getWindow()
        oldClose = window.closeEvent
        def closeEvent(event):
            self.stop()
            oldClose(event)
        window.closeEvent = closeEvent

        # Add thread synchronization to the draw and paintEvent methods on
        # FigureCanvasQTAgg, so that we can do drawing updates in the reader
        # thread at the same time that the main thread might traw to redraw.
        # Without synchronization, this leads to exceptions and flickering.
        lock = threading.Lock()
        oldDraw = self._canvas.draw
        def draw():
            with lock:
                oldDraw()
        self._canvas.draw = draw
        oldPaint = self._canvas.paintEvent
        def paintEvent(e):
            with lock:
                oldPaint(e)
        self._canvas.paintEvent = paintEvent
        self._renderLock = lock

    def _getWindow(self):
        for manager in pyplot._pylab_helpers.Gcf.get_all_fig_managers():
            if manager.num == self._figure.number:
                return manager.window
        return None

    def _redraw(self):
        """
        Redraw the plot and ask the widget to display the update.
        """
        # Perform the redraw with the render lock held, preventing the graphics
        # thread from trying to do a redraw at the same time.
        self._renderLock.acquire()
        try:
            # NB: In matplotlib 0.99, if you call the canvas' draw() method
            #     directly, it immediately redraws the canvas, but sets replot
            #     to true, leading to another redraw when the update is handled
            #     by the graphics thread.
            FigureCanvasAgg.draw(self._canvas)
            self._canvas.replot = False
            self._canvas.update()
        finally:
            self._renderLock.release()

    def _threadFunc(self):
        """
        Main method for plot update thread; do not call directly.
        """
        # Update the plot, unless there is nothing to do (i.e., not connected
        # to a data source).
        if self._update():
            self._redraw()
            return ThreadStatus.NORMAL
        else:
            return ThreadStatus.NOOP

    def close(self):
        """
        Close the plot.
        """
        if self._figure:
            pylab.close(self._figure)
            self._figure = None
            self._plot = None
            self._canvas = None

    def releaseObject(self):
        """
        Releases the plot and cleans up resources.
        """
        self.stop()
        super(PlotBase,self).releaseObject()
        self.close()

    def _getUnitLabel(self, units):
        return {
            BULKIO.UNITS_NONE:         '',
            BULKIO.UNITS_TIME:         'Time (sec)', 
            BULKIO.UNITS_DELAY:        'Delay (sec)',
            BULKIO.UNITS_FREQUENCY:    'Frequency (Hz)',
            BULKIO.UNITS_TIMECODE:     'Time code format',
            BULKIO.UNITS_DISTANCE:     'Distance (m)',
            BULKIO.UNITS_VELOCITY:     'Velocity (m/sec)',
            BULKIO.UNITS_ACCELERATION: 'Acceleration (m/sec^2)',
            BULKIO.UNITS_JERK:         'Jerk (m/sec^3)',
            BULKIO.UNITS_DOPPLER:      'Doppler (Hz)',
            BULKIO.UNITS_DOPPLERRATE:  'Doppler rate (Hz/sec)',
            BULKIO.UNITS_ENERGY:       'Energy (J)',
            BULKIO.UNITS_POWER:        'Power (W)',
            BULKIO.UNITS_MASS:         'Mass (g)'
        }.get(units, '')

    def _getXLabel(self):
        return self._getUnitLabel(self._getXUnits())

    def _getXUnits(self):
        return BULKIO.UNITS_NONE

    def _getYLabel(self):
        return self._getUnitLabel(self._getYUnits())

    def _getYUnits(self):
        return BULKIO.UNITS_NONE

    def _configureAxes(self):
        pass

    def _updateAxes(self):
        xlabel = self._getXLabel()
        self._plot.xaxis.set_label_text(xlabel)
            
        ylabel = self._getYLabel()
        self._plot.yaxis.set_label_text(ylabel)

class PlotEndpoint(PortEndpoint):
    def __init__(self, plot, port, connectionId):
        super(PlotEndpoint,self).__init__(plot, port)
        self.connectionId = connectionId

    def getReference(self):
        try:
            self.supplier._addTrace(self.port, self.connectionId)
        except KeyError:
            pass
        portname = '%s-%s' % (self.port['Port Name'], self.connectionId)
        return self.supplier.getPort(portname)


class LineBase(PlotBase):
    """
    Abstract base class for line plots, supporting multiple traces on the same
    plot. Each connection creates a new port to enable this behavior, although
    this behavior should be transparent to the sandbox user.
    """
    def __init__(self, frameSize, ymin, ymax):
        PlotBase.__init__(self)
        self._frameSize = frameSize
        self._ymin = ymin
        self._ymax = ymax
        self._plot.set_ylim(self._ymin, self._ymax)

        # Line state
        self._lines = {}
        self._linesLock = threading.Lock()

    def _getEndpoint(self, port, connectionId):
        if not port['Port Name'] in self._providesPortDict:
            raise RuntimeError, "Line plot '%s' has no provides port '%s'", (self._instanceName, name)
        return PlotEndpoint(self, port, connectionId)

    def _disconnected(self, connectionId):
        self._linesLock.acquire()
        try:
            for name, trace in self._lines.iteritems():
                if trace['id'] == connectionId:
                    trace['port'].stopPort()
                    line = trace['line']
                    line.remove()
                    del self._lines[name]
                    return
        finally:
            self._linesLock.release()

    def getPort(self, name):
        with self._linesLock:
            return self._lines[name]['port']._this()

    def _lineOptions(self):
        return {}

    def _addTrace(self, port, name):
        with self._linesLock:
            traceName = '%s-%s' % (port['Port Name'], name)
            if traceName in self._lines:
                raise KeyError, "Trace '%s' already exists" % traceName

            port = self._createPort(port, traceName)
            
            options = self._lineOptions()
            line, = self._plot.plot([], [], label=name, scalex=False, scaley=False, **options)
            trace = { 'port':   port,
                      'xdelta': None,
                      'line':   line,
                      'id':     name }
            self._lines[traceName] = trace
 
    def _updateTrace(self, trace):
        port = trace['port']
        line = trace['line']

        # Read next frame.
        stream = port.getCurrentStream(bulkio.const.NON_BLOCKING)
        if not stream:
            return False
        block = stream.read(self._frameSize)
        if not block:
            return False
        x_data, y_data = self._formatData(block, stream)
        line.set_data(x_data, y_data)

        # Check for new xdelta and update canvas if necessary.
        if block.sriChanged:
            trace['xdelta'] = block.xdelta
            xmin, xmax = self._getXRange(stream)
            self._plot.set_xlim(xmin, xmax)
        return True

    def _update(self):
        # Get a copy of the current set of lines to update, then release the
        # lock to do the reads. This allows the read to be interrupted (e.g.,
        # if a source is disconnected) without deadlock.
        with self._linesLock:
            traces = self._lines.values()
 
        redraw = [self._updateTrace(trace) for trace in traces]
        if not any(redraw):
            return False

        if self._ymin is None or self._ymax is None:
            self._plot.relim()
            if self._ymin is not None:
                ymin = self._ymin
            else:
                ymin = self._plot.dataLim.y0
            if self._ymax is not None:
                ymax = self._ymax
            else:
                ymax = self._plot.dataLim.y1
            # Update plot scale.
            self._plot.set_ylim(ymin, ymax)

        return True

    def _startHelper(self):
        log.debug("Starting line plot '%s'", self._instanceName)
        super(LineBase,self)._startHelper()
        # Start all associated ports.
        self._linesLock.acquire()
        try:
            for trace in self._lines.itervalues():
                trace['port'].startPort()
        finally:
            self._linesLock.release()

    def _stopHelper(self):
        log.debug("Stopping line plot '%s'", self._instanceName)
        # Stop all associated port.
        self._linesLock.acquire()
        try:
            for trace in self._lines.itervalues():
                trace['port'].stopPort()
        finally:
            self._linesLock.release()
        super(LineBase,self)._stopHelper()

    # Plot settings
    def _setYView(self, ymin, ymax):
        self._plot.set_ylim(ymin, ymax)
        self._redraw()

    def _setXView(self, xmin, xmax):
        self._plot.set_xlim(xmin, xmax)
        self._redraw()

    def _check_yrange(self, ymin, ymax):
        if ymin is None or ymax is None:
            return
        if ymax < ymin:
            raise ValueError, 'Y-axis bounds cannot overlap (%d > %d)' % (ymin, ymax)

    @property
    def ymin(self):
        """
        The lower bound of the Y-axis. If set to None, the lower bound will be
        determined automatically per-frame based on the data.
        """
        return self._ymin

    @ymin.setter
    def ymin(self, ymin):
        self._check_yrange(ymin, self._ymax)
        self._ymin = ymin
        self._setYView(self._ymin, self._ymax)

    @property
    def ymax(self):
        """
        The upper bound of the Y-axis. If set to None, the upper bound will be
        determined automatically per-frame based on the data.
        """
        return self._ymax

    @ymax.setter
    def ymax(self, ymax):
        self._check_yrange(self._ymin, ymax)
        self._ymax = ymax
        self._setYView(self._ymin, self._ymax)


class LinePlot(LineBase):
    """
    Line plot for time data, supporting multiple input sources. Each connection
    is drawn as its own colored line, and lines can be dynamically created or
    removed.
    """
    def __init__(self, frameSize=1024, ymin=None, ymax=None):
        """
        Create a new line plot.

        Arguments:
          frameSize  - Number of elements to draw per frame.
          ymin, ymax - Y-axis constraints. If not given, constraint will be
                       automatically calculated per-frame based on the data.
        """
        super(LinePlot,self).__init__(frameSize, ymin, ymax)
        self._complex = False

    def _getXUnits(self):
        return BULKIO.UNITS_TIME

    def _formatData(self, block, stream):
        # Bit blocks don't have a .complex attribute; default to False
        if getattr(block, 'complex', False):
            self._complex = True
            imag_data = block.buffer[1::2]   # y-axis: imaginary
            real_data  = block.buffer[::2] # x-axis: real
            self._xmin = min(real_data)
            self._xmax = max(real_data)
            self._plot.set_xlim(self._xmin, self._xmax)
            return real_data, imag_data
        else:
            data = block.buffer
            times = numpy.arange(len(data)) * block.xdelta
        return times, data

    def _getXRange(self, sri):
        if self._complex:
            return self._xmin, self._xmax
        else:
            return 0.0, (self._frameSize-1)*sri.xdelta


    def _check_xrange(self, xmin, xmax):
        if xmin is None or xmax is None:
            return
        if xmax < xmin:
            raise ValueError, 'X-axis bounds cannot overlap (%f > %f)' % (xmin, xmax)

    # Plot properties
    @property
    def xmin(self):
        """
        The lower bound of the X-axis.
        """
        return self._xmin

    @xmin.setter
    def xmin(self, xmin):
        self._check_xrange(xmin, self._xmax)
        self._xmin = xmin
        self._setXView(self._xmin, self._xmax)

    @property
    def xmax(self):
        """
        The upper bound of the X-axis.
        """
        return self._xmax

    @xmax.setter
    def xmax(self, xmax):
        self._check_xrange(self._xmin, xmax)
        self._xmax = xmax
        self._setXView(self._xmin, self._xmax)

class PSDBase(object):
    """
    Mix-in class for plots that the power spectral density (PSD) of input
    signal(s).
    """
    def __init__(self, nfft):
        self._nfft = nfft

    def _getFreqOffset(self, stream):
        if stream.hasKeyword('CHAN_RF'):
            return stream.getKeyword('CHAN_RF')
        elif stream.hasKeyword('COL_RF'):
            return stream.getKeyword('COL_RF')
        else:
            return 0.0

    def _psd(self, data, stream):
        y_data, x_data = mlab.psd(data, NFFT=self._nfft, Fs=self._getSampleRate(stream))
        offset = self._getFreqOffset(stream)
        if offset:
            x_data += offset
        return y_data, x_data

    def _getSampleRate(self, stream):
        if stream.xdelta > 0.0:
            # Round sample rate to an integral value to account for the fact
            # that there is typically some rounding error in the xdelta value.
            return round(1.0 / stream.xdelta)
        else:
            # Bad SRI xdelta, use normalized value.
            return 1.0

    def _getFreqRange(self, stream):
        nyquist = 0.5 * self._getSampleRate(stream)
        upper = nyquist
        if stream.complex:
            # Negative and positive frequencies.
            lower = -nyquist
        else:
            # Non-negative frequencies only.       
            lower = 0
        offset = self._getFreqOffset(stream)
        upper += offset
        lower += offset
        return lower, upper
        

class LinePSD(LineBase, PSDBase):
    """
    Line plot for displaying the power spectral density (PSD) of one or more
    input signals. Input data is padded to the FFT size, if necessary, and
    a Hanning window is applied before calculating the PSD.

    The Y-axis (magnitude) is displayed using a logarithmic scale.
    """
    def __init__(self, nfft=1024, frameSize=None, ymin=None, ymax=None):
        """
        Create a new line PSD plot.

        Arguments:

          nfft       - FFT size
          frameSize  - Number of elements to process per frame; if not given,
                       defaults to the FFT size. Must be less than or equal to
                       FFT size.
          ymin, ymax - Y-axis constraints. If not given, constraint will be
                       automatically calculated per-frame based on the data.
        """
        if not frameSize:
            frameSize = nfft
        LineBase.__init__(self, frameSize, ymin, ymax)
        PSDBase.__init__(self, nfft)

    def _configureAxes(self):
        self._plot.set_yscale('log')

    def _getXUnits(self):
        return BULKIO.UNITS_FREQUENCY

    def _formatData(self, block, stream):
        # Bit blocks don't have a .complex attribute; default to False
        if getattr(block, 'complex', False):
            data = block.cxdata
        else:
            data = block.buffer

        # Calculate PSD of input data.
        data, freqs = self._psd(data, stream)

        # Return x data (frequencies) and y data (magnitudes)
        return freqs, data.reshape(-1)

    def _getXRange(self, sri):
        return self._getFreqRange(sri)


class RasterBase(PlotBase):
    """
    Abstract base class for falling raster plots. Time is displayed on the
    Y-axis, while magnitude (Z-axis) is displayed using a color map. The
    meaning of the X-axis varies depending on the data being displayed.
    """
    def __init__(self, zmin, zmax, lines=None, readSize=None):
        PlotBase.__init__(self)
        self._zmin = zmin
        self._zmax = zmax
        self._readSize = readSize
        self._frameSize = None
        self._frameOffset = 0
        self._bitMode = False
        if lines is None:
            self._lines = 512
        else:
            self._lines = self._check_lines(lines)

        # Raster state: start with a 1x1 image with the default value
        self._imageData = self._createImageData(1, 1)
        self._buffer = self._imageData.reshape((1,))
        self._bufferOffset = 0
        self._image = self._plot.imshow(self._imageData, extent=(0, 1, 1, 0))
        norm = self._getNorm(self._zmin, self._zmax)
        self._image.set_norm(norm)
        self._plot.set_aspect('auto')

        # Use a horizontal yellow line to indicate the redraw position
        self._line = self._plot.axhline(y=0, color='y')

        self._stateLock = threading.Lock()

    def _createImageData(self, width, height):
        data = numpy.empty((height, width))
        data[:] = self._zmin
        return data

    def _getImageSize(self):
        return (self._frameSize, self._lines)

    def _portCreated(self, port, portDict):
        super(RasterBase,self)._portCreated(port, portDict)
        self._image.set_interpolation('nearest')
        if port.name == 'bitIn':
            self._bitMode = True
            self._setBitMode()
        else:
            # Add a colorbar
            self._colorbar = self._figure.colorbar(self._image)

        return port

    def _formatData(self, block, stream):
        return block.buffer

    def _setBitMode(self):
        pass

    def _getReadSize(self):
        return self._readSize

    def _update(self):
        if not self._port:
            return False

        # Read and format data.
        stream = self._port.getCurrentStream(bulkio.const.NON_BLOCKING)
        if not stream:
            return False
        block = stream.read(self._getReadSize())
        if not block:
            return False

        with self._stateLock:
            self._frameSize = self._getFrameSize(stream)
            frame_offset = self._frameOffset
            image_width, image_height = self._getImageSize()

        update_image = False
        if self._imageData.shape != (image_height,image_width):
            # TODO: Save references to old buffer?
            self._imageData = self._createImageData(image_width, image_height)
            self._buffer = self._imageData.reshape(-1)
            self._bufferOffset = 0
            update_image = True

        # If xdelta changes, update the X and Y ranges.
        redraw = True
        if block.sriChanged or update_image:
            # Update the X and Y ranges
            x_min, x_max = self._getXRange(stream, self._frameSize)
            y_min, y_max = self._getYRange(stream, self._frameSize)
            self._image.set_extent((x_min, x_max, y_max, y_min))

            # Trigger a redraw to update the axes
            redraw = True
        else:
            x_min, x_max, y_min, y_max = self._image.get_extent()

        # Update the framebuffer
        data = self._formatData(block, stream)
        start = (self._bufferOffset + frame_offset) % len(self._buffer)
        end = start + len(data)
        self._writeBuffer(self._buffer, start, end, data)
        self._bufferOffset = (self._bufferOffset + len(data)) % len(self._buffer)

        last_row = start // self._frameSize
        new_row  = self._bufferOffset // self._frameSize
        if new_row != last_row:
            # Move the draw position indicator
            y_range = y_max - y_min
            ypos = abs(y_min + ((new_row+1) * y_range)) / self._imageData.shape[0]
            self._line.set_ydata([ypos, ypos])

            # Update the image; by only doing this when at least one row is
            # completed, we can reduce the amount of time spent in redraws
            update_image = True

        # Only redraw the image from the framebuffer if something has changed
        if update_image:
            self._image.set_data(self._imageData)
            redraw = True

        return redraw

    def _writeBuffer(self, dest, start, end, data):
        if end <= len(dest):
            dest[start:end] = data
        else:
            count = len(dest) - start
            dest[start:] = data[:count]
            remain = end - len(dest)
            dest[:remain] = data[count:]

    # Plot settings
    def _check_lines(self, lines):
        lines = int(lines)
        if lines <= 0:
            raise ValueError('lines must be a positive integer')
        return lines

    @property
    def lines(self):
        """
        Number of frames worth of history to display. Must be greater than 0.
        """
        return self._lines

    @lines.setter
    def lines(self, lines):
        with self._stateLock:
            self._lines = self._check_lines(lines)

    def _check_zrange(self, zmin, zmax):
        if zmax < zmin:
            raise ValueError, 'Z-axis bounds cannot overlap (%d > %d)' % (zmin, zmax)

    def _update_zrange(self, zmin, zmax):
        self._zmin = zmin
        self._zmax = zmax
        norm = self._getNorm(self._zmin, self._zmax)
        self._image.set_norm(norm)

    @property
    def zmin(self):
        """
        The lower bound of the Z-axis.
        """
        return self._zmin

    @zmin.setter
    def zmin(self, zmin):
        self._check_zrange(zmin, self._zmax)
        self._update_zrange(zmin, self._zmax)

    @property
    def zmax(self):
        """
        The upper bound of the Z-axis.
        """
        return self._zmax

    @zmax.setter
    def zmax(self, zmax):
        self._check_zrange(self._zmin, zmax)
        self._update_zrange(self._zmin, zmax)

    @property
    def readSize(self):
        return self._readSize

    @readSize.setter
    def readSize(self, size):
        if size is not None:
            size = int(size)
            if size <= 0:
                raise ValueError('read size must be a positive integer')
        self._readSize = size


class RasterPlot(RasterBase):
    """
    Falling raster plot of time data. The Y-axis represents inter-frame time,
    while the X-axis represents intra-frame time. The Z-axis, mapped to a color
    range, represents the magnitude of each sample.
    """
    def __init__(self, frameSize=None, imageWidth=None, imageHeight=None, zmin=-1.0, zmax=1.0, lines=None, readSize=None):
        """
        Create a new raster plot.

        Arguments:

          frameSize   - Number of elements to draw per line
          zmin, zmax  - Z-axis (magnitude) constraints. Data is clamped to the
                        range [zmin, zmax].
          lines       - Number of frames worth of history to display (default 512).
          readSize    - Number of elements to read from the data stream at a
                        time (default is to use packet size)

        Deprecated arguments:
          imageWidth  - Width of the backing image in pixels (width is always
                        effective frame size)
          imageHeight - Height of the backing image in pixels (use lines)
        """
        if imageHeight is not None:
            if lines is not None:
                raise ValueError("'lines' and 'imageHeight' cannot be combined")
            warnings.warn('imageHeight is deprecated, use lines', DeprecationWarning)
            lines = imageHeight
        super(RasterPlot,self).__init__(zmin, zmax, lines=lines, readSize=readSize)
        self._frameSizeOverride = frameSize

    def _getNorm(self, zmin, zmax):
        if self._bitMode:
            return matplotlib.colors.NoNorm()
        else:
            return matplotlib.colors.Normalize(zmin, zmax)

    def _setBitMode(self):
        self._update_zrange(0, 1)

    def _getXLabel(self):
        return 'Time offset (s)'

    def _getYUnits(self):
        return BULKIO.UNITS_TIME

    def _getXRange(self, sri, frameSize):
        # X range is time per line.
        return 0, sri.xdelta * frameSize

    def _getYRange(self, sri, frameSize):
        # First, get the X range.
        x_min, x_max = self._getXRange(sri, frameSize)
        x_range = x_max - x_min

        # Y range is the total time across all lines.
        return 0, frameSize*x_range

    def _formatData(self, block, stream):
        # Image data cannot be complex; just use the real component.
        if self._bitMode:
            return block.buffer.unpack()
        elif block.complex:
            return list(numpy.abs(block.cxdata))
        else:
            return block.buffer

    def _getFrameSize(self, stream):
        if self._frameSizeOverride is not None:
            # Explicit frame size override
            return self._frameSizeOverride
        elif stream.subsize > 0:
            # Stream is 2-dimensional, use frame size
            return stream.subsize
        else:
            # Auto frame size, default to 1K
            return 1024

    def _check_zrange(self, zmin, zmax):
        if zmin != 0:
            raise ValueError('Z-axis minimum is always 0 with bit data')
        elif zmax != 1:
            raise ValueError('Z-axis minimum is always 1 with bit data')
        super(RasterPlot,self)._check_zrange(zmin, zmax)

    @property
    def frameSize(self):
        """
        The number of elements in a single frame (in other words, a single line
        of the raster). If set to None, the frame size is automatically
        determined from the data stream's subsize, defaulting to 1024 if
        subsize is 0.
        """
        return self._frameSizeOverride

    @frameSize.setter
    def frameSize(self, frameSize):
        if frameSize is not None:
            frameSize = int(frameSize)
            if frameSize <= 0:
                raise ValueError('frame size must be a positive value')
        with self._stateLock:
            self._frameSizeOverride = frameSize

    @property
    def frameOffset(self):
        """
        Offset, in number of real samples, to adjust the frame start.
        """
        return self._frameOffset

    @frameOffset.setter
    def frameOffset(self, offset):
        frameOffset = int(offset)
        with self._stateLock:
            self._frameOffset = offset

class RasterPSD(RasterBase, PSDBase):
    """
    Falling raster plot for displaying the power spectral density (PSD) of an
    input signal. Input data is padded to the FFT size, if necessary, and a
    Hanning window is applied before calculating the PSD.

    The Y-axis represents inter-frame time,  while the X-axis represents
    frequency.  The Z-axis, mapped to a color range, represents the magnitude
    of each frequency.

    The Z-axis (magnitude) is displayed using a logarithmic scale.
    """
    def __init__(self, nfft=1024, frameSize=None, imageWidth=None, imageHeight=None, zmin=1.0e-16, zmax=1.0, lines=None):
        """
        Create a new raster PSD plot.

        Arguments:

          nfft        - FFT size
          frameSize   - Number of elements to process per line; if not given,
                        defaults to the FFT size. Must be less than or equal to
                        FFT size.
          imageWidth  - Width of the backing image in pixels
          zmin, zmax  - Z-axis (magnitude) constraints. Data is clamped to the
                        range [zmin, zmax].
          lines       - Number of frames worth of history to display (default 512).


        Deprecated arguments:
          imageWidth  - Width of the backing image in pixels (width is always
                        determined from nfft)
          imageHeight - Height of the backing image in pixels (use lines)
        """
        if imageWidth is not None:
            warnings.warn('imageWidth is deprecated', DeprecationWarning)
        if imageHeight is not None:
            if lines is not None:
                raise ValueError("'lines' and 'imageHeight' cannot be combined")
            warnings.warn('imageHeight is deprecated, use lines', DeprecationWarning)
            lines = imageHeight
        RasterBase.__init__(self, zmin, zmax, lines=lines, readSize=frameSize)
        PSDBase.__init__(self, nfft)

    def _getNorm(self, zmin, zmax):
        return matplotlib.colors.LogNorm(zmin, zmax)

    def _getXUnits(self):
        return BULKIO.UNITS_FREQUENCY

    def _getYUnits(self):
        return BULKIO.UNITS_TIME

    def _getXRange(self, sri, frameSize):
        return self._getFreqRange(sri)

    def _getYRange(self, stream, frameSize):
        # Y range is the total time across all lines.
        dtime = stream.xdelta * self._nfft
        return 0, frameSize *dtime

    def _getFrameSize(self, stream):
        if stream.complex:
            return self._nfft
        else:
            return (self._nfft//2) + 1

    def _getReadSize(self):
        if self._readSize is not None:
            return self._readSize
        return self._nfft

    def _formatData(self, block, stream):
        if self._bitMode:
            data = block.buffer.unpack()
        elif block.complex:
            data = block.cxdata
        else:
            data = block.buffer

        # Calculate PSD of input data.
        data, freqs = self._psd(data, stream)
        return data.reshape(-1)

    @property
    def nfft(self):
        return self._nfft

    @nfft.setter
    def nfft(self, nfft):
        nfft = int(nfft)
        if nfft <= 0:
            raise ValueError('FFT size must be a positive value')
        with self._stateLock:
            self._nfft = nfft


class XYPlot(LineBase):
    """
    Scatter plot for displaying complex pairs as X/Y coordinates. The real
    component is plotted on the X-axis, while the imaginary component is
    plotted on the Y-axis.
    """
    def __init__(self, frameSize=1024, xmin=-1.0, xmax=1.0, ymin=-1.0, ymax=1.0):
        """
        Create a new X/Y plot.

        Arguments:

          frameSize  - Number of complex values to draw per frame.
          xmin, xmax - X-axis constraints.
          ymin, ymax - Y-axis constraints.
        """
        LineBase.__init__(self, frameSize, ymin, ymax)
        self._xmin = xmin
        self._xmax = xmax
        self._plot.set_xlim(self._xmin, self._xmax)

        # Enable gridlines and draw X-axis and Y-axis through the origin
        self._plot.grid(True)
        self._plot.axhline(0, color='black')
        self._plot.axvline(0, color='black')

    def _getXLabel(self):
        return 'Real'

    def _getYLabel(self):
        return 'Imaginary'

    def _formatData(self, data, sri):
        # Split real and imaginary components into X and Y coodinates.
        if sri.complex:
            return data.buffer[::2], data.buffer[1::2]
        else:
            return data.buffer, [0.0]*len(data.buffer)

    def _getXRange(self, sri):
        return self._xmin, self._xmax

    def _lineOptions(self):
        return {'marker': 'o', 'linestyle': 'None'}

    def _check_xrange(self, xmin, xmax):
        if xmin is None or xmax is None:
            return
        if xmax < xmin:
            raise ValueError, 'X-axis bounds cannot overlap (%f > %f)' % (xmin, xmax)

    # Plot properties
    @property
    def xmin(self):
        """
        The lower bound of the X-axis.
        """
        return self._xmin

    @xmin.setter
    def xmin(self, xmin):
        self._check_xrange(xmin, self._xmax)
        self._xmin = xmin
        self._setXView(self._xmin, self._xmax)

    @property
    def xmax(self):
        """
        The upper bound of the X-axis.
        """
        return self._xmax

    @xmax.setter
    def xmax(self, xmax):
        self._check_xrange(self._xmin, xmax)
        self._xmax = xmax
        self._setXView(self._xmin, self._xmax)
