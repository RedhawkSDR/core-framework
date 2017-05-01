#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK rh.SourceSDDS.
#
# REDHAWK rh.SourceSDDS is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK rh.SourceSDDS is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import ctypes
import time
import datetime


def  BitsToNumber(sbits, reverse=False ):
     tbits=sbits[:]
     if reverse:
         tbits.reverse()
     f = [x << n for n, x in enumerate(tbits)]
     return  reduce(lambda x, y: x + y, f)


class format_identifier(ctypes.Structure):
      _pack_ = 1
      _fields_ = [ ('dm',ctypes.c_uint8,3), 
                  ('ss',ctypes.c_uint8,1),
                  ('of',ctypes.c_uint8,1),
                  ('pp',ctypes.c_uint8,1),
                  ('sos',ctypes.c_uint8,1),
                  ('sf',ctypes.c_uint8,1),
                  ('bps',ctypes.c_uint8,5),
                  ('vw',ctypes.c_uint8,1),
                  ('snp',ctypes.c_uint8,1),
                  ('cx',ctypes.c_uint8,1) ]

      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(format_identifier,cls).__new__(cls)

      def __init__(self,data=None):
           self.sf = 1
           self.sos = 1
           self.pp = 0
           self.of = 0
           self.ss = 0
           self.bps = 8
           self.vw = 0
           self.snp = 0
           self.cx = 0
           self.dm=1

      def __str__(self):
          return  ' '.join( [ x[0]+':'+str(getattr(self,x[0])) for x in self._fields_ ])

      
      def set_dmode(self, dm, reverse=False ):
           if type(dm) == list:
                self.dm = BitsToNumber(dm)
           else:
                self.dm = dm
             
      def set_bps(self, bps, reverse=False ):
           _bps=bps
           if type(bps) == list:
                _bps = BitsToNumber(bps, reverse)

           if _bps == 32 : _bps = 31
           self.bps= _bps

      def get_bps(self):
           _bps=self.bps
           if _bps == 31 : _bps = 32
           return _bps

      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))

class frame_sequence(ctypes.BigEndianStructure):
      MAX_SEQ_NUMBER=65536
      _pack_ = 1
      _fields_ = [ ('seq',ctypes.c_ushort,16) ]

      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(frame_sequence,cls).__new__(cls)

      def __init__(self,data=None):
           self.seq = 0

      def inc(self):
           self.seq = (self.seq + 1 ) % frame_sequence.MAX_SEQ_NUMBER

      def __str__(self):
          return  ''.join( [ str(getattr(self,x[0])) for x in self._fields_ ])

      def get(self):
           return self.seq

      def set(self, v ):
           self.seq = v

      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))

class msptr_data (ctypes.Structure):
      _pack_ = 1
      _fields_ = [ ('msptr',ctypes.c_ushort,16),
                   ('msdelta',ctypes.c_ushort,16)  ]

      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(msptr_data,cls).__new__(cls)

      def __init__(self,data=None):
           self.msptr=0
           self.msdelta=0

      def __str__(self):
          return  '/'.join( [ x[0] for x in self._fields_ ]) + ': '+ '/'.join( [ str(getattr(self,x[0])) for x in self._fields_ ])

      def set_msptr( self, val ):
           val = val&0x07FF
           self.msptr = val

      def get_msptr( self ):
           return self.msptr

      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))

class ttag_info_struct(ctypes.Structure):
      _pack_ = 1
      _fields_ = [ ('pad2',ctypes.c_uint8,8),
                   ('msv',ctypes.c_uint8,1),
                   ('ttv',ctypes.c_uint8,1),
                   ('sscv',ctypes.c_uint8,1),
                   ('pi',ctypes.c_uint8,1),
                   ('peo',ctypes.c_uint8,1),
                   ('pad1',ctypes.c_uint8,3),
                   ('pad3',ctypes.c_uint16,16) ]
                   
      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(ttag_info_struct,cls).__new__(cls)

      def __init__(self,data=None):
           self.ttv=0
           self.sscv=0
           self.pi=0
           self.msv=0
           self.peo=0
           self.pad1=0
           self.pad2=0
           self.pad3=0

      def __str__(self):
          return  ' '.join( [ x[0]+':'+str(getattr(self,x[0])) for x in self._fields_ if x[0] in ['msv','ttv','sscv','pi','peo']])

      def get_sscv(self):
           return self.sscv

      def set_sscv(self, valid=True):
           if valid:
                self.sscv = 1
           else:
                self.sscv = 0

      def get_ttv(self):
           return self.ttv

      def set_ttv(self, valid=True):
           if valid:
                self.ttv = 1
           else:
                self.ttv = 0

      def get_msv(self):
           return self.msv

      def set_msv(self, valid=True):
           if valid:
                self.msv = 1
           else:
                self.msv = 0

      def get_pi(self):
           return self.pi

      def set_pi(self, onoff=True):
           if onoff:
                self.pi = 1
           else:
                self.pi = 0

      def get_peo(self):
           return self.peo

      def set_peo(self, odd=False):
           if odd:
                self.peo = 1
           else:
                self.peo = 0

      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))

