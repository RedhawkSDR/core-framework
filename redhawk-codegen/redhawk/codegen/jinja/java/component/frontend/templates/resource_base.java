/*#
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
 #*/
//% extends "pull/resource_base.java"
//
/*{% block frontendimports %}*/
/*{% if component.hasfrontendprovides %}*/
import frontend.*;
/*{% endif %}*/
/*{% endblock %}*/
/*{% block baseadditionalimports %}*/
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Map;
import java.util.HashMap;
import org.omg.CORBA.Any;
/*{% if 'FrontendTuner' in component.implements %}*/
/*{%     if not component.hasmultioutport %}*/
import java.util.List;
/*{%     endif %}*/
import org.omg.CORBA.ORB;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;
import CF.DeviceManager;
import CF.DevicePackage.InvalidCapacity;
/*{%  endif %}*/
/*{% endblock %}*/

/*{% block getTunerStatus %}*/
/*{% if 'FrontendTuner' in component.implements %}*/
    public CF.DataType[] getTunerStatus(final String allocation_id) throws FRONTEND.FrontendException
    {
        int tuner_id = getTunerMapping(allocation_id);
        if (tuner_id < 0){
            throw new FRONTEND.FrontendException("ERROR: ID: " + allocation_id + " IS NOT ASSOCIATED WITH ANY TUNER!");
        }
        Any prop;
        prop = frontend_tuner_status.getValue().get(tuner_id).toAny();
        CF.DataType[] tmpVal = (CF.DataType[]) AnyUtils.convertAny(prop);
        return tmpVal;
    }
/*{% endif %}*/
/*{% endblock %}*/

/*{% block extensions %}*/
/*{% if 'FrontendTuner' in component.implements %}*/
    protected Map<String, String> listeners = new HashMap<String, String>();

    public void frontendTunerStatusChanged(final List<frontend_tuner_status_struct_struct> oldValue, final List<frontend_tuner_status_struct_struct> newValue)
    {
        this.tuner_allocation_ids = new ArrayList<frontend.FrontendTunerDevice<frontend_tuner_status_struct_struct>.tunerAllocationIdsStruct>(this.frontend_tuner_status.getValue().size());
    }

    public void assignListener(final String listen_alloc_id, final String allocation_id)
    {
        // find control allocation_id
        String existing_alloc_id;
        existing_alloc_id = listeners.get(allocation_id);
        if (existing_alloc_id == null){
            existing_alloc_id = allocation_id;
        }
        listeners.put(listen_alloc_id,existing_alloc_id);

/*{% if component.hasmultioutport %}*/
        List<connection_descriptor_struct> old_table = this.connectionTable.getValue();
        List<connection_descriptor_struct> new_entries = new ArrayList<connection_descriptor_struct>();

        for (Iterator<connection_descriptor_struct> itr = this.connectionTable.getValue().iterator(); itr.hasNext();) {
            connection_descriptor_struct entry = itr.next();
            if (entry.connection_id.getValue().equals(existing_alloc_id)) {
                connection_descriptor_struct tmp = new connection_descriptor_struct();
                tmp.connection_id.setValue(listen_alloc_id);
                tmp.stream_id.setValue(entry.stream_id.getValue());
                tmp.port_name.setValue(entry.port_name.getValue());
                new_entries.add(tmp);
            }
        }
        for (Iterator<connection_descriptor_struct> new_itr = new_entries.iterator(); new_itr.hasNext();) {
            boolean foundEntry = false;
            connection_descriptor_struct new_entry = new_itr.next();
            for (Iterator<connection_descriptor_struct> itr = this.connectionTable.getValue().iterator(); itr.hasNext();) {
                connection_descriptor_struct entry = itr.next();
                if (entry.equals(new_entry)) {
                    foundEntry = true;
                    break;
                }
            }
            if (!foundEntry) {
                this.connectionTable.getValue().add(new_entry);
            }
        }
        this.connectionTableChanged(old_table, this.connectionTable.getValue());
/*{% endif %}*/
    }

    public void removeListener(final String listen_alloc_id)
    {
        listeners.remove(listen_alloc_id);
/*{% if component.hasmultioutport %}*/
        List<connection_descriptor_struct> old_table = this.connectionTable.getValue();
        for (Iterator<connection_descriptor_struct> itr = this.connectionTable.getValue().iterator(); itr.hasNext();) {
            connection_descriptor_struct entry = itr.next();
            if (entry.connection_id.getValue().equals(listen_alloc_id)) {
                itr.remove();
            }
        }
        ExtendedCF.UsesConnection[] tmp;
/*{%   for port_out in component.ports if port_out.multiout %}*/
        // Check to see if port "${port_out.javaname}" has a connection for this listener
        tmp = this.${port_out.javaname}.connections();
        for (int i=0; i<this.${port_out.javaname}.connections().length; i++) {
            String connection_id = tmp[i].connectionId;
            if (connection_id.equals(listen_alloc_id)) {
                this.${port_out.javaname}.disconnectPort(connection_id);
            }
        }
/*{%   endfor %}*/
        this.connectionTableChanged(old_table, this.connectionTable.getValue());
/*{% endif %}*/
    }
