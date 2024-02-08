/************************************************************/
/* SWC         : FOTA Handler                               */
/* Author      : Mohamed Ghoneim and Ahmed Aborehab         */
/* Version     : V 1.0                                      */
/* Date        : 19 Dec 2023                                */
/* Description : SWC for recieving and flashing new image   */
/************************************************************/


#include "OTA.h"
#include "usart.h"

#include <stdlib.h>



/*
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

*/


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








uint8_t Flash_Memory_Write_Payload(uint16_t *Host_Payload, uint32_t Payload_Start_Address, uint8_t Payload_Len)
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
		for(Payload_Counter = 0; Payload_Counter < Payload_Len; Payload_Counter+=2)
        {
			/* Program a byte at a specified address */
			HAL_Status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, Payload_Start_Address + Payload_Counter, Host_Payload[(Payload_Counter/2)]);
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
/*---------------------------------------------------------------------------------------------------------------*/
 void Bootloader_Upload_Sequence(void)
{
  	uint32_t CCC=0;
	/*uint8_t Update_Status = UPDATE_FAILED;*/

    uint8_t Erase =  Perform_Flash_Erase (3,3);
    if (Erase == SUCCESSFUL_ERASE)
    {
    	uint8_t rx_data;
    	uint8_t rx_buffer[50]={0};
    	uint8_t rx_index = 0;
    	//uint8_t receivedData[RAM_HOST_BUFFER_RX_LENGTH];
    	uint32_t receivedData_IDX =0;

        // send ACK
        //uint8_t RAM_Buffer [RAM_HOST_BUFFER_RX_LENGTH];
        //uint16_t Payload_Data_Len = sizeof(receivedData)/sizeof(receivedData[0]);
		//uint8_t TX_ARRAY [1] = {0};
		//memset(RAM_Buffer, 0xFF, RAM_HOST_BUFFER_RX_LENGTH);
        //memset(receivedData, 0xFF, sizeof(receivedData));
        HAL_UART_Transmit(&huart1, (uint8_t*)"START\n", 6, HAL_MAX_DELAY); // Send Start condition
        HAL_Delay(500);

		  while (CCC<RAM_HOST_BUFFER_RX_LENGTH)
		  {

		    /* USER CODE END WHILE */

		    /* USER CODE BEGIN 3 */
			  if (HAL_UART_Receive(&huart1, &rx_data, 1, 1000) == HAL_OK)
			      {
			        if (rx_data == ':')
			        {
			        	rx_index = 0;
			        	rx_buffer[rx_index]=':';
			        	rx_index++;
			        	HAL_UART_Transmit(&huart1, (uint8_t*)"ACK\n", 4, HAL_MAX_DELAY); // Send acknowledgment
			        }
			        else if (rx_data == '\n')
			        {
			        	rx_buffer[rx_index]='\n';
			        	/*
			          rx_buffer[rx_index] = '\0'; // Null terminate the string
			          for(uint8_t Counter=0; Counter<50; Counter++)
			          	  {
			          		  	  if(rx_buffer[Counter]=='\0')
			          		  	  {
			          		  		rx_buffer[Counter]='\n';
			          		  		rx_index++;
			          		  		break;
			          		  	  }
			          			  receivedData[receivedData_IDX]=rx_buffer[Counter];
			          			  receivedData_IDX++;


			          	  }
			          	  */
			          HexParser_vParseData(rx_buffer);

			          HAL_UART_Transmit(&huart1, (uint8_t*)"ACK\n", 4, HAL_MAX_DELAY); // Send acknowledgment
			          rx_index = 0; // Reset buffer index
			        }
			        else
			        {
			          rx_buffer[rx_index++] = rx_data; // Store received character
			          HAL_UART_Transmit(&huart1, (uint8_t*)"ACK\n", 4, HAL_MAX_DELAY); // Send acknowledgment
			        }
			      }
			  CCC++;
			  receivedData_IDX=0;
		  }
		  //Added by Abdallah

		  //uint8_t* byteArray = convertHexCharArrayToByteArray(receivedData, sizeof(receivedData));
		  /**************************************/
		  //Flash_Memory_Write_Payload(binaryData, 0x800C800, sizeof(binaryData));
		  //flashCodeToMemory((uint32_t*)0x800C800, binaryData, binaryArraySize);
/*
		  		if (Writing_cheker == WRITE_FAILED)
		  		{
		  			// BL_Print_Message("writing failed \r\n"); -----------------------> writing NACK

		  		}
		          else if (Writing_cheker == WRITE_PASSED)
		          {
		              // BL_Print_Message("writing Passed \r\n"); -----------------------> writing ACK
		          }
		          else
		          {
		          // send NACK
		          }
		          */
		  /* USER CODE END 3 */
		}

}
/***********Code of Abdullah***********/

uint16_t FlashData[100];

uint8_t HexParser_u8Ascii2Num(uint8_t Copy_u8Ascii)
 {
	uint8_t Local_u8Return = 0;
 	if(Copy_u8Ascii >= '0' && Copy_u8Ascii <= '9')
 	{
 		Local_u8Return = Copy_u8Ascii - '0';
 	}
 	else if(Copy_u8Ascii >='A' && Copy_u8Ascii <='F')
 	{
 		Local_u8Return = Copy_u8Ascii - 55;
 	}
 	return Local_u8Return;
 }


uint32_t upper_bits = 0;