class ttag_info_union(ctypes.Union):
      _pack_ = 1
      _fields_ = [ ('msptr', msptr_data), 
                   ('info', ttag_info_struct )]

      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(ttag_info_union,cls).__new__(cls)

      def __init__(self,data=None):
          pass

      def __str__(self):
          return  'info: '+str(self.info)+ ' ' + str(self.msptr)

      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))

class ttag_values(ctypes.BigEndianStructure):
      _pack_ = 1
      _fields_ = [ ('ttag',ctypes.c_uint64), 
                   ('ttage',ctypes.c_uint32)
                 ]
      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(ttag_values,cls).__new__(cls)

      def __init__(self,data=None):
           self.ttag=0
           self.ttage=0

      def __str__(self):
          return  str(self.get_SDDSTime())

      def set(self, ps250, pf250):
           self.ttag = ps250
           self.ttage = pf250

      def set_SDDSTime(self, sdds_time):
           self.ttag = sdds_time.picoTicks()
           self.ttage = sdds_time.picoTicksFractional()

      def get_SDDSTime(self):
           from ossie.utils.sdds import Time
           t=Time()
           t.set( self.ttag, self.ttage )
           return t


      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))


class ttag_info(ctypes.Structure):
      _pack_ = 1
      _fields_ = [ ('info', ttag_info_union ),
                   ('tstamp', ttag_values ) ]

      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(ttag_info,cls).__new__(cls)

      def __init__(self,data=None):
          pass

      def __str__(self):
          return  str(self.info)+' tstamp: '+str(self.tstamp)

      def get_msptr( self ):
           return self.info.msptr.get_msptr()

      def set_msptr( self, val ):
           self.info.msptr.set_msptr(val)

      def get_msdelta( self ):
           return self.info.msptr.get_msdelta()

      def set_msdelta( self, val ):
           self.info.msptr.set_msdelta(val)

      def clear_msptr(self):
           self.info.msptr.msptr = 0
           self.info.msptr.msdelta = 0

      def set_msv(self, valid=True):
           self.info.info.set_msv(valid)

      def get_msv(self):
           return self.info.info.get_msv()

      def get_ttv(self):
           return self.info.info.get_ttv()

      def set_ttv(self, valid=True):
           self.info.info.set_ttv(valid)

      def get_sscv(self):
           return self.info.info.get_sscv()

      def set_sscv(self, valid=True):
           self.info.info.set_sscv(valid)


      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))

class ssc_info_struct(ctypes.BigEndianStructure):
      _pack_ = 1
      _fields_ = [ ('dfdt',ctypes.c_int32), 
                   ('freq',ctypes.c_uint64)
                 ]
      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(ssc_info_struct,cls).__new__(cls)

      def __init__(self,data=None):
          pass

      def __str__(self):
          return  'freq: '+ str(self.get_freq())+' dfdt: '+ str(self.get_dfdt())

      def get_freq(self):
           # frequency units in resolution 125mhz/2^63
           rfreq = ( self.freq * 1.3552527156068805e-11 )
           return rfreq

      def set_freq(self, freq):
           # frequency units resolution 2^63/125mhz
           sfreq= freq* 73786976294.838211
           self.freq = long(sfreq)

      def get_dfdt(self):
           sdfdt = self.dfdt * 9.3132257461547852e-10
           return sdfdt

      def set_dfdt(self, val ):
           sdfdt = val * 1073741824.0
           self.dfdt = ctypes.c_int32(int(sdfdt))

      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))


