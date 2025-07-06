#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include "libtelnet.h"

#define LOG_INFO(info, ...) printf("[\033[32mINFO\033[0m]  "info"\n", ##__VA_ARGS__)
#define LOG_WAR(info, ...) printf("[\033[33mWARN\033[0m]  "info"\n", ##__VA_ARGS__)
#define LOG_ERR(info, ...) printf("[\033[31mERROR\033[0m] "info"\n", ##__VA_ARGS__)

typedef struct {
    uint8_t *buffer;
    uint8_t size;
} frame_t;


frame_t* parse_frame() {
    frame_t frame = {
        .buffer = calloc(1, 32),
        .size = 32
    };


}

void finde_RTT_signature(frame_t frame) {

}

void telnet_callback(telnet_t *telnet, telnet_event_t *ev, void *user_data) {
    switch (ev->type) {
        case TELNET_EV_SEND: {
            LOG_INFO("SEND EVENT");
            send(*(int*)user_data, ev->data.buffer, ev->data.size, 0);
            break;
        }

        case TELNET_EV_DATA: {
            LOG_INFO("RECV");
            fwrite(ev->data.buffer, 1, ev->data.size, stdout);
            fflush(stdout);
            
            break;
        }

        case TELNET_EV_ERROR: {
            fprintf(stderr, "Telnet error: %s\n", ev->error.msg);
            return;
        }
        default: { } 
    }
}

int open_tcp(uint16_t port) { 
    struct sockaddr_in server_addr; 
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        LOG_ERR("Socket cant be open");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        LOG_ERR("inet local host fuck you");
        return -1;
    }

    int conn = connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (conn != 0) {
        LOG_ERR("Connection fuck you");
        return -1;
    }

    LOG_INFO("Establish connection: ");
    return  fd; 
}

int main() {
    LOG_INFO("Terminal Start...%d", 1);

    int fd = open_tcp(4444);
    
    telnet_telopt_t telopt;
    telnet_t *htel = telnet_init(&telopt, telnet_callback, 0, &fd);

    telnet_send(htel, "mdb 0x200006c0 124\n", 19);
    
    char buffer[512] = {0};
    int size = 0;
    
    LOG_INFO("RECV...");
    while (( size = recv(fd, buffer, sizeof(buffer), 0)) > 0) {
        telnet_recv(htel, buffer, size);
    }

    close(fd);
    return 0;
}



