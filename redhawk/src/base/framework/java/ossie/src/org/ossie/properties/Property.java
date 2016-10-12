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

import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;
import org.omg.CORBA.TCKind;
import org.ossie.properties.AnyUtils;

import CF.DevicePackage.InvalidCapacity;
import CF.DevicePackage.InvalidState;

/**
 * Internal class
 *
 * @param <T>
 */
abstract class Property<T extends Object> implements IProperty {

    protected String id;
    protected String name;
    protected Mode mode;
    protected Action action;
    protected Set<Kind> kinds;
    protected boolean optional;

    protected T value;

    protected List<PropertyListener<T>> changeListeners = new LinkedList<PropertyListener<T>>();
    protected List<PropertyListener< Object >> voidListeners = new LinkedList<PropertyListener<Object >>();
    protected Allocator<T> allocator = null;
    
    protected Property(String id, String name, T value, Mode mode, Action action, Kind[] kinds) {
        super();
        this.id = id;
        this.name = name;
        this.value = value;
        this.optional = false;
            
        if (action == null) {
            this.action = Action.EXTERNAL;
        } else {
            this.action = action;
        }
              
        if (mode == null) {
            this.mode = Mode.READWRITE;
        } else {
            this.mode = mode;
        }

        this.kinds = new HashSet<Kind>();
        if (kinds == null) {
            // RESOLVE --  need to deprecate 
            this.kinds.add(Kind.CONFIGURE);
            this.kinds.add(Kind.PROPERTY);
        } else {
            for (Kind kind : kinds) {
                this.kinds.add(kind);
            }
        }
    }

    protected Property(String id, String name, T value, Mode mode, Action action, Kind[] kinds, boolean optional) {
        super();
        this.id = id;
        this.name = name;
        this.value = value;
        this.optional = optional;
            
        if (action == null) {
            this.action = Action.EXTERNAL;
        } else {
            this.action = action;
        }
              
        if (mode == null) {
            this.mode = Mode.READWRITE;
        } else {
            this.mode = mode;
        }

        this.kinds = new HashSet<Kind>();
        if (kinds == null) {
            // RESOLVE --  need to deprecate 
            this.kinds.add(Kind.CONFIGURE);
            this.kinds.add(Kind.PROPERTY);
        } else {
            for (Kind kind : kinds) {
                this.kinds.add(kind);
            }
        }
    }

    /**
     * Updates the value of the property, triggering any change listeners.
     */
    public void construct(Any any) {
        T oldValue = this.value;
        fromAny(any);
    }


    /**
     * Updates the value of the property, triggering any change listeners.
     */
    public void configureNoCallbacks(Any any) {
        fromAny(any);
    }
    
    /**
     * Updates the value of the property, triggering any change listeners.
     */
    public void configure(Any any) {
        boolean trigger_callback = false;
        if (AnyUtils.compareAnys(this.toAny(), any, "ne")) {
            trigger_callback = true;
        }
        T oldValue = this.value;
        configureNoCallbacks(any);
        if (trigger_callback) {
            for (PropertyListener<T> listener : changeListeners) {
                listener.valueChanged(oldValue, this.value);
            }

            for (PropertyListener<Object> listener : voidListeners) {
                listener.valueChanged(oldValue, this.value);
            }
        }
    }

    /**
     * Gets the current value of the property.
     */
    public T getValue() {
        return this.value;
    }
    
    /**
     * Sets the current value of the property.
     */
    public void setValue(T value) {
        T tmpValue = this.value;
        this.value = value;
        for (PropertyListener<Object> listener : voidListeners) {
            listener.valueChanged(value, tmpValue);
        }
    }

    /**
     * Registers a listener for changes to this property's value.
     */
    public void addChangeListener(PropertyListener<T> listener) {
        changeListeners.add(listener);
    }
    
