 /* -------------------------- Includes ----------------------------------------*/
 #include "Bootloader.h"
 
 
 
/* ----------------------- Private typedef ------------------------------------*/



/* ------------------------- Private define -----------------------------------*/


/* ------------------------- Private macro ------------------------------------*/



/* ------------------------ Private variables Definitions ---------------------------------*/
static uint8_t Data_Received_Buffer[BL_Buffer_RX_LENGTH]={0};

static uint8_t Bootloader_CMDS[CBL_COMMANDS_NUMBERS]={
 CBL_GET_VER_CMD,         					  
 CBL_GET_HELP_CMD,        					  
 CBL_GET_CID_CMD,         						
 CBL_RDP_STATUS_CMD,          				
 CBL_GO_TO_ADDR_CMD,         					
 CBL_FLASH_ERASE_CMD,         				
 CBL_MEM_WRITE_CMD,          					
 CBL_EN_R_W_PROTECT_CMD,          		
 CBL_MEM_READ_CMD,                    
 CBL_READ_SECTOR_STATUS_CMD, 
 CBL_OTP_READ_CMD,	
 CBL_CHANGE_ROP_PROTECT_CMD
};



/* --------------------- Private function prototypes --------------------------*/
static void Bootloader_GET_Version(uint8_t *Host_Buffer);         					  
static void Bootloader_GET_HELP(uint8_t *Host_Buffer);         					  
static void Bootloader_GET_Chip_Identification_Number(uint8_t *Host_Buffer);        						
static void Bootloader_Read_Protection_Level(uint8_t *Host_Buffer);         				
static void Bootloader_Jump_To_Address(uint8_t *Host_Buffer);        					
static void Bootloader_FLASH_ERASE(uint8_t *Host_Buffer);         				
static void Bootloader_MEM_WRITE(uint8_t *Host_Buffer);          					
static void Bootloader_Enable_RW_PROTECT(uint8_t *Host_Buffer);          		
static void Bootloader_MEM_READ(uint8_t *Host_Buffer);                    
static void Bootloader_READ_SECTOR_Protection_STATUS(uint8_t *Host_Buffer);          
static void Bootloader_OTP_READ(uint8_t *Host_Buffer);                   
static void Bootloader_CHANGE_READ_OUT_PROTECTION_LEVEL(uint8_t *Host_Buffer);
static uint8_t BL_CRC_Verify(uint8_t *Pdata, uint8_t Data_Len, uint32_t Host_CRC32);
static void Bootloader_Send_NACK();
static void Bootloader_Send_ACK(uint8_t Reply_Len);
static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint8_t Data_Len);
static void BootLoader_Jump_To_User_App(void);
static uint8_t Bootloader_Jump_Address_Verification(uint32_t Jump_Address);
static uint8_t Perform_Flash_Erase(uint8_t Number_Sector, uint8_t Number_Of_Sectors);
static uint8_t Flash_Memory_Write_Payload(uint8_t *Host_Payload, uint16_t Payload_Len, uint32_t Start_address);
static uint8_t CBL_stm32f407_GET_RDP_Level();
static uint8_t Chane_ROP_LEVEL(uint32_t ROP_Level);



/* --------------------- Private function Definitions --------------------------*/


static void BootLoader_Jump_To_User_App(void){
	 /* value of the main Stack pointer of our main application */
	uint32_t MSP_value = *((volatile uint32_t *)(FLASH_SECTOR2_BASE_ADDRESS));
	
	 /* Reset handler definition function of our main application */
	uint32_t MainAppAdd = *((volatile uint32_t *)(FLASH_SECTOR2_BASE_ADDRESS + 4));

	/* Fetech the reset Handler address of the user application */
	PmainAdd ResetAdd = (PmainAdd)MainAppAdd;
	
	/* Set Main Stack Pointer */
	__set_MSP(MSP_value);
	
	/* Deinitialized Modules */
	HAL_RCC_DeInit();
	
	/* Jump to Application Reset Handler */
	ResetAdd();
}


