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


#include <iostream>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>

#include "ossie/ExecutableDevice_impl.h"


//vector<CF::ExecutableDevice::ProcessID_Type> ExecutableDevice_impl::PID;
PREPARE_LOGGING(ExecutableDevice_impl)


/* ExecutableDevice_impl ****************************************
    - constructor 1: no capacities defined
****************************************************************** */
ExecutableDevice_impl::ExecutableDevice_impl (char* devMgr_ior, char* id, char* lbl, char* sftwrPrfl):
    LoadableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl)
{
}



/* ExecutableDevice_impl ************************************************
    - constructor 2: capacities defined
******************************************************************** */
ExecutableDevice_impl::ExecutableDevice_impl (char* devMgr_ior, char* id, char* lbl, char* sftwrPrfl,
                                              CF::Properties capacities):
    LoadableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl, capacities)
{
}

/* ExecutableDevice_impl ****************************************
    - constructor 1: no capacities defined
****************************************************************** */
ExecutableDevice_impl::ExecutableDevice_impl (char* devMgr_ior, char* id, char* lbl, char* sftwrPrfl, 
                                              char* composite_ior):
    LoadableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl, composite_ior)
{
}


/* ExecutableDevice_impl ************************************************
    - constructor 2: capacities defined
******************************************************************** */
ExecutableDevice_impl::ExecutableDevice_impl (char* devMgr_ior, char* id, char* lbl, char* sftwrPrfl,
                                              CF::Properties capacities, char* composite_ior):
    LoadableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl, capacities, composite_ior)
{
}


