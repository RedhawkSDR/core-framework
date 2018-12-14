/*#
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
 #*/
/*{%extends "pull/resource_base.cpp"%}*/
/*{%block license%}*/
/*{% from "gpl.cpp" import gplHeader%}*/
${gplHeader(component)}
/*{%endblock%}*/

/*{% block componentConstructor %}*/
PREPARE_LOGGING(${className})

/**
 * Constructor.  Makes the initial call to set up the Octave interpreter.
 */
${ super() -}
/*{% endblock %}*/

/*{% block constructorBody %}*/
    // start up octave
    const char * argvv [] = {"",  /* program name*/
                             "--silent"};
    octave_main (2, (char **) argvv, true /* embedded */);

/*{% for port in component.ports if port is uses %}*/
    std::string streamID_${className}_${port.cppname} = "${component.name}_${port.cppname}";
    outputPackets["${port.cppname}"] = createDefaultDataTransferType(streamID_${className}_${port.cppname});
/*{% endfor %}*/

    _sriPort = "";

${ super() -}
/*{% endblock %}*/

/*{% block destructorBody %}*/
${ super() }
/*{%if component.ports%}*/
    // Deallocate the allocated DataTransferType objects for the output packets
    std::map<std::string, bulkio::InDoublePort::DataTransferType*>::iterator iter;
    for (iter = outputPackets.begin(); iter != outputPackets.end(); ++iter) {
       delete iter->second;
    }

/*{%endif%}*/
    // Prevent octave from leaving around temporary files after shutdown.
    clean_up_and_exit(0);
/*{% endblock %}*/

/*{%block extensions%}*/
/**
 * Wrapper to Octave's feval method that monitors the Octave error state.
 * If an error is detected, it is logged and an exception (invalid_argument)
 * is thrown.
 */
const octave_value_list ${className}::_feval(
    const std::string function,
    const octave_value_list &functionArguments)
{
    octave_value_list result;

    // Make sure the error state is cleared before calling feval so that
    // we only get errors related to this feval call.
    reset_error_handler();
    result = feval(function.c_str(), functionArguments);

    if (error_state != 0) { // If Octave reports an error
        // Formulate the error message to send to the REDHAWK log
        std::string errorStr = "Octave entered an error state.  ";
        std::string error_msg = last_error_message();
        if ((error_msg != "\n") and (not error_msg.empty())) {
            errorStr += "\nThe following error occurred:\n"+error_msg+"\n";
        }
        errorStr += "If the diaryEnabled property is set to \"true\", you may check the diary file written to ";
        errorStr += _diaryFile;

        // Log the error and throw an exception
        RH_ERROR(this->_baseLog, errorStr);
        throw std::invalid_argument("");
    }
    return result;
}

/**
 * Set the OCTAVEPATH to the same directory as the component executable
 */
void ${className}::setCurrentWorkingDirectory(std::string& cwd)
{
    Resource_impl::setCurrentWorkingDirectory(cwd);

    // set the OCTAVEPATH to the same directory as the component executable
    octave_value_list functionArguments; // pass to octave
    functionArguments(0) = octave_value(getCurrentWorkingDirectory());
    _feval("addpath", functionArguments);
}

/**
 * Get logs/@@@WAVEFORM.NAME@@@/@@@COMPONENT.NAME@@@ from the
 * log4j.appender.__octave.File=logs/@@@WAVEFORM.NAME@@@/@@@COMPONENT.NAME@@@/__log
 * line of the log configuration string; then return:
 *
 *      ${PWD}/../../../logs/@@@WAVEFORM.NAME@@@/@@@COMPONENT.NAME@@@
 *
 * This should facilitate running the component in a logs directory that
 * resides in the top-level of the device cache.
 *
 * The current working directory is returned if a valid log filename is not
 * found.
 *
 * This approach should be replaced by getting the file name through the
 * appropriate log API once it is available.
 */
