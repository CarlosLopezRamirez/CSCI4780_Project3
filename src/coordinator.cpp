// File: coordinator.cpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#include "include/coordinator.hpp"
#include "include/multicast_message.hpp"

#include <filesystem>
#include <sstream>
#include <fstream>

Coordinator::Coordinator(uint16_t localport, int persistence_time) :
    localport_(localport), persistence_time_(persistence_time)
{ }

void Coordinator::start() {
    this->is_running_ = true;
    this->coordinator_socket_.do_bind(this->localport_);
    this->coordinator_socket_.do_listen(10);
    incoming_messages_thread_ = std::thread(&Coordinator::handleIncomingMessages, this);
    return;
}

void Coordinator::stop() { this->is_running_ = false; }

void Coordinator::handleIncomingMessages() {
    PollInfo connection_request;
    connection_request.readable  = true;
    connection_request.writeable = false;

    // TODO: Implement
    while (this->is_running_) {
        // poll to see if there are any connection requests
        PollInfo result = this->coordinator_socket_.do_poll(connection_request, 1 * 1000 /* timeout after 1 second */);
        if (!result.valid || (result.valid && !result.readable)) continue;

        // We do not execute from here on forward if there is no connection from a participant
        InternetSocket part_socket = this->coordinator_socket_.do_accept();

        // Participant will only seek to connect when it is about to send a message, otherwise it would not connect
        // Get the message header
        Buffer recv_buffer(sizeof(MulticastMessageHeader));
        size_t bytes_recv = part_socket.do_recv(recv_buffer, MSG_PEEK);

        // Return if the socket closed (recv() returns 0)
        if (bytes_recv == 0) continue;

        part_socket.do_recvall(recv_buffer);

        MulticastMessageHeader header = MulticastMessageHeader::from_buffer(recv_buffer);

        // Get the message data (the message that was sent over multicast)
        Buffer data_buffer(header.size);
        std::string data;
        if (header.size > 0) {
            part_socket.do_recvall(data_buffer);
            data = std::string((char *)data_buffer.data(), header.size);
        }
        MulticastMessage part_req(header.type, header.pid);
        part_req << data;

        std::cout << "[Multicast Request] " << header << "\n";
        std::thread handle_request_thread = std::thread(&Coordinator::handleRequest, part_req, part_socket, this);
        handle_request_thread.detach();
    }
}

void Coordinator::handleRequest(MulticastMessage part_req, InternetSocket part_sock) {
    if (part_req.header().type != MulticastMessageType::INVALID) {
        MulticastMessage ack(MulticastMessageType::ACKNOWLEDGEMENT, part_req.header().pid);
        part_sock.do_sendall(ack.to_buffer());
        part_sock.do_shutdown();
    }
    else {
        MulticastMessage ack(MulticastMessageType::NEGATIVE_ACKNOWLEDGEMENT, part_req.header().pid);
        part_sock.do_sendall(ack.to_buffer());
        part_sock.do_shutdown();
    }
    switch(part_req.header().type) {
        case(MulticastMessageType::PARTICIPANT_REGISTER): {
            this->handleRegister();
            break;
        };
        case(MulticastMessageType::PARTICIPANT_DEREGISTER): {
            this->handleDeregister();
            break;
        };
        case(MulticastMessageType::PARTICIPANT_RECONNECT): {
            this->handleReconnect();
            break;
        };
        case(MulticastMessageType::PARTICIPANT_DISCONNECT): {
            this->handleDisconnect();
            break;
        };
        case(MulticastMessageType::PARTICIPANT_MSEND): {
            this->handleMSend();
            break;
        }
        default: {
            break;
        }
    }
}
