
import os
import re
import fileinput
import sys
import subprocess
import traceback
#import urllib.parse
import json
import uuid
import shutil
import tempfile
import pprint
import logging
import contextlib
import copy
from argparse import ArgumentParser, RawDescriptionHelpFormatter
from textwrap import TextWrapper
import yaml

if sys.version_info[0] == 3:
    from io import StringIO
else:
    from StringIO import StringIO

    def __debug(*args):
        logging.debug('args: %s', args)
        return args[0]

    '''
    allow StringIO to use `with` statement
    '''
    StringIO.__exit__ = __debug
    StringIO.__enter__ = __debug

from jinja2 import Environment, FileSystemLoader
#from jinja2.ext import debug

from redhawk.codegen.lang.idl import CorbaTypes, IDLInterface


_rh_template_paths=['templates', 'redhawk/packagegen/templates']
try:
    import redhawk.packagegen.templates
    _rh_template_paths.append(redhawk.packagegen.templates.__path__[0])
except:
    pass



logger=None

BOLD = "\033[;1m"
RESET = "\033[0;0m"
FORMAT = '%(asctime)-15s %(levelname)-5s - %(message)s'
logger = logging.getLogger('updates')



#################################
# Convenience routines
#################################


@contextlib.contextmanager
def pushd(new_dir):
    previous_dir = os.getcwd()
    os.chdir(new_dir)
    try:
        yield
    finally:
        os.chdir(previous_dir)


def configureLogging(lvl='info',ofile=sys.stdout):
    global logger

    print("logging level:", lvl, " log file:", ofile)
    _lvl=logging.INFO
    if lvl.upper() == 'CRITICAL':
        _lvl=logging.CRITICAL
    if lvl.upper() == 'ERROR':
        _lvl=logging.ERROR
    if lvl.upper() == 'WARN':
        _lvl=logging.WARN
    if lvl.upper() == 'DEBUG':
        _lvl=logging.DEBUG

    logging.basicConfig(
        filename=ofile,
        level=_lvl,
        format=FORMAT)

#
# get_macros
#
# build a dictionary of macro strings and values that can be used in
# when filling in the fei_devices.yml file
#
# %%sdrroot%%      - expands to environment variable
# %%ossiehome%%    - expands to environment variable
# %%deps%%         - expands to $SDRROOT/deps
# %%devices%%      - expands to $SDRROOT/dev/devices
#
def get_macros(args=None):
    """
    create list of macros that can be expanded
    """
    macros={}
    # SDRROOT
    _sdrroot=args.sdrroot
    macros["%%sdrroot%%"]=_sdrroot

    # OSSIEHOME
    _ossiehome=args.ossiehome
    macros["%%ossiehome%%"]=_ossiehome

    # Dependencies install path
    macros["%%deps%%"]=os.path.join(_sdrroot,"dom/deps")

    # Devices install path
    macros["%%devices%%"]=os.path.join(_sdrroot,"dev/devices")

    return macros


#
# expand_macros
#
# perform a text replace of macro strings in a source string
#
# @param source    - string to search and replace macros
# @param macros    - dictionary of macros and their substitutions
# @returns string  - source string with macro replacements

def expand_macros(source, macros):
    """
    Perform a find-and-replace on the source based on the find/replace
    values stored in macros
    """
    text=source
    if source and macros:
        for i, j in macros.items():
            try:
                text = text.replace(i, str(j))
            except:
                logger.error("Unable to apply macro {} and value {}".format(i,j))
    return text


#
# byteify_text
#
# replace unicode formatting
#
def byteify_text(data, ignore_dicts = False):
    """
    Remove unicode from yaml processing
    """
    # if this is a unicode string, return its string representation
    if isinstance(data, unicode):
        return data.encode('utf-8')
    # if this is a list of values, return list of byteified values
    if isinstance(data, list):
        return [ byteify_text(item, ignore_dicts=True) for item in data ]
    # if this is a dictionary, return dictionary of byteified keys and values
    # but only if we haven't already byteified it
    if isinstance(data, dict) and not ignore_dicts:
        return {
            byteify_text(key, ignore_dicts=True): byteify_text(value, ignore_dicts=True)
            for key, value in data.iteritems()
        }
    # if it's anything else, return it in its original form
    return data


#################################################
#
#  Redhawk 3.0 Interface Definitions
#
# The following dictionaries are used to build a
# SCA Software Component descriptor file.
#
#################################################


#
# Mapping of a device's port type to SCA/REDHAWK Interfaces
#
# These device port attributes only list a port's name
# and not their type to reduce user input.
#
# data_inputs and data_outputs will refer to specific data typed ports
# and are not included here.
#
#
rh3_fei3_yaml_port_types= { 'rf_info' :  'FRONTEND/RFInfo',
                            'rf_source' :  'FRONTEND/RFSource',
                            'gps' : 'FRONTEND/GPS',
                            'nav' : 'FRONTEND/NavData',
                            'message' : 'ExtendedEvent/MessageEvent' }

#
# Mapping of different device types for a FEI3 device
#
rh3_fei3_yaml_sca_types= { 'DEVICE' :  'CF/Device',
                           'AGGREGATE_PLAIN_DEVICE' : 'CF/AggregatePlainDevice' }


#
# FEI 3.0 Device Types
#
rh3_fei3_device_types= { 'ANTENNA' :  'antenna',
                         'RX' :  'rx_analog',
                         'RX_ARRAY' : 'rx_array',
                         'ABOT' : 'analog_tuner_bank',
                         'ARDC' : 'rx_analog_digital',
                         'DBOT' : 'digital_tuner_bank',
                         'RDC' : 'rdc_tuner',
                         'SRDC' : 'digital_snapshot',
                         'DRDC' : 'digital_delay',
                         'TX' : 'tx_analog',
                         'TX_ARRAY' : 'tx_array',
                         'TDC' : 'tdc_tuner'
                         }


def format_interface_name(repid):
    if not repid.startswith('IDL:'):
        repid='IDL:{}'.format(repid)

    if not repid.endswith(':1.0'):
        repid='{}:1.0'.format(repid)

    return repid

def _resolve_supported_interfaces(interface):
    supportsinterface = []

    # check/format repid to be idl:<interface>:1.0
    repid=format_interface_name(interface)
    idl = IDLInterface(repid)
    supportsinterface.append((idl.repid(), idl.interface()))

    for parent in idl.idl().inherits:
        idl = IDLInterface(parent)
        supportsinterface.append((idl.repid(), idl.interface()))

    return supportsinterface


#
# get_supported interfaces
#
# Called from the scd.xml.jinja tempate to lookup the list
# of supported interfaces for a device's sca_type value (CF/Device or
#  CF/AggregateDevice)
#
def get_supported_interfaces( interface ):
    retval = []
    if type(interface) == str:
        interface=[interface]

    for x in interface:
        retval += _resolve_supported_interfaces(x)

    return retval

#
# resolve_interface_hierarchy
#
# Called from get_interface_hierachy to resolve the interface
# hierarchy for a specified interface
#
# @param interface  - interface format string <module>/<interface class>  i.e. CF/Device
# @return a list of interface tuple ( interface, list of interface hierarchy )
#
def resolve_interface_hierarchy(interface):
    iflist=[]
    interface=format_interface_name(interface)
    idl = IDLInterface(interface)
    if interface and idl:
        inherited_names=None
        if len(idl.idl().inherited_names) > 0:
            inherited_names= [ "IDL:{}/{}:1.0".format(x[0],x[1]) for x in idl.idl().inherited_names ]
        iflist.append( ( (idl.repid(), idl.interface()), inherited_names) )
        if inherited_names:
            for parent_if in inherited_names:
                iflist+=resolve_interface_hierarchy(parent_if)
    return iflist

