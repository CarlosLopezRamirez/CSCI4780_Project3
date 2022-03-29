// File: mycoordinator.cpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#include <iostream>
#include <fstream>
#include <string>
#include <iostream>

#include "include/coordinator.hpp"

int main (int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <coordinator_config_file>\n";
        return EXIT_FAILURE;
    }

    // read from file
    std::vector<std::string> coordinator_args;
    std::string file_name = argv[1];
    std::ifstream infile(file_name);
    std::string line;
    if (!infile.is_open()) {
        std::cout << "Could not open the file - " << file_name << "\n";
        return EXIT_FAILURE;
    }
    while(std::getline(infile, line)) {
        coordinator_args.push_back(line);
    }
    Coordinator coordinator(stoi(coordinator_args.at(0)), stoi(coordinator_args.at(1)));
    coordinator.start();
    return EXIT_SUCCESS;
}