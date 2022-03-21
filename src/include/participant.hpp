// File: include/participant.hpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#pragma once

#include <string>
#include <iostream>
#include <unordered_map>

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

        // Function that continually handles messages in the message queue
        void await_message_recv_();

        // Handle participant requests to coordinator
        void handle_request_(MulticastMessage participant_request);

        // Socket to be used by this participant
        InternetSocket participant_socket_;

        // Coordinator address that will be connected to
        std::string remoteaddr;

        // Coordinator port that will be connected to
        int coordinator_port;

        // Maps string to Command, to be used in `parse_input`
        const std::unordered_map<std::string, MulticastMessageType> cmd_map_ = {
            {"register", MulticastMessageType::PARTICIPANT_REGISTER}, 
            {"deregister", MulticastMessageType::PARTICIPANT_DEREGISTER}, 
            {"disconnect", MulticastMessageType::PARTICIPANT_DISCONNECT},
            {"reconnect" ,MulticastMessageType::PARTICIPANT_RECONNECT}, 
            {"msend", MulticastMessageType::PARTICIPANT_MSEND}
        };
};