    /**
     * Unregisters a listener for changes to this property's value.
     */
    public void removeChangeListener(PropertyListener<T> listener) {
        changeListeners.remove(listener);
    }

    /**
     * Registers a listener for changes to this property's value.
     */
    public void addObjectListener(PropertyListener<Object> listener) {
        voidListeners.add(listener);
    }
    
    /**
     * Unregisters a listener for changes to this property's value.
     */
    public void removeObjectListener(PropertyListener<Object> listener) {
        voidListeners.remove(listener);
    }


    /**
     * Attempts to allocate capacity from this property. If an allocator is
     * set, the operation is delgated to the allocator.
     */
    public boolean allocate(Any any) throws InvalidCapacity, InvalidState {
        if (!isAllocatable()) {
            throw new UnsupportedOperationException("Property " + this.id + " is not allocatable");
        }
        T capacity = fromAny_(any);
        if (this.allocator != null) {
            return this.allocator.allocate(capacity);
        } else {
            return allocate(capacity);
        }
    }

    /**
     * Default implementation of capacity allocation; always returns false (no
     * allocation occurred). Subclasses that can handle allocation without a
     * delegate (such as simple numeric properties) may override this method.
     */
    protected boolean allocate(T capacity) {
        return false;
    }

    /**
     * Attempts to deallocate capacity from this property. If an allocator is
     * set, the operation is delgated to the allocator.
     */
    public void deallocate(Any any) throws InvalidCapacity, InvalidState {
        if (!isAllocatable()) {
            throw new UnsupportedOperationException("Property " + this.id + " is not allocatable");
        }
        T capacity = fromAny_(any);
        if (this.allocator != null) {
            this.allocator.deallocate(capacity);
        } else {
            deallocate(capacity);
        }
    }

    /**
     * Default implementation of capacity deallocation; does nothing.
     * Subclasses that can handle deallocation without a delegate (such as
     * simple numeric properties) may override this method.
     */
    protected void deallocate(T capacity) {

    }

    public void fromAny(Any any) {
        this.value = fromAny_(any);
    }
    protected abstract T fromAny_(Any any);

    /**
     * Set the delegate responsible for handling allocation and deallocation
     * of this property.
     */
    public void setAllocator(Allocator<T> listener) {
        allocator = listener;
    }
    
    public String getId() {
        return id;
    }

    
    public String getName() {
        return name;
    }

    
    public String getMode() {
        return mode.toString();
    }

    
    public String getAction() {
        return action.toString();
    }

    
    public String[] getKinds() {
        String[] ret = new String[this.kinds.size()];
        int ii = 0;
        for (Kind kind : this.kinds) {
            ret[ii++] = kind.toString();
        }
        return ret;
    }


    public boolean isQueryable() {
        if (this.kinds.contains(Kind.CONFIGURE) || 
            this.kinds.contains(Kind.EXECPARAM) || 
            this.isProperty() ) {
            return (mode != Mode.WRITEONLY);
        }
        return false;
    }

    
    public boolean isProperty() {
        if (this.kinds.contains(Kind.PROPERTY)) {
            return true;
        }
        return false;
    }

    public boolean isConfigurable() {
        if (this.kinds.contains(Kind.CONFIGURE) || this.isProperty() ) {
            return (mode != Mode.READONLY);
        }
        return false;
    }
    
    public boolean isAllocatable() {
        if (this.kinds.contains(Kind.ALLOCATION)) {
            return (action == Action.EXTERNAL);
        }
        return false;
    }

    public boolean isEventable() {
        return this.kinds.contains(Kind.EVENT);
    }

    public boolean isSet() {
        boolean retval = false;
        if (this.optional == true) {
            if (this.getValue() instanceof List) {
                if (!((List)this.getValue()).isEmpty()) {
                    retval = true;
                }
            } else {
                if (this.getValue() != null) {
                    retval = true;
                }
            }
        } else {
            retval = true;
        }
        return retval;
    }      
}
