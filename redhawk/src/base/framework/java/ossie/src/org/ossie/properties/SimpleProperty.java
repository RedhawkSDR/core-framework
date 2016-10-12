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
import org.omg.CORBA.TypeCode;
import org.omg.CORBA.TCKind;

@Deprecated
public class SimpleProperty<T extends Object> extends LegacyProperty<T> {
    final private T defaultValue;
    final private String type;
    final private TypeCode corbaType;
    private Any origCap = null;

    public SimpleProperty(String   id,
                          String   name,
                          String   type,
                          T        value,
                          String   mode,
                          String   action, 
                          String[] kinds) {
        super(id, name, value, mode, action, kinds);
        this.defaultValue = value;
        this.type = type;
        this.corbaType = AnyUtils.convertToTypeCode(type);
    }

    public Any toAny() {
        return AnyUtils.toAny(value, corbaType);
    }

    private Any getOrigCap() {
        // If this is the first time, store original value.
        if (this.origCap == null) {
            this.origCap = this.toAny();
        }
        return this.origCap;
    }
    
    protected T fromAny_(Any any) {
        try {
            // Suppress unchecked cast warning; due to type erasure we do not
            // know the actual type of T, so this is an unsafe operation, which
            // is one of the reasons for deprecating this class.
            @SuppressWarnings("unchecked") T val = (T)AnyUtils.convertAny(any);
            return val;
        } catch (ClassCastException ex) {
            throw new IllegalArgumentException("Incorrect any type recevied");
        }
    }

    /**
     * Backwards-compatible allocation for legacy properties, taken from Device.
     */
    public boolean allocate(Any resourceRequest){
        // If this is the first time, store original value.
        getOrigCap();

        Any deviceCapacity = this.toAny();
        TypeCode tc1 = deviceCapacity.type();

        try {
            switch (tc1.kind().value()){
            case TCKind._tk_ulong: {
                Number devCapac, rscReq;    
                devCapac = (Number) AnyUtils.convertAny(deviceCapacity);
                rscReq = (Number) AnyUtils.convertAny(resourceRequest);

                if (rscReq.intValue() <= devCapac.intValue()){
                    AnyUtils.insertInto(deviceCapacity, devCapac.intValue() - rscReq.intValue(), tc1.kind());
                    this.configure(deviceCapacity);
                    return true;
                } else {
                    return false;
                }
            }

            case TCKind._tk_long: {
                Number devCapac, rscReq;
                devCapac = (Number) AnyUtils.convertAny(deviceCapacity);
                rscReq = (Number) AnyUtils.convertAny(resourceRequest);

                if (rscReq.intValue() <= devCapac.intValue()){
                    AnyUtils.insertInto(deviceCapacity, devCapac.intValue() - rscReq.intValue(), tc1.kind());
                    this.configure(deviceCapacity);
                    return true;
                } else {
                    return false;
                }

            }

            case TCKind._tk_short: {
                Number devCapac, rscReq;
                devCapac = (Number) AnyUtils.convertAny(deviceCapacity);
                rscReq = (Number) AnyUtils.convertAny(resourceRequest);

                if (rscReq.shortValue() <= devCapac.shortValue()){
                    AnyUtils.insertInto(deviceCapacity, devCapac.shortValue() - rscReq.shortValue(), tc1.kind());
                    this.configure(deviceCapacity);
                    return true;
                } else {
                    return false;
                }
            }

            default:
                return false;
            }
        } catch (final ClassCastException ex) {
            throw new IllegalArgumentException("Non-numeric value type");
        }
    }

    /**
     * Backwards-compatible deallocation for legacy properties, take from Device.
     */
    public void deallocate(Any resourceRequest) {
        final Any deviceCapacity = this.toAny();
        TypeCode tc1 = deviceCapacity.type();

        try {
            switch(tc1.kind().value()){
            case TCKind._tk_ulong: {
                Number devCapac, rscReq;
                devCapac = (Number) AnyUtils.convertAny(deviceCapacity);
                rscReq = (Number) AnyUtils.convertAny(resourceRequest);
                int newCap = devCapac.intValue() + rscReq.intValue();
                AnyUtils.insertInto(deviceCapacity, devCapac.intValue() + rscReq.intValue(), tc1.kind());
                break;
            }

            case TCKind._tk_long: {
                Number devCapac, rscReq;
                devCapac = (Number) AnyUtils.convertAny(deviceCapacity);
                rscReq = (Number) AnyUtils.convertAny(resourceRequest);
                AnyUtils.insertInto(deviceCapacity, devCapac.intValue() + rscReq.intValue(), tc1.kind());
                break;
            }

            case TCKind._tk_short: {
                Number devCapac, rscReq;
                devCapac = (Number) AnyUtils.convertAny(deviceCapacity);
                rscReq = (Number) AnyUtils.convertAny(resourceRequest);
                AnyUtils.insertInto(deviceCapacity, devCapac.shortValue() + rscReq.shortValue(), tc1.kind());
                break;
            }

            default:
                break;
            }
        } catch (final ClassCastException ex) {
            throw new IllegalArgumentException("Non-numeric value type");
        }
        
        if (AnyUtils.compareAnys(deviceCapacity, this.getOrigCap(), "gt")) {
            throw new ArithmeticException("New capacity would exceed original bound");
        }

        this.configure(deviceCapacity);
    }

    public boolean isFull() {
        return AnyUtils.compareAnys(this.toAny(), this.getOrigCap(), "ge");
    }

    @Override
    public String toString() {
        return this.id + "/" + this.name + " = " + this.value;
    }
    
    
    public void fromString(String str) {
        // Suppress unchecked cast warning; due to type erasure we do not
        // know the actual type of T, so this is an unsafe operation, which
        // is one of the reasons for deprecating this class.
        @SuppressWarnings("unchecked") T val = (T)convertString(str, type);
        this.setValue(val);
    }

    public String getType() {
        return this.type;
    }
}
