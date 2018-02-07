class StreamBase(object):
    def __init__(self, sri):
        self._sri = sri

    @property
    def streamID(self):
        return self._sri.streamID

    def sri(self):
        return self._sri

    @property
    def xstart(self):
        return self._sri.xstart

    @property
    def xdelta(self):
        return self._sri.xdelta

    @property
    def xunits(self):
        return self._sri.xunits

    @property
    def subsize(self):
        return self._sri.subsize

    @property
    def ystart(self):
        return self._sri.ystart

    @property
    def ydelta(self):
        return self._sri.ydelta

    @property
    def yunits(self):
        return self._sri.yunits

    @property
    def complex(self):
        return self._sri.mode != 0

    @property
    def blocking(self):
        return self._sri.blocking

    def keywords(self):
        return self._sri.keywords

    def hasKeyword(self, name):
        for dt in self._sri.keywords:
            if dt.id == name:
                return True
        return False

    def getKeyword(self, name):
        for dt in self._sri.keywords:
            if dt.id == name:
                return dt.value._v
        raise KeyError(name)
