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

package helpers;

import bulkio.InDataPort;
import bulkio.OutDataPort;

/**
 * Test helper to provide a generic interface for input and output port tests.
 *
 * Each port type pair should have a concrete implementation.
 */
public interface TestHelper<E extends BULKIO.updateSRIOperations,A> {
    /**
     * Returns the base name of the interface (e.g., "dataFloat").
     */
    public String getName();

    /**
     * Returns the size of a single element (sample, character, etc.) in bits.
     */
    public int bitsPerElement();

    /**
     * Creates an input port.
     */
    public InDataPort<E,A> createInPort(String name);

    /**
     * Creates an output port.
     */
    public OutDataPort<E,A> createOutPort(String name);

    /**
     * Creates a test CORBA stub that can be used for testing output ports.
     */
    public stubs.Stub<A> createStub();

    /**
     * Returns the length of packet data.
     *
     * Abstracts the differences between the packet data types (arrays, string,
     * bit sequence).
     */
    public int dataLength(A data);

    /**
     * Creates uninitialized packet data of the given size.
     *
     * Abstracts the differences between the packet data types (arrays, string,
     * bit sequence).
     */
    public A makeData(int length);

    /**
     * Returns the CORBA interface for an input port, for use in testing the
     * external CORBA API.
     */
    public E toCorbaType(InDataPort<E,A> port);

    /**
     * Inject a test packet into an input port.
     *
     * Abstracts the differences between XML and other ports.
     */
    public void pushTestPacket(InDataPort<E,A> port, int length, BULKIO.PrecisionUTCTime time, boolean eos, String streamID);

    /**
     * Send a test packet through an output port.
     *
     * Abstracts the differences between XML and other ports.
     */
    public void pushTestPacket(OutDataPort<E,A> port, int length, BULKIO.PrecisionUTCTime time, boolean eos, String streamID);
}