class ssd_data(ctypes.BigEndianStructure):
      DATA_LEN=2
      _pack_ = 1
      _fields_ = [ ('data', ctypes.c_uint16* DATA_LEN ) ]

      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(ssd_data,cls).__new__(cls)

      def __init__(self,data=None):
          pass

      def __str__(self):
          return  ','.join( [ str(hex(ord(x))) for x in self.asBuffer() ] )

      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))

class aad_data(ctypes.BigEndianStructure):
      DATA_LEN=20
      _pack_ = 1
      _fields_ = [ ('data', ctypes.c_uint8*DATA_LEN) ]

      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(aad_data,cls).__new__(cls)

      def __init__(self,data=None):
          pass

      def __str__(self):
          return  ','.join( [ str(hex(ord(x))) for x in self.asBuffer() ] )

      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))


class sdds_header(ctypes.Structure):
      PARITY_SEQ_NUMBER=32
      MAX_SEQ_NUMBER=65536
      _pack_ = 1
      _fields_ = [ ('formatid', format_identifier), 
                   ('fsn',  frame_sequence),
                   ('ttag', ttag_info ),
                   ('ssc', ssc_info_struct ),
                   ('ssd',  ssd_data ),
                   ('aad',  aad_data ),
                 ]

      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(sdds_header,cls).__new__(cls)

      def __init__(self,data=None, skip_parity=True):
           
           self._skip_parity=skip_parity
           pass

      def __str__(self):
           return 'format_id: '+str(self.formatid)+'\n'+ \
                  '      fsn: '+str(self.fsn) +'\n' + \
                  '     ttag: '+str(self.ttag)+'\n' + \
                  '      ssc: '+str(self.ssc)+'\n' + \
                  '      ssd: '+str(self.ssd)+'\n' + \
                  '      aad: '+str(self.aad)


      def inc(self):
           self.fsn.inc()
           if self._skip_parity:
                if self.fsn.seq % sdds_header.PARITY_SEQ_NUMBER == 31:
                     self.fsn.inc()
           
           if self.fsn == 0:
                self.formatid.sos=0
      
      ##
      ##  Format Identifier 
      ##
      def get_complex(self):
           return self.formatid.cx

      def set_complex(self, isComplex=False ):
           self.formatid.cx=0
           if isComplex:
                self.formatid.cx=1
      
      def set_dmode(self,dm):
           self.formatid.set_dmode(dm)
           
      def get_dmode(self):
           return self.formatid.dm

      def set_spectralsense(self, ison=False ):
           self.formatid.ss= 0
           if isone:
                self.formatid.ss= 1

      def get_spectralsense(self):
           return self.formatid.ss

      def get_vw(self):
           return self.formatid.vw

      def set_vw(self, isVeryWide=False ):
           if isVeryWide:
               self.formatid.vw = 1
           else:
               self.formatid.vw = 0

      def get_bps(self):
           return self.formatid.get_bps()

      def set_bps(self, bps ):
           self.formatid.set_bps(bps)

      ##
      ## frame sequence
      ##
      def get_fsn(self):
           return self.fsn.get()

      def set_fsn(self, v ):
           self.fsn.set(v)

      ##
      ##  ttag  - time tag
      ##
      def get_msptr( self ):
           return self.ttag.info.msptr.get_msptr()

      def set_msptr( self, val ):
           self.ttag.info.msptr.set_msptr(val)

      def get_msdelta( self ):
           return self.ttag.info.msptr.get_msdelta()

      def set_msdelta( self, val ):
           self.ttag.info.msptr.set_msdelta(val)

      def clear_msptr(self):
           self.ttag.info.msptr.msptr = 0
           self.ttag.info.msptr.msdelta = 0

      def set_msv(self, valid=True):
           self.ttag.info.info.set_msv(valid)

      def get_msv(self):
           return self.ttag.info.info.get_msv()

      def get_ttv(self):
           return self.ttag.info.info.get_ttv()

      def set_ttv(self, valid=True):
           self.ttag.info.info.set_ttv(valid)

      def get_sscv(self):
           return self.ttag.info.info.get_sscv()

      def set_sscv(self, valid=True):
           self.ttag.info.info.set_sscv(valid)

      def set_time(self, ps250, pf250 ):
           self.ttag.tstamp.ttag = ps250
           self.ttag.tstamp.ttage = pf250

      def set_SDDSTime(self, sdds_time, ):
           self.ttag.tstamp.ttag = sdds_time.picoTicks()
           self.ttag.tstamp.ttage = sdds_time.picoTicksFractional()

      def get_SDDSTime(self):
           from ossie.utils.sdds import Time
           t=Time()
           t.set( self.ttag.tstamp.ttag,
                  self.ttag.tstamp.ttage )
           return t


      ##
      ##  ssc - synchronous sample clock
      ##
      def get_freq(self):
           return self.ssc.get_freq()

      def set_freq(self, freq):
           self.ssc.set_freq(freq)

      def get_rate(self):
           rate = self.get_freq()
           if self.get_vw() == 1:
              rate *= 16.0
              
           if self.get_complex() == 1:
              rate *= 0.5
           return rate

      def set_rate(self, rate):
           vw=0
           if rate>= 125e6:
                vw = 1
                val = val * 0.0625
           self.set_vw(vw)
           self.set_freq(freq)

      def get_dfdt(self):
           return self.ssc.get_dfdt()

      def set_dfdt(self, freq):
           self.ssc.set_dfdt(freq)


      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))


