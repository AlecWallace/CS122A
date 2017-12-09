//from http://www.raspberry-projects.com/pi/programming-in-c/uart-serial-port/using-the-uart
//and help from Michael Evans

//Very simple program, effectively a button press. Sends a byte across UART to tell the microcontroller
//to run the throughput test with bursty data.
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART

int uart0_filestream = -1;

void USART_init(){
  uart0_filestream = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);
	if (uart0_filestream == -1)
	{
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
    exit(1);
	}
  struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = 0;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);
}

void transmit(unsigned char* buf, unsigned char count){
    int tx_length = write(uart0_filestream, buf, count));
		if (tx_length < 0)
		{
			printf("UART TX error\n");
		}
}

void receive(unsigned char* buf, unsigned char count){
    int rx_length = read(uart0_filestream, (void*)buf, count));
		if (rx_length < 0)
		{
			printf("UART TX error\n");
      buf[0] = '\0';
		}
    else{
      buf[rx_length] = '\0';
    }
}

int main(){
  USART_init();
  unsigned char buff[] = {0x02};
  unsigned char size = 0x01;
  transmit(buf,size);
}
