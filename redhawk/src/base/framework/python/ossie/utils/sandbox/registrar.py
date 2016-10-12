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

import logging
import threading

from ossie.cf import CF, CF__POA
import CosNaming

from omniORB import CORBA, URI
from ossie.utils import log4py

log = logging.getLogger(__name__)

class ApplicationStub(CF__POA.Application):
    def __init__(self, id, name):
        self._id = id
        self._name = name

    def _get_identifier(self):
        return self._id
    
    def _get_name(self):
        return self._name
    
class ApplicationRegistrarStub(CF__POA.ApplicationRegistrar):
    def __init__(self, app_id, app_name):
        self.__context = {}
        self.__lock = threading.RLock()
        self._app = ApplicationStub(app_id, app_name)
        
    def _get_app(self):
        return self._app._this()
    
    def _get_domMgr(self):
        return None
    
    def registerComponent(self, Name, obj):
        pass
    
    def bind(self, name, object):
        uri = URI.nameToString(name)
        self.__lock.acquire()
        try:
            if uri in self.__context:
                raise CosNaming.NamingContext.AlreadyBound()
            log.debug('Binding "%s" into virtual NamingContext', uri)
            self.__context[uri] = object
        finally:
            self.__lock.release()

    def rebind(self, name, object):
        uri = URI.nameToString(name)
        log.debug('Rebinding "%s" into virtual NamingContext', uri)
        self.__lock.acquire()
        try:
            self.__context[uri] = object
        finally:
            self.__lock.release()

    def getObject(self, name):
        self.__lock.acquire()
        try:
            return self.__context.get(name, None)
        finally:
            self.__lock.release()

    def to_name(self, name):
        """
        Returns a list of CosNaming.NameComponent from the incoming string.
        
        The name is a string used to generate NameComponent objects.  The 
        NameComponent object takes 2 optional arguments: id and kind.  The 
        string can have one or more names as follow:
        
            id1.kind1/id2.kind2/id3./.kind4
        
        Where the first two objects have the id and the kind.  The third object
        does not have a kind defined, and the fourth object does not have an 
        id.  Both, the id and the kind, can use a '\' as a escape character to
        include either the '.' or the '/' as part of them.  For instance the 
        following pair is valid:
        
            my\.id.my\/kind ==> ("my.id" "my/kind")
        
        If an error is found while parsing the string, then a ValueError is 
        raised and an empty list is returned.
        
        """
        names = []
        # Before splitting the name, replace all the escapes
        data = {'DOT_TOKEN':'\.', 'SLASH_TOKEN':'\/'}

        try:
            # if a escape dot is found replace it with the DOT_TOKEN
            name = name.replace('\.', '%(DOT_TOKEN)s')
            # if a escape slash is found replace it with the SLASH_TOKEN
            name = name.replace('\/', '%(SLASH_TOKEN)s')

            # now that we are not 'escaping' we can split the name into what 
            # will become NameComponent objects
            items = name.split('/')
            # for evert NameComponent, get the id and the kind
            for item in items:
                # Separate the id and the kind
                end = item.find('.')
                if end >= 0:
                    id_ = item[0:end]
                    kind_ = item[end + 1:]
                    # replace the DOT_TOKEN and the SLASH_TOKEN back 
                    id_ = id_ % data
                    kind_ = kind_ % data
                else:
                    id_ = item
                    kind_ = ''
                # create the NameComponent object and append it to the list
                names.append(CosNaming.NameComponent(id_, kind_))

            return names
        except:
            raise ValueError("NamingContextStub:to_name() '%s' is an invalid name" % name)

        return []
