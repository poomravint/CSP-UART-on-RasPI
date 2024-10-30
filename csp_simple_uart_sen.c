#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <csp/csp.h>
#include <csp/drivers/usart.h>

#define SERVER_ADDR 1  //*Server Address
#define SERVER_PORT 10 //*Server Port
#define CLIENT_ADDR 2  //*Address address

bool done = false;

// TODO : gcc csp_simple_uart_sen.c -lcsp -lpthread -o cspsimpleuartsen

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
    csp_conn_t *conn;        //* For store connection structure
    csp_packet_t *packet;    //* Packet for store data to send to server
    csp_packet_t *reply;     //* Packet for store reply from server
    pthread_t router_thread; //* For use Pthread

    int ret;          //* For check error
    int data_to_send; //* For store data to send

    csp_init(); //* Start CSP

    ret = csp_usart_open_and_add_kiss_interface(&conf, CSP_IF_KISS_DEFAULT_NAME, CLIENT_ADDR, &iface); //* Start Uart
    //* If error to start UART will exit this code *//
    if (ret != CSP_ERR_NONE)
    {
        csp_print("open failed\n");
        exit(1);
    }

    iface->addr = CLIENT_ADDR; //* Set client's address
    iface->is_default = 1;

    csp_conn_print_table(); //* Print connection table
    csp_iflist_print();     //* Print my KISS address

    ret = pthread_create(&router_thread, NULL, do_route, NULL); //* Start route thread
    if (ret != 0)
    {
        printf("pthread failed\n");
        exit(1);
    }

    data_to_send = -100;        //! Set data to send !//
    packet = csp_buffer_get(0); //* Get free buffer from task context

    *(int *)(packet->data) = data_to_send; //* Store value of data_to_send in packet to send to Server

    packet->length = 4; //* Set data length (Bytes) MAX : 251

    conn = csp_connect(CSP_PRIO_NORM, SERVER_ADDR, SERVER_PORT, 0, CSP_O_NONE); //* Connect to server

    csp_send(conn, packet); //* Send packet to server

    csp_close(conn); //* Close an open connection

    return 0;
}