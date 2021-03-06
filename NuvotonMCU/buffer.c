/****************************************************************************
 * @file     buffer.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 2017/11/22 10:00p $
 * @brief    All kinds of ring-buffer function - for UART & for output pulse
 *
 * @note
 *
 *
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "N575.h"
#include "buffer.h"
//#include "timer_app.h"

// helper function
// Clear Buffer
#define BUF_CLEAR(WRITE_PTR, REAR_PTR, BUFFER_START_PTR) {  WRITE_PTR=BUFFER_START_PTR; REAR_PTR=BUFFER_START_PTR; }
// Increase Ring Buffer Pointer by 1 and back to buffer starting point when out of bound
#define BUF_PTR_INCREASE(PTR, BUFFER_START_PTR, BUFFER_SIZE)                        \
    {                                                                               \
        PTR++;                                                                      \
        if(PTR>=(BUFFER_START_PTR+BUFFER_SIZE))                                     \
        {                                                                           \
          PTR = BUFFER_START_PTR;                                                   \
        }                                                                           \
    }
//
// End of helper function
//

// UART-RX Buffer
#define UART_RX_BUF_SIZE      128
uint8_t u8Buffer_RX[UART_RX_BUF_SIZE];
uint8_t *UART_BUF_RX_WRITE_PTR = u8Buffer_RX;
uint8_t *UART_BUF_RX_REAR_PTR = u8Buffer_RX;
uint8_t UART_BUF_RX_FULL = 0;

// UART-TX Buffer
#define UART_TX_BUF_SIZE      128
uint8_t u8Buffer_TX[UART_TX_BUF_SIZE];
uint8_t *UART_BUF_TX_WRITE_PTR = u8Buffer_TX;
uint8_t *UART_BUF_TX_REAR_PTR = u8Buffer_TX;
//uint8_t UART_BUF_TX_FULL = 0;

//// Store inputing IR data from UART    
//// IR-Data Array
//#define IR_DATA_BUF_SIZE      384
//uint32_t u32Buffer_IR_DATA_Width[IR_DATA_BUF_SIZE];
//uint32_t *IR_BUF_DATA_WRITE_PTR = u32Buffer_IR_DATA_Width;
///*
//// Use as data for Tx output
//// IR-pulse-TX Array
//#define IR_TX_BUF_SIZE      IR_DATA_BUF_SIZE
//uint32_t u32Buffer_IR_TX_Width[IR_TX_BUF_SIZE];
//uint32_t *IR_BUF_TX_WRITE_PTR =u32Buffer_IR_TX_Width;
//uint32_t *IR_BUF_TX_REAR_PTR =u32Buffer_IR_TX_Width;
//uint8_t IR_BUF_TX_FULL = 0;
//*/
//// New Tx mechanism -- try to use solely PWM
//// IR-PWM-Pulse-Array
//#define IR_PWM_BUF_SIZE      (IR_DATA_BUF_SIZE)
//T_PWM_BUFFER T_PWM_BUFFER_Buf[IR_PWM_BUF_SIZE];
//T_PWM_BUFFER *PWM_BUF_WRITE_PTR =T_PWM_BUFFER_Buf;
//T_PWM_BUFFER *PWM_BUF_READ_PTR =T_PWM_BUFFER_Buf;
////uint8_t PWM_BUF_FULL = 0;

////
//// Common function
////

//void Init_IR_buffer(void)
//{
//  IR_BUF_DATA_WRITE_PTR = u32Buffer_IR_DATA_Width;  
//  PWM_BUF_WRITE_PTR =T_PWM_BUFFER_Buf;
//  PWM_BUF_READ_PTR =T_PWM_BUFFER_Buf;
//}

void Initialize_buffer(void)
{
  BUF_CLEAR(UART_BUF_RX_WRITE_PTR, UART_BUF_RX_REAR_PTR, u8Buffer_RX);
  BUF_CLEAR(UART_BUF_TX_WRITE_PTR, UART_BUF_TX_REAR_PTR, u8Buffer_TX);
  //Init_IR_buffer();
}

//
// buffer function for UART-Input
//

uint8_t uart_input_queue_empty_status(void)
{
  return (UART_BUF_RX_WRITE_PTR==UART_BUF_RX_REAR_PTR)?TRUE:FALSE;
}

