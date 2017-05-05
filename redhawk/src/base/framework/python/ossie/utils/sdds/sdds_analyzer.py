
from  binascii import hexlify as _hexify
from   StringIO import StringIO
import ossie.utils.sb.helpers as _helpers
import sdds_pkt as _sdds_pkt
import traceback

__all__ = [ 'SDDSAnalyzer' ]

class SDDSAnalyzer(object):
    """
    
    Return a SDDS Analyzer object that can process a set list of raw bytes as SDDS packets and can perform the 
    following actions:

    trackChanges - montior fsn, bps, dmode, freq, sample rate, time stamps, ttv, changes between runs of packets 
    dumpRawPackets() - dump raw packets with contents managed by an optional Pager
    dumpPackets() - dump packets with bit representations, contents managed by an optional Pager
    getPackets - returns a generator object for access packets in a looping construct
       
    """
    _VAILID_VAL_='+'
    _TRACK_OK_='-'
    _TRACK_ERROR_='***'

    def __init__(self, pkts, data, raw_data, total_bytes_read, pkt_len=1080 ):
        self.pkts_ = pkts
        self.data_ = data
        self.raw_data_ = raw_data
        self.pkt_len_ = pkt_len
        self.total_bytes_read = total_bytes_read

    def dumpRawPackets(self, pkt_start=0, pkt_end=None, row_width=80, bytes_per_group=2, pkt_len=None, use_pager=True ):
        if pkt_end == None: 
            pkt_end = self.pkts_
            if pkt_len:
                pkt_end = self.pkts_

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
        if pkt_end == None: pkt_end = self.pkts_
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
            if next_pkt.get_ttv():
                if res['freq'] == False:
                    rate=pkt.get_freq();
                    t2 = next_pkt.get_time()
                    offset = 1.0/rate*last_nsamps
                    t_ck = sdds_time.add( last_tstamp, offset )
                    if sdds_time.compare( t2, t_chk ) == False:
                        res['timeslip']=self._TRACK_ERROR_
                else:
                   res['timeslip']=self._TRACK_ERROR_

            
    def trackChanges(self, pkt_start=0, pkt_end=None, repeat_header=20, use_pager=True ):
        genf=self._gen_packet( self.raw_data_, pkt_start ) 
        if pkt_end == None: pkt_end = self.pkts_
        res = StringIO()
        keys = [ 'pkt', 'fsn', 'dmode', 'bps', 'freq', 'rate', 'timeslip', 'ttv' ]
        hdrs = [ 'PKT', 'SEQ', 'FMT', 'BPS',  'FREQ',  'CLK', 'TIME SLIP', 'TIME VALID']
        hdr_fmt='{pkt:^5s} {fsn:^5s} {dmode:^5s} {bps:^5s} {freq:^5s} {rate:^5s} {timeslip:^9s} {ttv:^10s}'
        line_fmt='{pkt:^5d} {fsn:^5s} {dmode:^5s} {bps:^5s} {freq:^5s} {rate:^5s} {timeslip:^9s} {ttv:^10s}'
        last_pkt = None
        last_tstamp=None
        last_nsamps=0
        for i, pkt in enumerate(genf,pkt_start):
            if ( i % repeat_header ) == 0:
                print >>res, hdr_fmt.format( **dict(zip(keys,hdrs)))

            cmp_res = dict.fromkeys( keys, self._TRACK_OK_ )
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
        if pkt_end == None: pkt_end = self.pkts_
        for i, pkt in enumerate(genf,pkt_start):
            if i < pkt_end:
                yield i,pkt
            else:
                StopIteration

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
            
