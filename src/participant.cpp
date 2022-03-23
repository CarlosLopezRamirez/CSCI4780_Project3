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
    remoteaddr(remoteaddr), coordinator_port(remote_port) { }

void Participant::start() {
    is_running_ = true;
    // Check if participant is already registered at coordinator, if yes, then 
    // set registered to true (this handles maybe the participant registering and 
    // then killing the application)
    bool registered_ = previouslyRegistered();
    while (is_running_) {
        // TODO: Implement Multicast Message Constructor
        MulticastMessage participant_request = MulticastMessage();
        try {
            participant_request = prompt_participant();
        }
        catch (std::out_of_range &err) {
            std::cout << "Error: invalid command\n";
            continue;
        }
        handle_request_(participant_request);
    }
    return;
}

void Participant::stop() {
    is_running_ = false;
}

MulticastMessage Participant::prompt_participant() {
    std::cout << "multicast_participant> ";
    std::string user_input;
    std::getline(std::cin, user_input);
    return parse_input(user_input);
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
    MulticastMessage participant_req;

    if (input_vector.size() > 1) {
        std::string req_data;
        req_data = input_vector[1];
        participant_req << req_data;
    }

    return participant_req;
}