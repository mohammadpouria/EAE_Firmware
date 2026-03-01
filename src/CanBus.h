#pragma once
#include <stdint.h>

#define CAN_ID_EXT                  0x00000004U
#define CAN_RTR_DATA                0x00000000U
#define CAN_RTR_REMOTE              0x00000001U

typedef enum {
    HAL_OK       = 0x00,
    HAL_ERROR    = 0x01
} HAL_StatusTypeDef;

// Dummy struct to configure a CAN transmission
typedef struct {
    uint32_t ExtId;    // Extended identifier (J1939 uses 29-bit)
    uint32_t IDE;      // Identifier type (Standard or Extended)
    uint32_t RTR;      // Frame type (Data or Remote)
    uint32_t DLC;      // Data Length Code (0 to 8 bytes)
} CAN_TxHeaderTypeDef;

// Dummy handle type for the CAN peripheral
typedef struct {
    uint32_t Instance;
} CAN_HandleTypeDef;


// CAN BUS CLASS
class CanBus {
private:
    CAN_HandleTypeDef hcan1; // The simulated CAN1 peripheral handle instance
    uint32_t txMailbox;      // The simulated mailbox index for transmission

    // Mock HAL transmission function
    HAL_StatusTypeDef HAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan, 
                                           CAN_TxHeaderTypeDef *pHeader, 
                                           uint8_t aData[], 
                                           uint32_t *pTxMailbox);

    void HAL_CAN_RxFifoMsgPendingCallback(CAN_HandleTypeDef *hcan) {
        // This would be called by the HAL when a message is received in the FIFO
        // For this simulation, we won't implement reception logic
    }   

public:
    void init();
    void sendCoolingCommand(float pumpSpeedPct, float fanSpeedPct);
};