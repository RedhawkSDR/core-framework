#include <ossie/shm/MappedFile.h>

#include <stdexcept>
#include <cstring>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

using namespace redhawk::shm;

static std::string error_string()
{
    return strerror(errno);
}

const size_t MappedFile::PAGE_SIZE = sysconf(_SC_PAGESIZE);

MappedFile::MappedFile(const std::string& name) :
    _name(name),
    _fd(-1)
{
}

void MappedFile::create()
{
    if (_fd >= 0) {
        throw std::runtime_error("shm file is already open");
    }

    _fd = shm_open(_name.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0666);
    if (_fd < 0) {
        throw std::runtime_error("shm_open: " + error_string());
    }
}

void MappedFile::open()
{
    if (_fd >= 0) {
        throw std::runtime_error("shm file is already open");
    }

    _fd = shm_open(_name.c_str(), O_RDWR, 0);
    if (_fd < 0) {
        throw std::runtime_error("shm_open: " + error_string());
    }
}

MappedFile::~MappedFile()
{
    close();
}

const std::string& MappedFile::name() const
{
    return _name;
}

size_t MappedFile::size() const
{
    struct stat statbuf;
    if (fstat(_fd, &statbuf)) {
        throw std::runtime_error("fstat: " + error_string());
    }
    return statbuf.st_size;
}

void MappedFile::resize(size_t bytes)
{
    if (ftruncate(_fd, bytes)) {
        throw std::runtime_error("ftruncate: " + error_string());
    }
}

void* MappedFile::map(size_t bytes, mode_e mode, off_t offset)
{
    int prot = PROT_READ;
    if (mode == READWRITE) {
        prot |= PROT_WRITE;
    }

    void* addr = mmap(0, bytes, prot, MAP_SHARED, _fd, offset);
    if (addr == MAP_FAILED) {
        throw std::runtime_error("mmap: " + error_string());
    }
    return addr;
}

void* MappedFile::remap(void* oldAddr, size_t oldSize, size_t newSize)
{
    int flags = MREMAP_MAYMOVE;
    void* addr = mremap(oldAddr, oldSize, newSize, flags);
    if (addr == MAP_FAILED) {
        throw std::runtime_error("mremap: " + error_string());
    }
    return addr;
}

void MappedFile::unmap(void* ptr, size_t bytes)
{
    if (munmap(ptr, bytes)) {
        throw std::runtime_error("munmap: " + error_string());
    }
}

void MappedFile::close()
{
    if (_fd >= 0) {
        ::close(_fd);
        _fd = -1;
    }
}

void MappedFile::unlink()
{
    if (shm_unlink(_name.c_str())) {
        throw std::runtime_error("unlink: " + error_string());
    }
}