#
# get_interface_hierachy
#
# Called from the scd.xml.jinja template to return the list of interfaces
# for the implemented interface (i.e CF/Device) and any ports that is defined
# for the device.
#
# @param scd dictionary passed to the scd.xml.jinja template
# @return list of interface tuples that describe the interface hiearchies for a resource
#
def get_interface_hierarchy(scd):
    interfaces=[]
    if 'implemented_interface' in scd:
        _ifaces = get_supported_interfaces(scd['implemented_interface'])
        for iface in _ifaces:
            # only extract interface hierarchy if not in the current list
            if len([ iface for cface in interfaces if cface[0][0]  == iface[0] ]) == 0:
                interfaces+=resolve_interface_hierarchy(iface[0])

    ports=[]
    if 'input_ports' in scd:
        x=[ ports.append(x['itype']) for x in scd['input_ports'] if x['itype'] not in ports ]

    if 'output_ports' in scd:
        x=[ ports.append(x['itype']) for x in scd['output_ports'] if x['itype'] not in ports ]

    for input_interface in ports:
        interfaces+=resolve_interface_hierarchy(input_interface)

    return interfaces





###################################################################################
#
#   step1-create - Generate fei3_devices_lists.yml from template and command line arguments
#
######################################################################################

def fei3_create_types(args):

    fei_devices_file=args.output
    if args.output_dir:
        fei_devices_file=os.path.join(args.output_dir,args.output)

    env = Environment(loader = FileSystemLoader(_rh_template_paths), trim_blocks=True, lstrip_blocks=True)
    template = env.get_template('fei3_devices_list.yml.jinja')
    with open(fei_devices_file,'w+') as fei3_devices:
        fei3_devices.write(template.render())
    logger.info("Generated FEI devices list file: {}".format(fei_devices_file))


def fei3_devices_yaml(args):

    #
    # generate yaml file that with requested FEI types and class names
    #
    with open(args.devices_list) as init_yaml_file:

        fei_devices_file=args.output
        if args.output_dir:
            fei_devices_file=os.path.join(args.output_dir,args.output)

        init_yaml_file.seek(0)
        logger.debug("Device Generation List: \n{}".format(init_yaml_file.read()))
        init_yaml_file.seek(0)
        data=yaml.load(init_yaml_file)

        # reload the yaml data to expand out references as dictionaries or lists
        yaml.Dumper.ignore_aliases=lambda *args: True
        expanded_data=StringIO()
        yaml.dump(data, expanded_data)
        expanded_data.seek(0)
        data=yaml.load(expanded_data)

        if not data or not len(data):
            msg="Error creating initial yaml file check parameters"
            logger.error(msg)
            raise SystemExit(msg)

        # determine parent/child device build relationships
        _fei_spec_data={}
        _children=[]
        parent_type=None
        parent_class=None

        fei3_types = list(rh3_fei3_device_types.items())

        # Check if parent exists
        if 'PARENT' in data:
            if data['PARENT'] and len(data['PARENT']) > 0:
                parent_class=data['PARENT'][0]
                _fei_spec_data['PARENT']=parent_class

        # for each device type from devices list file
        for dtype, devices in list(data.items()):

            # skip type if no devices
            if not devices or 'PARENT' in dtype :
                continue

            # lookup type names in descriptions
            fei_type=None
            fei_template_type=None
            try:
                fei_type=re.match('.*\((.*)\)',dtype).groups()[0]
            except:
                raise SystemExit("Invalid FEI type {} from devices list file {} ".format(dtype,args.devices_list))

            if not fei_type in rh3_fei3_device_types:
                raise SystemExit("Invalid FEI type {} from devices list file {} ".format(dtype,args.devices_list))

            fei_template_type=rh3_fei3_device_types[fei_type]

            # check if parent device is in list...
            _devices=devices[:]
            if parent_class:
                if parent_class in devices:
                    parent_type = fei_template_type
                    # remove parent from devices
                    _devices=[ x for x in devices if x != data['PARENT']  ]
                # add in child list
                _children+=_devices

            # save back list of class to generate for the type
            _fei_spec_data[fei_type]=_devices[:]


        if len(_children) > 0:
            # if parent entry exists
            if not parent_type:
                parent_type='simple_parent'
                if 'PARENT' not in _fei_spec_data:
                    _fei_spec_data['PARENT']='SimpleDeviceParent'
            _fei_spec_data['parent_type']=parent_type
            _fei_spec_data['PARENT_CHILDREN']=_children[:]


        logger.debug("Context for jinja generations: \n{}".format(pprint.pformat(_fei_spec_data)))
        env = Environment(loader = FileSystemLoader(_rh_template_paths), trim_blocks=True, lstrip_blocks=True)
        template = env.get_template('fei3_devices.yml.jinja')
        with open(fei_devices_file,'w+') as fei3_devices:
            fei3_devices.write(template.render(fei3_spec=_fei_spec_data))
        logger.info("Generated FEI devices file: {}".format(fei_devices_file))


###################################################################################
#
#   step3-xml - Load in a fei3_device_descriptions.yml file an generate a
#               redhawk profile (spd,scd,prf) for each device
#
# The fei3_device_descriptions.yml file will contain a list of devices to generate. Those devices
# will described under the "DEVICES" node.
#
# Each Parent evice will have the following information:
# <Device Class Name>:
# codetype: Executable          - SPD code type Executable
# sca_type: AGGREGATE_PLAIN_DEVICE - SCA interface implemented AGGREGATE_PLAIN_DEVICE
# required_properties:          - preset values of device_kind and device_model
# properties:                   - additional properties to add that are not tuner status
# messages:                     - message property definitions
# data_output:                  - list of bulkio output ports
# data_inputs:                  - list of bulkio input ports
# message_inputs:               - list of message input port names
# message_outputs:              - list of message output port names
# dependencies:                 - list of softpackage dependencies
# children:                     - list of device children this aggregatge can create

#
# Each FEI3 child device will have the following information:
# <Device Class Name>:
# fei_type:                     - FEI device type RX, RDC, etc
# codetype: SharedLibrary       - SPD code type of SharedLibrary
# sca_type: DEVICE              - SCA interface implemented DEVICE or AGGREGATE_PLAIN_DEVICE
# children:                     - List of child devices to associate with the device
# scanning: false               - True/False if the device supports scanning
# required_properties:          - preset values of device_kind and device_model
# properties:                   - additional properties to add that are not tuner status
# tuner_status:                 - place holder when generating tuner status sequence
# additional_tuner_status:      - additional tuner status properties to add
# messages:                     - message property definitions
# data_output:                  - list of bulkio output ports
# data_inputs:                  - list of bulkio input ports
# control_inputs:               - list of Non-FEI control input ports to add
# control_outputs:              - list of Non-FEI control output ports to add
# rf_info_inputs:               - list of RFInfo input port names
# rf_info_outputs:              - list of RFInfo output port names
# gps_inputs:                   - list of GPS input port names
# gps_outputs:                  - list of GPS output port names
# nav_inputs:                   - list of Nav input port names
# nav_outputs:                  - list of Nav output port names
# message_inputs:               - list of message input port names
# message_outputs:              - list of message output port names
# dependencies:                 - list of softpackage dependencies
#
#
######################################################################################

