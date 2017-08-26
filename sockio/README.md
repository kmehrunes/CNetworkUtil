
# SOCKIO

This is a library I wrote to make networking in C more tolerable. It provides more programmer-friendly interfaces for creating servers, clients, and exchanging data between them. It also makes it easier to deal with timeouts.

The library relies on callbacks to de-couple the logic from the basic operations. All its functions also return a new state, and never attempt to modify the state of what is given to them.

### TCP server with no timeouts
```C
void blocking_tcp ()
{
    Server server = prepare_tcp_server(port);
    server = start_tcp_server(server, client_handler, client_err);

    if (srv.state == SRV_ERROR) // check if it exited with an error
        printf("An error occurred: %d\n", server.err);
    printf("Done\n");
}
```

```C
ClientCallbackResult client_handler (ClientConnection client)
{
    return (ClientCallbackResult){
        client, /* the client connection this callback processed */
        false, /* don't disable clean up operation */
        false /* don't process next, exit now */
    };
}
```

```C
ClientCallbackResult client_err (ClientConnection client)
{
    return (ClientCallbackResult){
        client, /* the client connection this callback processed */
        false, /* don't disable clean up operation */
        true /* process the next connection */
    };
}
```
*_*
### TCP server with timeouts
The timeout callback is called if there was no new connection for a certain amount of time. Helpful in some situations especially if you want to terminate the server gracefully, in which case just make the callback return *false*

```C
void nonblocking_tcp ()
{
    Server server = prepare_async_tcp_server(7895);
    server = start_async_tcp_server(server, seconds(5), client_handler, client_err, timeout_handler);
    if (server.state == SRV_ERROR)
        printf("An error occurred: %d\n", server.err);
    printf("Done\n");
}
```

```C
bool timeout_handler (Server server)
{
    printf("No conections so far!\n");
    return true; // continue processing
}
```
*_*
### TCP client with no timeouts
```C
void client_connect ()
{
    Client client = connect_client(create_client(TCP, "127.0.0.1", port, true /* blocking */), connect_cb);
    if (client.state == CLN_ERROR)
        printf("Client error %d\n", client.err);
}
```
```C
Client connect_cb (Client client)
{
    printf("Successfully connected\n");
    return close_client(client); // not needed, connect_client() will close it if it wasn't closed
}
```
*_*
### TCP client with timeouts
```C
void client_connect ()
{
    Client client = connect_client_async(create_client(TCP, "127.0.0.1", port, false /* non-blocking*/), seconds(3), connect_cb, connect_timeout);
    if (client.state == CLN_ERROR)
        printf("Client error %d\n", client.err);
}
```

```C
Client connect_timeout (Client client)
{
    printf("Failed to connect to %s\n", client.server_ip);
    return client;
}
```
