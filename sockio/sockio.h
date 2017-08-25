#ifndef SOCKIO_H
#define SOCKIO_H

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

#include <net/route.h>

#include "waitio.h"

extern struct timeval sys_time(Timeout timeout);

enum ServerState { SRV_CREATED, SRV_BOUND, SRV_LISTENING, SRV_CLOSED, SRV_ERROR };
enum ClientState { CLN_CREATED, CLN_CONNECTED, CLN_CLOSED, CLN_ERROR };
enum ConnectionState { CONN_ACCEPTED, CONN_CLOSED, CONN_ERROR};
enum ChunkState { CHK_EMPTY, CHK_FULL, CHK_PARTIAL, CHK_NOMORE, CHK_ERROR };
enum TransportProtocol { UDP, TCP };

typedef struct
{
    enum ServerState state;
    enum TransportProtocol protocol;
    unsigned port;
    int sock_fd;
    int err;
    int backlog;
    bool blocking;
} Server;

typedef struct
{
    enum ClientState state;
    enum TransportProtocol protocol;
    char *server_ip;
    unsigned server_port;
    int sock_fd;
    int err;
    bool blocking;
} Client;

typedef struct
{
    enum ConnectionState state;
    struct sockaddr_in address;
    int sock_fd;
    int err;
} ClientConnection;

typedef struct
{
    ClientConnection connection;
    bool no_cleanup;
    bool process_next;
} ClientCallbackResult;

typedef struct
{
    enum ChunkState state;
    char *data;
    unsigned sequence;
    unsigned len;
    unsigned capacity;
    int err;
} DataChunk;

typedef ClientCallbackResult (*ClientHandlerFunc) (ClientConnection);
typedef bool (*ServerTimeoutFunc) (Server);
typedef Client (*ClientTimeoutFunc) (Client);
typedef Client (*ServerConnectionFunc) (Client);
typedef void (*ChunkTimeoutFunc) (DataChunk);

struct sockaddr_in create_address (unsigned port);
struct sockaddr_in create_remote_address (char *ip, unsigned port);
int protocol_flag (enum TransportProtocol protocol);
int create_inet_socket (enum TransportProtocol protocol, bool blocking);
int make_non_blocking (int sock_fd);

/* Server control functions */
Server set_server_error (Server server, int err);
Server set_server_state (Server server, enum ServerState state);

Server create_server (enum TransportProtocol protocol, unsigned port, bool blocking);
Server bind_server (Server server);
Server listen_server (Server server);
Server prepare_tcp_server (unsigned port);
Server prepare_async_tcp_server (unsigned port);
Server close_server (Server server);
int interrupt_server (Server server);

/* Server operation functions */
ClientConnection accept_connection (Server server);
ClientConnection close_connection (ClientConnection connection);
ClientConnection set_client_connection_error (ClientConnection connection, int err);
ClientConnection set_client_connection_state (ClientConnection connection, enum ConnectionState state);

ClientCallbackResult conn_cb_cleanup (ClientCallbackResult cb_res);
ClientCallbackResult async_connection (ClientConnection connection);
ClientCallbackResult async_no_next_connection (ClientConnection connection);

bool read_client_connection (Server server, ClientHandlerFunc conn_success_cb,
                             ClientHandlerFunc conn_err_cb);

Server start_tcp_server (Server server, ClientHandlerFunc conn_success_cb,
                         ClientHandlerFunc conn_err_cb);
Server start_async_tcp_server (Server server, Timeout timeout,
                                ClientHandlerFunc conn_success_cb,
                                ClientHandlerFunc conn_err_cb,
                                ServerTimeoutFunc timeout_cb);

/* Client control functions */
Client set_client_error (Client client, int err);
Client set_client_state (Client client, enum ClientState state);

Client create_client (enum TransportProtocol protocol, char *ip, unsigned port, bool blocking);
Client client_cb_cleanup (Client client);
Client connect_client (Client client, ServerConnectionFunc connect_cb);
Client connect_client_async (Client client, Timeout timeout,
                             ServerConnectionFunc connect_cb,
                             ClientTimeoutFunc timeout_cb);
Client close_client (Client client);

/* Data transmission functions */
DataChunk set_chunk_error (DataChunk chunk, int err);
DataChunk set_chunk_len (DataChunk chunk, int len);

DataChunk receive_chunk (int sock_fd, DataChunk chunk);
DataChunk receive_chunk_no_block (int sock_fd, DataChunk chunk, Timeout timeout,
                                  ChunkTimeoutFunc timeout_cb);

DataChunk send_chunk (int sock_fd, DataChunk chunk);
DataChunk send_chunk_no_block (int sock_fd, DataChunk chunk, Timeout timeout,
                               ChunkTimeoutFunc timeout_cb);

#endif
