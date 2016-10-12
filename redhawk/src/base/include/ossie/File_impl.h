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

#include <fstream>
#include <string>

#include <omniORB4/CORBA.h>

#include <boost/filesystem/path.hpp>
#include <boost/thread/mutex.hpp>

#include "ossie/CF/cf.h"
#include "ossie/ossiecf.h"
#include "ossie/debug.h"
#include "ossie/ossieSupport.h"

class FileSystem_impl;

#include "FileSystem_impl.h"

class OSSIECF_API File_impl: public virtual POA_CF::File
{
    ENABLE_LOGGING

public:
    //File_impl (const char* fileName, boost::filesystem::path& path, bool readOnly, bool create);
    File_impl (const char* fileName, boost::filesystem::path& path, FileSystem_impl *ptrFs, bool readOnly, bool create);
    ~File_impl ();

    char* fileName () throw (CORBA::SystemException) {return CORBA::string_dup(fName.c_str()); };
    void read (CF::OctetSequence_out data, CORBA::ULong length) throw (CF::File::IOException, CORBA::SystemException);
    void write (const CF::OctetSequence& data) throw (CF::File::IOException, CORBA::SystemException);
    void close () throw (CF::FileException, CORBA::SystemException);
    void setFilePointer (CORBA::ULong _filePointer) throw (CF::FileException, CF::File::InvalidFilePointer, CORBA::SystemException);
    CORBA::ULong filePointer () throw (CORBA::SystemException) {return f.tellg(); };
    CORBA::ULong sizeOf ()throw (CF::FileException, CORBA::SystemException);
    CORBA::ULong _local_sizeOf ()throw (CF::FileException, CORBA::SystemException);

private:
    std::string fName;
    std::string fullFileName;
    std::fstream f;
    FileSystem_impl *ptrFs;
    boost::mutex interfaceAccess;

 public:
    void setIOR( const std::string &ior );

 private:
    std::string fileIOR;
};                                                /* END CLASS DEFINITION File */
#endif                                            /* __FILE_IMPL__ */
