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

# $Id: bluefile.py 4301 2012-06-02 00:33:05Z jai $

"""
Python interface to the X-Midas BLUE file format.  Uses the xmpyapi
module for X-Midas read/write AUXILIARY paths if available.  Supports
header and data in IEEE big and little endian formats only.

Main header keywords are imported as a dictionary. Extended header
keywords are imported as a list of (key, value) tuples.

ASCII data is imported in chunks corresponding to the format code
given (e.g. XA would be 80 character chunks) and by default, is
returned with trailing nulls and spaces removed.  The user can
optionally turn off the removal of trailing null characters by
setting bluefile's retain_nulls variable to True.
For example,

  import bluefile
  bluefile.retain_nulls = True

will turn off ALL stripping of null characters for the current session,
or until the variable is set back to False.

When ASCII data is exported it is space padded to the end of the atom.
Supports bit data only if it is accessed on byte boundaries and returns
it as a single string for type 1000, or a list of strings for type 2000
files.  Type 3000/5000 data is imported as a list of dictionaries.  Type
4000 data is imported as a list of records, each record being a list of
(key, value) tuples, the same format as the extended header keywords.

All multi-element numeric data is returned as a NumPy array.
Complex double and float data (CD and CF) are supported directly by
the numpy module; complex integer data and all other groupings with
more than 2 scalars per atom are returned as a multi-dimensional
NumPy array.

Type 2000 numeric data is typically returned as a list of NumPy
arrays; however, for users that prefer a 2-dimensional array, it is
possible to override this behavior.  Refer to the help for the method
set_type2000_format() for more detail.

Extended long integers (64-bits, data type 'X'), introduced in X-Midas
4.3.0, are supported for both keywords and data. Type 1000/2000 data is
handled as a NumPy array of type numpy.int64, while keywords, subrecords
and components are returned as Python longs.

Typical usage:

    import bluefile

    header, data = bluefile.read('file1')
    # change header and/or data
    bluefile.write('file2', hdr, data)
    
"""

import struct
import os
import re
import numpy
import warnings
from exceptions import UserWarning

try:
    import xmpyapi
except:
    xmpyapi = None

class XMTable:
    '''Not yet instantiable, a type used as a flag value in bluefile module'''
    def __init__(self):
        raise Exception(self.__doc__)

class XMKVList:
    '''Not yet instantiable, a type used as a flag value in bluefile module'''
    def __init__(self):
        raise Exception(self.__doc__)

_rep_tran = {
    '' : '',         # Native
    # 'VAX'  : '<',  # Little endian ints; reals translation get messy
    'IEEE' : '>',    # Big endian
    'EEEI' : '<'     # Little endian
}

if struct.pack('h', 1) == struct.pack('%sh' % _rep_tran['IEEE'], 1):
    _native_rep = 'IEEE'
else:
    _native_rep = 'EEEI'
_native_endian = _rep_tran[_native_rep]

_xm_to_struct = {
    'P' : '.125s', # bit data
    'A' : '8s', # 8 byte ASCII
    'O' : 'B', # numpy.uint8
    'B' : 'b', # numpy.int8
    'I' : 'h', # numpy.int16
    'L' : 'i', # numpy.int32
    'X' : 'q', # numpy.int64
    'F' : 'f', # numpy.float32
    'D' : 'd'  # numpt.float64
    }

_xm_to_numpy = {
    'O' : numpy.uint8,
    'B' : numpy.int8,
    'I' : numpy.int16,
    'L' : numpy.int32,
    'X' : numpy.int64,
    'F' : numpy.float32,
    'D' : numpy.float64
    }

_numpy_to_xm = {
    numpy.uint8 : 'O',
    numpy.int8  : 'B',
    numpy.int16 : 'I',
    numpy.int32 : 'L',
    numpy.int64 : 'X',
    numpy.float32 : 'F',
    numpy.float64 : 'D'
    }

# Unsigned integer types are promoted up one level of integer precision, when
# available.
_numpy_xm_promotion = {
    numpy.int     : numpy.int32,
    numpy.int8    : numpy.int8,
    numpy.uint8   : numpy.uint8,
    numpy.int16   : numpy.int16,
    numpy.uint16  : numpy.int32,
    numpy.int32   : numpy.int32,
    numpy.uint32  : numpy.int32,
    numpy.int64   : numpy.int64,
    numpy.float32 : numpy.float32,
    numpy.float64 : numpy.float64,
    }

# Use numpy native types for complex floating point types.
_complex_type = {
    numpy.float32 : numpy.complex64,
    numpy.float64 : numpy.complex128
}

_mode_tran = {
    'S' : 1,
    'C' : 2,
    'V' : 3,
    'Q' : 4,
    'M' : 9,
    'X' : 10,
    'T' : 16,
    'U' : 1,
    '1' : 1,
    '2' : 2,
    '3' : 3,
    '4' : 4,
    '5' : 5,
    '6' : 6,
    '7' : 7,
    '8' : 8,
    '9' : 9
    }

_type_tran = {
    'B' : 1,
    'I' : 2,
    'L' : 4,
    'X' : 8,
    'F' : 4,
    'D' : 8,
    'A' : 8,
    'P' : .125,
    'O' : 1,
    }

# Global variable, indicates to all functions in this module whether
# trailing NULLs should be stripped from strings
retain_nulls = False

# Global variable, indicates that the strings should grabbed as is,
# with spaces: if you want the full length of the field (even with spaces
# and nulls), set this before a read.
raw_mode = False

# Regular expression (re module) objects, whose sub() methods are
# used to remove trailing whitespace and nulls or trailing whitespace
# only (while retaining nulls) from a string
_m_length_re = re.compile('[\0- ]+$')
_m_length_retain_nulls_re = re.compile('[ \t\n\r\f\v]+$')

# Type 2000 data format. In the future, we may want to change the default
# to numpy.array and eventually deprecate list (and this function as
# well).
_type2000_format = list
def set_type2000_format(format=list):
    """
    Sets the data format returned when reading in type 2000 files.

    The default is 'list', meaning a list of NumPy arrays, where the
    length of each array is equal to the frame size. To return type 2000
    data as a 2-d array, <format> should be 'numpy.ndarray', e.g.:

      import bluefile, numpy
      bluefile.set_type2000_format(numpy.ndarray)

    Note that <format> is expected to a type object.
    """
    global _type2000_format
    if format not in [ list, numpy.ndarray ]:
        raise TypeError, 'Only list and numpy.ndarray are supported'
    _type2000_format = format


def _m_length(string, repl_text=''):
    """
    Private function which removes trailing whitespace from strings, and
    will also remove trailing nulls, unless the user has explicitly set
    bluefile's global variable retain_nulls to True.
    """
    if retain_nulls:
        # User explicitly requested to retain null characters on strings
        return _m_length_retain_nulls_re.sub(repl_text, string)
    else:
        # The standard behavior
        return _m_length_re.sub(repl_text, string)
    
def _m_length_raw(string, repl_text=''):
    """
    Private function which removes trailing whitespace from strings, and
    will also remove trailing nulls, unless the user has explicitly set
    bluefile's global variable retain_nulls to True.
    """
    if raw_mode : return string
    if retain_nulls:
        # User explicitly requested to retain null characters on strings
        return _m_length_retain_nulls_re.sub(repl_text, string)
    else:
        # The standard behavior
        return _m_length_re.sub(repl_text, string)


def _truncate_struct_def(struct_def, max_bytes):
    """
    Returns a struct_def dictionary.
    
    Removes fields from and updates the given struct_def so that
    it represents a packed struct of max_bytes in length.
    """

    # A struct we're doing a partial read of, skip the last tuple
    # as it is the "overall" calculation
    s = ''
    npacking = 0
    index = 0
    for name, fmt, count, byte_offset in struct_def['fields']:
        if byte_offset >= max_bytes:
            break
        elif not isinstance(fmt, str):
            # A composite type, return a raw <n>s byte string
            s += str(count * fmt['nbytes']) + 's'
            npacking += 1
        elif count > 1:
            s += str(count) + fmt
            npacking += count
        else:
            s += fmt
            npacking += 1
        index += 1
    return { 'packing':s, 'fields':struct_def['fields'][:index],
             'npacking':npacking, 'nbytes':byte_offset }

    
def _unpack_blue_struct(buf, struct_def, endian='@'):
    """
    Returns the string representation of a blue struct as a dictionary.
    
    Unpack the string buf to a dictionary according to struct_def, or
    as much of the struct as is available in buf.  If buf is None,
    a default structure corresponding to struct_def is created.  See
    _pack_blue_struct() for explanations of the struct_def and 
    endian arguments.
    """

    if buf is None:
        sdict = {}
        for name, fmt, count, byte_offset in struct_def['fields']:
            # fmt is an optional repcount and a struct pack char
            if isinstance(fmt, str):
                if fmt.endswith('s'):
                    val = ''
                else:
                    val = struct.unpack(fmt, struct.pack(fmt, 0))
                if not isinstance(val, tuple):
                    val = (val,)
                if 1 == count:
                    val = val[0]
                else:
                    val = count * val
                sdict[name] = val
            else: # something else
                # Set to None, on the theory that eventually we'll try
                # to unpack it as a struct, and will figure out what
                # it should have in it then.
                sdict[name] = None
        if 'defaults' in struct_def:
            sdict.update(struct_def['defaults'])
        return sdict

    if len(buf) < struct_def['nbytes']:
        # Partial struct unpack
        packing = _truncate_struct_def(struct_def, len(buf))['packing']
    else:
        packing = struct_def['packing']
        if len(buf) > struct_def['nbytes']:
            buf = buf[:struct_def['nbytes']]

    vals = struct.unpack(endian + packing, buf)
    index = 0
    sdict = {}
    for name, fmt, count, byte_offset in struct_def['fields']:
        if index >= len(vals):
            break
        elif not isinstance(fmt, str):
            # Embedded struct(s), union(s) are read in as a single string
            sdict[name] = vals[index]
            count = 1
        elif count > 1:
            # Tuple numeric - should this be a list instead?
            sdict[name] = vals[index:index+count]
        elif fmt.endswith('s'):
            # String, remove trailing white space and NULLs
            sdict[name] = _m_length_raw(vals[index])
        else:
            # Scalar numeric
            sdict[name] = vals[index]
        index += count

    return sdict



def _unpack_blue_struct_array(buf, struct_def, count, endian='@'):
    """
    Returns a list of dictionaries.
    
    Unpacks the string <buf> into a list of dictionaries according to
    the <struct_def> arg.  The <count> arg indicates the maximum
    number of dictionaries to be unpacked; this is limited by the
    length of <buf>.  See _pack_blue_struct() for explanations of the
    <struct_def> and <endian> arguments.

    If <buf> is None, returns a list of dictionaries with default
    values as indicated by the <struct_def> argument.  Fields with out
    a default value specified within <struct_def> numeric fields
    default in the following ways: numeric fields to 0, strings to "",
    and sub-structures to None.
    """
    if buf is None:
        return [_unpack_blue_struct(None, struct_def) for ii in xrange(count)]
    else:
        nbytes = struct_def['nbytes']
        return [_unpack_blue_struct(buf[ii:ii+nbytes], struct_def, endian)
                for ii in xrange(0, min(len(buf), count*nbytes), nbytes)]



def _blue_subrecord_map(hdr):
    """
    Return a blue struct_def dictionary for the subrecords/components
    list in a type 3000/5000/6000 header.  See _pack_blue_struct() for
    an explanation of the format of a struct_def dictionary.
    """
    if hdr.has_key('comp'):
        subrecord_defs = hdr['comp']
    else:
        subrecord_defs = hdr['subr']

    fields = []
    packing = ''
    npacking = 0
    byte_offset = 0

    for subr in subrecord_defs:
        # Type 6000 subrecords can have multiple elements
        num_elements = subr.get('num_elts', 1) 
        if subr.has_key('offset') and subr['offset'] > byte_offset:
            padding = subr['offset'] - byte_offset
            packing += 'x' * padding
            byte_offset += padding
        count = _mode_tran[subr['format'][0]] * num_elements
        if subr['format'][1] == 'A':
            fmt = str(8 * count) + 's'
            count = 1
        else:
            fmt = str(count) + _xm_to_struct[subr['format'][1]]
        fields.append((_m_length(subr['name'].lower()),
                       fmt, count, byte_offset))
        packing += fmt
        npacking += count
        byte_offset = struct.calcsize(packing)

    # Pad out the end if necessary
    record_length = struct.calcsize(packing)
    if record_length < hdr['record_length']:
        packing += str(hdr['record_length'] - record_length) + 'x'

    return { 'fields': fields, 'nbytes': hdr['record_length'],
             'packing': packing, 'npacking': npacking }


def form_read_path(filename, extensions=('.tmp', '.prm')):
    """
    Return the full file name as a string, path + root + ext, for a
    file in the X-Midas read path if the xmpyapi module is available.

    If the file name given contains an environment variable or a '~'
    symbol, the file name is replaced with its expanded value.
    
    If the file name already contains a path and extension, it is
    returned with the path in expanded form, if applicable.

    If no path is given and the xmpyapi module is available, the
    current X-Midas read path is searched, otherwise only the current
    directory is searched.

    If the filename has no extension the extensions given in the
    extensions arg, default ('.tmp', '.prm'). If multiple read paths
    and multiple extensions are searched, the extensions take
    precedence.  I.e. paths are stepped through one at a time and all
    extensions within that path are attempted before going on to the
    next path.

    If an X-Midas AUX path begins with a '$' it is assumed to contain
    UNIX environment variable names which are upper-cased and resolved
    (if found) before the path is returned.  Prior to X-Midas 3.8.0
    only the leading path element could be a UNIX environment variable
    and the fully resolved path length was limited to 80 characters.
    """
    # Expand any environment variables or ~ symbols
    filename = os.path.expanduser(os.path.expandvars(filename))

    path, root = os.path.split(filename)
    root, ext = os.path.splitext(root)
    if path and ext:
        return filename

    paths = (path,)
    if not path and xmpyapi is not None:
        # If the xmpyapi module is available to us, use the current
        # AUXILIARY read path
        paths = xmpyapi.current_auxes()[1]

    if ext:
        extensions = (ext,)

    for p in paths:
        if isinstance(p, int):
            p = xmpyapi.form_path(p, 'r')
        f = os.path.join(p, root)
        for e in extensions:
            if os.path.isfile(f + e):
                return f + e

    return filename



