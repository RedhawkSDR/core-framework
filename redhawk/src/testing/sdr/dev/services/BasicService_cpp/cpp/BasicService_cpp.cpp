/**************************************************************************

    This is the service code. This file contains the child class where
    custom functionality can be added to the service. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "BasicService_cpp.h"

PREPARE_LOGGING(BasicService_cpp_i)
BasicService_cpp_i::BasicService_cpp_i(char *devMgr_ior, char *name) :
    BasicService_cpp_base(devMgr_ior, name)
{
}

BasicService_cpp_i::~BasicService_cpp_i()
{
}

void BasicService_cpp_i::remove(const char* fileName)
{
    // TODO: Fill in this function
}

void BasicService_cpp_i::copy(const char* sourceFileName, const char* destinationFileName)
{
    // TODO: Fill in this function
}

void BasicService_cpp_i::move(const char* sourceFileName, const char* destinationFileName)
{
    // TODO: Fill in this function
}

CORBA::Boolean BasicService_cpp_i::exists(const char* fileName)
{
    CORBA::Boolean tmpVal = false;
    // TODO: Fill in this function
    
    return tmpVal;
}

CF::FileSystem::FileInformationSequence* BasicService_cpp_i::list(const char* pattern)
{
    CF::FileSystem::FileInformationSequence* tmpVal = new CF::FileSystem::FileInformationSequence();
    // TODO: Fill in this function
    
    return tmpVal;
}

CF::File_ptr BasicService_cpp_i::create(const char* fileName)
{
    CF::File_ptr tmpVal = CF::File::_nil();
    // TODO: Fill in this function
    
    return tmpVal;
}

CF::File_ptr BasicService_cpp_i::open(const char* fileName, CORBA::Boolean read_Only)
{
    CF::File_ptr tmpVal = CF::File::_nil();
    // TODO: Fill in this function
    
    return tmpVal;
}

void BasicService_cpp_i::mkdir(const char* directoryName)
{
    // TODO: Fill in this function
}

void BasicService_cpp_i::rmdir(const char* directoryName)
{
    // TODO: Fill in this function
}

void BasicService_cpp_i::query(CF::Properties& fileSystemProperties)
{
    // TODO: Fill in this function
}

void BasicService_cpp_i::mount(const char* mountPoint, CF::FileSystem_ptr file_System)
{
    // TODO: Fill in this function
}

void BasicService_cpp_i::unmount(const char* mountPoint)
{
    // TODO: Fill in this function
}

CF::FileManager::MountSequence* BasicService_cpp_i::getMounts()
{
    CF::FileManager::MountSequence* tmpVal = new CF::FileManager::MountSequence();
    // TODO: Fill in this function
    
    return tmpVal;
}


