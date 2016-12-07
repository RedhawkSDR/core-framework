/**************************************************************************

    This is the device code. This file contains the child class where
    custom functionality can be added to the device. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "LongDevice.h"

#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/time_duration.hpp>
#include <boost/thread/pthread/thread_data.hpp>


PREPARE_LOGGING(LongDevice_i)

LongDevice_i::LongDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
    LongDevice_base(devMgr_ior, id, lbl, sftwrPrfl)
{
}

LongDevice_i::LongDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    LongDevice_base(devMgr_ior, id, lbl, sftwrPrfl, compDev)
{
}

LongDevice_i::LongDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    LongDevice_base(devMgr_ior, id, lbl, sftwrPrfl, capacities)
{
}

LongDevice_i::LongDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    LongDevice_base(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev)
{
}

LongDevice_i::~LongDevice_i()
{
   if ( delay == 0 ) {
         condition.notify_all();
   }

}

void LongDevice_i::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    if ( delay == 0 ) {
        RH_NL_INFO("LongDevice", "Stopping the wait.......");
        condition.notify_all();
    }
    LongDevice_base::stop();
}


void LongDevice_i::_delay( const std::string &from ) {
    if ( !disable_delay ) {
        if ( delay == 0 ) {
            // block for ever on condition
            boost::mutex::scoped_lock dlock(delay_mutex);
            condition.wait(dlock);
        }
        else {
            int32_t d =  ( delay < 0 ) ? 3000 : delay;
            RH_NL_DEBUG("LongDevice", "delay before return " << delay << " from " << from);
            boost::this_thread::sleep( boost::posix_time::milliseconds( d ) );
        }

    }
}

char* LongDevice_i::identifier () throw (CORBA::SystemException) {
    _delay("identifier");
    return Resource_impl::identifier();
}


char* LongDevice_i::softwareProfile ()  throw (CORBA::SystemException)  {
    _delay("softwareProfile");
    return Resource_impl::softwareProfile();
}

CORBA::Boolean LongDevice_i::started () throw (CORBA::SystemException)  {
    _delay("started");
    return Resource_impl::started();
}

CF::Device::UsageType LongDevice_i::usageState () throw (CORBA::SystemException)  {
    _delay("usageState");
    return Device_impl::usageState();
}
CF::Device::AdminType LongDevice_i::adminState ()  throw (CORBA::SystemException) {
    _delay("adminstate");
    return Device_impl::adminState();
}

CF::Device::OperationalType LongDevice_i::operationalState () throw (CORBA::SystemException)  {
    _delay("operatinalState");
    return Device_impl::operationalState();
}

char * LongDevice_i::label()  throw (CORBA::SystemException) {
    _delay("label");
    return Device_impl::label();
}

CORBA::Boolean LongDevice_i::allocateCapacity(const CF::Properties &props )
 throw (CF::Device::InvalidState, CF::Device::InvalidCapacity, CF::Device::InsufficientCapacity, CORBA::SystemException)
{
    _delay("allocateCapacity");
    return Device_impl::allocateCapacity(props);
}

void LongDevice_i::deallocateCapacity(const CF::Properties &props)
    throw (CF::Device::InvalidState, CF::Device::InvalidCapacity, CORBA::SystemException) {
    _delay("deallocateCapacity");
    return Device_impl::deallocateCapacity(props);
}

void LongDevice_i::constructor()
{
    /***********************************************************************************
     This is the RH constructor. All properties are properly initialized before this function is called
    ***********************************************************************************/
}

/**************************************************************************

    This is called automatically after allocateCapacity or deallocateCapacity are called.
    Your implementation should determine the current state of the device:

       setUsageState(CF::Device::IDLE);   // not in use
       setUsageState(CF::Device::ACTIVE); // in use, with capacity remaining for allocation
       setUsageState(CF::Device::BUSY);   // in use, with no capacity remaining for allocation

**************************************************************************/
void LongDevice_i::updateUsageState()
{
}