/*{% endif %}*/
/*{% if component.hasmultioutport %}*/

    public void removeAllocationIdRouting(final int tuner_id)
    {
        String allocation_id = getControlAllocationId(tuner_id);
        List<connection_descriptor_struct> old_table = this.connectionTable.getValue();
        for (Iterator<connection_descriptor_struct> itr = this.connectionTable.getValue().iterator(); itr.hasNext();) {
            connection_descriptor_struct entry = itr.next();
            if (entry.connection_id.getValue().equals(allocation_id)) {
                itr.remove();
            }
        }
        for (Map.Entry<String, String> listener:listeners.entrySet()) {
            if (listener.getValue().equals(allocation_id)) {
                for (Iterator<connection_descriptor_struct> itr = this.connectionTable.getValue().iterator(); itr.hasNext();) {
                    connection_descriptor_struct entry = itr.next();
                    if (entry.connection_id.getValue().equals(listener.getKey())) {
                        itr.remove();
                    }
                }
            }
        }
        this.connectionTableChanged(old_table, this.connectionTable.getValue());
    }

    public void removeStreamIdRouting(final String stream_id, final String allocation_id)
    {
        List<connection_descriptor_struct> old_table = this.connectionTable.getValue();
        for (Iterator<connection_descriptor_struct> itr = this.connectionTable.getValue().iterator(); itr.hasNext();) {
            connection_descriptor_struct entry = itr.next();
            if (allocation_id == "") {
                if (entry.stream_id.getValue().equals(stream_id)) {
                    itr.remove();
                }
            } else {
                if ((entry.stream_id.getValue().equals(stream_id)) && (entry.connection_id.getValue().equals(allocation_id))) {
                    itr.remove();
                }
            }
        }
        for (Map.Entry<String, String> listener:listeners.entrySet()) {
            if (listener.getValue().equals(allocation_id)) {
                for (Iterator<connection_descriptor_struct> itr = this.connectionTable.getValue().iterator(); itr.hasNext();) {
                    connection_descriptor_struct entry = itr.next();
                    if ((entry.connection_id.getValue().equals(listener.getKey())) && (entry.stream_id.getValue().equals(stream_id))) {
                        itr.remove();
                    }
                }
            }
        }
        this.connectionTableChanged(old_table, this.connectionTable.getValue());
    }

    public void matchAllocationIdToStreamId(final String allocation_id, final String stream_id, final String port_name)
    {
        if (port_name != "") {
            for (Iterator<connection_descriptor_struct> itr = this.connectionTable.getValue().iterator(); itr.hasNext();) {
                connection_descriptor_struct entry = itr.next();
                if (!entry.port_name.getValue().equals(port_name))
                    continue;
                if (!entry.stream_id.getValue().equals(stream_id))
                    continue;
                if (!entry.connection_id.getValue().equals(allocation_id))
                    continue;
                // all three match. This is a repeat
                return;
            }
            List<connection_descriptor_struct> old_table = this.connectionTable.getValue();
            connection_descriptor_struct tmp = new connection_descriptor_struct();
            tmp.connection_id.setValue(allocation_id);
            tmp.port_name.setValue(port_name);
            tmp.stream_id.setValue(stream_id);
            this.connectionTable.getValue().add(tmp);
            this.connectionTableChanged(old_table, this.connectionTable.getValue());
            return;
        }
        List<connection_descriptor_struct> old_table = this.connectionTable.getValue();
        connection_descriptor_struct tmp = new connection_descriptor_struct();
/*{%   for port in component.ports if port.multiout %}*/
        tmp.connection_id.setValue(allocation_id);
        tmp.port_name.setValue("${port.name}");
        tmp.stream_id.setValue(stream_id);
        this.connectionTable.getValue().add(tmp);
/*{%   endfor %}*/
        this.connectionTableChanged(old_table, this.connectionTable.getValue());
    }
/*{% else %}*/
    public void removeAllocationIdRouting(final int tuner_id)
    {
    }
/*{% endif %}*/
/*{% endblock %}*/

/*{% block resourcemain %}*/
    /**
     * The main function of your ${artifactType}.  If no args are provided, then the
     * CORBA object is not bound to an SCA Domain or NamingService and can
     * be run as a standard Java application.
     * 
     * @param args
     * @generated
     */
    public static void main(String[] args)
    {
        final Properties orbProps = new Properties();
        ${userclass}.configureOrb(orbProps);

        try {
/*{% if component is device %}*/
            Device.start_device(${userclass}.class, args, orbProps);
/*{% else %}*/
            ${superClass}.start_${artifactType}(${userclass}.class, args, orbProps);
/*{% endif %}*/
        } catch (InvalidObjectReference e) {
            e.printStackTrace();
/*{% if component is not device %}*/
        } catch (NotFound e) {
            e.printStackTrace();
        } catch (CannotProceed e) {
            e.printStackTrace();
        } catch (InvalidName e) {
            e.printStackTrace();
/*{% endif %}*/
        } catch (ServantNotActive e) {
            e.printStackTrace();
        } catch (WrongPolicy e) {
            e.printStackTrace();
        } catch (InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
    }
/*{% endblock %}*/
}
