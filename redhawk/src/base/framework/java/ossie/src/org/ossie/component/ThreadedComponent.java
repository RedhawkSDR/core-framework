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

/**
 * Interface describing contract of threaded components and devices.
 */
public interface ThreadedComponent {
    /**
     * Processing is complete; there is no need to invoke service function
     * again.
     */
    int FINISH = -1;

    /**
     * There was no work to do; pause before invoking service function again.
     */
    int NOOP   = 0;

    /**
     * Normal processing occurred; invoke service function again immediately.
     */
    int NORMAL = 1;

    /**
     * Performs a single unit of work.
     *
     * @return status indicating work done (NORMAL, NOOP, FINISH)
     */
    int process ();

    /**
     * Gets the delay between calls to service function after a NOOP.
     *
     * @return current delay, in seconds
     */
    float getThreadDelay ();

    /**
     * Sets the delay between calls to service function after a NOOP.
     *
     * @param delay - new delay, in seconds
     */
    void setThreadDelay (float delay);
}