####################################################################################
#
# SDDS Payload Containers
#
####################################################################################

class sdds_sb_payload(ctypes.BigEndianStructure):
      NUM_SAMPLES=1024
      PAYLOAD_SIZE=1024
      _pack_ = 1
      _fields_ = [ ('data', ctypes.c_uint8 *PAYLOAD_SIZE ) ]

      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(sdds_sb_payload,cls).__new__(cls)

      def __init__(self,data=None):
          pass

      def get_data(self):
           return self.data[:]

      def set_data(self, samples ):
           if type(samples) == list:
                for i,x in enumerate(samples):
                     if i < sdds_sb_payload.NUM_SAMPLES:
                          self.data[i] = x
           else:
                fit = min(len(samples), ctypes.sizeof(self))
                ctypes.memmove(ctypes.addressof(self), samples, fit)

      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))


class sdds_cb_payload(ctypes.BigEndianStructure):
      NUM_SAMPLES=512
      PAYLOAD_SIZE=1024
      _pack_ = 1
      _fields_ = [ ('data', ctypes.c_uint8 *PAYLOAD_SIZE ) ]

      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(sdds_cb_payload,cls).__new__(cls)

      def __init__(self,data=None):
          pass

      def get_data(self):
           return self.data[:]

      def set_data(self, samples ):
           if type(samples) == list:
                for i,x in enumerate(samples):
                     if i < sdds_cb_payload.NUM_SAMPLES:
                          self.data[i] = x
           else:
                fit = min(len(samples), ctypes.sizeof(self))
                ctypes.memmove(ctypes.addressof(self), samples, fit)

      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))


class sdds_si_payload(ctypes.BigEndianStructure):
      NUM_SAMPLES=512
      PAYLOAD_SIZE=512
      _pack_ = 1
      _fields_ = [ ('data', ctypes.c_uint16 * PAYLOAD_SIZE) ]

      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(sdds_si_payload,cls).__new__(cls)

      def __init__(self,data=None):
          pass

      def get_data(self):
           return self.data[:]

      def set_data(self, samples ):
           if type(samples) == list:
                for i,x in enumerate(samples):
                     if i < sdds_si_payload.NUM_SAMPLES:
                          self.data[i] = x
           else:
                fit = min(len(samples), ctypes.sizeof(self))
                ctypes.memmove(ctypes.addressof(self), samples, fit)

      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))

class sdds_ci_payload(ctypes.BigEndianStructure):
      NUM_SAMPLES=256
      PAYLOAD_SIZE=512
      _pack_ = 1
      _fields_ = [ ('data', ctypes.c_uint16 * PAYLOAD_SIZE) ]

      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(sdds_ci_payload,cls).__new__(cls)

      def __init__(self,data=None):
          pass

      def get_data(self):
           return self.data[:]

      def set_data(self, samples ):
           if type(samples) == list:
                for i,x in enumerate(samples):
                     if i < sdds_ci_payload.NUM_SAMPLES:
                          self.data[i] = x
           else:
                fit = min(len(samples), ctypes.sizeof(self))
                ctypes.memmove(ctypes.addressof(self), samples, fit)

      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))

class sdds_sn_sample(ctypes.BigEndianStructure):
      _pack_ = 1
      _fields_ = [ ('sn2', ctypes.c_uint8,4 ),
                   ('sn1', ctypes.c_uint8,4 )
                 ]

      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(sdds_sn_sample,cls).__new__(cls)

      def __init__(self,data=None):
          pass

      def get_data(self):
           return [ self.sn1, self.sn2 ]

      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))

