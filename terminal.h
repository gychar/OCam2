/*
+-------------------------------- DISCLAIMER ---------------------------------+
|                                                                             |
| This application program is provided to you free of charge as an example.   |
| Despite the considerable efforts of Euresys personnel to create a usable    |
| example, you should not assume that this program is error-free or suitable  |
| for any purpose whatsoever.                                                 |
|                                                                             |
| EURESYS does not give any representation, warranty or undertaking that this |
| program is free of any defect or error or suitable for any purpose. EURESYS |
| shall not be liable, in contract, in torts or otherwise, for any damages,   |
| loss, costs, expenses or other claims for compensation, including those     |
| asserted by third parties, arising out of or in connection with the use of  |
| this program.                                                               |
|                                                                             |
+-----------------------------------------------------------------------------+
*/
/*****************************************************************************
terminal.h - header of CameraLink serial API as per Camera Link Specification
v1.1, 2004 (Appendix B in particular). Euresys libclseremc.so must be present
in system, which is typically installed by MutliCam installer.
*******************************************************************************/

#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#ifdef __GNUC__
#define CLSEREMC_API
#define CLSEREMC_CC
#else

#ifdef CLSEREMC_EXPORTS
#define CLSEREMC_API __declspec(dllexport)
#else
#define CLSEREMC_API __declspec(dllimport)
#endif

#define CLSEREMC_CC __cdecl

#endif 


//////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////

// Baud rates
#define CL_BAUDRATE_9600                        1
#define CL_BAUDRATE_19200                       2
#define CL_BAUDRATE_38400                       4
#define CL_BAUDRATE_57600                       8
#define CL_BAUDRATE_115200                      16
#define CL_BAUDRATE_230400                      32
#define CL_BAUDRATE_460800                      64
#define CL_BAUDRATE_921600                      128

//////////////////////////////////////////////////////////////////////
//  Error Codes
//////////////////////////////////////////////////////////////////////
#define CL_ERR_NO_ERR                           0
#define CL_ERR_BUFFER_TOO_SMALL                 -10001
#define CL_ERR_MANU_DOES_NOT_EXIST              -10002
#define CL_ERR_UNABLE_TO_OPEN_PORT              -10003
#define CL_ERR_PORT_IN_USE                      -10003
#define CL_ERR_TIMEOUT                          -10004
#define CL_ERR_INVALID_INDEX                    -10005
#define CL_ERR_INVALID_REFERENCE                -10006
#define CL_ERR_ERROR_NOT_FOUND                  -10007
#define CL_ERR_BAUD_RATE_NOT_SUPPORTED          -10008
#define CL_ERR_UNABLE_TO_LOAD_DLL               -10098
#define CL_ERR_FUNCTION_NOT_FOUND               -10099


// ***************************************************************************
#ifdef __cplusplus
extern "C" {
#endif
	CLSEREMC_API int CLSEREMC_CC clSerialInit(unsigned long SerialIndex, void** SerialRefPtr);
	CLSEREMC_API int CLSEREMC_CC clSerialWrite(void* SerialRef, char* Buffer, unsigned long* BufferSize, unsigned long SerialTimeout);
	CLSEREMC_API int CLSEREMC_CC clSerialRead(void* SerialRef, char* Buffer, unsigned long* BufferSize, unsigned long SerialTimeout);
	CLSEREMC_API int CLSEREMC_CC clSerialClose(void* SerialRef);

	CLSEREMC_API int CLSEREMC_CC clGetManufacturerInfo(char* ManufacturerName, unsigned int* BufferSize, unsigned int *Version);
	CLSEREMC_API int CLSEREMC_CC clGetNumSerialPorts(unsigned int* NumSerialPorts);
	CLSEREMC_API int CLSEREMC_CC clGetSerialPortIdentifier(unsigned long SerialIndex, char* PortId, unsigned long* BufferSize);
	CLSEREMC_API int CLSEREMC_CC clGetSupportedBaudRates(void *SerialRef, unsigned int* BaudRates);
	CLSEREMC_API int CLSEREMC_CC clSetBaudRate(void* SerialRef, unsigned int BaudRate);
	CLSEREMC_API int CLSEREMC_CC clGetErrorText(int ErrorCode, char *ErrorText, unsigned int *ErrorTextSize);
	CLSEREMC_API int CLSEREMC_CC clGetNumBytesAvail(void *SerialRef, unsigned int *NumBytes);
	CLSEREMC_API int CLSEREMC_CC clFlushInputBuffer(void *SerialRef);
#ifdef __cplusplus
};
#endif

#endif //_TERMINAL_H_
