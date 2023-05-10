#include "server.h"

int listen_fd;
struct io_uring _ring;

void on_message_send(struct uring_event * event, int outfd) {
  char * line = event->buffer;
  int size = strlen(line);
  printf("Recieved = %s\n", line);
  write(outfd, line, size);
  submit_write(event);
}

void server_run(int outfd, int port) {
  struct io_uring_cqe *cqe;
  init_listenfd(port);
  struct uring_event * event = (struct uring_event *) malloc(sizeof(struct uring_event));
  event->fd = listen_fd;
  event->client_addr_len = sizeof(struct sockaddr_storage);

  submit_accept(event);

  while (true) {
    int err = io_uring_wait_cqe(&_ring, &cqe);
    if (err < 0) {
      printf("io_uring_wait_cqe error");
      exit(1);
    }
    struct uring_event * user_event = (struct uring_event *)cqe->user_data;
    if (cqe->res < 0) {
      printf("io_uring_wait_cqe error %d\n", errno);
      exit(1);
    }

    if (user_event->type == ACCEPT) {
      struct uring_event * read_event = (struct uring_event *) malloc(sizeof(struct uring_event));
      read_event->type = READ;
      read_event->fd = cqe->res;
      read_event->client_addr = event->client_addr;
      read_event->client_addr_len = event->client_addr_len;

      submit_accept(event);
      submit_read(read_event);
    } else if (user_event->type == READ) {
      if (!cqe->res) {
	io_uring_cqe_seen(&_ring, cqe);
	continue;
      }
      on_message_send(user_event, outfd);
    } else if (user_event->type == WRITE) {
      close(user_event->fd);
      sleep(3);
      free(user_event);
    } else {
    }
    io_uring_cqe_seen(&_ring, cqe);
  }
}

void submit_accept(struct uring_event * event) {
  struct io_uring_sqe *sqe = io_uring_get_sqe(&_ring);
  event->type = ACCEPT;
  io_uring_prep_accept(sqe, event->fd, (struct sockaddr *) &event->client_addr,
		       &event->client_addr_len, 0);
  io_uring_sqe_set_data(sqe, event);
  io_uring_submit(&_ring);
}

void submit_read(struct uring_event * event) {
  struct io_uring_sqe *sqe = io_uring_get_sqe(&_ring);
  event->type = READ;
  io_uring_prep_read(sqe, event->fd, &event->buffer, BUFFER_SIZE, 0);
  io_uring_sqe_set_data(sqe, event);
  io_uring_submit(&_ring);
}

void submit_write(struct uring_event * event) {
  struct io_uring_sqe *sqe = io_uring_get_sqe(&_ring);
  event->type = WRITE;
  const char * accept_msg = "ACCEPTED\n";
  io_uring_prep_write(sqe, event->fd, accept_msg, strlen(accept_msg), 0);
  io_uring_sqe_set_data(sqe, event);
  io_uring_submit(&_ring);
}

void init_listenfd(int port) {
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(port);
  int on = 1;
  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  int err = bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
  if (err < 0) {
    printf("bind error\n");
    exit(1);
  }

  err = listen(listen_fd, 10);
  if (err < 0) {
    printf("listen error\n");
    exit(1);
  }
}

