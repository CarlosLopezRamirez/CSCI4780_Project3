// File: include/participant.hpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#pragma once

#include <string>
#include <iostream>
#include <unordered_map>
#include <thread>

#include "multicast_message.hpp"
#include "inet/internet_socket.hpp"

class Participant {
    public:
        // Constructs a client identified by `pid` that logs all received multicast messages in 
        // `log_file` after connecting to the coordinator at address `remoteaddr` on port `remote_port`
        Participant(int pid, std::string log_file, std::string remoteaddr, uint16_t remote_port);

        // Establishes connection with the coordinator and begins multicast process
        void start();

        // Ends this participants multicast session
        void stop();
    private:
        // Returns a `MulticastMessage` constructed from the parsing of the user's command input
        MulticastMessage prompt_participant();

        // Returns a `MulticastMessage` representation of a string that contains the user's input
        MulticastMessage parse_input(std::string participant_input);

        // Handle participant requests to coordinator
        void handle_request(MulticastMessage participant_request);

        // Handle Register Command
        void handleRegister(MulticastMessage participant_request);

        // Handle Deregister Command
        void handleDeregister(MulticastMessage participant_request);

        // Handle Reconnect Command
        void handleReconnect(MulticastMessage participant_request);

        // Handle Disconnect Command
        void handleDisconnect(MulticastMessage participant_request);

        // Handle MSend Command
        void handleMSend(MulticastMessage participant_request);

        // Get ACK or NEG_ACK
        MulticastMessageHeader getACK();

        // Handle Quit Command
        void handleQuit();

        // Handle all messages that are sent by other participants
        void handleIncomingMulticastMessages();

        // Socket to be used by this participant to send messages
        InternetSocket participant_send_socket_;

        // Socket to be used by this participant to receive messages
        InternetSocket participant_receive_socket_;

        // Thread to be used for handling incoming multicast messages
        std::thread incoming_messages_thread_;

        // Coordinator address that will be connected to
        std::string remoteaddr;

        // Coordinator port that will be connected to
        int coordinator_port;

        // Port to bind to receive messages
        int register_port_;

        // ID of this participant
        int pid_;

        // Is the participant registered
        bool registered_ = false;

        // Is the participant connected
        bool connected_ = false;

        // Is the participant running
        bool is_running_;

        // path of log_file
        std::string log_file_path_;

        // Maps string to Command, to be used in `parse_input`
        const std::unordered_map<std::string, MulticastMessageType> cmd_map_ = {
            {"register", MulticastMessageType::PARTICIPANT_REGISTER}, 
            {"deregister", MulticastMessageType::PARTICIPANT_DEREGISTER}, 
            {"disconnect", MulticastMessageType::PARTICIPANT_DISCONNECT},
            {"reconnect" ,MulticastMessageType::PARTICIPANT_RECONNECT}, 
            {"msend", MulticastMessageType::PARTICIPANT_MSEND},
            {"quit", MulticastMessageType::PARTICIPANT_QUIT}
        };
};