static uint8_t BL_CRC_Verify(uint8_t *Pdata, uint8_t Data_Len, uint32_t Host_CRC32){
	  uint8_t Status_CRC = CRC_VERIFICATION_FAILED; 
		uint32_t CRC_returned_value = 0;
	  uint32_t Data_Counter = 0;
	  uint32_t Data_Buffer = 0;
	 for(Data_Counter =0 ; Data_Counter < Data_Len; Data_Counter++){
		 Data_Buffer = (uint32_t)Pdata[Data_Counter];
		 CRC_returned_value = HAL_CRC_Accumulate(CRC_ENGINE_OBJ, &Data_Buffer, Data_Len);
	 }
	 
    /* Reset the CRC VERIFICATION unit  */
	  __HAL_CRC_DR_RESET(CRC_ENGINE_OBJ);
	 

		/* compare CRC host and CRC_BL  */
	if(Host_CRC32 == CRC_returned_value)
		 Status_CRC = CRC_VERIFICATION_PASSED;
	else
		 Status_CRC = CRC_VERIFICATION_FAILED; 
	
	return Status_CRC;
}




static void Bootloader_Send_NACK(){
	uint8_t ACK_Value =0;
	ACK_Value = CBL_SEND_NACK;
	Bootloader_Send_Data_To_Host(&ACK_Value, 1);
}
	
static void Bootloader_Send_ACK(uint8_t Reply_Len){
	uint8_t ACK_Value[2] ={0};
	ACK_Value[0] = CBL_SEND_NACK;
	ACK_Value[1] = Reply_Len;
	Bootloader_Send_Data_To_Host(ACK_Value, 2);
}


static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer, uint8_t Data_Len){
		  HAL_UART_Transmit(BL_COMMAND_UART, Host_Buffer, Data_Len, HAL_MAX_DELAY);
}

static void Bootloader_GET_Version(uint8_t *Host_Buffer){
	 uint8_t BL_VERSION[CBL_VERSION_SIZE]={CBL_VERSION_ID, CBL_SW_MAJOR_VERSION, CBL_SW_MINOR_VERSION, CBL_SW_PATCH_VERSION};
	 uint8_t Host_Packet_Len = 0;
	 uint32_t Host_CRC32_value = 0;
	 
	 
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	 BL_Print_Mesage("read the Bootloader version from the MCU \r\n");
#endif
	 /* Extract the CRC32 and packet Length sent by host */
	 Host_Packet_Len = Host_Buffer[0]  + 1;
	 Host_CRC32_value = *((uint32_t *)((Host_Buffer + Host_Packet_Len) - CRC_TYPE_SIZE));
	 
	 
	 /* CRC Verifications */
	 if(CRC_VERIFICATION_PASSED == BL_CRC_Verify(Host_Buffer, Host_Packet_Len - CRC_TYPE_SIZE, Host_CRC32_value)){
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	  BL_Print_Mesage("CRC VERIFICATION_PASSED  \r\n");
#endif
	   Bootloader_Send_ACK(CBL_VERSION_SIZE);
		 Bootloader_Send_Data_To_Host(BL_VERSION, CBL_VERSION_SIZE);
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	 BL_Print_Mesage("Bootloadrer Ver. %d.%d.%d \r\n", BL_VERSION[1], BL_VERSION[2], BL_VERSION[3]);
#endif
	 }
	 else {
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	 BL_Print_Mesage("CRC VERIFICATION_Failed  \r\n");
#endif
	   Bootloader_Send_NACK();
	 }
 } 
 