def fei3_xml_gen(args):

    logger.info("Loading FEI specification file: {}".format(args.fei3_devices))
    fei3_devices=load_fei3_devices_specification(args)

    # check that the devices list is in the file
    if 'DEVICES' not in fei3_devices:
        msg = "Missing DEVICES list from FEI device specification file {}".format(args.fei3_devices)
        logger.error(msg)
        raise SystemExit(msg)

    # DEVICES:
    #  sequence of device definition mappings, where initial entry is parent device if multiple devices are
    #  defined.
    #   [ { dev1 : { device definition mapping" },
    #     { dev2 : { device definition mapping" },
    #   .. ]

    logger.debug("Generating DEVICES {} ".format(pprint.pformat(fei3_devices['DEVICES'])))

    # default first device as parent, for packing and namespace resolution
    parentName, parentDevice = list(fei3_devices['DEVICES'][0].items())[0]
    parentClassName='{}'.format( parentName.split('.')[-1])

    ns=None
    # determine namespace if used
    if args.enable_ns:

        # if namespace exists in classname in fei_devices.yml file then use that otherwise use class name
        ns="{}".format(''.join(parentName.split('.')[:-1]))
        if len(ns) == 0:
            ns=parentClassName
        logger.info("Generated namespace {}".format(ns))

        # if ns override specified then use that and ignore all namespaced class definitions
        if args.ns:
            ns=args.ns
            logger.info("Overriding namespace to {}".format(ns))

    # check if device namespace differs from on in the specified device name
    deviceNameSpace ='{}'.format(''.join(parentName.split('.')[:-1]))
    if args.enable_ns and len(deviceNameSpace) and ns != deviceNameSpace:
        logger.warning("Ignoring device specified namespace {}, differs from derived namespace {} ".format(deviceNameSpace,ns))

    parentProfile=None
    nest=None
    for dev in fei3_devices['DEVICES']:

        devClassName, device_definition = list(dev.items())[0]
        className='{}'.format(devClassName.split('.')[-1])

        namespace=ns
        if not parentProfile and 'children' in device_definition and device_definition['children']:
            device_definition['root_device']=True
            namespace=None

        # for each device create spd/scd/prf pojects and generate xml in output directory/device class name
        logger.debug("Generating FEI Device {} in namespace {} from device/definition {}/{} ".format(className,
                                                                                                     namespace,
                                                                                                     devClassName,
                                                                                                     device_definition))

        spd, scd, prf = generate_redhawk_profile( namespace, devClassName, className, device_definition,
                                                  fei3_devices['DEVICES'], fei3_devices, args )

        # determine output directory for profile
        opath=determine_profile_directory( namespace, className, args, nest, parent=(not parentProfile))

        # write out a redhawk profile
        profile=write_redhawk_profile( className, spd, scd, prf, opath, args)

        if not parentProfile:
            parentProfile=profile

        # check if we should nest children under parent
        if not args.unnest and not nest:
            nest=parentClassName

    # run codegen with parent spd file
    if args.codegen:
        args.overwrite=False
        args.spd_xml= os.path.join(parentProfile['spd'][1])
        fei3_codegen(args)
    return



########################################################################
#
# load_fei3_device_specification
#
# Load the fei3_device yaml file and create a devices dictionary
#
########################################################################
def load_fei3_devices_specification(args):

    logger.debug("Loading specifications file: {}\n".format(args.fei3_devices))

    macros=get_macros(args)

    with open(args.fei3_devices) as src:
        content=src.read()
        content=expand_macros(content,macros)
        with StringIO() as fei_spec_doc:
            fei_spec_doc.write(content)
            fei_spec_contents=fei_spec_doc.getvalue()


    """
    load devices specification file with expanded macros
    """
    try:
        logger.debug("Specification file: {}\n".format(fei_spec_contents))
        fei3_devices=yaml.load(fei_spec_contents)
	#
	# need to reload devices specification and expand anchors to remove references to each anchor
	#
        yaml.Dumper.ignore_aliases=lambda *args: True
        expanded_data=StringIO()
        yaml.dump(fei3_devices, expanded_data)
        expanded_data.seek(0)
        fei3_devices=yaml.load(expanded_data)

    except:
        msg="Failure processing FEI device specification file {}".format(args.fei3_devices)
        logger.error(msg)
        traceback.print_exc()
        raise SystemExit(msg)

    logger.info("Successfully loaded {} document".format(args.fei3_devices))
    logger.debug("FEI Specifications file:")
    logger.debug("{}".format(pprint.pprint(fei3_devices,indent=4)))
    return fei3_devices




########################################################################
#
# determine_profile_directory
#
# Determine profile directory to write redhawk profile
#
########################################################################
def determine_profile_directory( namespace, className, args, nest=None, parent=False):

    logger.debug("Deteremine output directory for profile definition {} with args.output_dir {} \n".format(className,
                                                                                                             args.output_dir))

    opath=''
    if namespace:
        opath=namespace
        logger.debug("Using namespaces for profiles {}, opath {}".format(namespace,opath))

    if nest:
        opath=os.path.join(opath,nest)
        logger.debug("Nesting profiles under {} opath {}".format(nest, opath))

    # check if we should put output in a specific directory
    # otherwise just put output in namespace directory
    if args.output_dir:
        opath=os.path.join(args.output_dir,opath)
        logger.debug("Output directory {} opath {}".format(args.output_dir, opath))

    if not parent:
        if args.childirs:
            opath=os.path.join(opath,className)
            logger.debug("Add child directory {} to opath {}".format(className, opath))

    else:
        # dump files into parent directory
        opath=os.path.join(opath,className)
        logger.debug("Parent profile {} to opath {}".format(className, opath))

    if opath and opath != '':
        try:
            if sys.version_info[0] == 3:
                os.makedirs(opath, exist_ok = True)
            else:
                os.makedirs(opath)
            logger.info("Create directory '{}'".format(opath))
        except OSError as error:
            if error.errno != 17:
                msg="Error creating directory '{}'.".format(opath)
                logger.error(msg)
                raise SystemExit(msg)

    return opath


########################################################################
#
# write_redhawk_profile
#
# Write out redhawk profile using appropriate jinja template
#
########################################################################
def write_redhawk_profile( className, spd, scd, prf, output_dir, args):

    logger.debug("Writing out profile definition for {} to output_dir {} \n".format(className,
                                                                                    output_dir))

    opath=output_dir
    if not output_dir:
        opath=''

    spd_name="{}.spd.xml".format(className)
    spd_path=os.path.join(opath,spd_name)
    logger.info("Device {} generating {} ({})".format(className, spd_name,opath))

    env = Environment(loader = FileSystemLoader(_rh_template_paths),  trim_blocks=True, lstrip_blocks=True)
    template = env.get_template('spd.xml.jinja')
    f=open(spd_path,'w+')
    f.write(template.render(spd=spd))

    scd_name="{}.scd.xml".format(className)
    scd_path=os.path.join(opath,scd_name)
    logger.info("Device {} generating {} ({})".format(className,scd_name,opath))

    template = env.get_template('scd.xml.jinja')
    f=open(scd_path,'w+')
    f.write(template.render(scd=scd))

    prf_name="{}.prf.xml".format(className)
    prf_path=os.path.join(opath,prf_name)
    logger.info("Device {} generating {} ({})".format(className,prf_name,opath))

    template = env.get_template('prf.xml.jinja')
    f=open(prf_path,'w+')
    f.write(template.render(properties=prf))

    return { 'spd': (spd_name, spd_path), 'scd' : (scd_name, scd_path), 'prf': (prf_name, prf_path) }


########################################################################
#
# generate_redhawk_profile
#
# Generate a redhawk xml profile for a FEI device
#
########################################################################
def generate_redhawk_profile(namespace, devClassName, className, device_definition,
                             devices_list, fei3_devices, args):

    source_file=args.fei3_devices

    logger.debug("Creating Profile definition for {} from \n{}\n".format(className,
                                                                         pprint.pformat(device_definition)))

    spd=None
    logger.debug("Creating SPD definition for {} in namespace {} ".format(className, namespace))
    spd=create_spd_definition( namespace, devClassName, className, args.lang, device_definition, devices_list, fei3_devices, args)
    logger.debug("Created SPD {} as {} ".format(className,spd))

    scd=None
    logger.debug("Creating SCD definition for {}".format(className))
    scd=create_scd_definition( devClassName, className, device_definition, fei3_devices )
    logger.debug("Created SCD {} as {} ".format(className,scd))

    prf=None
    logger.debug("Creating PRF definition for {}".format(className))
    prf=create_prf_definition( className, device_definition, fei3_devices, source_file)
    logger.debug("Created PRF {} as {} ".format(className,prf))

    return spd, scd, prf