uint8_t uart_input_queue_full_status(void)
{
    uint8_t *temp_rear_ptr = UART_BUF_RX_REAR_PTR;
    
    if (temp_rear_ptr<UART_BUF_RX_WRITE_PTR)
    {
        /* ....rear.....write.....*/
        temp_rear_ptr+=UART_RX_BUF_SIZE;
        /* .............write.....rear*/
    }
    
    if(UART_BUF_RX_WRITE_PTR==(temp_rear_ptr-1))                                                 
    {
        return 1;           // full
    }
    else
    {
        return 0;           // not-full
    }
}

//
// This function is currently only used in UART Rx-interrupt
//
uint8_t uart_input_enqueue(uint8_t input_data)
{
  uint8_t   return_value;
    
  if(!uart_input_queue_full_status())  // must check a buffer-full status before an "add"
  {
    *UART_BUF_RX_WRITE_PTR = input_data;
    BUF_PTR_INCREASE(UART_BUF_RX_WRITE_PTR,u8Buffer_RX,UART_RX_BUF_SIZE);
    return_value = TRUE;
  }
  else
  {
    return_value = FALSE;
  }

  return return_value;
}

uint8_t uart_input_dequeue(void)
{
  uint8_t   return_value;

  UART0->INTEN &= ~UART_INTEN_RDAIEN_Msk; // update buffer-full status in the block where UART-RX interrupt is disabled temporarily.

  return_value = *UART_BUF_RX_REAR_PTR;

  if(!uart_input_queue_empty_status())
  {
    BUF_PTR_INCREASE(UART_BUF_RX_REAR_PTR,u8Buffer_RX,UART_RX_BUF_SIZE);
  }

  UART0->INTEN |= UART_INTEN_RDAIEN_Msk;
  return return_value;
}

//
// buffer function for UART-Output
//

//
// Please Make sure this function is used when interrupt is disabled (or within ISR)
//
uint8_t uart_output_queue_empty_status(void)
{
  return (UART_BUF_TX_WRITE_PTR==UART_BUF_TX_REAR_PTR)?TRUE:FALSE;
}

uint8_t uart_output_queue_full_status(void)
{
    uint8_t *temp_rear_ptr = UART_BUF_TX_REAR_PTR;
    
    if (temp_rear_ptr<UART_BUF_TX_WRITE_PTR)
    {
        /* ....rear.....write.....*/
        temp_rear_ptr+=UART_TX_BUF_SIZE;
        /* .............write.....rear*/
    }
    
    if(UART_BUF_TX_WRITE_PTR==(temp_rear_ptr-1))                                                 
    {
        return 1;           // full
    }
    else
    {
        return 0;           // not-full
    }
}

uint8_t uart_output_enqueue(uint8_t input_data)
{
  uint8_t bRet;
    
  UART0->INTEN &= ~UART_INTEN_THREIEN_Msk;   // access queue so UART-TX interrupt (where it will dequeue) is disabled temporarily.  

  // if output_queue is empty now, we can put data to Tx directly  
  if((uart_output_queue_empty_status())&&(!(UART0->FIFOSTS & UART_FIFOSTS_TXFULL_Msk)))
  {
    UART0->DAT = input_data;
    bRet = 1;
  } 
  else if(!uart_output_queue_full_status())  // must check a buffer-full status before an "add"
  {
    *UART_BUF_TX_WRITE_PTR = input_data;
    BUF_PTR_INCREASE(UART_BUF_TX_WRITE_PTR,u8Buffer_TX,UART_TX_BUF_SIZE);
    bRet = 1;
  }
  else
  {
    bRet = 0;
  }
  UART0->INTEN |= UART_INTEN_THREIEN_Msk;   // Enable Tx interrupt to consume output buffer  
  return bRet;
}

uint8_t uart_output_enqueue_with_newline (uint8_t input_data)
{
    if(uart_output_enqueue(input_data))
    {
        if(uart_output_enqueue('\n'))
        {
            return (2);
        }
        else
        {
            return (1);
        }
    }
    else
    {
        return(0);
    }
}

//
// This function is currently only used in UART Tx-interrupt
//
uint8_t uart_output_dequeue(void)
{
  uint8_t   return_value;

  return_value = *UART_BUF_TX_REAR_PTR;
  if(!uart_output_queue_empty_status())
  {
    BUF_PTR_INCREASE(UART_BUF_TX_REAR_PTR,u8Buffer_TX,UART_TX_BUF_SIZE);
  }

  return return_value;
}

