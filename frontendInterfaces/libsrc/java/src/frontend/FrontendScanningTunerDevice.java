/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK frontendInterfaces.
 *
 * REDHAWK frontendInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK frontendInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
package frontend;

import BULKIO.StreamSRI;
import BULKIO.UNITS_TIME;
import BULKIO.UNITS_NONE;
import CF.AggregateDevice;
import CF.DataType;
import CF.DeviceManager;
import CF.DevicePackage.UsageType.*;
import CF.DevicePackage.InvalidCapacity;
import CF.DevicePackage.InvalidCapacityHelper;
import CF.DevicePackage.InvalidState;
import CF.DevicePackage.UsageType;
import CF.PortSetPackage.PortInfoType;
import CF.InvalidObjectReference;
import ExtendedCF.UsesConnection;
import FRONTEND.RFInfoPkt;
import FRONTEND.BadParameterException;
import java.lang.Math.*;
import java.lang.reflect.*;
import java.text.*;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.concurrent.*;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.TimeZone;
import java.util.UUID.*;
import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;
import org.ossie.component.ThreadedDevice;
import org.ossie.properties.Action;
import org.ossie.properties.Allocator;
import org.ossie.properties.AnyUtils;
import org.ossie.properties.Kind;
import org.ossie.properties.Mode;
import org.ossie.properties.StringProperty;
import org.ossie.properties.StructProperty;
import org.ossie.properties.StructSequenceProperty;

public abstract class FrontendScanningTunerDevice<TunerStatusStructType extends frontend.FETypes.default_frontend_tuner_status_struct_struct> extends FrontendTunerDevice<TunerStatusStructType> {


    public FrontendScanningTunerDevice() {
        super();
    }

    public FrontendScanningTunerDevice(Class<TunerStatusStructType> _genericType) {
        super(_genericType);
    }

    public boolean callDeviceSetTuning(final frontend.FETypes.frontend_tuner_allocation_struct frontend_tuner_allocation, TunerStatusStructType fts, int tuner_id) {
        if (this._has_scanner) {
            return deviceSetTuningScan(frontend_tuner_allocation, frontend_scanner_allocation.getValue(), fts, tuner_id);
        }
        return deviceSetTuning(frontend_tuner_allocation, fts, tuner_id);
    }

    public void checkValidIds(DataType[] capacities) throws InvalidCapacity, InvalidState {
        this._has_scanner = false;
        for (DataType cap : capacities) {
            if (!cap.id.equals("FRONTEND::tuner_allocation") && !cap.id.equals("FRONTEND::listener_allocation") && (!cap.id.equals("FRONTEND::scanner_allocation"))) {
                throw new CF.DevicePackage.InvalidCapacity("Invalid allocation property", capacities);
            }
            if (cap.id.equals("FRONTEND::scanner_allocation")) {
                this._has_scanner = true;
            }
        }
    }

    protected abstract boolean deviceSetTuningScan(final frontend.FETypes.frontend_tuner_allocation_struct request, final frontend.FETypes.frontend_scanner_allocation_struct scan_request, TunerStatusStructType fts, int tuner_id);

    protected StructProperty<frontend.FETypes.frontend_scanner_allocation_struct> frontend_scanner_allocation;

    private boolean _has_scanner = false;

    // this is implemented in the generated base class once all properties are known
    public void loadProperties(){
        frontend_scanner_allocation =
        new StructProperty<frontend.FETypes.frontend_scanner_allocation_struct>(
            "FRONTEND::scanner_allocation", //id
            "frontend_scanner_allocation", //name
            frontend.FETypes.frontend_scanner_allocation_struct.class, //type
            new frontend.FETypes.frontend_scanner_allocation_struct(), //default value
            Mode.READWRITE, //mode
            new Kind[] {Kind.ALLOCATION} //kind
            );

        addProperty(device_kind);
        addProperty(device_model);
        addProperty(frontend_tuner_allocation);
        addProperty(frontend_listener_allocation);
        addProperty(frontend_scanner_allocation);
        addProperty(frontend_tuner_status);
    }
}