def create_spd_definition( namespace, devClassName, className, lang, device_definition, devices_list, fei3_devices, args):
    macros=get_macros(args)

    #
    # Create SPD dictionary that will be used by the jinja template
    #
    # require keys:
    #     id   - uuid generated value
    #     ns   - namespace for device class derived from className or pass cmd line args
    #     name -  non-namespaced className
    #     version - fixed 3.0.0
    #     ns_path = format devClassname with / instead of .
    #     lang - language to generated
    #     dependencies - list of softpkg deps to use
    #     children - list of child device associated with this device
    #
    # sub-types
    # dependencies  - list of localfile paths to specific dep.spd.xml file
    #
    # children - list of dictionaries of { 'name' : <class name assigned to device'
    #                                      'tuner_type' : fei_type from fei_devices file }
    #
    spd={}.fromkeys( [ 'id', 'name', 'ns', 'version', 'ns_path', 'lang', 'dependencies', 'children' ] )

    logger.info("Building device {} is using namespace {}".format(className, namespace))

    spd['id']='{}'.format(uuid.uuid1())
    spd['ns']=namespace
    spd['name']=className
    spd['version']='{}'.format('3.0.0')
    spd['ns_path']=''
    if namespace:
        spd['ns_path']='{}'.format(spd['ns'].replace('.','/'))
    spd['lang']='{}'.format(lang)
    spd['impl_codetype']='Executable'
    spd['impl_ext']=''
    if spd['lang'] == 'python':
        spd['impl_ext']=".py"
    if 'codetype' in device_definition and device_definition['codetype']:
        spd['impl_codetype'] = device_definition['codetype']
        if spd['impl_codetype'] == 'SharedLibrary':
            if spd['lang'] == 'cpp':
                spd['impl_ext']=".so"

    #
    # Create dependency list for this device. Dependencies will fill in <localfile>path to spd.xml file</localfile>
    #  of SPD XML file.
    #
    #  The items in the dependencies sequence in the fei_devices.yml file and take the following forms:
    #     - <name spaced depenedency name>   e.g. rh.RedhawkDevUtils
    # or
    #    path to spd.xml in two forms
    #    - depname: %%deps%%/<path>/<to>/<dep>/<dep>.spd.xml  where %%deps%% will be expanded to $SDRROOT/dom/deps
    #    or
    #    - depname: /var/redhawk/sdr/dom/deps/<dep>.spd.xml
    #
    #
    spd['dependencies'] = None
    if 'dependencies' in device_definition and device_definition['dependencies']:
        _deps=[]
        for _dep in device_definition['dependencies']:
            dep=_dep
            if type(_dep) == dict:
                dep=next( ( v for k,v in _dep.items()),None)
            # if directory specification then just use as is
            if dep.startswith('/'):
                _depPath=dep
            else:
                _depName=dep.split('.')[-1]
                _depPath=os.path.join("%%deps%%", dep.replace('.','/'), "{}.spd.xml".format(_depName))
                _depPath=expand_macros(_depPath,macros)

            logger.info("Device {} has dependency {} ".format(devClassName, _depPath))
            _deps.append(_depPath)

        spd['dependencies'] = _deps

    #
    # Take the list of child device class names and create definition object of the following:
    #   name  - The class name of the device from the fei_devices.yml file
    #   tuner_type - the fei_type attribute for the device in the fei_devices.yml file
    #
    # the template will expand the child definitions to dependences in the SPD xml file as
    # and the local file as follows:
    #     "/devices/namespace/childDevice/childDevice.spd.xml"
    spd['children'] = None
    if 'children' in device_definition and device_definition['children']:
        _children=[]
        for chld in device_definition['children']:
            logger.debug("Adding child definition {} for device {}".format(chld, className))
            try:
                chld_idx = [ list(d.keys())[0] for d in devices_list ].index(chld)
            except:
                msg="Missing child definition {} for device {}".format(chld, devClassName)
                logger.error(msg)
                raise SystemExit(msg)

            child_def = devices_list[chld_idx][chld]
            _children.append( { 'tuner_type' : child_def['fei_type'],
                                'name' : chld })
        spd['children']=_children

    return spd

def create_scd_definition( devClassName, className, device, devices):

    #
    # Create SCD dictionary that will be used by the scd.xml.jinja template
    #
    # require keys:
    #     get_supported_interfaces   - callback to get list of support interfaces
    #     get_interface_hierachy     - callback to resolve interfaces hiearchy for each support interface/port
    #     input_ports                - list of port definitions
    #     output_ports               - list of port definitions
    #     implemented_interface      - Specific interface implemented e.g. CF/Device, CF/Resource
    #
    # sub dictionaries
    # port definitions:
    #     itype                       - interface type CF/Device, BULKIO/xxx
    #     type                        - sca defined port type control or data
    #     name                        - name assiged to port
    #

    # create dictionary keys used by scd.xml.jinja template
    scd={}.fromkeys( [ 'get_supported_interfaces',            # callback to get supported interfaces
                       'get_interface_hierarchy',             # callback to get interfaces list
                       'input_ports'                          # list of input port definitions
                       'output_ports'                         # list of output port definitions
                       'implemented_interface' ] )            # base interface implements by this device

    port_def={}.fromkeys( ['itype',                           # interface type e.g. CF/Device
                           'name',                            # name of the port
                           'type' ] )                         # port type, data, control, et

    #
    # add in required function calls used by jinja template that
    #  resolve interface names and hierarchies
    #
    scd['get_supported_interfaces' ] = get_supported_interfaces
    scd['get_interface_hierarchy' ] = get_interface_hierarchy

    if 'sca_type' in device:
        try:
            sca_type=device['sca_type']
            if 'children' in device and device['children'] and len(device['children']) > 0:
                if 'AGG' not in device['sca_type'].upper():
                    logger.warning("Promoting SCA type for device {} from {} to Aggregate_Plain_Device ".format(devClassName,
                                                                                           sca_type))
                    sca_type='Aggregate_Plain_Device'
            scd['implemented_interface'] = rh3_fei3_yaml_sca_types[sca_type.upper()]
        except:
            msg="Bad sca_type attribute {} for device {} from device specification file {}".format(
                device['sca_type'],devClassName,args.fei_devices)
            logger.error(msg)
            raise SystemExit(msg)
    else:
        msg="Missing sca_type attribute for device {} from device specification file {}".format(
            devClassName,args.fei_devices)
        logger.error(msg)
        raise SystemExit(msg)


    # process output ports
    uses_ports=[]

    # list of all possible output port types from the fei3_devices.yml file
    for output_port_type in [ 'control_outputs', 'data_outputs', 'gps_outputs', 'nav_outputs', 'message_outputs', \
                              'rf_info_outputs']:
        if output_port_type in device and device[output_port_type]:

            # for each port
            for port in device[output_port_type]:
                if type(port) == str:
                    pmap={ 'itype' : rh3_fei3_yaml_port_types[output_port_type.replace('_outputs','')],
                           'type' : 'data',
                           'name' : port }
                    logger.debug('Device {}/{} output port type {} has items {}'.format(devClassName,className,
                                                                                     output_port_type,
                                                                                     pmap))
                    uses_ports.append(pmap)
                else:
                    for pname, port_def in port.items():
                        if { 'itype', 'type' } < set(port_def.keys()):
                            logger.error("Device {}, Port {} missing itype or type".format(devClassName,pname))
                            continue
                            continue
                        uses_ports.append( { 'itype' : port_def['itype'],
                                             'type' : port_def['type'],
                                             'name' : pname } )

    scd['output_ports']=uses_ports

    # process input ports
    provides_ports=[]

    # list of all possible input port types from the fei3_devices.yml file
    for input_port_type in [ 'control_inputs', 'data_inputs', 'gps_inputs', 'nav_inputs', 'message_inputs', \
                              'rf_info_inputs']:
        if input_port_type in device and device[input_port_type]:

            # for each port
            for port in device[input_port_type]:
                if type(port) == str:
                    pmap={ 'itype' : rh3_fei3_yaml_port_types[input_port_type.replace('_inputs','')],
                           'type' : 'data',
                           'name' : port }
                    logger.debug('Device {}/{} input port type {} has items {}'.format(devClassName,className,
                                                                                    input_port_type,
                                                                                    pmap))
                    provides_ports.append(pmap)
                else:
                    logger.debug('Device {} input port type {} has items {}'.format(devClassName,
                                                                                    input_port_type,
                                                                                    port.items()))

                    for pname, port_def in port.items():
                        if { 'itype', 'type' } < set(port_def.keys()):
                            logger.error("Device {}, Port type missing for {}".format(devClassName,pname))
                            continue
                        provides_ports.append( { 'itype' : port_def['itype'],
                                                 'type' : port_def['type'],
                                                 'name' : pname } )

    scd['input_ports']=provides_ports
    return scd


