// File: participant.cpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#include "include/participant.hpp"
#include "include/multicast_message.hpp"

#include <filesystem>
#include <sstream>
#include <fstream>

Participant::Participant(int pid, std::string log_file, 
    std::string remoteaddr, uint16_t remote_port) : 
    pid_(pid), log_file_path_(log_file),
    remoteaddr(remoteaddr), coordinator_port(remote_port) 
{ }

void Participant::start() {
    this->is_running_ = true;
    while (is_running_) {
        MulticastMessage participant_request = MulticastMessage(MulticastMessageType::INVALID, this->pid_, std::time(0));
        try {
            participant_request = this->prompt_participant();
        }
        catch (std::out_of_range &err) {
            std::cout << "Error: invalid command\n";
            continue;
        }
        handle_request(participant_request);
    }
    return;
}

void Participant::stop() { is_running_ = false; }

MulticastMessage Participant::prompt_participant() {
    std::cout << "multicast_participant> ";
    std::string user_input;
    std::getline(std::cin, user_input);
    return this->parse_input(user_input);
}

MulticastMessage Participant::parse_input(std::string participant_input) {
    std::vector<std::string> input_vector;
    std::stringstream ss(participant_input);
    std::string temp;

    while (getline(ss, temp, ' ')) {
        input_vector.push_back(temp);
    }

    MulticastMessageType req_type = cmd_map_.at(input_vector[0]);

    // TODO: Implement Multicast Message Constructor
    MulticastMessage participant_req(req_type, this->pid_, std::time(0));

    if (input_vector.size() > 1) {
        std::string req_data;
        req_data = input_vector[1];
        participant_req << req_data;
    }

    return participant_req;
}

void Participant::handle_request(MulticastMessage participant_request) {
    switch (participant_request.header().type) {
        case MulticastMessageType::PARTICIPANT_REGISTER: {
            this->handleRegister(participant_request);
            break;
        };
        case MulticastMessageType::PARTICIPANT_DEREGISTER: {
            this->handleDeregister(participant_request);
            break;
        };
        case MulticastMessageType::PARTICIPANT_RECONNECT: {
            this->handleReconnect(participant_request);
            break;
        };
        case MulticastMessageType::PARTICIPANT_DISCONNECT: {
            this->handleDisconnect(participant_request);
            break;
        };
        case MulticastMessageType::PARTICIPANT_MSEND: {
            this->handleMSend(participant_request);
            break;
        };
        case MulticastMessageType::PARTICIPANT_QUIT: {
            this->handleQuit();
            break;
        };
        default: {
            break;
        };
    }
}

void Participant::handleRegister(MulticastMessage participant_request) {
    if (this->registered_) {
        std::cout << "You are already registered" << "\n";
        return;
    }
    InternetSocket participant_send_socket_;
    participant_send_socket_.do_connect(this->remoteaddr, this->coordinator_port);
    participant_send_socket_.do_sendall(participant_request.to_buffer());
    Buffer header_buffer(sizeof(MulticastMessageHeader));
    size_t bytes_recvd = participant_send_socket_.do_recv(header_buffer, MSG_PEEK);

    if (bytes_recvd == 0) {
        return;
    }
    participant_send_socket_.do_recvall(header_buffer);
    MulticastMessageHeader header = MulticastMessageHeader::from_buffer(header_buffer);
    if (header.type == MulticastMessageType::ACKNOWLEDGEMENT) {
        std::cout << "You are now registered and connected to the multicast group" << "\n";
        this->registered_ = true;
        this->connected_ = true;
        this->participant_receive_socket_.do_bind(stoi(participant_request.body()));
        this->participant_receive_socket_.do_listen(10);
        incoming_messages_thread_ = std::thread(&Participant::handleIncomingMulticastMessages, this);
        incoming_messages_thread_.detach();
        return;
    }
    else {
        std::cout << "You were not able to register to the multicast group" << "\n";
        return;
    }
}

void Participant::handleDeregister(MulticastMessage participant_request) {
    if (!this->registered_) {
        std::cout << "You are already deregistered" << "\n";
        return;
    }
    if (this->connected_) {
        // TODO: Ask if this is an ok way to handle this
        std::cout << "Please disconnect before deregistering" << "\n";
        return;
    }
    InternetSocket participant_send_socket_;
    participant_send_socket_.do_connect(this->remoteaddr, this->coordinator_port);
    participant_send_socket_.do_sendall(participant_request.to_buffer());
    Buffer header_buffer(sizeof(MulticastMessageHeader));
    size_t bytes_recvd = participant_send_socket_.do_recv(header_buffer, MSG_PEEK);

    if (bytes_recvd == 0) {
        return;
    }
    participant_send_socket_.do_recvall(header_buffer);
    MulticastMessageHeader header = MulticastMessageHeader::from_buffer(header_buffer);
    if (header.type == MulticastMessageType::ACKNOWLEDGEMENT) {
        std::cout << "You are now deregistered from the multicast group" << "\n";
        this->registered_ = false;
        return;
    }
    else {
        std::cout << "You were not able to register to the multicast group" << "\n";
        return;
    }
}

