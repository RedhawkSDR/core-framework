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

import java.util.ArrayList;
import java.util.List;

import org.junit.runner.Description;
import org.junit.runner.manipulation.Filter;

/**
 * JUnit test filter that combines multiple filters, selecting any test that
 * satisfies one of the filters.
 */
public class ChainFilter extends Filter {
    public void addFilter(Filter filter)
    {
        filters.add(filter);
    }

    @Override
    public boolean shouldRun(Description description)
    {
        for (Filter filter : this.filters) {
            if (filter.shouldRun(description)) {
                return true;
            }
        }
        return false;
    }

    @Override
    public String describe()
    {
        String result = "";
        for (Filter filter : this.filters) {
            if (!result.isEmpty()) {
                result = result + ", ";
            }
            result += filter.describe();
        }
        return "[" + result + "]";
    }

    private List<Filter> filters = new ArrayList<>();
}
