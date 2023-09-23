# BootLoader_Community

## Custom Boot-Loader
I'm excited to share my latest achievement: developing a custom bootloader for the STM32F407VG board from scratch.
In my design, I assigned the bootloader program to sector 0 &1, enabling it to automatically run the user_app application if no user input is detected. However, if user input is received, it enters boot mode and listens for data over UART2. And save it to the sector 2 & 3.

To facilitate the process, I wrote a Python script to parse the hex file into a specific telemetry format. The goal was to send the telemetry data using this format. 

Here's a glance of what the bootloader offers:

 CBL_GET_VER_CMD: Fetch the bootloader version.   					  

 CBL_GET_HELP_CMD: Access helpful information.   					  

 CBL_GET_CID_CMD: Read the Chip Identification Number     						

 CBL_RDP_STATUS_CMD: Check RDP status.     				

 CBL_GO_TO_ADDR_CMD: Jump to a specific address.  					

 CBL_FLASH_ERASE_CMD: Erase Flash memory.  				

 CBL_MEM_WRITE_CMD: Write to memory.   					

 CBL_CHANGE_ROP_PROTECT_CMD: Change ROP status 



I'd appreciate any feedback and suggestions, feel free to reach out.





#STM32 #Bootloader #EmbeddedSystems #Innovation #Microcontroller #Development #OpenSource