def _resolve_detach_name(hdr, exc_on_fail=1):
    """
    Sets the 'detach_name' field of the given header dictionary based
    on the current value of the 'detached' key.  This is the full
    file path for the detached data portion (if any) of the BLUE file.

    If 'detached' is 0, there is no detached portion and the
    'detach_name' field is removed if present.

    If 'detached' is < 0 or == 1, the detached portion should be in
    the same directory as the header with the same base name and a
    .det extension.

    If 'detached' > 1, it indicates the X-Midas auxiliary path of the
    detached portion of the file with the same base name and a .det
    extension. If the xmpyapi module is not present to resolve the aux
    path, an Exception is raised **unless** either the 'detach_name'
    field is already present or the exc_on_fail parameter is 0.  If
    'detach_name' is present it is assumed that the file name given is
    correct; this allows manual writing of a detached file without
    xmpyapi being present.  Otherwise, if exc_on_fail is 0, the header
    remains unmodified and is returned without a 'detach_name' field.
    """
    if not hdr.get('detached'):
        if hdr.has_key('detach_name'):
            del hdr['detach_name']
        return
    
    fname = os.path.splitext(hdr['file_name'])[0] + '.det'
    if hdr['detached'] < 0 or hdr['detached'] == 1:
        # An aux of 1 or -1 indicates the detached data is in the same dir
        hdr['detach_name'] = fname
    elif xmpyapi is not None:
        # Use the detached aux given
        path, fname = os.path.split(fname)
        hdr['detach_name'] = os.path.join(
            xmpyapi.form_path(hdr['detached'], 'r'), fname)
    elif not hdr.has_key('detach_name') and exc_on_fail:
        # Without xmpyapi we cannot resolve detach_name
        raise Exception("Location of detached data of %(file_name)s on AUX "
                        "%(detached)d cannot be resolved without direct "
                        "X-Midas support (xmpyapi module)" % hdr)



def _read_ext_header(hdr, f):
    """
    Reads in the extended header of an X-Midas BLUE file given a BLUE
    header dictionary hdr (as returned by read() or readheader()) and
    a file object f open to the file containing the extended header
    (the same file that contained the main BLUE header).  The hdr
    dictionary must contain 'ext_size', and 'ext_start' fields
    describing the size and location of the extended header in the
    file.  The extended header is read in as a string of raw bytes
    'ext_size' bytes long and is assigned to the key 'ext_header' and
    then the 'ext_size' field is removed.  An empty extended header is
    represented by the empty string.
    """
    if hdr['ext_size'] > 0:
        # Do a few sanity checks on the extended header information first
        
        if hdr['ext_start'] == 0:
            raise ValueError('Corrupted header - ext_start conflicts with '
                             'location of main header')

        # Calculate extended header start location in bytes, since we
        # will need to reference this number several times
        ext_start = hdr['ext_start'] * 512

        if not hdr.get('detached') and hdr['data_size'] > 0:
            # There is data and an extended header in one file.
            # Make sure their reported locations don't conflict
            
            if hdr['data_start'] < ext_start:
                # When data preceeds extended header, check that end of
                # data does not overlap with start of extended header
                if (hdr['data_start'] + hdr['data_size']) > ext_start:
                    raise ValueError('Corrupted header - ext_start conflicts '
                                     'with location of data')
            else:
                # When extended header preceeds data, check that end of
                # extended header does not overlap with start of data
                if (ext_start + hdr['ext_size']) > hdr['data_start']:
                    raise ValueError('Corrupted header - ext_start + ext_size '
                                     'overlaps with location of data')
        
        if hdr['file_name']:
            # We can't do an os.stat on hdr['file_name'] in the case
            # of stream types, since it will be set to None.
            if (ext_start + hdr['ext_size']) > os.stat(hdr['file_name'])[6]:
                raise ValueError('Corrupted header - given extended header '
                                 'location extends beyond end of file')

        # Passed sanity checks.  Read the extended header
        f.seek(ext_start)
        hdr['ext_header'] = f.read(hdr['ext_size'])
    else:
        hdr['ext_header'] = ''
    del hdr['ext_size']


def _unpack_header_adjunct(hdr):
    """
    Unpacks the adjunct from a BLUE file header dictionary.  Expects
    the 'adjunct' field to be a 256 byte raw string, or None (in which
    case it returns with default values).
    """
    # Convert the adjunct (file type specific) part of the header
    file_class = int(hdr['type'] / 1000)
    endian = _rep_tran[hdr['head_rep']]
    hdr.update(_unpack_blue_struct(hdr['adjunct'],
                                   _bluestructs['T%dADJUNCT' % file_class],
                                   endian))
    del hdr['adjunct']
    if file_class == 3:
        hdr['subr'] = _unpack_blue_struct_array(hdr['subr'],
                                                _bluestructs['SUBRECSTRUCT'],
                                                hdr['subrecords'], endian)
    elif file_class == 5:
        hdr['comp'] = _unpack_blue_struct_array(hdr['comp'],
                                                _bluestructs['COMPSTRUCT'],
                                                hdr['components'], endian)
        hdr['quadwords'] = _unpack_blue_struct(hdr['quadwords'],
                                               _bluestructs['T5QUADWORDS'],
                                               endian)
    elif file_class == 6:
        # For Type 6000 files, the hdr['subr'] field gets filled in from
        # information in the extended header, which is not unpacked
        # until after the main header and header adjunct are unpacked.
        # See _open_t6subrecords(), which gets called by readheader().   
        # Just initialize it to an empty list here.
        hdr['subr'] = []
    

def _unpack_header_main(raw_header_str):
    """
    Unpacks the 512 byte main header without adding any of the
    'internals' derivative fields.  The adjunct fields (the last 256
    bytes) remain packed as a raw string. To unpack the adjunct, call
    _unpack_header_adjunct().

    If raw_header_str is None or '', a default empty header is
    returned (type=1000, format='SF', data_start=512.0, etc.) same as
    one returned from M$INIT().  The 'adjunct' field is set to None.
    """
    if raw_header_str:
        # Extract the header representation from the head_rep string
        # field in the raw packed header.
        hdr = _unpack_blue_struct(raw_header_str, _bluestructs['HEADER'],
                                  _rep_tran[raw_header_str[4:8]])
    else:
        # Create a default header with default values.
        hdr = _unpack_blue_struct(None, _bluestructs['HEADER'])

    # Import the main header keywords, they're not a plain old FORTRAN string
    if hdr['keylength'] > 0:
        lkey = _bluestructs['HEADER']['lookups']['keywords'][3]
        keystr = raw_header_str[lkey:lkey+hdr['keylength']]
        hdr['keywords'] = dict([kv.split('=', 1)
                                for kv in keystr.split('\0') if len(kv)])
    else:
        hdr['keywords'] = {}

    return hdr
        

def update_header_internals(hdr):
    """
    Update the given header dictionary, with selected useful
    'internals' fields, based on main header fields ('type', 'format',
    etc.).

    The following additional fields are added based on information present
    in the raw header:
      'class' - 1,2,3,4,5 or 6 (the thousands place of 'type')
      'size'  - the number of elements in the file (rounded down if fractional)
      'bpe'   - bytes/elem   (not avail for variable record length KW data)
      'bps'   - bytes/scalar (not avail for KW, NH or Undefined format data)
      'spa'   - scalars/atom (not avail for KW, NH or Undefined format data)
      'bpa',  - bytes/atom   (not avail for KW, NH or Undefined format data)
      'ape',  - atoms/elem   (not avail for KW or NH data)

    The given header must already have its 'adjunct' fields unpacked.
    """
    file_class = hdr['class'] = int(hdr['type'] / 1000)
    format = hdr['format']

    # Disallow type 4000 data when calculating bps, spa and bpa: its
    # always KW format data no matter what the 'format' field reads so
    # these fields are misleading.  Also, don't try calculating bps,
    # spa, and bpa for undefined formats (such as those that can occur
    # in Type 6000 files).
    if file_class != 4 and format not in ('NH', 'KW') \
       and _mode_tran.has_key(format[0]):
        # bytes per scalar
        if format[1] == 'P':
            hdr['bps'] = .125
        elif _xm_to_struct.has_key(format[1]):
            hdr['bps'] = struct.calcsize(_xm_to_struct[format[1]])
        hdr['spa'] = _mode_tran[format[0]]      # scalars per atom
        hdr['bpa'] = hdr['bps'] * hdr['spa']    # bytes per atom
    else:
        for k in ('bps', 'spa', 'bpa', 'ape'):
            if k in hdr: del hdr[k]

    if file_class <= 2:
        hdr['ape'] = hdr.get('subsize', 1)      # atoms per element
        hdr['bpe'] = hdr['bpa'] * hdr['ape']    # bytes per element
    elif file_class == 4:
        hdr['bpe'] = hdr['vrecord_length']      # bytes per element
        if hdr['bpe'] <= 0:
            hdr['size'] = hdr['nrecords']
    elif file_class <= 6:
        # Covers type 3000, 5000 and 6000
        hdr['bpe'] = hdr['record_length']       # bytes per element
        if hdr.has_key('bpa'):
            hdr['ape'] = hdr['bpe'] / hdr['bpa'] # atoms per element

    if hdr['bpe'] > 0:
        if hdr['type'] % 1000 / 100 == 2:
            # If this is a packetized data file (i.e., type x200), we
            # need to subtract size of packet data to get true data_size
            try:
                hdr['size'] = long((hdr['data_size']
                                    - int(hdr['keywords']['PKT_BYTE_COUNT']))
                                   / hdr['bpe'])
            except:
                hdr['size'] = long(hdr['data_size'] / hdr['bpe'])
        else:
            hdr['size'] = long(hdr['data_size'] / hdr['bpe'])


def unpack_header(raw_header_str, endian=None):
    """
    Returns a header in the form of a dictionary.
    
    Unpack the 512 length raw string of bytes that comprise a BLUE
    header to a dictionary representing it with the same field names
    as the main portion of the X-Midas HEADER struct.  If the
    raw_header_str is None rather than a raw string, a default type
    1000 BLUE header is returned ala bluefile.header().

    Additional fields are added to the header based on information
    present in the raw header.  These fields are documented under the
    update_header_internals() method.

    The endian argument is currently ignored. The endianness of the
    data packed in the raw_header_str is determined by looking at the
    packed 'head_rep' field within it.
    """
    hdr = _unpack_header_main(raw_header_str)
    _unpack_header_adjunct(hdr)
    update_header_internals(hdr)
    return hdr


def header(type=1000, format='SF', **kw):
    """
    Return a header dict of the given file type and format. All header
    fields default to the same values as those returned by the X-Midas
    M$INITIALIZE() routine unless they are explicitly overridden with
    trailing keyword arguments.  Keyword argument names should match
    the names of the fields in the header.  The internals fields (see
    unpack_header for a list of these fields) are calculated after the
    keyword arguments are assigned to the header.

    The keyword arguments should match the header field names, which
    differ from the tags supported by the HEADERMOD command. Common
    fields and the equivalent HEADERMOD tag are:
    
        xunits     XU
        xstart     XS
        xdelta     XD
        subsize    FS
        yunits     YU
        ystart     YS
        ydelta     YD

    Some fields, such as 'subsize', affect multiple internal header
    field values. Changing these values after creation may result in
    inconsistent behavior and should be avoided. In general, the best
    practice is to set all of the header fields you wish to alter at
    creation time.

    Examples:
    To return a type 2000 file with a specific frame size and abscissa
    you could use the calling arguments:
      hdr = bluefile.header(2000, 'SF', subsize=512, ystart=128.0)

    Adding subrecords or components to a new type 3000/5000 file
    requires multiple fields to be calculated in concert.  Use the
    addsubr()/addcomp()/addt6subr() methods to ensure correctness:

      hdr = bluefile.header(3000, rstart=1.0)
      bluefile.addsubr(hdr, 'TIME', 'SD')
      bluefile.addsubr(hdr, 'FREQ', 'SF')
    """
    hdr = _unpack_header_main(None)
    hdr['type'] = type
    hdr['format'] = format.upper()
    _unpack_header_adjunct(hdr)

    if 'quadwords' in hdr:
        qw = {}
        for k in kw.keys():
            if not k in hdr.keys() and k in hdr['quadwords'].keys():
                qw[k] = kw[k]
                del kw[k]
        hdr['quadwords'].update(qw)
        
    hdr.update(kw)
    update_header_internals(hdr)
    if hdr['class'] == 6:
        hdr['ext_header'] = []
    return hdr


def _open_t6subrecords(hdr):
    """
    Finds and extracts the Type 6000 subrecords from the extended
    header's 'SUBREC_DEF' key, puts the subrecords into the main
    header's 'subr' key as a list of dictionaries, and removes the
    SUBREC_DEF key from the extended header keywords.  The stringized
    subrecord fields in hdr['subr'] are then cast into their
    appropriate data type.
    """
    repack = 0
    subrec_def_string = 0
    
    if isinstance(hdr['ext_header'], str):
        unpack_ext_header(hdr)
        repack = 1
        
    if isinstance(hdr['ext_header'], dict):
        if 'SUBREC_DEF' in hdr['ext_header']:
            subrec_def_string = hdr['ext_header']['SUBREC_DEF']
            del hdr['ext_header']['SUBREC_DEF']
    elif isinstance(hdr['ext_header'], list):
        for index in xrange(0, len(hdr['ext_header'])):
            if hdr['ext_header'][index][0] == 'SUBREC_DEF':
                subrec_def_string = hdr['ext_header'][index][1]
                del hdr['ext_header'][index]
                break
    if repack:
        pack_ext_header(hdr)
        
    if subrec_def_string == 0:
        warnings.warn("'SUBREC_DEF' keyword not found in extended header",
                      UserWarning, stacklevel=2)
        return

    hdr['subr'] = _unpack_blue_struct_array(subrec_def_string,
                                            _bluestructs['T6SUBR_STR_STRUCT'],
                                            hdr['subrecords'],
                                            _rep_tran[hdr['head_rep']])

    # Cast selected subrecord fields to thier appropriate numeric data types
    for subrec in hdr['subr']:
        subrec['minval']   = float(subrec['minval'])
        subrec['maxval']   = float(subrec['maxval'])
        subrec['offset']   = int(subrec['offset'])
        subrec['units']    = int(subrec['units'])
        subrec['num_elts'] = int(subrec['num_elts'])
        subrec['uprefix']  = int(subrec['uprefix'])
        del subrec['reserved']
        