static void Bootloader_GET_HELP(uint8_t *Host_Buffer){
	 uint8_t Host_Packet_Len = 0;
	 uint32_t Host_CRC32_value = 0;
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	 BL_Print_Mesage("Read the Commands Supported by Bootloader  \r\n");
#endif
	 
	 /* Extract the CRC32 and packet Length sent by host */
	 Host_Packet_Len = Host_Buffer[0]  + 1;
	 Host_CRC32_value = *((uint32_t *)((Host_Buffer + Host_Packet_Len) - CRC_TYPE_SIZE));
	 
	 
	 /* CRC Verifications */
	 if(CRC_VERIFICATION_PASSED == BL_CRC_Verify(Host_Buffer, Host_Packet_Len - CRC_TYPE_SIZE, Host_CRC32_value)){
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Mesage("CRC VERIFICATION_PASSED  \r\n");
#endif
	   Bootloader_Send_ACK(CBL_COMMANDS_NUMBERS);
		 Bootloader_Send_Data_To_Host(Bootloader_CMDS, CBL_COMMANDS_NUMBERS);
		 
	 }
	 else {
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Mesage("CRC VERIFICATION_Failed  \r\n");
#endif
	   Bootloader_Send_NACK();
	 }
}      

 
static void Bootloader_GET_Chip_Identification_Number(uint8_t *Host_Buffer){
	 uint8_t Host_Packet_Len = 0;
	 uint32_t Host_CRC32_value = 0;
	 uint16_t Identification_Number = 0 ;
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	 BL_Print_Mesage("Read the Chip Identification Number Supported by Bootloader  \r\n");
#endif
	 
	 /* Extract the CRC32 and packet Length sent by host */
	 Host_Packet_Len = Host_Buffer[0]  + 1;
	 Host_CRC32_value = *((uint32_t *)((Host_Buffer + Host_Packet_Len) - CRC_TYPE_SIZE));
	 
	 
	 /* CRC Verifications */
	 if(CRC_VERIFICATION_PASSED == BL_CRC_Verify(Host_Buffer, Host_Packet_Len - CRC_TYPE_SIZE, Host_CRC32_value)){
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Mesage("CRC VERIFICATION_PASSED  \r\n");
#endif
		  /* Read the MCU Chip Identification Number */
	   Identification_Number = (uint16_t)(DBGMCU->IDCODE  & MASK_FIrst_TWO_BYTES);
		 
		  /* Report the MCU Chip Identification Number to Host */
		 Bootloader_Send_ACK(LENGTH_IDENTIFICATION_NUMBER);
		 Bootloader_Send_Data_To_Host((uint8_t *)&Identification_Number, LENGTH_IDENTIFICATION_NUMBER);
		 
	 }
	 else {
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	BL_Print_Mesage("CRC VERIFICATION_Failed  \r\n");
#endif
	   Bootloader_Send_NACK();
	 }
 }        				
 

static uint8_t CBL_stm32f407_GET_RDP_Level(){
	 FLASH_OBProgramInitTypeDef OBProgramInitTypeDef;
	/*  Get the Option byte configuration */ 
	HAL_FLASHEx_OBGetConfig(&OBProgramInitTypeDef);
   return (uint8_t)(OBProgramInitTypeDef.RDPLevel);
}


static void Bootloader_Read_Protection_Level(uint8_t *Host_Buffer){
	 uint8_t Host_Packet_Len = 0;
	 uint32_t Host_CRC32_value = 0;
	 uint8_t RDP_Level = 0;
	 
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	 BL_Print_Mesage("the flash read protection out the level \r\n\r\n");
#endif
	 /* Extract the CRC32 and packet Length sent by host */
	 Host_Packet_Len = Host_Buffer[0]  + 1;
	 Host_CRC32_value = *((uint32_t *)((Host_Buffer + Host_Packet_Len) - CRC_TYPE_SIZE));
	 
	 /* CRC Verifications */
	 if(CRC_VERIFICATION_PASSED == BL_CRC_Verify(Host_Buffer, Host_Packet_Len - CRC_TYPE_SIZE, Host_CRC32_value)){
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	  BL_Print_Mesage("CRC VERIFICATION_PASSED  \r\n");
#endif
		  Bootloader_Send_ACK(1);
		/* Read protection level */
		  RDP_Level = CBL_stm32f407_GET_RDP_Level();
		  Bootloader_Send_Data_To_Host((uint8_t *)&RDP_Level, 1);
	 }
	 else {
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	 BL_Print_Mesage("CRC VERIFICATION_Failed  \r\n");
#endif
	   Bootloader_Send_NACK();
	 }  
 }          				
 
 
 

 
 