class sdds_sn_payload(ctypes.Structure):
      NUM_SAMPLES=2048
      PAYLOAD_SIZE=1024
      _pack_ = 1
      _fields_ = [ ('data', sdds_sn_sample*PAYLOAD_SIZE) ]

      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(sdds_sn_payload,cls).__new__(cls)

      def __init__(self,data=None):
           pass

      def get_data(self):
           _ret=[]
           for x in self.data[:]:
                _ret += x.get_data()
           return _ret

      def set_data(self, samples ):
           if type(samples) == list:
                for i,x in enumerate(samples):
                     if i < sdds_sn_payload.NUM_SAMPLES:
                          self.data[i] = x
           else:
                fit = min(len(samples), ctypes.sizeof(self))
                ctypes.memmove(ctypes.addressof(self), samples, fit)

      def asBuffer(self):
           return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))

class sdds_sf_payload(ctypes.BigEndianStructure):
      NUM_SAMPLES=256
      PAYLOAD_SIZE=256
      _pack_ = 1
      _fields_ = [ ('data', ctypes.c_float * PAYLOAD_SIZE ) ]

      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(sdds_sf_payload,cls).__new__(cls)

      def __init__(self,data=None):
          pass

      def get_data(self):
           return self.data[:]

      def set_data(self, samples ):
           if type(samples) == list:
                for i,x in enumerate(samples):
                     if i < sdds_sf_payload.NUM_SAMPLES:
                          self.data[i] = x
           else:
                fit = min(len(samples), ctypes.sizeof(self))
                ctypes.memmove(ctypes.addressof(self), samples, fit)

      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))

class sdds_payload(ctypes.Union):
      PAYLOAD_SIZE=1024
      _pack_ = 1
      _fields_ = [ ('raw', ctypes.c_uint8 *PAYLOAD_SIZE ), 
                   ('sn',  sdds_sn_payload ),
                   ('sb',  sdds_sb_payload ),
                   ('cb',  sdds_sb_payload ),
                   ('si',  sdds_si_payload ),
                   ('ci',  sdds_si_payload ),
                   ('sf',  sdds_sf_payload )
                 ]

      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(sdds_payload_struct,cls).__new__(cls)

      def __init__(self,data=None):
          pass

      def __str__(self):
          return  ','.join( [ str(x) for x in self.raw[:40] ] )

      def get_data(self):
           return self.raw[:]

      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))


