#ifndef ULTRALCD_H
#define ULTRALCD_H

#include "gsender.h"

#define GCODE_G1			"G1"
#define GCODE_M3			"M3"
#define GCODE_AUTO_HOME	"G0 X0 Y0 Z0 \n"
#define GCODE_ZEROPOS		"G92 X0 Y0 Z0\n"
#define GCODE_SPINDLE		"$32=0\n"
#define GCODE_LASER			"$32=1\n"

#define GCODE_G90			"G90\n"
#define GCODE_G91			"G91\n"
#define GCODE_XYHOME		"G00 X0 Y0\n"
#define GCODE_WLIGHT_ON	"M3 S1000\n"
#define GCODE_WLIGHT_OFF	"M3 S0\n"

#define CMD_STATUS_REPORT '?'
#define CMD_CYCLE_START 	'~'
#define CMD_FEED_HOLD	 	'!'

#define LEVEL0_MENU		0
#define LEVEL1_MENU		1
#define LEVEL2_MENU		2
#define LEVEL3_MENU		3
#define LEVEL4_MENU		4
#define LEVEL5_MENU		5

#define INFO_TIMEOUT	1000*10	//10s
#define UPDATE_INFOMENU_TIME	1000		//1000ms
#define POSBUF_SIZE				8

//buzzer
const int buzzer_pin = 16;	//PC2

typedef struct {
	int x;
	int y;
	int z;
}position_t;

extern uint8_t menu_level;
extern char thisfile_name[16];
extern float progress;
extern unsigned long info_timeout;

void ultralcd_init(void);
void ultralcd_update_menu(void);
void ultralcd_update_infomenu(char *xpos, char *ypos, char *zpos, float p);
void ultralcd_back_infomenu(void);

#endif