def create_prf_definition( className, device, devices, source_file):

    #
    # Create PRF dictionary that will be used by the prf.xml.jinja template
    #
    # require keys:
    #     attrs                      - dictionary of attributes found in prf.dtd
    # optional keys:                 - same as prf.dtd elements for simple property
    #     description
    #     value                      - single value or list [] of values for sequences
    #     units
    #     enumeration                - list [] of enum dictionaries
    #     kind                       - list [] of kindtypes: allocation, property, message
    #     action                     - action for evaluating property: default external
    #
    # sub dictionaries
    # attrs
    #    id                          - optional will be namespace as needed or use name entry
    #    name                        - name of property
    #    mode                        - values readonly, writeonly, readwrite
    #    complex                     - if property can have complex values
    #    commandline                 - true/false if property should be passed on command line
    #    type                        - simple type: char, octet, short, long, longlong, ushort, ulong, ulonglong,
    #                                   double, float, utctime
    # enum
    #    label                       - label of value
    #    value                       - value reported for label

    prf=[]

    if 'properties' not in device or device['properties'] is None:
        device['properties']=[]

    fei_device_type=None
    # add required device properties
    if 'required_properties' in device  and device['required_properties']:
        req_props=[]
        #
        # account for differences property definition expansion from the yaml
        # property definitions can be a dictionary or list
        #
        if type(device['required_properties']) == dict:
            # property definitions are indexed as keys
            req_props = [ {k:v} for k,v in list(device['required_properties'].items()) ]
        else:
            req_props= device['required_properties']

        # transform device_kind to FRONTEND::<fei_type> or FRONTEND::PARENT::<fei_type>
        for prop in req_props:
            if 'device_kind' in prop:
                fei_device_kind="FRONTEND"
                if 'root_device' in device:
                    fei_device_kind="{}::PARENT".format(fei_device_kind)

                if 'fei_type' in device:
                    fei_device_kind="{}::{}".format(fei_device_kind,device['fei_type'])
                    fei_device_type=device['fei_type']

                prop['device_kind']['value']=fei_device_kind

        device['properties'] =  req_props + device['properties' ]

    if 'messages' in device and device['messages']:
        msg_attrs = devices.get('message_attrs')
        msgs = create_messages( className, device['messages'], msg_attrs, source_file )
        device['properties'] +=  msgs


    # check if frontend tuner
    if 'fei_type' in device and device['fei_type'] is not None:

        if 'scanning' in device and device['scanning']:
            device['properties'].append( {'frontend_scanner_allocation' : convert_properties_to_list(devices['fei_scanner_allocation']) } )

        if 'fei_listener_allocation' in devices:
            device['properties'].append( {'frontend_listener_allocation' : convert_properties_to_list(devices['fei_listener_allocation']) } )

        if 'fei_tuner_allocation' in devices:
            device['properties'].append( {'frontend_tuner_allocation' : convert_properties_to_list(devices['fei_tuner_allocation']) } )

        if 'fei_upstream_allocation' in devices:
            device['properties'].append( {'frontend_upstream_allocation' : convert_properties_to_list(devices['fei_upstream_allocation']) } )

        # add device type allocation properties
        if fei_device_type.upper() == 'SRDC':
            if 'fei_snapshot_allocation' in devices:
                device['properties'].append( {'frontend_snapshot_allocation' : convert_properties_to_list(devices['fei_snapshot_allocation']) } )

        if fei_device_type.upper() == 'DRDC':
            if 'fei_delay_allocation' in devices:
                device['properties'].append( {'frontend_delay_allocation' : convert_properties_to_list(devices['fei_delay_allocation']) } )

        # add transmitter allocation
        if fei_device_type.upper() in [ 'TX', 'TDC']:
            if 'fei_transmitter_allocation' in devices:
                device['properties'].append( {'frontend_transmitter_allocation' : convert_properties_to_list(devices['fei_transmitter_allocation']) } )

        # add array allocation, which is a simple so we don't need to convert
        if fei_device_type.upper() in [ 'TX_ARRAY', 'RX_ARRAY']:
            if 'fei_array_allocation' in devices:
                device['properties'] +=  devices['fei_array_allocation']

        # check for bulkio output ports
        if 'data_outputs' in device and device['data_outputs']:
            # only way to get the list comprehension to work..
            data_output_ports=copy.copy(device.get('data_outputs'))
            port_descs=[ list(y.values())[0] for y in data_output_ports ]
            typelist=[ x['itype'] for x in port_descs if 'itype' in x and 'BULKIO' in x['itype'] ]
            if len(typelist) > 0:
                if 'fei_stream_id_allocation' in devices:
                    device['properties'].append( {'frontend_stream_id_allocation' : convert_properties_to_list(devices['fei_stream_id_allocation']) } )

                if 'fei_data_format_allocation' in devices:
                    device['properties'].append( {'frontend_data_format_allocation' : convert_properties_to_list(devices['fei_data_format_allocation']) } )

                # check for vita49 and sdds port
                words=['SDDS', 'VITA49']
                if len([ s for s in typelist if any(a in s.upper() for a in words)]):
                    if 'fei_payload_format_allocation' in devices:
                        device['properties'].append( {'frontend_payload_format_allocation' : convert_properties_to_list(devices['fei_payload_format_allocation']) } )


        if 'fei_tuner_status_sequence' not in devices:
            msg="Missing 'fei_tuner_status_sequence' source file {}".format(source_file)
            logger.error(msg)
            raise SystemExit(msg)

        if 'tuner_status' not in device or 'fei_tuner_status' not in devices:
            msg="Device {} missing 'tuner_status' information from source file {} ".format(className, source_file)
            logger.error(msg)
            raise SystemExit(msg)

        fei_name='frontend_tuner_status'
        fei_ns='FRONTEND::tuner_status'
        frontend_tuner_status_seq = copy.deepcopy(devices['fei_tuner_status_sequence'])
        tuner_status_struct = next( ( prop for prop in frontend_tuner_status_seq if 'struct' in prop ), None)
        attrs = next( ( prop for prop in frontend_tuner_status_seq if 'attrs' in prop ), None)
        if attrs is None:
            fei_attrs=devices['structseq_attributes'].copy()
            fei_attrs['id'] = fei_ns
            fei_attrs['name'] = fei_name
            fei_attrs['mode'] = 'readonly'
            fei_attrs['kind'] = ['property']
            fei_attrs['ptype'] = ['structseq']
            frontend_tuner_status_seq = [ { 'attrs' : fei_attrs.copy() } ]+ frontend_tuner_status_seq

        # check if user overrode devices tuner status struct
        override_tuner_status_struct=convert_properties_to_list( device.get('tuner_status') )

        if tuner_status_struct is None and override_tuner_status_struct is None:
            msg="fei_tuner_status_sequence macro is missing 'struct' entry from file {} ".format(source_file)
            logger.error(msg)
            raise SystemExit(msg)

        if tuner_status_struct is None:
            tuner_status_struct = { 'struct' : override_tuner_status_struct }
            frontend_tuner_status_seq.append(tuner_status_struct)
        else:
            if fei_device_type and 'value' in tuner_status_struct['struct']['tuner_type']:
                tuner_status_struct['struct']['tuner_type']['value']= fei_device_type

            tuner_status_struct['struct'] = convert_properties_to_list(tuner_status_struct['struct'])

        # add additional properties to the tuner_status struct
        additional_tuner_status = device.get('additional_tuner_status')
        if additional_tuner_status:
            tuner_status_struct['struct'] += convert_properties_to_list( additional_tuner_status )

        apply_namespace_to_properties( fei_name, tuner_status_struct['struct'], className, ns_name=fei_ns )

        # add frontend tuner status into properties list
        device['properties'].append( { fei_name : frontend_tuner_status_seq } )


    #
    # generate property definitions to be used by prf.xml.jinja
    # properties are a list of dictionaries with prop_name : prop_definition
    for prop in device['properties']:

        # check to make sure each entry is a dictionary if not then report as error
        if type(prop) != dict:
            logger.error("Device {} property {} not defined correct in source file {}".format(className, prop, source_file))

        prop_name, prop_definition = list(prop.items())[0]

        # check if propery is simple or simpleseq
        if type(prop_definition) == dict:

            if 'attrs' not in prop_definition:
                msg="Device {} missing required 'attrs' for property {}".format(className, prop_name)
                logger.error(msg)
                raise SystemExit(msg)

            if 'ptype' not in prop_definition['attrs']:
                msg="Device {} missing required property attribute 'ptype' for property {}".format(className, prop_name)
                raise SystemExit(msg)

            ptype = prop_definition['attrs']['ptype']
            if ptype == 'simple' or ptype == 'simpleseq':
                #
                # create simple or simpleseq property definition for use by prf.xml.jinja template
                #
                prop_id, prop_def = create_simple_property(className, prop_name, prop_definition )
            else:
                msg="Device {}, property {} has incorrect ptype attribute {} should be simple or simpleseq ".format(className, prop_name, ptype)
                logger.error(msg)
                raise SystemExit(msg)


        else:

            attrs=next( ( prop for prop in prop_definition if 'attrs' in prop ), None)
            if not attrs:
                msg="Device {} property {} missing attributes".format(className, prop_name)
                logger.error(msg)
                raise SystemExit(msg)

            # check required attributes
            if not { 'id', 'name', 'ptype' } < set(attrs['attrs'].keys()):
                msg="Device {} property {} missing required attribute values {} ".format(className, prop_name, attrs['attrs'])
                logger.error(msg)
                raise SystemExit(msg)

            ptype=attrs['attrs']['ptype']
            if ptype == 'struct':
                prop_id, prop_def = create_struct_property(className, prop_name, prop_definition )

            if ptype == 'structseq':
                prop_id, prop_def = create_structseq_property(className, prop_name, prop_definition )

        prf.append({ prop_id:  prop_def})

    logger.debug("Device {}, generated PRF content {} ".format(className, pprint.pformat(prf)))
    return prf

