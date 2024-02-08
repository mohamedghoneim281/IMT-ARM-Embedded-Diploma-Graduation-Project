/************************************************************/
/* SWC         : FOTA Handler                               */
/* Author      : Mohamed Ghoneim and Ahmed Aborehab         */
/* Version     : V 1.0                                      */
/* Date        : 19 Dec 2023                                */
/* Description : SWC for recieving and flashing new image   */
/************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "main.h"
//#include "spi.h"


#ifndef OTA_H
#define OTA_H





//#define BL_DEBUG_SPI                 &hspi2
//#define BL_HOST_COMMUNICATION_SPI  	 &hspi2









#define RAM_HOST_BUFFER_RX_LENGTH     86000


/* Write status */

#define WRITE_PASSED 			  0x01
#define WRITE_FAILED			  0x00

/* Jump Status */





/* Update Completion */
#define UPDATE_FAILED               0x00
#define UPDATE_FINISHED             0x01

/* New App */
#define New_App_Sector              5
#define No_Sectors   				1




#define CBL_SEND_NACK                0xAB
#define CBL_SEND_ACK                 0xCD

/* Start address of sector 2 */
#define FLASH_SECTOR2_BASE_ADDRESS    0x08000000

#define ADDRESS_IS_INVALID           0x00
#define ADDRESS_IS_VALID             0x01

#define STM32F401_SRAM1_SIZE          (64 * 1024)
#define STM32F401_FLASH_SIZE          (256 * 1024)

#define STM32F401_SRAM1_END           (SRAM1_BASE + STM32F401_SRAM1_SIZE)
#define STM32F401_FLASH_END           (FLASH_BASE + STM32F401_FLASH_SIZE)

/* CBL_FLASH_ERASE_CMD */
#define CBL_FLASH_MAX_SECTOR_NUMBER  12
#define CBL_FLASH_MASS_ERASE         0xFF   

#define INVALID_SECTOR_NUMBER        0x00
#define VALID_SECTOR_NUMBER          0x01
#define UNSUCCESSFUL_ERASE           0x02
#define SUCCESSFUL_ERASE             0x03

#define HAL_SUCCESSFUL_ERASE         0xFFFFFFFFU

/* CBL_MEM_WRITE_CMD */
#define FLASH_PAYLOAD_WRITE_FAILED   0x00
#define FLASH_PAYLOAD_WRITE_PASSED   0x01








//func declerations

uint8_t Bootloader_Erase_Flash(uint8_t *Host_Buffer);
uint8_t Bootloader_Memory_Write(uint8_t *Host_Buffer);

void Bootloader_Upload_Sequence(void);

void Bootloader_Send_ACK(uint8_t Replay_Len);
void Bootloader_Send_NACK(void);
void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len);

uint8_t Perform_Flash_Erase(uint8_t Sector_Numebr, uint8_t Number_Of_Sectors);
uint8_t HexParser_u8Ascii2Num(uint8_t Copy_u8Ascii);
void HexParser_vParseData(uint8_t *Data);
uint8_t Flash_Memory_Write_Payload(uint16_t *Host_Payload, uint32_t Payload_Start_Address, uint8_t Payload_Len);

HAL_StatusTypeDef flashCodeToMemory(uint32_t* flashAddress, uint8_t* codeBytes, uint32_t codeSize);

#endif