class sdds_packet(ctypes.Structure):
      FORMATS = { 
           'SN' : { 'dmode': 0, 'bps': 4, 'cplx': 0, 'samples': 2048,  'get_data': sdds_sn_payload.get_data },
           'SB' : { 'dmode': 1, 'bps': 8, 'cplx': 0, 'samples': 1024,  'get_data': sdds_sb_payload.get_data },
           'CB' : { 'dmode': 1, 'bps': 8, 'cplx': 1, 'samples': 512,   'get_data': sdds_cb_payload.get_data},
           'SI' : { 'dmode': 2, 'bps': 16, 'cplx': 0, 'samples': 512,  'get_data': sdds_si_payload.get_data},
           'CI' : { 'dmode': 2, 'bps': 16, 'cplx': 1, 'samples': 256,  'get_data': sdds_ci_payload.get_data},
           'SF' : { 'dmode': 7, 'bps': 32, 'cplx': 0, 'samples': 256,  'get_data': sdds_sf_payload.get_data},
           'AD4' : { 'dmode': 5, 'bps': 8, 'cplx': 0, 'samples': 1024,  'get_data': sdds_sb_payload.get_data},
           'AD12' : { 'dmode': 6, 'bps': 16, 'cplx': 0, 'samples': 512,  'get_data': sdds_si_payload.get_data},
           }
      PKT_LEN=1080
      HEADER_LEN=56
      PAYLOAD_LEN=1024
      _pack_ = 1
      _fields_ = [ ('header', sdds_header ),
                   ('payload', sdds_payload )
                 ]

      def __new__(cls,buf=None):
          if buf:
             return cls.from_buffer_copy(buf)
          else:
             return super(sdds_packet,cls).__new__(cls)

      def __init__(self,data=None, skip_parity=True):
           self._skip_parity=skip_parity
           pass

      def __str__(self):
           return ''.join(str(self.header)) + '\n' +\
                  '  payload: '+ ''.join(str(self.payload))

      def inc(self):
           self.header.inc()
      
      ##
      ##  Format Identifier 
      ##
      def get_complex(self):
           return self.header.get_complex()

      def set_complex(self, isComplex=False ):
           self.header.set_complex(isComplex)

      def set_spectralsense(self, ison=False ):
           self.header.set_spectralsense(ison)

      def get_spectralsense(self, ison=False ):
           return self.header.get_spectralsense()

      def get_vw(self):
           return self.header.get_vw()

      def set_vw(self, isVeryWide=False ):
           self.header.set_vw( isVeryWide )

      def get_bps(self):
           return self.header.get_bps()

      def set_bps(self, bps ):
           self.header.set_bps(bps)

      def set_dmode(self,dm, cplx=False, calc_bps=True, bps=None):
           if self.ok_dmode(dm):
                self.header.set_dmode(dm)
                self.header.set_complex(cplx)
                if calc_bps or bps==None:
                    self.header.set_bps( self.get_bps_for_mode(dm) )
                else:
                     self.header.set_bps(bps)

      def ok_dmode(self, dmode ):
           return dmode == 0 or dmode == 1 or dmode == 2 or dmode == 5 or dmode == 6 or dmode == 7;
           
      def get_dmode(self):
           return self.header.get_dmode()

      def get_bps_for_mode(self, dmode ):
           bps=8
           if dmode == 0:
                bps=4
           if dmode == 1 or dmode == 5:
                bps=8
           if dmode == 2 or dmode == 6:
                bps=16
           if dmode == 7: bps=32
           return bps

      ##
      ## frame sequence
      ##
      def get_fsn(self):
           return self.header.get_fsn()

      def set_fsn(self, v ):
           self.header.set_fsn(v)

      ##
      ##  ttag  - time tag
      ##
      def get_msptr( self ):
           return self.header.get_msptr()

      def set_msptr( self, val ):
           self.header.set_msptr(val)

      def clear_msptr( self):
           self.header.clear_msptr()

      def get_msdelta( self ):
           self.header.get_msdelta()

      def set_msdelta( self, val ):
           self.header.set_msdelta(val)

      def set_msv(self, valid=True):
           self.header.set_msv(valid)

      def get_msv(self):
           self.header.get_msv()

      def get_ttv(self):
           self.header.get_ttv()

      def set_ttv(self, valid=True):
           self.header.set_ttv(valid)

      def get_sscv(self):
           self.header.get_sscv()

      def set_sscv(self, valid=True):
           self.header.set_sscv(valid)

      def set_time(self, ps250, pf250 ):
           self.header.set_time(ps250, pf250)

      def set_SDDSTime(self, sdds_time, ):
           self.header.set_SDDSTime(sdds_time)

      def get_SDDSTime(self):
           return self.header.get_SDDSTime()

      ##
      ##  ssc - synchronous sample clock
      ##
      def get_freq(self):
           return self.header.get_freq()

      def set_freq(self, freq):
           self.header.set_freq(freq)

      def get_rate(self):
           return self.header.get_rate()

      def set_rate(self, freq):
           self.header.set_rate(freq)

      def get_dfdt(self):
           return self.header.get_dfdt()

      def set_dfdt(self, freq):
           self.freq.set_dfdt(freq)

      def get_format(self):
           dm=self.header.get_dmode()
           fmt='SB'
           for k, v in sdds_packet.FORMATS.items():
                if v['dmode'] == dm :
                     fmt=k
           return fmt

      def set_format(self, fmt):
           ret=1
           if fmt in sdds_packet.FORMATS.keys():
                _fmt = sdds_packet.FORMATS[fmt]
                ret=0
                cplx = _fmt['cplx']
                dm = _fmt['dmode']
                self.set_dmode( dm, cplx, bps=_fmt['bps'])


      def get_data(self):
           bps=self.header.get_bps()
           for k, v in sdds_packet.FORMATS.items():
                if v['bps'] == bps :
                     attr = getattr(self.payload, k.lower())
                     return v['get_data'](attr)
           return self.payload.sb.get_data()

      def asBuffer(self):
          return buffer(self)[:]

      def asString(self):
          return ctypes.string_at(ctypes.addressof(self),ctypes.sizeof(self))
