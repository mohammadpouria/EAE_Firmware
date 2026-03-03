#include "CANSocket.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <chrono>

namespace eae::drivers {

    CANSocket::CANSocket(const std::string& ifname) : iface_(ifname) {}

    CANSocket::~CANSocket() {
        close_socket();
    }

    bool CANSocket::open_socket() {
        struct sockaddr_can addr;
        struct ifreq ifr;

        // create a raw CAN socket
        if ((sockfd_ = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
            std::cerr << "[CAN] Error: Failed to open socket\n";
            return false;
        }

        // Specify interface name and get index 
        std::strncpy(ifr.ifr_name, iface_.c_str(), IFNAMSIZ - 1);
        ifr.ifr_name[IFNAMSIZ - 1] = '\0';
        
        if (ioctl(sockfd_, SIOCGIFINDEX, &ifr) < 0) {
            std::cerr << "[CAN] Error: Interface " << iface_ << " not found\n";
            close_socket();
            return false;
        }

        // Configure the socket address structure
        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        // Bind the socket to the CAN interface
        if (bind(sockfd_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            std::cerr << "[CAN] Error: Failed to bind to " << iface_ << "\n";
            close_socket();
            return false;
        }

        return true;
    }

    bool CANSocket::recv_frame(CANFrame& rx_frame) {
        if (sockfd_ < 0) return false;

        struct can_frame linux_frame;
        // Non-blocking read
        ssize_t nbytes = recv(sockfd_, &linux_frame, sizeof(struct can_frame), MSG_DONTWAIT);
        
        if (nbytes == sizeof(struct can_frame)) {
            rx_frame.id = linux_frame.can_id & CAN_EFF_MASK; // Strip extended flags if any
            rx_frame.dlc = linux_frame.can_dlc; // Data Length Code
            std::copy(linux_frame.data, linux_frame.data + linux_frame.can_dlc, rx_frame.payload.begin()); // Copy data bytes
            
            // Tag with current timestamp
            auto now = std::chrono::steady_clock::now().time_since_epoch();
            rx_frame.ts_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
            return true;
        }
        return false;
    }

    bool CANSocket::send_frame(const CANFrame& tx_frame) {
        if (sockfd_ < 0) return false;

        struct can_frame linux_frame;
        std::memset(&linux_frame, 0, sizeof(struct can_frame)); // Clear the structure
        
        linux_frame.can_id = tx_frame.id; // Set the CAN ID
        linux_frame.can_dlc = tx_frame.dlc; // Set the Data Length Code
        std::copy(tx_frame.payload.begin(), tx_frame.payload.begin() + linux_frame.can_dlc, linux_frame.data); // Copy data bytes

        ssize_t nbytes = write(sockfd_, &linux_frame, sizeof(struct can_frame));
        return (nbytes == sizeof(struct can_frame));
    }

    void CANSocket::close_socket() {
        if (sockfd_ >= 0) {
            close(sockfd_);
            sockfd_ = -1;
        }
    }

}