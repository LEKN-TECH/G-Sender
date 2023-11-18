#include "gsender.h"

#include "cmd_cache.h"

#include "encoder.h"
#include "ultralcd.h"
#include "sdcard_opt.h"
#include "setting.h"

#define SERIAL_NO_DATA -1

uint8_t sys_state =  STATE_IDLE;

File thisfile_fd;

static uint8_t get_cmdline(char *buffer, uint8_t buffer_len)
{
	char temp;
	uint8_t i = 0;

	uint8_t invalid_line;
	do{
		if(thisfile_fd.available()) {
			temp = thisfile_fd.peek();
			if(temp == '('){
				invalid_line = 1;
				//Serial.println("FOUND");
				while(thisfile_fd.size() - thisfile_fd.position() > 0){
					if(thisfile_fd.read() == '\n')
						break;
				}
			}else{
				invalid_line = 0;
			}
		}else {
			invalid_line = 0;
		}	
	}while(invalid_line);


	if(thisfile_fd.position() + buffer_len > thisfile_fd.size())
	    buffer_len = thisfile_fd.size() - thisfile_fd.position();
	if(buffer_len == 0)
	    return 0;
	
	for(i = 0; i < buffer_len; i++){
		if(thisfile_fd.available()) {
			buffer[i] = thisfile_fd.read();
			if(buffer[i] == '\n') //  0x0d:\r  0x0a:\n
				return i + 1;
		}
	}
	return buffer_len;
}


void write_cmd_to_cache()
{
	static uint8_t w_state = 0, w_cmd_len = 0;
	static uint8_t w_cmd[CMD_SIZE];

	if(thisfile_fd.position() == thisfile_fd.size()){
		return;
	}
	if(cmdcache_state(CMD_WRITE) != CMDCACHE_FULL){
		memset(w_cmd, 0, CMD_SIZE);
		w_cmd_len = get_cmdline(w_cmd, CMD_SIZE);	/*	w_cmd_len include '\r' '\n'	*/
		//Serial.print("w_cmd_len:");
		//Serial.println(w_cmd_len);
		//Serial.print("CMD:");
		//Serial.write(w_cmd, w_cmd_len);
		cmdcache_wirte(w_cmd, w_cmd_len);
	}
	//w_state = cmdcache_wirte(w_cmd, w_cmd_len);
}


static char xpos[POSBUF_SIZE] = {"0.0"}, ypos[POSBUF_SIZE] = {"0.0"}, zpos[POSBUF_SIZE] = {"0.0"};
void getposinfo(char *buffer, uint8_t len)
{
	uint8_t i, j, type = 0;

	if(buffer == NULL)
		return;		
	for(i = 0; i < len;){
		if(i >= 2 && buffer[i - 2] == '.' ){
			type = 0;
		}		 
		if(buffer[i] == 'X' || buffer[i] == 'x'){
			memset(xpos, 0, POSBUF_SIZE);
			type = 1;
			i++;
			j = 0;
		}else if(buffer[i] == 'Y' || buffer[i] == 'y'){
			memset(ypos, 0, POSBUF_SIZE);
			type = 2;
			i++;
			j = 0;
		}else if(buffer[i] == 'Z' || buffer[i] == 'z'){
			memset(zpos, 0, POSBUF_SIZE);
			type = 3;
			i++;
			j = 0;
		}else if(buffer[i] == ' ' || buffer[i] == 'S' || buffer[i] == 'F'){
			type = 0;
		}
		
		switch(type){
			case 0:
				i++;
				break;
			case 1:
				xpos[j] = buffer[i];
				i++; j++;
				break;
			case 2:
				ypos[j] = buffer[i];
				i++; j++;
				break;
			case 3:
				zpos[j] = buffer[i];
				i++; j++;
				break;			
		}
	}
//	Serial.println("POS:");
//	Serial.println(len);
//	Serial.println(xpos);
//	Serial.println(ypos);
//	Serial.println(zpos);	
}

void setup() {
	Serial.begin (115200);
	get_setting();
	switch(baud){
		case BAUD_9600: Serial.begin(9600L); break;
		case BAUD_19200: Serial.begin(19200L); break;
		case BAUD_38400: Serial.begin(38400L); break;
		case BAUD_57600: Serial.begin(57600L); break;
		case BAUD_115200: Serial.begin(115200L); break;
		default: Serial.begin(115200L); break;
	}		
	encoder_init();
	sdcard_state = sdcard_init();
	ultralcd_init();
	//Serial.println("@@@@");
	
}

