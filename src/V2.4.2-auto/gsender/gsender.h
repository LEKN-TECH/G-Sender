#ifndef GSENDER_H
#define GSENDER_H
#include <Arduino.h>
#include <SD.h>
//#include <MsTimer2.h> 

#define STATE_IDLE	0
#define STATE_CNC	1
#define STATE_CNC_PAUSE	2
#define STATE_CNC_STOP	3

#define VERSION	"G-Sender V2.4.2a"

extern uint8_t sys_state;

#endif
