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

from ossie.utils.idl import omniidl
from omniidl import idlast, idlvisitor, idlutil, main, idltype
from ossie.utils.idl import _omniidl
import os
import threading
from omniORB import CORBA

#import base

_lock = threading.Lock()

valList = ('null','void','short','long','ushort','ulong','float','double','boolean',
           'char','octet','any','TypeCode','Principal','objref','struct','union','enum',
           'string','sequence','array','alias','except','longlong','ulonglong',
           'longdouble','wchar','wstring','fixed','value','value_box','native',
           'abstract_interface','local_interface')
baseTypes = dict(enumerate(valList))

# Non-standard kinds for forward-declared structs and unions
baseTypes[100] = 'ot_structforward'
baseTypes[101] = 'ot_unionforward'

# Mapping of CORBA typecode numeric values to TCKind objects; this construction
# of the table is dependent on omniORB, but supporting another ORB should be
# trivial.
_kindMap = dict((tk._v, tk) for tk in CORBA.TCKind._items)
_baseKinds = (
    CORBA.tk_void,
    CORBA.tk_short,
    CORBA.tk_long,
    CORBA.tk_ushort,
    CORBA.tk_ulong,
    CORBA.tk_float,
    CORBA.tk_double,
    CORBA.tk_boolean,
    CORBA.tk_char,
    CORBA.tk_octet,
    CORBA.tk_any,
    CORBA.tk_string,
    CORBA.tk_longlong,
    CORBA.tk_ulonglong,
)

# Generic representation of IDL types that does not strictly depend on omniORB or omniidl.
class IDLType(object):
    def __init__(self, kind):
        self._kind = kind

    def __str__(self):
        return baseTypes[self.kind()._v]

    def kind(self):
        return self._kind

    @classmethod
    def instance(cls, type):
        kind = _kindMap[type.kind()]
        if kind in _baseKinds:
            return BaseType(kind)
        elif kind == CORBA.tk_sequence:
            sequenceType = IDLType.instance(type.seqType())
            return SequenceType(sequenceType)
        elif kind == CORBA.tk_alias:
            aliasType = IDLType.instance(type.decl().alias().aliasType())
            return AliasType(aliasType, type.scopedName())
        elif kind == CORBA.tk_TypeCode:
            return NamedType(kind, ['CORBA', 'TypeCode'])
        elif kind == CORBA.tk_enum:
            values = [EnumValue(en.scopedName()) for en in type.decl().enumerators()]
            return EnumType(type.scopedName(), values)
        else:
            return NamedType(kind, type.scopedName())

class BaseType(IDLType):
    pass

class SequenceType(IDLType):
    def __init__(self, sequenceType):
        super(SequenceType,self).__init__(CORBA.tk_sequence)
        self._sequenceType = sequenceType

    def sequenceType(self):
        return self._sequenceType

class NamedType(IDLType):
    def __init__(self, kind, scopedName):
        super(NamedType,self).__init__(kind)
        self._scopedName = scopedName

    def __str__(self):
        return '::'.join(self.scopedName())

    def scopedName(self):
        return self._scopedName

class AliasType(NamedType):
    def __init__(self, aliasType, scopedName):
        super(AliasType,self).__init__(CORBA.tk_alias, scopedName)
        self._aliasType = aliasType
    
    def __str__(self):
        return str(self.aliasType())

    def aliasType(self):
        return self._aliasType

class EnumValue(object):
    def __init__(self, scopedName):
        self._scopedName = scopedName

    def identifier(self):
        return self._scopedName[-1]

    def scopedName(self):
        return self._scopedName

class EnumType(NamedType):
    def __init__(self, scopedName, values):
        super(EnumType,self).__init__(CORBA.tk_enum, scopedName)
        self._enumValues = values

    def enumValues(self):
        return self._enumValues

def ExceptionType(type):
    return NamedType(CORBA.tk_except, type.scopedName())

# Internal cache of parsed IDL interfaces
_interfaces = {}

