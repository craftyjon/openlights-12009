/* Openlights Strand Controller (12009) Firmware
 * http://openlights.org
 * 
 * Copyright (c) 2012 Jonathan Evans <jon@craftyjon.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _GLOBALS_H
#define _GLOBALS_H

#include "asf.h"
#include "usb.h"

#ifdef GLOBALS_ALLOC
#define EXTERN extern
#else
#define EXTERN
#endif


/* Board metadata */

#define FW_REV_MAJOR 1
#define FW_REV_MINOR 0

/* Settings */
#define STRAND_BIT_RATE 2000000
#define RS485_BIT_RATE 2000000

EXTERN uint8_t g_address;
EXTERN uint8_t g_current_cmd_address;
EXTERN uint8_t g_usartData;
EXTERN uint64_t ticks;
EXTERN uint64_t updated_at;
EXTERN volatile uint8_t usb_inbyte;
EXTERN volatile uint8_t usb_dataready;


/* LEDs */
#define MAX_LEDS 160
#define ARRAY_SIZE 480
//#define RX_BUFFER_SIZE 490
#define RX_BUFFER_SIZE 1000
EXTERN uint16_t num_leds;

EXTERN uint16_t num_bytes;
EXTERN COMPILER_WORD_ALIGNED uint8_t data_buffer[ARRAY_SIZE];
//EXTERN COMPILER_WORD_ALIGNED uint8_t back_buffer[ARRAY_SIZE];
EXTERN volatile COMPILER_WORD_ALIGNED uint8_t rx_buffer[RX_BUFFER_SIZE];
EXTERN volatile uint16_t rx_write_ptr;
EXTERN volatile uint16_t rx_read_ptr;
EXTERN uint8_t *data_pointer;

EXTERN volatile uint8_t g_dirty;
EXTERN volatile uint8_t g_lock;
EXTERN volatile uint8_t g_pingpoing;
EXTERN uint8_t g_frame;


/* USB */
#define USB_BUFFER_SIZE 1000

EXTERN volatile bool _usb_configured;

EXTERN uint8_t g_cmdState;
EXTERN uint8_t g_escapeReturnState;
EXTERN uint8_t g_usbCommand;
EXTERN uint16_t g_usbDataLength;
EXTERN uint16_t g_usbDataCount;
EXTERN COMPILER_WORD_ALIGNED uint8_t g_usbDataBuffer[10 + ARRAY_SIZE];

EXTERN uint8_t g_usbConnected;
EXTERN volatile uint8_t g_rs485data;
EXTERN volatile uint8_t g_rs485rdy;

#endif
