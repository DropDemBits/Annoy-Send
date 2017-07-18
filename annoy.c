/*
 * Copyright (C) 2017 DropDemBits <r3usrlnd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#if defined(_POSIX_C_SOURCE) || defined(__CYGWIN__)
#   include <signal.h>
#   include <unistd.h>
#   include <arpa/inet.h>
#   include <netinet/in.h>
#   include <netinet/ip.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <sys/stat.h>
#   include <netdb.h>
#   include <fcntl.h>
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#   error "There is no port for Windows yet, just use CYGWIN or use Windows Subsystem for Linux"
#else
#   error "There is no port available for your operating system"
#endif

#define FILENAME_ARG 1
#define URL_ARG 2
#define PORT_ARG 3
#define FORMAT_ARG 4
#define COOLDOWN_ARG 5
#define MAX_ARGC (COOLDOWN_ARG + 2)

#ifdef __DEBUG__
#define DEBUG(format, ...) \
do { fprintf(stdout, format, __VA_ARGS__); } while(0);
#else
#define DEBUG(format, ...)
#endif

char* strnrepl(char* src, const char regx, const char* rep, size_t len);
void printUsage(void);
void init_os_specific(void);
#if defined(_POSIX_C_SOURCE) || defined(__CYGWIN__)
void handle_signal(int signal);
#else

#endif

const char* URL = "annoy.michaelreeves.us";
const char* PORT = "2888";
const char* GET_FORMAT = 
"GET /annoy?message=%s HTTP/1.0\r\n"
"\r\n";

static volatile int should_loop = 1; 

int total_bytes_sent = 0;
char ip_string[INET_ADDRSTRLEN];
char read_buffer[512];
char send_buffer[512];
char recv_buffer[512];

int main(int argc, const char** argv)
{
#if defined(_POSIX_C_SOURCE) || defined(__CYGWIN__)
    struct sockaddr_in socket_addr;
    struct addrinfo hints, *results, *ai_info;
    int socketfd, filefd;
#endif
    int status, bytes_sent, bytes_recieved, bytes_read, cooldown = 6;
    char *url = URL, *format = GET_FORMAT, *port = PORT;
    const char* pre_payload = "GET /annoy?message=%20 HTTP/1.0\r\n\r\n";

    if( argc > MAX_ARGC || argc == 1 ) {
        printUsage();
    }
    if( argc >= URL_ARG + 2 ) url = argv[URL_ARG];
    if( argc >= PORT_ARG + 2 ) port = argv[PORT_ARG];
    if( argc >= FORMAT_ARG + 2 ) format = argv[FORMAT_ARG];
    if( argc >= COOLDOWN_ARG + 2 ) cooldown = argv[COOLDOWN_ARG];
    
    printf("Args: %s\n", argv[FILENAME_ARG]);
    init_os_specific();
    
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    memset(&socket_addr, 0, sizeof socket_addr);
    
    if( (status = getaddrinfo(url, (const char*)port, &hints, &results)) ) {
        fprintf(stderr, "Error: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }
    
    for( ai_info = results; ai_info != NULL; ai_info = ai_info->ai_next ) {
        if( ai_info->ai_family != AF_INET ) continue;
        socket_addr.sin_addr = ((struct sockaddr_in*) ai_info->ai_addr)->sin_addr;
        socket_addr.sin_port = ((struct sockaddr_in*) ai_info->ai_addr)->sin_port;
        
        inet_ntop(AF_INET, &(socket_addr.sin_addr), ip_string, INET_ADDRSTRLEN);
        printf("Connecting to address %s:%d...\n", ip_string, ntohs(socket_addr.sin_port));
        
        if( (socketfd = socket(ai_info->ai_family, ai_info->ai_socktype, getprotobyname("tcp")->p_proto)) == -1 ) {
            fprintf(stderr, "Error: %s\n", strerror(errno));
            continue;
        }
        
        if( connect(socketfd, ai_info->ai_addr, ai_info->ai_addrlen) == -1 ) {
            fprintf(stderr, "Error: %s\n", strerror(errno));
            continue;
        }
        
        break;
    }
    
    //Send precursor message
    bytes_sent = send(socketfd, pre_payload, strlen(pre_payload), 0);
    DEBUG("%d, ", bytes_sent);
    bytes_recieved = recv(socketfd, recv_buffer, 511, 0);
    DEBUG("%d, %s\n", bytes_recieved, recv_buffer);
    
    close(socketfd);
        
    if( (socketfd = socket(ai_info->ai_family, ai_info->ai_socktype, getprotobyname("tcp")->p_proto)) == -1 ) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    if( connect(socketfd, ai_info->ai_addr, ai_info->ai_addrlen) == -1 ) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    //Prepare sending...
    if( (filefd = open(argv[FILENAME_ARG], O_RDONLY)) < 0) {
        int errsv = errno;
        fprintf(stderr, "Error: %s\n", strerror(errsv));
        exit(EXIT_FAILURE);
    }
    
    //Release Payload
    do {
        bytes_read = read(filefd, read_buffer, 50);
        if(bytes_read == 0) break;
        else if(bytes_read < 0) {
            int errsv = errno;
            fprintf(stderr, "Error: %s\n", strerror(errsv));
            exit(EXIT_FAILURE);
        }
        
        printf("%s", read_buffer);
        strnrepl(read_buffer, ' ', "%20", 256);
        sprintf(send_buffer, (const char*)format, read_buffer);
        
        bytes_sent = send(socketfd, send_buffer, strlen(send_buffer), 0);
        DEBUG("%d, ", bytes_sent);
        bytes_recieved = recv(socketfd, recv_buffer, 511, 0);
        DEBUG("%d, %s\n", bytes_recieved, recv_buffer);
        total_bytes_sent = bytes_sent;
        memset(read_buffer, 0, strlen(read_buffer));
        memset(send_buffer, 0, strlen(send_buffer));
        
        close(socketfd);
        
        if( (socketfd = socket(ai_info->ai_family, ai_info->ai_socktype, getprotobyname("tcp")->p_proto)) == -1 ) {
            fprintf(stderr, "Error: %s\n", strerror(errno));
            continue;
        }
        
        if( connect(socketfd, ai_info->ai_addr, ai_info->ai_addrlen) == -1 ) {
            fprintf(stderr, "Error: %s\n", strerror(errno));
            continue;
        }
        
        //Do a cooldown
        sleep(cooldown);
    } while( bytes_read > 0 && should_loop );
    
    //Cleanup
#if defined(_POSIX_C_SOURCE) || defined(__CYGWIN__)
    close(filefd);
    close(socketfd);
    freeaddrinfo(results);
#endif
    
    printf("\nBytes sent: %d\n", total_bytes_sent);
    
    return EXIT_SUCCESS;
}

char* strnrepl(char* str, const char regx, const char* rep, size_t len)
{
    if(str == NULL || rep == NULL || len == 0) return str;
    
    char* temp;
    size_t index = 0;
    size_t store_len = 0;
    
    if(strlen(str) >= len) return str;
    if( (temp = malloc(len)) == NULL ) return str;
    
    do {
        if( str[index] != regx ) continue;
        DEBUG("%c\n", str[index]);
        strncpy(temp, str + (index+1), (store_len = strlen(str) - index));
        DEBUG("%s, %lu\n", temp, store_len);
        if( strlen(rep) >= len-(index+1) ) goto abort;
        DEBUG("nabort (%lu, %lu)\n", strlen(rep), len-(index+1));
        strncpy(str + index, rep, strlen(rep));
        DEBUG("%s\n", str);
        strncpy(str + (index+strlen(rep)), temp, store_len);
        DEBUG("%s\n", str);
    } while(str[++index] && index < len);
    
    abort:
    free(temp);
    temp = NULL;
    return str;
}

void printUsage()
{
    printf("Usage: annoy <file> [url] [port] [format] [cooldown]\n");
    exit(EXIT_FAILURE);
}

#if defined(_POSIX_C_SOURCE) || defined(__CYGWIN__)
void handle_signal(int signal)
{
    switch(signal) {
    case SIGHUP:
    case SIGINT:
        should_loop = 0;
        return;
    default:
        break;
    }
}

void init_os_specific()
{
    struct sigaction sa;
    sa.sa_handler = &handle_signal;
    sa.sa_flags = SA_RESTART;
    sigfillset(&sa.sa_mask);
    
    if( sigaction(SIGHUP, &sa, NULL) == -1 ) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
    }
    
    if( sigaction(SIGINT, &sa, NULL) == -1 ) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
    }
    
}
#else
void init_os_specific () { }
#endif