static uint8_t Bootloader_Jump_Address_Verification(uint32_t Jump_Address){
		uint8_t Address_Verify = ADDRESS_IS_INVALID;
	 
	 if((Jump_Address >= SRAM1_BASE) && (Jump_Address <= stm32f407xx_SRAM1_END)){
				Address_Verify = ADDRESS_IS_VALID;
	 }
	 else if((Jump_Address >= CCMDATARAM_BASE) && (Jump_Address <= stm32f407xx_CCMDATARAM_END)){
				Address_Verify = ADDRESS_IS_VALID;
	 }
	 else if((Jump_Address >= SRAM2_BASE) && (Jump_Address <= stm32f407xx_SRAM2_END)){
				Address_Verify = ADDRESS_IS_VALID;
	 }
	else if((Jump_Address >= FLASH_BASE) && (Jump_Address <= stm32f407xx_FLASH_END)){
				Address_Verify = ADDRESS_IS_VALID;
	 }
	else {
			Address_Verify = ADDRESS_IS_INVALID;
	 }
	 return Address_Verify;
 }
 

 
static void Bootloader_Jump_To_Address(uint8_t *Host_Buffer){
	 uint8_t Host_Packet_Len = 0;
	 uint32_t Host_CRC32_value = 0;
	 uint32_t Host_Jump_Address = 0;
	 uint8_t Address_Verify = ADDRESS_IS_INVALID;
	 
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	 BL_Print_Mesage("GO TO Address Command \r\n\r\n");
#endif
	 /* Extract the CRC32 and packet Length sent by host */
	 Host_Packet_Len = Host_Buffer[0]  + 1;
	 Host_CRC32_value = *((uint32_t *)((Host_Buffer + Host_Packet_Len) - CRC_TYPE_SIZE));
	 
	 
	 /* CRC Verifications */
	 if(CRC_VERIFICATION_PASSED == BL_CRC_Verify(Host_Buffer, Host_Packet_Len - CRC_TYPE_SIZE, Host_CRC32_value)){
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	  BL_Print_Mesage("CRC VERIFICATION_PASSED  \r\n");
#endif
		  Bootloader_Send_ACK(1);
		 /* Extract the Address from the Host Packet */
		 Host_Jump_Address = *((uint32_t *)&Host_Buffer[2]);
		 /* Verify the extracted address to be valid address*/
		 Address_Verify = Bootloader_Jump_Address_Verification(Host_Jump_Address);
		 if(Address_Verify == ADDRESS_IS_VALID){
				/* Report Address Verification Succeeded */
			 Bootloader_Send_Data_To_Host((uint8_t *)&Address_Verify, 1);
			 JumpPtr jump_Address = (JumpPtr)(Host_Jump_Address + 1);
			 jump_Address();
		 }
		 else {
			 /* Report Address Verification Failed  */
				Bootloader_Send_Data_To_Host((uint8_t *)&Address_Verify, 1);
		 }
		
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	 BL_Print_Mesage("CBL_GO_TO_ADDR_CMD \r\n");
#endif
	 }
	 else {
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	 BL_Print_Mesage("CRC VERIFICATION_Failed  \r\n");
#endif
	   Bootloader_Send_NACK();
	 }  
 }       
 
 
 
 
 
 
 
 
 
