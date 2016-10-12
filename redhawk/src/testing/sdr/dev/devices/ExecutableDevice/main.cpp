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
#include <string>
#include <cstdlib>
#include "ossie/debug.h"
#include "ossie/ossieSupport.h"

#include "ossie/CF/AggregateDevices.h"
#include "ossie/ExecutableDevice_impl.h"
#include "ossie/debug.h"

class TestDevice : public virtual CF::AggregateExecutableDevice, public virtual ExecutableDevice_impl {
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

    TestDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
          ExecutableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl, capacities)
    {
        loadProperties();
    }

    TestDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
          ExecutableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl, capacities)
    {
        loadProperties();
    }

    ~TestDevice()
    {
    };

    void addDevice(CF::Device_ptr associatedDevice) {
    }

    void removeDevice(CF::Device_ptr associatedDevice) {
    }

    virtual void query(CF::Properties& configProperties) throw (CF::UnknownProperties, CORBA::SystemException)
    {
        resync();
        ExecutableDevice_impl::query(configProperties);
    }

    CF::DeviceSequence* devices() {
        return 0;
    }

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
    }

    std::string getEnvVar(const char* name) {
        std::string result;
        const char* value = getenv(name);
        if (value) {
            result = value;
        }
        return result;
    }

    void resync() {
        LD_LIBRARY_PATH = getEnvVar("LD_LIBRARY_PATH");
        PYTHONPATH = getEnvVar("PYTHONPATH");
        CLASSPATH = getEnvVar("CLASSPATH");
    }

    std::string LD_LIBRARY_PATH;
    std::string PYTHONPATH;
    std::string CLASSPATH;
};

CREATE_LOGGER(ExecutableTestDevice)

TestDevice *testDevice;

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
    sa.sa_handler = signal_catcher;
    sa.sa_flags = 0;

    Device_impl::start_device(&testDevice, sa, argc, argv);
}
