/************************************************************/
/* SWC         : Bootloader Driver                          */
/* Author      : Mohamed Ghoneim                            */
/* Version     : V 1.0.0                                    */
/* Date        : 11 Dec 2023                                */
/* Description : SWC for System Bootloader                  */
/************************************************************/

/*File Guard*/
#ifndef BOOTLOADER_H
#define BOOTLOADER_H


#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <gpio.h>



/*#defines*/
#define APP1_START (0x08006400)			//Origin + Bootloader size
#define APP2_START (0x800C800)			//Origin + Bootloader size + App1 Bank



//Custom Types
typedef enum{
	BOOT,
    APP1,
    APP2
}ChooseAPP;

typedef void (application_t)(void);

typedef struct
{
    uint32_t		stack_addr;     // Stack Pointer
    application_t*	func_pc;        // Program Counter
} JumpStruct;





/*Main Function*/
void Bootloader_Init();

/*Helper Function*/
void Jump_To_App(const uint32_t address);
uint32_t readAddress(uint32_t address);
void deinitEverything();

#endif
