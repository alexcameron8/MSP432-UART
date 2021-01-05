/**
Final project SYSC3310
Alex Cameron
101114698
*/
#include "msp.h" 

/*
Configures the switches as GPIO
*/
void configSwitches(void){
	//Set SEL0 and SEL1 to 0 for GPIO
	P1->SEL0 &=  (uint8_t)(~((1 << 4) | (1 << 1) | (1 << 0)));
	P1->SEL1 &= (uint8_t)(~((1 << 4) | (1 << 1) | (1 << 0)));
	P2->SEL0 &= (uint8_t)(~((1 << 2) | (1 << 1) | (1 << 0)));
	P2->SEL1 &= (uint8_t)(~((1 << 2) | (1 << 1) | (1 << 0)));
	
	//Configure the switches (P1.1 and P1.4) as inputs, using pull-up internal resistors
	P1->DIR &= (uint8_t)(~((1 << 4) | (1 << 1)));
	P1->REN |= (uint8_t)((1 << 4) | (1 << 1));
	P1->OUT |= (uint8_t)((1 << 4) | (1 << 1));
	
	//Device interrupt configuration
	P1IES |= (uint8_t)((1 << 4) | (1 << 1));
	P1IFG &= (uint8_t)(~((1 << 4) | (1 << 1)));
	P1IE |= (uint8_t)((1 << 4) | (1 << 1)); 
	
	//NVIC configuration for (Port1)
	NVIC_SetPriority(PORT1_IRQn, 2);
	NVIC_ClearPendingIRQ(PORT1_IRQn);
	NVIC_EnableIRQ(PORT1_IRQn);
	}

void clearLEDS(void){
	//clear all LEDS
	P1OUT &= ~(uint8_t) (1<<0);
	P2OUT &= ~(uint8_t) (1<<0);
}

void setState1(void){ //No leds on
	//Clear LEDS;
	clearLEDS();
}

void setState2(void){ // LED 1 is on
	P1OUT |= (uint8_t)(1<<0);	 //Turn LED1 on
	P2OUT &= ~(uint8_t) (1<<0); //Turn LED2 off
}

void setState3(void){ // LED 2 is on
	//LED 2 on 
	P1OUT &= ~(uint8_t) (1<<0); //LED1 is off
	P2OUT |= (uint8_t)(1<<0);	 //LED2 is on
}

void setState4(void){ // both LEDS are on
	//Both Leds on
	P1OUT |= (uint8_t)(1<<0);	//Turn LED 1 on
	P2OUT |= (uint8_t)(1<<0); //Turn LED 2 on
}

/*
This function acts as the state machine in which given certain data in the parameter
it sets the state on the board to 1,2,3 or 4.
*/
void setStateX(uint16_t s){ //
	if(s == 49){
		setState1();
	}else if(s ==50){
		setState2();
	}else if(s ==51){
		setState3();
	}else if(s ==52){
		setState4();
	}
}
/*
Configuring the GPIO for the LED outputs.
*/
void configureLEDGPIO(void){
		// Configure the LEDs (P1.0 and P2.0, P2.1, P2.2) as outputs. 
	P1->DIR |= (uint8_t) ((1 << 0));
	P2->DIR |= (uint8_t) ((1 << 1) | (1 << 2) | (1 << 0));
	
	// outputs active high
	P2->OUT &=~ (uint8_t) ((1 << 1) | (1 << 2) | (1 << 0));
	P1->OUT &=~ (uint8_t) ((1 << 0));
	
	//Initialize LEDs states (all turned off)
	P1OUT &=~ (uint8_t)((1 << 0));
	P2OUT &=~ (uint8_t)((1 << 1) | (1 << 2) | (1 << 0));
}

