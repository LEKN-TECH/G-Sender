#include "sdcard_opt.h"

/*
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 10 
 */

#define SDPIN_CS		10	
//#define SDPIN_DETECT 15	// PC1

uint8_t sdcard_state = SDSTATE_NO;

uint8_t sd_detectpin	= 0;
uint8_t old_sd_detectpin = 0;


File root;
uint8_t sdcard_init(void)
{
	pinMode(SDPIN_CS, OUTPUT);

	if(digitalRead(SDPIN_DETECT) == 1){
		//Serial.println("no sdcard!");
		return 1;
	}

	if (!SD.begin(SDPIN_CS)) {
		//Serial.println("initialization failed!");
		return 1;
	}
	//Serial.println("initialization done.");
	root = SD.open("/");

	return 0;
}


uint8_t sdcard_filenum(void)
{
	uint8_t filenum = 0;
	while(true) { 
		File entry =  root.openNextFile();
		if (! entry) {
			// no more files
			 root.rewindDirectory();
			break;
		}
		if (!entry.isDirectory()) {
			if(filenum == 0xff)
				return filenum;
			filenum++;
		}
		entry.close();
	}
	return filenum;
}

