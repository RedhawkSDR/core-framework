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
package JavaTestDevice.java;

import java.util.Properties;

import org.ossie.properties.Allocator;

/**
 * This is the device code. This file contains the derived class where custom
 * functionality can be added to the device. You may add methods and code to
 * this class to handle property changes, respond to incoming data, and perform
 * general device housekeeping
 *
 * Source: JavaTestDevice.spd.xml
 */
public class JavaTestDevice extends JavaTestDevice_base {
	/**
	 * 	Max value for load_average allocation property.
	 */
	public static final float MAX_LOAD = 4.0f;

    /**
     * This is the device constructor. In this method, you may add additional
     * functionality to properties, such as listening for changes and handling
     * allocation, and set up internal state for your device.
     *
     * A device may listen for external changes to properties (i.e., by a
     * call to configure) using the PropertyListener interface. Listeners are
     * registered by calling addPropertyListener() on the property instance
     * with an object that implements the PropertyListener interface for that
     * data type (e.g., "PropertyListener<Float>" for a float property). More
     * than one listener can be connected to a property.
     *
     *   Example:
     *       // This example makes use of the following properties:
     *       //  - A float value called scaleValue
     *       // The file must import "org.ossie.properties.PropertyListener"
     *
     *       this.scaleValue.addPropertyListener(new PropertyListener<Float>() {
     *           public void valueChanged(Float oldValue, Float newValue) {
     *               logger.debug("Changed scaleValue " + oldValue + " to " + newValue);
     *           }
     *       });
     *
     * The recommended practice is for the implementation of valueChanged() to
     * contain only glue code to dispatch the call to a private method on the
     * device class.
     *
     * Devices may contain allocation properties with "external" action, which
     * are used in capacity allocation and deallocation. In order to support
     * this capability, allocation properties require additional functionality.
     * This is implemented by calling setAllocator() on the property instance
     * with an object that implements the Allocator interface for that data type.
     *
     *   Example:
     *       // This example makes use of the following properties
     *       //  - A struct property called tuner_alloc
     *       // The following methods are defined elsewhere in your class:
     *       //  - private boolean allocate_tuner(tuner_alloc_struct capacity)
     *       //  - private void deallocate_tuner(tuner_alloc_struct capacity)
     *       // The file must import "org.ossie.properties.Allocator"
     *
     *       this.tuner_alloc.setAllocator(new Allocator<tuner_alloc_struct>() {
     *           public boolean allocate(tuner_alloc_struct capacity) {
     *               return allocate_tuner(capacity);
     *           }
     *           public void deallocate(tuner_alloc_struct capacity) {
     *               deallocate_tuner(capacity);
     *           }
     *       });
     *
     * The recommended practice is for the allocate() and deallocate() methods
     * to contain only glue code to dispatch the call to private methods on the
     * device class.
     */
    public JavaTestDevice() {
    	super();
    	
        // Connect device load capacity allocation methods to property
        this.load_average.setAllocator(new Allocator<Float>() {
			@Override
			public boolean allocate(Float capacity) {
				return allocate_load(capacity);
			}

			@Override
			public void deallocate(Float capacity) {
				deallocate_load(capacity);
			}
        });
        
        // Connect device memory allocation methods to memory allocation property. 
        this.memory_allocation.setAllocator(new Allocator<memory_allocation_struct>() {
			@Override
			public boolean allocate(memory_allocation_struct capacity) {
				return allocate_memory(capacity);
			}

			@Override
			public void deallocate(memory_allocation_struct capacity) {
				deallocate_memory(capacity);
			}
        	
        });
    }

    private boolean allocate_memory(memory_allocation_struct capacity) {
    	if (capacity.contiguous.getValue()) {
    		// Pretend that there is never enough contiguous memory.
    		return false;
    	}

    	int requested_mem = capacity.capacity.getValue();

    	// Check that there's enough total memory.
    	long total_mem = this.memory_capacity.getValue();
    	if (requested_mem > total_mem) {
    		return false;
    	}
    	
    	// If request is for shared memory, check that as well.
    	if (capacity.memory_type.getValue() == 1) {
        	int shared_mem = this.shared_memory.getValue();
        	if (requested_mem > shared_mem) {
        		return false;
        	}
        	// Update shared memory capacity.
        	this.shared_memory.setValue(shared_mem - requested_mem);
    	}
    	
    	// Update total memory capacity.
    	this.memory_capacity.setValue(total_mem - requested_mem);
    	
    	return true;
    }
    
    private void deallocate_memory(memory_allocation_struct capacity) {
    	long total_mem = this.memory_capacity.getValue();
    	int released_capacity = capacity.capacity.getValue();
    	this.memory_capacity.setValue(total_mem + released_capacity);
    	
    	if (capacity.memory_type.getValue() == 1) {
    		int shared_mem = this.shared_memory.getValue();
    		this.shared_memory.setValue(shared_mem + released_capacity);
    	}
    }
    
    private boolean allocate_load(float capacity) {
    	float current_load = this.load_average.getValue();
    	if (current_load + capacity > MAX_LOAD) {
    		return false;
    	}
    	this.load_average.setValue(current_load + capacity);
    	return true;
    }
    
