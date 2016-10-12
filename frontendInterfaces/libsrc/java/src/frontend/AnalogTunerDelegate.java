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

    public void fe_setTunerCenterFrequency(String id, double freq) throws FrontendException, BadParameterException, NotSupportedException;

    public double fe_getTunerCenterFrequency(String id) throws FrontendException, BadParameterException, NotSupportedException;

    public void fe_setTunerBandwidth(String id, double bw) throws FrontendException, BadParameterException, NotSupportedException;

    public double fe_getTunerBandwidth(String id) throws FrontendException, BadParameterException, NotSupportedException;

    public void fe_setTunerAgcEnable(String id, boolean enable) throws FrontendException, BadParameterException, NotSupportedException;

    public boolean fe_getTunerAgcEnable(String id) throws FrontendException, BadParameterException, NotSupportedException;

    public void fe_setTunerGain(String id, float gain) throws FrontendException, BadParameterException, NotSupportedException;

    public float fe_getTunerGain(String id) throws FrontendException, BadParameterException, NotSupportedException;

    public void fe_setTunerReferenceSource(String id, int source) throws FrontendException, BadParameterException, NotSupportedException;

    public int fe_getTunerReferenceSource(String id) throws FrontendException, BadParameterException, NotSupportedException;

    public void fe_setTunerEnable(String id, boolean enable) throws FrontendException, BadParameterException, NotSupportedException;

    public boolean fe_getTunerEnable(String id) throws FrontendException, BadParameterException, NotSupportedException;
}
