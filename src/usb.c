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

#include <asf.h>

#include "globals.h"
#include "usb.h"
#include "commands.h"
#include "pins.h"


void process_usb(uint8_t byte)
{
	//usart_putchar(&USARTC0, 0xFF);
	//usart_putchar(&USARTC0, g_cmdState);
	switch (g_cmdState) {
		case STATE_IDLE:
			if (byte == ESCAPE_CHAR) {
				g_cmdState = STATE_ESCAPE;
			}
			break;
		
		case STATE_ESCAPE:
escape:
			if (byte == ESCAPE_SOF) {
				g_cmdState = STATE_ADDRESS;
			} else if (byte == ESCAPE_99) {
				g_cmdState = g_escapeReturnState;
				if (g_cmdState == STATE_START) {
					goto start;
				} else if (g_cmdState == STATE_RECEIVE) {
					goto receive;
				}
			}
			break;
			
		case STATE_ADDRESS:
			ioport_set_pin_high(LED_ACT);
			//usart_putchar(&USARTC0, 0x00);
			g_current_cmd_address = byte;
			g_usbDataCount = 0;
			g_cmdState = STATE_START;
			break;

		case STATE_START:
			if (byte == ESCAPE_CHAR) {
				g_escapeReturnState = STATE_START;
				g_cmdState = STATE_ESCAPE;
				break;
			}
start:
			//usart_putchar(&USARTC0, g_usbDataCount);
			if (g_usbDataCount == 0) {
				g_usbCommand = byte;
				g_usbDataCount++;
			} else if (g_usbDataCount == 1) {
				g_usbDataLength = byte;
				g_usbDataCount++;
			} else {
				g_usbDataLength += (byte << 8);
				if (g_usbDataLength > (USB_BUFFER_SIZE - 1)) {
					g_usbDataLength = USB_BUFFER_SIZE - 1;
				}
				
				if (g_usbDataLength > ARRAY_SIZE) {
					g_cmdState = STATE_IDLE;
					asm("nop;");
					break;
				}
				
				g_usbDataCount = 0;
				g_cmdState = STATE_RECEIVE;
			}
			break;

		case STATE_RECEIVE:
			if (byte == ESCAPE_CHAR) {
				g_escapeReturnState = STATE_RECEIVE;
				g_cmdState = STATE_ESCAPE;
				break;
			}
receive:
			//usart_putchar(&USARTC0, g_usbDataCount);
			if (g_usbDataCount < g_usbDataLength - 1) {
				g_usbDataBuffer[g_usbDataCount++] = byte;
			} else {
				//usart_putchar(&USARTC0, 0x88);
				g_usbDataBuffer[g_usbDataCount] = byte;
				if (g_current_cmd_address == g_address || g_current_cmd_address == 0) {
					//usart_putchar(&USARTC0, 0x99);
					ioport_set_pin_high(LED_DATA);
					process_command();
				}

				g_cmdState = STATE_IDLE;
			}
			break;
		default:
			g_cmdState = STATE_IDLE;
			break;
	}
}

#ifdef CHECKSUM
bool usb_checksum(uint8_t input)
{
	uint8_t calc = 0;
	uint16_t i;
	
	if (g_usbDataLength == 1) {
		return (input == g_usbDataBuffer[0]);
	}
	
	for (i = 0; i < g_usbDataLength; i++) {
		calc ^= g_usbDataBuffer[i];
	}
	
	return (input == calc);
}
#endif