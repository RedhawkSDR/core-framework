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
        NumericInCharPortTest.class,
        OutCharPortTest.class,
        NumericInOctetPortTest.class,
        OutOctetPortTest.class,
        NumericInShortPortTest.class,
        OutShortPortTest.class,
        NumericInUShortPortTest.class,
        OutUShortPortTest.class,
        NumericInLongPortTest.class,
        OutLongPortTest.class,
        NumericInULongPortTest.class,
        OutULongPortTest.class,
        NumericInLongLongPortTest.class,
        OutLongLongPortTest.class,
        NumericInULongLongPortTest.class,
        OutULongLongPortTest.class,
        NumericInFloatPortTest.class,
        OutFloatPortTest.class,
        NumericInDoublePortTest.class,
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
        OutSDDSPort_Test.class,
        InFileStreamTest.class,
        InXMLStreamTest.class,
        InBitStreamTest.class,
        InCharStreamTest.class,
        InOctetStreamTest.class,
        InShortStreamTest.class,
        InUShortStreamTest.class,
        InLongStreamTest.class,
        InULongStreamTest.class,
        InLongLongStreamTest.class,
        InULongLongStreamTest.class,
        InFloatStreamTest.class,
        InDoubleStreamTest.class,
        OutXMLStreamTest.class,
        OutFileStreamTest.class,
        OutBitStreamTest.class,
        OutCharStreamTest.class,
        OutOctetStreamTest.class,
        OutShortStreamTest.class,
        OutUShortStreamTest.class,
        OutLongStreamTest.class,
        OutULongStreamTest.class,
        OutLongLongStreamTest.class,
        OutULongLongStreamTest.class,
        OutFloatStreamTest.class,
        OutDoubleStreamTest.class,
        OutVITA49Port_Test.class
})
public class AllTests {
}
