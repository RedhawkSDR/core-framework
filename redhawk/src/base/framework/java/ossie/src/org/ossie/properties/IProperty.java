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

import org.omg.CORBA.Any;

import CF.DevicePackage.InvalidCapacity;
import CF.DevicePackage.InvalidState;

public interface IProperty {
    
    /**
     * Update the value of this property from an Any.
     *
     * @return
     */
    public void configure(Any value);

    /**
     * Attempt to allocate capacity from the property.
     *
     * @return true if allocation was successful, false otherwise
     */
    public boolean allocate(Any value) throws InvalidCapacity, InvalidState;

    /**
     * Deallocate capacity from the property.
     *
     * @return
     */
    public void deallocate(Any value) throws InvalidCapacity, InvalidState;

    /**
     * Convert's this property to an Any.
     * 
     * @return
     */
    public Any toAny();
    
    /**
     * Convert's this property to an Any.
     * 
     * @return
     */
    public void fromAny(Any value);
    
    /**
     * Given a string initializer (from an execparam) set the value.  Only
     * SimpleProperty will be able to implement this.
     * 
     * @return
     */
    public void fromString(String value);
    
    /**
     * This returns the ID of the property
     * 
     * @return the property's ID
     */
    public String getId();

    /**
     * This returns the name of the property.
     * 
     * @return the property's name
     */
    public String getName();

    /**
     * This returns the SCA mode of the property
     * 
     * @return the property's SCA mode
     */
    public String getMode();

    /**
     * This returns the SCA action for the property
     * 
     * @return the property's SCA Action
     */
    public String getAction();

    /**
     * This returns the SCA Kinds for this property
     * 
     * @return the property's SCA Kinds values
     */
    public String[] getKinds();
    
    /**
     * This returns whether or not the property is queryable
     * 
     * @return whether or not the property is queryable
     */
    public boolean isQueryable();
    
    /**
     * This returns whether or not the property is configurable
     * 
     * @return whether or not the property is configurable
     */
    public boolean isConfigurable();
    
    /**
     * This returns whether or not the property is allocatable
     * 
     * @return whether or not the property is allocatable
     */
    public boolean isAllocatable();

    /**
     * This returns whether or not the property is supposed to issue an event when it changes
     * 
     * @return whether or not the property is eventable
     */
    public boolean isEventable();
}
