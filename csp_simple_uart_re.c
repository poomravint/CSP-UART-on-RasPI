#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <csp/csp.h>
#include <csp/drivers/usart.h>

#define SERVER_ADDR 1  //*Server Address
#define SERVER_PORT 10 //*Server Port

bool done = false;

// TODO : gcc csp_simple_uart_re.c -lcsp -lpthread -o cspsimpleuartre

void *do_route(void *arg)
{
    while (!done)
    {
        csp_route_work(); //* Route packet from the incoming router queue
    }
}

//! Config Uart !//
csp_usart_conf_t conf = {
    .device = "/dev/serial0",
    .baudrate = 115200,
    .databits = 8,
    .stopbits = 1,
    .paritysetting = 0,
};

int main(int argc, char *argv[])
{
    csp_iface_t *iface;      //* For set interface
    csp_socket_t sock = {0}; //* Create socket with no specific socket options, e.g. accepts CRC32, HMAC, etc. if enabled during compilation
    pthread_t router_thread; //* For use Pthread
    int ret;
    csp_conf.version = 1; //! Set Version !!!!!!!!!!!!!!!!!!!!!
    csp_init();           //* Start CSP

    ret = csp_usart_open_and_add_kiss_interface(&conf, CSP_IF_KISS_DEFAULT_NAME, SERVER_ADDR, &iface); //* Start Uart
    //* If error to start UART will exit this code *//
    if (ret != CSP_ERR_NONE)
    {
        printf("open failed\n");
        exit(1);
    }
    iface->addr = SERVER_ADDR; //* Set Server's address
    iface->is_default = 1;

    csp_conn_print_table(); //* Print connection table
    csp_iflist_print();     //* Print my KISS address

    ret = pthread_create(&router_thread, NULL, do_route, NULL); //* Start route thread
    if (ret != 0)
    {
        printf("pthread failed\n");
        exit(1);
    }

    csp_bind(&sock, CSP_ANY);       //* Bind socket to all ports, e.g. all incoming connections will be handled here
    csp_listen(&sock, SERVER_PORT); //* Create a backlog of 10 connections, i.e. up to 10 new connections can be queued

    while (!done) //* While loop for waiting client to connect
    {
        csp_packet_t *req; //* Create packet for store data to send from client
        csp_conn_t *conn;  //* For store connection structure

        conn = csp_accept(&sock, CSP_MAX_TIMEOUT); //* Accept/Waiting new connection
        req = csp_read(conn, CSP_MAX_TIMEOUT);     //* Read packet from connection

        switch (csp_conn_dport(conn)) //* Check destination port of connection
        {
        case 10: //* Destination match with server port
            printf("Size  of : %d\n", sizeof(req));
            printf("Packet received on SERVER_PORT: %d\n", *(int *)req->data); //* Show received data from client

            break;

        default: //* Destination doesn't match with server port
            printf("wrong port\n");
            exit(1);
            break;
        }

        csp_buffer_free(req); //* Clear data in req
        csp_close(conn);      //* Close an open connection
    }

    return 0;
}