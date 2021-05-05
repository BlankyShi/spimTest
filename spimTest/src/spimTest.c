/* Read and write SPI Slave EEPROM.
 * Linux instructions:
 *  1. Ensure libft4222.so is in the library search path (e.g. /usr/local/lib)
 *  2. gcc spim.c -lft4222 -Wl,-rpath,/usr/local/lib
 *  3. sudo ./a.out
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "ftd2xx.h"
#include "libft4222.h"
#include <unistd.h>


// SPI Master can assert SS0O in single mode
// SS0O and SS1O in dual mode, and
// SS0O, SS1O, SS2O and SS3O in quad mode.
#define SLAVE_SELECT(x) (1 << (x))
#define   NUM_SLAVE   4

uint8 sendData[1];
uint8 recvData[4];
uint16 sizeTransferred;
uint16 angleData_16[NUM_SLAVE];
uint16 angleData[NUM_SLAVE];

FT_HANDLE            ftHandle1 = (FT_HANDLE)NULL;
FT_HANDLE            ftHandle2 = (FT_HANDLE)NULL;
FT_HANDLE            ftHandle3 = (FT_HANDLE)NULL;
FT_HANDLE            ftHandle4 = (FT_HANDLE)NULL;
FT4222_STATUS        ft4222Status;

void SPI_WriteRead(FT_HANDLE ftHandle,int deviceNumber);
void SPI_init(FT_HANDLE ftHandle);
void SPI_CS_DESELECTALL(void);
void SPI_CS_SELECT(FT_HANDLE ftHandle);

static int testFT4222(void)
{
    FT_STATUS                 ftStatus;
    FT_DEVICE_LIST_INFO_NODE *devInfo = NULL;
    DWORD                     numDevs = 0;
    int                       i; 
    ftStatus = FT_CreateDeviceInfoList(&numDevs);
    if (ftStatus != FT_OK) 
    {
        printf("FT_CreateDeviceInfoList failed (error code %d)\n", 
            (int)ftStatus);
        return 0;
    }
    
    if (numDevs == 0)
    {
        printf("No devices connected.\n");
        return 0;
    }

    /* Allocate storage */
    devInfo = calloc((size_t)numDevs,
                    sizeof(FT_DEVICE_LIST_INFO_NODE));
    if (devInfo == NULL)
    {
        printf("Allocation failure.\n");
        return 0;
    }
    
    /* Populate the list of info nodes */
    ftStatus = FT_GetDeviceInfoList(devInfo, &numDevs);
    printf("numDevs is %d\n",numDevs);
    if (ftStatus != FT_OK)
    {
        printf("FT_GetDeviceInfoList failed (error code %d)\n",
            (int)ftStatus);
        return 0;
    }

    for (i = 0; i < (int)numDevs; i++) 
    {
        
        printf("\nDevice %d is FT4222H\n",i);
        printf("  0x%08x  %s  %s\n", 
                (unsigned int)devInfo[i].ID,
                devInfo[i].SerialNumber,
                devInfo[i].Description);
        //(void)exercise4222(devInfo[i].LocId);
    }
//1
    ftStatus = FT_Open(0,&ftHandle1);
    
    if (ftStatus != FT_OK)
    {
        printf("FT_Open failed (error %d)\n", 
            (int)ftStatus);
        return 0;
    }
//2

    ftStatus = FT_Open(1,&ftHandle2);
    if (ftStatus != FT_OK)
    {
        printf("FT_Open failed (error %d)\n", 
            (int)ftStatus);
        return 0;
    }
//3
    ftStatus = FT_Open(2,&ftHandle3);
    if (ftStatus != FT_OK)
    {
        printf("FT_Open failed (error %d)\n", 
            (int)ftStatus);
        return 0;
    }
//4
    ftStatus = FT_Open(3,&ftHandle4);
    if (ftStatus != FT_OK)
    {
        printf("FT_Open failed (error %d)\n", 
            (int)ftStatus);
        return 0;
    }
    
    //printf("Chip version: %08X, LibFT4222 version: %08X\n",(unsigned int)ft4222Version.chipVersion,(unsigned int)ft4222Version.dllVersion);

    SPI_init(ftHandle1);
    SPI_init(ftHandle2);
    SPI_init(ftHandle3);
    SPI_init(ftHandle4);


//
    
    while(1)
    {
        SPI_WriteRead(ftHandle1,0);
        SPI_WriteRead(ftHandle2,1);
        SPI_WriteRead(ftHandle3,2);
        SPI_WriteRead(ftHandle4,3);
        printf("angleData is \t%d\t%d\t%d\t%d\r\n",angleData[0],angleData[1],angleData[2],angleData[3]);
        sleep(0.01);
    }

    return 0;
}

void SPI_CS_DESELECTALL(void)
{
    ft4222Status = FT4222_SPIMaster_SetCS(ftHandle4, CS_ACTIVE_POSTIVE);
    ft4222Status = FT4222_SPIMaster_SetCS(ftHandle1, CS_ACTIVE_POSTIVE);
    ft4222Status = FT4222_SPIMaster_SetCS(ftHandle2, CS_ACTIVE_POSTIVE);
    ft4222Status = FT4222_SPIMaster_SetCS(ftHandle3, CS_ACTIVE_POSTIVE);
}

void SPI_CS_SELECT(FT_HANDLE ftHandle)
{
    ft4222Status = FT4222_SPIMaster_SetCS(ftHandle, CS_ACTIVE_NEGTIVE);
}
void SPI_init(FT_HANDLE ftHandle)
{
    ft4222Status = FT4222_SPIMaster_Init(
                        ftHandle, 
                        SPI_IO_SINGLE, // 1 channel
                        CLK_DIV_32, // 60 MHz / 32 == 1.875 MHz
                        CLK_IDLE_HIGH, // clock idles at logic 0
                        CLK_TRAILING, // data captured on rising edge
                        SLAVE_SELECT(0)); // Use SS0O for slave-select
    if (FT4222_OK != ft4222Status)
    {
        printf("FT4222_SPIMaster_Init failed (error %d)\n",
            (int)ft4222Status);
        return;
    }

    ft4222Status = FT4222_SPI_SetDrivingStrength(ftHandle,
                                                DS_8MA,
                                                DS_8MA,
                                                DS_8MA);
    if (FT4222_OK != ft4222Status)
    {
        printf("FT4222_SPI_SetDrivingStrength failed (error %d)\n",
            (int)ft4222Status);
        return;
    }

}

void SPI_WriteRead(FT_HANDLE ftHandle,int deviceNumber)
{
    SPI_CS_DESELECTALL();
    SPI_CS_SELECT(ftHandle);
    ft4222Status = FT4222_SPIMaster_SingleWrite(ftHandle, &sendData[0], 1, &sizeTransferred, FALSE);
    if (FT4222_OK != ft4222Status)
    {
        // spi master write failed
        printf("spi master write failed");
        return;
    }

    ft4222Status = FT4222_SPIMaster_SingleRead(ftHandle, &recvData[0], 4, &sizeTransferred, TRUE);
    if (FT4222_OK != ft4222Status)
    {
        // spi master read failed
        printf("spi master read failed\n");
        return;
    }
    angleData_16[deviceNumber] = (uint16)(((uint16)recvData[0] << 8) + recvData[1]);
    angleData[deviceNumber] = angleData_16[deviceNumber] * 360 / 65536;
       
}
int main(void)
{
    sendData[0] = 0x05;
    return testFT4222();
}

