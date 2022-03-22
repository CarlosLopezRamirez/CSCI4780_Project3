// File: include/inet/internet_socket.hpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#pragma once

#include <netinet/in.h>

#include <limits>
#include <string>
#include <vector>

#include "buffer.hpp"

// Represents the address associated with an internet socket
class InternetAddress {
  public:
    InternetAddress();

    // Constructs an internet address
    InternetAddress(const sockaddr_in &ipv4_addr, socklen_t ipv4_addr_size);

    // Constructs an InternetAddress with information from `getaddrinfo()`, which the specified
    // `address` and `port` are passed to
    static InternetAddress from_ip_address(const char *name, const char *port);

    // Returns a pointer to the underlying address structure of this internet address
    sockaddr_in *ptr();

    // Returns the size of the underlying address structure of this internet address
    socklen_t size();

    // Returns the string representation of IP address associated with this address
    std::string addr_str();

    // Returns the string representation of the port associated with this address
    std::string port_str();

  private:
    // The underlying address structure of this internet address
    sockaddr_in addr_;

    // The size of the underlying address structure of this internet address
    socklen_t size_;
};

// Represents information about a socket that is being polled for its state
struct PollInfo;

// Represents an IPv4 TCP socket
class InternetSocket {
  public:
    // Creates a socket
    InternetSocket();

    // Deleting these makes this socket non-copyable
    InternetSocket(InternetSocket &other);
    InternetSocket &operator=(InternetSocket &other);

    // Defining these makes this socket move constructable and move assignable
    InternetSocket(InternetSocket &&other);
    InternetSocket &operator=(InternetSocket &&other);

    // Cleans up the resources associated with this socket
    ~InternetSocket();

    // Binds the socket to port `host_port` on the host machine
    void do_bind(uint16_t host_port);

    // Connects to the machine located at `remote_addr` (which can be in common or dotted form) on
    // port `remote_port`
    void do_connect(std::string remote_addr, uint16_t remote_port);

    // Prompts this socket to listen for incoming connections with a backlog of size `backlog_size`
    void do_listen(size_t backlog_size);

    // Accepts the next waiting connection and returns its associated socket
    InternetSocket do_accept();

    // Returns the number of bytes of `buffer` that were successfully sent to the remote end of the
    // connection
    size_t do_send(const Buffer &data, int flags = 0);

    // Sends all of the bytes of `buffer` to the remote end of the connection, blocking until all
    // bytes are sent
    void do_sendall(const Buffer &data, int flags = 0);

    // Possibly returns a `Buffer` containing up to `max_bytes_expected` bytes of data received from
    // the remote end of the connection
    size_t do_recv(Buffer &data, int flags = 0);

    // Possibly returns a `Buffer` containing `bytes_expected` bytes of data received from the
    // remote end of the connection, blocking until `bytes_expected` bytes are received.
    void do_recvall(Buffer &data, int flags = 0);

    // Shuts down the write end of this socket, indicating an attempt to gracefully close the
    // connection that this socket is bound to
    void do_shutdown();

    // Returns the string representation of the IP address of the host end of the connection
    std::string host_ip();

    // Returns the string representation of the port of the host end of the connection, or an empty
    // string if this socket is not bound
    std::string host_port();

    // Returns the string representation of the full IP address of the host, or "INVALID" if this
    // socket is not bound
    std::string host_addr();

    // Returns the string representation of the IP address of the remote end of the connection, or
    // an empty string if this socket has not been connected
    std::string remote_ip();

    // Returns the string representation of the port of the remote end of the connection, or an
    // empty string if this socket has not been connected
    std::string remote_port();

    // Returns the string representation of the full IP address of the remote, or "INVALID" if this
    // socket is not bound
    std::string remote_addr();

    // Returns the result of polling this socket for a change in status for `timeout` milliseconds,
    // rounded up to the system clock's granularity
    //
    // For `timeout` > 0, this call will timeout for `timeout` seconds unless a change in status
    //                    occurs during the interval.
    // For `timeout` = 0, this call will return immediately, reporting the status of this socket.
    // For `timeout` < 0, this call will block indefinitely until a change in status occurs.
    PollInfo do_poll(PollInfo request, size_t timeout);

  private:
    // Used to construct remote sockets
    InternetSocket(int file_desc, InternetAddress host_addr, InternetAddress remote_addr);

    // Opens the underlying OS socket
    void do_open_();

    // Closes the underlying OS socket
    void do_close_();

    // The file descriptor that identifies this socket
    int file_desc_;

    // The internet address that describes this socket
    InternetAddress host_addr_;

    // The internet address that describes the remote socket
    InternetAddress remote_addr_;
};

struct PollInfo {
    // On input:  Ignored
    // On output: True if the socket is in a valid state (is open)
    bool valid;

    // On input:  Ignored
    // On output: True if the poll request timed out before a state change
    bool timedout;

    // On input:  True when the caller wants to know if the socket is readable
    // On output: True if there was data available to read from the socket
    bool readable;

    // On input:  True when the caller wants to know if the socket is readable
    // On output: True if data may be written to the socket
    bool writeable;
};