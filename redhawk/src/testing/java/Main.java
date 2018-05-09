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

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import org.junit.Test;
import org.junit.runner.JUnitCore;
import org.junit.runner.Description;
import org.junit.runner.Result;
import org.junit.runner.Request;

import utils.ChainFilter;
import utils.TestFilter;
import utils.TextListener;

public class Main {

    public static Description getTestDescription(String target) throws ClassNotFoundException, NoSuchMethodException
    {
        // Try to see if it's a class with tests first
        try {
            return getClassDescription(target);
        } catch (ClassNotFoundException exc) {
            // The target might be "<package.>class.method"
        }

        // Split package/class from method name
        int pos = target.lastIndexOf('.');
        if (pos < 0) {
            // No dots, must be an invalid class
            throw new ClassNotFoundException(target);
        }
        String suite = target.substring(0, pos);
        String name = target.substring(pos+1);

        // Class and method lookup may throw exceptions, but it's up to the
        // caller to handle them
        Class<?> clazz = Class.forName(suite);
        clazz.getMethod(name);
        return Description.createTestDescription(clazz, name);
    }

    public static Description getClassDescription(String target) throws ClassNotFoundException
    {
        Class<?> clazz = Class.forName(target);

        // Create a suite description 
        Description desc = Description.createSuiteDescription(clazz);
        for (Method method : clazz.getMethods()) {
            // Find all methods that are annotated as tests
            if (method.getAnnotation(Test.class) != null) {
                desc.addChild(Description.createTestDescription(clazz, method.getName(), method.getAnnotations()));
            }
        }

        return desc;
    }

    public static void main(String[] args) {
        List<String> tests = new ArrayList<>();

        boolean verbose = false;
        Level log_level = null;
        String log_config = null;

        Iterator<String> iter = Arrays.asList(args).iterator();
        while (iter.hasNext()) {
            String arg = iter.next();
            if (arg.startsWith("-")) {
                // Option argument
                if (arg.equals("--log-level")) {
                    log_level = Level.toLevel(iter.next());
                } else if (arg.equals("--log-config")) {
                    log_config = iter.next();
                } else if (arg.equals("-v") || arg.equals("--verbose")) {
                    verbose = true;
                } else {
                    System.err.println("Unrecognized option \"" + arg + "\"");
                    System.exit(1);
                }
            } else {
                // First non-option argument, add remaining arguments to the
                // list of tests
                tests.add(arg);
                while (iter.hasNext()) {
                    tests.add(iter.next());
                }
            }
        }        

        if (log_config != null) {
            PropertyConfigurator.configure(log_config);
        } else {
            BasicConfigurator.configure();
            if (log_level == null) {
                log_level = Level.INFO;
            }
        }

        if (log_level != null) {
            Logger.getRootLogger().setLevel(log_level);
        }

        Request request = Request.aClass(AllTests.class);
        if (!tests.isEmpty()) {
            ChainFilter filter = new ChainFilter();
            for (String test : tests) {
                try {
                    Description desc = getTestDescription(test);
                    filter.addFilter(new TestFilter(desc));
                } catch (ClassNotFoundException|NoSuchMethodException exc) {
                    System.err.println("ERROR: No test '" + test + "'");
                    System.exit(1);
                }
            }
            request = request.filterWith(filter);
        }

        JUnitCore runner = new JUnitCore();
        runner.addListener(new TextListener(verbose));
        Result result = runner.run(request);
        System.exit(result.wasSuccessful() ? 0 : 1);
    }
}
