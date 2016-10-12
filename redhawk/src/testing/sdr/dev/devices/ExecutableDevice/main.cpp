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

#include <string>
#include <cstdlib>
#include <signal.h>
#include <sys/wait.h>

#include <ossie/ExecutableDevice_impl.h>

class TestDevice : public virtual ExecutableDevice_impl {
    public:
    TestDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
          ExecutableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl)
    {
        loadProperties();
    }

    TestDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
          ExecutableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl)
    {
        loadProperties();
    }

    ~TestDevice()
    {
    };

private:
    void loadProperties() {
        addProperty(LD_LIBRARY_PATH,
                    "LD_LIBRARY_PATH",
                    "",
                    "readonly",
                    "",
                    "external",
                    "configure");

        addProperty(PYTHONPATH,
                    "PYTHONPATH",
                    "",
                    "readonly",
                    "",
                    "external",
                    "configure");

        addProperty(CLASSPATH,
                    "CLASSPATH",
                    "",
                    "readonly",
                    "",
                    "external",
                    "configure");

        addProperty(OCTAVE_PATH,
                    "OCTAVE_PATH",
                    "",
                    "readonly",
                    "",
                    "external",
                    "configure");

        setPropertyQueryImpl(LD_LIBRARY_PATH, this, &TestDevice::get_LD_LIBRARY_PATH);
        setPropertyQueryImpl(PYTHONPATH, this, &TestDevice::get_PYTHONPATH);
        setPropertyQueryImpl(CLASSPATH, this, &TestDevice::get_CLASSPATH);
        setPropertyQueryImpl(OCTAVE_PATH, this, &TestDevice::get_OCTAVE_PATH);
    }

    std::string getEnvVar(const char* name) {
        std::string result;
        const char* value = getenv(name);
        if (value) {
            result = value;
        }
        return result;
    }

    std::string get_LD_LIBRARY_PATH()
    {
        return getEnvVar("LD_LIBRARY_PATH");
    }

    std::string get_PYTHONPATH()
    {
        return getEnvVar("PYTHONPATH");
    }

    std::string get_CLASSPATH()
    {
        return getEnvVar("CLASSPATH");
    }

    std::string get_OCTAVE_PATH()
    {
        return getEnvVar("OCTAVE_PATH");
    }

    std::string LD_LIBRARY_PATH;
    std::string PYTHONPATH;
    std::string CLASSPATH;
    std::string OCTAVE_PATH;
};

CREATE_LOGGER(ExecutableTestDevice)

TestDevice *testDevice;

void sigchld_catcher( int sig ) {
    int status;
    pid_t child_pid;

    while( (child_pid = waitpid(-1, &status, WNOHANG)) > 0 ) {
    }
}

void signal_catcher( int sig )
{
    // IMPORTANT Don't call exit(...) in this function
    // issue all CORBA calls that you need for cleanup here before calling ORB shutdown
    LOG_DEBUG(ExecutableTestDevice, "Terminate signal " << sig << " received")
    if (testDevice) {
      testDevice->halt();
    } else {
      LOG_DEBUG(ExecutableTestDevice, "Device not instantiated")
    }
    LOG_DEBUG(ExecutableTestDevice, "Device halted")
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    sa.sa_handler = sigchld_catcher;
    sa.sa_flags = 0;

    // We need to catch sigchld 
    if( sigaction( SIGCHLD, &sa, NULL ) == -1 ) {
        exit(EXIT_FAILURE);
    }

    sa.sa_handler = signal_catcher;
    sa.sa_flags = 0;
    Device_impl::start_device(&testDevice, sa, argc, argv);
}