    private void deallocate_load(float capacity) {
    	float current_load = this.load_average.getValue();
    	if (capacity > current_load) {
    		throw new ArithmeticException("Attempt to deallocate more load capacity than was allocated");
    	}
    	this.load_average.setValue(current_load - capacity);
    }
    
    /**
     *
     * Main processing function
     *
     * General functionality:
     *
     * The serviceFunction() is called repeatedly by the device's processing
     * thread, which runs independently of the main thread. Each invocation
     * should perform a single unit of work, such as reading and processing one
     * data packet. 
     *
     * The return status of serviceFunction() determines how soon the next
     * invocation occurs:
     *   - NORMAL: the next call happens immediately
     *   - NOOP:   the next call happens after a pre-defined delay (100 ms)
     *   - FINISH: no more calls occur
     *
     * StreamSRI:
     *    To create a StreamSRI object, use the following code:
     *            String stream_id = "testStream";
     *            BULKIO.StreamSRI sri = new BULKIO.StreamSRI();
     *            sri.mode = 0;
     *            sri.xdelta = 0.0;
     *            sri.ydelta = 1.0;
     *            sri.subsize = 0;
     *            sri.xunits = 1; // TIME_S
     *            sri.streamID = (stream_id != null) ? stream_id : "";
     *
     * PrecisionUTCTime:
     *    To create a PrecisionUTCTime object, use the following code:
     *            BULKIO.PrecisionUTCTime tstamp = bulkio.time.utils.now();
     *
     * Ports:
     *
     *    Each port instance is accessed through members of the following form:
     *
     *        this.port_<PORT NAME>
     *
     *    Input BULKIO data is obtained by calling getPacket on the provides
     *    port. The getPacket method takes one argument: the time to wait for
     *    data to arrive, in milliseconds. A timeout of 0 causes getPacket to
     *    return immediately, while a negative timeout indicates an indefinite
     *    wait. If no data is queued and no packet arrives during the waiting
     *    period, getPacket returns null.
     *
     *    Output BULKIO data is sent by calling pushPacket on the uses port. In
     *    the case of numeric data, the pushPacket method takes a primitive
     *    array (e.g., "float[]"), a timestamp, an end-of-stream flag and a
     *    stream ID. You must make at least one call to pushSRI to associate a
     *    StreamSRI with the stream ID before calling pushPacket, or receivers
     *    may drop the data.
     *
     *    When all processing on a stream is complete, a call should be made to
     *    pushPacket with the end-of-stream flag set to "true".
     *
     *    Interactions with non-BULKIO ports are left up to the discretion of
     *    the device developer.
     *
     * Properties:
     *
     *    Properties are accessed through members of the same name; characters
     *    that are invalid for a Java identifier are replaced with "_". The
     *    current value of the property is read with getValue and written with
     *    setValue:
     *
     *        float val = this.float_prop.getValue();
     *        ...
     *        this.float_prop.setValue(1.5f);
     *
     *    Primitive data types are stored using the corresponding Java object
     *    wrapper class. For example, a property of type "float" is stored as a
     *    Float. Java will automatically box and unbox primitive types where
     *    appropriate.
     *
     *    Numeric properties support assignment via setValue from any numeric
     *    type. The standard Java type coercion rules apply (e.g., truncation
     *    of floating point values when converting to integer types).
     *
     * Example:
     *
     *    This example assumes that the device has two ports:
     *        - A bulkio.InShortPort provides (input) port called dataShort_in
     *        - A bulkio.InFloatPort uses (output) port called dataFloat_out
     *    The mapping between the port and the class is found in the device
     *    base class file.
     *    This example also makes use of the following Properties:
     *        - A float value called amplitude with a default value of 2.0
     *        - A boolean called increaseAmplitude with a default value of true
     *
     *    InShortPort.Packet data = this.port_dataShort_in.getPacket(125);
     *
     *    if (data != null) {
     *        float[] outData = new float[data.getData().length];
     *        for (int i = 0; i < data.getData().length; i++) {
     *            if (this.increaseAmplitude.getValue()) {
     *                outData[i] = (float)data.getData()[i] * this.amplitude.getValue();
     *            } else {
     *                outData[i] = (float)data.getData()[i];
     *            }
     *        }
     *
     *        // NOTE: You must make at least one valid pushSRI call
     *        if (data.sriChanged()) {
     *            this.port_dataFloat_out.pushSRI(data.getSRI());
     *        }
     *        this.port_dataFloat_out.pushPacket(outData, data.getTime(), data.getEndOfStream(), data.getStreamID());
     *    }
     *
     */
    protected int serviceFunction() {
        logger.debug("serviceFunction() example log message");

        return NOOP;
    }

    /**
     * Set additional options for ORB startup. For example:
     *
     *   orbProps.put("com.sun.CORBA.giop.ORBFragmentSize", Integer.toString(fragSize));
     *
     * @param orbProps
     */
    public static void configureOrb(final Properties orbProps) {
    }
}
