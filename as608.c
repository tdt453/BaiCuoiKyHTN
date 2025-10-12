#include "as608.h"
#include <string.h>

extern UART_HandleTypeDef huart1;

static bool FP_SendPacket(uint8_t *payload, uint16_t payload_len);
static uint16_t FP_ReceivePacket(uint8_t *buf, uint16_t buf_max, uint32_t timeout);

static bool FP_SendPacket(uint8_t *payload, uint16_t payload_len) {
    uint8_t pkt[256];
    if (payload_len > 240) return false;
    pkt[0]=0xEF; pkt[1]=0x01;
    pkt[2]=0xFF; pkt[3]=0xFF; pkt[4]=0xFF; pkt[5]=0xFF;
    pkt[6]=0x01;
    uint16_t len_field = payload_len + 2;
    pkt[7] = (len_field>>8)&0xFF;
    pkt[8] = len_field & 0xFF;
    memcpy(&pkt[9], payload, payload_len);
    uint16_t sum = 0;
    for (int i=6; i<9+payload_len; i++) sum += pkt[i];
    pkt[9+payload_len] = (sum>>8)&0xFF;
    pkt[10+payload_len] = sum & 0xFF;
    return (HAL_UART_Transmit(&huart1, pkt, 11+payload_len, FP_UART_TIMEOUT) == HAL_OK);
}

static uint16_t FP_ReceivePacket(uint8_t *buf, uint16_t buf_max, uint32_t timeout) {
    uint8_t header[9];
    if (HAL_UART_Receive(&huart1, header, 9, timeout) != HAL_OK) return 0;
    if (header[0]!=0xEF || header[1]!=0x01) return 0;
    uint16_t len_field = (header[7]<<8)|header[8];
    if (len_field + 9 > buf_max) return 0;
    memcpy(buf, header, 9);
    if (HAL_UART_Receive(&huart1, buf+9, len_field, timeout) != HAL_OK) return 0;
    return 9+len_field;
}

bool FP_GetImage(void) {
    uint8_t pl[1]={0x01}; uint8_t rx[64];
    if (!FP_SendPacket(pl,1)) return false;
    uint16_t r=FP_ReceivePacket(rx,sizeof(rx),FP_UART_TIMEOUT);
    return (r>=11 && rx[9]==0x00);
}

bool FP_Image2Tz(uint8_t bufferID) {
    uint8_t pl[2]={0x02,bufferID}; uint8_t rx[64];
    if (!FP_SendPacket(pl,2)) return false;
    uint16_t r=FP_ReceivePacket(rx,sizeof(rx),FP_UART_TIMEOUT);
    return (r>=11 && rx[9]==0x00);
}

bool FP_RegModel(void) {
    uint8_t pl[1]={0x05}; uint8_t rx[64];
    if (!FP_SendPacket(pl,1)) return false;
    uint16_t r=FP_ReceivePacket(rx,sizeof(rx),FP_UART_TIMEOUT);
    return (r>=11 && rx[9]==0x00);
}

bool FP_Store(uint16_t id) {
    uint8_t pl[4]={0x06,0x01,(id>>8)&0xFF,id&0xFF}; uint8_t rx[64];
    if (!FP_SendPacket(pl,4)) return false;
    uint16_t r=FP_ReceivePacket(rx,sizeof(rx),FP_UART_TIMEOUT);
    return (r>=11 && rx[9]==0x00);
}

int16_t FP_Search(void) {
    uint8_t pl[6]={0x04,0x01,0x00,0x00,0x00,(uint8_t)MAX_ID_SELECT}; uint8_t rx[64];
    if (!FP_SendPacket(pl,6)) return -1;
    uint16_t r=FP_ReceivePacket(rx,sizeof(rx),FP_UART_TIMEOUT);
    if (r>=13 && rx[9]==0x00) return ((int16_t)rx[10]<<8)|rx[11];
    return -1;
}

bool FP_DeleteAll(void) {
    uint8_t pl[1]={0x0D}; uint8_t rx[64];
    if (!FP_SendPacket(pl,1)) return false;
    uint16_t r=FP_ReceivePacket(rx,sizeof(rx),FP_UART_TIMEOUT);
    return (r>=11 && rx[9]==0x00);
}
