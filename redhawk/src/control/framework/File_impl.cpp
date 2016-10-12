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
#include <fstream>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

namespace fs = boost::filesystem;

#include "ossie/File_impl.h"
#include "ossie/CorbaUtils.h"
#include "ossie/debug.h"

PREPARE_LOGGING(File_impl)

File_impl::File_impl (const char* fileName, fs::path& path, FileSystem_impl *_ptrFs, bool readOnly, bool create)
{
    TRACE_ENTER(File_impl)

    LOG_TRACE(File_impl, "In constructor with " << fileName << " and path " << path.string());
    fName = fileName;
    fullFileName = path.string();
    fullFileName += fileName;
    fs::path filePath(path / fileName);
    ptrFs = _ptrFs;
    fileIOR="";

    std::ios_base::openmode mode;
    if (create) {
        mode = std::ios::in | std::ios::out | std::ios::trunc;
    } else if (readOnly) {
        mode = std::ios::in;
    } else {
        mode = std::ios::in | std::ios::out;
    }

    int fopenAttempts = 0;
    while (fopenAttempts < 10) {
        f.open(filePath.string().c_str(), mode);
        if (!f) {
            LOG_WARN(File_impl, "Error opening file. Attempting again");
            ++fopenAttempts;
            usleep(10000);
        } else {
            break;
        }
    }

    if (!f) {
        std::string errmsg = "Could not open file: " + filePath.string();
        LOG_ERROR(File_impl, errmsg << " (fail=" << f.fail() << " bad=" << f.bad() << ")");
        throw CF::FileException(CF::CF_EIO, errmsg.c_str());
    }

    TRACE_EXIT(File_impl)
}


File_impl::~File_impl ()
{
  TRACE_ENTER(File_impl);
  LOG_TRACE(File_impl, "Closing file..... " << fullFileName );
  f.close();
  TRACE_EXIT(File_impl);
}


void File_impl::setIOR( const std::string &ior)
{
  boost::mutex::scoped_lock lock(interfaceAccess);
  fileIOR=ior;
}



