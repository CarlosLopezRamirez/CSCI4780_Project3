// File: internet_socket.cpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#include "include/inet/internet_socket.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/poll.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

// Utility Functions -------------------------------------------------------------------------------

void perror_and_exit(const char *header) {
    std::perror(header);
    std::exit(EXIT_FAILURE);
}

// InternetAddress Public API Functions ------------------------------------------------------------

InternetAddress::InternetAddress() : size_(0) { std::memset(&addr_, 0, sizeof(addr_)); }

InternetAddress::InternetAddress(const sockaddr_in &ipv4_addr, socklen_t ipv4_addr_size) :
    addr_(ipv4_addr),
    size_(ipv4_addr_size) {}

InternetAddress InternetAddress::from_ip_address(const char *name, const char *port) {
    // Use getaddrinfo() to get information using the inputted arguments
    addrinfo hints, *socket_info;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(name, port, &hints, &socket_info);
    if (status != 0) {
        std::cout << "getaddrinfo() failed: " << gai_strerror(status);
        std::exit(EXIT_FAILURE);
    }
    sockaddr_in sa_in = *(sockaddr_in *)socket_info->ai_addr;

    InternetAddress result(sa_in, socket_info->ai_addrlen);

    freeaddrinfo(socket_info);

    return result;
}

sockaddr_in *InternetAddress::ptr() { return (size_ > 0) ? &addr_ : nullptr; }

socklen_t InternetAddress::size() { return size_; }

std::string InternetAddress::addr_str() {
    return (size_ > 0) ? inet_ntoa(ptr()->sin_addr) : "INVALID";
}

std::string InternetAddress::port_str() {
    return (size_ > 0) ? std::to_string((int)ntohs(addr_.sin_port)) : "INVALID";
}

// InternetSocket Public API Functions -------------------------------------------------------------

InternetSocket::InternetSocket() { do_open_(); }

InternetSocket::InternetSocket(InternetSocket &other) { *this = other; }

InternetSocket &InternetSocket::operator=(InternetSocket &other) {
    if (this == &other) return *this;

    // Move values from the other socket to this one
    this->file_desc_   = other.file_desc_;
    this->host_addr_   = other.host_addr_;
    this->remote_addr_ = other.remote_addr_;

    return *this;
}

InternetSocket::InternetSocket(InternetSocket &&other) { *this = std::move(other); }

InternetSocket &InternetSocket::operator=(InternetSocket &&other) {
    if (this == &other) return *this;

    // Move values from the other socket to this one
    this->file_desc_   = other.file_desc_;
    this->host_addr_   = other.host_addr_;
    this->remote_addr_ = other.remote_addr_;

    // Invalidate the other socket
    other.file_desc_   = -1;
    other.host_addr_   = InternetAddress();
    other.remote_addr_ = InternetAddress();

    return *this;
}

InternetSocket::~InternetSocket() { do_close_(); }