def readheader(filename, ext_header_type=None, keepopen=0):
    """
    Returns a dictionary [, file object tuple].

    Returns the main header for BLUE files as a Python dictionary.

    <filename> can be either be the name of a file on disk, or
    an open, file-like object with the file pointer positioned at
    the beginning of a bluefile header.  If a file-like object,
    <keepopen> should be set to True; in which case, the given
    <filename> is simply returned back to the caller as the file
    object, and closure of the in-memory object is avoided.
    
    If <keepopen> (default 0) is true, a file object wrapping the
    open BLUE file is also returned.

    If <ext_header_type> (default None, or list for type 6000 files)
    is the type object str, list, dict, bluefile.XMKVList or
    bluefile.XMTable, the extended header keywords section is read in
    and assigned to the 'ext_header' field in the Python dictionary
    returned.  If type is str, the extended header is kept in its
    binary packed format and is returned as a raw string. This string
    can later be unpacked to a list of (key, value) pairs by
    unpack_ext_header(), or can be written directly back to disk
    unaltered.  If the type given is list, return the keywords
    unpacked as a list of (key, value) pairs, the native format for
    X-Midas keywords.  A type of dict takes this list and constructs a
    Python dictionary with the dict() method before returning it (see
    the discussion of precedence of repeated keys below).  The
    bluefile.XMKVList and bluefile.XMTable types are similar to list
    and dict but also interpret any structure tags found in the
    X-Midas keywords that indicate nested dictionaries and lists.
    These tags are embedded in the X-Midas keywords by calling
    writeheader() or write() with bluefile.XMKVList or
    bluefile.XMTable and preserve the structure of any Python
    dictionaries and lists found in the extended header.  See the
    discussion of structured vs unstructured keywords in the doc
    string for pack_keywords() for more details.

    Since subrecord information for type 6000 files is stored in the
    extended header and is required to interpret the data section of
    the file, the extended header is always opened for type 6000
    files.  <ext_header_type> can still be specified but defaults to
    list if not specified.

    Repeated Key Precedende in the Extended Header: The most correct
    Python representation of X-Midas extended header keywords is a
    list of (key, value) pairs, although they are frequently treated
    as dictionaries.  X-Midas has limited capability for embedding
    structure to its keywords by repeating key names in a convention
    called ''keyword scoping''. Scoped sections are the keywords found
    between two repeated key names.  When specifying dict or
    bluefile.XMTable as the preferred <ext_header_type>, readheader()
    unpacks the keywords to the native (key, value) pair list, then
    calls the Python dict() method on this top level list to produce
    the Python dictionary requested.  In dict(), if any key names are
    repeated in the list, the value of the LAST repeated key will
    replace the value of any previous keys.  In X-Midas if you perform
    a KEYWORD GET on a key name that is repeated, the value of the
    FIRST repeated key is returned.  For the KEYWORD GET behavior
    working with an extended header in a dictionary format do the
    following:

      hdr = readheader(file_name, ext_header_type=list)
      hdr['ext_header'].reverse()
      hdr['ext_header'] = dict(hdr['ext_header'])

    If the data portion of the file is not detached, the file object
    returned has its current read pointer positioned at the first data
    element.  Otherwise, the name of the detached portion of the file
    is returned in the 'detach_name' field.  If the 'detached' aux is
    > 1 and X-Midas is not available to resolve the location of the
    aux, the 'detach_name' will not be present in the returned header.
    The data in the detached file begins at the 'data_start' byte
    offset and is 'data_size' bytes in length.
    """
    # Read in the blue header bytes and convert common parts to a dictionary
    if hasattr(filename, 'read'):
        # We have been given an open, file-like object
        pathname = None
        f = filename
    else:
        pathname = form_read_path(filename)
        f = open(pathname, 'rb')
        
    rawhdr = f.read(512)
    head_rep = rawhdr[4:8]

    if head_rep != 'IEEE' and head_rep != 'EEEI':
        raise Exception(head_rep + ' formatted BLUE headers not supported, '
                        'convert to IEEE or EEEI format first')
    
    hdr = unpack_header(rawhdr)
    hdr['file_name'] = pathname
    
    if hdr['class'] == 6 and ext_header_type is None:
        ext_header_type = list

    if ext_header_type is not None:
        _read_ext_header(hdr, f)
        structured = ext_header_type in (XMKVList, XMTable)
        if ext_header_type in (str, ''):
            pass
        elif ext_header_type in (list, [], XMKVList):
            unpack_ext_header(hdr, structured=structured)
        elif ext_header_type in (dict, {}, XMTable):
            # XMTable and dict are not legal values for unpack_ext_header()
            unpack_ext_header(hdr, structured=structured)
            # The following line would get us the KEYWORD GET precedence
            # of repeated keys.
            # hdr['ext_header'].reverse()
            hdr['ext_header'] = dict(hdr['ext_header'])
        else:
            raise Exception('Extended header cannot be converted to type: ' +
                            repr(ext_header_type))

    if hdr['class'] == 6:
        _open_t6subrecords(hdr)
        
    _resolve_detach_name(hdr, exc_on_fail=0)

    if keepopen:
        if not hdr.get('detached'):
            # If the data portion is not detached, seek to the first
            # element in the open file we are returning.
            f.seek(long(hdr['data_start']))
        return hdr, f
    else:
        f.close()
        return hdr


def _convert_to_type(hdr, new_type=None):
    """
    Converts a header in place to the new file type.  There are two
    ways to specify the new file type:  as a keyword argument to
    convert_to_type(), or by changing hdr['type'] to the desired
    type before calling convert_to_type on it.
    
    Calling convert_to_type without providing a new type in one of these
    two manners simply keeps the file type the same, but resets many of
    the fields in the header (this may result in undesirable behavior).
    The 'file_name' and 'ext_header' fields are propagated from the old
    header to the new one if present.
    
    Intended for converting Type 1000 files to other formats only.  If
    converting to Type 6000, the extended header (if any), should have
    been opened along with the Type 1000 header, and the information
    should be in hdr['ext_header'], otherwise any extended header
    information will be lost.
    """
    if new_type is not None:
        hdr['type'] = new_type
    
    has_ext_header = hdr.has_key('ext_header')
    if has_ext_header:
        ehdr = hdr['ext_header']
        del hdr['ext_header']
        
    new_hdr = unpack_header(pack_header(hdr))

    if 'file_name' in hdr:
        new_hdr['file_name'] = hdr['file_name']
    if has_ext_header:
        new_hdr['ext_header'] = ehdr
    elif new_type == 6:
        new_hdr['ext_header'] = []
    hdr.clear()
    hdr.update(new_hdr)


def _add_subr(hdr, name, format, ctype=0, units=0, \
              minval=0, maxval=0, num_elems=1, uprefix=0):
    """
    Adds a new type 3000/5000/6000 subrecord/component to the given header.
    Performs common error checking.  The subrecord description is
    appended to the 'subr'/'comp' array.  This subrecord description
    contains:
       'name', 'format', and byte 'offset' fields for type 3000,
       'name', 'format', 'type' and 'units' fields for type 5000, and
       'name', 'format', byte 'offset', 'minval', 'maxval', 'numelems',
       'units', and 'uprefix' fields for type 6000.
    """
    adjunct_fields = _bluestructs['T%dADJUNCT' % hdr['class']]['fields']
    nfield = adjunct_fields[3][0]    # 'subrecords' / 'components'
    subr = hdr[adjunct_fields[8][0]] # 'subr' / 'comp'
    maxfields = adjunct_fields[8][2] # 26 / 14
    name = name.upper()

    # Unless this is a Type 6000 subrecord, we cannot exceed the limit
    # on the number of subrecords/components, and the subrecord name must
    # be <= 4 characters.
    if not hdr['class'] == 6:
        if len(subr) >= maxfields:
            raise Exception("File already contains maximum number of %s."
                            % nfield)
        if len(name) > 4:
            raise Exception('%s name %s too long. X-Midas requires it be <= 4 '
                            'characters.' % (nfield[:-1].title(), name))
                            
    # Format must be upper case, 2 characters and no bit data allowed
    format = format.upper()
    if len(format) == 1:
        format = 'S' + format
    sbpa = bpa(format)
    if sbpa < 0:
        raise Exception('%s %s contains bit data; must be filled in '
                        'explicitly.' % (nfield[:-1].title(), name))

    # Check for duplicate field name
    for s in subr:
        if name != s['name']: continue
        raise Exception("%s %s already exists." % (nfield[:-1].title(), name))

    # OK, now we can add the subrecord and update associated fields.
    if not subr:
        hdr['format'] = format
        hdr['record_length'] = 0
    if format != hdr['format']:
        hdr['format'] = 'NH'
    if hdr['record_length'] % _type_tran[format[1]]:
        warnings.warn("%s %s is not naturally aligned." %
                      (nfield[:-1].title(), name), UserWarning, stacklevel=3)
    s = {'name':name, 'format':format}
    if hdr['class'] == 3:
        s['offset'] = hdr['record_length']
    elif hdr['class'] == 5:
        s['type'] = int(ctype)
        s['units'] = int(units)
    elif hdr['class'] == 6:
        s['offset']   = hdr['record_length']
        s['minval']   = float(minval)
        s['maxval']   = float(maxval)
        s['num_elts'] = int(num_elems)
        s['units']    = int(units)
        s['uprefix']  = int(uprefix)
    subr.append(s)
    hdr[nfield] = len(subr)
    # Note: num_elems will only vary from the default of 1 for T6000 subrs
    hdr['record_length'] += (sbpa * num_elems)
    update_header_internals(hdr)

    
def addsubr(hdr, name, format):
    """
    Adds a subrecord to a type 3000 file.  Subrecords added in this
    manner are assumed to be contiguous and must not contain bit
    data formats.  The subrecord's offset is calculated and the file
    header's 'subrecords' and 'record_length' fields are updated.
    Additionally, if a record is added that does not have the same
    format as is indicated by the file header's 'format' field, the
    header's 'format' field will be changed to 'NH' for non-homogeneous.

    This method also checks for natural alignment of subrecords.  If a
    subrecord is added which is not naturally aligned, a warning message
    is displayed.
    
    If the file header passed in is not a type 3000 file, the header
    is converted to a type 3000 file by throwing away all of the non
    type 3000 fields and adding in default type 3000 fields for those
    that are not present.  For example, if you have assigned 'rstart'
    and 'rdelta' fields into the non type 3000 header prior to passing
    it in, they are preserved.

    Example Usage:
    A new header dictionary constructed from scratch:
        hdr = bluefile.header(3000)
        bluefile.addsubr(hdr, 'TIME', 'SD')
        bluefile.addsubr(hdr, 'FREQ', 'SF')

    is the same as explicitly filling in the following dictionary elements:
        hdr = bluefile.header(3000)
        hdr['format'] = 'NH'
        hdr['subrecords'] = 2
        hdr['record_length'] = 12
        hdr['subr'] = [{'name':'TIME', 'format':'SD', 'offset':0},
                       {'name':'FREQ', 'format':'SF', 'offset':8}]

    You can also call these routines to modify the header of an
    existing file (note that the data will have to be repacked and
    rewritten to disk if you keep the same file name).
        hdr = bluefile.readheader('myfile')
        bluefile.addsubr(hdr, 'TIME', 'SD')
        bluefile.addsubr(hdr, 'FREQ', 'SF')
        bluefile.writeheader('myalteredfile', hdr)
    """
    if hdr['class'] != 3:
        _convert_to_type(hdr, 3000)
    _add_subr(hdr, name, format)
    

def addcomp(hdr, name, format, ctype=0, units=0):
    """
    Adds a component to a type 5000 file.  Components added in this
    manner are assumed to be contiguous and must not contain bit data
    formats.  The component dictionary is added to the hdr['comp']
    list and the file header's 'components' and 'record_length' fields
    are updated.  Additionally, if a component is added that does not
    have the same format as is indicated by the file header's 'format'
    field, the header's 'format' field will be changed to 'NH' for
    non-homogeneous.
    
    addcomp() checks for natural alignment of components.  If a
    component is added which is not naturally aligned, a warning
    message is displayed.

    If the file header passed in is not a type 5000 file, the header
    is converted to a type 5000 file by throwing away all of the non
    type 5000 fields and adding in default type 5000 fields for those
    that are not present.  For example, if you have assigned 'tstart'
    and 'tdelta' fields into the non type 5000 header prior to passing
    it in, they are preserved.

    Example Usage:
    A new header dictionary constructed from scratch:
        hdr = bluefile.header(5000)
        bluefile.addcomp(hdr, 'POS',  'VD', 2, 5)
        bluefile.addcomp(hdr, 'VEL',  'VD', 2, 6)
        bluefile.addcomp(hdr, 'NAME', '2A', 1, 0)

    is the same as explicitly filling in the following dictionary elements:
        hdr = bluefile.header(5000)
        hdr['format'] = 'NH'
        hdr['comp'] = [{'name':'POS', 'format':'VD', 'type':2, 'units':5},
                       {'name':'VEL', 'format':'VD', 'type':2, 'units':6},
                       {'name':'NAME','format':'2A', 'type':1, 'units':0}]
        hdr['components'] = 3
        hdr['record_length'] = 64

    You can also call these routines to modify the header of an
    existing file (note that the data will have to be repacked and
    rewritten to disk if you keep the same file name and the file
    has associated data).
        hdr = bluefile.readheader('myfile')
        bluefile.addcomp(hdr, 'POS',  'VD', 2, 5)
        bluefile.addcomp(hdr, 'VEL',  'VD', 2, 6)
        bluefile.addcomp(hdr, 'NAME', '2A', 1, 0)
        bluefile.writeheader('myalteredfile', hdr)

    The 'quadwords' fields defining the reference frame for this
    header will have to be filled in explicitly in either case. See
    the Midas BLUE File Format document for more information.

    For more information about the <ctype> and <units> arguments, see
    the X-MIDAS documentation for M$COMPTYPE_NAME and M$UNITS_NAME.
    """
    # Type 5000 specific error checking
    if ctype not in (0, 1, 2, 3, 4, 5, 6, 10):
        raise Exception("Illegal component type %d in addcomp()" % ctype)
    if units < 0:
        raise Exception("Illegal units value %d in addcomp()" % units)
    # Do common error checking and add component
    if hdr['class'] != 5:
        _convert_to_type(hdr, 5000)
    _add_subr(hdr, name, format, ctype, units)


def addt6subr(hdr, name, format, minval, maxval, num_elems, units, uprefix=0):
    """
    hdr        Header for the file, as returned by read() or readheader()
    name       Name of the subrecord; 24 characters or less
    format     Subrecord format digraph
    minval     Minimum value for the subrecord
    maxval     Maximum value for the subrecord
    num_elems  Number of elements
    units      Units code
    uprefix    Prefix code
    
    Adds a subrecord to a type 6000 file.  Subrecords added in this
    manner are assumed to be contiguous and must not contain bit data
    formats.  The new subrecord dictionary is added to the hdr['subr']
    list and the file header's 'subrecords' and 'record_length' fields
    are updated.

    Additionally, if a subrecord is added that does not have the same
    format as is indicated by the file header's 'format' field, the
    header's 'format' field will be changed to 'NH' for non-homogeneous.
    
    addt6subr() checks for natural alignment of subrecords.  If a
    subrecord is added which is not naturally aligned, a warning
    message is displayed.

    If the file header passed in is not a type 6000 file, an
    exception is raised.

    Example Usage:
    A new header dictionary constructed from scratch:
        hdr = bluefile.header(6000)
        bluefile.addt6subr(hdr, 'TIMEOFDAY', 'SD', 0, 0, 1, 0, 0)
        bluefile.addt6subr(hdr, 'FREQUENCY', 'SF', 0, 0, 1, 0, 0)

    is the same as explicitly filling in the following dictionary elements:
        hdr = bluefile.header(6000)
        hdr['format'] = 'NH'
        hdr['subrecords'] = 2
        hdr['record_length'] = 12
        hdr['subr'] = [{'name':'TIMEOFDAY', 'format':'SD', 'minval':0,
                        'maxval':0, 'num_elts':1,'units':0, 'uprefix':0,
                        'offset':0},
                       {'name':'FREQUENCY', 'format':'SF','minval':0,
                        'maxval':0, 'num_elts':1,'units':0, 'uprefix':0,
                        'offset':8}]

    You can also call these routines to modify the header of an
    existing Type 6000 file (note that the data will have to be
    repacked and rewritten to disk if you keep the same file name).
        hdr = bluefile.readheader('myfile')
        bluefile.addt6subr(hdr, 'TIMEOFDAY', 'SD', 0, 0, 1, 0, 0)
        bluefile.addt6subr(hdr, 'FREQUENCY', 'SF', 0, 0, 1, 0, 0)
        bluefile.writeheader('myalteredfile', hdr)        
    """
    # Type 6000-specific error checking
    if hdr['class'] != 6:
        raise Exception('addt6subr() requires a Type 6000 file header')

    if len(name) > 24:
            raise Exception('Subrecord name %s too long. X-Midas requires'
                            'it to be <= 24 characters.' % name)
    if units < 0:
        raise Exception("Illegal units value %d in addt6subr()" % units)

    _add_subr(hdr, name, format, units=units, minval=minval, \
              maxval=maxval, num_elems=num_elems, uprefix=uprefix)


