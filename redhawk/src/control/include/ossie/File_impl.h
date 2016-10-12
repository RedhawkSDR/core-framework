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


/* SCA */
#ifndef FILE_IMPL_H
#define FILE_IMPL_H

#include <string>
#include <vector>
#include <omniORB4/CORBA.h>

#include <boost/thread/mutex.hpp>

#include "ossie/CF/cf.h"
#include "ossie/debug.h"

class FileSystem_impl;

class File_impl: public virtual POA_CF::File
{
    ENABLE_LOGGING

public:
    static File_impl* Create (const char* fileName, FileSystem_impl* ptrFs);
    static File_impl* Open (const char* fileName, FileSystem_impl* ptrFs, bool readOnly);

    ~File_impl ();

    char* fileName () throw (CORBA::SystemException);
    void read (CF::OctetSequence_out data, CORBA::ULong length) throw (CF::File::IOException, CORBA::SystemException);
    void write (const CF::OctetSequence& data) throw (CF::File::IOException, CORBA::SystemException);
    void close () throw (CF::FileException, CORBA::SystemException);
    void setFilePointer (CORBA::ULong _filePointer) throw (CF::FileException, CF::File::InvalidFilePointer, CORBA::SystemException);
    CORBA::ULong filePointer () throw (CORBA::SystemException);
    CORBA::ULong sizeOf () throw (CF::FileException, CORBA::SystemException);
    void setIOR( const std::string &ior);

private:
    File_impl (const char* fileName, FileSystem_impl *ptrFs, bool readOnly, bool create);

    CORBA::ULong getSize () throw (CF::FileException);

    std::string fName;
    std::string fullFileName;

    int fd;
    FileSystem_impl *ptrFs;
    boost::mutex interfaceAccess;
    std::vector<uint8_t>     _buf;
    std::string  fileIOR;
};                                                /* END CLASS DEFINITION File */
#endif                                            /* __FILE_IMPL__ */
