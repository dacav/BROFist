#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <assert.h>
#include <signal.h>

#include "headers/bro_comm.h"

#define BRO_MAX_CONNECTIONS     1

static
void remove_socket (int sig)
{
   unlink(SERVER_PATH);
   exit(EXIT_FAILURE);
}

static
void enable_cleanup ()
{
   signal(SIGINT, remove_socket);
}

int bro_start_server (int * server_sock, int * client_sock)
{
    // Variable definitions
    int    rc, length;
    struct sockaddr_un bro_server;

    // Initializing socket for server
    *server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (*server_sock < 0)
    {
     perror("socket() failed");
     return -1;
    }

    // Now we'll get a unique name for our server binding the socket
    memset(&bro_server, 0, sizeof(bro_server));
    bro_server.sun_family = AF_UNIX;
    strcpy(bro_server.sun_path, SERVER_PATH);

    rc = bind(*server_sock, (struct sockaddr *)&bro_server, SUN_LEN(&bro_server));
    if (rc < 0)
    {
     perror("bind() failed");
     return -1;
    }

    enable_cleanup();

    // We'll listen to connections up to a max of BRO_MAX_CONNECTIONS
    rc = listen(*server_sock, BRO_MAX_CONNECTIONS);
    if (rc< 0)
    {
     perror("listen() failed");
     return -1;
    }

    printf("Ready for client connect().\n");

    /*
     * The server uses the accept() function to accept an incoming
     * connection request.  The accept() call will block indefinitely
     * waiting for the incoming connection to arrive.
     */
    *client_sock = accept(*server_sock, NULL, NULL);
    if (*client_sock < 0)
    {
     perror("accept() failed");
     return -1;
    }

    /* We will use the SO_RCVLOWAT socket option to specify that we don't want 
     * recv() to wake up until all sizeof(bro_fist_t) of data have arrived.
     */
    length = sizeof(bro_fist_t) * BUFFER_SIZE;
    rc = setsockopt(*client_sock, SOL_SOCKET, SO_RCVLOWAT,
                                      (char *)&length, sizeof(length));
    if (rc < 0)
    {
     perror("setsockopt(SO_RCVLOWAT) failed");
     return -1;
    }
    
    return 0;
}

int bro_server_fist (bro_fist_t * input_fist, bro_fist_t * out_fist,
                     int scicos_sock, int spam_sock){

    int rc, i;
    
    assert(scicos_sock >= 0/* && spam_sock >= 0*/);
    
    /*
     * TODO: Prendere in input socket di SciCos e dell'NXT ed implementare la    
     * comunicazione tra i due QUI dentro
     */
     
    rc = recv(scicos_sock, input_fist, sizeof(bro_fist_t) * BUFFER_SIZE, 0);
    if (rc < 0)
    {
     perror("recv() failed");
     return -1;
    } 
    
    if (rc == 0 || rc < sizeof(bro_fist_t))
    {
     printf("The client closed the connection before all of the\n");
     printf("data was sent\n");
     return -1;
    }
    
    bro_bt_client_fist (input_fist, out_fist, spam_sock);
    
    memcpy(out_fist, input_fist, sizeof(bro_fist_t) * BUFFER_SIZE);
    
    for (i = 0; i < BUFFER_SIZE; i++) {
        out_fist[i].data = (float)((rand() % 10) + 1.0);
    }
    
    /********************************************************************/
    /* Echo the data back to the client                                 */
    /********************************************************************/
    rc = send(scicos_sock, out_fist, sizeof(bro_fist_t) * BUFFER_SIZE, 0);
    if (rc < 0)
    {
     perror("send() failed");
     return -1;
    }
    
    return 0;

}

int bro_stop_server (int server_sock, int client_sock){
    /***********************************************************************/
   /* Close down any open socket descriptors                              */
   /***********************************************************************/
   if (server_sock != -1)
      close(server_sock);

   if (client_sock != -1)
      close(client_sock);

    /***********************************************************************/
   /* Remove the UNIX path name from the file system                      */
   /***********************************************************************/
   unlink(SERVER_PATH);
   
   return 0;
}