/*
Initializes UART, uses no bit parity,8 bit mode, 9600 baud rate, SMCLK @ 12 MHZ, Oversampling mode
Note: This UART's configuration uses the same settings from the UART echo example provided on culearn webpage.
*/
void init_uart0(void){
	  CS->KEY = CS_KEY_VAL;                   // Unlock CS module for register access
    CS->CTL0 = 0;                           // Reset tuning parameters
    CS->CTL0 = CS_CTL0_DCORSEL_3;           // Set DCO to 12MHz (nominal, center of 8-16MHz range)
    CS->CTL1 = CS_CTL1_SELA_2 |             // Select ACLK = REFO
            CS_CTL1_SELS_3 |                // SMCLK = DCO
            CS_CTL1_SELM_3;                 // MCLK = DCO
    CS->KEY = 0;                            // Lock CS module from unintended accesses

		// Configure UART pins
    P1->SEL0 |= BIT2 | BIT3;                // set 2-UART pin as secondary function (bit 2 and bit 3)

    // Configure UART
    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST; // Put eUSCI in reset
	
    EUSCI_A0->CTLW0 = EUSCI_A_CTLW0_SWRST | ((uint16_t)0x0080);      //set EUSCI clock to SMCLK while EUSCI remains in reset
	
		//Baud Rate calculation: 000000/(16*9600) = 78.125
    //Fractional portion = 0.125
    //From Table 24-5:Recommended Settings For Typical Crystals and Baud Rates
		//UCBRSx = 0x10
    //UCBRFx = int ( (78.125-78)*16) = 2
		//This configuartion has no TX error % and approx no RX error
    EUSCI_A0->BRW = 78;                     // 12000000/16/9600
    EUSCI_A0->MCTLW = (2 << EUSCI_A_MCTLW_BRF_OFS) | EUSCI_A_MCTLW_OS16; // UCBRF Bit Offset and enabling oversampling

    EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST; // Init EUSCI
    EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;    // Clear previous EUSCI receive interrupt flag
    EUSCI_A0->IE |= EUSCI_A_IE_RXIE;        // Enable EUSCI receive registers interrupts
}

/*
Configures UART interrupts NVIC settings
*/
void uart_interrupt_config(void){
	EUSCI_A0->IE |= (EUSCI_A_IE_TXIE | EUSCI_A_IE_RXIE);
	NVIC_SetPriority(EUSCIA0_IRQn, 3);
	NVIC_EnableIRQ( EUSCIA0_IRQn );
	NVIC_ClearPendingIRQ(EUSCIA0_IRQn);
}

/*
This function uses the EUSCI transmitting register
*/
void transmit_info(uint16_t info){
	 while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));
	EUSCI_A0->TXBUF = info;
}

int main(void){
	
	//stop watchdog timer
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
	
	configSwitches();
	uart_interrupt_config();
	configureLEDGPIO();
	//uart_config();
	init_uart0();
	
		// Globally enable interrupts in CPU
	__ASM("CPSIE I"); 
	while(1){	} //while loop so interrupts can be handled
}


/*
UART interrupt service routine
*/
void EUSCIA0_IRQHandler(void)
{
    if (EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG) //If there is a receive interrupt
    {
			setStateX(EUSCI_A0->RXBUF); //set the state depending what value the monitor app sent
				
      // Check if the TX buffer is empty first
      while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));	
				//clear the transmit interrupt flag
				EUSCI_A0->IFG &= ~UCTXIFG;
        // Echo the received data back
        EUSCI_A0->TXBUF = EUSCI_A0->RXBUF;
    }
}


/*
Port 1 interrupt service routine 
*/
void PORT1_IRQHandler(void){
	static uint16_t state = 1;
	
	if((P1->IFG & (uint8_t)(1<<1))!=0){ //switch 1 was pressed
		//clear flag
		P1->IFG &= (uint8_t)(~(1<<1));
		//reset to state 1
		setState1();
		transmit_info(0x1); //send info back to monitoring app
		state =0x2;  //change to next state
	}
	else{
		if((P1IFG & (uint8_t) (1<<4)) != 0) //switch 2 has been pressed
				{
			P1->IFG &= (uint8_t)(~(1<<4)); //clear the P1.4 interrupt flag
				
				if(state ==1){
					setState1();
					transmit_info(state);
					state= 0x02; //change to next state
				}else if(state == 2){
					setState2();
					transmit_info(state);
					state=0x03;
				}else if(state ==3){
					setState3();
					transmit_info(state);
					state=0x04;
				}else{
					setState4();
					transmit_info(state);
					state=0x01;
				}
			}
		}
}