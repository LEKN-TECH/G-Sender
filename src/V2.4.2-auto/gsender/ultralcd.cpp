#include "LiquidCrystal.h"
#include "ultralcd.h"
#include "language.h"
#include "encoder.h"

#include "sdcard_opt.h"
#include "setting.h"
#define LCD_WIDTH 16
#define LCD_HEIGHT 2

#define ENCODER_PULSES_PER_STEP	4

uint8_t old_menu_level = 0, menu_level= 0; 
int16_t encoder_line_num = 0, old_encoder_line_num  = 0;

uint8_t menu_page_header = 0;
uint8_t cursor_pos = 0;

uint8_t old_menu_page_header[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t old_cursor_pos[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t old_encoder_value[8] = {0, 0, 0, 0, 0, 0, 0, 0};

int16_t encoder_dir = 0;

uint8_t card_filenum = 0;
char thisfile_name[16];

position_t syspos, temp_syspos;
int32_t sp_speed = 0;
int8_t autorun = 0;
float progress = 0;
char syspos_x[POSBUF_SIZE];
char syspos_y[POSBUF_SIZE];
char syspos_z[POSBUF_SIZE];

uint8_t mtype[16] = {0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0};

unsigned long info_timeout = 0;

int8_t grbl_mode = -1;

// initialize the library with the numbers of the interface pins
const int rs = 9, en = 8, d4 =7, d5 = 6, d6 = 5, d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


#define LCD_STR_UPLEVEL     "\x01"
#define LCD_STR_ARROW_RIGHT "\x02"
#define LCD_STR_SELECT  	   "\x03"
//#define LCD_STR_FOLDER      "\x04"
//#define LCD_STR_REFRESH     "\x05"

#define MENU_TYPE1(s)	s & 0xff
#define MENU_TYPE2(s)	(s >> 8) & 0xff
#define MENU_TYPE3(s)	(s >> 16) & 0xff
#define MENU_TYPE4(s)	(s >> 24) & 0xff

#define CLEAR_MENU_ARG()		\
		encoder_value = 0;		\
		encoder_line_num = 0;	\
		menu_page_header = 0;	\
		cursor_pos = 0;			\
		encoder_dir 	= 0
#define SAVE_MENU_ARG(level)		\
		{	\
			old_menu_page_header[level] = menu_page_header;	\
			old_cursor_pos[level] = cursor_pos;	\
			old_encoder_value[level] = encoder_value;		\
		}

#define RESTORE_MENU_ARG(level)		\
		{	\
			menu_page_header = old_menu_page_header[level];	\
			cursor_pos = old_cursor_pos[level];	\
			encoder_value = old_encoder_value[level];	\
			old_encoder_line_num = encoder_value / ENCODER_PULSES_PER_STEP;	\
		}

		
#define SET_MENU_TYPE(type, level)	\
		mtype[level] = type
#define CLEAR_MENU_TYPE(level)	\
		mtype[level]= 0
#define GET_MENU_TYPE(level)	\
		mtype[level]

#define FLUSH_ULTRALCD()	\
			lcd.clear();		\
			draw_menu(menu_level)	\

#define 	BUZZER();	\
		if(menu_buzzer){	\
			tone(buzzer_pin, 5000, 50);	\
		}
		

#define MENU_POS0	0
#define MENU_POS1	1
#define MENU_POS2	2
#define MENU_POS3	3
#define MENU_POS4	4
#define MENU_POS5	5
#define MENU_POS6	6
#define MENU_POS7	7
#define MENU_POS8	8
#define MENU_POS9	9

#define BACK				MENU_POS0

/*********menu level1********/
#define CONTROL_MENU			MENU_POS1
#define SETTING_MENU			MENU_POS2
#define CARD_MENU				MENU_POS3
#define MAIN_MENU_MAX			MENU_POS4

#define CONTROL_COPY_MENU		MENU_POS1
#define PAUSE_RESUME_MENU		MENU_POS2
#define STOP_MENU				MENU_POS3
#define MAIN_COPY_MENU_MAX	MENU_POS4

/*********menu level2********/
// type0 control
#define	AUTO_HOME_MENU		MENU_POS1
#define	ZERO_POSITION_MENU 	MENU_POS2
#define	MOVE_AXIS_MENU 		MENU_POS3
#define	SPINDLE_SPEED_MENU	MENU_POS4
#define	GRBL_MODE_MENU		MENU_POS5
#define	CONTROL_MENU_MAX		MENU_POS6
// type1 setting
#define	AUTORUN_SETTING_MENU	MENU_POS1
#define	BAUD_SETTING_MENU		MENU_POS2
#define	BUZZER_SETTING_MENU		MENU_POS3
#define	SETTING_MENU_MAX			MENU_POS4
// type2 card
#define CARD_MENU_MAX 	(card_filenum + 1)

/*********menu level3********/
//type0 move 10mm 1mm 0.1mm
#define   MOVE_10MM_MENU 		MENU_POS1
#define	MOVE_1MM_MENU 		MENU_POS2	 
#define	MOVE_01MM_MENU 		MENU_POS3
#define	MOVE_AXIS_MENU_MAX	MENU_POS4
//type1
#define   GRBL_MODE_SPINDLE_MENU 	MENU_POS1
#define	GRBL_MODE_LASER_MENU	MENU_POS2	 
#define	GRBL_MODE_MENU_MAX		MENU_POS3
//type2
#define 	BAUD9600_MENU		MENU_POS1
#define 	BAUD19200_MENU	MENU_POS2
#define 	BAUD38400_MENU	MENU_POS3
#define 	BAUD57600_MENU	MENU_POS4
#define 	BAUD115200_MENU	MENU_POS5
#define 	BAUD_MENU_MAX	MENU_POS6
//type3
#define HOMINGDIR_MIN_MENU	MENU_POS1
#define HOMINGDIR_MAX_MENU	MENU_POS2
#define HOMINGDIR_MENU_MAX	MENU_POS3
//type4
#define YES_MENU			MENU_POS1
#define NO_MENU				MENU_POS2
#define SELECT_MENU_MAX	MENU_POS3

//#define	REST_GRBL_MENU		MENU_POS2

/*********menu level4********/
//type0 move x y z
#define	MOVE_X_MENU			MENU_POS1	
#define	MOVE_Y_MENU			MENU_POS2	 
#define	MOVE_Z_MENU			MENU_POS3
#define	MOVE_MM_MENU_MAX	MENU_POS4



#define SPEED	"F2000"
#define SPSPEED_STEP	1000

#define RATIO	10;
#define XPOS_MAX	200*RATIO;
#define YPOS_MAX	300*RATIO;
#define ZPOS_MAX	60*RATIO;

static void lcd_set_custom_characters(void)
{
	static byte uplevel[8] = {
		B00100,
		B01110,
		B11111,
		B00100,
		B11100,
		B00000,
		B00000,
		B00000
	};
	static byte right[8] = {
		B00000,
		B00100,
		B00010,
		B11111,
		B00010,
		B00100,
		B00000,
		B00000,
	};
	static byte select[8] = {
		B00000,
		B00000,
		B00001,
		B00010,
		B10100,
		B01000,
		B00000,
		B00000,
	};
	/*
	static byte folder[8] = {
		B00000,
		B11100,
		B11111,
		B10001,
		B10001,
		B11111,
		B00000,
		B00000
	}; 
	static byte refresh[8] = {
		B00000,
		B00110,
		B11001,
		B11000,
		B00011,
		B10011,
		B01100,
		B00000,
	};	
	*/	
	lcd.createChar(LCD_STR_UPLEVEL[0], uplevel);
	lcd.createChar(LCD_STR_ARROW_RIGHT[0], right);	
	lcd.createChar(LCD_STR_SELECT[0], select);	
	//lcd.createChar(LCD_STR_FOLDER[0], folder);	
	//lcd.createChar(LCD_STR_REFRESH[0], refresh);	
		
}

/*
int8_t  setting_grblmode(int8_t mode)
{
	int8_t c, count, line;
	char arg[8];

	if(mode == 0){
		Serial.write("$32=0\n");
	}else if(mode == 1){
		Serial.write("$32=1\n");
	}
	delay(50);
	line = 0;
	count = 0;
	memset(arg, 0, 8);
	while(Serial.available() > 0 && line < 8) {
		c = Serial.read();
		arg[count] = c;
		count++;
		if(c == '\n' ){
			line++;
			if(count > 2 && arg[0] == 'o' && arg[1] == 'k'){
				if(grbl_mode != mode){
					grbl_mode = mode;
					EEPROM.write(ADDR_GRBL_MODE, grbl_mode);
				}
				return grbl_mode;
			}
			count = 0;
			memset(arg, 0, 8);
		}
	}
	return -1;
}

*/

static void drawmenu_beginmenu(void)
{
	lcd.print(VERSION);
	lcd.setCursor(0, 1);
	lcd.print("Powered by LEKN");

	uint8_t count;
	for (count = 0; count < 16; count++) {
		// scroll one position right:
		lcd.scrollDisplayRight();
		// wait a bit:
	}
	for (count = 0; count < 16; count++) {
		// scroll one position right:
		lcd.scrollDisplayLeft();
		// wait a bit:
		delay(50);
	}
	delay(800);

}


void  postochar(int number, char* string)
{
	uint8_t len = 0;
	itoa(number, string, 10);
	
	len = strlen(string);
	if(number >= 0){
		if(len == 1){
			string[len + 2] = '\0';
			string[len + 1] = string[len - 1];
			string[len] = '.';
			string[len - 1] = '0';
		}else {
			string[len + 1] = '\0';
			string[len] = string[len - 1];
			string[len - 1] = '.';
		}
	}else if(number < 0){
		if(len == 2){
			string[len + 2] = '\0';
			string[len + 1] = string[len - 1];
			string[len] = '.';
			string[len - 1] = '0';
		}else {
			string[len + 1] = '\0';
			string[len] = string[len - 1];
			string[len - 1] = '.';
		}
	}

	
}

static void drawmenu_generic(uint8_t thisItemNr, const char* pstr, const char pre_char, const char post_char)
{
//	menu_page_header = encoder_line_num - cursor_pos;

	if((menu_page_header + cursor_pos) == thisItemNr){
		lcd.setCursor(0, cursor_pos);
		lcd.print(pre_char);
		lcd.setCursor(LCD_WIDTH - 1, cursor_pos);
		lcd.print(post_char);
	}
	if( (thisItemNr >= menu_page_header) && ((thisItemNr - menu_page_header) < LCD_HEIGHT)){
		lcd.setCursor(1, thisItemNr - menu_page_header);
		uint8_t n = LCD_WIDTH - 2;
		while (char c = pgm_read_byte(pstr)) {
			n -= lcd.write(c);
			pstr++;
		}
		while (n--) lcd.print(' ');
	}

}

static void drawmenu_select(uint8_t thisItemNr, const char* pstr, const char pre_char, const char post_char)
{
	if((menu_page_header + cursor_pos) == thisItemNr){
		lcd.setCursor(0, cursor_pos);
		lcd.print(pre_char);
		//lcd.setCursor(LCD_WIDTH - 1, cursor_pos);
		//lcd.print(post_char);
	}
	if( (thisItemNr >= menu_page_header) && ((thisItemNr - menu_page_header) < LCD_HEIGHT)){
		lcd.setCursor(1, thisItemNr - menu_page_header);
		uint8_t n = LCD_WIDTH - 2;
		while (char c = pgm_read_byte(pstr)) {
			n -= lcd.write(c);
			pstr++;
		}
		while (n--) lcd.print(' ');
		
		lcd.setCursor(LCD_WIDTH - 1, thisItemNr - menu_page_header);
		lcd.print(post_char);		
	}
}

void drawmenu_sdcardfilelist(uint8_t thisItemNr, const char* pstr, const char pre_char, const char post_char)
{
//	menu_page_header = encoder_line_num - cursor_pos;

	if((menu_page_header + cursor_pos) == thisItemNr){
		lcd.setCursor(0, cursor_pos);
		lcd.print(pre_char);
		lcd.setCursor(LCD_WIDTH - 1, cursor_pos);
		lcd.print(post_char);
		strcpy(thisfile_name, pstr);//
	}
	if( (thisItemNr >= menu_page_header) && ((thisItemNr - menu_page_header) < LCD_HEIGHT)){
		lcd.setCursor(1, thisItemNr - menu_page_header);
		uint8_t n = LCD_WIDTH - 2;
		while (char c = *pstr) {
			n -= lcd.write(c);
			pstr++;
		}
		while (n--) lcd.print(' ');
	}

}

/*****************[0]type0_menu********************/
void draw_info_menu(void)
{
	char buffer[8] ;
	if (sys_state == STATE_IDLE){
		lcd.setCursor(0, 0);
		lcd.print('X');
		memset(buffer, 0, 8);
		postochar(syspos.x, buffer);
		lcd.setCursor(LCD_WIDTH / 2 -1 - strlen(buffer), 0);
		lcd.print(buffer);
		
		lcd.setCursor(LCD_WIDTH / 2, 0);
		lcd.print('Y');
		memset(buffer, 0, 8);
		postochar(syspos.y, buffer);
		lcd.setCursor(LCD_WIDTH - 1 - strlen(buffer), 0);
		lcd.print(buffer);
		
		lcd.setCursor(0, 1);
		lcd.print('Z');
		memset(buffer, 0, 8);
		postochar(syspos.z, buffer);
		lcd.setCursor(LCD_WIDTH / 2 -1 - strlen(buffer), 1);
		lcd.print(buffer);
	}
	else{
		lcd.setCursor(0, 0);
		lcd.print('X');
		lcd.setCursor(LCD_WIDTH / 2 -1 - strlen(syspos_x), 0);
		lcd.print(syspos_x);
		
		lcd.setCursor(LCD_WIDTH / 2, 0);
		lcd.print('Y');
		lcd.setCursor(LCD_WIDTH - 1 - strlen(syspos_y), 0);
		lcd.print(syspos_y);
		
		lcd.setCursor(0, 1);
		lcd.print('Z');
		lcd.setCursor(LCD_WIDTH / 2 -1 - strlen(syspos_z), 1);
		lcd.print(syspos_z);
	}
	
	lcd.setCursor(LCD_WIDTH / 2, 1);
	lcd.print("P");	
	memset(buffer, 0, 8);
	dtostrf(progress, 3, 1, buffer);
	lcd.setCursor(LCD_WIDTH  -2 - strlen(buffer), 1);
	lcd.print(buffer);
	//lcd.setCursor(LCD_WIDTH  -1, 1);
	lcd.print('%');	
	
}
/*****************[1]type1_menu********************/
void draw_menu_main(void)
{
	drawmenu_generic(BACK, PSTR(MSG_MAIN),  LCD_STR_UPLEVEL[0],  LCD_STR_UPLEVEL[0]);
	drawmenu_generic(CONTROL_MENU, PSTR(MSG_CONTROL), '>', LCD_STR_ARROW_RIGHT[0] );
	drawmenu_generic(SETTING_MENU, PSTR(MSG_SETTING), '>', LCD_STR_ARROW_RIGHT[0] );
	if(sdcard_state == SDSTATE_YES){
		drawmenu_generic(CARD_MENU, PSTR(MSG_CARD_MENU ), '>', LCD_STR_ARROW_RIGHT[0]);
	}else if(sdcard_state == SDSTATE_NO){
		drawmenu_generic(CARD_MENU, PSTR(MSG_NO_CARD_MENU ), '>', ' ');
	}	
}

void draw_menu_main_copy(void)
{
	drawmenu_generic(BACK, PSTR(MSG_MAIN),  LCD_STR_UPLEVEL[0],  LCD_STR_UPLEVEL[0]);
	drawmenu_generic(CONTROL_COPY_MENU, PSTR(MSG_CONTROL), '>', ' ');
	if(sys_state == STATE_CNC){
		drawmenu_generic(PAUSE_RESUME_MENU, PSTR(MSG_PAUSE_PRINT), '>', ' ');
	}else if(sys_state == STATE_CNC_PAUSE){
		drawmenu_generic(PAUSE_RESUME_MENU, PSTR(MSG_RESUME_PRINT), '>', ' ');
	}
	drawmenu_generic(STOP_MENU, PSTR(MSG_STOP_PRINT), '>', ' ');
}

/*****************[2]type2_menu********************/
void draw_menu_control(void)
{
	drawmenu_generic(BACK, PSTR(MSG_BACK),  LCD_STR_UPLEVEL[0],  LCD_STR_UPLEVEL[0]);
	drawmenu_generic(AUTO_HOME_MENU, PSTR(MSG_AUTO_HOME),  '>',  ' ');
	drawmenu_generic(SPINDLE_SPEED_MENU, PSTR(MSG_SPINDLE_SPEED),  '>',  ' ');	
	drawmenu_generic(MOVE_AXIS_MENU, PSTR(MSG_MOVE_AXIS),  '>',  LCD_STR_ARROW_RIGHT[0]);
	drawmenu_generic(ZERO_POSITION_MENU, PSTR(MSG_ZERO_POSITION),  '>',  ' ');
	drawmenu_generic(GRBL_MODE_MENU, PSTR(MSG_GRBL_MODE),  '>',  LCD_STR_ARROW_RIGHT[0]);
}

void draw_menu_setting(void)
{
	drawmenu_generic(BACK, PSTR(MSG_BACK),  LCD_STR_UPLEVEL[0],  LCD_STR_UPLEVEL[0]);
	drawmenu_generic(AUTORUN_SETTING_MENU, PSTR(MSG_AUTORUN),  '>',   LCD_STR_ARROW_RIGHT[0]);
	drawmenu_generic(BAUD_SETTING_MENU, PSTR(MSG_BAUD),  '>',   LCD_STR_ARROW_RIGHT[0]);
	drawmenu_generic(BUZZER_SETTING_MENU, PSTR(MSG_MENU_BUZZER),  '>',   LCD_STR_ARROW_RIGHT[0]);
}

void draw_menu_card(void)
{
	int count = 1;
	drawmenu_generic(BACK, PSTR(MSG_BACK),  LCD_STR_UPLEVEL[0],  LCD_STR_UPLEVEL[0]);

	while(true) { 
		File entry =  root.openNextFile();
		if (! entry) {
			// no more files
			//Serial.println("**nomorefiles**");
			 root.rewindDirectory();
			break;
		}
		if (!entry.isDirectory()) {
			//Serial.print(entry.name());
			drawmenu_sdcardfilelist(count, entry.name(),  '>',  ' ');
			count++;
		}
		entry.close();
		
	}	
}
/*****************[3]type3_menu********************/
void draw_menu_move_axis(void)
{
	drawmenu_generic(BACK, PSTR(MSG_BACK),  LCD_STR_UPLEVEL[0],  LCD_STR_UPLEVEL[0]);
	drawmenu_generic(MOVE_10MM_MENU, PSTR(MSG_MOVE_10MM),  '>',  LCD_STR_ARROW_RIGHT[0]);
	drawmenu_generic(MOVE_1MM_MENU, PSTR(MSG_MOVE_1MM),  '>',  LCD_STR_ARROW_RIGHT[0]);
	drawmenu_generic(MOVE_01MM_MENU, PSTR(MSG_MOVE_01MM ),  '>',  LCD_STR_ARROW_RIGHT[0]);
}

void draw_menu_spindle_speed(void)
{
	char string[8];
	lcd.setCursor(0, 0);
	lcd.print("SPEED:");
	lcd.setCursor(LCD_WIDTH - 1 - 6, 0);
	lcd.print(sp_speed);	
	//lcd.print('%');	
}

void draw_menu_grbl_mode(void)
{
	char post_char1 = ' ', post_char2 = ' ';
	switch(grbl_mode){
		case 0: post_char1 = LCD_STR_SELECT[0]; break;
 		case 1: post_char2 = LCD_STR_SELECT[0]; break;
	}
	drawmenu_generic(BACK, PSTR(MSG_BACK),  LCD_STR_UPLEVEL[0],  LCD_STR_UPLEVEL[0]);
	drawmenu_select(GRBL_MODE_SPINDLE_MENU, PSTR(MSG_SPINDLE),  '>',  post_char1);
	drawmenu_select(GRBL_MODE_LASER_MENU, PSTR(MSG_LASER),  '>',  post_char2);
	
}

void draw_menu_autorun(void)
{
	lcd.setCursor(0, 0);
	lcd.print("AutoRun:");
	lcd.setCursor(11, 0);
	lcd.print(autorun);	
}

void draw_menu_baud(void)
{
	char post_char1 = ' ', post_char2 = ' ', post_char3 = ' ', post_char4 = ' ', post_char5 = ' ' ;
	
	switch(baud){
		case BAUD_9600: post_char1 = LCD_STR_SELECT[0]; break;
 		case BAUD_19200: post_char2 = LCD_STR_SELECT[0]; break;
		case BAUD_38400: post_char3 = LCD_STR_SELECT[0]; break;
		case BAUD_57600: post_char4 = LCD_STR_SELECT[0]; break;
		case BAUD_115200: post_char5 = LCD_STR_SELECT[0]; break;
			
	}
	drawmenu_generic(BACK, PSTR(MSG_BACK),  LCD_STR_UPLEVEL[0],  LCD_STR_UPLEVEL[0]);
	drawmenu_select(BAUD9600_MENU, PSTR(MSG_BAUD_9600),  '>', post_char1);
	drawmenu_select(BAUD19200_MENU, PSTR(MSG_BAUD_19200),  '>', post_char2);
	drawmenu_select(BAUD38400_MENU, PSTR(MSG_BAUD_38400),  '>', post_char3);
	drawmenu_select(BAUD57600_MENU, PSTR(MSG_BAUD_57600),  '>', post_char4);
	drawmenu_select(BAUD115200_MENU, PSTR(MSG_BAUD_115200),  '>', post_char5);
}


void draw_menu_buzzer(void)
{
	char post_char1 = ' ', post_char2 = ' ';
	
	switch(menu_buzzer){
		case _YES: post_char1 = LCD_STR_SELECT[0];  break;
		case _NO: post_char2 = LCD_STR_SELECT[0]; break;
	}
	drawmenu_generic(BACK, PSTR(MSG_BACK),  LCD_STR_UPLEVEL[0],  LCD_STR_UPLEVEL[0]);
	drawmenu_select(YES_MENU, PSTR(MSG_YES),  '>',   post_char1);
	drawmenu_select(NO_MENU, PSTR(MSG_NO),  '>',   post_char2);	
}


/*****************[4]type4_menu********************/
void draw_menu_move_mm(void)
{	
	drawmenu_generic(BACK, PSTR(MSG_BACK),  LCD_STR_UPLEVEL[0],  LCD_STR_UPLEVEL[0]);
	drawmenu_generic(MOVE_X_MENU, PSTR(MSG_MOVE_X),  '>',  LCD_STR_ARROW_RIGHT[0]);
	drawmenu_generic(MOVE_Y_MENU, PSTR(MSG_MOVE_Y),  '>',  LCD_STR_ARROW_RIGHT[0]);
	drawmenu_generic(MOVE_Z_MENU, PSTR(MSG_MOVE_Z),  '>',  LCD_STR_ARROW_RIGHT[0]);	
}

/*****************[5]type5_menu********************/
void draw_menu_move_xyz(void)
{	
	char string[8];
	switch(GET_MENU_TYPE(4)){
		case MOVE_X_MENU: 
			lcd.setCursor(5, 0);
			lcd.print('X');
			lcd.setCursor(LCD_WIDTH - 1 - 5, 0);
			postochar(temp_syspos.x, string);
			lcd.print(string);	
			break;
		case MOVE_Y_MENU: 
			lcd.setCursor(5, 0);
			lcd.print('Y');
			lcd.setCursor(LCD_WIDTH - 1 - 5, 0);
			postochar(temp_syspos.y, string);
			lcd.print(string);		
			break;
		case MOVE_Z_MENU:
			lcd.setCursor(5, 0);
			lcd.print('Z');
			lcd.setCursor(LCD_WIDTH - 1 - 5, 0);
			postochar(temp_syspos.z, string);
			lcd.print(string);		
			break;
	}

}

/********************menu end**********************/


void draw_menu(uint8_t m_level)
{
	if(m_level == LEVEL0_MENU){
		draw_info_menu();
	}else if(m_level == LEVEL1_MENU){
		if(sys_state == STATE_IDLE){
			draw_menu_main();
		}else{
			draw_menu_main_copy();
		}
		
	}else if(m_level == LEVEL2_MENU){
		switch(GET_MENU_TYPE(1)){
 			case CONTROL_MENU: draw_menu_control();break;
			case SETTING_MENU: draw_menu_setting();break;
			case CARD_MENU:	draw_menu_card();break;
		}	
	}else if(m_level == LEVEL3_MENU){
		if(GET_MENU_TYPE(1) == CONTROL_MENU){			
			switch(GET_MENU_TYPE(2)){
	 			case MOVE_AXIS_MENU: draw_menu_move_axis();break;
				case SPINDLE_SPEED_MENU: 
					sp_speed += encoder_dir * SPSPEED_STEP;
					if(sp_speed < 0) sp_speed = 0;
					if(sp_speed > 24000) sp_speed = 24000;						
					draw_menu_spindle_speed();
					break;
				case GRBL_MODE_MENU: draw_menu_grbl_mode(); break;	
			}
		}else if(GET_MENU_TYPE(1) == SETTING_MENU){
			switch(GET_MENU_TYPE(2)){
				case AUTORUN_SETTING_MENU: 
					autorun += encoder_dir;
					if(autorun < 0) autorun = 0;
					if(autorun > 60) autorun = 60;	
					draw_menu_autorun();
					break;
	 			case BAUD_SETTING_MENU: draw_menu_baud();break;
				case BUZZER_SETTING_MENU: draw_menu_buzzer();break;
			}
		}
	}
	else if(m_level == LEVEL4_MENU){
		if(GET_MENU_TYPE(1) == CONTROL_MENU && GET_MENU_TYPE(2) == MOVE_AXIS_MENU){
			switch(GET_MENU_TYPE(3)){
	 			case MOVE_10MM_MENU: draw_menu_move_mm();break;
				case MOVE_1MM_MENU: draw_menu_move_mm();break;
				case MOVE_01MM_MENU: draw_menu_move_mm();break;
			}
		}
	}else if(m_level == LEVEL5_MENU){
		if(GET_MENU_TYPE(1) == CONTROL_MENU && GET_MENU_TYPE(2) == MOVE_AXIS_MENU){
			if(GET_MENU_TYPE(3) == MOVE_10MM_MENU){
				switch(GET_MENU_TYPE(4)){
		 			case MOVE_X_MENU: 
						temp_syspos.x += encoder_dir * 100;break;
					case MOVE_Y_MENU: 
						temp_syspos.y += encoder_dir * 100;break;
					case MOVE_Z_MENU:
						temp_syspos.z += encoder_dir * 100;break;
				}
			}
			else if(GET_MENU_TYPE(3) == MOVE_1MM_MENU){
				switch(GET_MENU_TYPE(4)){
		 			case MOVE_X_MENU: 
						temp_syspos.x += encoder_dir * 10;break;
					case MOVE_Y_MENU: 
						temp_syspos.y += encoder_dir * 10;break;
					case MOVE_Z_MENU:
						temp_syspos.z += encoder_dir * 10;break;
				}
			}
			else if(GET_MENU_TYPE(3) == MOVE_01MM_MENU){
				switch(GET_MENU_TYPE(4)){
		 			case MOVE_X_MENU: 
						temp_syspos.x += encoder_dir;break;
					case MOVE_Y_MENU: 
						temp_syspos.y += encoder_dir;break;
					case MOVE_Z_MENU:
						temp_syspos.z += encoder_dir;break;
				}
			}
			draw_menu_move_xyz();

		}

	}
	
}



void parsing_menu_level0(void)
{
	menu_level++;
	CLEAR_MENU_ARG();
}

void parsing_menu_level1(void)
{
	if(sys_state == STATE_IDLE){
		switch(menu_page_header + cursor_pos){
			case BACK:  
				if(menu_level){
					CLEAR_MENU_TYPE(menu_level);
					CLEAR_MENU_ARG();
					menu_level--;
				}	
				break;
			case CONTROL_MENU:
				SET_MENU_TYPE(CONTROL_MENU, menu_level);
				SAVE_MENU_ARG(menu_level);
				CLEAR_MENU_ARG();
				menu_level++;				
				break;
			case SETTING_MENU:
				SET_MENU_TYPE(SETTING_MENU, menu_level);
				SAVE_MENU_ARG(menu_level);
				CLEAR_MENU_ARG();
				menu_level++;				
				break;				
			case CARD_MENU:
				if(sdcard_state == SDSTATE_YES){
					SET_MENU_TYPE(CARD_MENU, menu_level);
					SAVE_MENU_ARG(menu_level);
					CLEAR_MENU_ARG();
					menu_level++;
				}
				break;	
		}
	}
	else {
		switch(menu_page_header + cursor_pos){
			case BACK:  
				if(menu_level){
					CLEAR_MENU_TYPE(menu_level);
					CLEAR_MENU_ARG();
					menu_level--;
				}	
				break;
			case CONTROL_COPY_MENU:
				break;
			case PAUSE_RESUME_MENU:
				if(sys_state == STATE_CNC){
					sys_state = STATE_CNC_PAUSE;
					FLUSH_ULTRALCD();
				}else if(sys_state == STATE_CNC_PAUSE){
					sys_state = STATE_CNC;
					FLUSH_ULTRALCD();
				}
				break;	
			case STOP_MENU:
				sys_state = STATE_CNC_STOP;
				
				break;
				
		}	
	}

}


void parsing_menu_level2(void)
{	
	if(GET_MENU_TYPE(1) == CONTROL_MENU){
		switch(menu_page_header + cursor_pos){
			case BACK:  
				if(menu_level){
					CLEAR_MENU_TYPE(menu_level);
					CLEAR_MENU_ARG();
					menu_level--;
					RESTORE_MENU_ARG(menu_level);
				}	
				break;
			case AUTO_HOME_MENU:
				Serial.print(GCODE_AUTO_HOME);
				break;
			case MOVE_AXIS_MENU:
				SET_MENU_TYPE(MOVE_AXIS_MENU, menu_level);
				SAVE_MENU_ARG(menu_level);
				CLEAR_MENU_ARG();
				menu_level++;
				break;
			case SPINDLE_SPEED_MENU:
				SET_MENU_TYPE(SPINDLE_SPEED_MENU, menu_level);
				SAVE_MENU_ARG(menu_level);
				CLEAR_MENU_ARG();
				menu_level++;
				break;				
			case ZERO_POSITION_MENU:
				Serial.print(GCODE_ZEROPOS);
				Serial.print(GCODE_G90);
				syspos.x = 0;
				syspos.y = 0;
				syspos.z = 0;
				break;
			case GRBL_MODE_MENU:
				SET_MENU_TYPE(GRBL_MODE_MENU, menu_level);
				SAVE_MENU_ARG(menu_level);
				CLEAR_MENU_ARG();
				menu_level++;
				break;		
		}	
	}
	else if(GET_MENU_TYPE(1) == SETTING_MENU){
		switch(menu_page_header + cursor_pos){
			case BACK:  
				if(menu_level){
					CLEAR_MENU_TYPE(menu_level);
					CLEAR_MENU_ARG();
					menu_level--;
					RESTORE_MENU_ARG(menu_level);
				}	
				break;
			case AUTORUN_SETTING_MENU:
				SET_MENU_TYPE(AUTORUN_SETTING_MENU, menu_level);
				SAVE_MENU_ARG(menu_level);
				CLEAR_MENU_ARG();
				menu_level++;
				break;
			case BAUD_SETTING_MENU:
				SET_MENU_TYPE(BAUD_SETTING_MENU, menu_level);
				SAVE_MENU_ARG(menu_level);
				CLEAR_MENU_ARG();
				menu_level++;
				break;
			case BUZZER_SETTING_MENU:
				SET_MENU_TYPE(BUZZER_SETTING_MENU, menu_level);
				SAVE_MENU_ARG(menu_level);
				CLEAR_MENU_ARG();
				menu_level++;
				break;

		}
	}

	else if(GET_MENU_TYPE(1) == CARD_MENU){
		switch(menu_page_header + cursor_pos){
			case BACK:  
				if(menu_level){
					CLEAR_MENU_TYPE(menu_level);
					CLEAR_MENU_ARG();
					menu_level--;
					RESTORE_MENU_ARG(menu_level);
				}	
				break;
			default:
				//read_cardfile(this_filename);
				sys_state = STATE_CNC;
				menu_level = LEVEL0_MENU;
				memset(syspos_x, 0, 8);
				memset(syspos_y, 0, 8);
				memset(syspos_z, 0, 8);
				break;
		}
	}
}

void parsing_menu_level3(void)
{
	if(GET_MENU_TYPE(2) == MOVE_AXIS_MENU && GET_MENU_TYPE(1) == CONTROL_MENU){
		switch(menu_page_header + cursor_pos){
			case BACK:  
				if(menu_level){
					CLEAR_MENU_TYPE(menu_level);
					CLEAR_MENU_ARG();
					menu_level--;
					RESTORE_MENU_ARG(menu_level);
				}	
				break;
			case MOVE_10MM_MENU:
				SET_MENU_TYPE(MOVE_10MM_MENU, menu_level);
				SAVE_MENU_ARG(menu_level);
				CLEAR_MENU_ARG();
				menu_level++;
				break;
			case MOVE_1MM_MENU:
				SET_MENU_TYPE(MOVE_1MM_MENU, menu_level);
				SAVE_MENU_ARG(menu_level);
				CLEAR_MENU_ARG();
				menu_level++;
				break;	
			case MOVE_01MM_MENU:
				SET_MENU_TYPE(MOVE_01MM_MENU, menu_level);
				SAVE_MENU_ARG(menu_level);
				CLEAR_MENU_ARG();
				menu_level++;
				break;			
		}
	}
	if(GET_MENU_TYPE(2) == SPINDLE_SPEED_MENU&& GET_MENU_TYPE(1) == CONTROL_MENU){
		char buffer[8];
		memset(buffer, 0, 8);
		strcpy(&buffer[0], GCODE_M3);
		buffer[2] = ' ';
		buffer[3] = 'S';
		itoa(sp_speed, &buffer[4], 10);
		buffer[strlen(buffer)] = '\0';					
		Serial.print(buffer);
		Serial.print("\n");
		
		CLEAR_MENU_TYPE(menu_level);
		CLEAR_MENU_ARG();
		menu_level--;
		RESTORE_MENU_ARG(menu_level);

	}
	if(GET_MENU_TYPE(2) == GRBL_MODE_MENU && GET_MENU_TYPE(1) == CONTROL_MENU){
		switch(menu_page_header + cursor_pos){
			case BACK:  
				if(menu_level){
					CLEAR_MENU_TYPE(menu_level);
					CLEAR_MENU_ARG();
					menu_level--;
					RESTORE_MENU_ARG(menu_level);
				}	
				break;
			case GRBL_MODE_SPINDLE_MENU:
				grbl_mode = 0;
				Serial.print(GCODE_SPINDLE);
				break;
			case GRBL_MODE_LASER_MENU:
				grbl_mode = 1;
				Serial.print(GCODE_LASER);
				break;
		}
		FLUSH_ULTRALCD();
	}
	if(GET_MENU_TYPE(2) == AUTORUN_SETTING_MENU && GET_MENU_TYPE(1) == SETTING_MENU){;
		save_setting(ADDR_AUTORUN, autorun);		
		CLEAR_MENU_TYPE(menu_level);
		CLEAR_MENU_ARG();
		menu_level--;
		RESTORE_MENU_ARG(menu_level);

	}
	
	if(GET_MENU_TYPE(2) == BAUD_SETTING_MENU && GET_MENU_TYPE(1) == SETTING_MENU){
		uint8_t old_baud = baud;
		switch(menu_page_header + cursor_pos){
			case BACK:  
				if(menu_level){
					CLEAR_MENU_TYPE(menu_level);
					CLEAR_MENU_ARG();
					menu_level--;
					RESTORE_MENU_ARG(menu_level);
				}	
				break;
			case BAUD9600_MENU:
				baud = BAUD_9600;
				break;
			case BAUD19200_MENU:
				baud = BAUD_19200;
				break;
			case BAUD38400_MENU:
				baud = BAUD_38400;
				break;
			case BAUD57600_MENU:
				baud = BAUD_57600;
				break;
			case BAUD115200_MENU:
				baud = BAUD_115200;
				break;
		}
		if(old_baud != baud){
			save_setting(ADDR_BAUD, baud);
			switch(baud){
				case BAUD_9600: Serial.begin(9600L);; break;
				case BAUD_19200: Serial.begin(19200L);; break;
				case BAUD_38400: Serial.begin(38400L);; break;
				case BAUD_57600: Serial.begin(57600L);; break;
				case BAUD_115200: Serial.begin(115200L);; break;
				default: Serial.begin(115200L);; break;
			}			
		}
		FLUSH_ULTRALCD();
	}
	if(GET_MENU_TYPE(2) == BUZZER_SETTING_MENU&& GET_MENU_TYPE(1) == SETTING_MENU){
		uint8_t old_menu_buzzer = menu_buzzer;
		switch(menu_page_header + cursor_pos){
			case BACK:  
				if(menu_level){
					CLEAR_MENU_TYPE(menu_level);
					CLEAR_MENU_ARG();
					menu_level--;
					RESTORE_MENU_ARG(menu_level);
				}	
				break;
			case YES_MENU:
				menu_buzzer = _YES;
				break;
			case NO_MENU:
				menu_buzzer = _NO;
				break;
			default:
				menu_buzzer = _YES;
				break;
		}
		if(old_menu_buzzer != menu_buzzer){
			save_setting(ADDR_BUZZER, menu_buzzer);
		}
		FLUSH_ULTRALCD();
	}
					
}

void parsing_menu_level4(void)
{
	if((GET_MENU_TYPE(3) == MOVE_10MM_MENU || GET_MENU_TYPE(3) == MOVE_1MM_MENU || GET_MENU_TYPE(3) == MOVE_01MM_MENU) 
		&& GET_MENU_TYPE(2) == MOVE_AXIS_MENU && GET_MENU_TYPE(1) == CONTROL_MENU){
		switch(menu_page_header + cursor_pos){
			case BACK:  
				if(menu_level){
					CLEAR_MENU_TYPE(menu_level);
					CLEAR_MENU_ARG();
					menu_level--;
					RESTORE_MENU_ARG(menu_level);
				}	
				break;
			case MOVE_X_MENU:
				SET_MENU_TYPE(MOVE_X_MENU, menu_level);
				SAVE_MENU_ARG(menu_level);
				CLEAR_MENU_ARG();
				menu_level++;
				break;
			case MOVE_Y_MENU:
				SET_MENU_TYPE(MOVE_Y_MENU, menu_level);
				SAVE_MENU_ARG(menu_level);
				CLEAR_MENU_ARG();
				menu_level++;
				break;	
			case MOVE_Z_MENU:
				SET_MENU_TYPE(MOVE_Z_MENU, menu_level);
				SAVE_MENU_ARG(menu_level);
				CLEAR_MENU_ARG();
				menu_level++;
				break;			
		}
	}
}

void parsing_menu_level5(void)
{
	char buffer[16];
	char pos_buffer[8];
	uint8_t len = 0;

	if(GET_MENU_TYPE(1) == CONTROL_MENU && GET_MENU_TYPE(2) == MOVE_AXIS_MENU){
		memset(buffer, 0, 16);
		strcpy(&buffer[0], GCODE_G1);
		buffer[2] = ' ';
		switch(GET_MENU_TYPE(4)){
 				case MOVE_X_MENU: 
					buffer[3] = 'X'; 
					postochar((temp_syspos.x), pos_buffer);
					strcpy(&buffer[4], pos_buffer);
					temp_syspos.x = 0;
					break;
				case MOVE_Y_MENU: 
					buffer[3] = 'Y'; 
					postochar((temp_syspos.y), pos_buffer);
					strcpy(&buffer[4], pos_buffer);
					temp_syspos.y = 0;
					break;
				case MOVE_Z_MENU: 
					buffer[3] = 'Z'; 
					postochar((temp_syspos.z), pos_buffer);
					strcpy(&buffer[4], pos_buffer);
					temp_syspos.z = 0;
					break;
		}
		buffer[strlen(buffer)] = ' ';
		strcpy(&buffer[strlen(buffer)], SPEED);
		buffer[strlen(buffer)] = '\0';	
		Serial.print(GCODE_G91);
		Serial.print(buffer);
		Serial.print("\n");
		Serial.print(GCODE_G90);
		
		CLEAR_MENU_TYPE(menu_level);
		CLEAR_MENU_ARG();
		menu_level--;
		RESTORE_MENU_ARG(menu_level);
	}
	
}


void change_menu_level(void)
{
	switch(menu_level){
		case 0: parsing_menu_level0();break;
		case 1: parsing_menu_level1();break;
		case 2: parsing_menu_level2();break;
		case 3: parsing_menu_level3();break;
		case 4: parsing_menu_level4();break;
		case 5: parsing_menu_level5();break;
	}

}

void update_menu_arg(void)
{	
		if( encoder_line_num < 0){
			encoder_line_num = 0;
			encoder_value = 0;
		}			 
		if(menu_level == LEVEL1_MENU){
			if(sys_state == STATE_IDLE){
				if(encoder_line_num >= MAIN_MENU_MAX){
					encoder_line_num = MAIN_MENU_MAX - 1;
					encoder_value = (MAIN_MENU_MAX - 1) * ENCODER_PULSES_PER_STEP;
				}
			}else{
				if(encoder_line_num >= MAIN_COPY_MENU_MAX){
					encoder_line_num = MAIN_COPY_MENU_MAX - 1;
					encoder_value = (MAIN_COPY_MENU_MAX - 1) * ENCODER_PULSES_PER_STEP;
				}
			}
		}else if(menu_level == LEVEL2_MENU){
			if(GET_MENU_TYPE(menu_level - 1) == CONTROL_MENU && encoder_line_num >= CONTROL_MENU_MAX){
				encoder_line_num = CONTROL_MENU_MAX - 1;
				encoder_value = (CONTROL_MENU_MAX - 1) * ENCODER_PULSES_PER_STEP;
			}else if(GET_MENU_TYPE(menu_level - 1) == SETTING_MENU && encoder_line_num >= SETTING_MENU_MAX){
				encoder_line_num = SETTING_MENU_MAX - 1;
				encoder_value = (SETTING_MENU_MAX - 1) * ENCODER_PULSES_PER_STEP;
			}else if(GET_MENU_TYPE(menu_level - 1) == CARD_MENU && encoder_line_num >= CARD_MENU_MAX){
				encoder_line_num = CARD_MENU_MAX - 1;
				encoder_value = (CARD_MENU_MAX - 1) * ENCODER_PULSES_PER_STEP;
			}
		}else if(menu_level == LEVEL3_MENU){
			if(GET_MENU_TYPE(menu_level - 2) == CONTROL_MENU && GET_MENU_TYPE(menu_level - 1) == MOVE_AXIS_MENU 
				&& encoder_line_num >= MOVE_AXIS_MENU_MAX){
				encoder_line_num = MOVE_AXIS_MENU_MAX - 1;
				encoder_value = (MOVE_AXIS_MENU_MAX - 1) * ENCODER_PULSES_PER_STEP;
			}else if(GET_MENU_TYPE(menu_level - 2) == CONTROL_MENU && GET_MENU_TYPE(menu_level - 1) == GRBL_MODE_MENU
				&& encoder_line_num >= GRBL_MODE_MENU_MAX){
				encoder_line_num = GRBL_MODE_MENU_MAX - 1;
				encoder_value = (GRBL_MODE_MENU_MAX - 1) * ENCODER_PULSES_PER_STEP;
			}else if(GET_MENU_TYPE(menu_level - 2) == SETTING_MENU &&  GET_MENU_TYPE(menu_level - 1) == BAUD_SETTING_MENU 
			&& encoder_line_num >= BAUD_MENU_MAX){
				encoder_line_num = BAUD_MENU_MAX - 1;
				encoder_value = (BAUD_MENU_MAX - 1) * ENCODER_PULSES_PER_STEP;
			}else if(GET_MENU_TYPE(menu_level - 2) == SETTING_MENU && GET_MENU_TYPE(menu_level - 1) == BUZZER_SETTING_MENU
			&& encoder_line_num >= SELECT_MENU_MAX){
				encoder_line_num = SELECT_MENU_MAX - 1;
				encoder_value = (SELECT_MENU_MAX - 1) * ENCODER_PULSES_PER_STEP;
			}
		}else if(menu_level == LEVEL4_MENU){
			if(GET_MENU_TYPE(menu_level - 3) == CONTROL_MENU && GET_MENU_TYPE(menu_level - 2) == MOVE_AXIS_MENU 
				&& encoder_line_num >= MOVE_MM_MENU_MAX){
				encoder_line_num = MOVE_MM_MENU_MAX - 1;
				encoder_value = (MOVE_MM_MENU_MAX - 1) * ENCODER_PULSES_PER_STEP;
			}
		}else if(menu_level == LEVEL5_MENU){
			encoder_line_num = 0;
			encoder_value = 0;
		}
		
		if(encoder_line_num > old_encoder_line_num){
			if(cursor_pos < LCD_HEIGHT - 1)
				cursor_pos++ ;
		}else if (encoder_line_num < old_encoder_line_num) {
			if(cursor_pos > 0)
				cursor_pos--;
		}
		old_encoder_line_num = encoder_line_num;
		old_menu_level = menu_level;
		menu_page_header = encoder_line_num - cursor_pos;
	
}

void ultralcd_init(void)
{
	
	lcd_set_custom_characters();
	lcd.begin(LCD_WIDTH, LCD_HEIGHT);
	drawmenu_beginmenu();
	lcd.clear();
	draw_menu(LEVEL0_MENU);
	syspos.x = 0;
	syspos.y = 0;
	syspos.z = 0;
	
	card_filenum = sdcard_filenum();

}

void ultralcd_update_menu(void)
{
	/*****************[1]encoder button********************/
	if(encoder_push == 1){
	BUZZER();
	encoder_push = 0;
		change_menu_level();
	}	
	
	/*****************[2]encoder rotate********************/
	encoder_line_num = encoder_value / ENCODER_PULSES_PER_STEP ;
	if(old_encoder_line_num != encoder_line_num || menu_level != old_menu_level){
		if(menu_level == old_menu_level){
			encoder_dir = encoder_line_num - old_encoder_line_num;
		}		
		update_menu_arg();
		FLUSH_ULTRALCD();
		
	}

	/****************[3]sdcard state change****************/
	if(sd_detectpin != old_sd_detectpin){
		BUZZER();
		if(sd_detectpin == LOW){
			sdcard_state = sdcard_init();
			if(sdcard_state == SDSTATE_YES)
				old_sd_detectpin = sd_detectpin;
			card_filenum = sdcard_filenum();
		}else if(sd_detectpin == HIGH){
			sdcard_state = SDSTATE_NO;
			old_sd_detectpin = sd_detectpin;
		}
		FLUSH_ULTRALCD();
	}

	info_timeout = millis();
}


void ultralcd_update_infomenu(char *xpos, char *ypos, char *zpos, float p)
{
//	static unsigned long temp = 0;
	
//	if((millis() - temp) > UPDATE_INFOMENU_TIME){		
//	}
	memcpy(syspos_x, xpos, POSBUF_SIZE);
	memcpy(syspos_y, ypos, POSBUF_SIZE);
	memcpy(syspos_z, zpos, POSBUF_SIZE);
	progress = p;
	//Serial.print(p);
	FLUSH_ULTRALCD();
}

void ultralcd_back_infomenu(void)
{
	if(menu_level != LEVEL0_MENU){
		menu_level = LEVEL0_MENU;
		old_menu_level = menu_level;
		FLUSH_ULTRALCD();
	}
}
