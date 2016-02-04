/*
 * Copyright (C) 2016 Jameson Williams <jameson@nosemaj.org>
 * 
 * This file is part of libgopro.
 *
 * libgopro is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * libgopro is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void die(const char *msg) {
    perror(msg);
    exit(0);
}

int gp_connect(const char *host, int port) {
    int sockfd = -1;
    struct hostent *server;
    struct sockaddr_in serv_addr;

    if (0 > (sockfd = socket(AF_INET, SOCK_STREAM, 0))) {
        perror("cannot create socket");
        return sockfd;
    }

    if (NULL == (server = gethostbyname(host))) {
        warn("no such host %s", host);
        close(sockfd);
        return -1;
    }

    memset(&serv_addr, 0, sizeof (serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (0 > connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
        perror("error connecting");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

gp_send_message(const int sockfd, const char *uri) {
    /* fill in the parameters */
    size_t total = 0;
    ssize_t bytes = -1;
    ssize_t sent = 0;

    char message[1024] = { '\0' };
    const char *message_fmt = "GET %s HTTP/1.0\r\n\r\n";

    sprintf(message, message_fmt, uri);
    printf("Request:\n%s\n", message);

    /* send the request */
    total = strlen(message);
    sent = 0;
    do {
        bytes = write(sockfd, message + sent, total - sent);

        if (bytes < 0) {
            perror("Error writing message to socket");
        }

        if (bytes == 0) {
            break;
        }

        sent += bytes;

    } while (sent < total);
}

gp_get_response(const int sockfd) {
    /* receive the response */
    ssize_t received = 0;
    ssize_t bytes = -1;
    char response[4096] = { '\0' };
    size_t total = sizeof(response) - 1;
    received = 0;

    do {
        bytes = read(sockfd, response + received, total - received);

        if (bytes < 0) {
            perror("Error reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        received += bytes;

    } while (received < total);

    if (received == total) {
        perror("Error storing complete response from socket");
    }

    /* process response */
    printf("Response:\n%s\n", response);
}

int main(int argc, char *argv[])
{
    const int sockfd = gp_connect("google.com", 80);
    gp_send_message(sockfd, "/");
    gp_get_response(sockfd);
    close(sockfd);

    return 0;
}

