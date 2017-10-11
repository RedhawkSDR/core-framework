#ifndef __bulkio_ipcfifo_h
#define __bulkio_ipcfifo_h

#include <string>

namespace bulkio {

    class IPCFifo {
    public:
        IPCFifo(const std::string& name);
        virtual ~IPCFifo();

        const std::string& name() const;

        virtual void beginConnect() = 0;
        virtual void finishConnect() = 0;

        void close();

        size_t read(void* buffer, size_t bytes);
        void write(const void* data, size_t bytes);

    protected:
        std::string _serverPath() const;
        std::string _clientPath() const;

        void _clearNonBlocking(int fd);

        void _mkfifo(const std::string& name);
        int _openfifo(const std::string& name, int flags);

        const std::string _name;
        int _sendfd;
        int _recvfd;
    };

    class IPCFifoServer : public IPCFifo {
    public:
        IPCFifoServer(const std::string& name);

        virtual void beginConnect();
        virtual void finishConnect();
    };

    class IPCFifoClient : public IPCFifo {
    public:
        IPCFifoClient(const std::string& name);

        virtual void beginConnect();
        virtual void finishConnect();
    };
}

#endif // __bulkio_ipcfifo_h