void InternetSocket::do_bind(uint16_t host_port) {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(host_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    int result = bind(file_desc_, (sockaddr *)&addr, sizeof(addr));
    if (result < 0) perror_and_exit("bind() failed");
}

void InternetSocket::do_connect(std::string remote_addr, uint16_t remote_port) {
    std::string port = std::to_string((int)remote_port);

    remote_addr_ = InternetAddress::from_ip_address(remote_addr.c_str(), port.c_str());
    int result   = connect(file_desc_, (sockaddr *)remote_addr_.ptr(), remote_addr_.size());
    if (result < 0) perror_and_exit("connect() failed");
}

void InternetSocket::do_listen(size_t backlog_size) {
    int result = listen(file_desc_, (int)backlog_size);
    if (result < 0) perror_and_exit("listen() failed");
}

InternetSocket InternetSocket::do_accept() {
    sockaddr_in sa_in;
    socklen_t sa_in_size = sizeof(sa_in);

    int remote_fd = accept(file_desc_, (sockaddr *)&sa_in, &sa_in_size);
    if (remote_fd < 0) perror_and_exit("accept() failed");

    return InternetSocket(remote_fd, host_addr_, InternetAddress(sa_in, sa_in_size));
}

size_t InternetSocket::do_send(const Buffer &buffer, int flags) {

    int bytes_sent = send(file_desc_, buffer.data(), buffer.size(), flags);
    if (bytes_sent < 0) perror_and_exit("send() failed");

    return bytes_sent;
}

void InternetSocket::do_sendall(const Buffer &buffer, int flags) {

    size_t total_sent    = 0;
    size_t bytes_to_send = buffer.size();
    int bytes_sent       = 0;

    while (total_sent < buffer.size()) {
        bytes_sent = do_send(buffer + total_sent, flags);
        if (bytes_sent < 0) perror_and_exit("send() failed");
        total_sent += bytes_sent;
        bytes_to_send -= bytes_sent;
    }
}

size_t InternetSocket::do_recv(Buffer &buffer, int flags) {

    int bytes_recvd = recv(file_desc_, buffer.data(), buffer.size(), flags);
    if (bytes_recvd < 0) perror_and_exit("recv() failed");

    return bytes_recvd;
}

void InternetSocket::do_recvall(Buffer &buffer, int flags) {

    size_t total_recvd   = 0;
    size_t bytes_to_recv = buffer.size();
    int bytes_recvd      = 0;

    while (total_recvd < buffer.size()) {
        Buffer moved = (buffer + total_recvd);
        bytes_recvd  = do_recv(moved, flags);
        if (bytes_recvd < 0) perror_and_exit("recv() failed");

        total_recvd += bytes_recvd;
        bytes_to_recv -= bytes_recvd;
    }
}

void InternetSocket::do_shutdown() {
    if (file_desc_ <= 0) return;

    int result = shutdown(file_desc_, SHUT_WR);
    if (result < 0) perror_and_exit("shutdown() failed");
}

std::string InternetSocket::host_ip() { return host_addr_.addr_str(); }

std::string InternetSocket::host_port() { return host_addr_.port_str(); }

std::string InternetSocket::host_addr() { return (host_ip() + ":" + host_port()); }

std::string InternetSocket::remote_ip() { return remote_addr_.addr_str(); }

std::string InternetSocket::remote_port() { return remote_addr_.port_str(); }

std::string InternetSocket::remote_addr() { return (remote_ip() + ":" + remote_port()); }

PollInfo InternetSocket::do_poll(PollInfo request, size_t timeout) {
    pollfd this_socket[1];

    // Set the file descriptor
    this_socket[0].fd = file_desc_;

    // Set the events to be polled for
    short events = 0;
    if (request.readable) events |= POLLIN;
    if (request.writeable) events |= POLLOUT;
    this_socket[0].events = events;

    // Poll the socket
    int ready_fds = poll(this_socket, 1, timeout);
    if (ready_fds == -1) perror_and_exit("poll() failed");

    PollInfo result;
    result.valid     = false;
    result.timedout  = false;
    result.readable  = false;
    result.writeable = false;

    if (ready_fds != 0) /* A state has changed */ {
        // Fill return struct with poll data
        result.valid = !((this_socket[0].revents & POLLHUP) | (this_socket[0].revents & POLLNVAL));
        result.readable  = (this_socket[0].revents & POLLIN);
        result.writeable = (this_socket[0].revents & POLLOUT);
    } else /* No state has changed */ {
        // Indicate that a timeout occured
        result.valid    = true;
        result.timedout = true;
    }

    return result;
}

// InternetSocket Private API Functions ------------------------------------------------------------

InternetSocket::InternetSocket(int file_desc, InternetAddress host_addr,
                               InternetAddress remote_addr) :
    file_desc_(dup(file_desc)),
    host_addr_(host_addr),
    remote_addr_(remote_addr) {}

void InternetSocket::do_open_() {
    // Open a new socket
    file_desc_ = socket(AF_INET, SOCK_STREAM, 0);
    if (file_desc_ < 0) perror_and_exit("socket() failed");

    // Use `setsockopt()` to allow port reuse and avoid "address already in use" errors
    int optval = 1;
    setsockopt(file_desc_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // Initialize this socket's address
    host_addr_ = InternetAddress();
}

void InternetSocket::do_close_() {
    // Close the file descriptor
    if (file_desc_ > 0) close(file_desc_);
}