def update_t6_maxmin(hdr, data):
    """
    Given a Type 6000 header and data, iterates through each data
    element to determine the current max and min for each subrecord,
    and updates the subrecord max and min fields accordingly.
    """
    # Type 6000-specific error checking
    if hdr['class'] != 6:
        raise Exception('update_t6_maxmin() is for Type 6000 files only')

    for index in xrange(0, len(data)):
        for subrecord in hdr['subr']:
            if subrecord['format'][1] in ['A', 'a']:
                continue
            
            if index == 0:
                subrecord['minval'] = data[index][subrecord['name'].lower()]
                subrecord['maxval'] = data[index][subrecord['name'].lower()]
                continue

            if data[index][subrecord['name'].lower()] < subrecord['minval']:
                subrecord['minval'] = data[index][subrecord['name'].lower()]
            if data[index][subrecord['name'].lower()] > subrecord['maxval']:
                subrecord['maxval'] = data[index][subrecord['name'].lower()]

                    
def _pack_blue_struct(data, struct_def, endian='@'):
    """
    Pack the given data dictionary to a raw block of bytes as
    described by the struct_def dictionary and return it as a string.

    struct_def is a dictionary containing the following fields:

      'fields': a tuple of (name, packing, count, byte_offset) tuples 
      'nbytes': the total length in bytes of the struct
      'packing': the packing string for the all fields (ala the struct module)
      'npacking': the number of arguments the packing string expects

    Each tuple in the 'fields' tuple describes the name of the field
    to extract, the packing string (ala the struct module), the number
    of elements to pack and the byte_offset into the struct where the
    data is to be written.  The struct_def dictionary can be recursive
    if the packing element is itself a struct_def dictionary instead
    of a string.  In this case the field in the data associated with
    the named key is expected to be a dictionary.

    Fields that are unions must be explicitly pre-packed as a string
    as there is typically some external information required to decide
    how to pack/unpack them.

    The endian argument (default '@') is the struct.pack() format
    character specifying the endianness of the data to be packed
    ('<' == EEEI, '>' == IEEE, '@' == native).
    """
    
    vals = []
    for name, fmt, count, byte_offset in struct_def['fields']:
        if isinstance(fmt, dict):
            if isinstance(fmt['fields'], dict):
                # A union should already be packed correctly
                vals.append(data[name])
            elif count == 1:
                # A struct puts its values in a sub-dict
                vals.append(_pack_blue_struct(data.get(name, {}), fmt, endian))
            else:
                # An array of structs is a list of dicts
                vals.append(_pack_blue_struct_array(data.get(name, []),
                                                    fmt, count, endian))
        elif fmt.endswith('s'):
            # Convert the data to a string if it isn't already (DR #666442-19).
            s = str(data.get(name, ''))
            # Space pad out
            vals.append(s + ' ' * (struct.calcsize(fmt) - len(s)))
        elif count == 1:
            vals.append(data.get(name, 0))
        elif data.has_key(name):
            vals += data[name]
        else:
            vals += [0] * count
    return struct.pack(endian + struct_def['packing'], *vals)


def _pack_blue_struct_array(data, struct_def, count, endian='@'):
    """
    Returns the data argument as a string.
    
    Pack the list of dictionaries passed in the data arg to a string
    as described by the struct_def dict arg.  Pads out with filler
    (zeros) if the length of data is less than count.  See
    _pack_blue_struct() for details on the format of struct_def and
    endian.
    """
    buf = ''
    for datum in data[:count]:
        buf += _pack_blue_struct(datum, struct_def, endian)
    return buf + '\0' * (struct_def['nbytes'] * (count - len(data)))


def form_write_path(filename, default_ext=None):
    """
    Returns a datapath in the form of a string.
    
    Return the full path for the given file name as though you were
    going to write to it.  If the xmpyapi module is available the
    read AUX list and write AUX are consulted.  If the write AUX is
    negative then the read AUX list is NOT consulted and the absolute
    value of the write AUX is used.

    If the file name given contains a '~' symbol, the file name is
    replaced with its expanded value.

    If an extension is not given on the file, the optional default_ext
    argument is used (defaults to '.tmp' or '.prm' like X-Midas would).

    We use default_ext=None to indicate 'try both .tmp and .prm files'
    (in that order) ... this is how X-Midas behaves.  If the write aux
    is negative, form_write_path don't do the search behavior (again,
    just like X-Midas) it will simply return the '.tmp' version of the
    file.

    If an X-Midas AUX path begins with a '$' it is assumed to contain
    UNIX environment variable names which are upper-cased and resolved
    (if found) before the path is returned.  Prior to X-Midas 3.8.0
    only the leading path element could be a UNIX environment variable
    and the fully resolved path length was limited to 80 characters.
    """
    # Expand any ~ symbols
    filename = os.path.expanduser(filename)
    
    if os.path.splitext(filename)[-1]:
        # Filename given already contains an extension
        list_of_extensions = [ "" ]
    elif default_ext == None :
        # By default, want to do what X-Midas does; look at both .tmp
        # and .prm files.
        list_of_extensions = ['.tmp', '.prm']  # look in this order
    else :
        # What the user passed in only
        list_of_extensions = [ default_ext ]

    # Filename does not contain a path.  Look up in aux list
    filename_contains_path = (os.path.basename(filename) != filename)
    writeaux_pathname = ""
    a = 0
    if not filename_contains_path and xmpyapi is not None:
        a = xmpyapi.current_auxes()[0]
        writeaux_pathname = xmpyapi.form_path(abs(a), 'w')
    
    # Try all possible extensions to see if this file already exists
    for extension_to_try in list_of_extensions :

        filewext = filename + extension_to_try

        if xmpyapi is not None:
            if filename_contains_path :
                # Filename DOES contain a path
                if os.path.isfile(filewext) :
                    return filewext

            else :
                # Filename does not contain a path.  Look up in aux list
                pathname = os.path.join(writeaux_pathname, filewext)
                # If the write aux is negative do NOT check the read aux list
                # NOTE ALSO: Standard X-Midas practice seems to be to NOT
                # look for a .prm file if the AUX write is negative: it just
                # takes the .tmp
                if a < 0:
                    return pathname
                # Now that we have at least a path to write to, search
                # to see if it already exists on the read paths.  This is
                # what M$ALLOCATE does.
                for a in xmpyapi.current_auxes()[1]:
                    fname = os.path.join(xmpyapi.form_path(a, 'r'), filewext)
                    if os.path.isfile(fname):
                        return fname
                
    # Assertion: never found a filename with any of the given extensions
    result = os.path.join(writeaux_pathname, filename) + list_of_extensions[0]
    return result
    

def pack_header(hdr, structured=0):
    """
    Returns the header in the form of a string.

    Pack the given BLUE header dictionary to its binary format for
    storage on disk.  All keywords, main and extended, are packed
    and updated in the hdr dict that is passed in.

    If you intend on writing this header out to disk, do not forget to
    write out the extended header as well.  It should be written at
    512*hdr['ext_start'] bytes offset from the beginning of the file
    and the extended header itself should be padded out to a multiple
    of 512 bytes.

    If there is an extended header present, setting the optional
    <structured> argument to true (default is false) will cause any
    Python dictionaries, lists, and tuples to be packed with their
    structures intact.  See the doc string for pack_keywords() for
    more details.
    """
    # Leave the user's copy alone
    hdr = hdr.copy()

    # Pack the main header keywords
    pack_main_header_keywords(hdr)

    # We have to pack the adjunct part explicitly because it is a union
    endian = _rep_tran[hdr['head_rep']]
    # Recalculate class from current type (user may have changed)
    file_class = int(hdr['type'] / 1000)
    hdr['adjunct'] = _pack_blue_struct(hdr, _bluestructs['T%dADJUNCT' %
                                                         file_class], endian)

    # Pack the extended header if present
    if hdr.has_key('ext_header'):
        pack_ext_header(hdr, structured=structured)

    return _pack_blue_struct(hdr, _bluestructs['HEADER'], endian)


def pack_main_header_keywords(hdr):
    """
    Replace the 'keywords' field of the given BLUE header dictionary
    with the X-Midas packed (str) form of the main header keywords.
    The order of the key value pairs is indeterminate.

    In packed form, keys are separated from values by a single '=',
    key-value pairs are separated from one another by a single '\0'
    and all values are stringized using str(). Hence, each key value
    pair takes up keylength + stringized value length + 2 characters.
    If the resulting packed string is longer than the max allowed for
    the BLUE header main keywords (96 characters) the string is
    truncated.  If no 'keywords' field is present or if it is an empty
    dict, then the keyword fields are updated to represent an empty
    main header keywords section.
    """
    keydict = hdr.get('keywords', {})
    if keydict:
        hdr['keywords'] = '\0'.join([k + '=' + str(v)
                                     for k,v in keydict.items()]) + '\0'
        hdr['keylength'] = min(len(hdr['keywords']),
                               struct.calcsize(_bluestructs['HEADER']
                                               ['lookups']['keywords'][1]))
        if hdr['keylength'] < len(hdr['keywords']):
            print "WARNING: Main header keywords truncated"
    else:
        hdr['keywords'] = '\0'
        hdr['keylength'] = 0


def _update_t6subrecords(hdr):
    """
    Used in the process of writing a header to disk.  Reads each Type
    6000 subrecord from hdr['subr'], converts them all into one string,
    and places the string into the SUBREC_DEF keyword in the ext header.

    Also scales down each subrecord in hdr['subr'] to the basic fields
    allowed in the main header for X-Midas Type 3000/6000 files:
    name[1:4], format and offset.

    This function expects that the extended header has been read from
    disk and is present in the hdr passed in.
    """
    # Make deep copies of these three data structures so that the
    # user's header passed to writeheader()/write() doesn't get altered,
    # just the shallow copy made by writeheader()/write() does.  Since
    # we only need the copy of hdr['subr'] long enough to cast the
    # subrecords to strings and pack them into the extended header, by
    # not copying it back into hdr['subr'], we can avoid having to cast
    # the 'offset' field of each subrecord back to an integer for when
    # the main header is packed by writeheader()/write().
    import copy
    subrecs = copy.deepcopy(hdr['subr'])
    hdr['keywords']   = copy.deepcopy(hdr['keywords'])
    hdr['ext_header'] = copy.deepcopy(hdr['ext_header'])
    
    # Cast the numeric fields of each subrecord in hdr['subr'] back to
    # the same type of strings X-Midas expects before packing into
    # hdr['ext_header']['SUBREC_DEF'].
    for subrec in subrecs:
        subrec['minval']   = "%+24.17E" % subrec['minval']
        subrec['maxval']   = "%+24.17E" % subrec['maxval']
        subrec['offset']   = "%08i"     % subrec['offset']
        subrec['num_elts'] = "%04i"     % subrec['num_elts']
        subrec['units']    = "%04i"     % subrec['units']
        subrec['uprefix']  = "%0+3i"    % subrec['uprefix']
        subrec['reserved'] = ""

    # Make sure the extended header is in list format 
    if isinstance(hdr['ext_header'], dict):
        hdr['ext_header'] = hdr['ext_header'].items()
    elif isinstance(hdr['ext_header'], str):
        unpack_ext_header(hdr)

    # Pack all of the subrecords into one string
    subrec_def = _pack_blue_struct_array(subrecs,
                                         _bluestructs['T6SUBR_STR_STRUCT'],
                                         hdr['subrecords'],
                                         _rep_tran[hdr['head_rep']])

    # Prepend the 'SUBREC_DEF' key to the beginning of the extended header
    hdr['ext_header'] = [('SUBREC_DEF', subrec_def)] + hdr['ext_header']


def writeheader(filename, hdr, keepopen=0, ext_header_type=list):
    """
    Returns a string [, file object tuple].
    
    Write the given header out to disk.  Returns the full path name to
    the file. If the keepopen parameter is true, an open file handle
    to the file containing the header is also returned. If the data
    portion of the file is in the same file as the header (i.e. it is
    not detached), the open file is returned with the write pointer
    positioned at the first data element in the file.  For detached
    files, you can determine the path to the data file by reading in
    the header with the readheader() method and inspecting the
    'detach_name' field.

    If an 'ext_header' field is present in the header given, the
    extended header is updated on disk.  When this is done for a file
    whose data is not detached, the position of the extended header
    relative to the current values of the 'data_start' + 'data_size'
    fields is also updated and the file is truncated at the end of the
    new extended header. To remove the extended header from a file,
    specify an 'ext_header' field with a value that is an empty string,
    empty list or empty dictionary.

    If there is an extended header present, setting the optional
    <ext_header_type> argument to bluefile.XMTable or
    bluefile.XMKVList (default is list) will cause any Python
    dictionaries, lists, and tuples in the extended header to be
    packed with their structure intact.  See pack_keywords() for more
    details.  Note that if this is the desired behavior,
    <ext_header_type> must be specified each time writeheader() is
    called.  Opening an extended header that had been previously
    packed in this format will not cause it to automatically be packed
    in this format again when it is later written back to disk.  Note
    that there is no difference between specifying bluefile.XMTable or
    bluefile.XMKVList, both are allowed for symmetry with the
    readheader() interface.

    If your write aux is negative, the header will be written to your
    write aux directory, even when writing a header back to disk that
    was read in from an existing file.  Note that this is different
    behavior than X-Midas' headermod command, which modifies a
    header in place and does not consult the aux list.  If your write
    aux is negative and your intention is to write a modified header
    back to an existing file, then you should specify the full path to
    the file in the 'filename' argument.  Specifying the full path
    causes the aux lists to be by-passed, so that a negative write aux
    is inconsequential and the desired behavior is achieved.  This can
    be done by simply passing the modified header's 'file_name' field
    as the 'filename' argument to writeheader:

    # Ensure that hdr is written back to the file from which it was read
    writeheader(hdr['file_name'], hdr)
    
    """
    pathname = form_write_path(filename)

    if os.path.isfile(pathname):
        f = open(pathname, 'r+wb')
    else:
        f = open(pathname, 'wb')
        
    h = hdr.copy()

    if h['class'] == 6:
        _update_t6subrecords(h)
    
    if h.has_key('ext_header'):
        # Determine where the data bytes end.
        if h.get('detached'):
            # Data is in a separate file, the extended header starts
            # right after the header.
            data_end = 512L
        else:
            # Add the .88 in case we are dealing with bit data that
            # doesn't end on byte boundaries.
            data_end = long(h['data_start'] + h['data_size'] + .88)

        # The extended header will start at the next multiple of 512
        # bytes after the last data byte.
        h['ext_start'] = int((data_end + 511) / 512)

        # Write out the updated header
        structured = ext_header_type in (XMTable, XMKVList)
        f.write(pack_header(h, structured=structured))

        # _update_extended_header() expects us to be at the end of the
        # data part of the file. 
        f.seek(data_end)
        _update_extended_header(h, f, structured=structured)
    else:
        f.write(pack_header(h))

    if keepopen:
        if not h.get('detached'):
            # If the data portion is not detached, seek to the first
            # element in the open file we are returning.
            f.seek(long(h['data_start']))
        return pathname, f
    else:
        f.close()
        return pathname