////
//// IR-pulse-DATA Array function
////
//void IR_data_restart_write_pointer(void)
//{
//  IR_BUF_DATA_WRITE_PTR = u32Buffer_IR_DATA_Width;
//}

//uint8_t IR_data_full(void)
//{
//    return (IR_BUF_DATA_WRITE_PTR>=(u32Buffer_IR_DATA_Width+IR_DATA_BUF_SIZE))? TRUE: FALSE;
//}

//uint8_t IR_data_add(uint32_t input_data)
//{
//  if(!IR_data_full())  // must check a buffer-full status before an "add"
//  {
//    *IR_BUF_DATA_WRITE_PTR = input_data;
//    IR_BUF_DATA_WRITE_PTR++;
//    return TRUE;
//  }
//  else
//  {
//    return FALSE;
//  }
//}
///*
//void Copy_Input_Data_to_Tx_Data_and_Start(void)
//{
//    uint32_t *src = u32Buffer_IR_DATA_Width, *end = IR_BUF_DATA_WRITE_PTR;
//    IR_BUF_TX_WRITE_PTR = u32Buffer_IR_TX_Width;
//    if(src<end)
//    {
//        *IR_BUF_TX_WRITE_PTR++ = *src++;
//        IR_Transmit_Buffer_StartSend();
//        while(src<end)                   // from u32Buffer_IR_DATA_Width to IR_BUF_DATA_WRITE_PTR
//        {
//            *IR_BUF_TX_WRITE_PTR++ = *src++;
//        }
//    }
//}

////
//// IR-pulse-TX Array function
////

//void IR_output_restart_read_pointer(void)
//{
//  IR_BUF_TX_REAR_PTR = u32Buffer_IR_TX_Width;
//}

//uint8_t IR_output_end_of_data(void)
//{
//    return (IR_BUF_TX_REAR_PTR==IR_BUF_TX_WRITE_PTR)? TRUE: FALSE;
//}

//uint8_t IR_output_read(uint32_t *return_value_ptr)
//{
//  if(IR_BUF_TX_REAR_PTR<IR_BUF_TX_WRITE_PTR)
//  {
//    *return_value_ptr = *IR_BUF_TX_REAR_PTR;
//    IR_BUF_TX_REAR_PTR++;
//    return TRUE;
//  }
//  else
//  {
//    return FALSE;
//  }
//}
//*/
////
//// PWM-pulse Array function
////
////uint32_t    Get_PWM_period(void) { return PWM_period; }
////void        Set_PWM_period(uint32_t period) { PWM_period = period; }
////uint32_t    Get_PWM_duty_cycle(void) { return PWM_duty_cycle; }

