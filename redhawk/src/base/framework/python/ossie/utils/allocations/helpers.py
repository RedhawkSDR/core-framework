#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
from ossie.cf import CF
import copy
from ossie import properties
import ossie.parsers.prf
from omniORB import tcInternal, CORBA, any

class WrongInputType(Exception):
    pass

class BadValue(Exception):
    pass

class MissingProperty(Exception):
    pass

def tvCode(name):
    if name == 'short':
        return tcInternal.tv_short
    elif name == 'long':
        return tcInternal.tv_long
    elif name == 'ushort':
        return tcInternal.tv_ushort
    elif name == 'ulong':
        return tcInternal.tv_ulong
    elif name == 'float':
        return tcInternal.tv_float
    elif name == 'double':
        return tcInternal.tv_double
    elif name == 'boolean':
        return tcInternal.tv_boolean
    elif name == 'char':
        return tcInternal.tv_char
    elif name == 'octet':
        return tcInternal.tv_octet
    elif name == 'string':
        return tcInternal.tv_string
    elif name == 'longlong':
        return tcInternal.tv_longlong
    elif name == 'ulonglong':
        return tcInternal.tv_ulonglong
    elif name == 'double':
        return tcInternal.tv_longdouble
    elif name == 'wchar':
        return tcInternal.tv_wchar
    raise BadValue('Could not find the sequence code for '+name)

def createRequest(requestId=None, props={}, pools=[], devices=[], sourceId=''):
    return CF.AllocationManager.AllocationRequestType(requestId, props, pools, devices, sourceId)

def allTypes():
    retval = []
    for _t in properties.getTypeMap():
        retval.append(_t)
    return retval

def __typeCheck(props):
    if not isinstance(props, list):
        raise WrongInputType('The argument "props" must be a list of properties (i.e.: CF.DataType). Use properties.props_from_dict')
    if len(props) == 0:
        raise MissingProperty('The list of properties must be greater than zero')
    for prop in props:
        if not isinstance(prop, CF.DataType):
            raise WrongInputType('The argument "props" must be a list of properties (i.e.: CF.DataType). Use properties.props_from_dict')

def __getProp(props, prop_id):
    for prop in props:
        if prop.id == prop_id:
            return prop
    raise MissingProperty('Could not find the property with id: '+str(prop_id))

def __getPropIdx(props, prop_id):
    idx = 0
    for prop in props:
        if prop.id == prop_id:
            return idx
        idx = idx + 1
    raise MissingProperty('Could not find the property with id: '+str(prop_id))

def getType(props, prop_id, typeCheck=True):
    '''
       return the property type for the element in the props sequence with id of prop_id
    '''
    if typeCheck:
        __typeCheck(props)
    prop = __getProp(props, prop_id)
    return properties.getTypeNameFromTC(prop.value._t)

def setType(props, prop_id, _type, typeCheck=True):
    '''
       change the property type for the element in the props sequence with id of prop_id
       This method returns a copy of props (does not modify the props input)
    '''
    if typeCheck:
        __typeCheck(props)
    if not properties.getTypeMap().has_key(_type):
        raise BadValue('Type "'+_type+'" does not exist')
    if _type == getType(props, prop_id, False):
        return props
    prop_idx = __getPropIdx(props, prop_id)
    ret_props = copy.deepcopy(props)
    if props[prop_idx].value._t._k == CORBA.tk_sequence:
        ret_props[prop_idx].value._t._d = (props[prop_idx].value._t._d[0], tvCode(_type), props[prop_idx].value._t._d[2])
    else:
        ret_props[prop_idx].value = properties.to_tc_value(props[prop_idx].value,_type)
    return ret_props

