#include "sockio.h"
#include <fcntl.h>
struct sockaddr_in create_address (unsigned port)
{
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    return address;
}

struct sockaddr_in create_remote_address (char *ip, unsigned port)
{
    struct hostent *remote = gethostbyname(ip);
    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    bcopy((char *)remote->h_addr, (char *)&address.sin_addr.s_addr, remote->h_length);

    return address;
}

int protocol_flag (enum TransportProtocol protocol)
{
    switch (protocol) {
        case UDP:
            return SOCK_DGRAM;
        case TCP:
            return SOCK_STREAM;
        default:
            return -1;
    }
}

int create_inet_socket (enum TransportProtocol protocol, bool blocking)
{
    return socket(AF_INET, protocol_flag(protocol) | (blocking? 0x0 : O_NONBLOCK), 0);
}

int make_non_blocking (int sock_fd)
{
    fcntl(sock_fd, F_SETFL, O_NONBLOCK);
}

/* Server control functions */
Server set_server_error (Server server, int error)
{
    return (Server) {
        SRV_ERROR,
        server.protocol,
        server.port,
        server.sock_fd,
        error,
        server.backlog,
        server.blocking
    };
}

Server set_server_state (Server server, enum ServerState state)
{
    return (Server) {
        state,
        server.protocol,
        server.port,
        server.sock_fd,
        server.err,
        server.backlog,
        server.blocking
    };
}

Server create_server (enum TransportProtocol protocol, unsigned port, bool blocking)
{
    int sock_fd = create_inet_socket(protocol, true); // the server doesn't need O_NONBLOCK

    if (sock_fd < 0)
    {
        return (Server) {
            SRV_ERROR,
            protocol,
            port,
            -1,
            errno,
            0,
            blocking
        };
    }

    return (Server) {
        SRV_CREATED,
        protocol,
        port,
        sock_fd,
        0,
        0,
        blocking
    };
}

Server bind_server (Server server)
{
    if (server.state == SRV_ERROR)
        return server;

    struct sockaddr_in address = create_address(server.port);
    socklen_t len = sizeof(struct sockaddr_in);

    if (bind(server.sock_fd, (struct sockaddr *)&address, len) < 0)
        return set_server_error(server, errno);

    return set_server_state(server, SRV_BOUND);
}

Server listen_server (Server server)
{
    if (server.state == SRV_ERROR)
        return server;

    if (listen(server.sock_fd, server.backlog) < 0)
        return set_server_error(server, errno);

    return set_server_state(server, SRV_LISTENING);
}

Server prepare_tcp_server (unsigned port)
{
    return listen_server(bind_server(create_server(TCP, port, true)));
}

Server prepare_async_tcp_server (unsigned port)
{
    return listen_server(bind_server(create_server(TCP, port, false)));
}

Server close_server (Server server)
{
    if (server.state == SRV_ERROR)
        return server;

    if (close(server.sock_fd) < 0)
        return set_server_error(server, errno);

    return set_server_state(server, SRV_CLOSED);
}

int interrupt_server (Server server)
{
    if (server.state == SRV_ERROR)
        return 0;

    return shutdown(server.sock_fd, SHUT_RDWR) == 0;
}

/* Server operations functions */
ClientConnection set_client_connection_error (ClientConnection connection, int error)
{
    return (ClientConnection) {
        CONN_ERROR,
        connection.address,
        connection.sock_fd,
        error
    };
}

ClientConnection set_client_connection_state (ClientConnection connection, enum ConnectionState state)
{
    return (ClientConnection) {
        state,
        connection.address,
        connection.sock_fd,
        connection.err
    };
}

ClientConnection accept_connection (Server server)
{
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(struct sockaddr_in);

    int client_fd = accept(server.sock_fd, (struct sockaddr *)&client_address, &client_len);

    if (client_fd < 0)
    {
        return (ClientConnection) {
            CONN_ERROR,
            client_address,
            client_fd,
            errno
        };
    }

    return (ClientConnection) {
        CONN_ACCEPTED,
        client_address,
        client_fd,
        0
    };
}

ClientConnection close_connection (ClientConnection connection)
{
    if (connection.state == CONN_ERROR)
        return connection;

    if (close(connection.sock_fd) < 0)
        return set_client_connection_error(connection, errno);

    return set_client_connection_state(connection, CONN_CLOSED);
}

ClientCallbackResult conn_cb_cleanup (ClientCallbackResult cb_res)
{
    if (cb_res.connection.state == CONN_CLOSED || cb_res.no_cleanup)
        return cb_res;

    return (ClientCallbackResult) {
        close_connection(cb_res.connection),
        cb_res.process_next
    };
}

ClientCallbackResult async_connection (ClientConnection connection)
{
    return (ClientCallbackResult){connection, false, true};
}

ClientCallbackResult async_no_next_connection (ClientConnection connection)
{
    return (ClientCallbackResult){connection, false, false};
}

bool read_client_connection (Server server, ClientHandlerFunc conn_success_cb,
                             ClientHandlerFunc conn_err_cb)
{
    ClientConnection client = accept_connection(server);
    if (client.state == CONN_ACCEPTED)
        return conn_cb_cleanup(conn_success_cb(client)).process_next;
    else if (client.state == CONN_ERROR)
        return conn_err_cb(client).process_next;
}

Server start_tcp_server (Server server, ClientHandlerFunc conn_success_cb,
                            ClientHandlerFunc conn_err_cb)
{
    if (server.state != SRV_LISTENING || !server.blocking)
        return server;

    while (read_client_connection(server, conn_success_cb, conn_err_cb));

    return close_server(server);
}

