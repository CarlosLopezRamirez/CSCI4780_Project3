// File: mycoordinator.cpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#include <iostream>
#include <fstream>
#include <string>
#include <iostream>

#include "include/coordinator.hpp";

int main (int argc, char** argv) {
    // tools for file input
    std::string file_name;
    std::ifstream file;

    // coordinator args
    std::string localport, persistence_time;

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << "<coordinator_config_file>\n";
        return EXIT_FAILURE;
    }

    // read from file
    file_name = argv[1];
    file.open(file_name);
    std::getline(file, localport);
    std::getline(file, persistence_time);

    Coordinator coordinator(std::stoi(localport), std::stoi(persistence_time));
    coordinator.start();
    return EXIT_SUCCESS;
}