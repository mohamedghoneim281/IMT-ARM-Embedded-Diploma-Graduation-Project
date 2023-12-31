/************************************************************/
/* SWC         : FOTA Handler                               */
/* Author      : Mohamed Ghoneim and Ahmed Aborehab         */
/* Version     : V 1.0                                      */
/* Date        : 19 Dec 2023                                */
/* Description : SWC for recieving and flashing new image   */
/************************************************************/


#include "OTA.h"





 void Bootloader_Send_ACK(uint8_t Replay_Len)
{
	uint8_t Ack_Value[2] = {0};
	Ack_Value[0] = CBL_SEND_ACK;
	Ack_Value[1] = Replay_Len;
	HAL_SPI_Transmit(BL_HOST_COMMUNICATION_SPI, (uint8_t *)Ack_Value, 2, HAL_MAX_DELAY);
}

 void Bootloader_Send_NACK(void)
{
	uint8_t Ack_Value = CBL_SEND_NACK;
	HAL_SPI_Transmit(BL_HOST_COMMUNICATION_SPI, &Ack_Value, 1, HAL_MAX_DELAY);
}


void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint32_t Data_Len)
{
	HAL_SPI_Transmit(BL_HOST_COMMUNICATION_SPI, Host_Buffer, Data_Len, HAL_MAX_DELAY);
}




uint8_t Host_Address_Verification(uint32_t Jump_Address){
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
	if((Jump_Address >= SRAM1_BASE) && (Jump_Address <= STM32F401_SRAM1_END)){
		Address_Verification = ADDRESS_IS_VALID;
	}
	else if((Jump_Address >= FLASH_BASE) && (Jump_Address <= STM32F401_FLASH_END)){
		Address_Verification = ADDRESS_IS_VALID;
	}
	else{
		Address_Verification = ADDRESS_IS_INVALID;
	}
	return Address_Verification;
}






 uint8_t Perform_Flash_Erase(uint8_t Sector_Numebr, uint8_t Number_Of_Sectors)
{
	uint8_t Sector_Validity_Status = INVALID_SECTOR_NUMBER;
	FLASH_EraseInitTypeDef pEraseInit;
	uint8_t Remaining_Sectors = 0;
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	uint32_t SectorError = 0;
	
	if(Number_Of_Sectors > CBL_FLASH_MAX_SECTOR_NUMBER)
    {
		/* Number Of sectors is out of range */
		Sector_Validity_Status = INVALID_SECTOR_NUMBER;
	}
	else{
		if((Sector_Numebr <= (CBL_FLASH_MAX_SECTOR_NUMBER - 1)) || (CBL_FLASH_MASS_ERASE == Sector_Numebr))
        {
			/* Check if user needs Mass erase */
			if(CBL_FLASH_MASS_ERASE == Sector_Numebr)
            {
				pEraseInit.TypeErase = FLASH_TYPEERASE_MASSERASE; /* Flash Mass erase activation */
				// BL_Print_Message("Flash Mass erase activation \r\n");   ---------------------> flash activated

			}
			else
            {
				/* User needs Sector erase */
				// BL_Print_Message("User needs Sector erase \r\n");  --------------------> input 

				Remaining_Sectors = CBL_FLASH_MAX_SECTOR_NUMBER - Sector_Numebr;
				if(Number_Of_Sectors > Remaining_Sectors)
                {
					Number_Of_Sectors = Remaining_Sectors;
				}
				else { /* Nothing */ }
				
				pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS; /* Sectors erase only */
				pEraseInit.Sector = Sector_Numebr;        /* Initial FLASH sector to erase when Mass erase is disabled */
				pEraseInit.NbSectors = Number_Of_Sectors; /* Number of sectors to be erased. */
			}
			
			pEraseInit.Banks = FLASH_BANK_1; /* Bank 1  */
			pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3; /* Device operating range: 2.7V to 3.6V */
			
			/* Unlock the FLASH control register access */
            HAL_Status = HAL_FLASH_Unlock();
			/* Perform a mass erase or erase the specified FLASH memory sectors */
			HAL_Status = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
			if(HAL_SUCCESSFUL_ERASE == SectorError)
            {
				Sector_Validity_Status = SUCCESSFUL_ERASE;
			}
			else
            {
				Sector_Validity_Status = UNSUCCESSFUL_ERASE;
			}
			/* Locks the FLASH control register access */
            HAL_Status = HAL_FLASH_Lock();
		}
		else
        {
			Sector_Validity_Status = UNSUCCESSFUL_ERASE;
		}
	}
	return Sector_Validity_Status;
}



///*
// uint8_t Bootloader_Erase_Flash(uint8_t *Host_Buffer)
//{
//	uint16_t Host_CMD_Packet_Len = 0;
//	uint8_t Erase_Status = 0;
//
//	// BL_Print_Message("Mass erase or sector erase of the user flash \r\n"); -------------------------> erase starts
//		/* Send acknowledgement to the HOST */
//		Bootloader_Send_ACK(1);
//		/* Perform Mass erase or sector erase of the user flash */
//		Erase_Status = Perform_Flash_Erase(Host_Buffer[2], Host_Buffer[3]);
//		if(SUCCESSFUL_ERASE == Erase_Status)
//        {
//			/* Report erase Passed */
//			Bootloader_Send_Data_To_Host((uint8_t *)&Erase_Status, 1);
//			// BL_Print_Message("Successful Erase \r\n");  ---------------------- erase ACK
//
//		}
//		else
//        {
//			/* Report erase failed */
//			Bootloader_Send_Data_To_Host((uint8_t *)&Erase_Status, 1);
//			// BL_Print_Message("Erase request failed !!\r\n"); -------------------->  erase NACK
//		}
//	return Erase_Status;
//}
//
//*/








