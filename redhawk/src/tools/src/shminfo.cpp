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

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>

#include <ossie/shm/SuperblockFile.h>

#define SHMINFO_DIR "/dev/shm"

namespace {

    enum SizeFormat {
        BYTES,
        KILOBYTES,
        HUMAN_READABLE
    } size_format;

    static bool startswith(const std::string& str, const std::string& prefix)
    {
        return str.compare(0, prefix.size(), prefix) == 0;
    }

    void human_readable(std::ostream& oss, size_t size)
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

    std::string size_to_string(size_t size)
    {
        std::ostringstream oss;
        switch (size_format) {
        case BYTES:
            oss << size;
            break;
        case KILOBYTES:
            oss << (size/1024) << "K";
            break;
        case HUMAN_READABLE:
            human_readable(oss, size);
            break;
        }
        return oss.str();
    }

    static void dump_filesystem(const std::string& path)
    {
        struct statvfs status;
        if (statvfs(path.c_str(), &status)) {
            std::cout << "  (cannot stat)" << std::endl;
            return;
        }

        std::cout << "  size: " << size_to_string(status.f_blocks * status.f_frsize) << std::endl;
        std::cout << "  free: " << size_to_string(status.f_bfree * status.f_frsize) << std::endl;
    }

    static void dump_heap(const std::string& name)
    {
        using redhawk::shm::SuperblockFile;

        SuperblockFile file(name);
        try {
            file.open();
        } catch (const std::exception& exc) {
            std::cerr << "error: " << exc.what() << std::endl;
            return;
        }

        pid_t owner = file.creator();
        std::cout << "  owner:     " << owner;
        if (kill(owner, 0) != 0) {
            std::cout << " (defunct)";
        }
        std::cout << std::endl;
        std::cout << "  refcount:  " << (file.refcount() - 1) << std::endl;

        SuperblockFile::Statistics stats = file.getStatistics();
        std::cout << "  total size:  " << size_to_string(stats.size) << std::endl;
        std::cout << "  total used:  " << size_to_string(stats.used) << std::endl;
        std::cout << "  superblocks: " << stats.superblocks << std::endl;
        std::cout << "  unused:      " << stats.unused << std::endl;
    }

    static void usage()
    {
    }
}

int main(int argc, char* argv[])
{
    bool all = false;
    size_format = BYTES;
    int opt;
    while ((opt = getopt(argc, argv, "akh")) != -1) {
        switch (opt) {
        case 'a':
            all = true;
            break;
        case 'k':
            size_format = KILOBYTES;
            break;
        case 'h':
            size_format = HUMAN_READABLE;
            break;
        default:
            usage();
            exit(-1);
        }
    };

    std::string shminfo_dir = SHMINFO_DIR;

    std::cout << shminfo_dir << ':' << std::endl;
    dump_filesystem(shminfo_dir);

    DIR* dir = opendir(shminfo_dir.c_str());
    if (!dir) {
        std::cerr << "could not open shm filesystem " << shminfo_dir << std::endl;
        return -1;
    }

    while (struct dirent* entry = readdir(dir)) {
        const std::string filename = entry->d_name;
        if ((filename == ".") || (filename == "..")) {
            continue;
        }
        bool is_heap = startswith(filename, "heap-");
        if (!is_heap && !all) {
            continue;
        }

        std::string path = shminfo_dir + "/" + filename;
        std::cout << std::endl << path << ":" << std::endl;;

        struct stat status;
        if (stat(path.c_str(), &status)) {
            std::cout << "  (cannot access)" << std::endl;
            continue;
        }
        std::cout << "  file size: " << size_to_string(status.st_size) << std::endl;
        std::cout << "  allocated: " << size_to_string(status.st_blocks * 512) << std::endl;
        std::cout << "  type:      ";

        if (is_heap) {
            std::cout << "REDHAWK heap" << std::endl;
            dump_heap(filename);
        } else {
            std::cout << "other" << std::endl;
        }
    }
    closedir(dir);

    return 0;
}