static uint8_t Perform_Flash_Erase(uint8_t Number_Sector, uint8_t Number_Of_Sectors){
	uint8_t Validity_Status = ERASE_IS_UNSUCCESSFUL;
	FLASH_EraseInitTypeDef pEraseInit;
	uint8_t Remaining_Sector = 0;
	HAL_StatusTypeDef Status = HAL_OK;
	uint32_t Sector_Error = 0;
	 
	 if(Number_Of_Sectors > FLASH_MAX_SECTORS_NUMBER){
		 	 /* Number of sectors is out of range*/
		 Validity_Status = ERASE_IS_UNSUCCESSFUL;
	 }
	 else{
		 /* Number of sectors is into range*/
			if(Number_Sector <= (FLASH_MAX_SECTORS_NUMBER - 1) || (CBL_FLASH_MASS_ERASE == Number_Sector)){
					if(CBL_FLASH_MASS_ERASE == Number_Sector){
						pEraseInit.TypeErase = FLASH_TYPEERASE_MASSERASE; /* MASS ERASE ACTIVITION*/
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	  BL_Print_Mesage("MASS ERASE ACTIVITION \r\n");
#endif							
					}
	 else{ 
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	  BL_Print_Mesage("User Sectors ERASE ACTIVITION \r\n");
#endif
		 Remaining_Sector = FLASH_MAX_SECTORS_NUMBER - Number_Sector;
		 if(Number_Of_Sectors > Remaining_Sector){
		 Number_Of_Sectors = Number_Of_Sectors;
		 }
		 else { /* NOTHING */}
		   pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS; /* ONLY ERASE  SECTORS */
		 	 pEraseInit.NbSectors = Number_Of_Sectors; /*  Number of sectors to be erased.*/
			 pEraseInit.Sector = Number_Sector;    /*Initial FLASH sector to erase when Mass erase is disabled */
	 }
	 pEraseInit.Banks = FLASH_BANK_1; /* BANK_1 */
	 pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3   /* The device voltage range which defines the erase parallelism */;
	 
	 /* Unlock the FLASH control register access */
	 Status = HAL_FLASH_Unlock();
	 
	 /*  Perform a mass erase or erase the specified FLASH memory sectors  */
	 Status = HAL_FLASHEx_Erase(&pEraseInit, &Sector_Error);
	 
	 if(HAL_SUCCESSFUL_ERASE == Sector_Error){
	   Validity_Status = ERASE_IS_SUCCESSFUL;
	 }
	 else{
	   Validity_Status = ERASE_IS_UNSUCCESSFUL;
	 }

   /* lock the FLASH control register access */
	 Status = HAL_FLASH_Lock();
			}
	 else{
			Validity_Status = ERASE_IS_UNSUCCESSFUL;
	 }
}
	 return Validity_Status;
 }
 
 
static void Bootloader_FLASH_ERASE(uint8_t *Host_Buffer){
   uint8_t Host_Packet_Len = 0;
	 uint32_t Host_CRC32_value = 0;
	 uint8_t Check_erase = ERASE_IS_UNSUCCESSFUL;
	 
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	 BL_Print_Mesage("Mass erase oe erase Sector of the user flash \r\n");
#endif
	 /* Extract the CRC32 and packet Length sent by host */
	 Host_Packet_Len = Host_Buffer[0]  + 1;
	 Host_CRC32_value = *((uint32_t *)((Host_Buffer + Host_Packet_Len) - CRC_TYPE_SIZE));
	 
	 /* CRC Verifications */
	 if(CRC_VERIFICATION_PASSED == BL_CRC_Verify(Host_Buffer, Host_Packet_Len - CRC_TYPE_SIZE, Host_CRC32_value)){
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	  BL_Print_Mesage("CRC VERIFICATION_PASSED  \r\n");
#endif
		 Bootloader_Send_ACK(1);
		 Check_erase = Perform_Flash_Erase(Host_Buffer[2], Host_Buffer[3]);
		 if(Check_erase == ERASE_IS_SUCCESSFUL){
			 /* Report Erase Passed  */
				Bootloader_Send_Data_To_Host((uint8_t *)&Check_erase, 1);
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	  BL_Print_Mesage("ERASE_IS_SUCCESSFUL \r\n");
#endif			 
		 }
		 else{
		   /* Report Erase Failed  */
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	  BL_Print_Mesage("ERASE_IS_UNSUCCESSFUL \r\n");
#endif				 
			Bootloader_Send_Data_To_Host((uint8_t *)&Check_erase, 1);
		 }
	 }
	 else {
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	 BL_Print_Mesage("CRC VERIFICATION_Failed  \r\n");
#endif
	   Bootloader_Send_NACK();
	 } 
 }    
 
 
 