def create_messages( className, messages, msg_attrs, source_file):
    msgs = []

    if type(messages) != list:
        msg="Device {} -messages is not a list of messages, source file {}".format(className,source_file)
        logger.error(msg)
        raise SystemExit(msg)

    if not msg_attrs:
        msg_attrs=devices['struct_attributes'].copy()
        msg_attrs['mode'] = 'readonly'
        msg_attrs['kind'] = ['message']
        msg_attrs['ptype'] = 'struct'

    # messagees are a  list of dictionaires with single key == message name
    for msg_struct in messages:

        msg_name, msg_props = list(msg_struct.items())[0]

        msg_name, msg_props = create_message_properties( className, msg_name, msg_props, msg_attrs)

        msgs.append( { msg_name : msg_props } )

    return msgs


def create_message_properties( className, msg_name, msg_props, msg_attrs):

    msg_props = convert_properties_to_list(msg_props)

    attrs = next( ( prop for prop in msg_props if 'attrs' in prop ), None)
    if attrs is None:
        msg_props=[ { 'attrs' : msg_attrs.copy() } ] + msg_props

    # apply msg_name as namespace
    apply_namespace_to_properties( msg_name, msg_props, className, ns_name=msg_name )

    return msg_name, msg_props

def convert_properties_to_list( properties ) :

    # if properties is in dictionary format reformat to list of individual dictionaries
    if type(properties) == dict:
        props=[ {k:v} for k,v in properties.items()  ]
    else:
        props=properties

    if props:
        attrs=next( ( prop for prop in props if 'attrs' in prop ), None)
        if attrs:
            props=[ attrs ] + [ prop for prop in props if attrs != prop  ]

    return props

#
# apply_namespace_to_properties
#
# for a list of property definition diectionaries [ 'attrs', { 'prop1', { .. } }, ..]
#
# Apply one of the following as the namespace:
# if ns_name is provided then use that name
# else
#    lookup name in 'attrs' dictionary and use that at the namespace name for the propertie in the list
#

def apply_namespace_to_properties( prop_name, properties, className, ns_name=None ) :

    logger.debug("Device {} property {} ns_name(param)= {} properties set {}".format(className, prop_name, ns_name, properties))

    resolve_ns=False
    if not ns_name or ns_name == '':
        # try and determine namespace from prop_name, name attribute or id attribute
        resolve_ns=True

    # if properties is in dictionary format reformat to list of individual dictionaries
    if type(properties) == dict:
        props=[ {k:v} for k,v in properties.items()  ]
    else:
        props =  properties

    # grab attributes if they exists to try and determine if we need to use provide attribute id or name
    attrs=next( ( prop for prop in props if 'attrs' in prop ), None)
    if attrs is None:
        msg="Device {} property {} is missing 'attrs' from property definition".format(className, prop_name)
        logger.error(msg)
        raise SystemExit(msg)

    attrs=attrs['attrs']
    if not {'id', 'name'} < set(attrs.keys()):
        msg="Device {} property {} missing required attributes {}".format(className, prop_name, attrs)
        logger.error(msg)
        raise SystemExit(msg)

    # try and determine what should be the namespace, priority is given to ns_name parameter
    pname=attrs['name']
    _id=attrs['id']

    if not pname:
        logger.debug("Device {} property {} assigning 'name' attribute {} ".format(className,prop_name,prop_name))
        pname = prop_name

    if pname and prop_name != pname:
        logger.warning("Device {} property {} has 'name' attribute mismatch {} != {} ".format(className,prop_name,prop_name,pname))

    if resolve_ns:
        ns_name=prop_name

        # if no id attribute then set to prop_name
        if not _id:
            _id = prop_name

        # id is set so use that as namespace
        ns_name = _id

    else:
        if _id != ns_name:
            logger.warning("Device {} property {} with 'id' attribute {}  caller requests namespace {} ".format(className,prop_name,_id, ns_name))
        else:
            _id = prop_name

    logger.info("Device {} property {} setting member's properties namespace to {} ".format(className,prop_name,ns_name))

    attrs['id']=_id
    attrs['name']=pname

    for prop in props:
        k,v = list(prop.items())[0]

        # check if attrs is configured
        if k == 'attrs':
            continue

        pname=k
        if not v or 'attrs' not in v:
            msg="Device {}, property {} has  member property {} with no attributes".format(className,prop_name, k)
            logger.error(msg)
            raise SystemExit(msg)

        attrs=v['attrs']
        if  not {'id', 'name' } < set(attrs.keys()):
            msg="Device {} property {} member property {} missing required attributes (id,name)".format(className, prop_name, k)
            logger.error(msg)
            raise SystemExit(msg)

        if attrs['name']:
            logger.debug("Device {} property {}  member property {} provides 'name' attribute: {} ".format(className,prop_name, k, attrs['name']))
        else:
            attrs['name'] = pname
            logger.debug("Device {} property {}  member property {} setting 'name' attribute: {} ".format(className,prop_name, k, pname ))

        # figure out namespace for id attribute
        _id ="{}::{}".format(ns_name,pname)

        if attrs['id']:
            logger.debug("Device {} property {}  member property {} provides 'id' attribute: {} ".format(className,prop_name,k,attrs['id']))
        else:
            attrs['id']=_id
            logger.debug("Device {} property {}  member property {} setting 'id' attribute: {} ".format(className,prop_name,k,attrs['id']))



def create_struct_property(className, prop_name, properties, ns_override=None ):

    # create a struct property definition as follows
    # { 'attrs' :  { struct attributes dictionary},  <- always first entry
    # { 'prop1' :  simple prop dictionary },
    # { 'prop2' :  simple prop dictionary },
    #   etc...
    #
    if type(properties) == dict:
        _props=[ {k:v} for k,v in properties.items()  ]
    else:
        _props=properties

    # apply namespace to structure id values
    apply_namespace_to_properties( prop_name, _props, className, ns_name=ns_override)

    # for each attribute convert property definition to entries for jinja template
    props=[]
    for prop in _props:

        if 'attrs' in prop:
            props.append(prop)
            continue

        # structs only support simple or simpleseq object, each prop is in the form { 'prop_name' : { prop_definition } }
        pname, prop_def = next(iter(prop.items()))
        _id, new_prop_def=create_simple_property(className, pname, prop_def)
        props.append( { _id : new_prop_def } )

    # grab attributes if they exists to try and determine if we need to use provide attribute id or name
    attrs=next( ( prop for prop in props if 'attrs' in prop ), None)
    prop_id = attrs['attrs']['id']
    return prop_id, props

