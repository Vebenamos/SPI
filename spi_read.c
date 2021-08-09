/* Layer specfication ---------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------
--
-- This layer for W25Q64FV SPI flash
-- 2013 04 22 :
--                    Update.
-- 2013 04 25 :
--                    Update the init function
-------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------*/

#include "memory_flash.h"

#if     KIT_SPI_FLASH_EN
/* Layer specfication ---------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------
--
-- This layer for Hard define
--
   W25Q64 organized into 32768 pages of 256-bytes each. Up to 256 bytes can be programmed at a time.
     Pages can be erased in groups of 16(4kb sector erase)
     SPI clock frequencies of up to 104MHz are supported allowing equivalent clock rates of 208 MHz for
     dual I/O and 416MHz for quad I/O when using the fast read dual/quad I/O and QPI instructions.
     
     Address Presentation
     >>>8M bytes --------------------------- address: 0x000000~0x7fffff
       Block [0~127] : 64Kb
       .............each Block > sectors[0~15] : 4Kb
       .......................................each sector > 4Kb
     
     >>>SFDP Register ---------------------- address: 0x000000~0x0000ff
     >>>Security Register1~3 --------------- address: 0x000000~0x0000ff
     
     Function Presentation
     >>>Power on -> Reset(66h+99h) -> Device initialization-> Standard spi/ Dual spi/quad spi/operations -><-
        enable QPI(38h)/disable QPI(ffh)
            
     Registers
        busy : S0
-------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------*/

#define SPIFLASH_SEC_SIZE 0x1000
#define SPIFLASH_WE      0x06//write enable
#define SPIFLASH_SR      0x50//volatile SR write enable
#define SPIFLASH_WD      0x04//write disable
#define SPIFLASH_RSR1    0x05//read status register 1
#define SPIFLASH_RSR2    0x35//read status register 2
#define SPIFLASH_WSR     0x01//write status register
#define SPIFLASH_PP      0x02//page program
#define SPIFLASH_SE4     0x20//sector erase 4KB
#define SPIFLASH_BE32    0x52//block erase 32KB
#define SPIFLASH_BE64    0xd8//block erase 64KB
#define SPIFLASH_CE      0xc7//0x60 chip erase
#define SPIFLASH_EPS     0x75//erase/program suspend
#define SPIFLASH_EPR     0x7a//erase/program resume
#define SPIFLASH_PD      0xb9//power down
#define SPIFLASH_RD      0x03//read data
#define SPIFLASH_FR      0x0b//fast read
#define SPIFLASH_RPD     0xab//release powerdown /ID
#define SPIFLASH_MID     0x90//manufacture/device ID
#define SPIFLASH_JID     0x9f//JEDEC ID
#define SPIFLASH_RUID    0x4b//read unique ID
#define SPIFLASH_RSFR    0x5a//read SFDP register
#define SPIFLASH_ESR     0x44//erase security registers
#define SPIFLASH_PSR     0x42//program security registers
#define SPIFLASH_RSR     0x48//read security registers
#define SPIFLASH_EQPI    0x38//enable QPI
#define SPIFLASH_ERST    0x66//enable reset
#define SPIFLASH_RST     0x99//reset

/*
** SPI controller register
*/
#define SPI_FLASH_CPHA      (0<<3)//1:Data is sampled on the second clock edge of the SCK
                            //0:Data is sampled on the first clock edge of SCK
#define SPI_FLASH_CPOL      (0<<4)//1:SCK is active low.
                            //0:SCK is active high.
#define SPI_FLASH_MSTR      (1<<5)//0:Slave mode. 1: Master mode.
#define SPI_FLASH_LSBF      (0<<6)//0:transferred MSB first. 1:LSB first.
#define SPI_FLASH_SPIE      (0<<7)//0: interrupts are inhibited. 
                            //1: interrupt is generated each time the SPIF or MODF
#define SPI_FLASH_BITS      (8<<8)//1000:8bits 1001:9 ~~ 1111:15 0000:16


INT16U  spi_flash_id_MID;
/* Layer specfication -------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------
--
-- This layer for spi hardware
--
-----------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------*/

/*
* function 
*/
void fun_waitbusy(void){
    SPI_FLASH_CS_LOW();
    SPI_FLASH_SWAPE(SPIFLASH_RSR1);
    while( SPI_FLASH_SWAPE(SPIFLASH_RSR1)&0x01);
    SPI_FLASH_CS_HIGH();
}
void fun_waitWEL(void){
    SPI_FLASH_CS_HIGH();
    SPI_FLASH_CS_LOW();
    SPI_FLASH_SWAPE(SPIFLASH_RSR1);
    while( SPI_FLASH_SWAPE(SPIFLASH_RSR1)&0x03);
    SPI_FLASH_CS_HIGH();
}
/*
* function 
*/
void fun_flashEraseSector(INT32U sos,INT32U eos){
    INT32U i,addr;
    if(sos>eos){ i=eos;eos=sos;sos=i;  }

    SPI_FLASH_CS_LOW();
    SPI_FLASH_SWAPE(SPIFLASH_SR);
    SPI_FLASH_CS_HIGH();
        
    while(sos<=eos){    
        SPI_FLASH_CS_LOW();
        SPI_FLASH_SWAPE(SPIFLASH_WE);
        SPI_FLASH_CS_HIGH();

        fun_waitbusy();    
        //address
        addr = sos++*SPIFLASH_SEC_SIZE;    
        SPI_FLASH_CS_LOW();
        SPI_FLASH_SWAPE(SPIFLASH_SE4     );
        SPI_FLASH_SWAPE((INT8U)(addr>>16));
        SPI_FLASH_SWAPE((INT8U)(addr>> 8));
        SPI_FLASH_SWAPE((INT8U)(addr>> 0));
        SPI_FLASH_CS_HIGH();
        fun_waitWEL();
    }  
}

