#!/usr/bin/env python
#
#
# AUTO-GENERATED
#
# Source: EmptyString.spd.xml
from ossie.resource import start_component
import logging

from EmptyString_base import *

class EmptyString_i(EmptyString_base):
    """<DESCRIPTION GOES HERE>"""
    def __init__(self, identifier, execparams):
        EmptyString_base.__init__(self,identifier,execparams)
        if not self.estr:
            self.estr="ctor-value"

    def constructor(self):
        """
        """
        # TODO add customization here.

    def process(self):
        # TODO fill in your code here
        self._log.debug("process() example log message")
        return NOOP


if __name__ == '__main__':
    logging.getLogger().setLevel(logging.INFO)
    logging.debug("Starting Component")
    start_component(EmptyString_i)
