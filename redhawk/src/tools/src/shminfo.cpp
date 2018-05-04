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
#include <stdexcept>
#include <algorithm>
#include <cmath>

#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <pwd.h>
#include <grp.h>

#include <ossie/shm/System.h>

#include "ShmVisitor.h"

using redhawk::shm::SuperblockFile;

namespace {
    static std::string executable;

    static void usage()
    {
        std::cout << "Usage: " << executable << " [OPTION]..." << std::endl;
        std::cout << "Display status of shared memory file system and REDHAWK heaps." << std::endl;
        std::cout << std::endl;
        std::cout << "  -h, --help           display this help and exit" << std::endl;
        std::cout << "  -a, --all            include non-REDHAWK heap shared memory files" << std::endl;
        std::cout << "  -l                   do not look up user and group names" << std::endl;
        std::cout << "      --format=FORMAT  display sizes in FORMAT (default 'auto')" << std::endl;
        std::cout << "      --version        output version information and exit" << std::endl;
        std::cout << std::endl;
        std::cout << "FORMAT may be one of the following:" << std::endl;
        std::cout << "  auto        use human readable sizes (e.g., 4K, 2.4G)" << std::endl;
        std::cout << "  b           bytes" << std::endl;
        std::cout << "  k           kilobytes" << std::endl;
    }

    static std::string percent(float percent)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << percent << "%";
        return oss.str();
    }
}

class SizeFormatter
{
public:
    enum Format {
        BYTES,
        KILOBYTES,
        AUTO
    };

    explicit SizeFormatter(Format format=AUTO) :
        _format(format)
    {
    }

    static Format Parse(const std::string& strval)
    {
        std::string format_str;
        std::transform(strval.begin(), strval.end(), std::back_inserter(format_str), ::tolower);
        if (format_str == "auto") {
            return AUTO;
        } else if (format_str == "k") {
            return KILOBYTES;
        } else if (format_str == "b") {
            return BYTES;
        } else {
            throw std::invalid_argument("invalid format '" + strval + "'");
        }
    }

    void setFormat(Format format)
    {
        _format = format;
    }

    void setFormat(const std::string& strval)
    {
        setFormat(SizeFormatter::Parse(strval));
    }