static uint8_t Flash_Memory_Write_Payload(uint8_t *Host_Payload, uint16_t Payload_Len, uint32_t Start_address){
   HAL_StatusTypeDef Status = HAL_OK;
	 uint8_t Flash_Write_status = FLASH_WRITE_PAYLOAD_PASSED;
	 uint16_t Payload_Counter = 0;
	 
	 /* Unlock the FLASH control register access */
	 Status = HAL_FLASH_Unlock();
	 
	 if(Status != HAL_OK){
			 Flash_Write_status = FLASH_WRITE_PAYLOAD_FALIED;
	 }
	 else{
	 /*  Perform a mass erase or erase the specified FLASH memory sectors  */
	 for(Payload_Counter = 0; Payload_Counter <Payload_Len; Payload_Counter++){
	    Status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, Start_address + Payload_Counter, Host_Payload[Payload_Counter]);
		 if(Status != HAL_OK){
			 Flash_Write_status = FLASH_WRITE_PAYLOAD_FALIED;
			 break;
		 }
		 else{
		    Flash_Write_status = FLASH_WRITE_PAYLOAD_PASSED;
		 }
	 }
 }
	 if((Status == HAL_OK) && (Flash_Write_status == FLASH_WRITE_PAYLOAD_PASSED)){
   /* lock the FLASH control register access */
	 Status = HAL_FLASH_Lock();
		if(Status != HAL_OK){
			 Flash_Write_status = FLASH_WRITE_PAYLOAD_FALIED;
	  }
	  else{
	   Flash_Write_status = FLASH_WRITE_PAYLOAD_PASSED;
	  }  
	}
	else{
		 Flash_Write_status = FLASH_WRITE_PAYLOAD_FALIED;
	 }
	 return Flash_Write_status;
 }
 
 
static void Bootloader_MEM_WRITE(uint8_t *Host_Buffer){
	 uint8_t Host_Packet_Len = 0;
	 uint32_t Host_CRC32_value = 0;
	 uint8_t Payload_Len = 0;
	 uint32_t Host_address = 0;
	 uint8_t Address_Verify = ADDRESS_IS_INVALID;
	 uint8_t Flash_Write_status = FLASH_WRITE_PAYLOAD_PASSED;

	 
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	  BL_Print_Mesage("Write Data into different memories of the MCU\r\n");
#endif
	 /* Extract the CRC32 and packet Length sent by host */
	 Host_Packet_Len = Host_Buffer[0]  + 1;
	 Host_CRC32_value = *((uint32_t *)((Host_Buffer + Host_Packet_Len) - CRC_TYPE_SIZE));
	 
	 /* CRC Verifications */
	 if(CRC_VERIFICATION_PASSED == BL_CRC_Verify(Host_Buffer, Host_Packet_Len - CRC_TYPE_SIZE, Host_CRC32_value)){
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	  BL_Print_Mesage("CRC VERIFICATION_PASSED  \r\n");
#endif
		 /* Send Acknowledgemnet to the host */
		 Bootloader_Send_ACK(1);
		 /* Extract the start address from the host packet */
		 Host_address = *((uint32_t *)&Host_Buffer[2]);
		 /* Extract the payload length from the host packet */
		 Payload_Len = Host_Buffer[6];
		 
		  /* check for the address is valid or not from the host packet */
		 Address_Verify = Bootloader_Jump_Address_Verification(Host_address); ;
		 /* the address is valid */
		 if(Address_Verify == ADDRESS_IS_VALID){
		    Flash_Write_status = Flash_Memory_Write_Payload((uint8_t *)&Host_Buffer[7], Payload_Len, Host_address);
			 
			 if(FLASH_WRITE_PAYLOAD_PASSED == Flash_Write_status){
				 	/* FLASH_WRITE_PAYLOAD_PASSED */
				 Bootloader_Send_Data_To_Host((uint8_t *)&Flash_Write_status, 1);
			 }
			 else{
				 		/* FLASH_WRITE_PAYLOAD_FAILED */
			    Bootloader_Send_Data_To_Host((uint8_t *)&Flash_Write_status, 1);
			 }
		 }
		 else {
			 /* the address is valid */
	 	     Bootloader_Send_Data_To_Host((uint8_t *)&Address_Verify, 1);
		 }
	 }
	 else {
		 /* CRC VERIFICATION_Failed */
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	 BL_Print_Mesage("CRC VERIFICATION_Failed  \r\n");
#endif
	   Bootloader_Send_NACK();
	 }
 }  
 
 
 
 


static void Bootloader_Enable_RW_PROTECT(uint8_t *Host_Buffer){
	 
 }          		
 
static void Bootloader_MEM_READ(uint8_t *Host_Buffer){
	 
 } 
 
