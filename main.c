/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/17 10:00p $
 * @brief    Uart driver demo sample.
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "N575.h"
#include "uart_app.h"
#include "buffer.h"

/* Buffer size, this buffer for uart receive & send data. */
#define UART_BUF_SIZE      64

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable External OSC49M */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Switch HCLK clock source to HXT */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));

    /* Enable IP clock */
    CLK_EnableModuleClock(UART_MODULE);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set GPG multi-function pins for UART0 RXD and TXD */
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA8MFP_Msk) ) | SYS_GPA_MFP_PA8MFP_UART_TX;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA9MFP_Msk) ) | SYS_GPA_MFP_PA9MFP_UART_RX;

    /* Lock protected registers */
    SYS_LockReg();
}

/* Main */
int main(void)
{
//	uint8_t u8Buffer[UART_BUF_SIZE];
	
    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

    Initialize_buffer();
    UART_init();
    /* Init UART to 115200-8n1 for print message */
        
    OutputString_with_newline("\r\n+-----------------------+\r");
    OutputString_with_newline    ("| N575 Uart Demo Sample |\r");
    OutputString_with_newline    ("+-----------------------+\r");
	OutputString_with_newline    ("Press any key to test.\r");
	
	while(1)
	{
//		memset( u8Buffer, '\0', sizeof(u8Buffer) );
//		if( UART_Read( UART0, u8Buffer, sizeof(u8Buffer) ) > 0 )
//			UART_Write( UART0, u8Buffer, sizeof(u8Buffer) );
        
        if(!uart_input_queue_empty_status())
        {
            uint8_t input_ch;
            input_ch = uart_input_dequeue();
            while(uart_output_enqueue(input_ch)==0) {}
        }
	}
}

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
