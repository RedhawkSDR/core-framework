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

import org.omg.CORBA.ORB;
import org.omg.CORBA.TCKind;

/**
 * Internal class
 *
 * @param <T>
 */
abstract class Property<T extends Object> implements IProperty {

    protected String id;
    protected String name;
    protected String mode;
    protected String action;
    protected String[] kinds;
    
    public Property(String id, String name, String mode, String action, String[] kinds) {
        super();
        this.id = id;
        this.name = name;
            
        if (action == null) {
            this.action = "external";
        } else {
            this.action = action;
        }
              
        if (mode == null) {
            this.mode = mode;
        } else {
            this.mode = mode;
        }
        
        if (kinds == null) {
            this.kinds = new String[] {"configure"};
        } else {
            this.kinds = kinds;
        }
    }

    /**
     * Get's the current value of the property.
     */
    public abstract T getValue();
    
    /**
     * Set's the current value of the property.
     */
    public abstract void setValue(T value);
    
    
    public String getId() {
        return id;
    }

    
    public String getName() {
        return name;
    }

    
    public String getMode() {
        return mode;
    }

    
    public String getAction() {
        return action;
    }

    
    public String[] getKinds() {
        return kinds;
    }


    public boolean isQueryable() {
        for (String kind : this.kinds) {
            if ((kind.compareTo("configure")==0)|(kind.compareTo("execparam")==0)) {
                if (mode.compareTo("writeonly")!=0) {
                    return true;
                }
            }
        }
        return false;
    }
    
    public boolean isConfigurable() {
        for (String kind : this.kinds) {
            if ((kind.compareTo("configure")==0)) {
                if (mode.compareTo("readonly")!=0) {
                    return true;
                }
            }
        }
        return false;
    }
    
    public boolean isAllocatable() {
        for (String kind : this.kinds) {
            if ((kind.compareTo("allocation")==0)) {
                if (action.compareTo("external")==0) {
                    if (this instanceof SimpleProperty){
                        return true;
                    }
                }
            }
        }
        return false;
    }

    public boolean isEventable() {
        for (String kind : this.kinds) {
            if ((kind.compareTo("event")==0)) {
                return true;
            }
        }
        return false;
    }


    
    /**
     * For execparams we need to convert the string representation to the 
     * equivalent java type.
     * 
     * @param theString
     * @param type
     * @return
     */
    public Object convertString(final String theString, final String type) {
        if (theString == null) {
            return "";
        }
        if (type.equals("string")) {
            return theString;
        } else if (type.equals("wstring")) {
            return theString;
        } else if (type.equals("boolean")) {
            return Boolean.parseBoolean(theString);
        } else if (type.equals("char")) {
            return theString.charAt(0);
        } else if (type.equals("wchar")) {
            return theString.charAt(0);
        } else if (type.equals("double")) {
            return Double.parseDouble(theString);
        } else if (type.equals("float")) {
            return Float.parseFloat(theString);
        } else if (type.equals("short")) {
            return Short.parseShort(theString);
        } else if (type.equals("long")) {
            return Integer.parseInt(theString);
        } else if (type.equals("long long")) {
            return Long.parseLong(theString);
        } else if (type.equals("ulong")) {
            return Long.parseLong(theString);
        } else if (type.equals("ushort")) {
            return Short.parseShort(theString);
        } else if (type.equals("objref")) {
            final ORB orb = ORB.init();
            return orb.string_to_object(theString);
        } else if (type.equals("octet")) {
            return theString.getBytes()[0];
        } else {
            throw new IllegalArgumentException("Unknown CORBA Type: " + type);
        }
    }
    
    public TCKind convertToTCKind(final String type) {
        if (type == null || type.equals("")) {
            return TCKind.tk_null;
        } else if (type.equals("boolean")) {
            return TCKind.tk_boolean;
        } else if (type.equals("char")) {
            return TCKind.tk_char;
        } else if (type.equals("double")) {
            return TCKind.tk_double;
        } else if (type.equals("fixed")) {
            return TCKind.tk_fixed;
        } else if (type.equals("float")) {
            return TCKind.tk_float;
        } else if (type.equals("long")) {
            return TCKind.tk_long;
        } else if (type.equals("longlong")) {
            return TCKind.tk_longlong;
        } else if (type.equals("objref")) {
            return TCKind.tk_objref;
        } else if (type.equals("octet")) {
            return TCKind.tk_octet;
        } else if (type.equals("short")) {
            return TCKind.tk_short;
        } else if (type.equals("string")) {
            return TCKind.tk_string;
        } else if (type.equals("typecode")) {
            return TCKind.tk_TypeCode;
        } else if (type.equals("ulong")) {
            return TCKind.tk_ulong;
        } else if (type.equals("ulonglong")) {
            return TCKind.tk_ulonglong;
        } else if (type.equals("ushort")) {
            return TCKind.tk_ushort;
        } else if (type.equals("value")) {
            return TCKind.tk_value;
        } else if (type.equals("wchar")) {
            return TCKind.tk_wchar;
        } else if (type.equals("wstring")) {
            return TCKind.tk_wstring;
        } else {
            throw new IllegalArgumentException("Unknown type: " + type);
        }
    }

}