void File_impl::read (CF::OctetSequence_out data, CORBA::ULong length) throw (CORBA::SystemException, CF::File::IOException)
{
    TRACE_ENTER(File_impl)
    boost::mutex::scoped_lock lock(interfaceAccess);
    LOG_TRACE(File_impl, "In read with length " << length);
    if (length > ossie::corba::giopMaxMsgSize()) {
        std::ostringstream message;
        message << "[File_impl::read] Unable to read ";
        message << length;
        message << " bytes. The maximum read length is ";
        message << ossie::corba::giopMaxMsgSize();
        message << " bytes";
        throw CF::File::IOException (CF::CF_EIO, message.str().c_str());
    }

    int checkSuccessAttempts = 0;
    bool checkSuccess = false;
    while (!checkSuccess) {
        try {
            // Per the spec, if the file-pointer represents the eof
            // return a 0-length sequence
            if (f.eof()) {
                data =  new CF::OctetSequence();
                TRACE_EXIT(File_impl);
                return;
            } else {
                // Check that the file is good before trying a read
                if (!f.good()) {
                    LOG_WARN(File_impl, "Error reading from file, good() is false " << fName);
                    throw CF::File::IOException (CF::CF_EIO, "[File_impl::read] Error reading from file");
                }
            }
            checkSuccess = true;
        } catch ( const fs::filesystem_error& ex ) {
            LOG_WARN(File_impl, "Error in filesystem: "<<ex.what()<<". Attempting again")
            checkSuccessAttempts++;
            if (checkSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( ... ) {
            LOG_WARN(File_impl, "Caught an unhandled file system exception. Attempting again")
            checkSuccessAttempts++;
            if (checkSuccessAttempts == 10)
                { break; }
            usleep(10000);
        }
    }

    if (!checkSuccess) {
        LOG_WARN(File_impl, "Error reading from file, good() is false " << fName);
        throw CF::File::IOException (CF::CF_EIO, "[File_impl::read] Error reading from file");
    }

    CORBA::Octet* buf = CF::OctetSequence::allocbuf (length);
    int readSuccessAttempts = 0;
    bool readSuccess = false;
    unsigned int count = 0;
    while (!readSuccess) {
        try {
            f.read((char*)buf, length);
            count = f.gcount();
            readSuccess = true;
            LOG_TRACE(File_impl, "Read " << count << " bytes from file.");
        } catch ( const fs::filesystem_error& ex ) {
            LOG_WARN(File_impl, "Error in filesystem: "<<ex.what()<<". Attempting again")
            readSuccessAttempts++;
            if (readSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( ... ) {
            LOG_WARN(File_impl, "Caught an unhandled file system exception. Attempting again")
            readSuccessAttempts++;
            if (readSuccessAttempts == 10)
                { break; }
            usleep(10000);
        }
    }

    if (!readSuccess) {
        LOG_WARN(File_impl, "Error reading from file, good() is false " << fName);
        throw CF::File::IOException (CF::CF_EIO, "[File_impl::read] Error reading from file");
    }

    try {
        data = new CF::OctetSequence(length, count, buf, true);
    } catch ( ... ) {
        LOG_WARN(File_impl, "Error reading from file, " << fName);
        throw CF::File::IOException (CF::CF_EIO, "[File_impl::read] Unable to create Octet sequence necessary to read the file");
    }

    // If we failed but we aren't at the EOF then something bad happened,
    // otherwise, per the spec return the number of bytes actually read
    if (f.fail() && !f.eof()) {
        LOG_WARN(File_impl, "Error reading from file, " << fName);
        throw CF::File::IOException (CF::CF_EIO, "[File_impl::read] Error reading from file");
    }

    TRACE_EXIT(File_impl)
}


void
File_impl::write (const CF::OctetSequence& data)
throw (CORBA::SystemException, CF::File::IOException)
{
    TRACE_ENTER(File_impl)
    boost::mutex::scoped_lock lock(interfaceAccess);

    int writeSuccessAttempts = 0;
    bool writeSuccess = false;
    while (!writeSuccess) {
        try {
            f.write((const char*)data.get_buffer(), data.length());
            writeSuccess = true;
        } catch ( const fs::filesystem_error& ex ) {
            LOG_WARN(File_impl, "Error in filesystem: "<<ex.what()<<". Attempting again")
            writeSuccessAttempts++;
            if (writeSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( ... ) {
            LOG_WARN(File_impl, "Caught an unhandled file system exception. Attempting again")
            writeSuccessAttempts++;
            if (writeSuccessAttempts == 10)
                { break; }
            usleep(10000);
        }
    }

    if (!writeSuccess) {
        LOG_ERROR(File_impl, "Error writing to file, " << fName);
        throw (CF::File::IOException (CF::CF_EIO, "[File_impl::write] Error writing to file"));
    }

    if (f.fail()) {
        LOG_ERROR(File_impl, "Error writing to file, " << fName);
        throw (CF::File::IOException (CF::CF_EIO, "[File_impl::write] Error writing to file"));
    }

    TRACE_EXIT(File_impl)
}


CORBA::ULong File_impl::sizeOf ()throw (CORBA::SystemException,
                                        CF::FileException)
{
    boost::mutex::scoped_lock lock(interfaceAccess);
    return _local_sizeOf();
}

CORBA::ULong File_impl::_local_sizeOf ()throw (CORBA::SystemException,
                                        CF::FileException)
{
    TRACE_ENTER(File_impl)

    CORBA::ULong fileSize(0), filePos(0);

    int fsOpSuccessAttempts = 0;
    bool fsOpSuccess = false;
    while (!fsOpSuccess) {
        try {
            filePos = f.tellg();
            f.seekg(0, std::ios::end);
            fileSize = f.tellg();
            f.seekg(filePos);

            fsOpSuccess = true;

            LOG_TRACE(File_impl, "In sizeOf with size = " << fileSize);

        } catch ( const fs::filesystem_error& ex ) {
            LOG_WARN(File_impl, "Error in filesystem: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( ... ) {
            LOG_WARN(File_impl, "Caught an unhandled file system exception. Attempting again")
            fsOpSuccessAttempts++;
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        }
    }
    if (!fsOpSuccess) {
        LOG_ERROR(File_impl, "Error checking the size of file, " << fName);
        throw (CF::File::IOException (CF::CF_EIO, "[File_impl::sizeOf] Error checking the size of file"));
    }

    TRACE_EXIT(File_impl)
    return fileSize;
}


void
File_impl::close ()
throw (CORBA::SystemException, CF::FileException)
{
    TRACE_ENTER(File_impl)
    boost::mutex::scoped_lock lock(interfaceAccess);

    int fsOpSuccessAttempts = 0;
    bool fsOpSuccess = false;
    while (!fsOpSuccess) {
        try {
            f.close();
            fsOpSuccess = true;
        } catch ( const fs::filesystem_error& ex ) {
            LOG_WARN(File_impl, "Error in filesystem: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( ... ) {
            LOG_WARN(File_impl, "Caught an unhandled file system exception. Attempting again")
            fsOpSuccessAttempts++;
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        }
    }

    if ( ptrFs) {
      std::string ior;
      if ( fileIOR != "" ) {
        ior = fileIOR;
      }
      else {
        CF::File_var fileObj = _this();
        ior = ossie::corba::objectToString(fileObj);
      }
      ptrFs->decrementFileIORCount(fullFileName, ior);
    }

    // clean up reference and clean up memory
    try {
        PortableServer::POA_var poa = ossie::corba::RootPOA()->find_POA("Files", 0);
        PortableServer::ObjectId_var oid = poa->servant_to_id(this);
        poa->deactivate_object(oid);
    } catch ( ... ) {

    }

    if (!fsOpSuccess) {
        LOG_ERROR(File_impl, "Error closing file, " << fName);
        throw (CF::FileException (CF::CF_EIO, "[File_impl::close] Error closing file"));
    }

    TRACE_EXIT(File_impl)
}


void
File_impl::setFilePointer (CORBA::ULong _filePointer)
throw (CORBA::SystemException, CF::File::InvalidFilePointer,
       CF::FileException)
{
    TRACE_ENTER(File_impl)
    boost::mutex::scoped_lock lock(interfaceAccess);

    if (_filePointer > this->_local_sizeOf ()) {
        LOG_ERROR(File_impl, "File pointer set beyond EOF")
        throw CF::File::InvalidFilePointer ();
    }

    int fsOpSuccessAttempts = 0;
    bool fsOpSuccess = false;
    while (!fsOpSuccess) {
        try {
            f.seekg(_filePointer);
            f.seekp(_filePointer);
            fsOpSuccess = true;
        } catch ( const fs::filesystem_error& ex ) {
            LOG_WARN(File_impl, "Error in filesystem: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( ... ) {
            LOG_WARN(File_impl, "Caught an unhandled file system exception. Attempting again")
            fsOpSuccessAttempts++;
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        }
    }
    if (!fsOpSuccess) {
        LOG_ERROR(File_impl, "Error setting file pointer for file, " << fName);
        throw (CF::FileException (CF::CF_EIO, "[File_impl::setFilePointer] Error setting file pointer for file"));
    }

    TRACE_EXIT(File_impl)
}
