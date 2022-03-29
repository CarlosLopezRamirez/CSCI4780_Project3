// File: myparticipant.cpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#include <iostream>
#include <fstream>
#include <string>
#include <iostream>

#include "include/participant.hpp"

int main(int argc, char** argv) {
    //std::string pid, log_file, network_info, remoteaddr, remote_port;

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <participant_config_file>\n";
        return EXIT_FAILURE;
    }

    // read from file
    std::vector<std::string> participant_args;
    std::string file_name = argv[1];
    std::ifstream infile(file_name);
    std::string line;
    if (!infile.is_open()) {
        std::cout << "Could not open the file - " << file_name << "\n";
        return EXIT_FAILURE;
    }
    while(std::getline(infile, line)) {
        participant_args.push_back(line);
    }
    

    // parse network info
    std::string remoteaddr = participant_args.at(2).substr(0, participant_args.at(2).find(" "));
    int remote_port = stoi(participant_args.at(2).substr(participant_args.at(2).find(" ") + 1));

    Participant participant(std::stoi(participant_args.at(0)), participant_args.at(1), remoteaddr, remote_port);
    participant.start();
    return EXIT_SUCCESS;
}