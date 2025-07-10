#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "libtelnet.h"

#define LOG_INFO(info, ...) printf("[\033[32mINFO\033[0m]  "info"\n", ##__VA_ARGS__)
#define LOG_WAR(info, ...) printf("[\033[33mWARN\033[0m]  "info"\n", ##__VA_ARGS__)
#define LOG_ERR(info, ...) printf("[\033[31mERROR\033[0m] "info"\n", ##__VA_ARGS__)

typedef struct {
    uint8_t *buffer;
    uint8_t size;
} frame_t;

typedef struct  {
    int fd;
} connection_context_t;

typedef struct {
    size_t len;
    uint8_t *buff;
} buff_t;


void free_rx_buff(buff_t rx) {
    free(rx.buff);
}

uint32_t check_sum(char* buff) {
    uint8_t sum = 0;
    while (*buff) {
        sum += *buff;
        buff++;
    }
    return sum;
}

buff_t paylaod_command(const char* command, ...) {
    char tmp_buffer[124] = {0};
    buff_t ret;

    va_list args;
    va_start(args, command);
    ret.len = vsnprintf(tmp_buffer, sizeof(tmp_buffer), command, args);
    va_end(args);

    sprintf(tmp_buffer+ret.len, "#%d", check_sum(tmp_buffer));

    ret.len = strlen(tmp_buffer);
    ret.buff = malloc(ret.len + 1);

    ret.buff[0] = '$';
    memcpy(ret.buff + 1, tmp_buffer, ret.len);
    memset(tmp_buffer, 0, 124);

    return ret;
}

buff_t dump_ram(int fd, uint32_t addr, uint32_t len) {
    buff_t tx = paylaod_command("m%x,%x", addr, len);
    send(fd, tx.buff, tx.len, 0);
    LOG_INFO("send: %s", tx.buff);
    free_rx_buff(tx);

    char tmp_buff[1024] = {0};
    int ret = recv(fd, tmp_buff, sizeof(tmp_buff), 0);
    LOG_INFO("recv: %s", tmp_buff);
    if (ret <= 0) {
        LOG_ERR("Recived %d", ret);
        return (buff_t){.len = 0, NULL};
    }

    buff_t rx = {
        .len = ret,
        .buff = malloc(ret+1),
    };
    
    memcpy(rx.buff, tmp_buff, rx.len);
    rx.buff[ret] = 0;

    return rx;
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

// mdb 0x200006c0 124   

int main() {
    LOG_INFO("Terminal Start...%d", 1);
    
    connection_context_t conn = {0};
    conn.fd = open_tcp(3333);

    char buffer[512] = {0};
    int size = 0;

    buff_t tx = paylaod_command("monitor reset halt");
    send(conn.fd, tx.buff, tx.len, 0);
    LOG_INFO("send: %s", tx.buff);
    free_rx_buff(tx);
    
    char buff[1024] = {0};
    recv(conn.fd, buff, sizeof(buff), 0);
    LOG_INFO("recv: %s", buff);


    // buff_t rx = paylaod_command("m%x,%x", 0x200006c0, 124);
    buff_t rx = dump_ram(conn.fd, 0x200006c0, 124);
    LOG_INFO("recv: %s", rx.buff);

    close(conn.fd);
    return 0;
}



