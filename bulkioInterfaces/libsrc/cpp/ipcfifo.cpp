#include "ipcfifo.h"

#include <iostream>
#include <cstdio>
#include <stdexcept>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using bulkio::IPCFifo;
using bulkio::IPCFifoServer;
using bulkio::IPCFifoClient;

IPCFifo::IPCFifo(const std::string& name) :
    _name(name),
    _sendfd(-1),
    _recvfd(-1)
{
}

IPCFifo::~IPCFifo()
{
    close();
}

const std::string& IPCFifo::name() const
{
    return _name;
}

void IPCFifo::close()
{
    if (_sendfd >= 0) {
        ::close(_sendfd);
        _sendfd = -1;
    }
    if (_recvfd >= 0) {
        ::close(_recvfd);
        _recvfd = -1;
    }
}

void IPCFifo::write(const void* data, size_t bytes)
{
    size_t total = 0;
    while (total < bytes) {
        ssize_t pass = ::write(_sendfd, data, bytes);
        if (pass <= 0) {
            if (pass < 0) {
                perror("write");
                throw std::runtime_error("write failed");
            }
            return;
        }
        total += pass;
    }
}

size_t IPCFifo::read(void* buffer, size_t bytes)
{
    ssize_t count = ::read(_recvfd, buffer, bytes);
    if (count < 0) {
        perror("read");
        throw std::runtime_error("read failed");
    }
    return count;
}

std::string IPCFifo::_serverPath() const
{
    return "/tmp/" + _name + "-s";
}

std::string IPCFifo::_clientPath() const
{
    return "/tmp/" + _name + "-c";
}

void IPCFifo::_clearNonBlocking(int fd)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);
}

void IPCFifo::_mkfifo(const std::string& name)
{
    if (mkfifo(name.c_str(), 0666)) {
        perror("mkfifo");
        throw std::runtime_error("mkfifo " + name);
    }
}

int IPCFifo::_openfifo(const std::string& name, int flags)
{
    int fd = open(name.c_str(), flags);
    if (fd < 0) {
        perror("open");
        throw std::runtime_error("open " + name);
    }
    return fd;
}

IPCFifoServer::IPCFifoServer(const std::string& name) :
    IPCFifo(name)
{
}

void IPCFifoServer::beginConnect()
{
    std::string send_path = _serverPath();
    _mkfifo(send_path);

    std::string recv_path = _clientPath();
    _mkfifo(recv_path);

    // Open the client-to-server FIFO for read in non-blocking mode, so the
    // remote end can open the write side.
    _recvfd = _openfifo(recv_path, O_RDONLY|O_NONBLOCK);
}

void IPCFifoServer::finishConnect()
{
    // Open the write end and send a test character to the remote side to give
    // it a chance to read and re-synchronize.
    std::string send_path = _serverPath();
    _sendfd = _openfifo(send_path, O_WRONLY);
    char tmp = 'w';
    write(&tmp, 1);

    // Clear the non-blocking flag and re-synchronize by reading a single
    // character, which the client has already sent.
    std::string recv_path = _clientPath();
    _clearNonBlocking(_recvfd);
    char buf;
    if (read(&buf, 1) != 1) {
        std::cerr << "Server connect failed" << std::endl;
    }

    // Now that both sides of the client-to-server FIFO have been established,
    // the file can be removed.
    unlink(recv_path.c_str());
}

IPCFifoClient::IPCFifoClient(const std::string& name) :
    IPCFifo(name)
{
}

void IPCFifoClient::beginConnect()
{
    // The server should have already opened the client-to-server FIFO in read
    // mode; open the write end and send a test character.
    std::string send_path = _clientPath();
    _sendfd = _openfifo(send_path, O_WRONLY);
    char tmp = 'r';
    write(&tmp, 1);

    // Open the server-to-client FIFO for reading in non-blocking mode so that
    // this function can return now. Then, when the server opens it for write,
    // it can return immediately.
    std::string recv_path = _serverPath();
    _recvfd = _openfifo(recv_path, O_RDONLY|O_NONBLOCK);
}

void IPCFifoClient::finishConnect()
{
    // Clear the non-blocking flag and read the sync character. Even though the
    // flag is cleared, read may return immediately with no data; retrying the
    // read until it returns data ensures that the FIFO is fully connected.
    _clearNonBlocking(_recvfd);
    char buf;
    int retries = 0;
    while (read(&buf, 1) == 0) {
        ++retries;
    }

    // At this point, both ends of the server-to-client FIFO are open, so the
    // file can be removed (the client-to-server FIFO is also established, but
    // the server removes that).
    std::string recv_path = _serverPath();
    unlink(recv_path.c_str());
}
