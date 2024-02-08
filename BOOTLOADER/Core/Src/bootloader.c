/************************************************************/
/* SWC         : Bootloader Driver                          */
/* Author      : Mohamed Ghoneim                            */
/* Version     : V 1.0.0                                    */
/* Date        : 11 Dec 2023                                */
/* Description : SWC for System Bootloader                  */
/************************************************************/
#include "bootloader.h"

void Bootloader_Init(){
ChooseAPP SelectedAPP;
if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5)==GPIO_PIN_SET){
    SelectedAPP=APP1;
}
else if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6)==GPIO_PIN_SET)
{
    SelectedAPP=APP2;
}
else{
    /*Stay in Bootloader*/
}


if (SelectedAPP==APP1)
{

    /* Check that the first 40 bytes are not empty*/
    uint8_t emptyCellCount = 0;
			for(uint8_t i=0; i<10; i++)
			{
				if(readAddress(APP1_START + (i*4)) == -1)
					emptyCellCount++;
			}

			if(emptyCellCount != 10)
				Jump_To_App(APP1_START);
}
else if (SelectedAPP==APP2)
{
    /* Check that the first 40 bytes are not empty*/
    uint8_t emptyCellCount = 0;
			for(uint8_t i=0; i<10; i++)
			{
				if(readAddress(APP2_START + (i*4)) == -1)
					emptyCellCount++;
			}

			if(emptyCellCount != 10)
				Jump_To_App(APP2_START);
}

}


/*Helper Functions Implementation*/


uint32_t readAddress(uint32_t address)
{
	uint32_t read_data;
	read_data = *(uint32_t*)(address);
	return read_data;
}

void Jump_To_App(const uint32_t address){

	const JumpStruct* vector_p = (JumpStruct*)address;

	deinitEverything();

	/* let's do The Jump! */
    /* Jump, used asm to avoid stack optimization */
    asm("msr msp, %0; bx %1;" : : "r"(vector_p->stack_addr), "r"(vector_p->func_pc));
}

void deinitEverything()
{
	//-- reset peripherals to guarantee flawless start of user application
	HAL_GPIO_DeInit(GPIOA, 0);
	HAL_GPIO_DeInit(GPIOA, 5);
	HAL_GPIO_DeInit(GPIOA, 6);
	HAL_GPIO_DeInit(GPIOA, 2);
	  __HAL_RCC_GPIOC_CLK_DISABLE();
	  __HAL_RCC_GPIOD_CLK_DISABLE();
	  __HAL_RCC_GPIOB_CLK_DISABLE();
	  __HAL_RCC_GPIOA_CLK_DISABLE();
	HAL_RCC_DeInit();
	HAL_DeInit();
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;
}