std::string ${className}::getLogDir()
{
    std::string logConfig = this->getLogConfig();
    std::string tag1 = "log4j.appender.__octave.File=";
    std::string tag2 = "__log";
    std::string logDir = getCurrentWorkingDirectory();
    if (std::string::npos != logConfig.find(tag1) &&
        std::string::npos != logConfig.find(tag2)) {
        // if we found both tags

        // get the substring we are looking for
        unsigned int start = logConfig.find(tag1) + tag1.length();
        unsigned int end = logConfig.find(tag2,start);
        logDir += "/../../../" + logConfig.substr(start,end-start);

        if (std::string::npos != logDir.find("\n")) {
            // If a newline is present, tag1 and tag2 were not on the same
            // line.  If this is the case, then we did not actually find the
            // filename/path that we were looking for
            logDir = getCurrentWorkingDirectory();
        }
    }
    return logDir;
}

/**
 * Recursively create a directory.
 */
void ${className}::createDirectoryTree(std::string target_dir) {
    boost::filesystem::path dirPath(target_dir);
    boost::filesystem::path currentPath;

    // recursively create the directory for the diary file (in case it
    // does not exist)
    for (boost::filesystem::path::iterator walkPath = dirPath.begin();
         walkPath != dirPath.end();
         ++walkPath) {

        currentPath /= *walkPath;
        if (!boost::filesystem::exists(currentPath)) {
            boost::filesystem::create_directory(currentPath);
        }
    }
}

/**
 * Turn the diary on or off based on the value of the class data field
 * (property) diaryEnabled.
 *
 * Change directories to the log directory so that all files (e.g., the diary)
 * get written to the same location.  This allows the output files to be
 * written to a persistent directory that is unique to the waveform and
 * component instances.
 */
void ${className}::setDiary()
{
    // Change directory into log directory (if log directory is available).
    std::string logDir = getLogDir();
    _diaryFile = logDir;

    // In many cases the directory will have already been created.  However, it
    // is possible that the log directory might not exist (e.g., if thie file
    // appender exists but it is not enabled).
    if (!boost::filesystem::exists(logDir)) {
        createDirectoryTree(logDir);
    }

    octave_value_list functionArguments; // pass to octave
    functionArguments(0) = octave_value(logDir);
    _feval("cd", functionArguments);

    // call the diary function to turn the diary on or off
    if (diaryEnabled) {
        functionArguments(0) = octave_value("${component.name}.diary");
    } else {
        functionArguments(0) = octave_value("off");
    }
    _feval("diary", functionArguments);
}

/**
 * Flush the diary by turning it off.
 *
 * Ignore any errors reported by the Octave interpreter.
 */
void ${className}::flushDiary()
{
    octave_value_list functionArguments; // pass to octave
    functionArguments(0) = octave_value("off");
    try {
        _feval("diary", functionArguments);
    } catch ( std::invalid_argument ) {}
}

/*# bulkio will only be imported if ports are defined #*/
/*{%if component.ports%}*/
/**
 * Initialize a new DataTransferType with some default values.
 *
 * The returned DataTransferType object must be deleted later.
 */
bulkio::InDoublePort::DataTransferType* ${className}::createDefaultDataTransferType(
    std::string& streamID)
{
    const PortTypes::DoubleSequence emptyVector;

    CORBA::Boolean EOS     = true;
    bool inputQueueFlushed = false;
    bool sriChanged        = true;
    bool blocking          = true;

    BULKIO::PrecisionUTCTime timestamp = BULKIO::PrecisionUTCTime();

    BULKIO::StreamSRI SRI = bulkio::sri::create(
        streamID,           /* stream ID    */
        -1,                 /* sample rate  */
        BULKIO::UNITS_NONE, /* units        */
        blocking);          /* blocking     */

    return new bulkio::InDoublePort::DataTransferType(
        emptyVector,        /* default value        */
        timestamp,          /* timestamp            */
        EOS,                /* EOS                  */
        streamID.c_str(),   /* stream ID            */
        SRI,                /* SRI                  */
        sriChanged,         /* SRI-chaged flag      */
        inputQueueFlushed); /* flush-to-report flag */
}

/**
 * Sub-method of appendInputPacketToFunctionArguments.
 */
