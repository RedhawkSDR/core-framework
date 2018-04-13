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

using redhawk::shm::SuperblockFile;

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
    Clean clean;
    try {
        clean.visit();
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << std::endl;
        return -1;
    }
    return 0;
}
