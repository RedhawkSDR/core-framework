#!/usr/bin/env python
#
# AUTO-GENERATED
#
# Source: S2_pre.spd.xml

import sys, signal, copy, os
import logging

from ossie.cf import CF, CF__POA #@UnusedImport
from ossie.service import start_service
from omniORB import any, CORBA, URI, PortableServer

from ossie.cf import CF
from ossie.cf import CF__POA
from ossie import properties;


class S2_pre(CF__POA.PropertySet):

    def __init__(self, name="S2_pre", execparams={}):
        self.name = name
        self._log = logging.getLogger(self.name)
        self._props = properties.PropertyStorage(self, (), execparams)
        try:
            self._props._addProperty( S2_pre.p1 )
            self._props._addProperty( S2_pre.p2)
        except KeyError, e:
            pass
        except Exceptiopn, e:
            raise e
        self._props.initialize()

    def terminateService(self):
        pass

    def configure(self, configProperties):
        notSet = []
        error_message = ''
        for prop in configProperties:
            try:
                if self._props.has_id(prop.id) and self._props.isConfigurable(prop.id):
                    try:
                        self._props.configure(prop.id, prop.value)
                    except Exception, e:
                        self._log.warning("Invalid value provided to configure for property %s: %s", prop.id, e)
                        notSet.append(prop)
                else:
                    self._log.warning("Tried to configure non-existent, readonly, or property with action not equal to external %s", prop.id)
                    notSet.append(prop)
            except Exception, e:
                error_message += str(e)
                self._log.exception("Unexpected exception.")
                notSet.append(prop)

        if len(notSet) > 0 and len(notSet) < len(configProperties):
            self._log.warning("Configure failed with partial configuration, %s", notSet)
            raise CF.PropertySet.PartialConfiguration(notSet)
        elif len(notSet) > 0 and len(notSet) >= len(configProperties):
            self._log.warning("Configure failed with invalid configuration, %s", notSet)
            raise CF.PropertySet.InvalidConfiguration("Failure: "+error_message, notSet)
        self._log.trace("configure(%s)", configProperties)


    def query(self, configProperties):
        if configProperties == []:
            self._log.trace("query all properties")
            try:
                rv = []
                for propid in self._props.keys():
                    if self._props.has_id(propid) and self._props.isQueryable(propid):
                        try:
                            value = self._props.query(propid)
                        except Exception, e:
                            self._log.error('Failed to query %s: %s', propid, e)
                            value = any.to_any(None)
                        prp = self._props.getPropDef(propid)
                        if type(prp) == properties.struct_property:
                            newvalval = []
                            for v in value.value():
                                if prp.fields[v.id][1].optional == True:
                                    if isinstance(v.value.value(), list):
                                        if v.value.value() != []:
                                            newvalval.append(v)
                                    else:
                                        if v.value.value() != None:
                                            newvalval.append(v)
                                else:
                                    newvalval.append(v)
                            value = CORBA.Any(value.typecode(), newvalval)

                        rv.append(CF.DataType(propid, value))
            except:
                raise

        # otherwise get only the requested ones
        else:
            self._log.trace("query %s properties", len(configProperties))
            try:
                unknownProperties = []
                for prop in configProperties:
                    if self._props.has_id(prop.id) and self._props.isQueryable(prop.id):
                        try:
                            prop.value = self._props.query(prop.id)
                        except Exception, e:
                            self._log.error('Failed to query %s: %s', prop.id, e)
                        prp = self._props.getPropDef(prop.id)
                        if type(prp) == properties.struct_property:
                            newvalval = []
                            for v in prop.value.value():
                                if prp.fields[v.id][1].optional == True:
                                    if isinstance(v.value.value(), list):
                                        if v.value.value() != []:
                                            newvalval.append(v)
                                    else:
                                        if v.value.value() != None:
                                            newvalval.append(v)
                                else:
                                    newvalval.append(v)
                            prop.value = CORBA.Any(prop.value.typecode(), newvalval)
                    else:
                        self._log.warning("property %s cannot be queried.  valid Id: %s",
                                        prop.id, self._props.has_id(prop.id))
                        unknownProperties.append(prop)
            except:
                raise

            if len(unknownProperties) > 0:
                self._log.warning("query called with invalid properties %s", unknownProperties)
                raise CF.UnknownProperties(unknownProperties)

            rv = configProperties
        self._log.trace("query -> %s properties", len(rv))
        return rv

    def initializeProperties(self, ctorProps):
        notSet = []
        for prop in ctorProps:
            try:
                if self._props.has_id(prop.id) and self._props.isProperty(prop.id):
                    try:
                        # run configure on property.. disable callback feature
                        self._props.construct(prop.id, prop.value)
                    except ValueError, e:
                        self._log.warning("Invalid value provided to construct for property %s %s", prop.id, e)
                        notSet.append(prop)
                else:
                    self._log.warning("Tried to construct non-existent, readonly, or property with action not equal to external %s", prop.id)
                    notSet.append(prop)
            except Exception, e:
                self._log.exception("Unexpected exception.")
                notSet.append(prop)

    def registerPropertyListener(self, obj, prop_ids, interval):
        # TODO
        pass

    def unregisterPropertyListener(self, id):
        # TODO
        pass

    p1 = properties.simple_property(id_="p1",
                             name="p1",
                             type_="string",
                             mode="readwrite",
                             action="external",
                             kinds=("configure",),
                             description=""" """)


    p2 = properties.simple_property(id_="p2",
                         name="p2",
                             type_="long",
                             mode="readwrite",
                             action="external",
                             kinds=("configure",))


if __name__ == '__main__':
    start_service(S2_pre, thread_policy=PortableServer.SINGLE_THREAD_MODEL)
