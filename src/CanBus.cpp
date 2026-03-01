#include "CanBus.h"
#include <iostream>
#include <iomanip>

void CanBus::init() {
    hcan1.Instance = 0x00000001U; // Representing CAN1
    std::cout << "[CAN BUS] CAN1 Peripheral Initialized.\n";
}

// MOCK HAL Tx IMPLEMENTATION
HAL_StatusTypeDef CanBus::HAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan, 
                                               CAN_TxHeaderTypeDef *pHeader, 
                                               uint8_t aData[], 
                                               uint32_t *pTxMailbox) 
{
    // For simulation, we print the raw hex payload to the terminal
    std::cout << "[CAN TX Mailbox " << *pTxMailbox << "] "
              << "ID: 0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << pHeader->ExtId << std::dec
              << " | DLC: " << pHeader->DLC << " | Payload: [";
    
    for(uint32_t i = 0; i < pHeader->DLC; i++) {
        std::cout << " 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)aData[i];
    }
    std::cout << " ]\n" << std::dec; // Reset to decimal formatting
    
    return HAL_OK;
}

// APPLICATION LAYER
void CanBus::sendCoolingCommand(float pumpSpeedPct, float fanSpeedPct) {
    CAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[8] = {0}; // Initialize 8-byte payload
    
    // Configure J1939 Header (29-bit Extended ID)
    TxHeader.ExtId = 0x18FFF000; 
    TxHeader.IDE = CAN_ID_EXT;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = 8; // Standard J1939 payload is exactly 8 bytes

    // Packing the Payload
    // Mapping percentages to a single byte
    uint8_t pumpRaw = static_cast<uint8_t>(pumpSpeedPct * 2.5f);
    uint8_t fanRaw  = static_cast<uint8_t>(fanSpeedPct * 2.5f);

    TxData[0] = pumpRaw; // Byte 0: Commanded Pump Speed
    TxData[1] = fanRaw;  // Byte 1: Commanded Fan Speed
    // Bytes 2-7 remain 0x00 (Reserved)

    // Transmit using the HAL API
    txMailbox = 0; // Hardcoded mailbox 0 for emulation
    
    if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &txMailbox) != HAL_OK) {
        std::cout << "[CAN HAL] Transmission Error!\n";
    }
}