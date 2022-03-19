// File: participant.cpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#include <iostream>
#include <fstream>
#include <string>
#include <iostream>

int main(int argc, char** argv) {
    // tools for file input
    std::string file_name;
    std::ifstream file;

    // participant args
    std::string participant_id, log_file, network_info, remoteaddr, remote_port;

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <participant_config_file>\n";
        return EXIT_FAILURE;
    }

    // read from file
    file_name = argv[1];
    file.open(file_name);
    std::getline(file, participant_id);
    std::getline(file, log_file);
    std::getline(file, network_info);

    // parse network info
    remoteaddr = network_info.substr(0, network_info.find(" "));
    remote_port = network_info.substr(network_info.find(" ") + 1);
}