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

#include "ShmVisitor.h"

#include <iostream>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>

#include <ossie/shm/System.h>

using redhawk::shm::SuperblockFile;

namespace {
    static std::string executable;

    static void usage()
    {
        std::cout << "Usage: " << executable << " [OPTION]... [FILE]..." << std::endl;
        std::cout << "Remove orphaned REDHAWK heaps and shared memory files." << std::endl;
        std::cout << std::endl;
        std::cout << "  -f, --force          remove the named file regardless of type" << std::endl;
        std::cout << "  -h, --help           display this help and exit" << std::endl;
        std::cout << "      --version        output version information and exit" << std::endl;
        std::cout << std::endl;
        std::cout << "When no file is given, all accessible REDHAWK heaps are scanned and any" << std::endl;
        std::cout << "orphaned heap is removed. File names are relative to " << redhawk::shm::getSystemPath() << "." << std::endl;
        std::cout << std::endl;
        std::cout << "A heap is considered orphaned if its creating process is no longer alive." << std::endl;
        std::cout << "The shared memory file is unlinked; however, if any active processes have" << std::endl;
        std::cout << "the memory mapped, the shared memory is not freed until they exit." << std::endl;
        std::cout << std::endl;
        std::cout << "By default, active heaps and non-REDHAWK shared memory files are not" << std::endl;
        std::cout << "removed. Use the --force (-f) option to remove these files." << std::endl;
    }
}

class Clean : public ShmVisitor
{
public:
    Clean() :
        _force(false)
    {
    }

    void setForce(bool force)
    {
        _force = force;
    }

    bool cleanFile(const std::string& filename)
    {
        if (_force) {
            if (shm_unlink(filename.c_str())) {
                perror(filename.c_str());
                return false;
            }
            return true;
        }

        if (!SuperblockFile::IsSuperblockFile(filename)) {
            std::cerr << filename << " is not a heap, use -f to force removal" << std::endl;
            return false;
        }

        SuperblockFile heap(filename);
        try {
            heap.open(false);
        } catch (const std::exception& exc) {
            std::cerr << filename << " could not be opened: " << exc.what() << std::endl;
            return false;
        }

        if (!heap.isOrphaned()) {
            std::cerr << filename << " is in use, use -f to force removal" << std::endl;
            return false;
        }

        _unlink(heap);
        return true;
    }

    virtual void visitHeap(SuperblockFile& heap)
    {
        if (!heap.isOrphaned()) {
            return;
        }
        _unlink(heap);
    }

private:
    void _unlink(SuperblockFile& heap)
    {
        std::cout << "unlinking " << heap.name() << std::endl;
        try {
            heap.file().unlink();
        } catch (const std::exception& exc) {
            std::cerr << "error: " << exc.what() << std::endl;
        }
    }

    bool _force;
};

int main(int argc, char* argv[])
{
    // Save the executable name for output, removing any paths
    executable = argv[0];
    std::string::size_type pos = executable.rfind('/');
    if (pos != std::string::npos) {
        executable.erase(0, pos + 1);
    }

    struct option long_options[] = {
        { "help", no_argument, 0, 'h' },
        { "version", no_argument, 0, 'V' },
        { "force", no_argument, 0, 'f' },
        { 0, 0, 0, 0 }
    };

    int opt;
    Clean clean;
    while ((opt = getopt_long(argc, argv, "hf", long_options, NULL)) != -1) {
        switch (opt) {
        case 'h':
            usage();
            return 0;
        case 'V':
            std::cout << executable << " version " << VERSION << std::endl;
            return 0;
        case 'f':
            clean.setForce(true);
            break;
        default:
            std::cerr << "Try `" << executable << " --help` for more information." << std::endl;
            return -1;
        }
    };

    if (optind < argc) {
        int errors = 0;
        for (int arg = optind; arg < argc; ++arg) {
            if (!clean.cleanFile(argv[arg])) {
                errors++;
            }
        }
        if (errors) {
            return -1;
        }
    } else {
        try {
            clean.visit();
        } catch (const std::exception& exc) {
            std::cerr << exc.what() << std::endl;
            return -1;
        }
    }
    return 0;
}
