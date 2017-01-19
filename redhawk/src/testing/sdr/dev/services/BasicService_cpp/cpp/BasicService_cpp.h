#ifndef BASICSERVICE_CPP_I_IMPL_H
#define BASICSERVICE_CPP_I_IMPL_H

#include "BasicService_cpp_base.h"

class BasicService_cpp_i;

class BasicService_cpp_i : public BasicService_cpp_base
{
    ENABLE_LOGGING
    public:
        BasicService_cpp_i(char *devMgr_ior, char *name);
        ~BasicService_cpp_i();
        void remove(const char* fileName);
        void copy(const char* sourceFileName, const char* destinationFileName);
        void move(const char* sourceFileName, const char* destinationFileName);
        CORBA::Boolean exists(const char* fileName);
        CF::FileSystem::FileInformationSequence* list(const char* pattern);
        CF::File_ptr create(const char* fileName);
        CF::File_ptr open(const char* fileName, CORBA::Boolean read_Only);
        void mkdir(const char* directoryName);
        void rmdir(const char* directoryName);
        void query(CF::Properties& fileSystemProperties);
        void mount(const char* mountPoint, CF::FileSystem_ptr file_System);
        void unmount(const char* mountPoint);
        CF::FileManager::MountSequence* getMounts();
};

#endif // BASICSERVICE_CPP_I_IMPL_H