void appendComplexInputPacketToFunctionArguments(
    octave_value_list&                        functionArguments, /* output */
    const bulkio::InDoublePort::dataTransfer* inputPacket)       /* input  */
{
    // create a complex list from the list of interleaved real/imag values
    std::vector<std::complex<double> >* complexList = 
        (std::vector<std::complex<double> >*) &(inputPacket->dataBuffer);

    if (inputPacket->SRI.subsize == 0) 
    { // vector data
        // convert std::vector<std::complex> to ComplexRowVector
        ComplexRowVector mRowVector(complexList->size());
        for (unsigned int i=0; i < complexList->size(); i++) {
            mRowVector(i) = (*complexList)[i];
        }
        functionArguments.append(octave_value(mRowVector));
    } // end vector data
    else
    { // Matrix (framed) data
        unsigned int numRows = complexList->size() / inputPacket->SRI.subsize;
        unsigned int numCols = inputPacket->SRI.subsize;
        ComplexMatrix mMatrix(numRows, numCols);
        unsigned int bulkioIndex = 0;
        for (unsigned int rowNum=0; rowNum < numRows; rowNum++) {
            for (unsigned int colNum=0; colNum <numCols; colNum++) {
                mMatrix(rowNum, colNum) = (*complexList)[bulkioIndex++];
            }
        }
        functionArguments.append(octave_value(mMatrix));
    } // end matrix data
}

/**
 * Sub-method of appendInputPacketToFunctionArguments.
 */
void appendScalarInputPacketToFunctionArguments(
    octave_value_list&                        functionArguments, /* output */
    const bulkio::InDoublePort::dataTransfer* inputPacket)       /* input  */
{
    if (inputPacket->SRI.subsize == 0) 
    { // vector data
        // convert std::vector to RowVector
        RowVector mRowVector(inputPacket->dataBuffer.size());
        for (unsigned int i=0; i < inputPacket->dataBuffer.size(); i++) {
            mRowVector(i) = (double)inputPacket->dataBuffer[i];
        }
        functionArguments.append(octave_value(mRowVector));
    } // end vector data
    else
    { // Matrix (framed) data
        unsigned int numRows = inputPacket->dataBuffer.size() / inputPacket->SRI.subsize;
        unsigned int numCols = inputPacket->SRI.subsize;
        Matrix mMatrix(numRows, numCols);
        unsigned int bulkioIndex = 0;
        for (unsigned int rowNum=0; rowNum < numRows; rowNum++) {
            for (unsigned int colNum=0; colNum <numCols; colNum++) {
                mMatrix(rowNum, colNum) = (double)inputPacket->dataBuffer[bulkioIndex++];
            }
        }
        functionArguments.append(octave_value(mMatrix));
    } // end matrix data
}

/**
 * Convert the inputPacket to a RowVector or matrix and append it to
 * functionArguments
 */
void appendInputPacketToFunctionArguments(
    octave_value_list&                        functionArguments, /* output */
    const bulkio::InDoublePort::dataTransfer* inputPacket)       /* input  */
{
    if (inputPacket->SRI.mode) {
        appendComplexInputPacketToFunctionArguments(
            functionArguments,
            inputPacket);
    } else {
        appendScalarInputPacketToFunctionArguments(
            functionArguments,
            inputPacket);
    }
}

/**
 * Buffer data until an EOS flag is encountered.
 *
 * Populates inputPackets[portName].  SRI, other than EOS, inputQueueFlushed,
 * and sriChanged, is assumed to be the same for all incoming sub-packets.
 * All subpacket data is appended to inputPackets[portName]->dataBuffer.
 *
 * Note that buffering requires an additional data copy.
 */
int ${className}::buffer(std::string portName, bulkio::InDoublePort* port)
{
    while (inputPackets[portName]->EOS == false) {
        // put data on the buffer
        bulkio::InDoublePort::DataTransferType* tmpPkt = port->getPacket(-1);
        if (not tmpPkt) {
            // No data is available because component is being killed
            return NOOP;
        }

        // copy the incoming packet data into the master packet
        inputPackets[portName]->dataBuffer.insert(
            inputPackets[portName]->dataBuffer.end(),
            tmpPkt->dataBuffer.begin(),
            tmpPkt->dataBuffer.end());

        if (tmpPkt->sriChanged) {
            // if any of the input packets in the stream indicated a change in
            // SRI, set sriChanged to true.
            inputPackets[portName]->sriChanged = true;
        }
        if (tmpPkt->inputQueueFlushed) {
            // if any of the input packets in the stream indicated a change in
            // SRI, set sriChanged to true.
            inputPackets[portName]->inputQueueFlushed = true;
        }
        inputPackets[portName]->EOS = tmpPkt->EOS;
        delete tmpPkt;
    }
    return NORMAL;
}