void Participant::handleReconnect(MulticastMessage participant_request) {
    if (!this->registered_) {
        std::cout << "You must be registered to be able to disconnect/reconnect" << "\n";
        return;
    }
    if (this->connected_) {
        std::cout << "You are already connected" << "\n";
        return;
    }
    InternetSocket participant_send_socket_;
    participant_send_socket_.do_connect(this->remoteaddr, this->coordinator_port);
    participant_send_socket_.do_sendall(participant_request.to_buffer());
    Buffer header_buffer(sizeof(MulticastMessageHeader));
    size_t bytes_recvd = participant_send_socket_.do_recv(header_buffer, MSG_PEEK);

    if (bytes_recvd == 0) {
        return;
    }
    participant_send_socket_.do_recvall(header_buffer);
    MulticastMessageHeader header = MulticastMessageHeader::from_buffer(header_buffer);
    if (header.type == MulticastMessageType::ACKNOWLEDGEMENT) {
        this->connected_ = true;
        this->participant_receive_socket_.do_bind(stoi(participant_request.body()));
        this->participant_receive_socket_.do_listen(10);
        incoming_messages_thread_ = std::thread(&Participant::handleIncomingMulticastMessages, this);
        incoming_messages_thread_.detach();
        std::cout << "You are now reconnected to the multicast group, will begin by sending missed messages" << "\n";
        return;
    }
    else {
        std::cout << "You were not able to reconnect to the multicast group" << "\n";
        return;
    }
}

void Participant::handleDisconnect(MulticastMessage participant_request) {
    if (!this->registered_) {
        std::cout << "You must be registered to be able to disconnect/reconnect" << "\n";
        return;
    }
    if (!this->connected_) {
        std::cout << "You are already disconnected" << "\n";
        return;
    }
    InternetSocket participant_send_socket_;
    participant_send_socket_.do_connect(this->remoteaddr, this->coordinator_port);
    participant_send_socket_.do_sendall(participant_request.to_buffer());
    Buffer header_buffer(sizeof(MulticastMessageHeader));
    size_t bytes_recvd = participant_send_socket_.do_recv(header_buffer, MSG_PEEK);

    if (bytes_recvd == 0) {
        return;
    }
    participant_send_socket_.do_recvall(header_buffer);
    MulticastMessageHeader header = MulticastMessageHeader::from_buffer(header_buffer);
    if (header.type == MulticastMessageType::ACKNOWLEDGEMENT) {        
        this->connected_ = false;
        this->participant_receive_socket_.do_shutdown();
        std::cout << "You are now disconnected from the multicast group" << "\n";
        incoming_messages_thread_.join();
        return;
    }
    else {
        std::cout << "You were not able to disconnect from the multicast group" << "\n";
    }
}

void Participant::handleMSend(MulticastMessage participant_request) {
    if (!this->registered_) {
        std::cout << "You must be registered to send messages to the multicast group" << "\n";
        return;
    }
    if (!this->connected_) {
        std::cout << "You must be connected to send messages to the multicast group" << "\n";
        return;
    }
    InternetSocket participant_send_socket_;
    participant_send_socket_.do_connect(this->remoteaddr, this->coordinator_port);
    participant_send_socket_.do_sendall(participant_request.to_buffer());
    Buffer header_buffer(sizeof(MulticastMessageHeader));
    size_t bytes_recvd = participant_send_socket_.do_recv(header_buffer, MSG_PEEK);

    if (bytes_recvd == 0) {
        return;
    }
    participant_send_socket_.do_recvall(header_buffer);
    MulticastMessageHeader header = MulticastMessageHeader::from_buffer(header_buffer);
    if (header.type == MulticastMessageType::ACKNOWLEDGEMENT) {
        std::cout << "Message sent to multicast group succesfully" << "\n";
        return;
    }
    else {
        std::cout << "Message was not sent succesfully to multicast group" << "\n";
        return;
    }
}

void Participant::handleQuit() {
    if (this->connected_) {
        std::cout << "Please disconnect before quitting" << "\n";
        return;
    }
    std::cout << "Thank you for using this persistent and asynchronous multicast" << "\n";
    this->stop();
}

void Participant::handleIncomingMulticastMessages() {
    PollInfo connection_request;
    connection_request.readable  = true;
    connection_request.writeable = false;

    while (this->connected_) {
        // poll to see if there are any connections
        PollInfo result = this->participant_receive_socket_.do_poll(connection_request, 1 * 1000 /* timeout after 1 second */);
        if (!result.valid || (result.valid && !result.readable)) continue;

        // We do not execute from here on forward if there is no connection from a coordinator
        InternetSocket coordinator_message_socket = participant_receive_socket_.do_accept();

        // Coordinator will only seek to connect when it is about to send a message, otherwise it would not connect
        // Get the message header
        Buffer recv_buffer(sizeof(MulticastMessageHeader));
        size_t bytes_recv = coordinator_message_socket.do_recv(recv_buffer, MSG_PEEK);

        // Return if the socket closed (recv() returns 0)
        if (bytes_recv == 0) continue;

        coordinator_message_socket.do_recvall(recv_buffer);

        MulticastMessageHeader header = MulticastMessageHeader::from_buffer(recv_buffer);

        // Get the message data (the message that was sent over multicast)
        Buffer data_buffer(header.size);
        std::string data;
        if (header.size > 0) {
            coordinator_message_socket.do_recvall(data_buffer);
            data = std::string((char *)data_buffer.data(), header.size);
        }

        std::stringstream time_stringstream_representation;
        time_stringstream_representation << header.coordinator_time;
        std::string time_string_representation = time_stringstream_representation.str();

        // cout received message
        std::string recvd_multi_msg = 
            "[Multicast Message Sent from Participant #" 
            + std::to_string(header.pid) 
            + "]: " 
            + data 
            + "\n"
            + "multicast_participant> ";
        std::cout << recvd_multi_msg;

        // log received message
        std::ofstream outfile;
        outfile.open(this->log_file_path_, std::ios_base::app);
        outfile << recvd_multi_msg;
    }
}