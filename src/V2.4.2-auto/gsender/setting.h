#ifndef SETTING_H
#define SETTING_H

#include "gsender.h"

#define ADDR_SETTING_FLAG	0x0
#define ADDR_BAUD			0x1			
#define ADDR_BUZZER	0x2
#define ADDR_AUTORUN		0x3

#define SETTING_FLAG_DEFAULT	0xfe
#define BAUD_DEFAULT			BAUD_115200
#define BUZZER_DEFAULT			_YES
#define AUTORUN_DEFAULT		0

#define BAUD_9600		0
#define BAUD_19200		1
#define BAUD_38400		2
#define BAUD_57600		3
#define BAUD_115200		4


#define _NO	0
#define _YES 1

extern int8_t autorun;
extern long baud;
extern uint8_t menu_buzzer;

void get_setting(void);
void save_setting(uint8_t address, uint8_t value);
int8_t  setting_grblmode(int8_t mode);

#endif