class Interface:
    def __init__(self,name,nameSpace="",operations=[],filename="",fullpath="",repoId=""):
        self.name = name
        self.nameSpace = nameSpace
        self.attributes = []
        self.operations = []
        self.filename = filename    #does not include the '.idl' suffix
        self.fullpath = fullpath
        self.inherited_names = []
        self.inherited = []
        self.inherits = set()
        self.repoId = repoId

    def __eq__(self,other):
        if isinstance(other, Interface):
            return (other.nameSpace == self.nameSpace ) and (other.name == self.name)
        else:
            return False

    def __ne__(self,other):
        if isinstance(other, Interface):
            return (other.nameSpace != self.nameSpace ) and (other.name != self.name)
        else:
            return True

    def __repr__(self):
        retstr = '{'
        retstr += "'name':'" + self.name + "',"
        retstr += "'nameSpace':'" + self.nameSpace + "',"
        retstr += "'filename':'" + self.filename + "',"
        retstr += "'fullpath':'" + self.fullpath + "',"
        retstr += "'attributes':[" + ','.join(str(attr) for attr in self.attributes) + "],"
        retstr += "'operations':[" + ','.join(str(op) for op in self.operations) + "]}"

        return retstr


class Operation:
    def __init__(self,name,returnType,params=[]):
        self.name = name
        self.returnType = returnType
        self.params = []
        self.raises = []

    def __repr__(self):
        retstr = '{'
        retstr += "'name':'" + self.name + "',"
        retstr += "'returnType':'" + str(self.returnType) + "',"
        retstr += "'params':[" + ','.join(str(p) for p in self.params) + '],'
        retstr += "'raises':[" + ','.join("'%s'" % r for r in self.raises) + ']}'

        return retstr

class Attribute:
    def __init__(self,name,readonly,attrType):
        self.name = name
        self.readonly = readonly
        self.attrType = attrType

    def __repr__(self):
        retstr = '{'
        retstr += "'name':'" + self.name + "',"
        retstr += "'readonly':'" + str(self.readonly) + "',"
        retstr += "'attrType':'" + str(self.attrType) + "'"
        retstr += '}'

        return retstr

class Param:
    def __init__(self,name,paramType='',direction=''):
        """
        Exampleinterface complexShort {
            void pushPacket(in PortTypes::ShortSequence I, in PortTypes::ShortSequence Q);
        };
        """

        self.name = name            # The actual argument name: 'I'
        self.paramType = paramType    # The type of the argument: 'PortTypes::ShortSequence'
        self.direction = direction  # Flow of data: 'in'

    def __repr__(self):
        retstr = '{'
        retstr += "'name':'" + self.name + "',"
        retstr += "'paramType':'" + str(self.paramType) + "',"
        retstr += "'direction':'" + self.direction + "'}"

        return retstr

class InterfaceVisitor(idlvisitor.AstVisitor):
    def _getInheritedRepoIDs(self, node):
        ifset = set()
        for parent in node.inherits():
            ifset.update(self._getInheritedRepoIDs(parent))
            ifset.add(parent.repoId())
        return ifset

    def visitInterface(self, node):
        self.interface = Interface(node.identifier(),node.scopedName()[0],repoId=node.fullDecl().repoId())
        self.interface.fullpath = node.file()
        for call in node.all_callables():
            call.accept(self)

        # find and store any inheritances
        self.interface.inherited_names = [(i.scopedName()[0],i.identifier()) for i in node.inherits()]
        self.interface.inherits = self._getInheritedRepoIDs(node)

    def visitAttribute(self, node):
        # create the Attribute object
        decl = node.declarators()[0]
        kind = node.attrType().kind()
        if (kind==idltype.tk_alias): # resolve the 'alias'
            kind = node.attrType().decl().alias().aliasType().kind()
        if hasattr(node.attrType(),'scopedName'):
            dataType = idlutil.ccolonName(node.attrType().scopedName())
        else:
            dataType = baseTypes[kind]
        new_attr = Attribute(decl.identifier(),node.readonly(),IDLType.instance(node.attrType()))

        self.interface.attributes.append(new_attr)

    def visitOperation(self, node):
        # create the Operation object
        new_op = Operation(node.identifier(),IDLType.instance(node.returnType()))

        # find and process the parameters of the operation
        for p in node.parameters():
            new_param = Param(p.identifier(), IDLType.instance(p.paramType()))
            if p.is_in() and p.is_out():
                new_param.direction = 'inout'
            elif p.is_out():
                new_param.direction = 'out'
            else:
                new_param.direction = 'in'
            new_op.params.append(new_param)

        new_op.raises = [ExceptionType(r) for r in node.raises()]

        self.interface.operations.append(new_op)


