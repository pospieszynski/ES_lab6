#include "AT91SAM9263.h"

#define LED_SWITCH 100000

#define SET_LED1         (1 << 29)
#define SET_LED2         (1 << 8)
#define SET_RIGHT_BUTTON (1 << 5)
#define SET_LEFT_BUTTON  (1 << 4)
#define SET_CLOCK        (1 << 4)

volatile unsigned int *PIOB_PER  = (volatile unsigned int *) 0xFFFFF400;
volatile unsigned int *PIOB_OER  = (volatile unsigned int *) 0xFFFFF410;
volatile unsigned int *PIOB_SODR = (volatile unsigned int *) 0xFFFFF430; 
volatile unsigned int *PIOB_CODR = (volatile unsigned int *) 0xFFFFF434;
volatile unsigned int *PIOC_ODR  = (volatile unsigned int *) 0xFFFFF614;
volatile unsigned int *PIOC_PDSR = (volatile unsigned int *) 0xFFFFF63C;
volatile unsigned int *PIOC_PUER = (volatile unsigned int *) 0xFFFFF664;
volatile unsigned int *PMC_PCER  = (volatile unsigned int *) 0xFFFFFC10;

volatile unsigned int *PIOC_CODR = (volatile unsigned int *) 0xFFFFF634;
volatile unsigned int *PIOC_SODR = (volatile unsigned int *) 0xFFFFF630;
volatile unsigned int *PIOC_OER  = (volatile unsigned int *) 0xFFFFF610;
volatile unsigned int *PIOC_PER  = (volatile unsigned int *) 0xFFFFF600;

void dbgu_print_ascii (const char *a) {};
void initLED();
void blinkLED();
void delay(unsigned int);
void enableLED1();
void disableLED1();
void checkIfButtonPressed();
void interruptHandler(void);
void initInterrupts();

volatile unsigned int increment = 0;
volatile unsigned int spuriousCounter = 0;

int main () {
	initLED();
	initInterrupts();
	while (1) {}
}

void initLED() {
	*PIOC_PER  = (volatile unsigned int) SET_LED1;
	*PIOC_OER  = (volatile unsigned int) SET_LED1;
	*PIOC_SODR = (volatile unsigned int) SET_LED1;

	*PIOB_PER  = (volatile unsigned int) SET_LED2;
	*PIOB_OER  = (volatile unsigned int) SET_LED2;
	*PIOB_SODR = (volatile unsigned int) SET_LED2;
	
	*PIOC_PER  = (volatile unsigned int) SET_RIGHT_BUTTON;
	*PIOC_ODR  = (volatile unsigned int) SET_RIGHT_BUTTON;
	*PIOC_PUER = (volatile unsigned int) SET_RIGHT_BUTTON;
	
	*PIOC_PER  = (volatile unsigned int) SET_LEFT_BUTTON;
	*PIOC_ODR  = (volatile unsigned int) SET_LEFT_BUTTON;
	*PIOC_PUER = (volatile unsigned int) SET_LEFT_BUTTON;
	
	*PMC_PCER  = (volatile unsigned int) SET_CLOCK;
}

void delay(unsigned int delay) {
	volatile unsigned int inc = 0;
	while (inc != delay) {
		inc++;
	}
}
void enableLED1() {
	*PIOC_CODR = (volatile unsigned int) SET_LED1;
}
void disableLED1() {
	*PIOC_SODR = (volatile unsigned int) SET_LED1;
}

void interruptHandler(void) {
	// INTERRUPT STATUS REGISTER PORT C
	// 1 = At least one Input Change has been detected on the I/O line since PIO_ISR was last read or since reset.
	// No Input Change has been detected on the I/O line since PIO_ISR was last read or since reset
	volatile unsigned int CLEAR_FLAG = AT91C_BASE_PIOC -> PIO_ISR;  // #1

	if(CLEAR_FLAG & ((SET_LEFT_BUTTON) && (SET_RIGHT_BUTTON))) // BOTH BTNS PRESSED
		*PIOB_CODR = SET_LED2;

	if(CLEAR_FLAG & (SET_LEFT_BUTTON)) { // LEFT BTN PRESSED
		*PIOB_SODR = SET_LED2;
	}
	else if(CLEAR_FLAG & (SET_RIGHT_BUTTON)) { // RIGHT BTN PRESSED
		*PIOB_CODR = SET_LED2;
	} else { // ERROR?
		spuriousCounter++;
	}

	AT91C_BASE_PIOC -> PIO_ISR; // needed? whe have it in #1(81 line)
	/*  all the interrupts are automatically cleared. This signifies that
	all the interrupts that are pending when PIO_ISR is read must be handled.
	Pending interrupt is the one with lowest priority, waiting in a queue??
	 */ 
}
void initInterrupts() {
	AT91C_BASE_PIOC -> PIO_ISR; // CLEAR ALL INTERRUPTS (TRIGGER ALL PENDING ONES NOT TO BE SURPRISED..)

	// Turn off interrupts for port C/D/E
	AT91C_BASE_AIC  -> AIC_IDCR = 1 << AT91C_ID_PIOCDE; 								//INTERRUPT DISABLE COMMAND REGISTER

	AT91C_BASE_AIC  -> AIC_SVR[AT91C_ID_PIOCDE]  = (unsigned int) interruptHandler; 	//STORE ADDRESS OF CORRESPONDING INTERRUPT HANDLER
	
	AT91C_BASE_AIC  -> AIC_SMR[AT91C_ID_PIOCDE]  = AT91C_AIC_SRCTYPE_EXT_HIGH_LEVEL;	//INTERRUPT DETECT ON HIGH
	AT91C_BASE_AIC  -> AIC_SMR[AT91C_ID_PIOCDE] |=  AT91C_AIC_PRIOR_HIGHEST;			//HIGH PRIORITY
	
	// DEACTIVATE INTERRUPT for C/D/E port HERE
	AT91C_BASE_AIC  -> AIC_ICCR = 1 << AT91C_ID_PIOCDE;									//INTERRUPT CLEAR COMMAND
	
	// ACTIVATE INTERRUPTS ON BTN PINS
	AT91C_BASE_PIOC -> PIO_IER  = SET_LEFT_BUTTON;										//INTERRUPT ENABLE REGISTER
	AT91C_BASE_PIOC -> PIO_IER  = SET_RIGHT_BUTTON;										//INTERRUPT ENABLE REGISTER

	// AND NOW ACTIVATE INTERRUPT for C/D/E port HERE
	AT91C_BASE_AIC  -> AIC_IECR = 1 << AT91C_ID_PIOCDE;									//ENABLE INT FOR BUTTONS
}