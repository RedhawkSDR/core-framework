
from  binascii import hexlify as _hexify
from   StringIO import StringIO
import ossie.utils.sb.helpers as _helpers
import sdds_pkt as _sdds_pkt
import traceback

__all__ = [ 'SDDSAnalyzer' ]

class SDDSAnalyzer(object):
    """
    The SDDSAnalyzer class can process a block of raw bytes as SDDS packets and perform the 
    following actions:

    trackChanges:

      Displays the following contents of the first packet's SDDS header fields.
          seq - sequence number of packet 
          fmt - data mode 
          cplx - complex flags
          bps -  bits per sample
          freq - collected signal frequency 
          clk - sample rate
          time valid  - value of ttv field
          time slip - amount of time slip between valid time stamps

      Then tracks changes in these fields and reports the following:
          -    No change in data detected, sequence number passed, 
               or expected time stamp passed (i.e., last valid time stamp + nsamples*1.0/sample rate)
         ***   Change in data field, out of sequence packet, time stamp invalid, 
               or expected time stamp did not pass
          +    Valid time stamp value provided in packet header

      Parameters:
          pkt_start = first packet to dump
          pkt_end =  last packet to dump  (None == end of list)
          repeat_header = displays column header every Nth packet displayed
          use_pager =  display data using pager to limit number of packets that are displayed

    dumpRawPackets:

         Dump the contents of the capture with the results managed by a pager.

      Parameters:
        pkt_start = first packet to dump
        pkt_end =  last packet to dump  (None == end of list)
        row_width = 80 the number of bytes to display per row
        bytes_per_group = the number of bytes to group when hexify-ing
        pkt_len = the number of bytes in a packet, None defaults to 1080 or length when getData method was called
        use_pager =  display data using pager to limit number of packets that are displayed

    dumpPackets:

       Dump packet fields and their data values, contents managed by a pager

       Parameters:
         pkt_start = first packet to dump
         pkt_end =  last packet to dump  (None == end of list)
         payload_start = starting payload sample to display
         payload_end = ending payload sample to display
         raw_payload = dump payload data as raw bytes
         header_only = only display header information for each packet
         use_pager  = display data using pager to limit number of packets that are displayed

    getPacketsIterator:

       Returns a Python iterator that will traverse the set of packets managed by the SDDSAnalyzer object
 
       Parameters:
         pkt_start = first packet to dump
         pkt_end =  last packet to dump  (None == end of list)

    getPackets - returns a list of sdds_packet objects

      Returns a list of sdds_packet objects
 
      Parameters:
        pkt_start = first packet to dump
        pkt_end =  last packet to dump  (None == end of list)

    __iter__ - The iterable returns a tuple (pkt number,sdds_packet)
    __len__  - returns the number of packets 

    """
    _VALID_VAL_='+'
    _TRACK_OK_='-'
    _TRACK_ERROR_='***'

    def __init__(self, raw_data, npkts, pkt_len=1080, total_bytes=None):
        self.npkts_ = npkts
        self.raw_data_ = raw_data
        self.pkt_len_ = pkt_len
        dsize=len(raw_data)
        expected_size = pkt_len * npkts

        if expected_size > dsize :
            raise RuntimeError("Invalid parameters, pkt_len*npkts is greater than raw_data size ")

        # adjust total bytes if necessary
        if total_bytes:
            if total_bytes > dsize:
                total_bytes=dsize
        else:
            total_bytes = dsize

        self.total_bytes_=total_bytes


    def dumpRawPackets(self, pkt_start=0, pkt_end=None, row_width=80, bytes_per_group=2, pkt_len=None, use_pager=True ):
        if pkt_end == None: 
            pkt_end = self.npkts_
            if pkt_len:
                pkt_end = self.npkts_
        else:
            pkt_end = pkt_end + 1

        if pkt_len == None: pkt_len = self.pkt_len_
        genf=self._gen_hex_dump( self.raw_data_, pkt_start, pkt_len, row_width, bytes_per_group )
        res = StringIO()
        for i, line in enumerate(genf,pkt_start):
            if i <  pkt_end:
                print >>res, 'pkt:'+str(i) + ' ' + line
            else:
                break

        if use_pager:
            _helpers.Pager( res.getvalue() )
        else:
            print res.getvalue()

    def dumpPackets(self, pkt_start=0, pkt_end=None, payload_start=0, payload_end=40, raw_payload=False, header_only=False, use_pager=True ):
        genf=self._gen_packet( self.raw_data_, pkt_start ) 
        if pkt_end == None: 
           pkt_end = self.npkts_ 
        else:
           pkt_end = pkt_end + 1
        res = StringIO()
        for i, pkt in enumerate(genf,pkt_start):
            if i < pkt_end:
                print >>res, 'Packet: ', str(i)
                print >>res, pkt.header_and_payload(payload_start, payload_end, header_only=header_only, raw=raw_payload )
            else:
                break

        if use_pager:
            _helpers.Pager( res.getvalue() )
        else:
            print res.getvalue()

    def _cmp_pkt( self, res, last_pkt, next_pkt, last_tstamp, last_nsamps ):
        if last_pkt:
            last_pkt.inc()
            if last_pkt.get_fsn() != next_pkt.get_fsn() :
                res['fsn']=self._TRACK_ERROR_

        if last_pkt and last_pkt.get_complex() != next_pkt.get_complex() :
            res['cplx']=self._TRACK_ERROR_

        if last_pkt and last_pkt.get_dmode() != next_pkt.get_dmode() :
            res['dmode']=self._TRACK_ERROR_

        if last_pkt and last_pkt.get_bps() != next_pkt.get_bps() :
            res['bps']=self._TRACK_ERROR_

        if last_pkt and last_pkt.get_freq() != next_pkt.get_freq() :
            res['freq']=self._TRACK_ERROR_

        if last_pkt and last_pkt.get_rate() != next_pkt.get_rate() :
            res['rate']=self._TRACK_ERROR_

        if last_pkt:
            if last_pkt.get_ttv(): res['ttv']=self._VALID_VAL_
            if last_pkt.get_ttv() != next_pkt.get_ttv():
                res['ttv']=self._TRACK_ERROR_

        if last_tstamp:
            # check that we have a good timestamp to check against
            t2 = next_pkt.get_SDDSTime()
            if next_pkt.get_ttv():
                if res['freq'] == False:
                    rate=pkt.get_freq();
                    offset = 1.0/rate*last_nsamps
                    t_ck = sdds_time.add( last_tstamp, offset )
                    if sdds_time.compare( t2, t_chk ) == False:
                        res['timeslip']=self._TRACK_ERROR_
                else:
                   res['timeslip']=self._TRACK_ERROR_

    def _first_pkt( self, res, next_pkt ):
        res['fsn'] = str(next_pkt.get_fsn())
        res['dmode']=str(next_pkt.get_dmode())
        res['cplx']=str(next_pkt.get_complex())
        res['bps']=str(next_pkt.get_bps())
        res['freq']=str(next_pkt.get_freq())
        res['rate']=str(next_pkt.get_rate())
        res['ttv']=str(next_pkt.get_ttv())
        t2 = next_pkt.get_SDDSTime()
        res['timeslip']=str(t2)
            
    def trackChanges(self, pkt_start=0, pkt_end=None, repeat_header=20, use_pager=True ):
        genf=self._gen_packet( self.raw_data_, pkt_start ) 
        if pkt_end == None: pkt_end = self.npkts_
        res = StringIO()
        keys = [ 'pkt', 'fsn', 'dmode', 'cplx', 'bps', 'freq', 'rate', 'ttv', 'timeslip' ]
        hdrs = [ 'PKT', 'SEQ', 'FMT', 'CPLX', 'BPS',  '    FREQ    ',  '    CLK    ', 'TIME VALID', 'TIME SLIP' ]
        hdr_fmt='{pkt:^5s} {fsn:^5s} {dmode:^5s} {cplx:^4s} {bps:^5s} {freq:^12s} {rate:^12s} {ttv:^10s} {timeslip:^9s}'
        line_fmt='{pkt:^5d} {fsn:^5s} {dmode:^5s} {cplx:^4s} {bps:^5s} {freq:^12s} {rate:^12s} {ttv:^10s} {timeslip:^9s}'
        last_pkt = None
        last_tstamp=None
        last_nsamps=0
        for i, pkt in enumerate(genf,pkt_start):
            if ( i % repeat_header ) == 0:
                print >>res, hdr_fmt.format( **dict(zip(keys,hdrs)))

            cmp_res = dict.fromkeys( keys, self._TRACK_OK_ )
            if i == pkt_start :
                self._first_pkt( cmp_res, pkt )
            else:
                self._cmp_pkt( cmp_res, last_pkt, pkt, last_tstamp, last_nsamps )
            cmp_res['pkt']=i

            dline=line_fmt.format( **cmp_res )
            print >>res, dline
            if pkt.get_ttv() : 
                last_tstamp=pkt.get_SDDSTime()
                last_nsamp=0

            # keep running count of samples
            last_nsamps += pkt.get_samples_for_bps()
            last_pkt=pkt

        if use_pager:
            _helpers.Pager( res.getvalue() )
        else:
            print res.getvalue()

    def getPacketIterator(self, pkt_start=0, pkt_end=None ):
        genf=self._gen_packet( self.raw_data_, pkt_start ) 
        if pkt_end == None: 
           pkt_end = self.npkts_ 
        else:
           pkt_end +=1
        for i, pkt in enumerate(genf,pkt_start):
            if i < pkt_end:
                yield i,pkt
            else:
                StopIteration

    def __iter__(self):
        return self.getPacketIterator()

    def __len__(self):
        return self.npkts_

    def getPackets(self, pkt_start=0, pkt_end=None ):
        res=[]
        for i, pkt in self.getPacketIterator( pkt_start, pkt_end ):
            res.append(pkt)
        return res
            
    def _gen_hex_dump( self, data, pkt_start, pkt_len, max_row_width=80, bytes_per_group=2 ):
        # break on pkt length
        bstart = pkt_start*self.pkt_len_
        pkt_iter=xrange( bstart, len(data), pkt_len)
        for x in pkt_iter:
            raw_pkt = data[x:x+max_row_width]
            d_iter = xrange(0, len(raw_pkt), bytes_per_group)
            yield  '  '.join( [ _hexify(raw_pkt[i:i+bytes_per_group]) for i in d_iter ] )

    def _gen_packet( self, data, pkt_start ):
        # break on pkt length
        bstart = pkt_start*self.pkt_len_
        pkt_iter=xrange( bstart, len(data), self.pkt_len_ )
        for x in pkt_iter:
           raw_pkt = data[x:x+self.pkt_len_]
           pkt=_sdds_pkt.sdds_packet(raw_pkt)
           yield pkt
            
