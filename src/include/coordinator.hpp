// File: include/coordinator.hpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#pragma once

#include <string>
#include <iostream>
#include <thread>

class Coordinator {
    public:
        // Constructs a coordinator that waits for incoming messages on `localport` and
        // has a persistence time threshold of `persistence_time`
        Coordinator(uint16_t localport, int persistence_time);

        // Begins listening for connections
        void start();

        // Ends multicast coordinator session
        void stop();

    private:
        // The port that this coordinator is listening on
        uint16_t localport_;

        // Time (in seconds) that messages will persist for disconnected participants
        int persistence_time_;
        
        // True when the Coordinator is not attempting to stop its operation
        std::atomic<bool> is_runnning_;
};