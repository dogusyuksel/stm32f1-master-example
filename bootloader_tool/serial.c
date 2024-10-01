
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "crc.h"

#define VERSION "2.0.0."
#define TIMEOUT_MS 5000
#define MAX_FAIL_COUNT 5

#define PACKET_SYNCH_SIZE 3
#define PACKET_INDEX_SIZE 1
#define PACKET_CRC_SIZE 4
#define PACKET_PAYLOAD_SIZE 1024
#define MAX_BUFFER_SIZE                                                        \
  (PACKET_PAYLOAD_SIZE + PACKET_SYNCH_SIZE + PACKET_INDEX_SIZE +               \
   PACKET_CRC_SIZE)

typedef enum {
    INITIAL,
    SHOULD_CONTINUE,
    FINISHED,
    FAILED
} xstates;

sem_t mutex;
int file_desc = -1;
unsigned char fw_response[MAX_BUFFER_SIZE * 2] = {0};
unsigned char fw_response_len;

static int set_interface_attribs(int fd, int speed) {
  struct termios tty;
  (void)speed;
  memset(&tty, 0, sizeof(tty));

  if (tcgetattr(fd, &tty) != 0) {
    fprintf(stderr, "Serial tcgetattr error\n");
    return 1;
  }

  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag |= CS8;
  tty.c_cflag &= ~CSIZE;

  cfsetospeed(&tty, speed);
  cfsetispeed(&tty, speed);
  cfmakeraw(&tty);

  if (tcsetattr(fd, TCSANOW, &tty)) {
    fprintf(stderr, "Serial tcsetattr error\n");
    return 1;
  }

  return 0;
}

static void *console_listener_task(void *vargp) {
  int rc;
  char buffer[5000];
  struct pollfd fds[1];
  unsigned char byte = 0;
  int fd = -1;

  if (!vargp) {
    goto out;
  }

  fd = *((int *)vargp);

  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;

  while (1) {
    rc = poll(fds, 1, TIMEOUT_MS);
    if (rc < 0) {
      fprintf(stderr, "poll() failed");
      goto out;
    }

    if (fds[0].revents != POLLIN) {
      continue;
    }

    memset(buffer, 0, sizeof(buffer));

    while (read(fds[0].fd, &byte, sizeof(unsigned char)) == 1) {
      if (write(fd, &byte, 1) == -1) {
        fprintf(stderr, "write() failed");
      }
    }
  }

out:
  pthread_exit(NULL);
}