void loop(){
	uint8_t cmdlen = 0;
	uint8_t r_cmd[CMD_SIZE];
	uint8_t posinfo[POSBUF_SIZE*3];
	char c;
	uint8_t sendcmd_flag;
	static unsigned long updateinfo_time = 0;
	static unsigned long autorun_timer = 0;
	static uint8_t autorun_timer_flag = 0, autorun_timer_begin = 0;
	static unsigned long get_grblstate_timer = 0;

	if(sys_state == STATE_IDLE){
		if(encoder_changed){
			encoder_changed = 0;
			ultralcd_update_menu();
		}else{
			if((millis() - info_timeout) > INFO_TIMEOUT){
				//ultralcd_back_infomenu();
			}
		}
	}
	if(autorun_timer_flag){
		if(autorun_timer_begin){
			if((millis() - get_grblstate_timer) > 1000){
				Serial.print(CMD_STATUS_REPORT);
				delay(10);
				get_grblstate_timer = millis();
				 if (Serial.available() > 0){
					String ss = Serial.readString();
					if(ss.indexOf("Idle") != -1 ){
						autorun_timer_begin = 0;
						autorun_timer = millis();
						//Serial.print("@1@:");
						//Serial.println(autorun_timer);
					}	
				 }	
			}
		}else{
			//Serial.print("@2@:");Serial.print(millis());Serial.print("++");Serial.println(autorun_timer);
				
			if((millis() - autorun_timer) > autorun* 1000){
				tone(buzzer_pin, 5000, 50);
				delay(500);
				tone(buzzer_pin, 5000, 50);
				delay(500);
				tone(buzzer_pin, 5000, 50);
				autorun_timer_flag = 0;
				sys_state = STATE_CNC;
			}
		}

	}
	
	if(sys_state == STATE_CNC){
		thisfile_fd = SD.open(thisfile_name);
		//Serial.print("@@@:");
		//Serial.println(thisfile_name);
		cmdcache_flush();
		sendcmd_flag = 1;
		//MsTimer2::start();  
		while(1){
			if(sys_state == STATE_CNC_PAUSE){
				while(1){
					if(encoder_changed){
						encoder_changed = 0;
						ultralcd_update_menu();
					}
					if(sys_state == STATE_CNC)
						break;
				}
			}else if(sys_state == STATE_CNC_STOP){
				autorun_timer_flag = 0;
				sys_state = STATE_IDLE;
				//Serial.print(CMD_FEED_HOLD);
				//Serial.print(GCODE_WLIGHT_OFF);
				//Serial.print("G0 Z5");
				//Serial.print(GCODE_XYHOME);
				//Serial.print("G0 Z0");
				menu_level = LEVEL0_MENU;
				progress = 0;
				ultralcd_update_menu();
				break;
			}
			if(encoder_changed){
				encoder_changed = 0;
				ultralcd_update_menu();
			}
			if(menu_level == LEVEL0_MENU && (millis() - updateinfo_time) > UPDATE_INFOMENU_TIME){					
				ultralcd_update_infomenu(xpos, ypos, zpos, ((float)thisfile_fd.position() / (float)thisfile_fd.size()) * 100);
				updateinfo_time = millis();
			}	
			write_cmd_to_cache();
			if(sendcmd_flag == 1){
				sendcmd_flag = 0;
				cmdlen = cmdcache_read(r_cmd);
				if(cmdlen != CMDCACHE_NO_DATA){
					Serial.write(r_cmd, cmdlen);
					getposinfo(r_cmd, cmdlen - 2);	// sub '\r' '\n'
				}else{
					ultralcd_update_infomenu(xpos, ypos, zpos, ((float)thisfile_fd.position() / (float)thisfile_fd.size()) * 100);
					//Serial.print("NO_DATA\n");					
				}					
			}
			unsigned long temp = millis();
			//while((c = Serial.read()) != SERIAL_NO_DATA){
			static uint8_t test_num = 0;
			while(1){
				//c = Serial.read();
				//Serial.print(">>>>>>");
				//Serial.println(c);
				//c1 = Serial.read();
				c = Serial.read();
				if(c == 'k'){	//rec:	ok
					test_num++;
					//sendcmd_flag = 1;
					//break;
				}
				if(c == ':'){	//rec: error:xxx
					test_num++;
				}
				if(test_num == 2){			// \r' or'\n'  
					test_num = 0;
					sendcmd_flag = 1;
					break;
				}
				if((millis() - temp) > 20)
					break;
			}			
			if(thisfile_fd.position() == thisfile_fd.size() && cmdlen == CMDCACHE_NO_DATA){
				if(autorun){
					autorun_timer_flag = 1;
					autorun_timer_begin = 1;
					
				}
				break;
			}
		}
		sys_state = STATE_IDLE;
		thisfile_fd.close();
		
	}
	
}


	

 
