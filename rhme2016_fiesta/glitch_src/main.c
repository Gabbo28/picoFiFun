#include "pico/stdlib.h"
#include "hardware/irq.h"
#include <stdio.h>
#include <string.h>

#define GLITCH_PIN 2
#define BUTTON_PIN 15
#define LED_PIN 14

#define UART_ID uart0
#define BAUD_RATE 19200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define BUF_SIZE 200

char buf[BUF_SIZE];
int k=0; //buffer index

// RX interrupt handler
void on_uart_rx() {
	if (k==BUF_SIZE) {
		k=0;
	}
    while (uart_is_readable(UART_ID)) {
	    uint8_t ch = uart_getc(UART_ID);
		buf[k]=ch;
		k++;
    }
}

void glitch(int delay) {
	gpio_put(GLITCH_PIN,0);
	for (int i=0; i<delay;i++) {asm("nop");}
	gpio_put(GLITCH_PIN,1);
}


int main() {
	
	stdio_init_all();
	
	// init uart: set base baudrate, then change with actual one
	uart_init(UART_ID, 2400);
	
	int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);
	
	// set TX and RX pins using function select on GPIO
	gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
	
	// Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

	
	// init other stuff
	gpio_init(LED_PIN);
	gpio_init(GLITCH_PIN);
	gpio_init(BUTTON_PIN);
	
	gpio_set_dir(LED_PIN,GPIO_OUT);
	gpio_set_dir(GLITCH_PIN,GPIO_OUT);
	gpio_set_dir(BUTTON_PIN,GPIO_IN);
	gpio_pull_down(BUTTON_PIN); // pull BUTTON_PIN down
	
	gpio_put(GLITCH_PIN,1);
	
	// variables for button_state readings
	int LAST_STATE = 0;
    int lastFlickeringState = 0;  // the previous flickerable state from the input pin
    int CURRENT_STATE; // these are BUTTON state variables, not LED. Needed to track the length of button's pressing
    int DEBOUNCE_DELAY = 25;   // the debounce time; increase if the output flickers
		
	// glitch delay definition
	int d=0;
	
	while (true) {
		// read values from uart0, receiving from the target
		printf("\nReading from target: ");
		printf("%s",buf);
		
		// toggle LED, or not, depending on button state (with debouncing)
		CURRENT_STATE = gpio_get(BUTTON_PIN); //read button state
		lastFlickeringState = CURRENT_STATE; //record the button state
		sleep_ms(DEBOUNCE_DELAY);
		CURRENT_STATE = gpio_get(BUTTON_PIN); //read button again
		if (lastFlickeringState == CURRENT_STATE) { //only toggle LED if change "was constant" for debounce_delay time
			if (CURRENT_STATE==1 && LAST_STATE==0) {
				gpio_put(LED_PIN, !gpio_get(LED_PIN));
				d=0; //reset glitch delay whenever LED changes state
			}
    		LAST_STATE = CURRENT_STATE;
		}
		
		// if LED is ON, glitch (using saved value for d)
		if (gpio_get(LED_PIN)==1) {
			if (d==0) {printf("\nGlitching!");}
			d=d+100;
			glitch(d);
			printf("\nGlitch delay set to %d",d);
			
		}
		sleep_ms(1000);
	}
}