def _extract_t4index(hdr, elements):
    """
    Returns a (list, kwindex) tuple.

    Given a header describing a BLUE type 4000 file which is keyword
    indexed and the number of elements in the file, extract the
    keyword index array and return its position within the extended
    header list of tuples.

    Keyword indexed type 4000 files are supposed to keep the byte
    offset of each element in an array of doubles in the extended
    header.

    This method expects that the extended header has been read from
    disk and is present in the hdr passed in.  It converts the
    extended header into a list of tuples and returns it in this form
    in the 'ext_header' field.  The t4keyword_index is an index into
    this list of tuples indicating where the T4INDEX keyword is
    located.
    """
    unpack_ext_header(hdr)
    ext_header = hdr['ext_header']
    if isinstance(ext_header, dict):
        ext_header = ext_header.items()
    elif isinstance(ext_header, tuple):
        ext_header = list(ext_header)

    t4keyword_index = -1
    t4index = []
    for ii in xrange(len(ext_header)):
        if ext_header[ii][0] == 'T4INDEX':
            if elements == 0:
                del ext_header[ii]
            else:
                t4keyword_index = ii
                t4index = ext_header[ii][1]
                # Convert to a list of floats so we can easily append
                if isinstance(t4index, float):
                    t4index = [t4index]
                elif isinstance(t4index, numpy.ndarray):
                    t4index = t4index.tolist()
            break
    else:
        if elements > 0:
            t4keyword_index = len(ext_header)
            ext_header.append(('T4INDEX', []))

    if hdr['size'] == 0:
        t4index = []
    elif len(t4index) != hdr['size']:
        raise Exception("Type 4000 keyword indexed file does not "
                        "contain a T4INDEX key of the proper length "
                        "in its extended header.")

    hdr['ext_header'] = ext_header

    return t4index, t4keyword_index



def write(filename, hdr=None, data=[], append=0, ext_header_type=list,
          start=None, truncate=True):
    """
    Returns the path to the written file as a string.

    Write data to an X-Midas blue type 1000,2000,3000,4000,5000 or
    6000 file.

    <filename> can be either be the name of a file on disk to write
    or create, or a file-like object to which to write out the given
    header and data.  In the case of a file-like object, it must already
    be opened for both read and write access, and the <hdr> argument must
    be provided.  Writing will begin at the current position of the
    file pointer.  Detached files are not supported for file-like objects.

    If the hdr argument (default None) is empty or None the file
    indicated by the filename argument is opened and edited in place.
    If the append argument (default 0) is False, all of the file's
    existing data is replaced with the data given. If the 'ext_header'
    key is present, the file's keywords are also replaced.  If the
    append argument is True, the data given is appended to the end
    of the existing data.  If the append argument is True and the
    optional start argument is not None, the data given will replace
    the existing data, beginning at the (existing data) element
    indicated by start.  Non-integral values of start are rounded to
    the nearest integer.

    If the truncate mode is False, then whatever values are in data
    will be written to that point in the file, overwriting what was
    there, and leaving any data left in the file (if any) afterwards.
    
    If the hdr argument is a non-empty dictionary, a new file will be
    created by the name given in the filename argument.  Thus note that
    the append and truncate arguments are noops in this case.  The header
    dictionary given is expected to contain all of the fields
    representing a BLUE header (e.g. as returned by read() or
    readheader() in this module.)

    If the 'detached' field is non-zero, then the data is written to a
    separate .det file either in the same directory (<0) or in the
    given aux directory.  If X-Midas is not available to resolve the
    path of the aux directory, you must explicitly pass in a header
    with the 'detach_name' full path to the data portion already
    filled in or else an exception is raised.

    Type 1000 data should be an array of data (or a list of tuples if
    the format is vector) of the proper format.

    Type 2000 data should be a list of arrays of data (or lists of
    tuples if the format is vector) with the proper length and
    format.

    Type 3000/5000/6000 data should be a list of dictionaries whose
    keys match the names of the 3000/5000/6000 subrecords lowercased
    without trailing spaces.  Use the format imported by the read()
    method.  If a corresponding key is not found, numeric fields are
    filled with zeros and strings are filled with spaces.

    Type 4000 data should be a list of [(key,value), ...] lists or a
    list of dictionaries if the order of the keys is unimportant.
    Note that if the 4000 file has a fixed record length (the
    'vrecord_length' is > 0) then supplying a data element that packs
    to a size greater than the fixed length will cause an exception to
    be raised.  All elements prior to this element will still be
    written.  All key names are upper-cased when they are written to
    the BLUE file.
    
    ASCII data written to the data portion of the blue file is padded
    out to its expected length with spaces and is truncated if too
    long.  One string should be supplied per atom, i.e. if the data
    type is '4A', you are expected to supply a single string to fill
    32 bytes of data.

    Bit data must be one large string for type 1000, or a list of
    strings for type 2000.  This module only supports writing bit data
    on natural byte (8 bit) boundaries.

    If <hdr> is not None and it contains an extended header, setting
    the optional <ext_header_type> argument to bluefile.XMTable or
    bluefile.XMKVList (default is list) will cause any Python
    dictionaries, lists, and tuples in the extended header to be
    packed with special key,value pairs to maintain their structures
    (embedded lists, dictionaries, etc.).  To retrieve extended header
    keywords with this structure you must specify bluefile.XMKVList or
    bluefile.XMTable specify <ext_header_type> as bluefile.XMKVList or
    bluefile.XMTable when reading them back in.  See the discussion of
    structured vs unstructured keywords in the doc string for
    pack_keywords() for more details.
    """
    if hasattr(filename, 'read'):
       # We have been given an open, file-like object
        if hasattr(filename, 'mode') and \
           (filename.mode.find('w') < 0 or filename.mode.find('r')) < 0:
            raise Exception("File-like objects must be opened for both "
                            "read and write access")
        if not hdr:
            raise Exception("hdr argument is required for writing "
                            "file-like objects")
        if hdr.get('detached'):
            raise Exception("'detached' feature not supported for writing "
                            "file-like objects")
        pathname = filename
        memory_file = True
    else:
        pathname = form_write_path(filename)
        memory_file = False

    # Can't set both no truncate and append
    if append and not truncate :
        raise Exception("Can't set both no truncate and append at once in write")

    if hdr:
        hdr, fhdr = hdr.copy(), None
        # When writing a file by a new name, we must update the filename
        # in the header before we can resolve the detached name.
        if memory_file:
            hdr['file_name'] = None
        elif 'file_name' not in hdr or pathname != hdr['file_name']:
            hdr['file_name'] = pathname
    else:
        hdr, fhdr = readheader(pathname, keepopen=1)

    structured = ext_header_type in (XMTable, XMKVList)

    # If the detached name cannot be resolved an exception is raised.
    _resolve_detach_name(hdr)

    if not fhdr or not (append or not truncate):
        # The presence of a header and data says, 'use this data and
        # this header' and has precedence over append or truncate flags
        hdr['data_size'] = hdr['size'] = 0
        
        
    
    # Round start to the nearest integral value to match X-Midas' file
    # trimming behavior.
    if start:
        start = round(start)
        
    if (fhdr and start) and (append or not truncate):
        if append :
            elements = (start-1) + len(data)
        elif not truncate :
            # DR: 769110-1: can't write to middle of file without truncate
            # Now, we can! by setting new keyword truncate=True
            if len(data) + (start-1) > hdr['size'] :
                # adding extra data to end, so overwriting bunch of data as
                # well as new data
                elements = len(data) + (start-1)
            else :
                # Keeping old data size, writing in middle of file
                elements = hdr['size']
    else:
        # Note, presence of header overrides the truncate, append options
        elements = hdr['size'] + len(data)
    data_size = float(elements * hdr['bpe'])
    keyword_indexed = (hdr['class'] == 4 and hdr['bpe'] < 0)

    # Determine whether we're going to have to move the extended header
    # If we're a type 4000 with variable length records, assume we will.
    if hdr.get('detached'):
        # The data is in a separate file, the extended header goes right
        # after the header.
        ext_start = 1
    else:
        # The .88 is in case we have bit data, round up to the next byte
        ext_start = int((hdr['data_start'] + data_size + 511.88)/512)

    if hdr.has_key('ext_header'):
        # We've got a new extended header
        pass
    elif fhdr and hdr['ext_size'] > 0:
        # We're updating an existing file and either we need to move its
        # existing extended header, or we need to update it because we're
        # a type 4000 file with keyword indexing.  Load it in.
        if ext_start != hdr['ext_start'] or keyword_indexed:
            # We need to move the existing extended header or we'll
            # need to update it for a type 4000 keyword indexed
            # file. Reread it in before writing over it with new data.
            _read_ext_header(hdr, fhdr)
    else:
        # We have no extended header, or want to remove an existing one
        hdr['ext_header'] = ''
        if hdr.has_key('ext_size'):
            del hdr['ext_size']

    t4index = None
    if keyword_indexed:
        # If it's a type 4000 keyword indexed file, we need to grab
        # these indices from the extended header.  Now that we have
        # the extended header we can do so.
        t4index, t4keyword_index = _extract_t4index(hdr, elements)

    reopen_flag = ''
    if fhdr:
        reopen_flag = 'r+'
        fhdr.close()

    if memory_file:
        fhdr = pathname
    else:
        fhdr = open(pathname, reopen_flag + 'wb')

    if reopen_flag:
        # Skip past existing header
        fhdr.seek(512)
    else:
        # Write our new header to disk with initial guesses of
        # byte offsets and lengths, we'll update it when we finish
        # writing out the data
        hdr['data_start'] = _bluestructs['HEADER']['nbytes']
        hdr['data_size'] = data_size
        fhdr.write(pack_header(hdr, structured=structured))

    if hdr.get('detached'):
        # Close the header file and open the detached data file
        if long(hdr['data_start'] + hdr['data_size']) == 0: reopen_flag = ''
        f = open(hdr['detach_name'], reopen_flag + 'wb')
    else:
        f = fhdr

    if reopen_flag:
        # Skip past data that is already there (if any)
        if start:
            # Skip to where we want to start writing
            start_location = (start - 1) * hdr['bpe']
            f.seek(long(hdr['data_start'] + start_location))
        else:
            # Skip to the end of the data (to append)
            f.seek(long(hdr['data_start'] + hdr['data_size']))
    else:
        # Pad the data start out
        f.write('\0' * long(hdr['data_start'] - f.tell()))

    endian = _rep_tran[hdr['data_rep']]

    try:
        if len(data) > 0:
            pack_data_to_stream(hdr, f, data, endian, t4index)
    finally:

        if not truncate :
            # If in no truncate mode, we don't use WHERE we finished,
            # there may be a bunch of data at the end!  But still
            # make sure we seek to end of data
            hdr['data_size'] = float(elements * hdr['bpe'])
            f.seek(long(hdr['data_start'] + hdr['data_size']))

        else :
            # Update the header with how many bytes we wrote out.  This needs
            # to happen regardless of whether we succeeded in writing out all
            # of the data.  Mark where the data ended and update the extended
            # header. Note that this will round up to the next byte for bit
            # data but we currently only support writes on byte boundaries. 
            hdr['data_size'] = float(f.tell()) - hdr['data_start']
            
        if hdr.get('detached') :
            # If the file is detached, we don't want to be setting this:
            # it was already set to 1 and doing the write thing, otherwise
            # we pad the file too far out (fixed as part of DR: #516015-3)
            pass
        else :
            hdr['ext_start'] = int(float(f.tell() + 511)/512)

        if hdr['class'] == 6:
            _update_t6subrecords(hdr)

        if keyword_indexed and t4keyword_index >= 0:
            # Update the keyword indexed type 4000 files
            if elements > 0:
                hdr['ext_header'][t4keyword_index] = ('T4INDEX', t4index)
            else:
                del hdr['ext_header'][t4keyword_index]
        _update_extended_header(hdr, fhdr, structured=structured)

        # Update the location and size fields in the header.
        fhdr.seek(_bluestructs['HEADER']['lookups']['ext_start'][3])
        fhdr.write(struct.pack(_rep_tran[hdr['head_rep']] + 'iidd',
                               hdr['ext_start'], hdr['ext_size'],
                               hdr['data_start'], hdr['data_size']))
        if hdr['class'] == 4:
            # Type 4000 files store the number of elements in the adjunct,
            # since it cannot always be computed from 'data_size'.
            fhdr.seek(_bluestructs['HEADER']['lookups']['adjunct'][3] +
                      _bluestructs['T4ADJUNCT']['lookups']['nrecords'][3])
            fhdr.write(struct.pack(_rep_tran[hdr['head_rep']] + 'i',
                                   elements))

        # Truncate detached files
        if f != fhdr:
            f.truncate()

        # Truncate to 512 bytes like X-Midas does: fixed as part
        # of DR: #516015-3
        if 0:   # You may or may not want this feature
            f.seek(0, 2)               # Get to end of file
            length_of_file = f.tell()  # Figure where we are
            remainder  = length_of_file % 512
            pad_to_512 = 512 - remainder
            if remainder != 0 : f.write('\0' * pad_to_512) 

        if not memory_file:
            f.close()

    return pathname


