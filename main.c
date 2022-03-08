#include<stm32f446xx.h>
#include "uart.h"

int main() {
	uart2_tx_rx_init();
	uart6_tx_rx_init();
	uart2_txe_rxne_interrupt_enable();
	uart6_txe_rxne_interrupt_enable();
	
	while(1) {
		
	}

}








