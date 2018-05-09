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

import org.junit.runner.Description;
import org.junit.runner.manipulation.Filter;

/**
 * JUnit test filter that selects a single test, or a suite of tests from a
 * single class.
 */
public class TestFilter extends Filter {
    public TestFilter(Description description)
    {
        test = description;
    }

    @Override
    public boolean shouldRun(Description description)
    {
        // Suite-to-suite or test-to-test comparison
        if (test.equals(description)) {
            return true;
        }
        if (description.isTest()) {
            for (Description child : test.getChildren()) {
                if (child.equals(description)) {
                    return true;
                }
            }
        } else {
            for (Description child : description.getChildren()) {
                if (shouldRun(child)) {
                    return true;
                }
            }
        }
        return false;
    }

    @Override
    public String describe()
    {
        if (test.isTest()) {
            return "Method " + test.getDisplayName();
        } else {
            return "Class " + test.getDisplayName();
        }
    }

    private Description test;
}
