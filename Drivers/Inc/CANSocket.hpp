#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <array>

namespace eae::drivers {

    struct CANFrame {
        uint32_t id{};
        uint8_t dlc{0}; // Data Length Code (how many bytes are actually used)
        std::array<uint8_t, 8> payload{};
        uint64_t ts_ms{}; // Timestamp in milliseconds
    };

    /**
     * \brief Manages the Linux SocketCAN network interface.
     *
     * This class encapsulates a raw CAN socket, providing RAII (Resource Acquisition 
     * Is Initialization) semantics to ensure the network resource is safely opened 
     * and closed without leaking file descriptors.
     */
    class CANSocket {
    public:
        
        explicit CANSocket(const std::string& ifname = "vcan0");
        ~CANSocket();

        // Prevent copying of the network socket
        CANSocket(const CANSocket&) = delete;
        CANSocket& operator=(const CANSocket&) = delete;

        bool open_socket();
        bool recv_frame(CANFrame& frame);
        bool send_frame(const CANFrame& frame);
        void close_socket();

    private:
        std::string iface_;
        int sockfd_{-1}; // Linux socket file descriptor
    };

}