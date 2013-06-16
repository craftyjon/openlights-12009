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

#include <stdio.h>
#include <string.h>
#include <asf.h>
#include <util/delay.h>

#define GLOBALS_ALLOC
#include "globals.h"
#undef GLOBALS_ALLOC

//void cdc_rx_callback(void);

#include "pins.h"
#include "usb.h"
#include "commands.h"
#include "init.h"


// High-frequency timer
static void strand_output_isr(void)
{
	//static uint8_t b = 0;
	
	//uint8_t ints = cpu_irq_save();
	
	if (g_dirty && !g_frame && !g_lock) {
		//b++;
		g_dirty = 0;
		// Reset strand by holding clock low for 500us
		PORTC.OUTCLR = (1<<7);
		//delay_us(500);
		for (int dly = 0; dly < 8000; dly++) { asm("nop;"); }
		PORTC.OUTSET = (1<<7);
		
		//memset(data_buffer, b, ARRAY_SIZE);
			
		spi_write_packet(&SPIC, data_buffer, ARRAY_SIZE);
		ioport_set_pin_low(LED_ACT);
		ioport_set_pin_low(LED_DATA);
	}
	
	//cpu_irq_restore(ints);
}


// Low-frequency timer
static void tick_isr(void)
{
	// Refresh the strand periodically even if no new data
	if (!g_frame && (updated_at != ticks) && (ticks % 2 == 0)) {
		ioport_set_pin_high(LED_ACT);
		ioport_set_pin_low(LED_DATA);
		g_dirty = 1;
		updated_at = ticks;
		//g_cmdState = STATE_IDLE;
	}
	
	ticks++;
}


// RS485 receive ISR
ISR(USARTC0_RXC_vect)
{
	rx_buffer[rx_write_ptr] = USARTC0.DATA;
	rx_write_ptr = (rx_write_ptr + 1) % RX_BUFFER_SIZE;
	//usb_inbyte = USARTC0.DATA;
	g_rs485rdy = 1;
}

/*
void cdc_rx_callback(void)
{
	ioport_toggle_pin_level(LED_DATA);
	usb_inbyte = udi_cdc_getc();
	usart_putchar(&USARTC0, usb_inbyte);
	usb_dataready = 1;	
}
*/

int main (void)
{
	//uint8_t usbbyte;

	// ASF board initialization
	sysclk_init();
	pmic_init();
	irq_initialize_vectors();
	cpu_irq_enable();
	
	// Openlights board initialization
	init_pins();
	init_globals();
	init_strand();
	init_rs485();
	//init_usb();

	// Setup timer interrupt for strand output
	tc_enable(&TCC0);
	tc_set_overflow_interrupt_callback(&TCC0, strand_output_isr);
	tc_set_wgm(&TCC0, TC_WG_NORMAL);
	tc_write_period(&TCC0, 818);	// about 60 fps
	tc_write_clock_source(&TCC0, TC_CLKSEL_DIV64_gc);	// 125 khz
	tc_set_overflow_interrupt_level(&TCC0, TC_INT_LVL_HI);
	
	
	// Low-frequency timer
	tc_enable(&TCC1);
	tc_set_overflow_interrupt_callback(&TCC1, tick_isr);
	tc_set_wgm(&TCC1, TC_WG_NORMAL);
	tc_write_period(&TCC1, 0xFFFF);
	tc_write_clock_source(&TCC1, TC_CLKSEL_DIV256_gc);
	tc_set_overflow_interrupt_level(&TCC1, TC_INT_LVL_LO);
	
	// run the "Identify" command at startup
	g_usbCommand = IDENTIFY;
	process_command();
	
	g_cmdState = STATE_IDLE;
	
	while (1) {
		if (!ioport_get_pin_level(SW5)) {	// Test mode
			// Disable transmitter
			ioport_set_pin_low(RS485_DE);
			ioport_set_pin_low(RS485_REn);
			ioport_set_pin_high(LED_DATA);
			memset(data_buffer, 100, sizeof(data_buffer));
			g_dirty = 1;
			usart_putchar(&USARTC0, 'U');
		} else {

			/*if (udi_cdc_is_rx_ready()) {
			//if (usb_dataready) {
				if (!g_usbConnected) {
					g_usbConnected = 1;
					// Enable RS485 transmitter if USB is connected.
					// For now, this requires a power-cycle to clear.
					ioport_set_pin_high(RS485_DE);
					ioport_set_pin_high(RS485_REn);
				}
				
				//usb_dataready = 0;
				
				//do {
					usb_inbyte = udi_cdc_getc();
					usart_putchar(&USARTC0, usb_inbyte);
					process_usb(usb_inbyte);
				//} while (udi_cdc_is_rx_ready());				
			} else */
			if (rx_write_ptr != rx_read_ptr) {
			//if (g_rs485rdy) {
				g_rs485rdy = 0;
				process_usb(rx_buffer[rx_read_ptr]);
				rx_read_ptr = (rx_read_ptr + 1) % RX_BUFFER_SIZE;
				//process_usb(usb_inbyte);
			}
		}
		/*
		if (g_dirty && !g_frame) { // && !g_lock) {
			g_dirty = 0;
			// Reset strand by holding clock low for 500us
			PORTC.OUTCLR = (1<<7);
			for (int dly = 0; dly < 8000; dly++) { asm("nop;"); }
			PORTC.OUTSET = (1<<7);
			
			spi_write_packet(&SPIC, data_buffer, ARRAY_SIZE);
			ioport_set_pin_low(LED_ACT);
			ioport_set_pin_low(LED_DATA);
		}
		*/
	}
}