#
#  create_structseq_property
#
#  Create a struct sequency property definition as follow
#  property keys:
#   'attrs' - (dict) property attribute name/values
#   'struct': ([ list of simple properties (see create_simple_property)
#
# As per redhawk specification, all members of the struct will be namespaced with the name of the
# the struct sequence. The struct itself will be namespaced if one is not provided.  If the struct
# is not named, then the default value of <prop_name>_struct will be assiged.
#

def create_structseq_property(className, prop_name, properties ):

    # create a struct property definition in the form of
    # { 'attrs' :  { attributes dictionary}
    #   'struct' : [ list of simple properties ]
    props=convert_properties_to_list(properties)

    # grab attributes if they exists to try and determine if we need to use provide attribute id or name
    attrs=next( ( prop for prop in props if 'attrs' in prop ), None)
    if not attrs:
        msg="Device {} property {} missing attributes".format(className, prop_name)
        logger.error(msg)
        raise SystemExit(msg)

    struct=next( ( prop for prop in props if 'struct' in prop ), None)
    if not struct:
        msg="Device {} property {} missing struct definition".format(className, prop_name)
        logger.error(msg)
        raise SystemExit(msg)
        
    # default id and name to prop_prop name if no overrides exist
    if 'id' in attrs['attrs'] and not attrs['attrs']['id']:
        attrs['attrs']['id']=prop_name
    if 'name' in attrs['attrs'] and not attrs['attrs']['name']:
        attrs['attrs']['name']=prop_name

    # converts a dictionary of property definitions i.e. { 'prop1': { 'attrs', ... }, 'prop2' ..}
    # to a list of individual dictionaries [ { prop1: { 'attrs', ..} , { 'prop2' : { ...}]
    # will also reorder list so 'attrs' is first and props follow
    struct_props=convert_properties_to_list(struct['struct'])
    struct_attrs=next( ( prop for prop in struct_props if 'attrs' in prop ), None)

    # need to check if user supplied id/name for the structure inside the sequence
    _set_id=False
    _set_name=False
    if struct_attrs:
        if 'id' in struct_attrs['attrs'] and not struct_attrs['attrs']['id']:
            _set_id=True
        if 'name' in struct_attrs['attrs'] and not struct_attrs['attrs']['name']:
            _set_name=True

    # convert member properties to proper format for jinja template
    _id, struct_props=create_struct_property(className, prop_name, struct_props )
    struct['struct']=struct_props

    # Set id and name of the structure it none was provided.  
    #    note - create_struct_property will set id/name using default rules we need to reset 
    #           set this 
    # 
    struct_attrs=next( ( prop for prop in struct_props if 'attrs' in prop ), None)
    if _set_name:
        struct_attrs['attrs']['name'] = "{}_struct".format(prop_name)
    _name= struct_attrs['attrs']['name']
    if _set_id:
        struct_attrs['attrs']['id'] = "{}::{}".format(prop_name, _name)

    prop_id = attrs['attrs']['id']
    return prop_id, props

#
#  create_simple_property
#
#  Translate a simple property definition to a dictionary for prf.xml file to interpret
#  property keys:
#   'attrs' - (dict) property attribute name/values
#   'description': (str) description text
#   'value' : (str) represents value
#   'units' : (str) units
#   'range' :  (str) range value
#   'enumeration' - ([ { 'name': 'n', 'value' : 'v' } ... ]
#   'kind' : ([ str, ])
#   'actions' : (str)
#
#  Attributes is required, all other fields are deemed option and will not be include
#  in the xml if entry is None or missing.
#
#  Minimum required attributes are: id, name, and type (aka ptype : int, short, etc)
#
#
def create_simple_property( className, property_name, property_definition ):

    # make copy
    prop=property_definition.copy()

    # resolve property id and property name
    prop_id=None
    if 'attrs' not in property_definition:
        msg='Invalid property definition, Device {} property {} missing attributes dictionary'.format(className,property_name)
        logger.error(msg)
        raise SystemExit(msg)

    # get attrs dictionary
    attrs=property_definition['attrs']

    if not { 'id', 'name', 'ptype' } < set(attrs.keys()):
        msg='Invalid property definition, Device {} property {} missing required attributes, {} '.format(className,property_name, attrs)
        logger.error(msg)
        raise SystemExit(msg)

    # if attributes missing name then default to property_name
    if not attrs['name']:
        attrs['name'] = property_name

    if attrs['name'] and property_name != attrs['name']:
        logger.warning('Device {} property attribute <name> mismatch name {} != {} name-attribute'.format(className,property_name,attrs['name']))

    if attrs['id'] is None:
        attrs['id']=attrs['name']

    #
    # convert enumerations from [ label_value, label_value, label_value .. ] to  {'label': label_value, 'value' : value_value } where value_value is derived
    #  from the property type ie. string type will default value to label string, numeric types will be original value starting at 0
    # OR
    #  convert enumerations from [ { label_value : value_value } , { label_value : value_value } , ] to  {'label': label_value, 'value' : value_value } 
    #
    enums=None
    _ord=0
    if 'enumeration' in property_definition and property_definition['enumeration']:
        enums=[]
        for e in property_definition['enumeration']:
            enum=copy.copy(e)
            if type(e) != dict:
                # convert singletons to tuple for dictionary processing
                enum= {e,e}
                if attrs['type']!='string':
                    # if default non-string types to indexed value starting at 0
                    enum={e,_ord}
                    _ord +=1
            _label, _value = next(iter(enum.items()))
            enums.append( { 'label' : _label, 'value' : _value })
        prop['enumeration']=enums[:]

    logger.debug("Device {}, Creating name/type {}/{} id/name {}/{} with property definition {}".format(className,
                                                                                                      property_name,
                                                                                                      attrs['ptype'],
                                                                                                      attrs['id'],
                                                                                                      attrs['name'],
                                                                                                      prop))
    return attrs['id'], prop




###################################################################################
#
#   step4-code- Run code generator using parent spd.xml as input. Code generator
#               should run in context of directory with parent spd.xml file.x
#
###################################################################################

def fei3_codegen(args):
    # create command string to run redhawk code generator
    cwd=os.getcwd()
    ovr=''
    if args.overwrite:
        ovr='-f'

    outdir=''
    if args.output_dir:
        outdir='--output-dir {}'.format(args.output_dir)
    else:
        outdir=os.path.dirname(args.spd_xml)

    spd_xml=os.path.basename(args.spd_xml)

    logger.info("Changing directory {}".format(outdir))
    if outdir != '':
       os.chdir(outdir)
    try:
        cmd="redhawk-codegen --frontend {} {}".format(ovr,spd_xml)
        logger.debug("Running codegen as <{}>".format(cmd))
        if sys.version_info[0] == 3:
            process=subprocess.run(cmd.split())
            print("process {}".format(process))
            if process.returncode != 0:
                logger.error("Codegen returned the following: {}".format(process.stderr))
        else:
            process=subprocess.Popen(cmd.split())
            sdata=process.communicate()[0]
            print("process {}".format(sdata))
            if process.returncode != 0:
                logger.error("Codegen returned the following: {}".format(process.stderr))

    except:
        traceback.print_exc()

    os.chdir(cwd)



#################################
# Update repo configuration context
#################################