static void Bootloader_READ_SECTOR_Protection_STATUS(uint8_t *Host_Buffer){
	 
 }   
 
static void Bootloader_OTP_READ(uint8_t *Host_Buffer){
	 
 }         
 

 
static uint8_t Chane_ROP_LEVEL(uint32_t ROP_Level){
    HAL_StatusTypeDef HAL_Status = HAL_OK;
		FLASH_OBProgramInitTypeDef *pOBInit;
		uint8_t ROP_Level_Status = ROP_LEVEL_INVALID;
		
	   /* Unlock the FLASH Option Control Registers access */
		HAL_Status = HAL_FLASH_OB_Unlock();
		if(HAL_Status != HAL_OK){
		       ROP_Level_Status = ROP_LEVEL_INVALID;
		}
		else {
			/*!< RDP option byte configuration  */
			pOBInit->OptionType = OPTIONBYTE_RDP;
			/*!< Bank 1   */
			pOBInit->Banks = FLASH_BANK_1; 
			/*!< Set the read protection level.*/
			pOBInit->RDPLevel = ROP_Level;
			/* Program Option bytes */
		  HAL_Status = HAL_FLASHEx_OBProgram(pOBInit);
			if(HAL_Status != HAL_OK){
			  ROP_Level_Status = ROP_LEVEL_INVALID;
			}
			else {
				/* Launch Option bytes loading */
					HAL_Status = HAL_FLASH_OB_Launch();
				if(HAL_Status != HAL_OK){
			  ROP_Level_Status = ROP_LEVEL_INVALID;
			}
				else {
				/* Lock the FLASH Option Control Registers access. */
		     HAL_Status = HAL_FLASH_OB_Lock();
					
					if(HAL_Status != HAL_OK){
			  ROP_Level_Status = ROP_LEVEL_INVALID;
			}
					else {
					ROP_Level_Status = ROP_LEVEL_VALID;
					}
				}
			
			}
		}
		return ROP_Level_Status;
	}
 
 
static void Bootloader_CHANGE_READ_OUT_PROTECTION_LEVEL(uint8_t *Host_Buffer){
   uint8_t Host_Packet_Len = 0;
	 uint32_t Host_CRC32_value = 0;
	 uint8_t ROP_Level_Status = ROP_LEVEL_INVALID;
	 uint8_t Host_ROP_Level = 0; 
	 
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	 BL_Print_Mesage("Change read protection level of the user flash  \r\n");
#endif
	 /* Extract the CRC32 and packet Length sent by host */
	 Host_Packet_Len = Host_Buffer[0]  + 1;
	 Host_CRC32_value = *((uint32_t *)((Host_Buffer + Host_Packet_Len) - CRC_TYPE_SIZE));
	 
	 /* CRC Verifications */
	 if(CRC_VERIFICATION_PASSED == BL_CRC_Verify(Host_Buffer, Host_Packet_Len - CRC_TYPE_SIZE, Host_CRC32_value)){
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	  BL_Print_Mesage("CRC VERIFICATION_PASSED  \r\n");
#endif
		 Bootloader_Send_ACK(1);
		 Host_ROP_Level = Host_Buffer[2];
		 if(OB_RDP_LEVEL_2 == Host_ROP_Level){
		     ROP_Level_Status = ROP_LEVEL_INVALID;
		 }
		 else {
		 ROP_Level_Status = Chane_ROP_LEVEL(Host_ROP_Level);
		 if(ROP_LEVEL_VALID == ROP_Level_Status){
		       /* Request the read out protection level */
		    Bootloader_Send_Data_To_Host((uint8_t *)&ROP_Level_Status, 1);
		 }
		 else {
		 /* Request the read out protection level */
		    Bootloader_Send_Data_To_Host((uint8_t *)&ROP_Level_Status, 1);
		 }
	 }
 }
	 else {
#if(CBL_DEBUG_ENABLE == DEBUG_INFO_ENABLE)
	 BL_Print_Mesage("CRC VERIFICATION_Failed  \r\n");
#endif
	   Bootloader_Send_NACK();
	 }  
 }


/* --------------------- Function Definitions --------------------------*/