def matchTypes(props, prop_ids=[], prf=None):
    '''
       match the property type for the element in the props sequence with id of prop_ids to the type specified in the prf file specified
       This method returns a copy of props (does not modify the props input)
       
       if prop_ids is not length 0, only those properties are updated
       if prop_ids is length 0, all matching properties (props vs prf xml string) are matched
       
       if prf file is one of the following:
         - a Python open file handle; e.g.: fp = open('/data/rh/sdrroot/dom/foo.prf.xml','r')
         - a CF::File object; e.g.: fp=fileMgr.open('/foo.prf.xml',True)
         - a local filename; e.g.: '/data/rh/sdrroot/dom/foo.prf.xml'
    '''
    __typeCheck(props)

    prfcontents=''
    if prf:
        if isinstance(prf, CF._objref_File):
            _len = len(prfcontents)
            while True:
                prfcontents += fp.read(10000)
                if _len == len(prfcontents):
                    break
                _len = len(prfcontents)
            prf.close()
        elif isinstance(prf, file):
            prfcontents = prf.read()
            prf.close()
        elif isinstance(prf, str):
            fp = open(prf, 'r')
            prfcontents = fp.read()
            fp.close()
        else:
            raise WrongInputType('The prf argument must be: a string, a file object, or a CF.File object')

    parsedPrf = ossie.parsers.prf.parseString(prfcontents)
    classes = {}

    structs = parsedPrf.get_struct()# + [_prop.get_struct() for _prop in parsedPrf.get_structsequence()]
    structsseq = [_prop.get_struct() for _prop in parsedPrf.get_structsequence()]
    for _prop in parsedPrf.get_struct():
        name = _prop.id_
        if _prop.name:
            name = _prop.name
        clazz = properties.xml_to_class(_prop)
        classes[name] = clazz
    for _prop in parsedPrf.get_structsequence():
        name = _prop.id_
        if _prop.name:
            name = _prop.name
        clazz = properties.xml_to_class(_prop.struct)
        # mark as structseq by setting to length 1 list
        classes[name] = [clazz]
    for _prop in parsedPrf.get_simple():
        name = _prop.id_
        if _prop.name:
            name = _prop.name
        classes[name] = _prop
    for _prop in parsedPrf.get_simplesequence():
        name = _prop.id_
        if _prop.name:
            name = _prop.name
        classes[name] = _prop

    if len(prop_ids) != 0:
        for prop_id in prop_ids:
            if not isinstance(prop_id, str):
                raise WrongInputType('prop_id must be either a string or None')
            if not classes.has_key(prop_id):
                raise MissingProperty(prop_id+' is not defined in the given reference prf file')
            props = setType(props, prop_id, classes[prop_id].type_, typeCheck=False)
        return props

    for _prop in props:
        if not classes.has_key(_prop.id):
            raise MissingProperty('props contains property '+_prop.id+', but the given reference prf file does not have it defined')
        if isinstance(classes[_prop.id], ossie.parsers.prf.simple) or isinstance(classes[_prop.id], ossie.parsers.prf.simpleSequence):
            props = setType(props, _prop.id, classes[_prop.id].type_, typeCheck=False)
        else:
            if isinstance(classes[_prop.id], list):
                _idx = __getPropIdx(props, _prop.id)
                item_idx = 0
                for item in props[_idx].value._v:
                    for field in classes[_prop.id][0].__fields:
                        props[_idx].value._v[item_idx]._v = setType(props[_idx].value._v[item_idx]._v, field.id_, field.type_, typeCheck=False)
                    item_idx += 1
            else:
                _idx = __getPropIdx(props, _prop.id)
                for field in classes[_prop.id].__fields:
                    props[_idx].value._v = setType(props[_idx].value._v, field.id_, field.type_, typeCheck=False)

    return props

def createProps(prop_dict, prf=None):
    props = []
    for _key in prop_dict:
        if isinstance(prop_dict[_key],dict):
            vals = []
            for _subkey in prop_dict[_key]:
                vals.append(CF.DataType(id=_subkey, value=any.to_any(prop_dict[_key][_subkey])))
            props.append(CF.DataType(id=_key, value = any.to_any(vals)))
        elif isinstance(prop_dict[_key],list):
            if len(prop_dict[_key]) == 0:
                props.append(CF.DataType(id=_key, value=any.to_any(prop_dict[_key])))
            elif isinstance(prop_dict[_key][0],dict):
                vals = []
                for _item in prop_dict[_key]:
                    subval = []
                    for _subkey in _item:
                        subval.append(CF.DataType(id=_subkey, value=any.to_any(_item[_subkey])))
                    vals.append(any.to_any(subval))
                props.append(CF.DataType(id=_key, value = any.to_any(vals)))
            else:
                props.append(CF.DataType(id=_key, value=any.to_any(prop_dict[_key])))
        else:
            props.append(CF.DataType(id=_key, value=any.to_any(prop_dict[_key])))
    if prf:
        props = matchTypes(props, prf=prf)
    return props