def pack_data_to_stream(hdr, f, data, endian='@', t4index=None):
    """
    Given a BLUE header dictionary (hdr), and an object with a
    .write() method (f) (typically a file object), and structured data
    (see the help for write() for information on how the data should
    be structured for the different BLUE file types), pack the
    structured data into bytes according to the structure described in
    'hdr', and write it to the stream 'f'.

    The endian argument (default '@') is the struct.pack() format
    character specifying the endianness of the data to be packed
    ('<' == EEEI, '>' == IEEE, '@' == native).
    """
    file_class = hdr['class']
    if file_class not in (1,2,3,4,5,6):
        raise Exception("File write of class %d not yet supported" %
                        file_class)

    # Write out the new data
    if file_class <= 2:
        packing = _xm_to_struct[hdr['format'][-1]]
        ape = hdr['ape']
        spa = hdr['spa']
        bpe = hdr['bpe']
        # Pretend 1000 data is one big frame, it all packs the same
        if file_class == 1:
            ape = len(data)
            bpe = hdr['bpa'] * ape
            data = [data]

        if packing.endswith('s'):
            if packing.startswith('.125'):
                # Bit data
                bpe = bpe * 8
                if ((hdr['data_start']+hdr['data_size']) % 1 > .1 or
                    bpe % 1 > .1):
                    raise Exception('Write of BIT data on non-byte '
                                    'boundaries not supported')
                bpe = long(bpe)
                for frame in data:
                    if not isinstance(frame, str):
                        raise Exception('Bit data given for write '
                                        'must be a string')
                    if len(frame) != bpe:
                        raise Exception('Bit data string given without '
                                        'proper length')
                    f.write(frame)
            else:
                # ASCII data
                bpa = hdr['bpa']
                for frame in data:
                    if len(frame) != ape:
                        raise Exception('Type 2000 data given without '
                                        'proper dimensions')
                    for elem in frame:
                        # Space pad out
                        f.write(elem + ' ' * (bpa - len(elem)))
        else:
            try: 
                # Numeric data
                # Retranslate format to NumPy, not struct
                packing = _xm_to_numpy[hdr['format'][-1]]
                if hdr['format'] in ('CD','CF'):
                    packing = _complex_type[packing]
                    spa = 1
                # Determine what the shape of each frame should be
                shape = (ape,)
                if spa != 1: shape += (spa,)

                for frame in data:
                    if not isinstance(frame, numpy.ndarray):
                        frame = numpy.array(frame, packing)
                    if numpy.shape(frame) != shape:
                        raise Exception('Type %d000 data given without '
                                        'proper dimensions: expected %s, '
                                        'got %s' % (file_class, shape,
                                                    numpy.shape(frame)))
                    if endian != '@'  and  endian != _native_endian:
                        f.write(frame.astype(packing).byteswapped().tostring())
                    else:
                        f.write(frame.astype(packing).tostring())
            except KeyError:
                # Unknown X-Midas data type.
                raise Exception('Unrecognized data format %s' %
                                (hdr['format'],))

    elif file_class in (3,5,6):
        # data is a list of dictionaries
        struct_def = _blue_subrecord_map(hdr)
        for d in data: f.write(_pack_blue_struct(d, struct_def, endian))

    elif file_class == 4:
        # Build a list of keyword lists [[(k,v), (k,v,), ...], ...]
        # The first 8 bytes of each type 4000 record on disk is a
        # VRB record header (the first 8 bytes of a VRBSTRUCT).
        vpacking = endian + _truncate_struct_def(
            _bluestructs['VRBSTRUCT'], 8)['packing']
        bpe = hdr['bpe']
        lkey = bpe - 8
        elements = hdr['size']
        for d in data:
            vdat = pack_keywords(d, endian, ucase=1)
            nvalid = len(vdat)
            if bpe <= 0:
                # Variable length record, no padding necessary
                if t4index is not None:
                    t4index.append(float(f.tell()))
            elif nvalid < lkey:
                # Fixed length, pad entire write out to bpe
                vdat += '\0' * (lkey - nvalid)
            elif nvalid > lkey:
                # Fixed length and too many bytes to write out
                print ('WARNING: VRB at index %d truncated by %d bytes '
                       'to fit fixed record length of %d bytes: %s' %
                       (elements, nvalid-lkey, bpe, str(d)))
                nvalid = lkey
                vdat = vdat[:lkey]

            f.write(struct.pack(vpacking, len(vdat), nvalid))
            f.write(vdat)
            elements += 1


def _update_extended_header(hdr, f, structured=0):
    """
    Writes out the extended header (if any) at the location indicated
    by the 'ext_start' key in the header.  Expects that the file
    object f passed in is open for write and is currently positioned
    at the end of any data.

    As per the X-Midas BLUE file format: if necessary the file is
    padded out to the start of the extended header with NULLs (the
    extended header starts at a hdr['ext_start']*512 bytes from the
    beginning of the file), the extended header is written, further
    padding is added to make the extended header a multiple of 512
    bytes, then the file is truncated.  If no 'ext_header' key is
    present and 'ext_size' is <= 0 the file is truncated at its
    current position. If no 'ext_header' key is present and 'ext_size'
    is > 0, nothing is done to the file.

    The optional <structured> argument indicates whether the extended
    header keywords should have any structure tags interpreted embedded
    in the packed keywords.  See pack_keywords() for more info.
    """
    if hdr.has_key('ext_header'):
        pack_ext_header(hdr, structured=structured)
        if len(hdr['ext_header']) > 0:
            # Pad out to where the extended header is supposed to start
            f.write('\0' * (hdr['ext_start'] * 512 - f.tell()))
            # Write out the extended header...
            f.write(hdr['ext_header'])
            # ...and pad out to multiple of 512 bytes
            leftover = f.tell() % 512
            if leftover: f.write('\0' * (512 - leftover))
        # Truncate
        f.truncate()
    elif hdr['ext_size'] <= 0:
        # No extended header, truncate at end of data
        f.truncate()


def read(filename, ext_header_type=None, start=None, end=None,
         fstart=None, fend=None, blocksize=16384):
    """
    Returns a (header, data) tuple for type 1000,2000,3000,4000,5000
    and 6000 BLUE files.

    <filename> can be either be the name of a file on disk, or a
    file-like object with the file pointer already positioned to the
    beginning of a bluefile header structure.  Detached files are
    not supported for file-like objects.

    All ASCII data atoms with multiple scalars (e.g. 2A, XA, etc.) are
    treated as single string entities and are returned with trailing
    spaces and nulls removed.

    All arrays of values are returned as NumPy arrays.  Complex
    floats and doubles are supported directly by NumPy.  All other
    complex integer types and all scalars/atom > 2 are expressed by
    shaping the returned NumPy array.
    
    Type 1000 data is returned as a NumPy array as described above.
    ASCII data is returned as a list of strings.

    Type 2000 framed data is returned as a list of NumPy arrays as
    described above, though it is possible to change this behavior with
    the method set_type2000_format().  ASCII framed data is returned as
    a list of lists of strings.

    Type 3000/5000/6000 record oriented data is returned as a list of
    dictionaries with the lower-cased subrecord names as key names.

    Type 4000 (keyword block) data is returned as a list of lists of
    (key,value) pairs with all key names lower-cased.

    If <ext_header_type> is not None (the default), open the extended
    header and return it in the 'ext_header' field of the returned
    header dictionary.  Possible values are list, dict,
    bluefile.XMKVList or bluefile.XMTable.  See the help for
    readheader() for more info.

    Optional 'start' and/or 'end' arguments can be used to return
    a specified portion of the data.  If 'start' and/or 'end' are
    given, elements are read from the filename starting at 'start'
    and going to 'end', using X-Midas file trimming semantics
    (1-based elements, range is inclusive, rounded to nearest
    integer).  'start' defaults to 1, and 'end' defaults to
    hdr['size'].  'start' must be at least 1, and cannot be greater
    than the total number of data elements in the file.  'end' must
    be at least 1, and if it is greater than the number of data
    elements in the file, only the data elements up to the end of
    the file are returned.  Data trimming is not supported for Type
    4000 variable length keyword files.

    If neither 'start' nor 'end' are given, the default behavior is
    to read and return the entire file.

    For type 2000 files, optional 'fstart' and/or 'fend' arguments
    can be used to trim the data within each frame.  If 'fstart'
    and/or 'fend' are given, each frame is trimmed to samples
    fstart:fend (1-based indexing, range is inclusive).  'fstart'
    defaults to 1, and 'fend' defaults to hdr['subsize'].  'fstart'
    must be at least 1, and cannot be greater than the frame size.
    'fend' must be at least 1, and if it is greater than the frame
    size, only the samples up to the end of each frame are returned.

    If neither 'fstart' nor 'fend' are given, the default behavior is
    to read and return the entire frame for each element.

    When 'trimmed' data is returned, the fields in the header returned
    by read() are adjusted to describe the trimmed data, not the
    original data.

    Data is read in blocks of <blocksize> bytes, rounded down to the
    nearest number of elements.  The default size is 16K bytes.  Smaller
    block sizes reduce the memory usage but tend to reduce performance.
    For most applications, block sizes greater than 16K yield minimal
    performance gains while increasing the memory footprint.
    """
    # Need to fix this so that it returns:
    # (1) graceful abort if file too large
    hdr, f = readheader(filename, ext_header_type, keepopen=1)

    # Check to see if data part of the file is detached
    if hdr.get('detached'):
        # If the detached name cannot be resolved an exception is raised.
        if 'detach_name' not in hdr: _resolve_detach_name(hdr)
        f = open(hdr['detach_name'], 'rb')
        if hdr['data_start'] > 0:
            f.seek(long(hdr['data_start']))

    if start is None and end is None:
        elements = 'all'
    else:
        # Round the start and end to the nearest integral values to match
        # X-Midas' trimming behavior.
        if start:
            start = int(round(start))
        if end:
            end = int(round(end))
        
        if start is None:
            start = 1
        elif start > hdr['size']:
            raise Exception, "start value given is > than data size"
        elif start < 1:
            raise Exception, "start value must be >= 1"
        elif hdr['class'] == 4 and hdr['bpe'] <= 0:
            raise Exception, "Cannot trim variable length keyword data"
        elif hdr['class'] == 1:
            # Adjust the 'xstart' field to reflect the data being returned
            hdr['xstart'] = hdr['xstart'] + ((start-1) * hdr['xdelta'])
        elif hdr['class'] == 2:
            # Adjust the 'ystart' field to reflect the data being returned
            hdr['ystart'] = hdr['ystart'] + ((start-1) * hdr['ydelta'])
                
        if end is None or end > hdr['size']:
            end = hdr['size']
        elif end < 1:
            raise Exception, "end value must be >= 1"

        elements = end - start + 1
        if elements <= 0:
            raise Exception, "Must choose a positive number of elements"

        # From our current position in the file, seek to our 'start' element
        f.seek((start - 1) * hdr['bpe'], 1)
        
    endian = _rep_tran[hdr['data_rep']]
    data = unpack_data_from_stream(hdr, f, elements, endian, fstart, fend,
                                   blocksize)
    return hdr, data


def unpack_data_from_stream(hdr, f, elements='all', endian='@', fstart=None,
                            fend=None, blocksize=16384):
    """
    Returns a list of elements from object f.

    Given the dictionary representation of a header (hdr), and an
    object with a .read() method (f) (typically a file object), read
    the given number of elements from 'f' and return them as unpacked,
    structured Python data.  It is expected that the given file object
    is properly positioned at the beginning of the elements that are
    to be read.

    If the elements arg (default 'all') is the string 'all', read all
    possible elements from the file; otherwise, read only the given
    number.

    For Type 2000 files, if fstart and/or fend arguments are given,
    each frame is trimmed to [fstart:fend] (1-based indexing, range
    is inclusive) before being returned.  See the help for read()
    for full details regarding these two optional arguments.

    See the help for the write() method in this module for a description
    of the structure of the data returned by this method based on the
    type of the BLUE file being read in.

    The endian argument (default '@') is the struct.unpack() format
    character specifying the endianness of the data to be unpacked
    ('<' == EEEI, '>' == IEEE, '@' == native).

    Data is read in blocks according to <blocksize>, which defaults to
    16K.  See the help for read() for full details regarding the use
    of <blocksize>.
    """

    if elements == 'all':
        elements = hdr['size']
        trim = False
    else:
        trim = True
        
    # File type specific conversions
    #
    # element = an element (t1000), a frame (t2000), a record (t3000)
    # atom = a complex float, a vector double, a scalar long
    # scalar = a float, a double, a long
    file_class = hdr['class']
    bpe = hdr['bpe'] # bytes per element

    if file_class <= 2:
        if hdr['format'][1] == 'P':
            packing = '.125s'
        else:
            packing = _xm_to_struct[hdr['format'][1]]
        spa = hdr['spa']
        ape = hdr['ape']
        bpa = hdr['bpa']
        bps = hdr['bps']

        if packing.endswith('s'):
            if file_class == 1:
                # Start pretending 1000 data is one big frame to reuse code
                bpe = bpa * elements
                ape = elements
                elements = 1

            if packing.startswith('.125'):
                # Bit data
                if max(hdr['data_start'] % 1, bpe % 1) > .1:
                    raise Exception('Read of BIT data on non-byte '
                                    'boundaries not supported')
                bpe = long(bpe)
                pydata = [f.read(bpe) for ii in xrange(elements)]
            else:
                # ASCII data, read and trim BPA at a time
                pydata = [[_m_length(f.read(bpa))
                           for jj in xrange(ape)]
                          for ii in xrange(elements)]

            if file_class == 1:
                # Stop pretending 1000 data is one big frame
                bpe = hdr['bpe']
                elements = ape
                pydata = pydata[0]
        else:
            try:
                packing = _xm_to_numpy[hdr['format'][1]]
                if hdr['format'] in ('CD','CF'):
                    # Use the complex formats supported directly by NumPy
                    packing = _complex_type[packing]
                    spa = 1
                    bps *= 2
            except KeyError:
                # Unknown X-Midas data type.
                raise Exception('Unrecognized data format %s' %
                                (hdr['format'],))

            # Pre-allocate the output array as a 1d array of the base
            # scalar type (array will be reshaped later).
            try:
                scalars = int(elements * ape * spa)
                pydata = numpy.zeros(scalars, packing)
            except OverflowError:
                raise MemoryError('File size exceeds maximum memory size; ' +
                                  'use file trimming')

            # Convert blocksize down to nearest scalar boundary (but must
            # be non-zero).
            scalars_per_pass = max(1, min(int(blocksize/bps), scalars))
            blocksize = scalars_per_pass * bps

            unpack_data_block = _unpack_data_block
            swap_bytes = (endian != '@' and endian != _native_endian)

            # Read the data into the output array block-by-block.
            scalars_read = 0
            while scalars_read < scalars:
                pydata[scalars_read:scalars_read+scalars_per_pass] = \
                       unpack_data_block(f, blocksize, packing)
                scalars_read += scalars_per_pass
                if scalars_per_pass > scalars - scalars_read:
                    scalars_per_pass = scalars - scalars_read
                    blocksize = scalars_per_pass * bps

            # If the endian-ness does not match (checked above), byteswap the
            # array.
            if swap_bytes:
                pydata = pydata.byteswapped()

            # Determine the appropriate shape of the output array based on
            # the frame size (type 2000) and and data type (i.e. scalars per
            # atom).
            shape = (elements,)
            if file_class == 2:
                shape += (ape,)
            if spa > 1:
                shape += (spa,)

            # Reshape the output array to match the expected shape.
            if len(shape) > 1:
                pydata = numpy.reshape(pydata, shape)

        if file_class == 2 and (fstart is not None or fend is not None):
            # A Type 2000 file that we need to trim in the 2nd dimension
            trim = True
            if fstart is None:
                fstart = 1
            elif fstart > hdr['subsize']:
                raise Exception, "fstart value given is > than the frame size"
            elif fstart < 1:
                raise Exception, "fstart value must be >= 1"
            else:
                # Adjust the xstart to reflect the data being returned
                hdr['xstart'] = hdr['xstart'] + ((fstart-1)*hdr['xdelta'])

            if fend is None or fend > hdr['subsize']:
                fend = hdr['subsize']
            elif fend < 1:
                raise Exception, "fend value must be >= 1"

            subsize = fend - fstart + 1
            if subsize <= 0:
                raise Exception, "Must choose a positive range within frame"

            # Trim each frame to the desired size
            if isinstance(pydata, numpy.ndarray):
                # Arrays support multiple-axis slicing
                pydata = pydata[:,fstart-1:fend]
            else:
                for ii in xrange(elements):
                    pydata[ii] = pydata[ii][fstart-1:fend]

            # Update hdr['subsize'] and bpe to reflect the trimmed frame size
            hdr['subsize'] = subsize
            bpe = bpa * subsize

        # Convert type 2000 data to a list of arrays for backwards
        # compatibility.
        if file_class == 2 and _type2000_format == list:
            pydata = [ frame for frame in pydata ]

    elif file_class in (3,5,6):
        # Build a list of dictionaries
        # NB: Coerce the number of bytes to read to a long to avoid
        #     deprecation warnings from read().
        pydata = _unpack_blue_struct_array(f.read(long(bpe * elements)),
                                           _blue_subrecord_map(hdr),
                                           elements, endian)
    elif file_class == 4:
        # Do a sanity check on the header before reading any data
        if hdr['size'] == 0 and hdr['data_size'] > 0:
            raise ValueError('Corrupted header - unable to read data')
        elif hdr['size'] > 0 and not hdr['data_size']/hdr['size'] >= 8:
            raise ValueError('Corrupted header - unable to read data')

        # Build a list of keyword lists [[(k,v), (k,v,), ...], ...]
        # The first 8 bytes of each type 4000 record on disk is a
        # VRB record header (the first 8 bytes of a VRBSTRUCT).
        pydata = []
        vpacking = endian + _truncate_struct_def(
            _bluestructs['VRBSTRUCT'], 8)['packing']
        for ii in xrange(elements):
            lblock, lvalid = struct.unpack(vpacking, f.read(8))
            pydata.append(unpack_keywords(f.read(lvalid), endian, lcase=1))
            if lblock > lvalid: f.seek(long(lblock-lvalid), 1)
        if trim:
            hdr['nrecords'] = len(pydata)

    if trim:
        # Update hdr['data_size'] to reflect trimmed data.  All other
        # header fields will be updated by update_header_internals()
        hdr['data_size'] = bpe * elements
        update_header_internals(hdr)
          
    return pydata


