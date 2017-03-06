#
# This file is protected by Copyright. Please refer to the COPYRIGHT file distributed with this 
# source distribution.
# 
# This file is part of REDHAWK Basic Components SigGen.
# 
# REDHAWK Basic Components SigGen is free software: you can redistribute it and/or modify it under the terms of 
# the GNU Lesser General Public License as published by the Free Software Foundation, either 
# version 3 of the License, or (at your option) any later version.
# 
# REDHAWK Basic Components SigGen is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
# PURPOSE.  See the GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License along with this 
# program.  If not, see http://www.gnu.org/licenses/.
#
'''
Waveform class produces the following types of waveforms:
 - whitenoise
 - sincos
 - square
 - triangle
 - sawtooth
 - pulse
 - constant
 - lrs
 - ramp
'''
import math, os

class Waveform:
    A = 67081293.0
    B=14181771.0
    T26=67108864.0
    
    BI = B/T26
    seed = 123456789
    
    TWOPI = math.pi * 2.0
    HALFPI = math.pi / 2.0
    
    # Binary variant of a Giga. Note that this differs from the typical version
    # (the decimal) version of Giga which is 10^9
    # Value: = 1G = 2^30
    B1G  = 1073741824.
    
    def setSeed(self, value):
        if value > 0: self.seed = value
        
    # Create a white noise array of given magnitude
    # @param fbuf The output array
    # @param sdev Standard deviation
    # @param n    Number of elements
    # @param spa  Scalars per atom, 2 for Complex
    # @return the new data buffer
    def whitenoise(self, sdev, n, spa=1):
        outbuff = range(n*spa)
        v1 = 0.0; v2 = 0.0; sum1 = 0.0
        fdev = float(sdev)
        factor = -2.0 / math.log(10.0)
        sis = float(self.seed)/self.T26
        
        maxIndex = n*spa
        #Do this is in a while loop instead of a for loop
        #because we don't know how many times the sum1 is invalid 
        #we are forced to continue
        i=0
        while i < maxIndex:
            sis = sis*self.A + self.BI;
            sis = sis - float(int(sis))
            v1 = float(sis)
            v1 = v1+v1-1

            sis = sis*self.A + self.BI;
            sis = sis - float(int(sis))
            v2 = float(sis)
            v2 = v2+v2-1

            sum1 = v1*v1 + v2*v2
            if sum1 >= 1.0 or sum1 <1e-20: continue
            sum1 = fdev * float(math.sqrt(factor*math.log(sum1)/sum1))
            outbuff[i] = float(v1*sum1)
            if (i+1) < maxIndex:
                outbuff[i+1] = float(v2*sum1)
            i+=2
        
        self.seed = int(sis*self.T26)
        
        return outbuff
    
    # Create a SIN or COSINE array of given magnitude
    # @param fbuf The output array
    # @param amp  Amplitude
    # @param p    Phase
    # @param dp   Delta Phase
    # @param n    Number of elements
    # @param spa  Scalars per atom, 2 for Complex
    # @return the new data buffer
   
    # fast algorithm based on:  sin(x+dp) = sin(x)*cos(dp) + cos(x)*sin(dp)
    #                           cos(x+dp) = cos(x)*cos(dp) - sin(x)*sin(dp)
    def sincos(self, amp, p, dp, n, spa):
        outbuff = range(n*spa)
        cxr = amp*math.cos(p*self.TWOPI)
        cxi = amp*math.sin(p*self.TWOPI)
        dxr = math.cos(dp*self.TWOPI)
        dxi = math.sin(dp*self.TWOPI)
        if spa==2:
            for i in range(0, n*2, 2):
                outbuff[i] = float(cxr)
                outbuff[i+1] = float(cxi)
                axr = (cxr*dxr) - (cxi*dxi)
                axi = (cxr*dxi) + (cxi*dxr)
                cxr=axr 
                cxi=axi
        elif spa==1:
            for i in range(n):
                outbuff[i] = float(cxi)
                axr = (cxr*dxr) - (cxi*dxi)
                axi = (cxr*dxi) + (cxi*dxr)
                cxr=axr
                cxi=axi
        elif spa==-1:
            for i in range(n):
                outbuff[i] = float(amp*math.sin(p*self.TWOPI))
                p += dp
        elif spa==-2:
            for i in range(0, n*2, 2):
                outbuff[i] = float(amp*math.cos(p*self.TWOPI))
                outbuff[i+1] = float(amp*math.sin(p*self.TWOPI))
                p += dp
                
        return outbuff
    
    # Create a SQUARE array of given amplitude
    # @param fbuf The output array
    # @param amp  Amplitude
    # @param p    Phase
    # @param dp   Delta Phase
    # @param n    Number of elements
    # @param spa  Scalars per atom, 2 for Complex
    # @return the new data buffer
    def square(self, amp, p, dp, n, spa):
        outbuff = range(n*spa)
        value = 0.0
        famp = float(amp)
        famp2 = -famp
        for i in range(0, n*spa, spa):
            value = famp2
            if p >= 1.0:
                p -= 1.0
            elif p >= 0.5:
                value = famp
            outbuff[i] = float(value)
            if spa == 2:
                outbuff[i+1] = float(value)
            p += dp
            
        return outbuff
    
    # Create a TRIANGLE array of given amplitude
    # @param fbuf The output array
    # @param amp  Amplitude
    # @param p    Phase
    # @param dp   Delta Phase
    # @param n    Number of elements
    # @param spa  Scalars per atom, 2 for Complex
    # @return the new data buffer
    def triangle(self, amp, p, dp, n, spa):
        outbuff = range(n*spa)
        value = 0.0
        famp = float(amp)
        famp2 = 4*famp
        fp = float(p) - 0.5
        for i in range(0, n*spa, spa):
            if fp >= 0.5:
                fp -= 1.0
            if fp > 0:
                value = float(famp - fp*famp2)
            else:
                value = float(famp + fp*famp2)
            outbuff[i] = float(value)
            if spa == 2:
                outbuff[i+1] = float(value)
            fp += dp
        
        return outbuff
            
    # Create a SAWTOOTH array of given amplitude
    # @param fbuf The output array
    # @param amp  Amplitude
    # @param p    Phase
    # @param dp   Delta Phase
    # @param n    Number of elements
    # @param spa  Scalars per atom, 2 for Complex
    # @return the new data buffer
    def sawtooth(self, amp, p, dp, n, spa):
        outbuff = range(n*spa)
        value = 0.0
        famp = float(amp)
        famp2 = 2*famp
        fp = float(p) - 0.5
        for i in range(0, n*spa, spa):
            if fp >= 0.5:
                fp -= 1.0
            value = float(fp*famp2)
            outbuff[i] = float(value)
            if spa == 2:
                outbuff[i+1] = float(value)
            fp += dp
            
        return outbuff
    
    # Create a PULSE array of given amplitude
    # @param fbuf The output array
    # @param amp  Amplitude
    # @param p    Phase
    # @param dp   Delta Phase
    # @param n    Number of elements
    # @param spa  Scalars per atom, 2 for Complex
    # @return the new data buffer
    def pulse(self, amp, p, dp, n, spa):
        outbuff = range(n*spa)
        value = 0.0
        famp = float(amp)
        for i in range(0, n*spa, spa):
            if p >= 1.0:
                value = famp
                p -= 1.0
            else:
                value = 0
            outbuff[i] = float(value)
            if spa == 2:
                outbuff[i+1] = float(value)
            p += dp
            
        return outbuff
    
    # Create a CONSTANT array of given amplitude
    # @param fbuf The output array
    # @param amp  Amplitude
    # @param n    Number of elements
    # @param spa  Scalars per atom, 2 for Complex
    # @return the new data buffer
    def constant(self, amp, n, spa):
        outbuff = range(n*spa)
        for i in range(n*spa):
            outbuff[i] = float(amp)
        
        return outbuff
            
    # Create an LRS noise array of given magnitude
    # @param fbuf The output array
    # @param amp  Amplitude
    # @param n    Number of elements
    # @param spa  Scalars per atom, 2 for Complex
    # @param lrs  LRS seed from previous call
    # @return the new data buffer and the LRS at end of array
    def lrs(self, amp, n, spa, lrs):
        outbuff = range(n*spa)
        factor = (amp/2.0/self.B1G)
        for i in range(0, n*spa, spa):
            data = (factor * lrs)
            outbuff[i] = float(data)
            if spa == 2:
                outbuff[i+1] = float(data)
                
            bit0 = (~(lrs ^ (lrs>>1) ^  (lrs>>5) ^ (lrs>>25)))&0x1
            lrs <<= 1
            lrs |= bit0
            # Correct for python not overflowing int_32s
            lrs &= 0xffffffff
            if lrs >= 2**31 -1:
                lrs &= 0x7fffffff
                lrs -= 2**31
                
        return outbuff
    
    # Create an RAMP array of given magnitude
    # @param fbuf The output array
    # @param amp  Amplitude
    # @param n    Number of elements
    # @param spa  Scalars per atom, 2 for Complex
    # @param data RAMP seed from previous call
    # @return the new data buffer and the RAMP value at end of array
    def ramp(self, amp, n, spa, data):
        outbuff = range(n*spa)
        for i in range(0, n*spa, spa):
            outbuff[i] = float(data)
            if spa == 2:
                outbuff[i+1] = float(data)
            data = data + 1
            if data >= amp:
                data = int(-amp)
                
        return outbuff, data
