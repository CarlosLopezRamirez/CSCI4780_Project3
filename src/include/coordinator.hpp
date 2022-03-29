// File: include/coordinator.hpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#pragma once

#include <string>
#include <iostream>
#include <thread>
#include <set>
#include <unordered_map>
#include <vector>
#include <atomic>

#include "multicast_message.hpp"
#include "inet/internet_socket.hpp"

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
        void handleIncomingMessages();

        void handleRequest(MulticastMessage part_req, std::string part_ip);

        void handleRegister(MulticastMessage part_req, std::string part_ip);

        void handleDeregister(MulticastMessage part_req);

        void handleReconnect(MulticastMessage part_req);

        void handleDisconnect(MulticastMessage part_req);

        void handleMSend(MulticastMessage part_req);
        
        // The port that this coordinator is listening on
        uint16_t localport_;

        // Time (in seconds) that messages will persist for disconnected participants
        int persistence_time_;

        // port that will accept connections from participants
        InternetSocket coordinator_socket_;

        // Threads that is actively listening for messages
        std::thread incoming_messages_thread_;
        
        // True when the Coordinator is not attempting to stop its operation
        std::atomic<bool> is_running_;

        // Set of every participant id that is connected
        // Key: pid
        // Val: port
        std::unordered_map<uint16_t, int> pids_connected_;

        // Set of every participant id that is registered
        // Key: pid
        // Val: ip addr
        std::unordered_map<int, std::string> pids_registered_;

        // Map of every registered but disconnected pid, with a set of messages they miss 
        std::unordered_map<int, std::vector<MulticastMessage>> pids_disconnected_;

        // Map of times that participants disconnected
        std::unordered_map<int, time_t> disconnect_times;
};