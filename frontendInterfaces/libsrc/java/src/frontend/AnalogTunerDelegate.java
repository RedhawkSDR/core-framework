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

import frontend.FrontendTunerDelegate;
import FRONTEND.FrontendException;
import FRONTEND.BadParameterException;
import FRONTEND.NotSupportedException;

public interface AnalogTunerDelegate extends FrontendTunerDelegate {

    public void setTunerCenterFrequency(String id, double freq) throws FrontendException, BadParameterException, NotSupportedException;

    public double getTunerCenterFrequency(String id) throws FrontendException, BadParameterException, NotSupportedException;

    public void setTunerBandwidth(String id, double bw) throws FrontendException, BadParameterException, NotSupportedException;

    public double getTunerBandwidth(String id) throws FrontendException, BadParameterException, NotSupportedException;

    public void setTunerAgcEnable(String id, boolean enable) throws FrontendException, BadParameterException, NotSupportedException;

    public boolean getTunerAgcEnable(String id) throws FrontendException, BadParameterException, NotSupportedException;

    public void setTunerGain(String id, float gain) throws FrontendException, BadParameterException, NotSupportedException;

    public float getTunerGain(String id) throws FrontendException, BadParameterException, NotSupportedException;

    public void setTunerReferenceSource(String id, int source) throws FrontendException, BadParameterException, NotSupportedException;

    public int getTunerReferenceSource(String id) throws FrontendException, BadParameterException, NotSupportedException;

    public void setTunerEnable(String id, boolean enable) throws FrontendException, BadParameterException, NotSupportedException;

    public boolean getTunerEnable(String id) throws FrontendException, BadParameterException, NotSupportedException;
}
