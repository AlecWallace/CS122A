/* 
 * FinalProjectTranceiver.c
 * RF Controller receiver for 256 byte loss test.
 * Author : Alec Wallace
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart_ATmega1284.h"
#include "lcd.h"


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
unsigned char K = 0x73;
unsigned int error = 0;
unsigned char STRING[] = "Lost Bytes:    "; //12,13,14 change
unsigned char temp;
unsigned char network = 0xAA;
unsigned char nodeAddress = 0xCC;//change for each
unsigned char counter1 = 0x01;
unsigned char counter = 0x00;
unsigned char received[256];
unsigned int receivedLength = 0;
unsigned int receivedCounter = 0;
unsigned char packet[256];
unsigned int packetLength = 0;
unsigned char receiving = 0x00;
unsigned char sendAddressUpdate = 0x00;
unsigned char sendPacket = 0x00;
unsigned char sendAck = 0x00;
unsigned int packetsSent = 0;
unsigned int sendTimer = 0;
unsigned int waitTimer = 0;//should be unused
unsigned int numAutoSend = 50;
unsigned int numPacketsToSend = 0;//sent by master
unsigned int timeToWait = 250;//numPacketsToSend*10
unsigned int gotBadCount = 0;
//unsigned char sendAddress[2] = {network,nodeAddress};

enum recive_state{receive, done, wait, load, send, returnData};
int TickFct_check(int state){
	switch(state){
		case receive:
			if(USART_HasReceived(1)){
				temp = USART_Receive(1);
				if(temp == 0x55){//ack
					sendAck = 0x01;
					state = load;
				}
				else if(temp == 0xAA){//address update
					sendAddressUpdate = 0x01;
					state = load;
				}
				else if(temp == 0x00){//do nothing
					state = receive;
				}
				else{
					USART_Send(0xFF,1);
					state = wait;
				}
			}
			state = receive;
		break;
		case done:
			//PORTD = 0x00;
			state = done;
		break;
		case wait:
			temp = ~PINB;
			if(temp == 0x01){
				PORTA = 0xBB;
				//state = receive;
			}
			state = wait;
		break;
		case load:
			UCSR0B |= (1 << TXEN0);
			UCSR0B &= ~(1 << RXEN0);
			state = send;
		break;
		case send:
			if((sendPacket == 0x01 && packetsSent >= numPacketsToSend) || ((sendAck == 0x01 || sendAddressUpdate == 0x01) && packetsSent >= numAutoSend)){
				packetsSent = 0;
				sendAddressUpdate = 0x00;
				sendAck = 0x00;
				sendPacket = 0x00;
				while(!USART_HasTransmitted(0)){
					PORTA = 0xFF;
				}
				UCSR0B &= ~(1 << TXEN0);
				PORTD &= 0xFD;
				UCSR0B |= (1 << RXEN0);
				//PORTD = 0x00;
				state = receive;
			}
			else{
				state = send;
			}
		break;
		case returnData:
			state = returnData; //once data is returned go to wait
		break;
	}	
	switch(state){
		case receive:
			if(USART_HasReceived(0)){
				unsigned char temp0 = USART_Receive(0);
				//PORTA = temp0;
				//USART_Flush(0);
				if (temp0 == nodeAddress){
					received[0] = nodeAddress;
					receivedLength++;
					while(!USART_HasReceived(0)){
						;
					}
					unsigned char temp1 = USART_Receive(0);

					received[receivedLength] = temp1;
					PORTA = temp1;
					
					counter1++;
					//error += temp1-counter;
					gotBadCount = 0;
					if(receivedLength >= 10){
						receiving = 0x01;
					}
					if(receivedLength >= 256){//or if num packets == 4
						PORTA = 0xFF;
						STRING[12] = ' ';
						STRING[13] = ' ';
						STRING[14] = '0';
						LCD_DisplayString(1,STRING);
						state = returnData;
					}
				}
				else if (temp0 == network){
					while(!USART_HasReceived(0)){
						;
					}
					unsigned char temp1 = USART_Receive(0);
					temp1 = temp1 ^ K;
					PORTA = temp1;
					received[0] = temp0;
					received[1] = temp1;
					state = returnData;
					receivedLength = 2;
					gotBadCount = 0;
				}
				else if(receiving == 0x01){
					gotBadCount++;
				}
				if(gotBadCount >= 400){
					gotBadCount = 0;
					error = 256 - counter1;
					STRING[12] = '0' + (int)(error/100);
					error = error%100;
					STRING[13] = '0' + (int)(error/10);
					error = error%10;
					STRING[14] = '0' + (int)(error);
					LCD_DisplayString(1,STRING);
					PORTA = 0x22;
					counter1 = 0;
					
					state = wait;
				}
			}
		break;
		case done:
			;//do nothing
		break;
		case wait:
			;
		break;
		case load:
			;//get send ready
		break;
		case send:
			if(sendTimer >= 10){
				sendTimer = 0;
				if(USART_IsSendReady(0)){
					packetsSent++;
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
					}
					else if(sendAck == 0x01){
						USART_Send(0x55,0);
						while(!USART_IsSendReady(0)){
							;
						}
						USART_Send(0x55,0);
					counter++;
						while(!USART_IsSendReady(0)){
							;
						}
						USART_Send(0x00,0);
					}
					else{//(sendPacket == 0x01)
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
					sendTimer++;
				}
			}
		break;
		case returnData:
			//give data back to master
			while(receivedCounter < receivedLength){
				while (!USART_IsSendReady(1)){
					;
				}
				USART_Send(received[receivedCounter],1);
				receivedCounter++;
			}
			receivedCounter = 0;
			counter1 = 0;//not used rn
			receivedLength = 0;
			state = wait;
		break;
				
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
	DDRB = 0x00; PORTB = 0xFF; // in
	DDRC = 0xFF; PORTC = 0x00; // Out
	DDRD = 0xFF; PORTD = 0x00; // Out
	initUSART(0);
	//initUSART(1);
	LCD_init();
	LCD_DisplayString(1,STRING);
    /* Replace with your application code */

	unsigned char i = 0;
	
	tasks[i].state = receive;
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
		;
	}
}

