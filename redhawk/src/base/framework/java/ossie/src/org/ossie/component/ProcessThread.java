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

import org.apache.log4j.Logger;

class ProcessThread implements Runnable {

    public ProcessThread (ThreadedComponent target)
    {
        this.target = target;
        // Try to use the most relevant logger for the target
        if (target instanceof Device) {
            logger = Device.logger;
        } else if (target instanceof Resource) {
            logger = Resource.logger;
        } else {
            logger = Logger.getLogger(ProcessThread.class.getName());
        }
    }

    public void run ()
    {
        while (this.isRunning()) {
            int state = ThreadedComponent.NORMAL;
            try {
                state = this.target.process();
            } catch (Throwable exc) {
                logger.fatal("Unhandled exception in service function: " + exc.getMessage());
                exc.printStackTrace();
                // Terminate the process on unhandled exceptions
                System.exit(-1);
            }
            if (state == ThreadedComponent.FINISH) {
                return;
            } else if (state == ThreadedComponent.NOOP) {
                try {
                    Thread.sleep(this.delay);
                } catch (final InterruptedException ex) {
                    return;
                }
            }
        }
    }

    public void start ()
    {
        this.running = true;
    }

    public synchronized void stop ()
    {
        this.running = false;
    }

    public synchronized boolean isRunning ()
    {
        return this.running;
    }

    public float getDelay ()
    {
        return this.delay / 1000.0f;
    }

    public void setDelay (float delay)
    {
        this.delay = (long)(delay*1000);
    }

    private ThreadedComponent target;
    private boolean running = true;
    private long delay = 125;
    private Logger logger;
}
