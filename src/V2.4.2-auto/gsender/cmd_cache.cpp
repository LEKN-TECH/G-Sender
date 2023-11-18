#include "cmd_cache.h"

cmd_t cmdcache[CMDCACHE_RING];
uint8_t cmdcache_head = 0;
volatile uint8_t cmdcache_tail = 0;

uint8_t cmdcache_wirte(uint8_t *cmd, uint8_t len)
{
	// Calculate next head
	uint8_t next_head = cmdcache_head + 1;
	if (next_head == CMDCACHE_RING) { 
		next_head = 0; 
	}
	// Write data to buffer unless it is full.
	if (next_head != cmdcache_tail) {
		/*
		Serial.print("W:");
		Serial.print(len);
		Serial.print(" CMD:");
		Serial.write(cmd, len);
		*/
		cmdcache[cmdcache_head].cmd_len = len;
		memset(&cmdcache[cmdcache_head].cmd[0], 0xff, CMD_SIZE);
		memcpy(&cmdcache[cmdcache_head].cmd[0], cmd, len);
		cmdcache_head = next_head;
		
		return 0;
	}else{
		
		return CMDCACHE_FULL;
	}
}


uint8_t cmdcache_read(uint8_t *cmd)
{
	uint8_t len = 0;
	uint8_t tail = cmdcache_tail; // Temporary serial_rx_buffer_tail (to optimize for volatile)

	if (cmdcache_head == tail) {
		return CMDCACHE_NO_DATA;
	} else {
		len = cmdcache[tail].cmd_len;
		memcpy(cmd, &cmdcache[tail].cmd[0], len);
		/*
		Serial.print("R:");
		Serial.print(len);
		Serial.print(" CMD:");
		Serial.write(cmd, len);
		*/
		tail++;
		if (tail == CMDCACHE_RING) {
			tail = 0; 
		}
		cmdcache_tail = tail;
		
		return len;
	}
}

void cmdcache_flush(void)
{
	cmdcache_head = 0;
	cmdcache_tail = 0;
}

uint8_t cmdcache_state(uint8_t flag)
{
	uint8_t state = CMCACHE_NORMAL;
	
	if(flag == CMD_WRITE){
		uint8_t next_head = cmdcache_head + 1;
		if (next_head == CMDCACHE_RING) { 
			next_head = 0; 
		}
		if (next_head != cmdcache_tail) {
			state = CMCACHE_NORMAL;
		}else{
			state = CMDCACHE_FULL;
		}
	}else if(flag == CMD_READ){
		uint8_t tail = cmdcache_tail;
		if (cmdcache_head == tail) {
			state = CMDCACHE_NO_DATA;
		}else{
			state = CMCACHE_NORMAL;
		}	
	}
	return state;
}
