/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
package bulkio;

import java.util.concurrent.locks.*;

class queueSemaphore
{
    private   int           maxValue;
    private   int           currValue;
    private   Lock          mutex = new ReentrantLock(); 
    private   Condition     condition = mutex.newCondition();

    public queueSemaphore( int initialMaxValue) {
            maxValue = initialMaxValue;
    }

    public void release() {
	  currValue=0;
	  condition.signalAll();
    }

    public void setMaxValue( int newMaxValue) {
	mutex.lock();
	try {
	    maxValue = newMaxValue;
	}
	finally {
	    mutex.unlock();
	}
    }

    public int getMaxValue() {
	mutex.lock();
	try {
	    return maxValue;
	}
	finally {
	    mutex.unlock();
	}
    }

    public void setCurrValue( int newValue) {
	mutex.lock();
	try {
            if (newValue < maxValue) {
                int oldValue = currValue;
                currValue = newValue;

                if (oldValue > newValue) {
                    condition.signal();
                }
            }

	}
	finally {
	    mutex.unlock();
	}
    }

    public void incr() {
	mutex.lock();
	try {
            while (currValue >= maxValue) {
		try {
		    condition.await();
		}catch( InterruptedException e) {};
		
            }
            ++currValue;
	}
	finally {
	    mutex.unlock();
	}
    }

    public void decr() {
	mutex.lock();
	try {
            if (currValue > 0) {
                --currValue;
            }
            condition.signal();
	}
	finally {
	    mutex.unlock();
	}
    }

};