def process_command_line():
    tw =  TextWrapper(initial_indent="\t", subsequent_indent="\t\t")
    tw2 = TextWrapper(initial_indent="\t", subsequent_indent="\t\t")

    # Set up usage, description, and other --help information
    usage = "usage: %(prog)s [--help] [options]"

    description = "Generate FEI3 device templates, XML and code generation \n\n"
    description += "Perform the following sequence to generate code for a FEI3 device:\n"
    description += "   step1: create-list                - creates list of all FEI devices types to define your device/hierarchy\n"
    description += "   step2: devices                    - generate device specifiction (YAML) for each type in the device types list file\n"
    description += "   step3: xml                        - generate a language specific REDHAWK XML profile for each device specification\n"
    description += "   step4: code                       - generate the language specific implementation from the REDHAWK XML profile\n"
    description += "\nEach command has optional arguments and help: fei3-generator xml --help"


    parser = ArgumentParser(usage=usage,
                            description=description,
                            formatter_class=RawDescriptionHelpFormatter)

    # Add the flags
    parser.add_argument('--log',
                        help="log output to file")

    parser.add_argument('--output-dir',
                        help="output directory",
                        required=False,
                        default=None )

    parser.add_argument( '--debug',
                        dest="debug",
                        default='info',
                        choices=['critical','error', 'warn', 'info', 'debug'],
                         help="Specify logging level for output messaging")

    subparsers = parser.add_subparsers(dest='cmd')
    subparsers.required=True


    #
    # Create DeviceTypes Class Template
    #
    fei3_create_types_description = "\tCreate FEI3 Device Classes List File "
    fei3_create_types_epilog = BOLD + "Examples:" + RESET
    fei3_create_types_epilog += tw2.fill(" \n ")
    fei3_create_types_epilog += "\n\tfei3-generator create-list "
    fei3_create_types_epilog += "\n"
    fei3_create_types_epilog += "\n\tAdd device classes as need for each type: - <device class>"
    fei3_create_types_epilog += "\n\tIf a single class is listed then a simple FEI3 device is created"
    fei3_create_types_epilog += "\n\tIf multiple classes are listed, then a Parent/Child FEI device will be created"
    fei3_create_types_epilog += "\n\t"
    fei3_create_types_epilog += """
\t...
\tPARENT:
\t
\tAnalog Input Receiver: RX
\t    - RCV
\t
\tAnalog Input Receiver Array: RX_ARRAY
\t...
\tDigital Receive Channel (RDC):
\t     - WidebandDDC
"""

    fei3_create_types_parser = subparsers.add_parser('create-list',
                                      description=fei3_create_types_description,
                                      epilog=fei3_create_types_epilog,
                                      formatter_class=RawDescriptionHelpFormatter)

    fei3_create_types_parser.add_argument('--output',
                                          help="Save as FEI device types list file (default: %(default)s)",
                                          required=False,
                                          default="fei3_devices_list.yml")

    fei3_create_types_parser.set_defaults(func=fei3_create_types)

    #
    # Create initial devices yaml specification
    #
    fei3_devices_yaml_description = "\tCreate FEI3 Devices Specification File "
    fei3_devices_yaml_epilog = BOLD + "Examples:" + RESET
    fei3_devices_yaml_epilog += tw2.fill(" \n ")
    fei3_devices_yaml_epilog += "\n\tCreate FEI3 devices specification file from a device types list file"
    fei3_devices_yaml_epilog += "\n\tfei3-generator devices "
    fei3_devices_yaml_epilog += "\n\tGenerates an output YAML file that contains a basic definition for each device type. It is expected each "
    fei3_devices_yaml_epilog += "\n\tdevice will be customized with ports, properties, and messages to meet the desired design"
    fei3_devices_yaml_epilog += "\n\t"
    fei3_devices_yaml_epilog += """
\t...
\tDeviceParent:
\t   <<: *simple_parent
\t   properties:
\t   ports:
\t   children:
\t       - DigitalWideband
\t       - DigitalNarrowband
\t
\tDigitalWideband:
\t   <<: *rdc_tuner
\t
\tDigitalNarrowband:
\t   <<: *rdc_tuner
...
"""

    fei3_devices_yaml_parser = subparsers.add_parser('devices',
                                      description=fei3_devices_yaml_description,
                                      epilog=fei3_devices_yaml_epilog,
                                      formatter_class=RawDescriptionHelpFormatter)

    fei3_devices_yaml_parser.add_argument('--devices-list',
                                help="Save as FEI device list",
                                default="fei3_devices_list.yml")

    fei3_devices_yaml_parser.add_argument('--output',
                                help="Save as FEI devices file",
                                default="fei3_devices.yml")

    fei3_devices_yaml_parser.set_defaults(func=fei3_devices_yaml)



    #
    # Create XML Files
    #
    fei3_xml_gen_description = "\tCreate FEI3 Device XML definitions "
    fei3_xml_gen_epilog = BOLD + "Examples:" + RESET
    fei3_xml_gen_epilog += tw2.fill(" \n ")
    fei3_xml_gen_epilog += "\n\tCreate REDHAWK FEI3 Device XML files: "
    fei3_xml_gen_epilog += "\n\tfei3-generator xml "
    fei3_xml_gen_parser = subparsers.add_parser('xml',
                                      description=fei3_xml_gen_description,
                                      epilog=fei3_xml_gen_epilog,
                                      formatter_class=RawDescriptionHelpFormatter)

    fei3_xml_gen_parser.add_argument('--fei3-devices',
                                     metavar="<file>.yml",
                                     help="Source FEI3 Devices specification file (default:%(default)s)",
                                     default="fei3_devices.yml")

    fei3_xml_gen_parser.add_argument('--ns',
                                     type=str,
                                     help="Set namespace for devices (overrides parent default)",
                                     default=None)

    fei3_xml_gen_parser.add_argument('--enable-ns',
                                     help="Enable namespace from parent device name",
                                     action="store_true",
                                     default=False)

    fei3_xml_gen_parser.add_argument('--unnest',
                                     help="Create child profiles in non-namespaced parent directory",
                                     action="store_true",
                                     default=False)

    fei3_xml_gen_parser.add_argument('--childirs',
                                     help="Create child profiles in their own directory",
                                     action="store_true",
                                     default=False)

    fei3_xml_gen_parser.add_argument('--lang',
                                     default="cpp",
                                     choices=['cpp','python', 'java'],
                                     help="Specify implementation language")

    fei3_xml_gen_parser.add_argument('--sdrroot',
                                     default="/var/redhawk/sdr",
                                     help="Specify path for SDRROOT")

    fei3_xml_gen_parser.add_argument('--ossiehome',
                                     default="/usr/local/redhawk",
                                     help="Specify path for OSSIEHOME")

    fei3_xml_gen_parser.add_argument('--codegen',
                                     help="Run codegen on generated xml files.",
                                     action="store_true",
                                     default=False)

    fei3_xml_gen_parser.set_defaults(func=fei3_xml_gen)


    #
    # Run codegen
    #
    fei3_codegen_description = "\tRun REDHAWK code generators for Frontend devices "
    fei3_codegen_epilog = BOLD + "Examples:" + RESET
    fei3_codegen_epilog += tw2.fill(" \n ")
    fei3_codegen_epilog += "\n\tCreate code files for REDHAWK FEI3 devices: "
    fei3_codegen_epilog += "\n\tfei3-generator code --spd-xml <path to spd.xml file>"
    fei3_codegen_parser = subparsers.add_parser('code',
                                      description=fei3_codegen_description,
                                      epilog=fei3_codegen_epilog,
                                      formatter_class=RawDescriptionHelpFormatter)

    fei3_codegen_parser.add_argument('--spd-xml',
                                     help="SPD xml file to use to generate code",
                                     default=None,
                                     required=True)

    fei3_codegen_parser.add_argument('--overwrite',
                                     help="Force codegen to over write files",
                                     action="store_true",
                                     default=False)

    fei3_codegen_parser.set_defaults(func=fei3_codegen)




    return parser.parse_known_args()

if __name__ == "__main__":

    # process command line
    args, remaining_args=process_command_line()
    print(args)

    configureLogging(args.debug, args.log)

    if args.output_dir :
        if not os.path.exists(args.output_dir):
                os.makedirs(args.output_dir)
        logger.info("Using  output directory {}".format(args.output_dir))

    # perform the selected menu function
    args.func(args)