static void *packet_prep_task(void *vargp) {
  int fd = -1;
  static unsigned char index = 0;
  xstates should_continue = INITIAL;
  unsigned char read_buffer[MAX_BUFFER_SIZE] = {0};
  uint32_t error_counter = 0, sent_packet_counter = 0, max_packet = 0;

  if (!vargp) {
    goto out;
  }

  fd = *((int *)vargp);

  struct stat fd_stat;
  fstat(fd, &fd_stat);
  max_packet = (uint32_t)(fd_stat.st_size / PACKET_PAYLOAD_SIZE) + 1;

  while(1) {
    should_continue = INITIAL;

    sem_wait(&mutex);
    fw_response_len = 0;

    fprintf(stderr, "FROM FW ==> %s", (char *)fw_response);

    if (error_counter >= MAX_FAIL_COUNT) {
        goto cont;
    }
    
    // check the packet is broadcast packet
    if (strstr((char *)fw_response, "broadcasting") && index == 0) {
        // broadcast received, continue
        fprintf(stderr, "TOOL    ==> broadcast received\n");
        should_continue = SHOULD_CONTINUE;
    } else if (strstr((char *)fw_response, "OK")) {
        //answer of previous packet, check index
        char index_buf[8] = {0};
        snprintf(index_buf, sizeof(index_buf), "%u", (index - 1));
        if (strstr((char *)fw_response, index_buf)) {
            //it means the answer is correct
            should_continue = SHOULD_CONTINUE;
        } else {
            should_continue = FAILED;
        }
    }

    if (should_continue == SHOULD_CONTINUE) {
        memset(read_buffer, 0, sizeof(read_buffer));

        // read the file and pack it and send it
        memset(read_buffer, 0xFF, sizeof(read_buffer));

        lseek(fd, (index * PACKET_PAYLOAD_SIZE), SEEK_SET);
        int ret_read = read(file_desc, &read_buffer[4], PACKET_PAYLOAD_SIZE);
        fprintf(stderr, "TOOL    ==> read bytes %d from file\n", ret_read);

        if (ret_read <= 0) {
            if (sent_packet_counter >= max_packet) {
                //transfer complete for one time
                read_buffer[0] = (unsigned char)'K';
                read_buffer[1] = (unsigned char)'D';
                read_buffer[2] = (unsigned char)'Y';
                read_buffer[3] = index++;
                memset(&read_buffer[PACKET_SYNCH_SIZE + PACKET_INDEX_SIZE], 0xCC, PACKET_PAYLOAD_SIZE);
                write(fd, read_buffer, sizeof(read_buffer));
                should_continue = FINISHED;
                sent_packet_counter = 0;
                fprintf(stderr, "TOOL    ==> sent DONE\n");
                goto cont;
            }

            error_counter = MAX_FAIL_COUNT;
            goto cont;
        }

        read_buffer[0] = (unsigned char)'K';
        read_buffer[1] = (unsigned char)'D';
        read_buffer[2] = (unsigned char)'Y';
        read_buffer[3] = index++;

        unsigned int crc = chksum_crc32_start();
        for (uint32_t i = 0; i < PACKET_PAYLOAD_SIZE; i++) {
            crc = chksum_crc32_continue(crc, &read_buffer[i + 4], 1);
        }
        crc = chksum_crc32_end(crc);

        memcpy(&read_buffer[sizeof(read_buffer) - 4], &crc, 4);

        write(fd, read_buffer, sizeof(read_buffer));

        fprintf(stderr, "TOOL    ==> sent packet %u\n", (index - 1));
        sent_packet_counter++;
    } else if (should_continue == FAILED) {
        //send the previous packet again
        fprintf(stderr, "TOOL    ==> send the previous packet again\n");

        write(fd, read_buffer, sizeof(read_buffer));

        error_counter++;

        if (error_counter >= MAX_FAIL_COUNT) {
            fprintf(stderr, "TOOL    ==> too many errors\n");
        }
    }
cont:
    memset(fw_response, 0, sizeof(fw_response));
    fw_response_len = 0;
  }

out:

  pthread_exit(NULL);
}

static void *uart_listener_task(void *vargp) {
  int rc;
  struct pollfd fds[1];
  unsigned char byte = 0;
  int fd = -1;

  if (!vargp) {
    goto out;
  }

  fd = *((int *)vargp);

  fds[0].fd = fd;
  fds[0].events = POLLIN; // poll input events

  while (1) {
    rc = poll(fds, 1, TIMEOUT_MS);
    if (rc < 0) {
      fprintf(stderr, "poll() failed");
      goto out;
    }

    if (fds[0].revents != POLLIN) {
      continue;
    }

    while (read(fds[0].fd, &byte, sizeof(unsigned char)) == 1) {
        if (file_desc > 0) {
            fw_response[fw_response_len] = byte;
            fw_response_len++;

            if (byte == (unsigned char)'\n') {
                sem_post(&mutex);
            }
        } else {
            if (write(STDOUT_FILENO, &byte, 1) == -1) {
                fprintf(stderr, "write() failed");
            }
        }
    }
    sem_post(&mutex);
  }

out:
  pthread_exit(NULL);
}

int main(int argn, char **args) {
  char *portname = NULL;
  pthread_t tid1;
  pthread_t tid2;
  int fd = -1;

  if (argn < 2) {
    fprintf(stderr, "give port name as argument\n");
    return 1;
  }
  fprintf(stderr, "%s version: %s\n", args[0], VERSION);

  portname = args[1];

  fd = open(portname, O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "port cannot opened\n");
    return 1;
  }

  set_interface_attribs(fd, B115200);

  if (argn == 3) {
    file_desc = open(args[2], O_RDONLY | O_NONBLOCK, 0444);
    if (file_desc < 0) {
        fprintf(stderr, "open() failed\n");
        return 1;
    }

    //it means we will use this app as FW flasher
    pthread_create(&tid2, NULL, packet_prep_task, &fd);
    sleep(2); // be sure console_listener_task() is executed first
  }
  pthread_create(&tid2, NULL, console_listener_task, &fd);
  sleep(2); // be sure console_listener_task() is executed first
  pthread_create(&tid1, NULL, uart_listener_task, &fd);


  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);

  if (fd > 0) {
    close(fd);
  }
  if (file_desc > 0) {
      close(file_desc);
  }

  return 0;
}
