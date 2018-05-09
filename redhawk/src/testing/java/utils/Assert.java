/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
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
package utils;

/**
 * Extended JUnit assertion class that adds an assert for checking that an
 * exception is thrown by an expression.
 */
public class Assert extends org.junit.Assert {
    private Assert()
    {
    }

    public static void assertThrows(Class<? extends Exception> exception, RunnableWithException runnable)
    {
        try {
            runnable.run();
        } catch (Exception exc) {
            assertTrue("expected exception:<"+ exception.getName() +
                        "> but got:<" + exc.getClass().getName() + ">",
                        exception.isInstance(exc));
            return;
        }
        fail("exception not raised");
    }
};
