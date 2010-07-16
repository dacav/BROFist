#ifndef __bro_headers_bt_h
#define __bro_headers_bt_h

#include <stdint.h>
#include <sys/socket.h>


#define MAX_BT_DEVICES 255
#define BT_INQUIRY_LEN 8

typedef struct bro_bt_device bro_bt_device_t;

size_t bro_bt_scan_devices (bro_bt_device_t *devices[MAX_BT_DEVICES]);

int bro_bt_connect_device (int * spam_sock, sockaddr_rc * addr_data,
                        bdaddr_t mac_addr);
                        
int bro_bt_close_connection (int spam_sock);
