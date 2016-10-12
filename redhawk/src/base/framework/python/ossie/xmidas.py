#!/usr/bin/env xmpy
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
# X-MIDAS interoperabiliy layer

import warnings
warnings.warn("ossie.xmidas is deprecated, use xmsca.resource instead", DeprecationWarning)

from XMinter import *

try:
    from xmsca.resource import *
except ImportError:
    # If it's not available, provide the last version of ossie.xmidas
    # This code should eventually be removed.
    from ossie.cf import CF
    from ossie.properties import simple_property, struct_property
    from omniORB import any

    def bind_to_res(property_, resname):
        """Rebinds a component property to a result table value.

        Example usage:

            class SomeClass(Resource):
                prop1 = simple_property(....)
                prop1 = bind_to_res(prop1, "RESNAME")
        """
        def _setter(self, v):
            res[resname] = v
        def _getter(self):
            return res[resname]
        return property_.rebind(fget=_getter, fset=_setter)

    class ResourceControlPanel(ControlPanel):
        """Provides a pre-populated control panel for ossie.resource.Resource objects.

        When possible, this control panel will be pre-populated with widgets that
        are tied to all properties defined on the component (using ossie.properties).
        """
        RES_PREFIX = "_CTRL_PANEL_"

        class _WidgetPropertyBinding(object):
            """Internal Class, not intended for use outside of this class."""
            def __init__(self, controlpanel, component, property):
                self.__controlpanel = controlpanel
                self.__component = component
                self.__property = property
                self.__reslabel = ResourceControlPanel.RES_PREFIX + self.__property.id_
                # X-MIDAS doesn't like certain characters in result labels
                self.__reslabel = self.__reslabel.replace("-", "_")
                self.__reslabel = self.__reslabel.replace(":", "_")
                self.__wid = None
                # This creates a result parameter so that
                # widgets can update their values with a query
                res[self.__reslabel] = self.__property.defvalue

                self.__name = self.__property.name
                if self.__name is None:
                    self.__name = self.__property.id_

                if self.__property.isWritable():
                    if self.__property.type_ in ("double"):
                        self.__wid = self.__controlpanel.dval(self.__reslabel, self.value, 1, -1, 0.01, name=self.__name, callback=self.configure)
                    if self.__property.type_ in ("float"):
                        self.__wid = self.__controlpanel.fval(self.__reslabel, self.value, 1, -1, 0.01, name=self.__name, callback=self.configure)
                    elif self.__property.type_ in ("short", "long"):
                        self.__wid = self.__controlpanel.lval(self.__reslabel, self.value, 1, -1, 1, name=self.__name, callback=self.configure)
                    elif self.__property.type_ in ("string"):
                        self.__wid = self.__controlpanel.aprompt(self.__reslabel, self.value, "%s:" % (self.__name), callback=self.configure)
                    elif self.__property.type_ =="boolean":
                        # Menu string will be set below by setting property
                        self.__wid = self.__controlpanel.menu(None, None, callback=self._booleanChange)
                else:
                    if self.__property.type_ in ("double"):
                        self.__wid = self.__controlpanel.dmonitor(self.__reslabel, name=self.__name+":", disp=True, callback=None)
                    elif self.__property.type_ in ("float"):
                        self.__wid = self.__controlpanel.fmonitor(self.__reslabel, name=self.__name+":", disp=True, callback=None)
                    elif self.__property.type_ in ("short", "long"):
                        self.__wid = self.__controlpanel.lmonitor(self.__reslabel, name=self.__name+":", disp=True, callback=None)
                    elif self.__property.type_ in ("string"):
                        self.__wid = self.__controlpanel.amonitor(self.__reslabel, name=self.__name+":", disp=True, callback=None)
                    elif self.__property.type_ == "boolean":
                        # Menu string will be set below by setting property
                        self.__wid = self.__controlpanel.menu(None, None, noedit=True, callback=None)

                # Set the property value, which will cause an update to the
                # X-Midas widget. Because the boolean widgets map value
                # changes into menu updates, this call has to be made after
                # the menu widget is created.
                self.value = self.__property.defvalue

            def __del__(self):
                del res[self.__reslabel]

            def _booleanChange(self, v):
                self.value = not self.value
                self.configure(self.value)

            def configure(self, v):
                self.__component.configure([CF.DataType(id=self.__property.id_, value=any.to_any(v))])

            def query(self, v):
                val = self.__component.query([CF.DataType(id=self.__property.id_, value=any.to_any(None))])
                return any.from_any(val[0].value)

            def set_value(self, v):
                if self.__property.type_ == "boolean":
                    if v:
                        name = ';+ ' + self.__name
                    else:
                        name = ';- ' + self.__name
                    self.__controlpanel.menu(None, None, name, mod=self.__wid)
                    res[self.__reslabel] = v
                else:
                    self.__controlpanel.setcontrol(self.__reslabel, v)

            def get_value(self):
                if self.__property.type_ == "boolean":
                    return res[self.__reslabel] == 1
                return res[self.__reslabel]

            value = property(fget=get_value, fset=set_value)

        def __init__(self, component_Obj, *args, **kw):
            ControlPanel.__init__(self, *args, **kw)
            self._component = component_Obj
            self._widgetBindings = {}

            # Create a top-level widget to start/stop
            self.menu('main', 0, 'MAIN;Refresh,Start,Stop,Exit', callback=self._resourceControl)

            # Create widgets for all properties that are configurable
            for name in dir(type(self._component)):
                attr = getattr(type(self._component), name)
                if isinstance(attr, simple_property) and (attr.isConfigurable() or attr.isQueryable()):
                    binding = ResourceControlPanel._WidgetPropertyBinding(self, self._component, attr)
                    self._widgetBindings[attr.id_] = binding

        def _resourceControl(self, v):
            if v == 1:
                self.refreshWidgets()
            elif v == 2:
                self._component.start()
            elif v == 3:
                self._component.stop()
            elif v == 4:
                self._component.stop()
                self.quit()

        def refreshWidgets(self):
            vals = self._component.query([])
            for dt in vals:
                self._widgetBindings[dt.id].value = any.from_any(dt.value)

if __name__ == "__main__":
    from ossie.resource import Resource
    from ossie.cf import CF__POA
    import logging

    class XmpyResource(Resource, CF__POA.Resource):
        def start(self):
            self.prop_short += 1 # FOR NOW, A CRAPPY TEST TO SEE IF REFRESH WORKS
            Resource.start(self)
            pipe_on()
            pipe_start()

        def stop(self):
            Resource.stop(self)
            pipe_off()
            pipe_wait()

        prop_float  = simple_property(id_="a_float", type_="float", defvalue=0.0)
        prop_double = simple_property(id_="a_double", type_="double", mode="readonly", defvalue=0.0)
        prop_long   = simple_property(id_="a_long", type_="long", defvalue=0)
        prop_short  = simple_property(id_="a_short", type_="short", defvalue=0)
        prop_bool   = simple_property(id_="a_bool", type_="boolean", defvalue=False)
        prop_string = simple_property(id_="a_string", type_="string", defvalue="")

        prop_float  = bind_to_res(prop_float, "INTERNAL_FLOAT")

    logging.getLogger().setLevel(logging.DEBUG)
    component = XmpyResource("", {}) 
    cp = ResourceControlPanel(component, setup=1)
    cp.run()
