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

        if (part_req.header().type != MulticastMessageType::INVALID) {
            MulticastMessage ack(MulticastMessageType::ACKNOWLEDGEMENT, part_req.header().pid, std::time(0));
            part_socket.do_sendall(ack.to_buffer());
            std::cout << "[Participant Request] " << header << "\n";
            std::string part_ip = part_socket.remote_addr().substr(0, part_socket.remote_addr().find(":"));
            this->handleRequest(part_req, part_ip);
        }
        else {
            MulticastMessage ack(MulticastMessageType::NEGATIVE_ACKNOWLEDGEMENT, part_req.header().pid, std::time(0));
            part_socket.do_sendall(ack.to_buffer());
            return;
        }
    }
}

void Coordinator::handleRequest(MulticastMessage part_req, std::string part_ip) {
    switch(part_req.header().type) {
        case(MulticastMessageType::PARTICIPANT_REGISTER): {
            this->handleRegister(part_req, part_ip);
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
    if (this->pids_disconnected_.count(part_req.header().pid) > 0) {
        std::remove(this->pids_disconnected_.at(part_req.header().pid).c_str());
        this->pids_disconnected_.erase(part_req.header().pid);
    }
    return;
}

void Coordinator::handleReconnect(MulticastMessage part_req) {
    // TODO: SEND ALL MESSAGES MISSED WHILE DISCONNECTED
    // OPEN FILE, FOR EACH LINE, SEND MESSAGE
    std::ifstream msgfile;
    std::string file_path = pids_disconnected_.at(part_req.header().pid);
    msgfile.open(file_path);
    std::string msg_body;
    uint16_t msg_pid;
    time_t msg_time;
    std::string line;
    while (std::getline(msgfile, line)) {
        std::istringstream iss(line);
        iss >> msg_body >> msg_pid >> msg_time;
        if (difftime(msg_time, disconnect_times.at(part_req.header().pid)) <= this->persistence_time_) {
            MulticastMessage missed_msg(MulticastMessageType::MULTI_MESSAGE, msg_pid, msg_time);
            missed_msg << msg_body;
            InternetSocket send_sock;
            send_sock.do_connect(this->pids_registered_.at(part_req.header().pid), stoi(part_req.body()));
            send_sock.do_sendall(missed_msg.to_buffer());
        }
    }
    std::remove(file_path.c_str());
    pids_disconnected_.erase(part_req.header().pid);
    disconnect_times.erase(part_req.header().pid);
    pids_connected_.insert({part_req.header().pid, stoi(part_req.body())});
    return;
}

void Coordinator::handleDisconnect(MulticastMessage part_req) {
    disconnect_times.insert({part_req.header().pid, std::time(0)});
    pids_connected_.erase(part_req.header().pid);
    std::string file_path = std::to_string(part_req.header().pid) + "_missed_msgs.txt";
    std::ofstream outfile(file_path);
    outfile.close();
    this->pids_disconnected_.insert({part_req.header().pid, file_path});
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
    std::cout << "[Message Sent to Group] " << part_req.body() << "\n";
    // Store message in map for those who are disconnected
    for (auto [key, val]: this->pids_disconnected_) {
        std::ofstream fileout;
        fileout.open(val, std::ios_base::app);
        fileout << part_req.body() << " " << part_req.header().pid << " " << part_req.header().coordinator_time << "\n";
        fileout.close();
    }
    return;
}