def _unpack_data_block(f, blocksize, packing):
    """
    Private method to read a block from a file into a NumPy array.
    """
    return numpy.fromstring(f.read(blocksize), packing)


def unpack_ext_header(hdr, structured=0):
    """
    Unpacks the extended header associated with the given header to a
    a list of (key, value) tuples.  The header given must have an
    'ext_header' key whose value is a raw string representing a packed
    BLUE extended header whose data fields are packed in the same
    machine representation as the 'head_rep' key.

    If <structured> is true then any embedded structure tags in the
    X-Midas keywords denoting heterogeneous lists and dictionaries are
    interpreted.  These structure tags are embedded by calling
    pack_ext_header() with the same structured=1.  See pack_keywords()
    for more info.
    """
    if isinstance(hdr['ext_header'], str):
        hdr['ext_header'] = unpack_keywords(hdr['ext_header'],
                                            _rep_tran[hdr['head_rep']],
                                            structured=structured)


def unpack_keywords(buf, endian='@', lcase=0, start=0, structured=0):
    """
    Returns keywords as a list of (key, value) tuples. 
    
    Unpack the given block of packed X-Midas keywords in the string
    arg buf into a list of (key, value) tuples.  If <lcase> is true
    (default 0), lower-case the key names in the returned list.

    <endian> (default '@') is the struct.unpack() format character
    specifying the endianness of the data to be unpacked ('<' == EEEI,
    '>' == IEEE, '@' == native).

    <start> indicates that you wish to start processing at buf[start].

    If <structured> is true, any structure tags found in the packed
    X-Midas keyword buf (presumably previously embedded by a call to
    pack_keywords()) are removed and used to reproduce the structure

    """
    lbuf = len(buf)
    byteswap = (endian != '@' and endian != _native_endian)
    keywords = []

    oldstyle = lbuf >= 8 and struct.unpack(endian + 'i', buf[4:8])[0] <= 128
    if oldstyle:
        # When extended header keywords were first introduced they were
        # only ASCII and the keyword header was 3 integers (knext, ltag,
        # ldata) followed directly by the tag name padded out to a multiple
        # of 4 bytes, followed directly by the data string.
        kpacking = endian + 'iii'
        format = 'A'
    else:
        kpacking = endian + _bluestructs['KEYSTRUCT']['packing']

    ii = start
    while ii < lbuf:
        if oldstyle:
            itag = ii + 12
            lnext, ltag, ldata = struct.unpack(kpacking, buf[ii:itag])
            # DR 531828-3: Not only should the next keyword offset be at least
            # as great as the current offset, it must point past at least the
            # header for the current keyword.
            if lnext < itag or lnext > lbuf:
                raise ValueError('Bad extended header conversion at offset %d'
                                 % ii)
            lkey = lnext - ii
            idata = itag + 4 * int((ltag+3)/4)
        else:
            # The index of the data is directly after 8-byte keyword header
            idata = ii + 8
            # lkey = entire length of keyword block: tag, data, kwhdr & padding
            # lextra = length of the keyword header, tag & padding
            # ltag   = length of just the tag
            lkey, lextra, ltag, format = struct.unpack(kpacking, buf[ii:idata])
            ldata = lkey - lextra
            # Index of the tag = the index of the data + the data length
            # (the tag is written directly after the data)
            itag = idata + ldata

            # Do a sanity check on keyword meta-data before trying to unpack
            # the keyword.  If the current index in the buf + the recorded
            # length of this keyword block is more than the length of the
            # buffer, the keyword information is corrupt.
            # DR 531828-3: The length of the data must also be non-negative,
            # otherwise an infinite loop will result.
            if ldata < 0 or ii + lkey > lbuf:
                raise ValueError('Corrupted header - unable to read keywords')

        tag = buf[itag:itag+ltag]
        data = buf[idata:idata+ldata]

        if format != 'A':
            try:
                if isinstance(data, str):
                    data = numpy.fromstring(data, _xm_to_numpy[format])
                    if byteswap:
                        data = data.byteswapped()
                    if len(data) == 1:
                        data = data[0]
                        if format == 'X':
                            # Coerce type 'X' keywords into longs so they
                            # are written back as 'X'.
                            data = long(data)
            except KeyError:
                # Unknown X-Midas data type.
                raise Exception('Unrecognized keyword format %s' % format)
        elif data == '<XN>':
            data = None
        elif not structured:
            pass
        elif ldata == 4 and data[0] == '<' and data[-1] == '>':
            delim = data[1:-1]
            if delim in ('XT', 'XL', 'XK'):
                data = unpack_keywords(buf, endian, lcase, ii+lkey, structured)
                if data and data[0][0] == '<XS>':
                    # Quietly discard the size tag.
                    data = data[1:]
                if data and isinstance(data[-1][0], int):
                    lkey, edelim = data.pop()
                    lkey = lkey - ii
                    if edelim != delim:
                        warnings.warn('Mismatched delimiters %(tag)s='
                                      '<%(delim)s> and </%(edelim)s>' % locals)
                if delim == 'XL':
                    data = [v for k,v in data]
                elif delim == 'XT':
                    data = dict(data)

        elif ldata == 0 and ltag == 5 and tag[0] == '<' and tag[-1] == '>':
            if tag in ('</XT>', '</XL>', '</XK>'):
                keywords.append((ii+lkey, tag[2:-1]))
                return keywords

        if lcase: tag = tag.lower()
        keywords.append((tag, data))
        ii += lkey

    return keywords


def _is_kvpair_list(value):
    """
    Returns whether the given value is a [(key,value),...] list
    such as one returned by dict.items().  Looks at the first element
    in the list, which must be a tuple with 2 items, the first of which
    is a string.  If the list is empty, returns false.
    """
    return (isinstance(value, list) and value and
            isinstance(value[0], tuple) and len(value[0]) == 2 and
            isinstance(value[0][0], str))

        
def pack_ext_header(hdr, structured=0):
    """
    Packs the value of the given BLUE file hdr dictionary's
    'ext_header' key into the BLUE file extended header format and
    updates the value of 'ext_size'.  The value of 'ext_header' can be
    a list of (key, value) tuples or a dict.  Before writing this out
    to disk at the end of a BLUE file it must be padded out to a
    multiple of 512 bytes.  If the keywords given are already a string
    it is presumed they are already packed and the 'ext_size' field is
    updated but the string itself is left alone.

    If <structured> is true, any embedded Python dictionaries, lists
    or tuples will pack their structure into the keywords with
    them. See pack_keywords() for more info.
    """
    packed = pack_keywords(hdr['ext_header'], _rep_tran[hdr['head_rep']],
                           structured=structured)
    hdr['ext_header'] = packed
    hdr['ext_size'] = len(packed)


def pack_keywords(keywords, endian='@', ucase=0, structured=0):
    """
    Packs the given keywords into a single string, ready to send to an
    X-Midas extended header.  The keywords arg can be a Python
    dictionary, a list of (key, value) tuples or a raw byte string.
    If it is already a string, it is passed back unmodified.  If the
    ucase arg (default 0) is true then the key names are upper-cased
    when they are packed.

    The endian argument (default '@') is the struct.unpack() format
    character specifying the endianness of the data to be packed
    ('<' == EEEI, '>' == IEEE, '@' == native).

    Warning: All key names are converted to strings before being
    packed and will be unpacked as strings on subsequent calls to
    unpack_ext_header (X-Midas only recognizes string keys).

    If <structured> is false (the default), all lists or tuples are
    converted to NumPy arrays (raising an Exception on failure) then
    packed as an X-Midas atomic result (i.e. a single key-value pair
    whose value is an array of like-typed numbers).  Python ints and
    floats are converted to 'L' and 'D' types respectively.  All other
    object types (including dictionaries and complex()) are converted
    to strings and stored as ASCII values (and will be returned as
    strings when the block is unpacked later).

    If <structured> is true, all values that are Python dicts, lists
    or tuples are packed with extra tag-value pairs to maintain their
    structure and each value within them is then packed individually
    using the same algorithm (recursively).  If you want to pack an
    atomic array of numeric elements as a single tag-value pair, the
    value must be a NumPy or array module array.  Complex values are
    packed as a 2 element NumPy array, and when extracted are
    returned as such; X-Midas keyword format currently has no way of
    differentiating between a complex value and an atomic array of 2
    numeric values.

    If these packed keywords are then unpacked by calling
    unpack_keywords(..., structured=1), the returned keywords will
    contain any embedded dicts, lists, tuples as Python dicts and
    lists.  This packed structured format is also understood by the
    XMValue C++ class (note that currently, converting a section with
    repeated key names to a dictionary is not allowed by the XMValue
    class).

    Note that these *structured* packed keywords are in legal X-Midas
    keyword format (completely backwards compatible) with additional
    tag-value pairs to describe their structure.  These keywords can
    still be read by all regular X-Midas keyword utilities.  You can
    therefore unpack them into the classic X-Midas key-value pair list
    format which will contain these extra structure tags.  The order
    of these tags in the list is important.  Converting this
    non-structured list to a dictionary and then packing it again will
    cause the order of these extra tags to change and therefore cause
    any future attempt at correctly unpacking them with structured
    format impossible.
    """
    kpacked = ''
    byteswap = (endian != '@' and endian != _native_endian)

    if isinstance(keywords, str):
        return keywords
    elif isinstance(keywords, dict):
        keywords = keywords.items()

    kpacking = endian + _bluestructs['KEYSTRUCT']['packing']

    for tag, value in keywords:
        tag = str(tag)
        if ucase: tag = tag.upper()
        ltag = len(tag)

        packing = kpacking

        # Python's struct module has difficulty with the special IEEE
        # values NaN and Inf (and their variations) when an endian-ness
        # is specified. Since NumPy can handle these values, create
        # a temporary array to pack 'D' keywords.
        if isinstance(value, float):
            value = numpy.array([value])
        elif isinstance(value, complex):
            # Header keywords do not directly support complex numbers, so
            # the best thing to do is to convert a complex to a 2-element
            # array.
            value = numpy.array([value.real, value.imag])

        if isinstance(value, (int,long)):
            # On 64-bit platforms, a Python int is also 64 bits, so treat
            # the value as 'L' if it fits into 32 bits, and 'X' if it does
            # not (performance is not critical here).
            if isinstance(value, int) and 2**31 > value >= -2**31:
                format = 'L'
                ldata = 4
                packing += 'i'
            else:
                format = 'X'
                ldata = 8
                packing += 'q'
        elif isinstance(value, str):
            format = 'A'
            ldata = len(value)
            packing += str(ldata) + 's'
        elif structured and isinstance(value, dict):
            kpacked += pack_structured(tag, 'XT', value.items(), endian, ucase)
            continue
        elif structured and isinstance(value, (list, tuple)):
            if _is_kvpair_list(value):
                kpacked += pack_structured(tag, 'XK', value, endian, ucase)
            else:
                value = [('#', v) for v in value]
                kpacked += pack_structured(tag, 'XL', value, endian, ucase)
            continue
        else:
            # Old way (i.e., NOT table format) of handling tuples, lists,
            # dicts, arrays, complex, etc.
            if (isinstance(value, (list, tuple)) or
                str(type(value)) == "<type 'array.array'>"):
                value = numpy.array(value)

            if isinstance(value, numpy.ndarray):
                typecode = value.dtype.type
                if not _numpy_to_xm.has_key(typecode):
                    # Convert to an X-Midas safe, nearly equivalent type
                    typecode = _numpy_xm_promotion[typecode]
                    value = value.astype(typecode)
                format = _numpy_to_xm[typecode]
                if byteswap:
                    value = value.byteswapped()
                value = value.tostring()
            elif structured and value is None:
                format = 'A'
                value = '<XN>'
            else:
                # All other types
                format = 'A'
                value = str(value)

            ldata = len(value)
            packing += str(ldata) + 's'
        
        lkey = int((ldata + ltag + 15) / 8) * 8
        lextra = lkey - ldata
        lltag = len(tag)
        packing += str(lltag) + 's' + ('x' * (lextra - lltag - 8))
        
        kpacked += struct.pack(packing, lkey, lextra, ltag, format, value, tag)

    return kpacked


