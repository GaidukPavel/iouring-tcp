#include "server.h"

int main(int argc, char **argv) {
  int port = atoi(argv[1]);
  char fname[FILENAMESIZE];
  int outfd;
  snprintf(fname, FILENAMESIZE, "%s.txt", argv[1]);
  outfd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);

  if (outfd < 0) {
    perror("open file");
    return 1;
  }

  io_uring_queue_init(QS, &_ring, 0);

  server_run(outfd, port);

  close(outfd);
  return 0;
}
