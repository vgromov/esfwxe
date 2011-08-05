#ifndef _serial_config_h_
#define _serial_config_h_

#define USE_SER_PORT0				// use only uart 0
#define SER_TX_BUFFLEN 4 		// specify TX queues length
#define SER_RX_BUFFLEN 0x40	// specify RX queues length

#define SER_INTERRUPT_PRIORITY (LOWEST_PRIORITY)

typedef enum {
#ifdef USE_SER_PORT0
	ser0,
#endif

#ifdef USE_SER_PORT1
	ser1,
#endif

	serPortCnt

} serPort;

#endif
