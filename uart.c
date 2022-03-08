#include "uart.h"

static char data1[100];
static int size1 = 100;
static int head1 = 0, tail1 = 0;

static char data2[100];
static int size2 = 100;
static int head2 = 0, tail2 = 0;

void uart2_tx_rx_init() {
	// enable clock to GPIOA
	RCC-> AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	// PA2, PA3 mode to alternate functions, 
	GPIOA-> MODER |= (0x2 << 4) | (0x2 << 6);
	// alternate function type to AF7 = 0111 for PA2 and PA3 
	GPIOA-> AFR[0] |= (0x7 << 8) | (0x7 << 12);
	
	// enable clock to usart2
	RCC-> APB1ENR |= RCC_APB1ENR_USART2EN;
	// Buadrate 9600 -> 0x683, 115200 -> 0x8B when periphClk = 16 MHz
	USART2-> BRR |= 0x8B;
	// usart2 enable, tx enable, rx enable
	USART2-> CR1 |= (1<<13) | (1<<3) | (1<<2);
	// enable uart2 interrupt in NVIC
	NVIC_EnableIRQ(USART2_IRQn);
}

void uart6_tx_rx_init() {
	// enable clock to GPIOC
	RCC-> AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	// PA6, PA7 mode to alternate functions, 
	GPIOC-> MODER |= (0x2 << 12) | (0x2 << 14);
	// alternate function type to AF8 = 1000 for PA6 and PA7 
	GPIOC-> AFR[0] |= (0x8U << 24) | (0x8U << 28);
	
	// enable clock access to USART6
	RCC-> APB2ENR |= RCC_APB2ENR_USART6EN;
	// baudrate 115200 -> BRR = 0x8B, 9600 -> BRR = 0x683 
	USART6-> BRR |= 0x8B;
	// uart enable, rx enable, tx enable
	USART6-> CR1 |= (1<<13) | (1<<3) | (1<<2);
	// enable uart6 interrupt in NVIC
	NVIC_EnableIRQ(USART6_IRQn);
}

void uart2_txe_rxne_interrupt_enable() {
	// usart2 TXE and RXNE interrupt enable
	USART2-> CR1 |= (1 << 7) | (1 << 5);
}
void uart2_txe_interrupt_enable() {
	// usart2 TXE interrupt enable
	USART2-> CR1 |= (1 << 7);
}
void uart2_txe_interrupt_disable() {
	// usart2 TXE interrupt disable 
	USART2-> CR1 &= ~(1U << 7);
}
void uart2_rxne_interrupt_enable() {
	// usart2 RXNE interrupt enable
	USART2-> CR1 |= (1 << 5);
}
void uart2_rxne_interrupt_disable() {
	// usart2 RXNE interrupt disable 
	USART2-> CR1 &= ~(1U << 5);
}
void uart6_txe_rxne_interrupt_enable() {
	// usart6 TXE and RXNE interrupt enable
	USART6-> CR1 |= (1 << 7) | (1 << 5);
}
void uart6_txe_interrupt_enable() {
	// usart6 TXE interrupt enable
	USART6-> CR1 |= (1 << 7);
}
void uart6_txe_interrupt_disable() {
	// usart6 TXE interrupt disable 
	USART6-> CR1 &= ~(1U << 7);
}
void uart6_rxne_interrupt_enable() {
	// usart6 RXNE interrupt enable
	USART6-> CR1 |= (1 << 5);
}
void uart6_rxne_interrupt_disable() {
	
	// usart6 RXNE interrupt disable 
	USART6-> CR1 &= ~(1U << 5);
}

// ISR for UART2 TXE and RXNE interrupt

void USART2_IRQHandler() {
	// if interrupt is for TXE and TXEIE is enabled 
	if ((USART2-> SR & 1<<7) && (USART2-> CR1 & 1<<7)) {
		uart2_tx_callback();
	}
	// else if interrupt is for RXNE
	if (USART2-> SR & 1<<5) {
		uart2_rx_callback();
	}
}

// ISR for UART6 TXE and RXNE interrupt 
void USART6_IRQHandler() {
	// if interrupt is for TXE and TXEIE is enabled 
	if ((USART6-> SR & 1<<7) && (USART6-> CR1 & 1<<7)) {
		uart6_tx_callback();
	}
	// else if interrupt is for RXNE
	if (USART6-> SR & 1<<5) {
		uart6_rx_callback();
	}
	
}

