
#ifndef _BOOTLOADER_H
#define _BOOTLOADER_H

/* -------------------------- Includes ----------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include "usart.h"
#include "crc.h"



/* ------------------------- Macro Declarations -----------------------------------*/
#define BL_DEBUG_UART            &huart3
#define BL_COMMAND_UART          &huart2
#define CRC_ENGINE_OBJ           &hcrc
#define CBL_VERSION_SIZE         4
#define CBL_COMMANDS_NUMBERS     12

#define MASK_FIrst_TWO_BYTES    (0xFFF)

#define LENGTH_IDENTIFICATION_NUMBER    2

#define DEBUG_INFO_DISABLE              0
#define DEBUG_INFO_ENABLE               1
#define CBL_DEBUG_ENABLE             DEBUG_INFO_ENABLE
#define BL_ENABLE_UART_DEBUG_MESSAGE



#define BL_Buffer_RX_LENGTH                  200
#define VALUE_ZERO       					             0

#define CBL_GET_VER_CMD         					  0X10
#define CBL_GET_HELP_CMD         					  0X11
#define CBL_GET_CID_CMD         						0X12
/* Get Read Protection Status */
#define CBL_RDP_STATUS_CMD          			  0X13
#define CBL_GO_TO_ADDR_CMD         					0X14
#define CBL_FLASH_ERASE_CMD         				0X15
#define CBL_MEM_WRITE_CMD          					0X16
/* Enable/Disable Write Protection */
#define CBL_EN_R_W_PROTECT_CMD          		0X17
#define CBL_MEM_READ_CMD                    0X18
#define CBL_READ_SECTOR_STATUS_CMD          0X19
#define CBL_OTP_READ_CMD                    0X20
/* Get Sector Read/Write Protection Status */
#define CBL_CHANGE_ROP_PROTECT_CMD          0X21



#define CBL_VERSION_ID                      100
#define CBL_SW_MAJOR_VERSION                  1
#define CBL_SW_MINOR_VERSION                  0
#define CBL_SW_PATCH_VERSION                  0

/* CRC_VERIFICATION */
#define CRC_TYPE_SIZE                         4
#define CRC_VERIFICATION_FAILED              0x00                    
#define CRC_VERIFICATION_PASSED              0x01  


#define CBL_SEND_NACK                        0xAB
#define CBL_SEND_ACK                         0xCD



/* Start of Address Function COMMAND */
#define FLASH_SECTOR2_BASE_ADDRESS          0x8008000U 
#define ADDRESS_IS_INVALID              					0x00                    
#define ADDRESS_IS_VALID                					0x01  
#define stm32f407xx_FLASH_SIZE           (1024 * 1024)                       
#define stm32f407xx_CCMDATARAM_SIZE        (64 * 1024)  
#define stm32f407xx_SRAM1_SIZE            (112 * 1024)                          
#define stm32f407xx_SRAM2_SIZE              (16* 1024)                           
#define stm32f407xx_FLASH_END           (stm32f407xx_FLASH_SIZE + FLASH_BASE)                       
#define stm32f407xx_CCMDATARAM_END      (stm32f407xx_CCMDATARAM_SIZE + CCMDATARAM_BASE)  
#define stm32f407xx_SRAM1_END           (stm32f407xx_SRAM1_SIZE + SRAM1_BASE)                          
#define stm32f407xx_SRAM2_END           (stm32f407xx_SRAM2_SIZE + SRAM2_BASE) 






/* Start of ERASE Function COMMAND */
#define ERASE_IS_UNSUCCESSFUL              0x00                    
#define ERASE_IS_SUCCESSFUL                0x01  
#define FLASH_MAX_SECTORS_NUMBER        12 

#define CBL_FLASH_MASS_ERASE			         0xFF
#define SECTOR_NUMBER_INVALID              0x00                    
#define SECTOR_NUMBER_VALID                0x01  

#define HAL_SUCCESSFUL_ERASE 0xFFFFFFFFU


/* Memory Write Payload COMMAND */
#define FLASH_WRITE_PAYLOAD_FALIED         0x00
#define FLASH_WRITE_PAYLOAD_PASSED         0x01




/* Start of RDP Protection COMMAND */
#define RDP_READ_LEVEL_INVALID         0x00
#define RDP_READ_LEVEL_VALID           0x01



/* Start of ROP Protection COMMAND */
#define ROP_LEVEL_INVALID         0x00
#define ROP_LEVEL_VALID           0x01




/* ------------------------- Macro Functions Declarations ------------------------------------*/





/* ------------------------ Data Types Declaration ---------------------------------*/
typedef enum{
	BL_NACK = 0,
	BL_OK
}BL_StatusTypeDef;


typedef void (*PmainAdd)(void);

typedef void (*JumpPtr)(void);
/* ---------------------  function Definitions--------------------------*/

void BL_Print_Mesage(char *format, ...);
BL_StatusTypeDef BL_UART_FETECH_COMMAND(void);

#endif /* _BOOTLOADER_H */