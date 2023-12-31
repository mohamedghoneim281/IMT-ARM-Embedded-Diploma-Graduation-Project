# Jump-Bootloader
Bootloader that can jump to execute APP1 or APP2 depending on input signal.
Micro-Controller used STM32F401.
##How to use
Burn each binary file to the specified location in Linker Script:.
BOOTLOADER in address 0x08000000.
APP1 in address 0x08006400.
APP2 in address 0x800C800.
The video included in the repo shows how it works.
