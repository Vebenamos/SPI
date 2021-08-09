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

#include "spi_port.h"


/* Macro defines for SSP SR register */
#define SSP_SR_TFE      ((uint32_t)(1<<0)) /** SSP status TX FIFO Empty bit */
#define SSP_SR_TNF      ((uint32_t)(1<<1)) /** SSP status TX FIFO not full bit */
#define SSP_SR_RNE      ((uint32_t)(1<<2)) /** SSP status RX FIFO not empty bit */
#define SSP_SR_RFF      ((uint32_t)(1<<3)) /** SSP status RX FIFO full bit */
#define SSP_SR_BSY      ((uint32_t)(1<<4)) /** SSP status SSP Busy bit */
#define SSP_SR_BITMASK    ((uint32_t)(0x1F)) /** SSP SR bit mask */

/**
  * @brief  Initializes the SSP0.
  *
  * @param  None
  * @retval None 
  */
void SPI_Init (void) 
{
//        INT32U Div_Freq;
        INT32U frequence = 12000000;

//LPC175X-6X
#if 1
    /* Enable SSPI0 block */
    LPC_SC->PCONP |= (1 << 21);

    /* Configure other SSP pins: SCK, MISO, MOSI */
    LPC_PINCON->PINSEL0 &= ~(3UL << 30);
    LPC_PINCON->PINSEL0 |=  (2UL << 30);          /* P0.15: SCK0 */
    LPC_PINCON->PINSEL1 &= ~((3UL<<2) | (3UL<<4));
    LPC_PINCON->PINSEL1 |=  ((2UL<<2) | (2UL<<4));    /* P0.17: MISO0, P0.18: MOSI0 */

    /* 8bit, SPI frame format, CPOL=0, CPHA=0, SCR=0 */  
    LPC_SSP0->CR0 = (0x07 << 0) |     /* data width: 8bit*/
                    (0x00 << 4) |     /* frame format: SPI */
                    (0x00 << 6) |     /* CPOL: low level */
                    (0x00 << 7) |     /* CPHA: first edge */
                    (0x00 << 8);      /* SCR = 0 */

    /* Enable SSP0 as a master */
    LPC_SSP0->CR1 = (0x00 << 0) |   /* Normal mode */
                    (0x01 << 1) |   /* Enable SSP0 */
                    (0x00 << 2) |   /* Master */
                    (0x00 << 3);    /* slave output disabled */

    /* Configure SSP0 clock rate to 400kHz (100MHz/250) */
    SPI_ConfigClockRate (FPCLK/frequence);
#endif

//LPC177X-8X
#if 0    

    LPC_SC->PCONP |= (0x1 << 21);                                   /* 开启SSP0外设                 */
    LPC_IOCON->P0_15 &= ~0x07;
    LPC_IOCON->P0_15 |=  0x02;                                      /* SSP CLK                      */
    //LPC_IOCON->P0_16 &= ~0x07;    
    //LPC_IOCON->P0_16 |=  0x02;                                    /* SSP SSEL                     */
    LPC_IOCON->P0_17 &= ~0x07;
    LPC_IOCON->P0_17 |=  0x02;                                      /* SSP MISO                     */
    LPC_IOCON->P0_18 &= ~0x07;    
    LPC_IOCON->P0_18 |=  0x02;                                      /* SSP MOSI                     */


    //In Master mode, this register must be an even number greater than or equal to 8
    Div_Freq = (FPCLK/frequence/2)&0xfe;
  LPC_SSP0->CPSR = 2;
    LPC_SSP0->CR0 &= 0xffffff00;    
  LPC_SSP0->CR0 = 0x07 << 0 |                                         /* 数据长度为8位                */
                  0x00 << 4 |                                         /* 帧格式为SPI                  */
                  0x00 << 6 |                                         /* CPOL为0                      */
                  0x00 << 7 |                                         /* CPHA为0                      */
                  (Div_Freq-1) << 8;                                  /* 串行时钟速率为7              */
    LPC_SSP0->CR1 |= (1<<1);
#endif    
}

/**
  * @brief  Configure SSP0 clock rate.
  *
  * @param  SPI_CLOCKRATE: Specifies the SPI clock rate.
  *         The value should be SPI_CLOCKRATE_LOW or SPI_CLOCKRATE_HIGH.
  * @retval None 
  *
  * SSP0_CLK = CCLK / SPI_CLOCKRATE
  */
void SPI_ConfigClockRate (INT32U SPI_CLOCKRATE)
{
    /* CPSR must be an even value between 2 and 254 */
    LPC_SSP0->CPSR = (SPI_CLOCKRATE & 0xFE);    
}
/**
  * @brief  Send one byte via MOSI and simutaniously receive one byte via MISO.
  *
  * @param  data: Specifies the byte to be sent out.
  * @retval Returned byte.
  *
  * Note: Each time send out one byte at MOSI, Rx FIFO will receive one byte. 
  */
INT8U SPI_SendByte (INT8U dat)
{
    /* Put the data on the FIFO */
    LPC_SSP0->DR = dat;
    /* Wait for sending to complete */
    while (LPC_SSP0->SR & SSP_SR_BSY);
    /* Return the received value */              
    return (LPC_SSP0->DR);                        
}

/**
  * @brief  Receive one byte via MISO.
  *
  * @param  None.
  * @retval Returned received byte.
  */
INT8U SPI_RecvByte (void)
{
    /* Send 0xFF to provide clock for MISO to receive one byte */
    return SPI_SendByte (0xFF);
}
/*
* function send byte
*/
INT8U SPI_SwapByte(INT8U dat){
      return SPI_SendByte(dat);
}
/* --------------------------------- End Of File ------------------------------ */