/**
 * Update the outputPacket->SRI.mode value and set the
 * outputPacket->sriChanged value appropriately.
 */
void setPacketMode(
    bulkio::InDoublePort::DataTransferType* outputPacket, /* input/output */
    int                                     newVal)
{
    int prevVal = outputPacket->SRI.mode;
    if(newVal != prevVal) {
        outputPacket->SRI.mode = newVal;
        outputPacket->sriChanged = true;
    }
}

/**
 * Update the outputPacket->SRI.subsize value and set the
 * outputPacket->sriChanged value appropriately.
 */
void setPacketSubsize(
    bulkio::InDoublePort::DataTransferType* outputPacket, /* input/output */
    int                                     newVal)
{
    int prevVal = outputPacket->SRI.subsize;
    if(newVal != prevVal) {
        outputPacket->SRI.subsize = newVal;
        outputPacket->sriChanged = true;
    }
}

/**
 * Sub-method of populateOutputPacket.
 */
void populateComplexOutputPacket(
    bulkio::InDoublePort::DataTransferType* outputPacket, /* output */
    const octave_value_list&                result,       /* input  */
    const int                               resultIndex)  /* input  */
{
    int ndims = result(resultIndex).ndims();

    // In Octave 3.6.4, the call:
    //    int ndims = result(resultIndex).ndims();
    // has historically produced bad data (sometimes returning
    // ndims=2 for n-by-m matrices).  As a result, we determine
    // ndims ourselves.  We can only do this if Octave returned
    // a matrix, as data that is explicityly one-dimentional
    // will not have the methods rows()/cols().
    if (ndims == 2) {
        int ncols = result(resultIndex).matrix_value().cols();
        int nrows = result(resultIndex).matrix_value().rows();
        if ((ncols == 1) or (nrows == 1) ){
            ndims = 1;
        } else {
            ndims = 2;
        }
    }

    if (ndims == 1)
    { // Vector Data
        // convert RowVector to std::vector
        // (with alternating real/complex values)
        ComplexRowVector outputVector = result(resultIndex).complex_array_value();
        outputPacket->dataBuffer.resize(outputVector.length()*2);
        for (int i = 0; i < outputVector.length(); i++) {
            outputPacket->dataBuffer[i*2]   = (CORBA::Double)outputVector(i).real();
            outputPacket->dataBuffer[i*2+1] = (CORBA::Double)outputVector(i).imag();
        }
        setPacketSubsize(outputPacket, 0);
    } // end vector data 
    else if (ndims == 2) 
    { // 2-D matrix
        ComplexMatrix outputMatrix = result(resultIndex).complex_matrix_value();
        outputPacket->dataBuffer.resize(outputMatrix.nelem()*2);
        int numRows = outputMatrix.rows(); 
        setPacketSubsize(outputPacket, outputMatrix.cols());
        unsigned int dataBufferCount = 0;
        for (int row = 0; row < numRows; row++) {
            for (int elem = row; elem < outputMatrix.nelem(); elem+=numRows) {
                 outputPacket->dataBuffer[dataBufferCount*2]   = (CORBA::Double)outputMatrix(elem).real();
                 outputPacket->dataBuffer[dataBufferCount*2+1] = (CORBA::Double)outputMatrix(elem).imag();
                 dataBufferCount++;
             }
         }
    } // end 2-D matrix
    else
    { // more thank 2 dimensions
        throw std::invalid_argument("BULKIO cannot support matricies with more than 2 dimensions.");
    }

    setPacketMode(outputPacket, true);
}

/**
 * Sub-method of populateOutputPacket.
 */
