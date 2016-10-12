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

package org.ossie.properties;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;

import CF.DataType;
import CF.PropertiesHelper;

public abstract class StructDef {
    private Map<String, IProperty> elements;

    public StructDef() {

    }

    public Map<String, IProperty> getElementsMap() {
        if (this.elements == null) {
            this.elements = new LinkedHashMap<String, IProperty>();
            initFields();
        }
        return this.elements;
    }

    private void initFields() {
        final List<Field> allFields = StructDef.getAllFields(new ArrayList<Field>(), getClass());
        for (final Field field : allFields) {
            try {
                // Skip uninitialized fields, which can happen with classes derived
                // from concrete subclasses of StructDef (such as the FRONTEND
                // tuner status property)
                if (field.get(this) == null) {
                    continue;
                }
                if (IProperty.class.isAssignableFrom(field.getType())) {
                    if (!field.isAccessible()) {
                        field.setAccessible(true);
                    }
                    addElement((IProperty) field.get(this));
                }
            } catch (final IllegalArgumentException e) {
                e.printStackTrace();
            } catch (final IllegalAccessException e) {
                e.printStackTrace();
            }
        }
    }

    private static List<Field> getAllFields(final List<Field> fields, final Class< ? > type) {
        for (final Field field : type.getDeclaredFields()) {
            fields.add(field);
        }
        if (type.getSuperclass() != null) {
            StructDef.getAllFields(fields, type.getSuperclass());
        }
        return fields;
    }

    protected void addElement(final IProperty element) {
        getElementsMap().put(element.getId(), element);
    }
    
    public boolean equals(final Object item) {
        if (!(item instanceof StructDef)) {
            return false;
        }
        StructDef tmp = (StructDef)item;
        for (final IProperty prop : this.getElements()) {
            Any tmp_prop = tmp.getElement(prop.getId()).toAny();
            if (tmp_prop == null) {
                return false;
            }
            if (!prop.toAny().equal(tmp_prop)) {
                return false;
            }
        }
        return true;
    }

    public IProperty[] getElements() {
        return getElementsMap().values().toArray(new IProperty[this.getElementsMap().size()]);
    }

    public IProperty getElement(final String id) {
        return getElementsMap().get(id);
    }

    public Any toAny() {
        Any retVal = ORB.init().create_any();

        DataType[] props = new DataType[this.getElementsMap().size()];
        int ii = 0;
        for (IProperty prop : this.getElementsMap().values()) {
            props[ii++] = new DataType(prop.getId(), prop.toAny());
        }
        
        PropertiesHelper.insert(retVal, props);
        return retVal;
    }

    public void fromAny(Any any) {
        if (!any.type().equivalent(PropertiesHelper.type())) {
            throw new IllegalArgumentException("Invalid Any type for struct");
        }
        for (final DataType prop : PropertiesHelper.extract(any)) {
            IProperty field = this.getElement(prop.id);
            // Ignore unknown fields
            if (field != null) {
                field.fromAny(prop.value);
            }
        }
    }

    @Override
    public String toString() {
        final StringBuffer sb = new StringBuffer();
        sb.append("StructDef:");
        sb.append(super.toString());
        sb.append(this.getElementsMap());
        return sb.toString();
    }

    public String getId() {
        // Ideally, this would be an abstract method; however, since there are
        // existing Java components whose structs do not override this method,
        // return an empty string to avoid breaking source compatibility.
        return "";
    }
};