/* Layer specfication -------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------
--
-- This layer for flash application
--
-----------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------*/
/*
* function
*/
BOOLEAN spi_flash_ReadData(INT8U* pdest,INT32U addr,INT32U size)
{
    INT32U i;

    fun_waitbusy();    //2017.11.17

    SPI_FLASH_CS_LOW();
    SPI_FLASH_SWAPE(SPIFLASH_FR);
    SPI_FLASH_SWAPE((INT8U)(addr>>16)&0xff);
    SPI_FLASH_SWAPE((INT8U)(addr>> 8)&0xff);
    SPI_FLASH_SWAPE((INT8U)(addr>> 0)&0xff);
    SPI_FLASH_SWAPE(0xff);
    for(i=0;i<size;i++){
        *(pdest+i) = SPI_FLASH_SWAPE(0xff);
    }
    SPI_FLASH_CS_HIGH();
    return TRUE;
}
/*
* function
*/
BOOLEAN spi_flash_WriteData(INT32U dest,INT8U* const psrc,INT32U size)
{
    INT32U cnt;
    INT8U  temp;
    INT8U* p=psrc;

    fun_waitbusy();//2017.11.17
    
    while(size > 0)
    {
        if(0 == dest%SPIFLASH_SEC_SIZE)
            fun_flashEraseSector(dest/SPIFLASH_SEC_SIZE,dest/SPIFLASH_SEC_SIZE);
        SPI_FLASH_CS_LOW();
        SPI_FLASH_SWAPE(SPIFLASH_RSR1);
        temp = SPI_FLASH_SWAPE(0xff);
        SPI_FLASH_CS_HIGH();
        SPI_FLASH_CS_LOW();
        SPI_FLASH_SWAPE(SPIFLASH_SR);
        SPI_FLASH_CS_HIGH();
        SPI_FLASH_CS_LOW();            
        SPI_FLASH_SWAPE(SPIFLASH_WSR);
        SPI_FLASH_SWAPE(0);//chip writeable     
        SPI_FLASH_CS_HIGH();                            
        for(cnt=0;cnt<SPIFLASH_SEC_SIZE && size>0;cnt++,dest++,size--){
            SPI_FLASH_CS_LOW();
            SPI_FLASH_SWAPE(SPIFLASH_WE);
            SPI_FLASH_CS_HIGH();
            SPI_FLASH_CS_LOW();
            SPI_FLASH_SWAPE(SPIFLASH_PP      );
            SPI_FLASH_SWAPE((INT8U)(dest>>16));
            SPI_FLASH_SWAPE((INT8U)(dest>> 8));
            SPI_FLASH_SWAPE((INT8U)(dest>> 0));
            SPI_FLASH_SWAPE(  *(p++)        );
            SPI_FLASH_CS_HIGH();
            fun_waitWEL();
        }
    }

    SPI_FLASH_CS_LOW();
    SPI_FLASH_SWAPE(SPIFLASH_WE);
    SPI_FLASH_CS_HIGH();        

    SPI_FLASH_CS_LOW();
    SPI_FLASH_SWAPE(SPIFLASH_SR);
    SPI_FLASH_CS_HIGH();            

    SPI_FLASH_CS_LOW();            
    SPI_FLASH_SWAPE(SPIFLASH_WSR);
    SPI_FLASH_SWAPE(temp);//    
    SPI_FLASH_CS_HIGH();            
    //
    return TRUE;
}
/*
* function erase data
* return Error code
* size = 256/512/1024/4096
SPIFLASH_SEC_SIZE
*/
BOOLEAN   spi_flash_EraseData        (INT32U addr,INT32U size)
{
    INT32U sos,eos;
    
    if( size == 0) return TRUE;
    
    //1.find the sector num
    sos = addr/SPIFLASH_SEC_SIZE;    
    //2.find the end of sector
    eos = (addr+size-1)/SPIFLASH_SEC_SIZE;
    //erase sector
    fun_flashEraseSector(sos,eos);
    
    return TRUE;
}

/* Layer specfication -------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------
--
-- This layer for initialization
--
-----------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------*/

/*
* function 
*/
INT32U spi_flashReadID(void)
{
    INT32U ID;
    SPI_FLASH_CS_LOW();
    SPI_FLASH_SWAPE(SPIFLASH_MID);
    SPI_FLASH_SWAPE(0x00);SPI_FLASH_SWAPE(0x00);SPI_FLASH_SWAPE(0x00);
    ID =  SPI_FLASH_SWAPE(0xff)<<8;   //manufacturer ID
    ID |= SPI_FLASH_SWAPE(0xff);      //Device ID
    SPI_FLASH_CS_HIGH();
    return ID;
}
void spi_flashReset(void)
{
    SPI_FLASH_CS_LOW();
    SPI_FLASH_SWAPE(SPIFLASH_ERST);
    SPI_FLASH_SWAPE(SPIFLASH_RST);
    SPI_FLASH_CS_HIGH();
}
#endif //#if     KIT_SPI_FLASH_EN

