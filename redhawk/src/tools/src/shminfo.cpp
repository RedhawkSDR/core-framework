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

#include <iomanip>
#include <iostream>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <getopt.h>
#include <signal.h>

#include "ShmVisitor.h"

using redhawk::shm::SuperblockFile;

namespace {
    static void usage()
    {
    }
}

class SizeFormatter
{
public:
    enum Format {
        BYTES,
        KILOBYTES,
        HUMAN_READABLE
    };

    explicit SizeFormatter(Format format=BYTES) :
        _format(format)
    {
    }

    void setFormat(Format format)
    {
        _format = format;
    }

    std::string operator() (size_t size)
    {
        std::ostringstream oss;
        switch (_format) {
        case KILOBYTES:
            oss << (size/1024) << "K";
            break;
        case HUMAN_READABLE:
            _toHumanReadable(oss, size);
            break;
        case BYTES:
        default:
            oss << size;
            break;
        }
        return oss.str();
    }

private:
    void _toHumanReadable(std::ostream& oss, size_t size)
    {
        const char* suffix[] = { "", "K", "M", "G", "T", 0 };
        double val = size;
        int pow = 0;
        while ((val >= 1024.0) && (suffix[pow+1])) {
            val /= 1024.0;
            pow++;
        }
        if (pow == 0) {
            oss << val;
        } else {
            oss << std::fixed << std::setprecision(1) << val << suffix[pow];
        }
    }

    Format _format;
};

class Info : public ShmVisitor
{
public:
    Info() :
        _all(false),
        _format()
    {
    }

    void setDisplayAll(bool all)
    {
        _all = all;
    }

    void setSizeFormat(SizeFormatter::Format format)
    {
        _format.setFormat(format);
    }

    virtual void visitFileSystem(const std::string& path)
    {
        std::cout << path << std::endl;
        struct statvfs status;
        if (statvfs(path.c_str(), &status)) {
            std::cout << "  (cannot stat)" << std::endl;
            return;
        }

        std::cout << "  size: " << _format(status.f_blocks * status.f_frsize) << std::endl;
        std::cout << "  free: " << _format(status.f_bfree * status.f_frsize) << std::endl;
    }

    virtual void visitHeap(SuperblockFile& heap)
    {
        pid_t creator = heap.creator();
        std::cout << "  creator:     " << creator;
        if (kill(creator, 0) != 0) {
            std::cout << " (defunct)";
        }
        std::cout << std::endl;
        std::cout << "  refcount:    " << heap.refcount() << std::endl;

        SuperblockFile::Statistics stats = heap.getStatistics();
        std::cout << "  total size:  " << _format(stats.size) << std::endl;
        std::cout << "  total used:  " << _format(stats.used) << std::endl;
        std::cout << "  superblocks: " << stats.superblocks << std::endl;
        std::cout << "  unused:      " << stats.unused << std::endl;
    }

    virtual void visitFile(const std::string& name)
    {
        if (isHeap(name)) {
            displayFile(name, "REDHAWK heap");
        } else if (_all) {
            displayFile(name, "other");
        }
    }

    virtual void heapException(const std::string& name, const std::exception& exc)
    {
        std::cerr << "error: " << exc.what() << std::endl;
    }

protected:
    void displayFile(const std::string& name, const std::string& type)
    {
        std::string path = getShmPath() + "/" + name;
        std::cout << std::endl << path << std::endl;;

        struct stat status;
        if (stat(path.c_str(), &status)) {
            std::cout << "  (cannot access)" << std::endl;
            return;
        }
        std::cout << "  file size:   " << _format(status.st_size) << std::endl;
        std::cout << "  allocated:   " << _format(status.st_blocks * 512) << std::endl;
        std::cout << "  type:        " << type << std::endl;
    }

    bool _all;
    SizeFormatter _format;
};

int main(int argc, char* argv[])
{
    Info info;

    int opt;
    while ((opt = getopt(argc, argv, "akh")) != -1) {
        switch (opt) {
        case 'a':
            info.setDisplayAll(true);
            break;
        case 'k':
            info.setSizeFormat(SizeFormatter::KILOBYTES);
            break;
        case 'h':
            info.setSizeFormat(SizeFormatter::HUMAN_READABLE);
            break;
        default:
            usage();
            return -1;
        }
    };

    try {
        info.visit();
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << std::endl;
        return -1;
    } 
    return 0;
}
