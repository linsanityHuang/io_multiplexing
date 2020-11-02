#include "../common.h"

#define INIT_SIZE 128

int main(int argc, char **argv)
{
    int listen_fd, socket_fd;
    int ready_number;
    ssize_t n;
    char buf[MAXLINE];
    struct sockaddr_in client_addr;

    listen_fd = tcp_server_listen(SERV_PORT);

    struct pollfd event_set[INIT_SIZE];
    event_set[0].fd = listen_fd;
    event_set[0].events = POLLIN; // POLLRDNORM

    int i;
    for (i = 1; i < INIT_SIZE; i++)
    {
        event_set[i].fd = -1;
    }

    for (;;)
    {
        if ((ready_number = poll(event_set, INIT_SIZE, -1)) < 0)
        {
            error(1, errno, "poll failed");
        }

        if (event_set[0].revents & POLLIN)
        {
            socklen_t client_len = sizeof(client_addr);
            socket_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);

            for (i = 1; i < INIT_SIZE; i++)
            {
                if (event_set[i].fd < 0)
                {
                    event_set[i].fd = socket_fd;
                    event_set[i].events = POLLIN;
                    break;
                }
            }

            if (i == INIT_SIZE)
            {
                error(1, errno, "can not hold so many clients");
            }

            if (--ready_number <= 0)
                continue;
        }

        for (i = 1; i < INIT_SIZE; i++)
        {
            if ((socket_fd = event_set[i].fd) < 0)
                continue;
            if (event_set[i].revents & (POLLIN | POLLERR))
            {
                if ((n = read(socket_fd, buf, MAXLINE)) > 0)
                {
                    if (write(socket_fd, buf, n) < 0)
                    {
                        error(1, errno, "write error");
                    }
                }
                else if (n == 0 || errno == ECONNRESET)
                {
                    close(socket_fd);
                    event_set[i].fd = -1;
                }
                else
                {
                    error(1, errno, "read error");
                }

                if (--ready_number <= 0)
                    break;
            }
        }
    }
}