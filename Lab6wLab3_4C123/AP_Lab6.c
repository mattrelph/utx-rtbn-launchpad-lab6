// AP_Lab6.c
// Runs on either MSP432 or TM4C123
// see GPIO.c file for hardware connections 

// Daniel Valvano and Jonathan Valvano
// November 20, 2016
// CC2650 booster or CC2650 LaunchPad, CC2650 needs to be running SimpleNP 2.2 (POWERSAVE)

#include <stdint.h>
#include "../inc/UART0.h"
#include "../inc/UART1.h"
#include "../inc/AP.h"
#include "AP_Lab6.h"
//**debug macros**APDEBUG defined in AP.h********
#ifdef APDEBUG
#define OutString(STRING) UART0_OutString(STRING)
#define OutUHex(NUM) UART0_OutUHex(NUM)
#define OutUHex2(NUM) UART0_OutUHex2(NUM)
#define OutChar(N) UART0_OutChar(N)
#else
#define OutString(STRING)
#define OutUHex(NUM)
#define OutUHex2(NUM)
#define OutChar(N)
#endif

//****links into AP.c**************
extern const uint32_t RECVSIZE;
extern uint8_t RecvBuf[];
typedef struct characteristics{
  uint16_t theHandle;          // each object has an ID
  uint16_t size;               // number of bytes in user data (1,2,4,8)
  uint8_t *pt;                 // pointer to user data, stored little endian
  void (*callBackRead)(void);  // action if SNP Characteristic Read Indication
  void (*callBackWrite)(void); // action if SNP Characteristic Write Indication
}characteristic_t;
extern const uint32_t MAXCHARACTERISTICS;
extern uint32_t CharacteristicCount;
extern characteristic_t CharacteristicList[];
typedef struct NotifyCharacteristics{
  uint16_t uuid;               // user defined 
  uint16_t theHandle;          // each object has an ID (used to notify)
  uint16_t CCCDhandle;         // generated/assigned by SNP
  uint16_t CCCDvalue;          // sent by phone to this object
  uint16_t size;               // number of bytes in user data (1,2,4,8)
  uint8_t *pt;                 // pointer to user data array, stored little endian
  void (*callBackCCCD)(void);  // action if SNP CCCD Updated Indication
}NotifyCharacteristic_t;
extern const uint32_t NOTIFYMAXCHARACTERISTICS;
extern uint32_t NotifyCharacteristicCount;
extern NotifyCharacteristic_t NotifyCharacteristicList[];
//**************Lab 6 routines*******************
// **********SetFCS**************
// helper function, add check byte to message
// assumes every byte in the message has been set except the FCS
// used the length field, assumes less than 256 bytes
// FCS = 8-bit EOR(all bytes except SOF and the FCS itself)
// Inputs: pointer to message
//         stores the FCS into message itself
// Outputs: none
void SetFCS(uint8_t *msg){
//****You implement this function as part of Lab 6*****
	int8_t fcsBit = 0;
	int16_t length = msg[1] + (msg[2]<<8);	//LSB + MSB of Length
	//msg Format = SOF [0], LENGTH[1-2], CMD0[3]. CMD[4], DATA[5-n], FCS[n+1]

	for (int i=1;i<5+length; i++)
	{
		fcsBit = fcsBit ^ msg[i];		//XOR of all Bytes between Length and FCS
	}
	msg[length + 5 ] = fcsBit;		//Add the FCS bit to the end of the message
  
}
//*************BuildGetStatusMsg**************
// Create a Get Status message, used in Lab 6
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will Get Status
void BuildGetStatusMsg(uint8_t *msg){
// hint: see NPI_GetStatus in AP.c
//****You implement this function as part of Lab 6*****
	//This message is always the same 
	msg[0] = SOF;			//SOF (Start of Frame)
	//Length of this data payload will always be 0 based on the given paramenters
	msg[1] = 0x00;		//LSB Length 1 (2 bytes) of data payload
	msg[2] = 0x00;		//MSB Length 2 < 4096
	msg[3] = 0x55;		//CMD0 (Async)
	msg[4] = 0x06;		//CMD1	(Get Status)
	msg[5] = 0x53;		//FCS (Frame Check Sequence)
}
//*************Lab6_GetStatus**************
// Get status of connection, used in Lab 6
// Input:  none
// Output: status 0xAABBCCDD
// AA is GAPRole Status
// BB is Advertising Status
// CC is ATT Status
// DD is ATT method in progress
uint32_t Lab6_GetStatus(void){volatile int r; uint8_t sendMsg[8];
  OutString("\n\rGet Status");
  BuildGetStatusMsg(sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return (RecvBuf[4]<<24)+(RecvBuf[5]<<16)+(RecvBuf[6]<<8)+(RecvBuf[7]);
}

//*************BuildGetVersionMsg**************
// Create a Get Version message, used in Lab 6
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will Get Status
void BuildGetVersionMsg(uint8_t *msg){
// hint: see NPI_GetVersion in AP.c
//****You implement this function as part of Lab 6*****
	msg[0] = SOF;			//SOF (Start of Frame)
	//Length of this data payload will always be 0 based on the given paramenters	
	msg[1] = 0x00;		//LSB Length 1 (2 bytes) of data payload
	msg[2] = 0x00;		//MSB Length 2 < 4096
	msg[3] = 0x35;		//CMD0	(Sync Req)
	msg[4] = 0x03;		//CMD1	(Get Revision)
	msg[5] = 0x36;		//FCS (Frame Check Sequence)  
  
}
//*************Lab6_GetVersion**************
// Get version of the SNP application running on the CC2650, used in Lab 6
// Input:  none
// Output: version
uint32_t Lab6_GetVersion(void){volatile int r;uint8_t sendMsg[8];
  OutString("\n\rGet Version");
  BuildGetVersionMsg(sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE); 
  return (RecvBuf[5]<<8)+(RecvBuf[6]);
}

//*************BuildAddServiceMsg**************
// Create an Add service message, used in Lab 6
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        pointer to empty buffer of at least 9 bytes
// Output none
// build the necessary NPI message that will add a service
void BuildAddServiceMsg(uint16_t uuid, uint8_t *msg){
//****You implement this function as part of Lab 6*****
{

	msg[0] = SOF;			//SOF (Start of Frame)
	//Length of this data payload will always be 3 based on the given paramenters	
	msg[1] = 0x03;		//LSB Length 1 (2 bytes) of data payload
	msg[2] = 0x00;		//MSB Length 2 < 4096
	
	msg[3] = 0x35;		//CMD0	(Sync Req)
	msg[4] = 0x81;		//CMD1	(Add Service)
	
	msg[5] = 0x01;		//Primary Service
													//UUID is Least Significant Byte First
	msg[6] = (uuid)% 256;		//Lower byte of 2 byte UUID
	msg[7] = (uuid >> 8);		//Upper byte of 2 byte UUID
	
	
	SetFCS(msg);		//Add FCS (Frame Check Sequence) 

};   
  
  
}
//*************Lab6_AddService**************
// Add a service, used in Lab 6
// Inputs uuid is 0xFFF0, 0xFFF1, ...
// Output APOK if successful,
//        APFAIL if SNP failure
int Lab6_AddService(uint16_t uuid){ int r; uint8_t sendMsg[12];
  OutString("\n\rAdd service");
  BuildAddServiceMsg(uuid,sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);  
  return r;
}
//*************AP_BuildRegisterServiceMsg**************
// Create a register service message, used in Lab 6
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will register a service
void BuildRegisterServiceMsg(uint8_t *msg){
//****You implement this function as part of Lab 6*****
	msg[0] = SOF;			//SOF (Start of Frame)
	//Length of this data payload will always be 0 based on the given paramenters	
	msg[1] = 0x00;		//LSB Length 1 (2 bytes) of data payload
	msg[2] = 0x00;		//MSB Length 2 < 4096
	
	msg[3] = 0x35;		//CMD0	(Sync Req)
	msg[4] = 0x84;		//CMD1	(Register Service)
	msg[5] = 0xB1;		//FCS (Frame Check Sequence) 
  
}
//*************Lab6_RegisterService**************
// Register a service, used in Lab 6
// Inputs none
// Output APOK if successful,
//        APFAIL if SNP failure
int Lab6_RegisterService(void){ int r; uint8_t sendMsg[8];
  OutString("\n\rRegister service");
  BuildRegisterServiceMsg(sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return r;
}

//*************BuildAddCharValueMsg**************
// Create a Add Characteristic Value Declaration message, used in Lab 6
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        permission is GATT Permission, 0=none,1=read,2=write, 3=Read+write 
//        properties is GATT Properties, 2=read,8=write,0x0A=read+write, 0x10=notify
//        pointer to empty buffer of at least 14 bytes
// Output none
// build the necessary NPI message that will add a characteristic value
void BuildAddCharValueMsg(uint16_t uuid,  
  uint8_t permission, uint8_t properties, uint8_t *msg){
// set RFU to 0 and
// set the maximum length of the attribute value=512
// for a hint see NPI_AddCharValue in AP.c
// for a hint see first half of AP_AddCharacteristic and first half of AP_AddNotifyCharacteristic
//****You implement this function as part of Lab 6*****

	
	msg[0] = SOF;			//SOF (Start of Frame)
	
	//Length of this data payload will always be 8 based on the given paramenters
		
	msg[1] = 0x08;		//LSB Length 1 (2 bytes) of data payload
	msg[2] = 0x00;		//MSB Length 2 < 4096
	
	msg[3] = 0x35;		//CMD0	(Sync Req)
	msg[4] = 0x82;		//CMD1	(Add Characteristic Value Declaration)
	
	msg[5] = permission;		// 1 byte GATT Permission (0=none,1=read,2=write, 3=Read+write)
	
	msg[6] = properties;		// LSB 2 bytes GATT Properties (2=read,8=write,0x0A=read+write,0x10=notify)
	msg[7] = 0x00;		// MSB GATT Properties
	
	msg[8] = 0x00;		// 1 byte RFU
	
	msg[9] = 0x00;		// LSB 2 bytes Maximum length of the attribute value = 512 = 0x0200
	msg[10] = 0x02;		// MSB Maximum length of the attribute
	
														//UUID is Least Significant Byte First
	msg[11] = (uuid)% 256;		//Lower byte of 2 byte UUID
	msg[12] = (uuid >> 8);		//Upper byte of 2 byte UUID
	
	SetFCS(msg);		//Add FCS (Frame Check Sequence)
    
}

//*************BuildAddCharDescriptorMsg**************
// Create a Add Characteristic Descriptor Declaration message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 20 bytes
//        pointer to empty buffer of at least 32 bytes
// Output none
// build the necessary NPI message that will add a Descriptor Declaration
void BuildAddCharDescriptorMsg(char name[], uint8_t *msg){
// set length and maxlength to the string length
// set the permissions on the string to read
// for a hint see NPI_AddCharDescriptor in AP.c
// for a hint see second half of AP_AddCharacteristic
//****You implement this function as part of Lab 6*****
  
	
	msg[0] = SOF;			//SOF (Start of Frame)
	
	//Length of this data payload will always be vary based on the length of the char array
	
	//Let's determine length of the char array - Less than 21 
	uint8_t arrayLength = 0;
	while (name[arrayLength] != 0)
	{
		++arrayLength;
	}
	arrayLength++;
		
	msg[1] = 6+arrayLength;		//LSB Length 1 (2 bytes) of data payload  = 6+ Length of Char Array
	msg[2] = 0x00;		//MSB Length 2 < 4096
	
	msg[3] = 0x35;		//CMD0	(Sync Req)
	msg[4] = 0x83;		//CMD1	(Add Characteristic Descriptor Declaration)
	

	msg[5] = 0x80;		//Header - Type = User Description String
	msg[6] = 0x01;		// Permissions (1 byte) - GATT Read Permissions
	
	msg[7] = arrayLength;		// LSB Max Length (2 bytes) = 20 = 0x14
	msg[8] = 0x00;		// MSB Max Length (2 bytes)
	
	msg[9] = arrayLength;		// LSB Initial Length (2 bytes) < 20
	msg[10] = 0x00;		// MSB Initial Length (2 bytes)
	
	//To do: Check if array is actually less than or equal to max length
	for(int i =0; i<arrayLength; i++)
	{
		msg[11+i] = name[i];		// Insert Chars from Array
	}

	SetFCS(msg);		//Add FCS (Frame Check Sequence)  
  
}

//*************Lab6_AddCharacteristic**************
// Add a read, write, or read/write characteristic, used in Lab 6
//        for notify properties, call AP_AddNotifyCharacteristic 
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        thesize is the number of bytes in the user data 1,2,4, or 8 
//        pt is a pointer to the user data, stored little endian
//        permission is GATT Permission, 0=none,1=read,2=write, 3=Read+write 
//        properties is GATT Properties, 2=read,8=write,0x0A=read+write
//        name is a null-terminated string, maximum length of name is 20 bytes
//        (*ReadFunc) called before it responses with data from internal structure
//        (*WriteFunc) called after it accepts data into internal structure
// Output APOK if successful,
//        APFAIL if name is empty, more than 8 characteristics, or if SNP failure
int Lab6_AddCharacteristic(uint16_t uuid, uint16_t thesize, void *pt, uint8_t permission,
  uint8_t properties, char name[], void(*ReadFunc)(void), void(*WriteFunc)(void)){
  int r; uint16_t handle; 
  uint8_t sendMsg[32];  
  if(thesize>8) return APFAIL;
  if(name[0]==0) return APFAIL;       // empty name
  if(CharacteristicCount>=MAXCHARACTERISTICS) return APFAIL; // error
  BuildAddCharValueMsg(uuid,permission,properties,sendMsg);
  OutString("\n\rAdd CharValue");
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  handle = (RecvBuf[7]<<8)+RecvBuf[6]; // handle for this characteristic
  OutString("\n\rAdd CharDescriptor");
  BuildAddCharDescriptorMsg(name,sendMsg);
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  CharacteristicList[CharacteristicCount].theHandle = handle;
  CharacteristicList[CharacteristicCount].size = thesize;
  CharacteristicList[CharacteristicCount].pt = (uint8_t *) pt;
  CharacteristicList[CharacteristicCount].callBackRead = ReadFunc;
  CharacteristicList[CharacteristicCount].callBackWrite = WriteFunc;
  CharacteristicCount++;
  return APOK; // OK
} 
  

//*************BuildAddNotifyCharDescriptorMsg**************
// Create a Add Notify Characteristic Descriptor Declaration message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 20 bytes
//        pointer to empty buffer of at least bytes
// Output none
// build the necessary NPI message that will add a Descriptor Declaration
void BuildAddNotifyCharDescriptorMsg(char name[], uint8_t *msg){
// set length and maxlength to the string length
// set the permissions on the string to read
// set User Description String
// set CCCD parameters read+write
// for a hint see NPI_AddCharDescriptor4 in VerySimpleApplicationProcessor.c
// for a hint see second half of AP_AddNotifyCharacteristic
//****You implement this function as part of Lab 6*****
 	
	msg[0] = SOF;			//SOF (Start of Frame)
	
	//Length of this data payload will always be vary based on the length of the char array
	
	//Let's determine length of the char array - Less than 21 
	uint8_t arrayLength = 0;
	while (name[arrayLength] != 0)
	{
		++arrayLength;
	}
	arrayLength++;
		
	msg[1] = 7+arrayLength;		//LSB Length 1 (2 bytes) of data payload  = 7+ Length of Char Array
	msg[2] = 0x00;		//MSB Length 2 < 4096
	
	msg[3] = 0x35;		//CMD0	(Sync Req)
	msg[4] = 0x83;		//CMD1	(Add Characteristic Descriptor Declaration)
	

	msg[5] = 0x84;		//Header - Type = User Description String + CCCD
	msg[6] = 0x03;		// Permissions (1 byte) - GATT Read+Write Permissions
	msg[7] = 0x01;		// ??
	
	msg[8] = arrayLength;		// LSB Max Length (2 bytes) = 20 = 0x14
	msg[9] = 0x00;		// MSB Max Length (2 bytes)
	
	msg[10] = arrayLength;		// LSB Initial Length (2 bytes) < 20
	msg[11] = 0x00;		// MSB Initial Length (2 bytes)
	
	//To do: Check if array is actually less than or equal to max length
	for(int i =0; i<arrayLength; i++)
	{
		msg[12+i] = name[i];		// Insert Chars from Array
	}

	SetFCS(msg);		//Add FCS (Frame Check Sequence)  
   
  
}
  
//*************Lab6_AddNotifyCharacteristic**************
// Add a notify characteristic, used in Lab 6
//        for read, write, or read/write characteristic, call AP_AddCharacteristic 
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        thesize is the number of bytes in the user data 1,2,4, or 8 
//        pt is a pointer to the user data, stored little endian
//        name is a null-terminated string, maximum length of name is 20 bytes
//        (*CCCDfunc) called after it accepts , changing CCCDvalue
// Output APOK if successful,
//        APFAIL if name is empty, more than 4 notify characteristics, or if SNP failure
int Lab6_AddNotifyCharacteristic(uint16_t uuid, uint16_t thesize, void *pt,   
  char name[], void(*CCCDfunc)(void)){
  int r; uint16_t handle; 
  uint8_t sendMsg[36];  
  if(thesize>8) return APFAIL;
  if(NotifyCharacteristicCount>=NOTIFYMAXCHARACTERISTICS) return APFAIL; // error
  BuildAddCharValueMsg(uuid,0,0x10,sendMsg);
  OutString("\n\rAdd Notify CharValue");
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  handle = (RecvBuf[7]<<8)+RecvBuf[6]; // handle for this characteristic
  OutString("\n\rAdd CharDescriptor");
  BuildAddNotifyCharDescriptorMsg(name,sendMsg);
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  NotifyCharacteristicList[NotifyCharacteristicCount].uuid = uuid;
  NotifyCharacteristicList[NotifyCharacteristicCount].theHandle = handle;
  NotifyCharacteristicList[NotifyCharacteristicCount].CCCDhandle = (RecvBuf[8]<<8)+RecvBuf[7]; // handle for this CCCD
  NotifyCharacteristicList[NotifyCharacteristicCount].CCCDvalue = 0; // notify initially off
  NotifyCharacteristicList[NotifyCharacteristicCount].size = thesize;
  NotifyCharacteristicList[NotifyCharacteristicCount].pt = (uint8_t *) pt;
  NotifyCharacteristicList[NotifyCharacteristicCount].callBackCCCD = CCCDfunc;
  NotifyCharacteristicCount++;
  return APOK; // OK
}

//*************BuildSetDeviceNameMsg**************
// Create a Set GATT Parameter message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 24 bytes
//        pointer to empty buffer of at least 36 bytes
// Output none
// build the necessary NPI message to set Device name
void BuildSetDeviceNameMsg(char name[], uint8_t *msg){
// for a hint see NPI_GATTSetDeviceNameMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_GATTSetDeviceName in AP.c
//****You implement this function as part of Lab 6*****
	
	msg[0] = SOF;			//SOF (Start of Frame)
	
	//Length of this data payload will always be vary based on the length of the char array
	
	//Let's determine length of the char array - Less than 25 
	uint8_t arrayLength = 0;
	while (name[arrayLength] != 0)
	{
		++arrayLength;
	}
	//arrayLength++; Not adding the 0x00 character to end
		
	msg[1] = 3+arrayLength;		//LSB Length 1 (2 bytes) of data payload  = 6+ Length of Char Array
	msg[2] = 0x00;		//MSB Length 2 < 4096
	
	msg[3] = 0x35;		//CMD0	(Sync Req)
	msg[4] = 0x8C;		//CMD1	(SNP Set GATT Parameter (0x8C))
	
	msg[5] = 0x01;		//Service ID - Generic Access Service
	
	msg[6] = 0x00;		// LSB Parameter ID (2 bytes) - Device Name
	msg[7] = 0x00;		// MSB Parameter ID (2 bytes)	 - Device Name
	
	//To do: Check if array is actually less than or equal to max length
	for(int i =0; i<arrayLength; i++)
	{
		msg[8+i] = name[i];		// Insert Chars from Array
	}

	SetFCS(msg);		//Add FCS (Frame Check Sequence)  
    
  
}
//*************BuildSetAdvertisementData1Msg**************
// Create a Set Advertisement Data message, used in Lab 6
// Inputs pointer to empty buffer of at least 16 bytes
// Output none
// build the necessary NPI message for Non-connectable Advertisement Data
void BuildSetAdvertisementData1Msg(uint8_t *msg){
// for a hint see NPI_SetAdvertisementMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_SetAdvertisement1 in AP.c
// Non-connectable Advertisement Data
// GAP_ADTYPE_FLAGS,DISCOVERABLE | no BREDR  
// Texas Instruments Company ID 0x000D
// TI_ST_DEVICE_ID = 3
// TI_ST_KEY_DATA_ID
// Key state=0
//****You implement this function as part of Lab 6*****
  
	msg[0] = SOF;			//SOF (Start of Frame)
	//Length of this data payload will always be 11 based on the given paramenters	
	msg[1] = 0x0B;		//LSB Length 1 (2 bytes) of data payload
	msg[2] = 0x00;		//MSB Length 2 < 4096
	
	msg[3] = 0x55;		//CMD0	(Async)
	msg[4] = 0x43;		//CMD1	(SNP Set Advertisement Data)
	
	msg[5] = 0x01;		//Not connected Advertisement Data
	
	msg[6] = 0x02;		//GAP_ADTYPE_FLAGS,DISCOVERABLE | no BREDR	
	msg[7] = 0x01;		//GAP_ADTYPE_FLAGS,DISCOVERABLE | no BREDR	
	msg[8] = 0x06;		//GAP_ADTYPE_FLAGS,DISCOVERABLE | no BREDR		
	
	msg[9] = 0x06;      // length, manufacturer specific
	msg[10] = 0xFF,      // length, manufacturer specific
	msg[11] = 0x0D;     // Texas Instruments Company ID
	msg[12] = 0x00;     // Texas Instruments Company ID
	msg[13] = 0x03;           // TI_ST_DEVICE_ID
	msg[14] = 0x00;           // TI_ST_KEY_DATA_ID
	msg[15] = 0x00;           // Key state	
	
	SetFCS(msg);		//Add FCS (Frame Check Sequence) 
 
}

//*************BuildSetAdvertisementDataMsg**************
// Create a Set Advertisement Data message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 24 bytes
//        pointer to empty buffer of at least 36 bytes
// Output none
// build the necessary NPI message for Scan Response Data
void BuildSetAdvertisementDataMsg(char name[], uint8_t *msg){
// for a hint see NPI_SetAdvertisementDataMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_SetAdvertisementData in AP.c
//****You implement this function as part of Lab 6*****
	
	msg[0] = SOF;			//SOF (Start of Frame)
	//Length of this data payload will depend on the glength of the name paramenter
	//Let's determine length of the char array - Less than 25 
	uint8_t arrayLength = 0;
	while (name[arrayLength] != 0)
	{
		++arrayLength;
	}
	arrayLength++; //Leave off 0x00
	
	msg[1] = arrayLength+11;		//LSB Length 1 (2 bytes) of data payload < 25 + 17 = < 42
	msg[2] = 0x00;		//MSB Length 2 < 4096
	
	msg[3] = 0x55;		//CMD0	(Async)
	msg[4] = 0x43;		//CMD1	(SNP Set Advertisement Data)
	
	msg[5] = 0x00;		//Scan Response Data
	
	msg[6] = arrayLength;		//Name Length
	msg[7] = 0x09;		//type=LOCAL_NAME_COMPLETE
	
	//To do: Check if array is actually less than or equal to max length
	uint8_t current = 0;
	while (current<arrayLength)
	{
		msg[8+current] = name[current];		// Insert Chars from Array
		++current;
	}
	
	// connection interval range
	msg[arrayLength+7] = 0x05;		// length of this data
	msg[arrayLength+8] = 0x12;		// GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE
	
	msg[arrayLength+9] = 0x50;		//LSB DEFAULT_DESIRED_MIN_CONN_INTERVAL
	msg[arrayLength+10] = 0x00;		//MSB DEFAULT_DESIRED_MIN_CONN_INTERVAL
	
	msg[arrayLength+11] = 0x20;		//LSB DEFAULT_DESIRED_MAX_CONN_INTERVAL
	msg[arrayLength+12] = 0x03;		//MSB DEFAULT_DESIRED_MAX_CONN_INTERVAL	
	
	// Tx power level
	msg[arrayLength+13] = 0x02;           // length of this data
  msg[arrayLength+14] = 0x0A;           // GAP_ADTYPE_POWER_LEVEL
  msg[arrayLength+15] = 0x00;           // 0dBm
	
	SetFCS(msg);		//Add FCS (Frame Check Sequence) 
  
  
}
//*************BuildStartAdvertisementMsg**************
// Create a Start Advertisement Data message, used in Lab 6
// Inputs advertising interval
//        pointer to empty buffer of at least 20 bytes
// Output none
// build the necessary NPI message to start advertisement
void BuildStartAdvertisementMsg(uint16_t interval, uint8_t *msg){
// for a hint see NPI_StartAdvertisementMsg in VerySimpleApplicationProcessor.c
// for a hint see NPI_StartAdvertisement in AP.c
//****You implement this function as part of Lab 6*****
	

	
	msg[0] = SOF;			//SOF (Start of Frame)
	//Length of this data payload will always be 14 based on the given paramenters	
	msg[1] = 0x0E;		//LSB Length 1 (2 bytes) of data payload
	msg[2] = 0x00;		//MSB Length 2 < 4096
	
	msg[3] = 0x55;		//CMD0	(Async)
	msg[4] = 0x42;		//CMD1	(SNP Start Advertisement)
	
	msg[5] = 0x00;		//Advertisement Type: (1 byte) - Connectable Undirected Advertisements
	
	msg[6] = 0x00;		//Timeout (2 bytes) - Advertise infinitely
	msg[7] = 0x00;		//Timeout (2 bytes) - How long to advertise for (in ms)
	
	msg[8] = (interval)% 256;;		//LSB Interval (2 bytes) - Advertising Interval (n * 0.625 ms)
	msg[9] = (interval >> 8);    //MSB Interval (2 bytes) - Advertising Interval (n * 0.625 ms)

	msg[10] = 0x00,      // Filter Policy RFU
	msg[11] = 0x00;     // Initiator Address Type RFU
	
	msg[12] = 0x00;     //LSB[0] Initiator Address (6 bytes)
	msg[13] = 0x01;     //SB[1] Initiator Address (6 bytes)
	msg[14] = 0x00;     //SB[2] Initiator Address (6 bytes)
	msg[15] = 0x00;     //SB[3] Initiator Address (6 bytes)
	msg[16] = 0x00;     //SB[4] Initiator Address (6 bytes)
	msg[17] = 0xC5;     //MSB[5] Initiator Address (6 bytes)
	
	msg[18] = 0x02;     // Advertising will restart with connectable advertising when a connection is terminated
	
	SetFCS(msg);		//Add FCS (Frame Check Sequence) 
  
}

//*************Lab6_StartAdvertisement**************
// Start advertisement, used in Lab 6
// Input:  none
// Output: APOK if successful,
//         APFAIL if notification not configured, or if SNP failure
int Lab6_StartAdvertisement(void){volatile int r; uint8_t sendMsg[40];
  OutString("\n\rSet Device name");
  BuildSetDeviceNameMsg("Shape the World",sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rSetAdvertisement1");
  BuildSetAdvertisementData1Msg(sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rSetAdvertisement Data");
  BuildSetAdvertisementDataMsg("Shape the World",sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rStartAdvertisement");
  BuildStartAdvertisementMsg(100,sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return r;
}