def pack_structured(tag, type, keywords, endian='@', ucase=0):
    """
    Packs Python dictionaries, lists and tuples into a string in
    serialized XMValue format, using open (e.g. '<XT>'), size ('<XS>')
    and close (e.g. '</XT>') tags.  The <type> argument should be one
    of the two-character tag strings 'XT', 'XL' or 'XK' for tables,
    lists and key-value lists, respectively.

    See pack_keywords() for more info.
    """
    # Pack the contents and add the close tag.
    kpacked = pack_keywords(keywords + [('</'+type+'>','')], endian, ucase,
                            structured=1)
    
    # Calculate the length of the packed data including the size tag
    # (which will be actually prepended once the size is known).
    bytes = len(kpacked) + len(pack_keywords([('<XS>',0)], endian, ucase))
    kpacked = pack_keywords([('<XS>',bytes)], endian, ucase) + kpacked

    # Return the packed data with a start tag prepended.
    return pack_keywords([(tag, '<'+type+'>')], endian, ucase) + kpacked


def _init_bluestructs ():
    bstructs = {
        'SUBRECSTRUCT': {
          'fields': ( ('name', '4s', 1, 0),
                      ('format', '2s', 1, 4),
                      ('offset', 'h', 1, 6) ),
          'nbytes': 8,
          'npacking': 3,
          'packing': '4s2sh' },

        'T6SUBR_STR_STRUCT': {
          'fields': ( ('name',    '24s', 1,  0),
                      ('minval',  '24s', 1, 24),
                      ('maxval',  '24s', 1, 48),
                      ('offset',   '8s', 1, 72),
                      ('num_elts', '4s', 1, 80),
                      ('units',    '4s', 1, 84),
                      ('format',   '2s', 1, 88),
                      ('uprefix',  '3s', 1, 90),
                      ('reserved', '3s', 1, 93) ),           
          'nbytes': 96,
          'npacking': 9,
          'packing': '24s24s24s8s4s4s2s3s3s' },

        'KEYSTRUCT': {
            'fields': ( ('lkey', 'i', 1, 0),
                        ('lext', 'h', 1, 4),
                        ('ltag', 'b', 1, 6),
                        ('type', 'c', 1, 7) ),
            'nbytes': 8,
            'npacking': 4,
            'packing': 'ihbc' },

        'COMPSTRUCT': {
            'fields': ( ('name', '4s', 1, 0),
                        ('format', '2s', 1, 4),
                        ('type', 'b', 1, 6),
                        ('units', 'b', 1, 7) ),
            'nbytes': 8,
            'npacking': 4,
            'packing': '4s2sbb' },

        'VRBSTRUCT': {
            'fields': ( ('fsize', 'i', 1, 0),
                        ('size', 'i', 1, 4),
                        ('buf_loc', 'i', 1, 8),
                        ('buf_len', 'i', 1, 12),
                        ('record_len', 'i', 1, 16),
                        ('next', 'i', 1, 20) ),
            'nbytes': 24,
            'npacking': 6,
            'packing': 'iiiiii' },

        'T5QUADWORDS': {
            'fields': ( ('frame_of_ref', '8s', 1, 0),  # coordinate ref frame
                        ('altitude', 'd', 1, 8),   # (meters) for topocentric
                        ('latitude', 'd', 1, 16),  # (deg) for topocentric
                        ('longitude', 'd', 1, 24), # (deg) for topocentric
                        ('azimuth', 'd', 1, 32),   # (deg) for topocentric
                        ('elevation', 'd', 1, 40), # (deg) for topocentric
                        ('roll', 'd', 1, 48),      # (deg) for topocentric
                        ('notused1', 'd', 1, 56),
                        ('notused2', 'd', 1, 64),
                        ('epoch_year', 'd', 1, 72),    # for ECI
                        ('epoch_seconds', 'd', 1, 80), # for ECI
                        ('hour_angle', 'd', 1, 88) ),  # GW angle (rad) at 
                                                       # epoch f ECI
            'nbytes': 96,
            'npacking': 12,
            'packing': '8sddddddddddd' } }


    bstructs.update({
        'T1ADJUNCT': {
            'fields': ( ('xstart', 'd', 1, 0),
                        ('xdelta', 'd', 1, 8),
                        ('xunits', 'i', 1, 16),
                        ('fill1', 'i', 1, 20),
                        ('fill2', 'd', 1, 24),
                        ('fill3', 'd', 1, 32),
                        ('fill4', 'i', 1, 40),
                        ('bid', 'i', 1, 44) ),
            'defaults': { 'xdelta': 1.0,
                          'fill3': 1.0 },
            'nbytes': 48,
            'npacking': 8,
            'packing': 'ddiiddii' }, 

        'T2ADJUNCT': {
            'fields': ( ('xstart', 'd', 1, 0),
                        ('xdelta', 'd', 1, 8),
                        ('xunits', 'i', 1, 16),
                        ('subsize', 'i', 1, 20),
                        ('ystart', 'd', 1, 24),
                        ('ydelta', 'd', 1, 32),
                        ('yunits', 'i', 1, 40),
                        ('bid', 'i', 1, 44) ),
            'defaults': { 'xdelta': 1.0,
                          'ydelta': 1.0,
                          'subsize': 1 },
            'nbytes': 48,
            'npacking': 8,
            'packing': 'ddiiddii' },

        'T3ADJUNCT': {
            'fields': ( ('rstart', 'd', 1, 0),
                        ('rdelta', 'd', 1, 8),
                        ('runits', 'i', 1, 16),
                        ('subrecords', 'i', 1, 20),
                        ('r2start', 'd', 1, 24),
                        ('r2delta', 'd', 1, 32),
                        ('r2units', 'i', 1, 40),
                        ('record_length', 'i', 1, 44),
                        ('subr', bstructs['SUBRECSTRUCT'], 26, 48) ),
            'defaults': { 'rdelta': 1.0,
                          'r2delta': 1.0 },
            'nbytes': 256,
            'npacking': 9,
            'packing': 'ddiiddii208s' },

        'T4ADJUNCT': {
            'fields': ( ('vrstart', 'd', 1, 0),
                        ('vrdelta', 'd', 1, 8),
                        ('vrunits', 'i', 1, 16),
                        ('nrecords', 'i', 1, 20),
                        ('vr2start', 'd', 1, 24),
                        ('vr2delta', 'd', 1, 32),
                        ('vr2units', 'i', 1, 40),
                        ('vrecord_length', 'i', 1, 44) ),
            'defaults': { 'vrdelta': 1.0,
                          'vr2delta': 1.0 },
            'nbytes': 48,
            'npacking': 8,
            'packing': 'ddiiddii' },

        'T5ADJUNCT': {
            'fields': ( ('tstart', 'd', 1, 0),
                        ('tdelta', 'd', 1, 8),
                        ('tunits', 'i', 1, 16),
                        ('components', 'i', 1, 20),
                        ('t2start', 'd', 1, 24),
                        ('t2delta', 'd', 1, 32),
                        ('t2units', 'i', 1, 40),
                        ('record_length', 'i', 1, 44),
                        ('comp', bstructs['COMPSTRUCT'], 14, 48),
                        ('quadwords', bstructs['T5QUADWORDS'], 1, 160) ),
            'defaults': { 'tdelta': 1.0,
                          't2delta': 1.0 },
            'nbytes': 256,
            'npacking': 21,
            'packing': 'ddiiddii112s96s' }
        })
    
    # The T6 adjunct is exactly the same as the T3 adjunct
    bstructs['T6ADJUNCT'] = bstructs['T3ADJUNCT'] 

    bstructs['ADJUNCT'] = {
        'fields': { 'bytes': ('bytes', '256s', 1, 0),
                    't1': ('t1', bstructs['T1ADJUNCT'], 1, 0),
                    't2': ('t2', bstructs['T2ADJUNCT'], 1, 0),
                    't3': ('t3', bstructs['T3ADJUNCT'], 1, 0),
                    't4': ('t4', bstructs['T4ADJUNCT'], 1, 0),
                    't5': ('t5', bstructs['T5ADJUNCT'], 1, 0),
                    't6': ('t6', bstructs['T6ADJUNCT'], 1, 0) },
        'nbytes': 256,
        'npacking': 1,
        'packing': '256s' }

    bstructs['HEADER'] = {
        'fields': ( ('version', '4s', 1, 0),
                    ('head_rep', '4s', 1, 4),
                    ('data_rep', '4s', 1, 8),
                    ('detached', 'i', 1, 12),
                    ('protected', 'i', 1, 16),
                    ('pipe', 'i', 1, 20),
                    ('ext_start', 'i', 1, 24),
                    ('ext_size', 'i', 1, 28),
                    ('data_start', 'd', 1, 32),
                    ('data_size', 'd', 1, 40),
                    ('type', 'i', 1, 48),
                    ('format', '2s', 1, 52),
                    ('flagmask', 'h', 1, 54),
                    ('timecode', 'd', 1, 56),
                    ('inlet', 'h', 1, 64),
                    ('outlets', 'h', 1, 66),
                    ('outmask', 'i', 1, 68),
                    ('pipeloc', 'i', 1, 72),
                    ('pipesize', 'i', 1, 76),
                    ('in_byte', 'd', 1, 80),
                    ('out_byte', 'd', 1, 88),
                    ('outbytes', 'd', 8, 96),
                    ('keylength', 'i', 1, 160),
                    ('keywords', '92s', 1, 164),
                    ('adjunct', bstructs['ADJUNCT'], 1, 256) ),
        'defaults': { 'version': 'BLUE',
                      'head_rep': _native_rep,
                      'data_rep': _native_rep,
                      'data_start': 512.0,
                      'type': 1000,
                      'format': 'SF' },
        'nbytes': 512,
        'npacking': 32,
        'packing': '4s4s4siiiiiddi2shdhhiiidd8di92s256s' }

    # Fill in the 'lookups' field for each struct definition, which is
    # a dictionary versions of its 'fields' list.
    for bstruct in bstructs.values():
        bstruct['lookups'] = dict([(f[0], f) for f in bstruct['fields']])

    return bstructs

_bluestructs = _init_bluestructs()


## Utilities

def bpa(format):
    """
    Returns bpa as an integer.

    (Doc string adapted from X-Midas' M$BPA help file)
  
    Given a 2-character X-Midas format specification string,
    returns the number of Bytes Per Atom for the named format.
    Atoms are composed of a mode and a type. The allowable modes are:
  
        format[0] = S  scalar (size=1)
                    C  complex (size=2)
                    V  vector (size=3)
                    Q  quad (size=4)
                    M  matrix (size=9)
                    T  transformation matrix (size=16)
                    U  user (size=1)
                   1-9 generic multiple valued fields
                    X  multiple valued field (size=10)
  
    The mode size is multiplied by the number of bytes used in the data type.
    Allowable types are:
  
        format[1] = B  byte (1 byte)
                    I  integer (2 bytes)
                    L  long integer (4 bytes)
                    X  extended long integer (8 bytes)
                    F  float (4 bytes)
                    D  double (8 bytes)
                    A  ASCII (8 bytes)
                    P  packed (1 bit)
                    O  offset byte (1 byte)

    any combination of mode and representation is allowed.

    The user defined mode "U" is available to treat any non-standard
    data formats a primitive programmer may encounter.  In this mode,
    the representation character is treated as a signed byte
    ( N = byte rep of format[1] ).
       if N > 0   N is the number bytes per user defined atom
       if N < 0   -N is the number bits per user defined atom
    The internal representation is up to the application.

    If the format contains bit data, the return value will be negative.
    Its absolute value of will be the number of BITS per atom.

    Examples:
       bpa('SF') ->  4    # Byte data
       bpa('QP') -> -4    # Bit data
    """
    if len(format) == 2:
        format = format.upper()
        if not format[1:2] == "P":
           return _mode_tran[format[0:1]] * _type_tran[format[1:2]] 
        else:
            return _mode_tran[format[0:1]] * (_type_tran[format[1:2]] * -8)
    else:
        raise Exception, "Format string must be exactly two characters"


def decode_xmformat(format):
    """
    (Doc string adapted from X-Midas' M$BPA help file)
  
    Given a 2-character X-Midas format specification string, returns
    a (mode size, type size) tuple, where
        mode size * type size = bytes per atom (bpa)

    Atoms (format specification strings) are composed of a mode and a
    type. The allowable modes are:
  
        format[0] = S  scalar (size=1)
                    C  complex (size=2)
                    V  vector (size=3)
                    Q  quad (size=4)
                    M  matrix (size=9)
                    T  transformation matrix (size=16)
                    U  user (size=1)
                   1-9 generic multiple valued fields
                    X  multiple valued field (size=10)

    Allowable types are:
  
        format[1] = B  byte (1 byte)
                    I  integer (2 bytes)
                    L  long integer (4 bytes)
                    X  extended long integer (8 bytes)
                    F  float (4 bytes)
                    D  double (8 bytes)
                    A  ASCII (8 bytes)
                    P  packed (.125 bytes (1 bit))
                    O  offset byte (1 byte)

    any combination of mode and representation is allowed.

    The user defined mode "U" is available to treat any non-standard
    data formats a primitive programmer may encounter.  In this mode,
    the representation character is treated as a signed byte
    ( N = byte rep of format[1] ).  The internal representation is
    up to the application.

    If the format contains bit data, the return value for the "type
    size" element of the tuple will be a floating point number
    indicating a fraction of a byte.  Note that this is different
    from the bpa and m_bpa functions, which return negative integers
    to indicate bit data.  Multiplying the two elements of the
    tuple returned by decode_xmformat will always give BYTES per
    atom (never BITS per atom).

    Examples:
       decode_xmformat('SF') -> (1, 4)       # Byte data
       decode_xmformat('QP') -> (4, .125)    # Bit data
    """
    if len(format) == 2:
        format = format.upper()
        return (_mode_tran[format[0:1]], _type_tran[format[1:2]])
    else:
        raise Exception, "Format string should be exactly two characters"


def fexists(filename):
    """
    Returns true if the given file name exists and is a BLUE file.

    Calls form_read_path on the given file name, then checks whether
    the result exists and is a BLUE file. Returns True if so, False
    otherwise.

    Note that if a non-Midas file with the given name exists in the
    read aux list before a Midas file with the same name, this
    function will returns false. For example, with read aux list of

    ['/dir1', '/dir2']

    and files

    '/dir1/aces.tmp', '/dir2/aces.prm'

    but /dir1/aces.tmp not a BLUE file, fexists('aces') will return
    False.    
    """
    # Developed for MF DR #504799-1
    candidate = form_read_path(filename)
    if os.path.isfile(candidate):
        try:
            magic = open(candidate, 'r').read(4)
            if magic == 'BLUE':
                return True
        except:
            pass

    return False


def is_blue_hdr(dict):
    """
    Performs rudimentary checking to determine if the dictionary
    given appears to be a BLUE file header.
    """
    is_blue = True
    required_fields = ['version', 'data_start', 'data_size', 'data_rep',
                       'keywords', 'head_rep', 'detached', 'format', 'type']

    for field in required_fields:
        if not field in dict:
            is_blue = False
            break
        elif field == 'version' and not dict[field] == 'BLUE':
            is_blue = False
            break
        
    return is_blue