// uart2_rx -> uart6_tx
// uart6_rx -> uart2_tx 

void uart2_rx_callback() {
	// buffer full, cant insert, data lost.
	if ((head1+1)%size1 == tail1) return;
	// insert byte into buffer
	data1[head1++] = (char)USART2-> DR;
	if (head1 == size1) head1 = 0;
	// data in, enable txe interrupt for uart6
	uart6_txe_interrupt_enable();
}

void uart6_tx_callback() {
	// if buffer empty, disable TXE interrupt and exit
	if (tail1 == head1) {
		uart6_txe_interrupt_disable();
		return;
	}
	// transmit byte from buffer
	char c = data1[tail1++];
	USART6-> DR = c;
	if (tail1 == size1) tail1 = 0;
}

void uart6_rx_callback() {
	// buffer full, cant insert, data lost.
	if ((head2+1)%size2 == tail2) return;
	// insert byte into buffer
	data2[head2++] = (char)USART6-> DR;
	if (head2 == size2) head2 = 0;
	// data in, enable txe interrupt for uart2
	uart2_txe_interrupt_enable();
}

void uart2_tx_callback() {
	// if buffer empty, disable TXE interrupt and exit
	if (tail2 == head2) {
		uart2_txe_interrupt_disable();
		return;
	}
	// transmit byte from buffer
	USART2-> DR = data2[tail2++];
	if (tail2 == size2) tail2 = 0;
}

// uart6_rx -> uart6_tx 
/*
void uart6_tx_callback() {
	// if buffer empty, disable TXE interrupt and exit
	if (tail1 == head1) {
		uart6_txe_interrupt_disable();
		return;
	}
	
	// transmit byte from buffer
	USART6-> DR = data1[tail1++];
	if (tail1 == size1) tail1 = 0;
}

void uart6_rx_callback() {
	// buffer full, cant insert, data lost.
	if ((head1+1)%size1 == tail1) return;
	// insert byte into buffer
	char c = (char) USART6->DR;
	USART2->DR = c;
	data1[head1++] = c;
	if (head1 == size1) head1 = 0;
	// data in, enable txe interrupt for uart6
	uart6_txe_interrupt_enable();
}
*/ 

// uart2_rx -> uart2_tx
/*
void uart2_tx_callback() {
	// if buffer empty, disable TXE interrupt and exit
	if (tail1 == head1) {
		uart2_txe_interrupt_disable();
		return;
	}
	// transmit byte from buffer
	USART2-> DR = data1[tail1++];
	if (tail1 == size1) tail1 = 0;
}

void uart2_rx_callback() {
	// buffer full, cant insert, data lost.
	if ((head1+1)%size1 == tail1) return;
	// insert byte into buffer
	char c = (char) USART2->DR;
	data1[head1++] = c;
	if (head1 == size1) head1 = 0;
	// data in, enable txe interrupt for uart6
	uart2_txe_interrupt_enable();
}
*/

// synchronous byte transfer(without interrupt)
/*
void uart2_write(char c) {		
	while(!(USART2->SR & (1 << 7))) {}
	USART2-> DR = c;
}
char uart2_read() {
	while(!(USART2->SR & (1 << 5))) {}
	return (char) USART2-> DR;
}

void uart2_read_str() {
	uint32_t cnt = 0;
  char c = 0;
	while(cnt != 20 && c != '\n') {
		c = uart2_read();
		data[cnt++] = c;
	}
	if (cnt == 20) data[cnt++] = '\n';
	data[cnt++] = '\r';
	data[cnt++] = '\0';
}
void uart2_write_str() {
	char *s = data;
	while(*s != '\0') {
		uart2_write(*s);
		++s;
	}
}
*/

/*
void uart6_write(char c) {
	while(!(USART6-> SR & (1 << 7))) {}
	USART6-> DR = c;
}

char uart6_read() {
	while(!(USART6->SR & (1 << 5))) {}
	return (char) USART6-> DR;
}

void uart6_read_str() {
	uint32_t cnt = 0;
  char c = 0;
	while(cnt != 20 && c != '\n') {
		c = uart6_read();
		data[cnt++] = c;
	}
	if (cnt == 20) data[cnt++] = '\n';
	data[cnt++] = '\r';
	data[cnt++] = '\0';
}
void uart6_write_str() {
	char *s = data;
	while(*s != '\0') {
		uart6_write(*s);
		++s;
	}
}
*/
