#include <EEPROM.h>

#include "setting.h"

long baud;
uint8_t menu_buzzer;


void get_setting(void)
{
	uint8_t setting_flag;
	
	setting_flag = EEPROM.read(ADDR_SETTING_FLAG);
	if(setting_flag == SETTING_FLAG_DEFAULT){
		baud = EEPROM.read(ADDR_BAUD);
		menu_buzzer = EEPROM.read(ADDR_BUZZER);
		autorun = EEPROM.read(ADDR_AUTORUN);
	}else {
		EEPROM.write(ADDR_SETTING_FLAG, SETTING_FLAG_DEFAULT);
		EEPROM.write(ADDR_BAUD, BAUD_DEFAULT);
		EEPROM.write(ADDR_BUZZER, BUZZER_DEFAULT);
		EEPROM.write(ADDR_AUTORUN, AUTORUN_DEFAULT);
		baud = BAUD_DEFAULT;
		menu_buzzer = BUZZER_DEFAULT;
	}
}

void save_setting(uint8_t address, uint8_t value)
{
	EEPROM.write(address, value);
}
