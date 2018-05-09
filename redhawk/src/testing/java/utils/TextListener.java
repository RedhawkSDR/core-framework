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

import java.io.PrintStream;
import java.text.NumberFormat;

import org.junit.runner.Description;
import org.junit.runner.Result;
import org.junit.runner.notification.Failure;
import org.junit.runner.notification.RunListener;

/**
 * JUnit RunListener to provide similar output to CppUnit and Python: mainly,
 * printing the name of each test as it runs with verbose mode enabled.
 */
public class TextListener extends RunListener {
    public TextListener(boolean verbose)
    {
        this.verbose = verbose;
        this.stream = System.out;
        this.testPassed = false;
    }

    public void testRunFinished(Result result)
    {
        stream.println();
        stream.println("Time: " + elapsedTimeAsString(result.getRunTime()));

        for (Failure failure : result.getFailures()) {
            stream.println(failure.getTestHeader());
            stream.println(failure.getTrace());
        }

        if (result.wasSuccessful()) {
            stream.println("OK (" + result.getRunCount() + " tests)");
        } else {
            stream.println("FAILURES!!!");
            stream.println("Tests run: " + result.getRunCount() + ", Failures: " + result.getFailureCount());
        }
    }

    public void testStarted(Description description)
    {
        if (verbose) {
            stream.print(description.getDisplayName() + " : ");
        } else {
            stream.print(".");
        }
        testPassed = true;
    }

    public void testIgnored(Description description)
    {
        if (verbose) {
            stream.print("IGNORED");
        } else {
            stream.print("I");
        }
        testPassed = false;
    }

    public void testFailure(Failure failure)
    {
        if (verbose) {
            stream.print("FAILED");
        } else {
            stream.print("F");
        }
        testPassed = false;
    }

    public void testFinished(Description description)
    {
        if (verbose) {
            if (testPassed) {
                stream.print("OK");
            }
            stream.println();
        }
    }

    protected String elapsedTimeAsString(long runTime) {
        return NumberFormat.getInstance().format((double) runTime / 1000);
    }

    private boolean verbose;
    private PrintStream stream;
    private boolean testPassed;
} 