/* execute *****************************************************************
    - executes a process on the device
************************************************************************* */
CF::ExecutableDevice::ProcessID_Type ExecutableDevice_impl::execute (
    const char* name, 
    const CF::Properties& options, 
    const CF::Properties& parameters) 
        throw (CORBA::SystemException, 
                CF::Device::InvalidState, 
                CF::ExecutableDevice::InvalidFunction, 
                CF::ExecutableDevice::InvalidParameters, 
                CF::ExecutableDevice::InvalidOptions, 
                CF::InvalidFileName, 
                CF::ExecutableDevice::ExecuteFail)
{
    CORBA::TypeCode_var tc;                       // CORBA type code
    const char* tempStr;                          // temporary character string
    CF::ExecutableDevice::ProcessID_Type PID;     // process ID
    int size = 2 * parameters.length () + 2;      // length of the POSIX argv arguments
    char** argv = new char *[size];               // POSIX arguments
    CORBA::ULong stackSize, priority;             // CORBA unsigned longs for options storage

    // verify device is in the correct state
    if (!isUnlocked () || isDisabled ()) {
        DEBUG(5, ExecutableDevice_impl, "Cannot execute. System is either LOCKED, SHUTTING DOWN or DISABLED.")
        throw (CF::Device::InvalidState("Cannot execute. System is either LOCKED, SHUTTING DOWN or DISABLED."));
    }

    priority = 0;                                 // this is the default value for the priority (it's actually meaningless in this version)
    stackSize = 4096;                             // this is the default value for the stacksize (it's actually meaningless in this version)

    {
    // verify valid options, both STACK_SIZE_ID and PRIORITY_ID must have unsigned-long types
    CF::Properties invalidOptions;
    invalidOptions.length(0);

    for (unsigned i = 0; i < options.length (); i++) {
        tc = options[i].value.type ();

        // extract priority and stack size from the options list
        if (strcmp (options[i].id, CF::ExecutableDevice::PRIORITY_ID)) {
            if (tc->kind () == CORBA::tk_ulong) {
                options[i].value >>= priority; 
            } else {
                LOG_ERROR(ExecutableDevice_impl, "Incorrect type provided for option PRIORITY");
                invalidOptions.length(invalidOptions.length() + 1);
                invalidOptions[invalidOptions.length() - 1] = options[i];
            }
        } else if (strcmp (options[i].id, CF::ExecutableDevice::STACK_SIZE_ID)) {
            if (tc->kind () == CORBA::tk_ulong) {
                options[i].value >>= stackSize;
            } else {
                LOG_ERROR(ExecutableDevice_impl, "Incorrect type provided for option STACK_SIZE");
                invalidOptions.length(invalidOptions.length() + 1);
                invalidOptions[invalidOptions.length() - 1] = options[i];
            }
        } else {
            LOG_ERROR(ExecutableDevice_impl, "Unsupported option " << options[i].id << " provided ");
            invalidOptions.length(invalidOptions.length() + 1);
            invalidOptions[invalidOptions.length() - 1] = options[i];
        }
    }

    if (invalidOptions.length() > 0) {
        throw CF::ExecutableDevice::InvalidOptions(invalidOptions);
    }
    }

// convert input parameters to the POSIX standard argv format

    char* name_temp = new char[strlen (name) + 1];
    if (name[0] == '/')
        { strcpy (name_temp, &name[1]); }
    else
        { strcpy (name_temp, name); }
    argv[0] = name_temp;
    DEBUG(5, ExecutableDevice_impl, "Executing " << argv[0]) {
        for (unsigned int i = 0; i < parameters.length (); i++) {
            name_temp = new char[strlen (parameters[i].id) + 1];
            strcpy (name_temp, parameters[i].id);
            argv[2 * i + 1] = name_temp;

            std::string interimTempStr = ossie::any_to_string(parameters[i].value);
            tempStr = interimTempStr.c_str();
            name_temp = new char[strlen (tempStr) + 1];
            strcpy (name_temp, tempStr);
            argv[2 * i + 2] = name_temp;
        }
        argv[size-1] = NULL;
    }

// execute file or function, pass arguments and options and execute, handler is returned
    CF::ExecutableDevice::ProcessID_Type new_pid;

    if ((new_pid = fork()) < 0) {
        DEBUG(5, ExecutableDevice_impl, "Error executing fork()")
        return((CF::ExecutableDevice::ProcessID_Type) - 1);
    }

    if (new_pid > 0) {

// in parent process
        PID = new_pid;
    } else {
// in child process
        if (getenv("VALGRIND")) {
            char* new_argv[24];
            strcpy(new_argv[0], "/usr/local/bin/valgrind");
            string logFile = "--log-file=";
            logFile += argv[0];
            new_argv[1] = (char*)logFile.c_str();
            unsigned int i;
            for (i = 2; argv[i-2]; i++) {
                new_argv[i] = argv[i-2];
            }
            new_argv[i] = NULL;
            DEBUG(3, ExecutableDevice_impl, "chmod'ing " << new_argv[0])
            struct stat stat_buf;
            stat(new_argv[0], &stat_buf);
            chmod(new_argv[0], stat_buf.st_mode | S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            DEBUG(3, ExecutableDevice_impl, "execv'ing " << new_argv[0])
            execv(new_argv[0], new_argv);
        } else {
            DEBUG(3, ExecutableDevice_impl, "chmod'ing " << argv[0])
            DEBUG(3, ExecutableDevice_impl, "execv'ing " << argv[0])
            struct stat stat_buf;
            stat(argv[0], &stat_buf);
            chmod(argv[0], stat_buf.st_mode | S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            execv(argv[0], argv);
        }

        DEBUG(5, ExecutableDevice_impl, strerror(errno))
        exit(-1);                                 // If we ever get here the program we tried to start and failed
    }

    {
        for (int i = 0; i < size; i++) {          /// \bug Does this leak memory when executeProcess Fails?
            delete argv[i];
        }
    }
    delete argv;

// create a PID and return it
    return (PID);
}


/* terminate ***********************************************************
    - terminates a process on the device
******************************************************************* */
void
ExecutableDevice_impl::terminate (CF::ExecutableDevice::ProcessID_Type processId) throw (CORBA::SystemException, CF::ExecutableDevice::InvalidProcess, CF::Device::InvalidState)
{

// validate device state
    if (isLocked () || isDisabled ()) {
        printf ("Cannot terminate. System is either LOCKED or DISABLED.");
        throw (CF::Device::
               InvalidState
               ("Cannot terminate. System is either LOCKED or DISABLED."));
    }

// go ahead and terminate the process
    int status;
    kill(processId, SIGKILL);
    waitpid(processId, &status, 0);
}


void  ExecutableDevice_impl::configure (const CF::Properties& capacities)
throw (CF::PropertySet::PartialConfiguration, CF::PropertySet::
       InvalidConfiguration, CORBA::SystemException)
{
    Device_impl::configure(capacities);
}