/***********************************************************************************************

    Basic functionality:

        The service function is called by the serviceThread object (of type ProcessThread).
        This call happens immediately after the previous call if the return value for
        the previous call was NORMAL.
        If the return value for the previous call was NOOP, then the serviceThread waits
        an amount of time defined in the serviceThread's constructor.

    SRI:
        To create a StreamSRI object, use the following code:
                std::string stream_id = "testStream";
                BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);

    Time:
        To create a PrecisionUTCTime object, use the following code:
                BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();


    Ports:

        Data is passed to the serviceFunction through by reading from input streams
        (BulkIO only). The input stream class is a port-specific class, so each port
        implementing the BulkIO interface will have its own type-specific input stream.
        UDP multicast (dataSDDS and dataVITA49) and string-based (dataString, dataXML and
        dataFile) do not support streams.

        The input stream from which to read can be requested with the getCurrentStream()
        method. The optional argument to getCurrentStream() is a floating point number that
        specifies the time to wait in seconds. A zero value is non-blocking. A negative value
        is blocking.  Constants have been defined for these values, bulkio::Const::BLOCKING and
        bulkio::Const::NON_BLOCKING.

        More advanced uses of input streams are possible; refer to the REDHAWK documentation
        for more details.

        Input streams return data blocks that automatically manage the memory for the data
        and include the SRI that was in effect at the time the data was received. It is not
        necessary to delete the block; it will be cleaned up when it goes out of scope.

        To send data using a BulkIO interface, create an output stream and write the
        data to it. When done with the output stream, the close() method sends and end-of-
        stream flag and cleans up.

        NOTE: If you have a BULKIO dataSDDS or dataVITA49  port, you must manually call
              "port->updateStats()" to update the port statistics when appropriate.

        Example:
            // This example assumes that the device has two ports:
            //  An input (provides) port of type bulkio::InShortPort called dataShort_in
            //  An output (uses) port of type bulkio::OutFloatPort called dataFloat_out
            // The mapping between the port and the class is found
            // in the device base class header file
            // The device class must have an output stream member; add to
            // LongDevice.h:
            //   bulkio::OutFloatStream outputStream;

            bulkio::InShortStream inputStream = dataShort_in->getCurrentStream();
            if (!inputStream) { // No streams are available
                return NOOP;
            }

            bulkio::ShortDataBlock block = inputStream.read();
            if (!block) { // No data available
                // Propagate end-of-stream
                if (inputStream.eos()) {
                   outputStream.close();
                }
                return NOOP;
            }

            short* inputData = block.data();
            std::vector<float> outputData;
            outputData.resize(block.size());
            for (size_t index = 0; index < block.size(); ++index) {
                outputData[index] = (float) inputData[index];
            }

            // If there is no output stream open, create one
            if (!outputStream) {
                outputStream = dataFloat_out->createStream(block.sri());
            } else if (block.sriChanged()) {
                // Update output SRI
                outputStream.sri(block.sri());
            }

            // Write to the output stream
            outputStream.write(outputData, block.getTimestamps());

            // Propagate end-of-stream
            if (inputStream.eos()) {
              outputStream.close();
            }

            return NORMAL;

        If working with complex data (i.e., the "mode" on the SRI is set to
        true), the data block's complex() method will return true. Data blocks
        provide functions that return the correct interpretation of the data
        buffer and number of complex elements:

            if (block.complex()) {
                std::complex<short>* data = block.cxdata();
                for (size_t index = 0; index < block.cxsize(); ++index) {
                    data[index] = std::abs(data[index]);
                }
                outputStream.write(data, block.cxsize(), bulkio::time::utils::now());
            }

        Interactions with non-BULKIO ports are left up to the device developer's discretion

    Messages:

        To receive a message, you need (1) an input port of type MessageEvent, (2) a message prototype described
        as a structure property of kind message, (3) a callback to service the message, and (4) to register the callback
        with the input port.

        Assuming a property of type message is declared called "my_msg", an input port called "msg_input" is declared of
        type MessageEvent, create the following code:

        void LongDevice_i::my_message_callback(const std::string& id, const my_msg_struct &msg){
        }

        Register the message callback onto the input port with the following form:
        this->msg_input->registerMessage("my_msg", this, &LongDevice_i::my_message_callback);

        To send a message, you need to (1) create a message structure, (2) a message prototype described
        as a structure property of kind message, and (3) send the message over the port.

        Assuming a property of type message is declared called "my_msg", an output port called "msg_output" is declared of
        type MessageEvent, create the following code:

        ::my_msg_struct msg_out;
        this->msg_output->sendMessage(msg_out);

    Accessing the Device Manager and Domain Manager:

        Both the Device Manager hosting this Device and the Domain Manager hosting
        the Device Manager are available to the Device.

        To access the Domain Manager:
            CF::DomainManager_ptr dommgr = this->getDomainManager()->getRef();
        To access the Device Manager:
            CF::DeviceManager_ptr devmgr = this->getDeviceManager()->getRef();

    Properties:

        Properties are accessed directly as member variables. For example, if the
        property name is "baudRate", it may be accessed within member functions as
        "baudRate". Unnamed properties are given the property id as its name.
        Property types are mapped to the nearest C++ type, (e.g. "string" becomes
        "std::string"). All generated properties are declared in the base class
        (LongDevice_base).

        Simple sequence properties are mapped to "std::vector" of the simple type.
        Struct properties, if used, are mapped to C++ structs defined in the
        generated file "struct_props.h". Field names are taken from the name in
        the properties file; if no name is given, a generated name of the form
        "field_n" is used, where "n" is the ordinal number of the field.

        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A boolean called scaleInput

            if (scaleInput) {
                dataOut[i] = dataIn[i] * scaleValue;
            } else {
                dataOut[i] = dataIn[i];
            }

        Callback methods can be associated with a property so that the methods are
        called each time the property value changes.  This is done by calling
        addPropertyListener(<property>, this, &LongDevice_i::<callback method>)
        in the constructor.

        The callback method receives two arguments, the old and new values, and
        should return nothing (void). The arguments can be passed by value,
        receiving a copy (preferred for primitive types), or by const reference
        (preferred for strings, structs and vectors).

        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A struct property called status

        //Add to LongDevice.cpp
        LongDevice_i::LongDevice_i(const char *uuid, const char *label) :
            LongDevice_base(uuid, label)
        {
            addPropertyListener(scaleValue, this, &LongDevice_i::scaleChanged);
            addPropertyListener(status, this, &LongDevice_i::statusChanged);
        }

        void LongDevice_i::scaleChanged(float oldValue, float newValue)
        {
            LOG_DEBUG(LongDevice_i, "scaleValue changed from" << oldValue << " to " << newValue);
        }

        void LongDevice_i::statusChanged(const status_struct& oldValue, const status_struct& newValue)
        {
            LOG_DEBUG(LongDevice_i, "status changed");
        }

        //Add to LongDevice.h
        void scaleChanged(float oldValue, float newValue);
        void statusChanged(const status_struct& oldValue, const status_struct& newValue);

    Allocation:

        Allocation callbacks are available to customize the Device's response to
        allocation requests. For example, if the Device contains the allocation
        property "my_alloc" of type string, the allocation and deallocation
        callbacks follow the pattern (with arbitrary function names
        my_alloc_fn and my_dealloc_fn):

        bool LongDevice_i::my_alloc_fn(const std::string &value)
        {
            // perform logic
            return true; // successful allocation
        }
        void LongDevice_i::my_dealloc_fn(const std::string &value)
        {
            // perform logic
        }

        The allocation and deallocation functions are then registered with the Device
        base class with the setAllocationImpl call. Note that the variable for the property is used rather
        than its id:

        this->setAllocationImpl(my_alloc, this, &LongDevice_i::my_alloc_fn, &LongDevice_i::my_dealloc_fn);



************************************************************************************************/
int LongDevice_i::serviceFunction()
{
    LOG_DEBUG(LongDevice_i, "serviceFunction() example log message");

    return NOOP;
}
