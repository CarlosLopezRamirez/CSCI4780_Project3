// File: participant.cpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#include "include/participant.hpp"
#include "include/multicast_message.hpp"

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
    // Start the request handling thread
}

void Participant::stop() {

}