    std::string operator() (size_t size)
    {
        std::ostringstream oss;
        switch (_format) {
        case KILOBYTES:
            oss << (size/1024) << "K";
            break;
        case AUTO:
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
        _showNames(true),
        _format()
    {
    }

    void setDisplayAll(bool all)
    {
        _all = all;
    }

    void setDisplayUserNames(bool display)
    {
        _showNames = display;
    }

    void setSizeFormatter(SizeFormatter& format)
    {
        _format = format;
    }

    virtual void visitHeap(SuperblockFile& heap)
    {
        std::cout << std::endl << heap.name() << std::endl;;
        std::cout << "  type:        REDHAWK heap" << std::endl;
        // Note: file size and allocated size are always the same for heaps
        std::cout << "  file size:   " << _format(heap.file().size()) << std::endl;

        SuperblockFile::Statistics stats = heap.getStatistics();
        float used_pct = stats.used * 100.0 / stats.size;
        std::cout << "  heap size:   " << _format(stats.size) << std::endl;
        std::cout << "  heap used:   " << _format(stats.used) << " (" << percent(used_pct) << ")" << std::endl;

        pid_t creator = heap.creator();
        std::cout << "  creator:     " << creator << std::endl;
        std::cout << "  orphaned:    " << std::boolalpha << heap.isOrphaned() << std::endl;
        std::cout << "  refcount:    " << heap.refcount() << std::endl;

        displayFileStats(heap.name(), false);
    }

    virtual void visitFile(const std::string& name)
    {
        if (!_all) {
            return;
        }

        std::cout << std::endl << name << std::endl;;
        std::cout << "  type:        other" << std::endl;

        displayFileStats(name, true);
    }

    virtual void heapException(const std::string& name, const std::exception& exc)
    {
        std::cout << std::endl << name << std::endl;;
        std::cout << "  type:        REDHAWK heap (unreadable)" << std::endl;

        displayFileStats(name, true);

        std::cerr << "  error:       " << exc.what() << std::endl;
    }

protected:
    void displayFileStats(const std::string& name, bool showSize)
    {
        const std::string shm_path = redhawk::shm::getSystemPath();
        std::string path = shm_path + "/" + name;
        struct stat status;
        if (stat(path.c_str(), &status)) {
            return;
        }
        if (showSize) {
            std::cout << "  file size:   " << _format(status.st_size) << std::endl;
            std::cout << "  allocated:   " << _format(status.st_blocks * 512) << std::endl;
        }
        std::cout << "  user:        " << _getUserName(status.st_uid) << std::endl;
        std::cout << "  group:       " << _getGroupName(status.st_gid) << std::endl;
        int mode = status.st_mode & (S_IRWXU|S_IRWXG|S_IRWXO);
        std::cout << "  mode:        " << std::oct << mode << std::endl;
    }

    std::string _getUserName(uid_t uid)
    {
        UserTable::iterator existing = _userNames.find(uid);
        if (existing != _userNames.end()) {
            return existing->second;
        }

        std::string name;

        // Unless it's disabled, look up the user entry
        if (_showNames) {
            struct passwd* user = getpwuid(uid);
            if (user) {
                name = user->pw_name;
            }
        }

        // Use the ID
        if (name.empty()) {
            std::ostringstream oss;
            oss << uid;
            name = oss.str();
        }

        _userNames[uid] = name;
        return name;
    }

    std::string _getGroupName(gid_t gid)
    {
        GroupTable::iterator existing = _groupNames.find(gid);
        if (existing != _groupNames.end()) {
            return existing->second;
        }

        std::string name;

        // Unless it's disabled, look up the group entry
        if (_showNames) {
            struct group* grp = getgrgid(gid);
            if (grp) {
                name = grp->gr_name;
            }
        }

        // Use the ID
        if (name.empty()) {
            std::ostringstream oss;
            oss << gid;
            name = oss.str();
        }

        _groupNames[gid] = name;
        return name;
    }

    bool _all;
    bool _showNames;
    SizeFormatter _format;

    typedef std::map<uid_t,std::string> UserTable;
    UserTable _userNames;

    typedef std::map<gid_t,std::string> GroupTable;
    GroupTable _groupNames;
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
        { "format", required_argument, 0, 'f' },
        { "help", no_argument, 0, 'h' },
        { "all", no_argument, 0, 'a' },
        { "version", no_argument, 0, 'V' },
        { 0, 0, 0, 0 }
    };

    Info info;
    SizeFormatter format;

    int opt;
    while ((opt = getopt_long(argc, argv, "ahl", long_options, NULL)) != -1) {
        switch (opt) {
        case 'a':
            info.setDisplayAll(true);
            break;
        case 'f':
            try {
                format.setFormat(optarg);
            } catch (const std::exception& exc) {
                std::cerr << exc.what() << std::endl;
                return -1;
            }
            break;
        case 'l':
            info.setDisplayUserNames(false);
            break;
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

    if (optind < argc) {
        std::cerr << "WARNING: arguments ignored" << std::endl;
    }

    std::cout << redhawk::shm::getSystemPath() << std::endl;
    size_t total_mem = redhawk::shm::getSystemTotalMemory();
    size_t free_mem = redhawk::shm::getSystemFreeMemory();
    float free_pct = free_mem * 100.0 / total_mem;
    std::cout << "  size: " << format(total_mem) << std::endl;
    std::cout << "  free: " << format(free_mem) << " (" << percent(free_pct) << ")" << std::endl;

    info.setSizeFormatter(format);
    try {
        info.visit();
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << std::endl;
        return -1;
    } 
    return 0;
}
