/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file 
 * distributed with this source distribution.
 * 
 * This file is part of REDHAWK core.
 * 
 * REDHAWK core is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your 
 * option) any later version.
 * 
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

package org.ossie.events;

import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;

import java.util.HashMap;
import java.util.Hashtable;

import CosEventChannelAdmin.*;
import org.ossie.events.MessageSupplierPort;
import org.ossie.events.Consumer_i;
import org.ossie.component.*;
import org.ossie.properties.IProperty;
import org.ossie.properties.StructDef;
import CF.*;
import CF.PortSupplierPackage.UnknownPort;
import ExtendedEvent.PropertySetChangeEventType;
import ExtendedEvent.PropertySetChangeEventTypeHelper;

public class PropertyEventSupplier extends MessageSupplierPort {

    class registeredPropertyElement {
        private String component_id;
        private String component_name;
        private IProperty property;

        public registeredPropertyElement(String component_id, String component_name, IProperty property) {
            this.component_id = component_id;
            this.component_name = component_name;
            this.property = property;
        }
        public String getComponentId() {
            return this.component_id;
        }
        public String getComponentName() {
            return this.component_name;
        }
        public IProperty getProperty() {
            return this.property;
        }
    }

    protected HashMap<String, registeredPropertyElement> propertyMap;

    public PropertyEventSupplier(String portName)
    {
        super(portName);
        this.propertyMap = new HashMap<String, registeredPropertyElement>();
    }

    public void sendPropertyEvent(final String id) {
        if (!propertyMap.containsKey(id)) {
            return;
        }
        // populate event type
        PropertySetChangeEventType eventOut = new PropertySetChangeEventType();
        eventOut.sourceId = propertyMap.get(id).getComponentId();
        eventOut.sourceName = propertyMap.get(id).getComponentName();
        eventOut.properties = new CF.DataType[1];
        eventOut.properties[0] = new CF.DataType();
        eventOut.properties[0].id = id;
        eventOut.properties[0].value = propertyMap.get(id).getProperty().toAny();
        Any event = ORB.init().create_any();
        PropertySetChangeEventTypeHelper.insert(event, eventOut);
        this.push(event);
    }

    public void registerProperty(final String component_id, final String component_name, final IProperty property) {
        // add a property to the port's map
        if (propertyMap.containsKey(property.getId())) {
            return;
        }
        registeredPropertyElement tmp = new registeredPropertyElement(component_id, component_name, property);
        propertyMap.put(property.getId(), tmp);
    }

}
