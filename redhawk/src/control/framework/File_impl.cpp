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


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ossie/File_impl.h"
#include "ossie/FileSystem_impl.h"
#include "ossie/CorbaUtils.h"
#include "ossie/debug.h"

PREPARE_LOGGING(File_impl)


File_impl* File_impl::Create (const char* fileName, FileSystem_impl *ptrFs)
{
    return new File_impl(fileName, ptrFs, false, true);
}

File_impl* File_impl::Open (const char* fileName, FileSystem_impl *ptrFs, bool readOnly)
{
    return new File_impl(fileName, ptrFs, readOnly, false);
}

File_impl::File_impl (const char* fileName, FileSystem_impl *_ptrFs, bool readOnly, bool create):
  fName(fileName),
  fullFileName(_ptrFs->getLocalPath(fileName)),
  fd(-1),
  ptrFs(_ptrFs),
  fileIOR("")
{
    TRACE_ENTER(File_impl)

    LOG_TRACE(File_impl, "In constructor with " << fileName << " and path " << fullFileName);

    int flags = 0;
    if (create) {
        flags = O_RDWR|O_CREAT|O_TRUNC;
    } else if (readOnly) {
        flags = O_RDONLY;
    } else {
        flags = O_RDWR;
    }
    int mode = S_IRWXU|S_IRWXG|S_IRWXO;
    
    fd = -1;
    do {
        fd = open(fullFileName.c_str(), flags, mode);
    } while ((fd < 0) && (errno == EINTR));

    if (fd < 0) {
        std::string errmsg = "Could not open file: " + fullFileName;
        throw CF::FileException(CF::CF_EIO, errmsg.c_str());
    }

    TRACE_EXIT(File_impl)
}


File_impl::~File_impl ()
{
  TRACE_ENTER(File_impl);
  LOG_TRACE(File_impl, "Closing file..... " << fullFileName );
  if ( fd > 0 ) ::close(fd);
  TRACE_EXIT(File_impl);
}

void File_impl::setIOR( const std::string &ior)
{
  boost::mutex::scoped_lock lock(interfaceAccess);
  fileIOR=ior;
}

char* File_impl::fileName ()
    throw (CORBA::SystemException)
{
    return CORBA::string_dup(fName.c_str());
}

void File_impl::read (CF::OctetSequence_out data, CORBA::ULong length)
    throw (CORBA::SystemException, CF::File::IOException)
{
    TRACE_ENTER(File_impl)
    boost::mutex::scoped_lock lock(interfaceAccess);

    // GIOP messages cannot exceed a certain (configurable) size, or an unhelpful
    // CORBA::MARSHALL error will be raised.
    if (length > ossie::corba::giopMaxMsgSize()) {
        std::ostringstream message;
        message << "Read of " << length << " bytes exceeds maximum read of "
                << ossie::corba::giopMaxMsgSize() << " bytes";
        throw CF::File::IOException(CF::CF_EIO, message.str().c_str());
    }

    LOG_TRACE(File_impl, "Reading " << length << " bytes from " << fName);

    // Pre-allocate a buffer long enough to contain the entire read.
    CORBA::Octet* buf = CF::OctetSequence::allocbuf(length);
    ssize_t count;
    do {
        count = ::read(fd, (char*)buf, length);
    } while ((count == -1) && (errno == EINTR));

    if (count == -1) {
        // Read failed, release the buffer.
        CF::OctetSequence::freebuf(buf);
        throw CF::File::IOException(CF::CF_EIO, "Error reading from file");
    }

    // Hand the buffer over to a new OctetSequence; if file pointer was already at the end,
    // it will be a zero-length sequence (which follows the spec).
    LOG_TRACE(File_impl, "Read " << count << " bytes from " << fName);
    data = new CF::OctetSequence(length, count, buf, true);

    TRACE_EXIT(File_impl)
}


void File_impl::write (const CF::OctetSequence& data)
    throw (CORBA::SystemException, CF::File::IOException)
{
    TRACE_ENTER(File_impl)
    boost::mutex::scoped_lock lock(interfaceAccess);

    const char* buffer = reinterpret_cast<const char*>(data.get_buffer());
    ssize_t todo = data.length();
    while (todo > 0) {
        ssize_t count = ::write(fd, buffer, todo);
        if ((count <= 0) && (errno != EINTR)) {
            throw CF::File::IOException(CF::CF_EIO, "Error writing to file");
        }
        buffer += count;
        todo -= count;
    }

    TRACE_EXIT(File_impl)
}


CORBA::ULong File_impl::sizeOf ()
    throw (CORBA::SystemException, CF::FileException)
{
    boost::mutex::scoped_lock lock(interfaceAccess);
    TRACE_ENTER(File_impl);

    CORBA::ULong size = getSize();

    TRACE_EXIT(File_impl);
    return size;
}

void File_impl::close ()
    throw (CORBA::SystemException, CF::FileException)
{
    TRACE_ENTER(File_impl)
    boost::mutex::scoped_lock lock(interfaceAccess);

    try {
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
    } catch ( ... ) {

    }

    // clean up reference and clean up memory
    try {
        PortableServer::POA_var poa = ossie::corba::RootPOA()->find_POA("Files", 0);
        PortableServer::ObjectId_var oid = poa->servant_to_id(this);
        poa->deactivate_object(oid);
    } catch ( ... ) {

    }

    int status;
    do {
        status = ::close(fd);
        fd = -1;
    } while (status && (errno == EINTR));
    fd = -1;

    if (status) {
        throw CF::FileException(CF::CF_EIO, "Error closing file");
    }

    TRACE_EXIT(File_impl)
}


CORBA::ULong File_impl::filePointer ()
    throw (CORBA::SystemException)
{
    TRACE_ENTER(File_impl);
    boost::mutex::scoped_lock lock(interfaceAccess);

    off_t pos = lseek(fd, 0, SEEK_CUR);

    TRACE_EXIT(File_impl);
    return pos;
};

void File_impl::setFilePointer (CORBA::ULong _filePointer)
    throw (CORBA::SystemException, CF::File::InvalidFilePointer, CF::FileException)
{
    TRACE_ENTER(File_impl)
    boost::mutex::scoped_lock lock(interfaceAccess);

    if (_filePointer > getSize()) {
        throw CF::File::InvalidFilePointer();
    }

    if (lseek(fd, _filePointer, SEEK_SET) == -1 ) {
        throw CF::FileException(CF::CF_EIO, "Error setting file pointer for file");
    }

    TRACE_EXIT(File_impl)
}

CORBA::ULong File_impl::getSize ()
    throw (CF::FileException)
{
    struct stat filestat;
    if (fstat(fd, &filestat)) {
        throw CF::FileException(CF::CF_EIO, "Error determining file size");
    }
    return filestat.st_size;
}
