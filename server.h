#ifndef SERVER_H
#define SERVER_H
#include <liburing.h>
#include <stdio.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

#define QS 10
#define BUFFER_SIZE 128
#define FILENAMESIZE 40

extern int listen_fd;
extern struct io_uring _ring;

enum etype {
  ACCEPT,
  READ,
  WRITE
};

struct uring_event {
  enum etype type;
  int fd;
  struct sockaddr_in client_addr;
  socklen_t client_addr_len;
  char buffer[BUFFER_SIZE];
  void (* send) (struct uring_event *);
};

void submit_accept(struct uring_event * event);
void submit_read(struct uring_event * event);
void submit_write(struct uring_event * event);
void init_listenfd(int port);
void on_message_send(struct uring_event * event, int outfd);
void server_run(int outfd, int port);

#endif
