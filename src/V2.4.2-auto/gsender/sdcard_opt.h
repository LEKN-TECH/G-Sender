#ifndef SDCARD_OPT_H
#define SDCARD_OPT_H

#include "gsender.h"

#define SDPIN_DETECT 15	// PC1

#define SDSTATE_NO	1
#define SDSTATE_YES	0

extern File root;
extern uint8_t sdcard_state;
extern uint8_t sd_detectpin;
extern uint8_t old_sd_detectpin;

uint8_t sdcard_init(void);
uint8_t sdcard_filenum(void);


#endif

