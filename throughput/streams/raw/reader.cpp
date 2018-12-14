/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK throughput.
 *
 * REDHAWK throughput is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK throughput is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <deque>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include <omnithread.h>

#include <timing.h>
#include <threaded_deleter.h>

#include "control.h"

int connect_unix(const std::string& address)
{
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_un writer;
    writer.sun_family = AF_UNIX;
    strcpy(writer.sun_path, address.c_str());
    int len = strlen(writer.sun_path) + sizeof(writer.sun_family);
    if (writer.sun_path[0] == '@') {
        writer.sun_path[0] = '\0';
    }
    if (connect(fd, (struct sockaddr*)&writer, len) < 0) {
        perror("connect");
        return -1;
    }
    return fd;
}

int connect_tcp(const std::string& address)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    std::string::size_type isep = address.find(':');
    server.sin_port = atoi(address.substr(isep+1).c_str());
    inet_aton(address.substr(0, isep).c_str(), &server.sin_addr);

    if (connect(fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect");
        return -1;
    }
    return fd;
}

int connect(const std::string& protocol, const std::string& address)
{
    if (protocol == "unix") {
        return connect_unix(address);
    } else if (protocol == "tcp") {
        return connect_tcp(address);
    } else {
        std::cerr << "Unknown protocol '" << protocol << "'" << std::endl;
        return -1;
    }
}

size_t read_buffer(int fd, char* buffer, size_t count)
{
    size_t bytes_read = 0;
    while (bytes_read < count) {
        ssize_t pass = read(fd, &buffer[bytes_read], count-bytes_read);
        if (pass <= 0) {
            if (pass < 0) {
                perror("read");
            }
            break;
        }
        bytes_read += pass;
    }

    return bytes_read;
}

int main(int argc, const char* argv[])
{
    if (argc < 4) {
        exit(1);
    }

    int fd = connect(argv[1], argv[2]);
    if (fd < 0) {
        exit(1);
    }

    threaded_deleter deleter;

    control* state = open_control(argv[3]);

    size_t total_packets = 0;
    double total_seconds = 0.0;
    size_t last_size = 0;

    ssize_t count = 0;
    while (true) {
        double start = get_time();

        size_t buffer_size = 0;
        if (read(fd, &buffer_size, sizeof(buffer_size)) < sizeof(buffer_size)) {
            break;
        }

        if (buffer_size != last_size) {
            last_size = buffer_size;
            total_packets = 0;
            total_seconds = 0.0;
            state->average_time = 0.0;
        }

        char* buffer = new char[buffer_size];
        size_t pass = read_buffer(fd, buffer, buffer_size);
        deleter.deallocate_array(buffer);
        if (pass == 0) {
            break;
        }
        double end = get_time();

        total_packets++;
        total_seconds += end - start;
        state->average_time = total_seconds / total_packets;
        state->total_bytes += pass;
    }

    close(fd);

    return 0;
}
