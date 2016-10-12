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

package org.ossie.component;

public abstract class ThreadedDevice extends Device implements ThreadedComponent {

    public ThreadedDevice ()
    {
        super();

        this._processThread = new ProcessThread(this);
    }

    public void run ()
    {
        this._processThread.run();
    }

    public void start () throws CF.ResourcePackage.StartError
    {
        this._processThread.start();
        super.start();
    }

    public void stop () throws CF.ResourcePackage.StopError
    {
        this._processThread.stop();
        super.stop();
    }

    public int process ()
    {
        return this.serviceFunction();
    }

    public float getThreadDelay ()
    {
        return this._processThread.getDelay();
    }

    public void setThreadDelay (float delay)
    {
        this._processThread.setDelay(delay);
    }

    protected abstract int serviceFunction ();

    private ProcessThread _processThread;
}