//#define PWM_CLOCK_1MS_TICK   (1024)       // 1ms has this number of tick of PWM-clock --> 1us has PWM_CLOCK_1MS_TICK/1000 tick
//void Copy_Input_Data_to_PWM_Data_and_Start(void)
//{
//    uint32_t *src = u32Buffer_IR_DATA_Width, *end = IR_BUF_DATA_WRITE_PTR;
//    // carrier width is (8*) of ticks in us so /8 -> pwm-prescaler* more ticks with such pwm-clock (assumed 48MHz) setting in us ==> pwm-clock is actually PWM_CLOCK_1MS_TICK/1000 in terms of 1us-duration
//    const uint32_t pwm_period = Get_PWM_period() * PWM_CLOCK_UNIT_DIVIDER * (PWM_CLOCK_1MS_TICK/8) / 1000; // Get_PWM_period() * PWM_CLOCK_UNIT_DIVIDER / 8 * PWM_CLOCK_1MS_TICK / 1000; 
//    const uint32_t pwm_high = ( pwm_period * Get_PWM_duty_cycle() * 2 + 1  ) / 200;
//    const uint32_t pwm_low = pwm_period - pwm_high;
//    uint64_t       temp_low_width;
//    PWM_BUF_WRITE_PTR = T_PWM_BUFFER_Buf; // Destination from beginning
//    while(src<end)
//    {
//        // Calculate PWM high pulse - width is ticks ticks in us -> pwm-prescaler* more ticks with such pwm-clock (assumed 48MHz) setting in us ==> pwm-clock is actually PWM_CLOCK_1MS_TICK/1000 in terms of 1us-duration
//        const uint32_t high_width = (*src++) * PWM_CLOCK_UNIT_DIVIDER * PWM_CLOCK_1MS_TICK / 1000;      
//        //const uint64_t low_width = ;       
//        const uint32_t complete_cycle = high_width / pwm_period;
//        const uint32_t buffer_limit = 0x10000;
//        temp_low_width = pwm_low + ((uint64_t)(*src++)) * PWM_CLOCK_UNIT_DIVIDER * PWM_CLOCK_1MS_TICK / 1000; 
//       
//        // Case 1: remaining is not zero -> last is extended to a *complete* but low_cnt combines with next low-pulse
//        if((high_width - (pwm_period*complete_cycle))>0)
//        {
//            PWM_BUF_WRITE_PTR->repeat_no=complete_cycle;
//            PWM_BUF_WRITE_PTR->high_cnt=pwm_high;
//            PWM_BUF_WRITE_PTR->low_cnt=pwm_low;
//            PWM_BUF_WRITE_PTR++;
//            PWM_BUF_WRITE_PTR->repeat_no = 1;
//            PWM_BUF_WRITE_PTR->high_cnt = pwm_high;
//        }
//        // Case 2: No Remaining --> last is simply the same except low_cnt combines with next low-pulse
//        else
//        {
//            PWM_BUF_WRITE_PTR->repeat_no=complete_cycle-1;
//            PWM_BUF_WRITE_PTR->high_cnt=pwm_high;
//            PWM_BUF_WRITE_PTR->low_cnt=pwm_low;
//            PWM_BUF_WRITE_PTR++;
//            PWM_BUF_WRITE_PTR->repeat_no = 1;
//            PWM_BUF_WRITE_PTR->high_cnt = pwm_high;
//        }
//        // Check if current (high_cnt+low_cnt) is > 0x10000, 
//        // Assumption: high_cnt is always short (< then 0x1000) and only low_cnt could be very long
//        if((temp_low_width+pwm_high)<buffer_limit)        
//        {
//            // if no, use it as low_cnt directly
//            PWM_BUF_WRITE_PTR->low_cnt = (uint32_t)temp_low_width;
//            PWM_BUF_WRITE_PTR++;
//        }
//        else    
//        {
//            // if yes, update last packet 
//            const uint32_t long_wait_cnt = buffer_limit - 0x2000; // to make sure there are some remaining length at last wait packet & (high_cnt+long_wait_cnt)< buffer_limit
//            PWM_BUF_WRITE_PTR->low_cnt = long_wait_cnt;
//            PWM_BUF_WRITE_PTR++;
//            
//            // and start to separate very long low-pulse into several data packet where high_cnt==0
//            temp_low_width -= long_wait_cnt;    
//            // if still to larger, divide until it is not too large
//            if(temp_low_width>=buffer_limit)        
//            {
//                // prepare long-delay packet
//                PWM_BUF_WRITE_PTR->repeat_no = 1;
//                PWM_BUF_WRITE_PTR->high_cnt = 0;
//                PWM_BUF_WRITE_PTR->low_cnt = long_wait_cnt;
//                temp_low_width -= long_wait_cnt;   
//                while(temp_low_width>=buffer_limit)
//                {                    
//                    PWM_BUF_WRITE_PTR->repeat_no++;
//                    temp_low_width -= long_wait_cnt;   
//                }
//                PWM_BUF_WRITE_PTR++;
//            }

//            // Pack the remaining low-pulse
//            PWM_BUF_WRITE_PTR->repeat_no = 1;
//            PWM_BUF_WRITE_PTR->high_cnt = 0;
//            PWM_BUF_WRITE_PTR->low_cnt = temp_low_width;
//            PWM_BUF_WRITE_PTR++;
//        }
//        
//        PWM_Transmit_Buffer_StartSend();
//    };
//}

//void PWM_Pulse_restart_read_pointer(void)
//{
//  PWM_BUF_READ_PTR = T_PWM_BUFFER_Buf;
//}

//uint8_t PWM_Pulse_end_of_data(void)
//{
//    return (PWM_BUF_READ_PTR==PWM_BUF_WRITE_PTR)? TRUE: FALSE;
//}

//uint8_t PWM_Pulse_read(T_PWM_BUFFER *return_value_ptr)
//{
//  if(PWM_BUF_READ_PTR<PWM_BUF_WRITE_PTR)
//  {
//    *return_value_ptr = *PWM_BUF_READ_PTR;
//    PWM_BUF_READ_PTR++;
//    return TRUE;
//  }
//  else
//  {
//    return FALSE;
//  }
//}
