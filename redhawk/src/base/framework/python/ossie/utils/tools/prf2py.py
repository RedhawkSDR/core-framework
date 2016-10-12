#!/usr/bin/env python
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

#
# Generates an immutable python data structure represeting the
# contents of a python PRF file.  If run from the command line,
# it will generate a python file suitable for import
from ossie import parsers
import sys

class PropertyDefs:
    TUPLE_NAMES = ("ID", "NAME", "TYPE", "MODE", "DEFAULT", "UNITS", "ACTION", "KINDS")

    def __init__(self, tupledef):
        self.__dict__['tupledef'] = tupledef
    
    def __setattr__(self, name, value):
        # Make PropertyDef immutable
        pass

    def __str__(self):
        output = []
        output.append("PROPERTIES = (")
        for prop in self.tupledef:
            output.append("              (")
            for index, name in enumerate(self.TUPLE_NAMES):
                output.append("               %s, # %s" % (repr(prop[index]), name))
            output.append("              ),")
        output.append("             )")
        return "\n".join(output)

def convertValue(type, value):
    if value == None:
        return None
    if type == "boolean":
        return {"TRUE": True, "FALSE": False}[value.strip().upper()]
    elif type in ("short", "long", "ulong", "ushort"):
        return int(value)
    elif type == "double":
        return float(value)
    elif type == "float":
        return float(value)
    else:
        return str(value)

def readPRF(filename):
    prf = parsers.PRFParser.parse(filename)
    props = []
    for property in prf.get_simple():
        if property.get_action():
            action = property.get_action().get_type()
        else:
            action = "external"

        if not action in ("eq", "ne", "gt", "lt", "ge", "le", "external"):
            raise StandardError("Invalid action")

        if property.get_mode():
            mode = property.get_mode()
        else:
            mode = "readwrite"

        for k in property.get_kind():
            if not k.get_kindtype() in ("allocation", "configure", "test", "execparam", "factoryparam"):
                raise StandardError("Invalid action %s for %s" % (action, property.get_id()))
        kinds = [ k.get_kindtype() for k in property.get_kind()]
        if len(kinds) == 0:
            kinds = ["configure"]
    
        p = (property.get_id(),
             property.get_name(),
             property.get_type(),
             mode,
             convertValue(property.get_type(), property.get_value()),
             property.get_units(),
             action,
             tuple(kinds))
        props.append(p)

    for property in prf.get_simplesequence():
        if property.get_action():
            action = property.get_action().get_type()
        else:
            action = "external"

        if property.get_mode():
            mode = property.get_mode()
        else:
            action = "readwrite"

        for k in property.get_kind():
            if not k.get_kindtype() in ("allocation", "configure", "test", "execparam", "factoryparam"):
                raise StandardError("Invalid action %s for %s" % (action, property.get_id()))
        kinds = [ k.get_kindtype() for k in property.get_kind()]
        if len(kinds) == 0:
            kinds = ["configure"]
        
        values = None
        if property.get_values() != None:
            values = tuple([ convertValue(property.get_type(), v) for v in property.get_values().get_value()]) 

        p = (property.get_id(),
             property.get_name(),
             property.get_type(),
             mode,
             values,
             property.get_units(),
             action,
             tuple(kinds))
        props.append(p)
    return PropertyDefs(tuple(props))

if __name__ == "__main__":
    import os.path
    import time
    print "#!/usr/bin/env python"
    print "#"
    print "# AUTO-GENERATED CODE.  DO NOT MODIFY!"
    print "#"
    print "# Source: %s" % os.path.basename(sys.argv[1])
    print "# Generated on: %s" % time.asctime()
    print ""

    p = readPRF(sys.argv[1])
    print p
    print ""