class ExampleVisitor (idlvisitor.AstVisitor):
    def __init__(self,*args):
        self.myInterfaces = []   #Used to store the interfaces that are found
        if hasattr(idlvisitor.AstVisitor,'__init__'):
            idlvisitor.AstVisitor.__init__(self,args)

    def visitAST(self, node):
        for n in node.declarations():
            n.accept(self)

    def visitModule(self, node):
        for n in node.definitions():
            n.accept(self)

    def visitInterface(self, node):
        # Use cached post-processed interface if available
        global _interfaces
        interface = _interfaces.get(node.repoId(), None)
        if not interface:
            visitor = InterfaceVisitor()
            node.accept(visitor)
            interface = visitor.interface
            _interfaces[node.repoId()] = interface
        self.myInterfaces.append(interface)

def run(tree, args):
    visitor = ExampleVisitor()
    tree.accept(visitor)
    return visitor.myInterfaces

def _locateIncludedFile(filename, includepath):
    for ipath in includepath:
        fullpath = os.path.join(ipath, filename)
        if os.path.exists(fullpath):
            return fullpath
    return filename

def getInterfacesFromFile(filename, includepath=None):
    popen_cmd = main.preprocessor_cmd
    if includepath:
        for newpath in includepath:
            popen_cmd += ' -I "' + newpath + '"'
    popen_cmd += ' "' + filename + '"'
    f = os.popen(popen_cmd, 'r')

    _lock.acquire()
    try:
        try:
            tree = _omniidl.compile(f)
        except TypeError:
            tree = _omniidl.compile(f, filename)
        if tree == None:
            return []

        try:
            ints = run(tree,'')
        except:
            pass
        f.close()
        del tree
        idlast.clear()
        idltype.clear()
        _omniidl.clear()
    finally:
        _lock.release()

    #store file name and location information for each interface
    for x in ints:
        if not x.fullpath.startswith('/'):
            x.fullpath = _locateIncludedFile(x.fullpath, includepath)
        x.filename = os.path.basename(x.fullpath)
        x.filename = x.filename[:-4] #remove the .idl suffix

    return ints

def getInterfacesFromFileAsString(filename, includepath=None):
    ints = getInterfacesFromFile(filename, includepath)
    ifaces = ""
    for x in ints:
        ifaces = ifaces + str(x) + "\n"

    return ifaces

