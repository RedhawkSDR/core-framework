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

#include <sys/types.h>
#include <signal.h>
#include <getopt.h>

using redhawk::shm::SuperblockFile;

namespace {
    static std::string executable;

    static void usage()
    {
        std::cout << "Usage: " << executable << " [OPTION]..." << std::endl;
        std::cout << "Remove orphaned REDHAWK heaps." << std::endl;
        std::cout << std::endl;
        std::cout << "  -h, --help           display this help and exit" << std::endl;
        std::cout << "      --version        output version information and exit" << std::endl;
        std::cout << std::endl;
        std::cout << "A REDHAWK heap is considered orphaned if its creating process is no longer" << std::endl;
        std::cout << "alive. The shared memory file is unlinked; however, if any active processes" << std::endl;
        std::cout << "have the memory mapped, the shared memory is not freed until they exit." << std::endl;
    }
}

class Clean : public ShmVisitor
{
public:
    virtual void visitHeap(SuperblockFile& heap)
    {
        pid_t creator = heap.creator();
        if (kill(creator, 0) != 0) {
            std::cout << "unlinking " << heap.name() << std::endl;
            try {
                heap.file().unlink();
            } catch (const std::exception& exc) {
                std::cerr << "error: " << exc.what() << std::endl;
            }
        }
    }
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
        { 0, 0, 0, 0 }
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "h", long_options, NULL)) != -1) {
        switch (opt) {
        case 'h':
            usage();
            return 0;
        case 'V':
            std::cout << executable << " version " << VERSION << std::endl;
            return 0;
        default:
            std::cerr << "Try `" << executable << " --help` for more information." << std::endl;
            return -1;
        }
    };

    Clean clean;
    try {
        clean.visit();
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << std::endl;
        return -1;
    }
    return 0;
}
