/*
 * Send and receive address test. Uses average of last 3 data bytes.
 * Author : Alec Wallace
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart_ATmega1284.h"

void delay_ms(int miliSec) { //for 8 Mhz crystal
	int i,j;
	for(i=0;i<miliSec;i++) {
		for(j=0;j<775;j++) {
			asm("nop");
		}
	}
}

volatile unsigned char TimerFlag = 0;


typedef struct task {
	int state; // Task's current state
	unsigned long period; // Task period
	unsigned long elapsedTime; // Time elapsed since last task tick
	int (*TickFct)(int); // Task tick function
} task;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;
unsigned long tasksPeriod = 1;
task tasks[1];
const unsigned short tasksNum = 1;

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1=0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}


void TimerISR() {
	unsigned char i;
	for (i = 0; i < tasksNum; ++i) {
		if ( tasks[i].elapsedTime >= tasks[i].period ) {
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasksPeriod;
	}
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
			unsigned char temp2;
			unsigned char temp1;
			unsigned char temp3;
			
			unsigned char lastOne = 0x10;
			unsigned char lastTwo = 0x01;
			
			unsigned char routingTable[256];
			unsigned int routingTableSize = 0;

unsigned char network = 0xAA;

unsigned char nodeAddress = 0x99;
unsigned int timeToWait = 1000;
unsigned char timeToWaitFlag = 0x00;

unsigned char counter1 = 0x01;
unsigned char counter = 0x00;
unsigned char received[256];
unsigned char receivedLength = 0x00;
unsigned char packet[256];
unsigned char packetLength = 0x00;
unsigned char sendAddressUpdate = 0x00;
unsigned char sendPacket = 0x00;
unsigned int packetsSent = 0;
unsigned int sendTimer = 0;
unsigned int waitTimer = 0;
unsigned int numPacketsToSend = 50;
//unsigned char sendAddress[2] = {network,nodeAddress};

enum recive_state{receive, done, wait, load, send, returnData};
int TickFct_check(int state){
	switch(state){
		case receive:
			temp3 = ~PINB;
			if (temp3 == 0x01){
				state = load;
			}
			else if(waitTimer >= timeToWait){
				waitTimer = 0;
				timeToWaitFlag = 0x00;
				state = load;
			}
			else{
				state = receive;
			}
		break;
		case done:
			//PORTD = 0x00;
			state = done;
		break;
		case wait:
			state = wait;
		break;
		case load:
			UCSR0B |= (1 << TXEN0);
			UCSR0B &= ~(1 << RXEN0);
			sendPacket = 0x00;
			sendAddressUpdate = 0x01;
			state = send;
		break;
		case send:
			if(packetsSent > numPacketsToSend){
				//PORTA = 0xAA;
				packetsSent = 0;
				//sendAddressUpdate = 0x00;
				sendPacket = 0x00;
				sendAddressUpdate = 0x00;
				counter = 0x00;
				state = receive;
				while(!USART_HasTransmitted(0)){
					;
				}
				UCSR0B &= ~(1 << TXEN0);
				PORTD &= 0xFD;
				UCSR0B |= (1 << RXEN0);
				//PORTD = 0x00;
			}
			else{
				state = send;
			}
		break;
		case returnData:
			state = wait; //not positive on this
		break;
	}	
	switch(state){
		case receive:
			if(timeToWaitFlag == 0x01){
				waitTimer++;
			}
			if(USART_HasReceived(0)){
				unsigned char temp0 = USART_Receive(0);
				//PORTA = temp0;
				//USART_Flush(0);
				if (temp0 == nodeAddress){
				while(!USART_HasReceived(0)){
					;
				}
				unsigned char temp1 = USART_Receive(0);
			
				/*while(!USART_HasReceived(0)){
					;
				}*/
				//unsigned char temp2 = USART_Receive(0);
				//if (temp0 == 0xAA){
					//if(temp1 == 0xAA){
						/*while(!USART_HasReceived(0)){
							;
						}
						unsigned char temp1 = USART_Receive(0);*/
						//temp1 = 0xFF;
					//}
					//state = done;
					//PORTC = temp2;
					//received[receivedLength] = temp1;
					PORTA = temp1;
					
					counter1++;
					receivedLength++;
					//if(counter >= 256){//or if num packets == 4
						//PORTA = 
					//}
				}
				if (temp0 == network){
					timeToWaitFlag = 0x01;
					while(!USART_HasReceived(0)){
						;
					}
					unsigned char temp1 = USART_Receive(0);
					PORTA = temp1;
					//received[receivedLength] = temp1;
					//waitTimer = 0x00;
					if (lastOne == temp1){
						int k;
						for(k = 0; k < routingTableSize; k++){
							if(routingTable[k] == temp1){
								timeToWaitFlag = 0;
							}
						}
						if(timeToWaitFlag == 0x01){
							routingTable[routingTableSize] = temp1;
							routingTableSize++;
						}
						lastTwo = lastOne;
						lastOne = temp1;
					}
					else{
						lastTwo = lastOne;
						lastOne = temp1;
					}
					//sendAddressUpdate = 0x01;
					//receivedLength++;
				}
			break;
			case done:
				;
			break;
			case wait:
				;
			break;
			case load:
				;
			break;
			case send:
				if(sendTimer >= 10){
					sendTimer = 0;
					if(USART_IsSendReady(0)){
						PORTA = nodeAddress;
						if(sendAddressUpdate == 0x01){
							USART_Send(network,0);
							while(!USART_IsSendReady(0)){
								;
							}
							USART_Send(nodeAddress,0);
							while(!USART_IsSendReady(0)){
								;
							}
							USART_Send(0x00,0);
							packetsSent++;
						}
						else if(sendPacket == 0x01){
							USART_Send(packet[0],0);
							while(!USART_IsSendReady(0)){
								;
							}
							USART_Send(packet[counter],0);
							counter++;
							while(!USART_IsSendReady(0)){
								;
							}
							USART_Send(0x00,0);
						}
						else{
							USART_Send(0xCC,0);
							while(!USART_IsSendReady(0)){
								;
							}
							USART_Send(counter,0);
							PORTA = counter;
							counter++;
							while(!USART_IsSendReady(0)){
								;
							}
							USART_Send(0x00,0);
						}
					}
				}
				sendTimer++;
			break;
			case returnData:
				;
			break;
				
		}
	}
	return state;
}
/*unsigned char counter = 0x00;
enum send_state {send};
int TickFct_send(int state) {

	switch(state) {
		case send:
		state = send;
		break;
	}
	switch(state) {
		case send:
		if(USART_IsSendReady(0)){
			USART_Send(0xAA,0);
			while(!USART_IsSendReady(0)){
				;	
			}
			USART_Send(counter,0);
			counter++;
			while(!USART_IsSendReady(0)){
				;
			}
			USART_Send(0x00,0);*/
			/*while(!USART_IsSendReady(0)){
				;
			}
			USART_Send(0x01,0);
			while(!USART_IsSendReady(0)){
				;
			}
			USART_Send(0x11,0);*/
			/*unsigned char counter = 0x01;
			for(counter = 0x01; counter <= 0x05; counter++){
				while(!USART_IsSendReady(0)){
					;
				}
				USART_Send(counter,0);
			}*/
		/*}
	}
	return state;
}*/

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00; // out
	DDRB = 0x00; PORTB = 0xFF; // In
	DDRC = 0xFF; PORTC = 0x00; // Out
	DDRD = 0xFF; PORTD = 0x00; // Out
	initUSART(0);
	//initUSART(1);
    /* Replace with your application code */

	unsigned char i = 0;
	
	tasks[i].state = load;
	tasks[i].period = 1;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &TickFct_check;
	
	/*i++;
	tasks[i].state = send;
	tasks[i].period = 10;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &TickFct_send;*/
	
	TimerSet(tasksPeriod);
	TimerOn();
	
	while(1){
		//PORTA = ~PINB;
		;
	}
}

