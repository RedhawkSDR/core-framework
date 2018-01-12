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

import frontend.DigitalTunerDelegate;
import FRONTEND.FrontendException;
import FRONTEND.BadParameterException;
import FRONTEND.NotSupportedException;
import FRONTEND.ScanningTunerPackage.ScanStatus;
import FRONTEND.ScanningTunerPackage.ScanStrategy;
import BULKIO.PrecisionUTCTime;

public interface DigitalScanningTunerDelegate extends DigitalTunerDelegate {

    public FRONTEND.ScanningTunerPackage.ScanStatus getScanStatus(String id) throws FrontendException, BadParameterException, NotSupportedException;

    public void setScanStartTime(String id, BULKIO.PrecisionUTCTime start_time) throws FrontendException, BadParameterException, NotSupportedException;

    public void setScanStrategy(String id, FRONTEND.ScanningTunerPackage.ScanStrategy scan_strategy) throws FrontendException, BadParameterException, NotSupportedException;
}
