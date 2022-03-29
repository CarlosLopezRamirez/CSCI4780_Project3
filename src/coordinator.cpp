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
    std::cout << "[Coordinator Message] Coordinator Starting - Listening on Port " 
        + std::to_string(this->localport_) 
        + " with a persistence time of "
        + std::to_string(this->persistence_time_)
        + " seconds"
        + "\n";
    this->is_running_ = true;
    this->coordinator_socket_.do_bind(this->localport_);
    std::cout << "[Coordinator Message] Coordinator Succesfully Binded to Port " + std::to_string(this->localport_) + "\n";
    this->coordinator_socket_.do_listen(10);
    std::cout << "[Coordinator Message] Coordinator Port " + std::to_string(this->localport_) + " Currently Listening With a Backlog of 10" + "\n";
    incoming_messages_thread_ = std::thread(&Coordinator::handleIncomingMessages, this);
    incoming_messages_thread_.join();
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
        MulticastMessage part_req(header.type, header.pid, header.coordinator_time);
        part_req << data;

        std::cout << "[Multicast Request] " << header << "\n";
        this->handleRequest(part_req, part_socket);
    }
}

void Coordinator::handleRequest(MulticastMessage part_req, InternetSocket part_sock) {
    if (part_req.header().type != MulticastMessageType::INVALID) {
        MulticastMessage ack(MulticastMessageType::ACKNOWLEDGEMENT, part_req.header().pid, std::time(0));
        part_sock.do_sendall(ack.to_buffer());
    }
    else {
        MulticastMessage ack(MulticastMessageType::NEGATIVE_ACKNOWLEDGEMENT, part_req.header().pid, std::time(0));
        part_sock.do_sendall(ack.to_buffer());
        part_sock.do_shutdown();
        return;
    }
    switch(part_req.header().type) {
        case(MulticastMessageType::PARTICIPANT_REGISTER): {
            this->handleRegister(part_req, part_sock.host_ip());
            break;
        };
        case(MulticastMessageType::PARTICIPANT_DEREGISTER): {
            this->handleDeregister(part_req);
            break;
        };
        case(MulticastMessageType::PARTICIPANT_RECONNECT): {
            this->handleReconnect(part_req);
            break;
        };
        case(MulticastMessageType::PARTICIPANT_DISCONNECT): {
            this->handleDisconnect(part_req);
            break;
        };
        case(MulticastMessageType::PARTICIPANT_MSEND): {
            this->handleMSend(part_req);
            break;
        }
        default: {
            break;
        }
    }
}

void Coordinator::handleRegister(MulticastMessage part_req, std::string part_ip) {
    this->pids_registered_.insert({part_req.header().pid, part_ip});
    this->pids_connected_.insert({part_req.header().pid, stoi(part_req.body())});
}

void Coordinator::handleDeregister(MulticastMessage part_req) {
    this->pids_registered_.erase(part_req.header().pid);
    return;
}

void Coordinator::handleReconnect(MulticastMessage part_req) {
    // TODO: SEND ALL MESSAGES MISSED WHILE DISCONNECTED
    time_t reconnect_time = std::time(0);
    std::vector<MulticastMessage> missed_messages = pids_disconnected_.at(part_req.header().pid);
    InternetSocket send_sock;
    send_sock.do_connect(this->pids_registered_.at(part_req.header().pid), stoi(part_req.body()));
    for (auto message : missed_messages) {
        // check if the message was sent within the threshold time
        if (difftime(message.header().coordinator_time, reconnect_time) <= this->persistence_time_) {
            send_sock.do_sendall(message.to_buffer());
        }
    }
    pids_disconnected_.erase(part_req.header().pid);
    disconnect_times.erase(part_req.header().pid);
    pids_connected_.insert({part_req.header().pid, stoi(part_req.body())});
    return;
}

void Coordinator::handleDisconnect(MulticastMessage part_req) {
    disconnect_times.insert({part_req.header().pid, std::time(0)});
    pids_connected_.erase(part_req.header().pid);
    std::vector<MulticastMessage> temp;
    this->pids_disconnected_.insert({part_req.header().pid, temp});
    return;
}

void Coordinator::handleMSend(MulticastMessage part_req) {
    // Send message to all who are connected
    for (auto [key, val] : this->pids_connected_) {
        InternetSocket send_sock;
        send_sock.do_connect(this->pids_registered_.at(key), val);
        MulticastMessage tempMessage(MulticastMessageType::MULTI_MESSAGE, part_req.header().pid, part_req.header().coordinator_time);
        tempMessage << part_req.body();
        send_sock.do_sendall(tempMessage.to_buffer());
        send_sock.do_shutdown();
    }
    // Store message in map for those who are disconnected
    for (auto [key, val]: this->pids_disconnected_) {
        // MAKE MESSAGE FROM BODY OF REQUEST
        MulticastMessage tempMessage(MulticastMessageType::MULTI_MESSAGE, part_req.header().pid, part_req.header().coordinator_time);
        tempMessage << part_req.body();
        val.push_back(tempMessage);
    }
    return;
}