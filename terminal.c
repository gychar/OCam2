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

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "terminal.h"

#define BOOL int
#define TRUE 1
#define FALSE 0
#define TERMINAL_BUFFER_SIZE 1024
#define TERMINAL_STRING_SIZE 128

int BaudRate2Id (int BaudRate);
void *ReadDataThread (void * SerialRefPtr);

int main(int argc, char* argv[])
{
	char ReadBuffer[TERMINAL_BUFFER_SIZE], ReadData[TERMINAL_BUFFER_SIZE];
	int status = -1, i, SupportedBaudRate, BaudRate, BaudRateId;
	pthread_t ReadDataThreadId;
	unsigned long size;
	unsigned int NumPorts, PortId;
	char PortName[TERMINAL_STRING_SIZE];
	void* SerialRefPtr;
	BOOL inited = FALSE;
	unsigned char c;

	// Enumerate Camera Link Serial Ports
	status = clGetNumSerialPorts (&NumPorts);
	if (status != CL_ERR_NO_ERR)
	{
		printf ("clGetNumSerialPorts error %d\n", status);
		goto Error;
	}
	if (NumPorts == 0)
	{
		printf ("\nSorry, no serial port detected.\n");
		printf ("Check if a GrabLink is present in your system and if the drivers are correctly loaded...\n");
		goto Error;
	}

	printf ("\nDetected ports:\n");
	for (i=0; i<NumPorts; i++)
	{
		size = TERMINAL_STRING_SIZE;
		status = clGetSerialPortIdentifier (i, PortName, &size);
		printf (" - Serial Index %d : %s\n", i, PortName);
	}

	// Camera Link Serial Port Selection
	PortId = NumPorts;
	while (PortId >= NumPorts)
	{
		printf ("\nPlease, select a port in the list above by entering its serial index...\n> ");
		if (scanf("%d", &PortId) == 0 || PortId >= NumPorts)
		{
			while (getchar() != '\n'); // Flush stdin buffer
			printf ("This is not a valid serial index.\n");
		}
	}
	printf ("Port selected : %d\n", PortId);

	// Initialize Camera Link Serial Connetion
	status = clSerialInit (PortId, &SerialRefPtr);
	if (status != CL_ERR_NO_ERR)
	{
		printf ("clSerialInit error %d\n", status);
		goto Error;
	}
	inited = TRUE;

	// Camera Link Serial Port Baudrate Selection
	status = clGetSupportedBaudRates (SerialRefPtr, &SupportedBaudRate);
	if (status != CL_ERR_NO_ERR)
	{
		printf ("clGetSupportedBaudRates error %d\n", status);
		goto Error;
	}
	printf ("Enter a baudrate, supported baudrates are:\n");
	if (SupportedBaudRate&CL_BAUDRATE_9600)
		printf("  - 9600 Bauds\n");
	if (SupportedBaudRate&CL_BAUDRATE_19200)
		printf("  - 19200 Bauds\n");
	if (SupportedBaudRate&CL_BAUDRATE_38400)
		printf("  - 38400 Bauds\n");
	if (SupportedBaudRate&CL_BAUDRATE_57600)
		printf("  - 57600 Bauds\n");
	if (SupportedBaudRate&CL_BAUDRATE_115200)
		printf("  - 115200 Bauds\n");
	if (SupportedBaudRate&CL_BAUDRATE_230400)
		printf("  - 230400 Bauds\n");
	if (SupportedBaudRate&CL_BAUDRATE_460800)
		printf("  - 460800 Bauds\n");
	if (SupportedBaudRate&CL_BAUDRATE_921600)
		printf("  - 921600 Bauds\n");

	printf("\n>");

	while (1)
	{
		if (scanf("%d", &BaudRate) == 0)
		{
			while (getchar() != '\n'); // Flush stdin buffer
			printf ("This baudrate is not supported.\n");
			printf ("Select a baudrate.\n>");
			continue;
		}

		BaudRateId=BaudRate2Id(BaudRate);
		if (BaudRateId!=0)
			break;

		printf ("This baudrate is not supported.\n");
		printf ("Select a baudrate.\n>");
	}
	printf ("Baudrate selected : %d (BaudRateId=%d)\n", BaudRate, BaudRateId);


	// Set Camera Link Serial Port Baudrate 
	status = clSetBaudRate (SerialRefPtr, BaudRateId);
	if (status != CL_ERR_NO_ERR)
	{
		printf ("clSetBaudRate error %d\n", status);
		goto Error;
	}

	while (getchar() != '\n'); // Flush stdin buffer

	// Terminal loop -> process lines and send data to the camera.
	printf ("Type your commands\n>");
	pthread_create (&ReadDataThreadId, NULL, ReadDataThread, SerialRefPtr);
	while (1)
	{
		c=(unsigned char)getchar();
		size = 1;
		if (c == '\n')
		{
			char cr = '\r';
			status = clSerialWrite(SerialRefPtr, &cr, &size, 1000);
			if (status != CL_ERR_NO_ERR)
			{
				printf("clSerialWrite error %d\n", status);
				goto Error;
			}
		}
		status = clSerialWrite (SerialRefPtr, &c, &size, 1000);
		if (status != CL_ERR_NO_ERR)
		{
			printf ("clSerialWrite error %d\n", status);
			goto Error;
		}
		sched_yield(); // Force task switching
	}

Error:
	// Process error condition
	if (inited == TRUE)
		clSerialClose (SerialRefPtr);

	return 0;
}

// Read Data Thread
//      This thread will read data (if available) from the 
//      Camera Link Serial port and display it in the console.
void *ReadDataThread (void * SerialRefPtr)
{
	int status;
	unsigned long size;
	char ReadBuffer[TERMINAL_BUFFER_SIZE];
	memset (ReadBuffer, 0, TERMINAL_BUFFER_SIZE*sizeof(char));

	while (1)
	{
		// Read Data 
		size = TERMINAL_BUFFER_SIZE-1;
		status = clSerialRead (SerialRefPtr, ReadBuffer, &size, 1000);
		if (status != CL_ERR_NO_ERR)
		{
			printf ("clSerialRead error %d\n", status);
			return;
		}
		char *current_pos = strchr(ReadBuffer ? '\r');
		while (current_pos)
		{
			*current_pos = '\n';
			current_pos = strchr(current_pos, '\r');
		}

		if (size>0)
		{
			ReadBuffer[size] = 0; 		// final char if the buffer is full            
			printf ("%s",ReadBuffer);
			fflush(stdout);
			memset (ReadBuffer, 0, TERMINAL_BUFFER_SIZE*sizeof(char));
		}

		usleep (5000);
	}
}

int BaudRate2Id (int BaudRate)
{
	switch(BaudRate)
	{
	case 9600:
		return CL_BAUDRATE_9600;
	case 19200:
		return CL_BAUDRATE_19200;
	case 38400:
		return CL_BAUDRATE_38400;
	case 57600:
		return CL_BAUDRATE_57600;
	case 115200:
		return CL_BAUDRATE_115200;
	case 230400:
		return CL_BAUDRATE_230400;
	case 460800:
		return CL_BAUDRATE_460800;
	case 921600:
		return CL_BAUDRATE_921600;
	default:
		return 0;
	}
}