def importStandardIdl(std_idl_path='/usr/local/share/idl/ossie', std_idl_include_path = '/usr/local/share/idl'):

    # list to hold any include paths the parser may need
    includePaths = []

    #Don't eat user supplied arguments
    if std_idl_path == '/usr/local/share/idl/ossie' and std_idl_include_path == '/usr/local/share/idl':

        # find where ossie is installed
        if 'OSSIEHOME' in os.environ and os.path.exists(os.environ['OSSIEHOME']):
            ossiehome_location = os.environ['OSSIEHOME']
            std_idl_path = os.path.join(ossiehome_location, 'share/idl')
            std_idl_include_path = os.path.join(ossiehome_location, 'share/idl')
            includePaths.append(std_idl_include_path)

            #In a local installation, omniORB idls are placed in $OSSIEHOME/share/idl
            includePaths.append(os.path.join(ossiehome_location, 'share/idl/omniORB'))
            includePaths.append(os.path.join(ossiehome_location, 'share/idl/omniORB/COS'))

    if not std_idl_include_path in includePaths:
        includePaths.append(std_idl_include_path)

    #Append standard omniORB idl location
    includePaths.append('/usr/share/idl/omniORB')
    includePaths.append('/usr/share/idl/omniORB/COS')

    #Append additional omniORB idl location
    includePaths.append('/usr/local/share/idl/omniORB')
    includePaths.append('/usr/local/share/idl/omniORB/COS')


    # normalize the path names
    std_idl_path = os.path.normpath(std_idl_path)

    if not os.path.exists(std_idl_path):
        print "Cannot find OSSIE installation location:\n" + std_idl_path
        return

    # list to hold IDL files
    idlList = []

    for (directory,sub,files) in os.walk(std_idl_include_path):
        includePaths.append(directory)

    # Add the CF interfaces first - in case another file includes them, we
    # don't want them asscociated with anything other than cf.idl
    cfdir = os.path.join(std_idl_path, "ossie/CF")
    if not os.path.isdir(cfdir):
        print "Cannot find CF idl files in the OSSIE installation location:\n" + cfdir

    for file in os.listdir(cfdir):
        if os.path.splitext(file)[1] == '.idl':
            if file not in idlList:
                idlList.append(os.path.join(cfdir,file))

    # search for .idl files recursively
    for dirpath, dirs, files in os.walk(std_idl_path):
        for f in files:
            #Omit problematic IDLs
            if not 'omniORB' in dirpath and not 'omniEvents' in dirpath:
                if os.path.splitext(f)[1] == '.idl':
                    idlList.append(os.path.join(dirpath, f))

    if len(idlList) <= 0:
        tmpstr = "Can't find any files in: " + std_idl_path
        print tmpstr
        return

    # Add the CF interfaces first - in case another file includes them, we
    # don't want them asscociated with anything other than cf.idl
    Available_Ints = []

    parsed = set()
    for idl_file in idlList:
        # Don't reparse files
        if idl_file in parsed:
            continue

        basename_idl_file = os.path.basename(idl_file)
        # standardIdl files are not included because they are aggregates of the other interfaces
        if 'standardIdl' in basename_idl_file:
            continue

        # customInterfaces files are not included because they are aggregates of the other interfaces
        if 'customInterfaces' in basename_idl_file:
            continue

        tempInts = getInterfacesFromFile(idl_file, includePaths)
        parsed.add(idl_file)
        for t in tempInts:
            # Mark the source file as parsed
            # NB: This can allow further optimizations to build interfaces from all .idl files
            #     seen during the parsing of a file, though it is order-dependent.
            parsed.add(t.fullpath)
            if t not in Available_Ints:
                Available_Ints.append(t)

    # correlate inherited interfaces
    for int1 in Available_Ints:
        for ns, inherited_int in int1.inherited_names:
            for int2 in Available_Ints:
                if ns == int2.nameSpace and inherited_int == int2.name:
                    int1.inherited.append(int2)
                    continue

    return Available_Ints


if __name__ == '__main__':
    # parse args
    from optparse import OptionParser
    parser = OptionParser()
    parser.add_option("--string", dest="string", default=False, action="store_true",
        help="Return interfaces as a string.")
    parser.add_option("-f", dest="filepath", default=None,
        help="File to parse.")
    parser.add_option("-i", dest="include", default=None,
        help="Comma-separated list of include paths.")
    parser.add_option("--cpp", dest="cpp", default=False, action="store_true",
        help="Return C++ language mappings.")

    (options, args) = parser.parse_args()

    if not options.filepath:
        parser.error("Must have a file")

    if options.string:
        includepaths = None
        if options.include:
            includepaths = [x for x in options.include.split(",") if x]

        print getInterfacesFromFileAsString(options.filepath, includepaths)
