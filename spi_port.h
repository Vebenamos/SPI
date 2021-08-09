#ifndef __SPI_PORT_H
#define __SPI_PORT_H

#include "includes.h"                     /* LPC17xx Definitions */

/* Public functions */
extern void    SPI_Init               (void);
extern void    SPI_Init16Bit          (void);
extern void    SPI_ConfigClockRate    (INT32U  SPI_CLOCKRATE);

//8  bit
extern INT8U   SPI_SendByte           (INT8U   dat);
extern INT8U   SPI_RecvByte           (void);
extern INT8U   SPI_SwapByte           (INT8U   dat);
//16 bit
extern INT16U  SPI_SendBytes          (INT16U  dat);
extern INT16U  SPI_RecvBytes          (void);
extern INT16U  SPI_SwapBytes          (INT16U  dat);

#endif  // __LPC17xx_SPI_H
/* --------------------------------- End Of File ------------------------------ */