static uint8_t Flash_Memory_Write_Payload(uint8_t *Host_Payload, uint32_t Payload_Start_Address, uint16_t Payload_Len)
{
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	uint8_t Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	uint16_t Payload_Counter = 0;
	
	/* Unlock the FLASH control register access */
  HAL_Status = HAL_FLASH_Unlock();
	
	if(HAL_Status != HAL_OK)
    {
		Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	}
	else
    {
		for(Payload_Counter = 0; Payload_Counter < Payload_Len; Payload_Counter++)
        {
			/* Program a byte at a specified address */
			HAL_Status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, Payload_Start_Address + Payload_Counter, Host_Payload[Payload_Counter]);
			if(HAL_Status != HAL_OK)
            {
				Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
				break;
			}
			else
            {
				Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_PASSED;
			}
		}
	}
	
	if((FLASH_PAYLOAD_WRITE_PASSED == Flash_Payload_Write_Status) && (HAL_OK == HAL_Status))
    {
		/* Locks the FLASH control register access */
		HAL_Status = HAL_FLASH_Lock();
		if(HAL_Status != HAL_OK)
        {
			Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
		}
		else
        {
			Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_PASSED;
		}
	}
	else
    {
		Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	}
	
	return Flash_Payload_Write_Status;
}





//
//
// uint8_t Bootloader_Memory_Write(uint8_t *Host_Buffer)
// {
//	uint16_t Host_CMD_Packet_Len = 0;
//	uint32_t HOST_Address = 0;
//	uint8_t Payload_Len = 0;
//	uint8_t Address_Verification = ADDRESS_IS_INVALID;
//	uint8_t Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
//	uint8_t Write_status = WRITE_FAILED;
//
//	// BL_Print_Message("Write data into different memories of the MCU \r\n");  ------------------> writing starts
//
//
//		/* Send acknowledgement to the HOST */
//	// Bootloader_Send_ACK(1);
//		/* Extract the start address from the Host packet */
//	HOST_Address = *((uint32_t *)(&Host_Buffer[2]));
//	// BL_Print_Message("HOST_Address = 0x%X \r\n", HOST_Address);-----------------------> Address to write
//	/* Extract the payload length from the Host packet */
//	Payload_Len = Host_Buffer[6];
//	/* Verify the Extracted address to be valid address */
//	Address_Verification = Host_Address_Verification(HOST_Address);
//	if(ADDRESS_IS_VALID == Address_Verification)
//    {
//		/* Write the payload to the Flash memory */
//		Flash_Payload_Write_Status = Flash_Memory_Write_Payload((uint8_t *)&Host_Buffer[7], HOST_Address, Payload_Len);
//	// 	if(FLASH_PAYLOAD_WRITE_PASSED == Flash_Payload_Write_Status)
//    //     {
//	// 		/* Report payload write passed */
//	//     	// Bootloader_Send_Data_To_Host((uint8_t *)&Flash_Payload_Write_Status, 1);
//	// 		Write_status = WRITE_PASSED;
//	// 		// BL_Print_Message("Payload Valid \r\n"); --------------------------> write ACK
//
//	// 	}
//	// else{
//	//     	// BL_Print_Message("Payload InValid \r\n");     ------------------------> write NACK
//	// 		/* Report payload write failed */
//	// 		// Bootloader_Send_Data_To_Host((uint8_t *)&Flash_Payload_Write_Status, 1);
//	// 		Write_status = WRITE_FAILED;
//	// 	}
//	}
//
//    else
//    {
//			/* Report address verification failed */
//		Address_Verification = ADDRESS_IS_INVALID;
//		Bootloader_Send_Data_To_Host((uint8_t *)&Address_Verification, 1);
//    }
//	return Write_status;
//}
//
//









/*---------------------------------------------------------------------------------------------------------------*/









 void Bootloader_Upload_Sequence(void)
{
  	
	uint16_t Payload_Data_Len = 0;
	/*uint8_t Update_Status = UPDATE_FAILED;*/

    uint8_t Erase =  Perform_Flash_Erase (3,2);
    if (Erase == SUCCESSFUL_ERASE)
    {
        // send ACK
        uint8_t RAM_Buffer [RAM_HOST_BUFFER_RX_LENGTH];
        uint32_t Payload_Data_Len = sizeof(RAM_Buffer)/sizeof(RAM_Buffer[0]);
		
		memset(RAM_Buffer, 0xFF, RAM_HOST_BUFFER_RX_LENGTH);
		HAL_SPI_Receive(BL_DEBUG_SPI, &RAM_Buffer, Payload_Data_Len, HAL_MAX_DELAY); //----------------------------SPI

		
		uint8_t Writing_cheker = Flash_Memory_Write_Payload(RAM_Buffer, 0x800C800, Payload_Data_Len);

		if (Writing_cheker == WRITE_FAILED) 
		{
			// BL_Print_Message("writing failed \r\n"); -----------------------> writing NACK
			
		}
        else if (Writing_cheker == WRITE_PASSED)
        {
            // BL_Print_Message("writing Passed \r\n"); -----------------------> writing ACK
        }
    }
    else  
    {
        // send NACK
    }

	
			/* -----------------------------------------------------code of Aborehab----------------------------------------- */

        
}
	

