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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import org.ossie.events.MessageSupplierPort;
import org.ossie.events.MessageConsumerPort;
import org.ossie.events.MessageListener;
import org.ossie.properties.*;
import org.omg.CORBA.ORB;
import org.omg.CORBA.Any;

import utils.Assert;

@RunWith(JUnit4.class)
public class PropertyTest {


    @Test
    public void testSetBoolean()
    {
        Any bool_any = ORB.init().create_any();
        bool_any.insert_boolean(true);

        Any bad_string_any = ORB.init().create_any();
        bad_string_any.insert_string("hello");

        Any good_string_any = ORB.init().create_any();
        good_string_any.insert_string("true");

        BooleanProperty boolprop = new BooleanProperty("my_id", "my_name", false, Mode.READWRITE, Action.EXTERNAL, new Kind[] {Kind.PROPERTY});
        boolprop.configure(bool_any);
        Boolean retval = boolprop.getValue();
        Assert.assertEquals(retval, true);

        boolprop.configure(bad_string_any);
        retval = boolprop.getValue();
        Assert.assertEquals(retval, false);

        boolprop.configure(good_string_any);
        retval = boolprop.getValue();
        Assert.assertEquals(retval, true);
    }
}
