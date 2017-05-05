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
import time
import datetime
import calendar
import struct
import math
import copy as _copy

def difference(t1, t2):
     tmp = _copy.copy(t1)
     if tmp.pf250_ >=  t2.pf250_ :
          tmp.pf250_ -= t2.pf250_
          tmp.ps250_ -= t2.ps250_
     else:
          tmp.pf250_  = Time.Two32 + ( tmp.pf250_ - t2.pf250_ )
          tmp.ps250_ -= t2.ps250_ + 1;
     return tmp

def sum( t1, t2 ):
     tmp=_copy.copy(t1)
     tfrac =long(tmp.pf250_) + t2.pf250_
     tmp.ps250_ += t2.ps250_ + int(tfrac>>32) 
     tmp.pf250_ = int(tfrac)
     return tmp

def add(t1, offset):
     if isinstance(offset, Time):
          return sum(t1, offset)
     else:
          return iadd(t1, offset)

def iadd(t1, offset):
     if not isinstance(t1, Time):
          return t1
     tmp=_copy.copy(t1)
     # make into  tics
     pfrac,pwhole = math.modf(offset*Time.TicFreq)
     tfrac = long(tmp.pf250_) + int(pfrac * Time.Two32)
     tmp.ps250_ += long(pwhole) + (tfrac>>32)
     tmp.pf250_ = int(tfrac)
     return tmp

def sub(t1, other):
    if isinstance(other, Time):
        return difference(t1, other)
    else:
        return isub(t1, other)

def isub(t1, offset):
    return iadd(t1, -offset)

def compare(t1, t2):
    if not isinstance(t1, Time) or  not isinstance(t2, Time):
        return -1
    if t1.ps250_ == t2.ps250_:
        return cmp(t1.pf250_,t2.pf250_)
    else:
        return cmp(t1.ps250_,t2.ps250_)

class Time:
    REDHAWK_FORMAT="%Y:%m:%d::%H:%M:%S"
    Tic = 250e-12
    TicFreq = 4000000000.0
    TicFreqLong = 4000000000L
    Two32 = 4294967296.0
    def __init__(self ):

         self.ps250_ = 0L
         self.pf250_ = 0
         self.startofyear = self.startOfYear()
         self.setFromTime()

    def setFromTime(self, time_sec=time.time() ):
         """
         Create a sdds time object from the input parameter. If the time_sec is from the epoch
         then we need to convert to the current year as per spec.
         """
         if time_sec:
              if time_sec >= self.startofyear:
                   # UTC.. need to convert to SDDS EPOCH
                   time_sec = time_sec - self.startofyear

         pfrac, pwhole = math.modf(time_sec*Time.TicFreq)
         self.ps250_ = long(pwhole)
         self.pf250_ = int( pfrac*Time.Two32)
         #print "td: %12Lu %12u %16.2Lf " % ( self.ps250_, self.pf250_, pfrac )
              
    def setFromPartial( self, integral, fractional ):
         pfrac, pwhole= math.modf(fractional*Time.TicFreq)
         self.ps250_ = long(integral*Time.TicFreqLong) + long(pwhole)
         self.pf250_ = int( pfrac * Time.Two32)
         #print "td: %12Lu %12u %16.2Lf " % ( self.ps250_, self.pf250_, pfrac )

    def set( self, psec, pfsec ):
         self.ps250_ = psec
         self.pf250_ = pfsec
         #print "td: %12Lu %12u " % ( self.ps250_, self.pf250_ )

    def secondsThisYear( self ):
         return self.ps250_*Time.Tic + self.pf250_ * (Time.Tic/Time.Two32)

    def seconds( self ):
         return self.startofyear + self.secondsThisYear()

    def picoTicks( self ):
         return self.ps250_

    def picoTicksFractional( self ):
         return self.pf250_

    def gmtime( self):
         return time.gmtime(self.startofyear+self.secondsThisYear())

    @staticmethod
    def toString(t1, fmt=None):
         gmt = t1.gmtime()
         frac =  int(t1.pf250_ * (Time.Tic/Time.Two32))
         if not fmt:
              fmt = Time.REDHAWK_FORMAT
              xx=time.strftime(fmt,gmt)
              return '%s.%06d' % (xx,frac)
         else:
              return time.strftime(fmt,gmt)

    def __str__(self):
         return toString(self)


    @staticmethod
    def startOfYear():
         soy=datetime.datetime(datetime.date.today().year,1,1,0,0,0)
         return calendar.timegm(soy.timetuple())

Time.__add__ = add
Time.__iadd__ = iadd
Time.__sub__ = sub
Time.__isub__ = isub
Time.__isub__ = isub
Time.__cmp__ = compare
Time.__str__ = Time.toString
