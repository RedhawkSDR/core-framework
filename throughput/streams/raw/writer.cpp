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
#include <vector>
#include <cstdlib>
#include <cstdio>

#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include <timing.h>

#include "control.h"

static volatile bool running = true;

static void sigint_received(int /*unused*/)
{
    running = false;
}

int main(int argc, const char* argv[])
{
    if (argc < 3) {
        exit(1);
    }

    // Set up a signal handler so that SIGINT will trigger a close and exit
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_received;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    int sockfd;
    if (strcmp(argv[1], "unix") == 0) {
        sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("socket");
            exit(1);
        }

        struct sockaddr_un server;
        server.sun_family = AF_UNIX;
        snprintf(server.sun_path, sizeof(server.sun_path), "@writer-%d", getpid());
        socklen_t len = strlen(server.sun_path) + sizeof(server.sun_family);
        std::cout << server.sun_path << std::endl;
        server.sun_path[0] = '\0';
        bind(sockfd, (struct sockaddr*)&server, len);
    } else if (strcmp(argv[1], "tcp") == 0) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("socket");
            exit(1);
        }

        struct sockaddr_in server;
        memset(&server, 0, sizeof(sockaddr_in));
        server.sin_family = AF_INET;
        server.sin_port = 0;
        server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (bind(sockfd, (struct sockaddr*)&server, sizeof(server)) < 0) {
            perror("bind");
            exit(1);
        }

        socklen_t len = sizeof(server);
        if (getsockname(sockfd, (struct sockaddr*)&server, &len) < 0) {
            perror("getsockname");
            exit(1);
        }

        std::cout << inet_ntoa(server.sin_addr) << ":" << server.sin_port << std::endl;
    } else {
        std::cerr << "Unknown protocol '" << argv[1] << "'" << std::endl;
        exit(1);
    }

    listen(sockfd, 1);

    control* state = open_control(argv[2]);

    int fd = accept(sockfd, NULL, NULL);
    if (fd < 0) {
        exit(1);
    }

    std::vector<char> buffer;

    char temp;
    std::cin.get(temp);

    size_t total_packets = 0;
    double total_seconds = 0.0;

    while (running) {
        size_t buffer_size = state->transfer_size;
        if (buffer_size != buffer.size()) {
            buffer.resize(buffer_size);
            total_packets = 0;
            total_seconds = 0.0;
            state->average_time = 0.0;
        }
        double start = get_time();
        write(fd, &buffer_size, sizeof(buffer_size));
        ssize_t pass = write(fd, &buffer[0], buffer.size());
        double end = get_time();
        total_packets++;
        total_seconds += end - start;
        state->average_time = total_seconds / total_packets;
        state->total_bytes += pass;
    }

    // Close the socket so the reader knows no more data is coming
    close(fd);

    close_control(state);

    exit(0);
}