void HexParser_vParseData(uint8_t *Data)
{
	/* Calculate record address */
	uint32_t Address = 0;
	uint32_t Offset=0;
	uint32_t Abs_Address=0;
	uint8_t i;
	uint8_t digit0 , digit1 , digit2 , digit3;

	//uint8_t recordTypeHigh = HexParser_u8Ascii2Num(Data[7]);
	//uint8_t recordTypeLow = HexParser_u8Ascii2Num(Data[8]);
	//uint8_t recordType = (recordTypeHigh << 4) | recordTypeLow;

	    /* If record type is data */
	/*
	    if (recordType == 0x00)
	    {
	        // Data record processing
	    	digit0 = HexParser_u8Ascii2Num(Data[3]);
	    		digit1 = HexParser_u8Ascii2Num(Data[4]);
	    		digit2 = HexParser_u8Ascii2Num(Data[5]);
	    		digit3 = HexParser_u8Ascii2Num(Data[6]);

	    		Address = (0x800C800 | (digit0 << 12)
	    								| (digit1 << 8)
	    								| (digit2 << 4)
	    								| digit3);

	    		//Calculate Length
	    		uint8_t Length_Low , Length_High , Length;
	    		Length_Low = HexParser_u8Ascii2Num(Data[2]);
	    		Length_High = HexParser_u8Ascii2Num(Data[1]);
	    		Length = (Length_High << 4) | Length_Low;

	    		//Store data in FlashArray
	    		for(i = 0; i < Length/2; i++)
	    		{
	    			digit0 = HexParser_u8Ascii2Num(Data[(4*i)+9]);
	    			digit1 = HexParser_u8Ascii2Num(Data[(4*i)+10]);
	    			digit2 = HexParser_u8Ascii2Num(Data[(4*i)+11]);
	    			digit3 = HexParser_u8Ascii2Num(Data[(4*i)+12]);
	    			uint16_t halfWord = ((digit2 << 12) | (digit3 << 8) | (digit0 << 4) | digit1);

	    			// Store half-word in FlashData array
	    			FlashData[i] = halfWord;

	    		}

	    		Flash_Memory_Write_Payload(FlashData, Address,Length);
	    		// Flashing data
	        // ...
	    }
	    // If record type is end-of-file
	    else
	    {
	        // End-of-file record processing
	        // ...
	    }
*/


		    /*		Address = (0x800C800 |
		    		*/





		    		//Calculate Length
		    		uint8_t Length_Low , Length_High , Length;
		    		Length_Low = HexParser_u8Ascii2Num(Data[2]);
		    		Length_High = HexParser_u8Ascii2Num(Data[1]);
		    		Length = (Length_High << 4) | Length_Low;
		    		if((Data[7] == '0' && Data[8] == '1'))
		    		{
		    			//end of file

		    		}else if((Data[7] == '0' && Data[8] == '4'))
		    		{
		    			upper_bits = (uint32_t)HexParser_u8Ascii2Num(Data[9]) * 4096 + HexParser_u8Ascii2Num(Data[10]) * 256 +
		    							HexParser_u8Ascii2Num(Data[11]) * 16 + HexParser_u8Ascii2Num(Data[12]);
		    			//upper_bits = (record[4] << 8) | record[5];
		    			upper_bits <<= 16; // Shift left by 16 bits to form a 32-bit address

		    		}else if((Data[7] == '0' && Data[8] == '5'))
		    		{

		    		}
		    		else{
		    			// Data record processing
			    		digit0 = HexParser_u8Ascii2Num(Data[3]);
		    			digit1 = HexParser_u8Ascii2Num(Data[4]);
		    		    digit2 = HexParser_u8Ascii2Num(Data[5]);
		    			digit3 = HexParser_u8Ascii2Num(Data[6]);

		    			Offset =((digit0 << 12) | (digit1 << 8) | (digit2 << 4) | (digit3));
		    			Abs_Address= Offset|upper_bits;
		    			if(digit0 <0xc)
		    			{
		    				if(digit1<0x8)
		    				{
		    					Address = ((uint32_t)(0x8000000) | Abs_Address) ;
		    				}
		    				else
		    				{
		    					Address = ((uint32_t)(0x8000800) | Abs_Address);
		    				}
		    			}
		    			else
		    			{
		    				if(digit1<0x8)
		    				{
		    					Address = ((uint32_t)(0x800C000) | Abs_Address) ;
		    				}
		    				else
		    				{
		    					Address = ((uint32_t)(0x800C800) | Abs_Address);
		    				}
		    			}

		    			//Store data in FlashArray
		    			for(i = 0; i < Length/2; i++)
		    			{

		    				digit0 = HexParser_u8Ascii2Num(Data[(4*i)+9]);
		    				digit1 = HexParser_u8Ascii2Num(Data[(4*i)+10]);
		    				digit2 = HexParser_u8Ascii2Num(Data[(4*i)+11]);
		    				digit3 = HexParser_u8Ascii2Num(Data[(4*i)+12]);
		    				uint16_t halfWord = ((digit2 << 12) | (digit3 << 8) | (digit0 << 4) | digit1);
		    				// Store half-word in FlashData array
		    				FlashData[i] = halfWord;

		    			}
		    			Flash_Memory_Write_Payload(FlashData, Address,Length);
		    		}



		    		// Flashing data
		        // ...


}

	
			/* -----------------------------------------------------code of Aborehab----------------------------------------- */

        

	