Server start_async_tcp_server (Server server, Timeout timeout,
                                ClientHandlerFunc conn_success_cb,
                                ClientHandlerFunc conn_err_cb,
                                ServerTimeoutFunc timeout_cb)
{
    if (server.state != SRV_LISTENING || server.blocking)
        return server;
    bool loop = true;
    WaitResult conn_wait;

    while (loop)
    {
        conn_wait = wait_for_read(server.sock_fd, timeout);

        if (conn_wait == WAIT_TIMEOUT)
            loop = timeout_cb(server);
        else if (conn_wait == WAIT_ON_TIME)
            loop = read_client_connection(server, conn_success_cb, conn_err_cb);
    }

    return close_server(server);
}

/* Client control functions */
Client set_client_error (Client client, int err)
{
    return (Client) {
        CLN_ERROR,
        client.protocol,
        client.server_ip,
        client.server_port,
        client.sock_fd,
        err,
        client.blocking
    };
}

Client set_client_state (Client client, enum ClientState state)
{
    return (Client) {
        state,
        client.protocol,
        client.server_ip,
        client.server_port,
        client.sock_fd,
        client.err,
        client.blocking
    };
}

Client create_client (enum TransportProtocol protocol, char *ip, unsigned port, bool blocking)
{
    int sock_fd = create_inet_socket(protocol, blocking);
    if (sock_fd < 0)
    {
        return (Client) {
            CLN_ERROR,
            protocol,
            ip,
            port,
            sock_fd,
            errno,
            blocking
        };
    }

    return (Client) {
        CLN_CREATED,
        protocol,
        ip,
        port,
        sock_fd,
        0,
        blocking
    };
}

Client client_cb_cleanup (Client client)
{
    if (client.state == CLN_CLOSED)
        return client;

    return close_client(client);
}

Client connect_client (Client client, ServerConnectionFunc connect_cb)
{
    struct sockaddr_in server_addr = create_remote_address(client.server_ip, client.server_port);
    socklen_t len = sizeof(server_addr);

    if (connect(client.sock_fd, (struct sockaddr *)&server_addr, len) == -1)
        return set_client_error(client, errno);

    return client_cb_cleanup(connect_cb(set_client_state(client, CLN_CONNECTED)));
}

Client connect_client_async (Client client, Timeout timeout,
                             ServerConnectionFunc connect_cb,
                             ClientTimeoutFunc timeout_cb)
{
    struct sockaddr_in server_addr = create_remote_address(client.server_ip, client.server_port);
    socklen_t len = sizeof(server_addr);

    if (connect(client.sock_fd, (struct sockaddr *)&server_addr, len) == -1
        && errno != EINPROGRESS)
    {
        return set_client_error(client, errno);
    }

    switch (wait_for_write(client.sock_fd, timeout)) {
        case WAIT_ON_TIME:
            return client_cb_cleanup(connect_cb(set_client_state(client, CLN_CONNECTED)));
        case WAIT_TIMEOUT:
            return timeout_cb(client);
        default:
            return set_client_error(client, -1);
    }
}

Client close_client (Client client)
{
    if (client.state != CLN_CONNECTED)
        return client;

    if (close(client.sock_fd) < 0)
        return set_client_error(client, errno);

    return set_client_state(client, CLN_CLOSED);
}

/* Data transmission functions */
DataChunk set_chunk_error (DataChunk chunk, int error)
{
    return (DataChunk) {
        CHK_ERROR,
        chunk.data,
        chunk.sequence,
        chunk.len,
        chunk.capacity,
        error
    };
}

DataChunk set_chunk_len (DataChunk chunk, int len)
{
    enum ChunkState state;
    if (len < 0)
        state = CHK_NOMORE;
    else if (len < chunk.capacity)
        state = CHK_PARTIAL;
    else
        state = CHK_FULL;

    return (DataChunk) {
        state,
        chunk.data,
        chunk.sequence,
        (len < 0) ? 0 : len,
        chunk.capacity,
        chunk.err
    };
}

DataChunk receive_chunk (int sock_fd, DataChunk chunk)
{
    chunk.sequence++;
    bzero(chunk.data, chunk.capacity);
    return set_chunk_len(chunk, recv(sock_fd, chunk.data, chunk.capacity, 0));
}

DataChunk receive_chunk_no_block (int sock_fd, DataChunk chunk, Timeout timeout,
                                  ChunkTimeoutFunc timeout_cb)
{
    switch (wait_for_read(sock_fd, timeout)) {
        case WAIT_ON_TIME:
            return receive_chunk(sock_fd, chunk);
        case WAIT_TIMEOUT:
            timeout_cb(chunk); // the timeout callback gets the last received chunk, not a new one
            return chunk; // return the last received chunk
        default:
            return chunk; // return the last received chunk
    }
}

DataChunk send_chunk (int sock_fd, DataChunk chunk)
{
    int sent = send(sock_fd, chunk.data, chunk.capacity, 0);
    if (sent == -1)
        return set_chunk_error(chunk, errno);

    return set_chunk_len(chunk, sent);
}

DataChunk send_chunk_no_block (int sock_fd, DataChunk chunk, Timeout timeout,
                               ChunkTimeoutFunc timeout_cb)
{
    switch (wait_for_write(sock_fd, timeout)) {
        case WAIT_ON_TIME:
            return send_chunk(sock_fd, chunk);
        case WAIT_TIMEOUT:
            timeout_cb(chunk);
            return chunk;
        default:
            return chunk;
    }
}
