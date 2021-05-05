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

uint8 sendData[2];
uint8 sendData2[2];
uint8 recvData[2];
uint16 sizeTransferred;

int angleData[NUM_SLAVE];
int turnData[NUM_SLAVE];

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
    ftStatus =  FT4222_SetClock(ftHandle1, SYS_CLK_24);
    ftStatus =  FT4222_SetClock(ftHandle2, SYS_CLK_24);
    ftStatus =  FT4222_SetClock(ftHandle3, SYS_CLK_24);
    ftStatus =  FT4222_SetClock(ftHandle4, SYS_CLK_24);

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
    }
//1
    ftStatus = FT_Open(0,&ftHandle1);
    
    if (ftStatus != FT_OK)
    {
        printf("FT_Open failed (error %d)\n", 
            (int)ftStatus);
        return 0;
    }

    ft4222Status = FT4222_SPIMaster_Init(
                        ftHandle1, 
                        SPI_IO_SINGLE, // 1 channel
                        CLK_DIV_32, // 60 MHz / 32 == 1.875 MHz
                        CLK_IDLE_HIGH, // clock idles at logic 0
                        CLK_TRAILING, // data captured on rising edge
                        SLAVE_SELECT(0)); // Use SS0O for slave-select
    if (FT4222_OK != ft4222Status)
    {
        printf("FT4222_SPIMaster_Init failed (error %d)\n",
            (int)ft4222Status);
        return 0;
    }

    ft4222Status = FT4222_SPI_SetDrivingStrength(ftHandle1,
                                                DS_8MA,
                                                DS_8MA,
                                                DS_8MA);
    if (FT4222_OK != ft4222Status)
    {
        printf("FT4222_SPI_SetDrivingStrength failed (error %d)\n",
            (int)ft4222Status);
        return 0;
    }

    while(1)
    {
        SPI_WriteRead(ftHandle1,0);
        SPI_WriteRead(ftHandle2,1);
        SPI_WriteRead(ftHandle3,2);
        SPI_WriteRead(ftHandle4,3);
        //printf("angleData is \t%d\t%d\t%d\t%d\n;turnsData is \t%d\t%d\t%d\t%d\n",angleData[0],angleData[1],angleData[2],angleData[3],turnData[0],turnData[1],turnData[2],turnData[3]);
        sleep(1);
    }

    return 0;
}




void SPI_WriteRead(FT_HANDLE ftHandle,int deviceNumber)
{
    SPI_CS_DESELECTALL();
    SPI_CS_SELECT(ftHandle);
    
    ft4222Status = FT4222_SPIMaster_SingleReadWrite(ftHandle, &recvData[0], &sendData[0], 2, &sizeTransferred, TRUE);
    if((ft4222Status!=FT4222_OK) || (sizeTransferred!=2)){
        // single read write failed
        printf("single read write failed\n");
        return ;
    }
    ft4222Status = FT4222_SPIMaster_SingleReadWrite(ftHandle, &recvData[0], &sendData[0], 2, &sizeTransferred, TRUE);
    if((ft4222Status!=FT4222_OK) || (sizeTransferred!=2)){
        // single read write failed
        return ;
    }
    
    //printf("1.2angleData_16 = %d\r\n", angleData_16);
    angleData[deviceNumber] = (((int)(recvData[0]&0x0f)<<8)+(int)recvData[1]) * 360 / 4096;
    //printf("1.3angleData = %d\r\n", angleData1);
    //printf("recvData is %x, %x, %x, %x;angleData_16 = %d; angleData = %d\n",recvData[0], recvData[1], recvData[2], recvData[3],angleData_16,angleData1);
    ft4222Status = FT4222_SPIMaster_SingleReadWrite(ftHandle, &recvData[0], &sendData2[0], 2, &sizeTransferred, TRUE);
    if((ft4222Status!=FT4222_OK) || (sizeTransferred!=2)){
        // single read write failed
        printf("single read write failed\n");
        return ;
    }
    ft4222Status = FT4222_SPIMaster_SingleReadWrite(ftHandle, &recvData[0], &sendData2[0], 2, &sizeTransferred, TRUE);
    if((ft4222Status!=FT4222_OK) || (sizeTransferred!=2)){
        // single read write failed
        return ;
    }
    //printf("1.2angleData_16 = %d\r\n", angleData_16);
    turnData[deviceNumber] = (((int)(recvData[0]&0x0f)<<8)+(int)recvData[1]) / 8;
}
int main(void)
{
    sendData[0] = 0x20;
    sendData[1] = 0x00;
    sendData2[0] = 0x2c;
    sendData2[1] = 0x00;
    return testFT4222();
}

