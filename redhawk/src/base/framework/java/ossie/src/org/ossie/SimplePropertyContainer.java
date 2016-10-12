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

package org.ossie;

import java.util.Vector;

import org.omg.CORBA.Any;
import CF.DataType;

public abstract class SimplePropertyContainer<T extends Object> {
    protected PropertyContainer prop;

    public SimplePropertyContainer() {
        this.prop = new PropertyContainer();
    }

    public SimplePropertyContainer(String id, String name, short type,
            String mode, Any initialValue, String units, String action,
            Vector<String> kinds) {
        this.prop = new PropertyContainer(id, name, type, mode, initialValue, units, action, kinds);
    }

    public abstract T getValue();
    
    public abstract void setValue(T newVal);

    /**
     * Compares this property to the given Any value.
     * 
     * @param any the value to compare to
     * @return
     */
    public int compare(Any any) {
        return this.prop.compare(any);
    }

    public void increment(Any any) {
        this.prop.increment(any);
    }

    public void decrement(Any any) {
        this.prop.decrement(any);
    }
    
    /**
     * This returns the ID of the property
     * 
     * @return the property's ID
     */
    public String getId() {
        return this.prop.getId();
    }

    /**
     * Sets the ID of the property.
     * 
     * @param id the new property ID
     */
    public void setId(String id) {
        prop.setId(id);
    }

    /**
     * Sets the name of the property
     * 
     * @param name the new property name
     */
    public void setName(String name) {
        prop.setName(name);
    }

    /**
     * Sets the CORBA type of the property
     * 
     * @param type the new CORBA type of the property
     */
    public void setType(short type) {
        prop.setType(type);
    }

    /**
     * This converts a string type value to a valid CORBA type for the property
     * 
     * @param type The new SCA string type of the property
     */
    public void setType(String type) {
        prop.setType(type);
    }

    /**
     * This sets the SCA mode of the property
     * 
     * @param mode the new SCA Mode
     */
    public void setMode(String mode) {
        prop.setMode(mode);
    }

    /**
     * This sets the SCA Units for the property
     * 
     * @param units the new SCA Units
     */
    public void setUnits(String units) {
        prop.setUnits(units);
    }

    /**
     * This sets the SCA action for the property
     * 
     * @param action the new SCA Action
     */
    public void setAction(String action) {
        prop.setAction(action);
    }

    /**
     * This sets the SCA kinds for the property
     * 
     * @param kinds the new SCA kinds
     */
    public void setKinds(Vector<String> kinds) {
        prop.setKinds(kinds);
    }

    /**
     * This sets the base property for the container
     * 
     * @param baseProperty the new base property
     */
    public void setBaseProperty(DataType baseProperty) {
        prop.setBaseProperty(baseProperty);
    }

    /**
     * This returns the name of the property.
     * 
     * @return the property's name
     */
    public String getName() {
        return this.prop.getName();
    }

    /**
     * This returns the CORBA type of the property
     * 
     * @return the property's CORBA type
     */
    public short getType() {
        return this.prop.getType();
    }

    /**
     * This returns the SCA mode of the property
     * 
     * @return the property's SCA mode
     */
    public String getMode() {
        return this.prop.getMode();
    }

    /**
     * This returns the SCA units for the property
     * 
     * @return the property's SCA units
     */
    public String getUnits() {
        return this.prop.getUnits();
    }

    /**
     * This returns the SCA action for the property
     * 
     * @return the property's SCA Action
     */
    public String getAction() {
        return this.prop.getAction();
    }

    /**
     * This returns the SCA Kinds for this property
     * 
     * @return the property's SCA Kinds values
     */
    public Vector<String> getKinds() {
        return this.prop.getKinds();
    }

    /**
     * This returns the underlying CF::DataType for the property
     * 
     * @return the base property object for this property
     */
    public DataType getBaseProperty() {
        return this.prop.getBaseProperty();
    }

}
