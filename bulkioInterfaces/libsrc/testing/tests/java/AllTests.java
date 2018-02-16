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

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

@RunWith(Suite.class)
@SuiteClasses({
        InCharPortTest.class,
        OutCharPortTest.class,
        InOctetPortTest.class,
        OutOctetPortTest.class,
        InShortPortTest.class,
        OutShortPortTest.class,
        InUShortPortTest.class,
        OutUShortPortTest.class,
        InLongPortTest.class,
        OutLongPortTest.class,
        InULongPortTest.class,
        OutULongPortTest.class,
        InLongLongPortTest.class,
        OutLongLongPortTest.class,
        InULongLongPortTest.class,
        OutULongLongPortTest.class,
        InFloatPortTest.class,
        OutFloatPortTest.class,
        InDoublePortTest.class,
        OutDoublePortTest.class,
        InBitPortTest.class,
        OutBitPortTest.class,
        InFilePortTest.class,
        OutFilePortTest.class,
        InXMLPortTest.class,
        OutXMLPortTest.class,
        PrecisionUTCTimeTest.class,
        StreamSRITest.class,
        InSDDSPort_Test.class,
        OutSDDSPort_Test.class
})
public class AllTests {
}