void populateScalarOutputPacket(
    bulkio::InDoublePort::DataTransferType* outputPacket, /* output */
    const octave_value_list&                result,       /* input  */
    const int                               resultIndex)  /* input  */
{
    int ndims = result(resultIndex).ndims();

    // In Octave 3.6.4, the call:
    //    int ndims = result(resultIndex).ndims();
    // has historically produced bad data (sometimes returning
    // ndims=2 for n-by-m matrices).  As a result, we determine
    // ndims ourselves.  We can only do this if Octave returned
    // a matrix, as data that is explicityly one-dimentional
    // will not have the methods rows()/cols().
    if (ndims == 2) {
        int ncols = result(resultIndex).matrix_value().cols();
        int nrows = result(resultIndex).matrix_value().rows();
        if ((ncols == 1) or (nrows == 1) ){
            ndims = 1;
        } else {
            ndims = 2;
        }
    }

    if (ndims == 1)
    { // Vector Data
        // convert RowVector to std::vector
        RowVector outputVector = result(resultIndex).array_value();
        outputPacket->dataBuffer.resize(outputVector.length());
        for (int i = 0; i <  outputVector.length(); i++) {
            outputPacket->dataBuffer[i] = (CORBA::Double)outputVector(i);
        }
        setPacketSubsize(outputPacket, 0);
    }
    else if (ndims == 2) 
    { // 2-D matrix
        Matrix outputMatrix = result(resultIndex).matrix_value();
        outputPacket->dataBuffer.resize(outputMatrix.nelem());
        int numRows = outputMatrix.rows(); 
        setPacketSubsize(outputPacket, outputMatrix.cols());

        unsigned int dataBufferCount = 0;
        for (int row = 0; row < numRows; row++) {
            for (int elem = row; elem < outputMatrix.nelem(); elem+=numRows) {
                outputPacket->dataBuffer[dataBufferCount++] = (CORBA::Double)outputMatrix(elem);
            }
        }
    } // end 2-D matrix
    else
    { // more thank 2 dimensions
        throw std::invalid_argument("BULKIO cannot support matricies with more than 2 dimensions.");
    }
    setPacketMode(outputPacket, false);
}

/**
 * Convert the item number resultIndex of result into a std::vector.
 * Populates and returns outputPacket.
 *
 * Set the streamID and sample rate of the outputPacket (uses port) to
 * that of the _sriPort (an input/prides port).
 *
 * If no provides (input) ports exist, leave the original settings
 * for the outputPacket (usesPort).
 *
 * Note that these SRI settings can be overloaded in the the postProcess()
 * method.
 */
void ${className}::populateOutputPacket(
    bulkio::InDoublePort::DataTransferType* outputPacket, /* output */
    const octave_value_list&                result,       /* input  */
    const int                               resultIndex)  /* input  */
{
    if (inputPackets.count(_sriPort) > 0) {
        // We are interested in the previously-outputted subsize and mode
        // since these values are set dynamically based on the octave output.
        // Knowing the previous value allows us to set sriChanged appropriately
        // within the poulate sub-methods.
        int prevSubsize = outputPacket->SRI.subsize;
        int prevMode    = outputPacket->SRI.mode;

        outputPacket->SRI          = inputPackets[_sriPort]->SRI;
        outputPacket->SRI.subsize  = prevSubsize;
        outputPacket->SRI.mode     = prevMode;
        outputPacket->T            = bulkio::time::utils::now();
        outputPacket->streamID     = inputPackets[_sriPort]->streamID.c_str();
        outputPacket->EOS          = inputPackets[_sriPort]->EOS;
        outputPacket->sriChanged   = inputPackets[_sriPort]->sriChanged;
    }

    if (resultIndex < result.length()) {
        // Got data for this argument
        if (result(resultIndex).is_complex_type()) {
            populateComplexOutputPacket(outputPacket, result, resultIndex);
        } else {
            populateScalarOutputPacket(outputPacket, result, resultIndex);
        }
     } else {
        // Did not get data for this argument.  Send an empty packet.
        outputPacket->dataBuffer.clear();
     }
}
/*# end bulkio conditional #*/
/*{%endif%}*/

int ${className}::preProcess(){return NORMAL;}
int ${className}::postProcess(){return NORMAL;}

int ${className}::serviceFunction()
{
/*{% from "mFunction/octaveEmbedding.cpp" import octaveEmbedding%}*/
    ${octaveEmbedding(component)|indent(4)}
}
/*{%endblock%}*/