BL_StatusTypeDef BL_UART_FETECH_COMMAND(void){
	BL_StatusTypeDef Status = BL_NACK;
	HAL_StatusTypeDef Status_Uart = HAL_ERROR;
	uint8_t Data_Length = 0;
	
	memset(Data_Received_Buffer, VALUE_ZERO, BL_Buffer_RX_LENGTH);
	
	Status_Uart = HAL_UART_Receive(BL_COMMAND_UART, Data_Received_Buffer, 1, HAL_MAX_DELAY);
	if(Status_Uart != HAL_OK){
			Status = BL_NACK;
	}
	else{
	  Data_Length = Data_Received_Buffer[0];
	  Status_Uart = HAL_UART_Receive(BL_COMMAND_UART, &Data_Received_Buffer[1], 1, HAL_MAX_DELAY);
		
		if(Status_Uart != HAL_OK){
			Status = BL_NACK;
	}
	else{
		switch(Data_Received_Buffer[1]){
			case CBL_GET_VER_CMD: 
			Bootloader_GET_Version(Data_Received_Buffer);
			Status = BL_OK;
			break;
			case CBL_GET_HELP_CMD:  
			Bootloader_GET_HELP(Data_Received_Buffer);
			Status = BL_OK;
		  break;	
			case CBL_GET_CID_CMD:    
			Bootloader_GET_Chip_Identification_Number(Data_Received_Buffer);
			Status = BL_OK;
		  break;
			case CBL_RDP_STATUS_CMD:
			Bootloader_Read_Protection_Level(Data_Received_Buffer);
			Status = BL_OK;
		  break;
			case CBL_GO_TO_ADDR_CMD:
			Bootloader_Jump_To_Address(Data_Received_Buffer);
			Status = BL_OK;
		  break;			
		 	case CBL_FLASH_ERASE_CMD:
			Bootloader_FLASH_ERASE(Data_Received_Buffer);
			Status = BL_OK;
      break;		
			case CBL_MEM_WRITE_CMD:
			Bootloader_MEM_WRITE(Data_Received_Buffer);
			Status = BL_OK;
      break;
			case CBL_EN_R_W_PROTECT_CMD:  
      BL_Print_Mesage("CBL_EN_R_W_PROTECT_CMD \r\n");
			Bootloader_Enable_RW_PROTECT(Data_Received_Buffer);
			Status = BL_OK;
		  break;
			case CBL_MEM_READ_CMD:
			BL_Print_Mesage("CBL_MEM_READ_CMD \r\n");
			Bootloader_MEM_READ(Data_Received_Buffer);
			Status = BL_OK;
		  break;	
		  case CBL_READ_SECTOR_STATUS_CMD:
			BL_Print_Mesage("CBL_READ_SECTOR_STATUS_CMD \r\n");
			Bootloader_READ_SECTOR_Protection_STATUS(Data_Received_Buffer);
			Status = BL_OK;
      break;		
      case CBL_OTP_READ_CMD:
      BL_Print_Mesage("CBL_OTP_READ_CMD \r\n");
			Bootloader_OTP_READ(Data_Received_Buffer);
			Status = BL_OK;
		  break;	
      case CBL_CHANGE_ROP_PROTECT_CMD:          		
			Bootloader_Disable_RW_PROTECT(Data_Received_Buffer);
			Status = BL_OK;
	    break;	
			default:
			BL_Print_Mesage("Invalid Command code received from host \r\n");
		  break;
		}
		
	}
}
	
	return Status;
}



void BL_Print_Mesage(char *format, ...){
	HAL_StatusTypeDef Status = HAL_ERROR;;
	char Message[100]={0};
	va_list args;
	/* Enables access to the variable arguments */ 
	va_start(args, format);
	/* Write formatted data from variable argument list to string  */ 
	vsprintf(Message, format, args);
	/* Trnsimt the formatted data through the defined UART  */
	#ifdef BL_ENABLE_UART_DEBUG_MESSAGE
	Status = HAL_UART_Transmit(BL_DEBUG_UART, (uint8_t *)Message, sizeof(Message), HAL_MAX_DELAY);
	/* Performed cleanup foe an app object initialized by a call to va_start */
	#endif
	va_end(args);
}
