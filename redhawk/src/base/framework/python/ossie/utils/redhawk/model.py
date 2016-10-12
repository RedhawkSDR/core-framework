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

import threading

from ossie.utils.notify import notification

class CorbaAttribute(object):
    """
    Wrapper for CORBA attributes with value caching and change notification.
    """

    @notification
    def changed(self, oldValue, newValue):
        """
        The value of this attribute changed from 'oldValue' to 'newValue'.
        """
        pass

    def __init__(self, getter, setter=None):
        self._getter = getter
        self._setter = setter
        self._lock = threading.RLock()

    @property
    def isCached(self):
        """
        True if a cached copy of the CORBA attribute is available.
        """
        self._lock.acquire()
        try:
            return hasattr(self, '_value')
        finally:
            self._lock.release()

    def get_value(self):
        """
        The cached value of the CORBA attribute. If the value is not cached yet
        it is fetched.
        """
        self._lock.acquire()
        try:
            if not self.isCached:
                self.sync()
            return self._value
        finally:
            self._lock.release()

    def set_value(self, value):
        """
        Sets the attribute value via CORBA. If the attribute is read-only, an
        AttributeError is raised.
        """
        if not self._setter:
            raise AttributeError, 'CORBA attribute is read-only'
        self._lock.acquire()
        try:
            if self.value == value:
                return
            self._setter(value)
            # Get the updated value from the CORBA attribute--there's no guarantee
            # that the new value is actually the value that was set (e.g. device
            # adminState ignores invalid state transitions).
            self.sync()
        finally:
            self._lock.release()

    value = property(get_value, set_value)

    def sync(self):
        """
        Synchronizes the cached value with the current value of the CORBA
        attribute. If a prior value was cached and the new value is different,
        the "changed" notification is sent.
        """
        value = self._getter()
        self._lock.acquire()
        try:
            if self.isCached:
                self.update(value)
            else:
                self._value = value
        finally:
            self._lock.release()

    def update(self, value):
        """
        Updates the cached value of the CORBA attribute, sending the "changed"
        notification if the new value differs from the old value.

        If the value is not cached yet, nothing happens.
        """
        if not self.isCached:
            return
        self._lock.acquire()
        try:
            oldValue = self._value
            self._value = value
        finally:
            self._lock.release()
        if value != oldValue:
            self.changed(oldValue, value)


class DomainObjectList(object):
    @notification
    def itemAdded(self, item):
        """
        Indicates that an item was added to the list.
        """
        pass

    @notification
    def itemRemoved(self, identifier):
        """
        Indicates that the item with the given identifier was removed from the
        list.
        """
        pass

    def __init__(self, update, create, identifier):
        self.__update = update
        self.__create = create
        self.__identifier = identifier
        self.__cached = False
        self.__data = {}
        self.__lock = threading.RLock()

    def __add(self, identifier, value, notify):
        if identifier in self.__data:
            # Item already exists.
            return self.__data[identifier]

        item = self.__create(value)

        self.__data[identifier] = item

        if notify:
            self.itemAdded(item)

        return item

    def __remove(self, identifier, notify):
        if identifier in self.__data:
            del self.__data[identifier]
        elif self.isCached:
            # Full state is known and item is not in list.
            raise KeyError, 'No item with identifier "%s"' % identifier

        if notify:
            self.itemRemoved(identifier)

    @property
    def isCached(self):
        self.lock()
        try:
            return self.__cached
        finally:
            self.unlock()

    def sync(self):
        items = self.__update()
        self.lock()
        try:
            # Track the identifiers of all the item in the new list. Any item whose
            # identifier is not in this set can be assumed to be stale.
            validIdentifiers = set()

            for item in items:
                try:
                    identifier = self.__identifier(item)
                    validIdentifiers.add(identifier)
                    self.__add(identifier, item, self.__cached)
                except:
                    pass

            # Remove stale items.
            for identifier in self.__data.keys():
                if identifier not in validIdentifiers:
                    self.__remove(identifier, self.__cached)

            self.__cached = True
        finally:
            self.unlock()

    def add(self, identifier, value):
        self.lock()
        try:
            return self.__add(identifier, value, True)
        finally:
            self.unlock()

    def remove(self, identifier):
        self.lock()
        try:
            self.__remove(identifier, True)
        finally:
            self.unlock()

    def values(self):
        self.lock()
        try:
            if not self.isCached:
                self.sync()
            return self.__data.values()
        finally:
            self.unlock()

    def lock(self):
        self.__lock.acquire()

    def unlock(self):
        self.__lock.release()
