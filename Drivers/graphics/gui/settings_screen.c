/*
 * debug_screen.c
 *
 *  Created on: Aug 2, 2017
 *      Author: jose
 */

#include "settings_screen.h"
#include "oled.h"

static widget_t *combo = NULL;
static uint16_t KP;
static uint16_t KI;
static uint16_t KD;
static uint16_t CONTRAST;
static uint16_t MAX_POWER;
static uint16_t oldBTIME, oldBTEMP, oldSLEEPTIME, oldSLEEPTEMP, oldSTANDBYTIME;
static widget_t *tipCombo = NULL;
static widget_t *delTipButton = NULL;
static comboBox_item_t *addNewTipComboItem = NULL;
char str[20];
static void edit_iron_tip_screen_init(screen_t *scr) {
	if(strcmp(tipCombo->comboBoxWidget.currentItem->text, "ADD NEW") == 0) {
		strcpy(str, "   ");
		delTipButton->enabled = 0;
	}
	else {
		strcpy(str, tipCombo->comboBoxWidget.currentItem->text);
		delTipButton->enabled = 1;
	}
	default_init(scr);
}
static void edit_iron_screen_init(screen_t *scr) {
	comboBox_item_t *i = tipCombo->comboBoxWidget.items;
	for(int x = 0; x < sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0]); ++x) {
		if(x < systemSettings.currentNumberOfTips) {
			strcpy(i->text, systemSettings.ironTips[x].name);
			i->enabled = 1;
		}
		else
			i->enabled = 0;
		i = i->next_item;
	}
	tipCombo->comboBoxWidget.currentItem = tipCombo->comboBoxWidget.items;
	tipCombo->comboBoxWidget.currentScroll = 0;
	if(systemSettings.currentNumberOfTips > sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0])) {
		addNewTipComboItem->enabled = 0;
	}
}
static void *getTipStr() {
	return str;
}

static void setTipStr(char *s) {
	strcpy(str, s);
}
static int saveTip(widget_t *w) {
	if(strcmp(tipCombo->comboBoxWidget.currentItem->text, "ADD NEW") == 0) {
		strcpy(systemSettings.ironTips[systemSettings.currentNumberOfTips].name, str);
		++systemSettings.currentNumberOfTips;
		saveSettings();
	}
	else {
		for(int x = 0; x < sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0]); ++ x) {
			if(strcmp(tipCombo->comboBoxWidget.currentItem->text, systemSettings.ironTips[x].name) == 0) {
				strcpy(systemSettings.ironTips[x].name, str);
				saveSettings();
				break;
			}
		}
	}
	return screen_edit_iron_tips;
}
static int cancelTip(widget_t *w) {
	return screen_edit_iron_tips;
}
static int delTip(widget_t *w) {
	uint8_t itemIndex = 0;
	for(int x = 0; x < sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0]); ++ x) {
		if(strcmp(tipCombo->comboBoxWidget.currentItem->text, systemSettings.ironTips[x].name) == 0) {
			itemIndex = x;
			break;
		}
	}
	for(int x = itemIndex; x < sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0]) - 1; ++ x) {
		systemSettings.ironTips[x] = systemSettings.ironTips[x + 1];
	}
	--systemSettings.currentNumberOfTips;
	saveSettings();
	return screen_edit_iron_tips;
}
////
static void * getMaxPower() {
	MAX_POWER = currentPID.max * 100;
	return &MAX_POWER;
}
static void setMaxPower(uint16_t *val) {
	MAX_POWER = *val;
	currentPID.max = (double)MAX_POWER / 100.0;
	setupPIDFromStruct();
}
static int savePower(widget_t *w) {
	systemSettings.ironTips[systemSettings.currentTip].PID.max = currentPID.max;
	saveSettings();
	return screen_settings;

}
static int cancelPower(widget_t *w) {
	currentPID.max = systemSettings.ironTips[systemSettings.currentTip].PID.max;
	setupPIDFromStruct();
	return screen_settings;
}
static void * getContrast_() {
	CONTRAST = getContrast();
	return &CONTRAST;
}
static void setContrast_(uint16_t *val) {
	CONTRAST = *val;
	setContrast(CONTRAST);
}
static int saveContrast(widget_t *w) {
	systemSettings.contrast = CONTRAST;
	saveSettings();
	return screen_settings;
}
static int cancelContrast(widget_t *w) {
	setContrast(systemSettings.contrast);
	return screen_settings;
}
////
static void * getBoostTime() {
	return &systemSettings.boost.Time;
}
static void setBoostTime(uint16_t *val) {
	systemSettings.boost.Time = *val;
}
static void * getBoostTemp() {
	return &systemSettings.boost.Temperature;
}
static void setBoostTemp(uint16_t *val) {
	systemSettings.boost.Temperature = *val;
}
static int saveBoost(widget_t *w) {
	saveSettings();
	return screen_settings;
}
static int cancelBoost(widget_t *w) {
	systemSettings.boost.Temperature = oldBTEMP;
	systemSettings.boost.Time = oldBTIME;
	return screen_settings;
}
////
static int saveSleep(widget_t *w) {
	saveSettings();
	return screen_settings;
}
static int cancelSleep(widget_t *w) {
	systemSettings.sleep.Temperature = oldSLEEPTEMP;
	systemSettings.sleep.Time = oldSLEEPTIME;
	return screen_settings;
}
static void setSleepTime(uint16_t *val) {
	systemSettings.sleep.Time = *val;
}

static void * getSleepTime() {
	return &systemSettings.sleep.Time;
}
static void setStandByTime(uint16_t *val) {
	systemSettings.standby.Time  = *val;
}
static void * getStandByTime() {
	return &systemSettings.standby.Time;
}
static void setSleepTemp(uint16_t *val) {
	systemSettings.sleep.Temperature = *val;
}
static void * getSleepTemp() {
	return &systemSettings.sleep.Temperature;
}

static int savePID(widget_t *w) {
	systemSettings.ironTips[systemSettings.currentTip].PID = currentPID;
	saveSettings();
	return screen_settings;
}
static int cancelPID(widget_t *w) {
	currentPID = systemSettings.ironTips[systemSettings.currentTip].PID;
	setupPIDFromStruct();
	return screen_settings;
}

static void * getKp() {
	KP = currentPID.Kp * 1000000;
	return &KP;
}
static void setKp(uint16_t *val) {
	KP = *val;
	currentPID.Kp = (double)KP / 1000000;
	setupPIDFromStruct();
}
static void * getKi() {
	KI = currentPID.Ki * 1000000;
	return &KI;
}
static void setKi(uint16_t *val) {
	KI = *val;
	currentPID.Ki = (double)KI / 1000000;
	setupPIDFromStruct();
}
static void * getKd() {
	KD = currentPID.Kd * 10000000;
	return &KD;
}
static void setKd(uint16_t *val) {
	KD = *val;
	currentPID.Kd = (double)KD / 10000000;
	setupPIDFromStruct();
}

static void on_Enter(screen_t *scr) {
	oldBTIME = systemSettings.boost.Time;
	oldBTEMP = systemSettings.boost.Temperature;
	oldSLEEPTIME = systemSettings.sleep.Time;
	oldSLEEPTEMP = systemSettings.sleep.Temperature;
	oldSTANDBYTIME = systemSettings.standby.Time;
}

static void settings_screen_init(screen_t *scr) {
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	default_init(scr);
	scr->current_widget = combo;
	scr->current_widget->comboBoxWidget.selectable.state = widget_selected;
}

void settings_screen_setup(screen_t *scr) {
	///settings combobox
	scr->draw = &default_screenDraw;
	scr->processInput = &default_screenProcessInput;
	scr->init = &settings_screen_init;
	scr->update = &default_screenUpdate;
	scr->onEnter = &on_Enter;

	widget_t *widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	char *s = "SETTINGS MENU";
	strcpy(widget->displayString, s);
	widget->posX = 10;
	widget->posY = 0;
	widget->font_size = &FONT_8X14_reduced;
	widget->reservedChars = 8;
	widget->draw = &default_widgetDraw;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_combo);
	widget->posY = 17;
	widget->posX = 0;
	widget->font_size = &FONT_6X8_reduced;
	comboAddItem(widget, "PID", screen_edit_pid);
	comboAddItem(widget, "POWER", screen_edit_max_power);
	comboAddItem(widget, "SLEEP", screen_edit_sleep);
	comboAddItem(widget, "BOOST", screen_edit_boost);
	comboAddItem(widget, "SCREEN", screen_edit_contrast);
	comboAddItem(widget, "TIPS", screen_edit_iron_tips);
	comboAddItem(widget, "CALIBRATION", screen_edit_calibration_wait);
	comboAddItem(widget, "EXIT", screen_main);
	combo = widget;

	//--------------------------------------------------------------------------------------------
	// Edit PID screen
	screen_t *sc =oled_addScreen(screen_edit_pid);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &default_init;
	sc->update = &default_screenUpdate;

	widget_t *w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	s = "PID";
	strcpy(w->displayString, s);
	w->posX = 50;
	w->posY = 0;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars =3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	s = "Kp:";
	strcpy(w->displayString, s);
	w->posX = 30;
	w->posY = 17;
	w->font_size = &FONT_6X8_reduced;
	w->reservedChars = 3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 55;
	w->posY = 17;
	w->font_size = &FONT_6X8_reduced;
	w->editable.inputData.getData = &getKp;
	w->editable.inputData.number_of_dec = 2;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 100;
	w->editable.step = 100;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setKp;
	w->reservedChars = 8;
	w->displayWidget.justify = justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	s = "Ki:";
	strcpy(w->displayString, s);
	w->posX = 30;
	w->posY = 29;
	w->font_size = &FONT_6X8_reduced;
	w->reservedChars = 3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 55;
	w->posY = 29;
	w->font_size = &FONT_6X8_reduced;
	w->editable.inputData.getData = &getKi;
	w->editable.inputData.number_of_dec = 2;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 100;
	w->editable.step = 100;
	w->editable.selectable.tab = 1;
	w->editable.setData = (void (*)(void *))&setKi;
	w->reservedChars = 8;
	w->displayWidget.justify = justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	s = "Kd:";
	strcpy(w->displayString, s);
	w->posX = 30;
	w->posY = 41;
	w->font_size = &FONT_6X8_reduced;
	w->reservedChars = 3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 55;
	w->posY = 41;
	w->font_size = &FONT_6X8_reduced;
	w->editable.inputData.getData = &getKd;
	w->editable.inputData.number_of_dec = 2;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 100;
	w->editable.step = 100;
	w->editable.selectable.tab = 2;
	w->editable.setData = (void (*)(void *))&setKd;
	w->reservedChars = 8;
	w->displayWidget.justify = justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 94;
	w->posY = 50;
	s = "BACK";
	strcpy(w->displayString, s);
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 3;
	w->buttonWidget.action = &cancelPID;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 1;
	w->posY = 50;
	s = "SAVE";
	strcpy(w->displayString, s);
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 4;
	w->buttonWidget.action = &savePID;

	//--------------------------------------------------------------------------------------------
	// Edit power screen
	sc = oled_addScreen(screen_edit_max_power);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &default_init;
	sc->update = &default_screenUpdate;
	w = screen_addWidget(sc);

	widgetDefaultsInit(w, widget_label);
	s = "MAX POWER";
	strcpy(w->displayString, s);
	w->posX = 27;
	w->posY = 0;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 8;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 40;
	w->posY = 20;
	w->font_size = &FONT_16X26_reduced;
	w->editable.inputData.getData = &getMaxPower;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 10;
	w->editable.step = 5;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setMaxPower;
	w->editable.max_value = 100;
	w->editable.min_value = 10;
	w->reservedChars = 3;
	w->displayWidget.justify=justify_right;


	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 94;
	w->posY = 50;
	s = "BACK";
	strcpy(w->displayString, s);
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 1;
	w->buttonWidget.action = &cancelPower;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 1;
	w->posY = 50;
	s = "SAVE";
	strcpy(w->displayString, s);
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 2;
	w->buttonWidget.action = &savePower;

	//--------------------------------------------------------------------------------------------------------
	// Edit sleep screen
	sc = oled_addScreen(screen_edit_sleep);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &default_init;
	sc->update = &default_screenUpdate;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	s = "SLEEP & STANDBY";
	strcpy(w->displayString, s);
	w->posX = 0;
	w->posY = 0;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 15;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	s = "Sleep Time(s):";
	strcpy(w->displayString, s);
	w->posX = 2;
	w->posY = 17;
	w->font_size = &FONT_6X8_reduced;
	w->reservedChars = 14;
	//
	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 100;
	w->posY = 17;
	w->font_size = &FONT_6X8_reduced;
	w->editable.inputData.getData = &getSleepTime;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setSleepTime;
	w->editable.max_value = 600;
	w->editable.min_value = 0;
	w->reservedChars = 3;
	w->displayWidget.justify =justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	s = "Sleep Temp(C):";
	strcpy(w->displayString, s);
	w->posX = 2;
	w->posY = 28;
	w->font_size = &FONT_6X8_reduced;
	w->reservedChars = 3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 100;
	w->posY = 28;
	w->font_size = &FONT_6X8_reduced;
	w->editable.inputData.getData = &getSleepTemp;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.selectable.tab = 1;
	w->editable.setData = (void (*)(void *))&setSleepTemp;
	w->reservedChars = 3;
	w->editable.max_value = 480;
	w->editable.min_value = 100;
	w->displayWidget.justify =justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	s = "StandBy Time(m):";
	strcpy(w->displayString, s);
	w->posX = 2;
	w->posY = 39;
	w->font_size = &FONT_6X8_reduced;
	w->reservedChars = 16;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 100;
	w->posY = 39;
	w->font_size = &FONT_6X8_reduced;
	w->editable.inputData.getData = &getStandByTime;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.selectable.tab = 2;
	w->editable.setData = (void (*)(void *))&setStandByTime;
	w->reservedChars = 3;
	w->editable.max_value = 600;
	w->editable.min_value = 0;
	w->displayWidget.justify =justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	//w->font_size = &FONT_6X8_reduced;
	//w->posX = 90;
	//w->posY = 56;
	s = "BACK";
	w->font_size = &FONT_8X14_reduced;
	w->posX = 94;
	w->posY = 50;
	strcpy(w->displayString, s);
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 3;
	w->buttonWidget.action = &cancelSleep;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	//w->font_size = &FONT_6X8_reduced;
	//w->posX = 2;
	//w->posY = 56;
	w->font_size = &FONT_8X14_reduced;
	w->posX = 1;
	w->posY = 50;
	s = "SAVE";
	strcpy(w->displayString, s);
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 4;
	w->buttonWidget.action = &saveSleep;

	//---------------------------------------------------------------------------------------------
	// Edit boost screen
	sc = oled_addScreen(screen_edit_boost);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &default_init;
	sc->update = &default_screenUpdate;
	w = screen_addWidget(sc);

	widgetDefaultsInit(w, widget_label);
	s = "BOOST";
	strcpy(w->displayString, s);
	w->posX = 50;
	w->posY = 0;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 5;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	s = "Time(s):";
	strcpy(w->displayString, s);
	w->posX = 30;
	w->posY = 22;
	w->font_size = &FONT_6X8_reduced;
	w->reservedChars = 8;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	s = "Temp(C):";
	strcpy(w->displayString, s);
	w->posX = 30;
	w->posY = 35;
	w->font_size = &FONT_6X8_reduced;
	w->reservedChars = 8;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 85;
	w->posY = 22;
	w->font_size = &FONT_6X8_reduced;
	w->editable.inputData.getData = &getBoostTime;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 10;
	w->editable.step = 10;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setBoostTime;
	w->editable.max_value = 600;
	w->editable.min_value = 10;
	w->reservedChars = 3;
	w->displayWidget.justify=justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 85;
	w->posY = 35;
	w->font_size = &FONT_6X8_reduced;
	w->editable.inputData.getData = &getBoostTemp;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 10;
	w->editable.step = 5;
	w->editable.selectable.tab = 1;
	w->editable.setData = (void (*)(void *))&setBoostTemp;
	w->editable.max_value = 480;
	w->editable.min_value = 200;
	w->reservedChars = 3;
	w->displayWidget.justify=justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 94;
	w->posY = 50;
	s = "BACK";
	strcpy(w->displayString, s);
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 2;
	w->buttonWidget.action = &cancelBoost;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 1;
	w->posY = 50;
	s = "SAVE";
	strcpy(w->displayString, s);
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 3;
	w->buttonWidget.action = &saveBoost;

	//-------------------------------------------------------------------------------------------------
	// Edit contrast screen
	sc = oled_addScreen(screen_edit_contrast);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &default_init;
	sc->update = &default_screenUpdate;
	w = screen_addWidget(sc);

	widgetDefaultsInit(w, widget_label);
	s = "CONTRAST";
	strcpy(w->displayString, s);
	w->posX = 32;
	w->posY = 0;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 8;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 40;
	w->posY = 20;
	w->font_size = &FONT_16X26_reduced;
	w->editable.inputData.getData = &getContrast_;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 50;
	w->editable.step = 10;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setContrast_;
	w->editable.max_value = 255;
	w->editable.min_value = 10;
	w->reservedChars = 3;
	w->displayWidget.justify=justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 94;
	w->posY = 50;
	s = "BACK";
	strcpy(w->displayString, s);
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 1;
	w->buttonWidget.action = &cancelContrast;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 1;
	w->posY = 50;
	s = "SAVE";
	strcpy(w->displayString, s);
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 2;
	w->buttonWidget.action = &saveContrast;

	//-------------------------------------------------------------------------------------------------------------
	// Edit iron tips screen
	sc = oled_addScreen(screen_edit_iron_tips);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &edit_iron_screen_init;
	sc->update = &default_screenUpdate;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	s = "TIPS";
	strcpy(w->displayString, s);
	w->posX = 48;
	w->posY = 0;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 4;
	//
	w = screen_addWidget(sc);
	tipCombo = w;
	widgetDefaultsInit(w, widget_combo);
	w->posY = 17;
	w->posX = 0;
	w->font_size = &FONT_6X8_reduced;
	for(int x = 0; x < sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0]); ++x) {
		char *t = malloc(sizeof(systemSettings.ironTips[0].name)/sizeof(systemSettings.ironTips[0].name[0]));
		t[0] = '\0';
		if(!t)
		   Error_Handler();
		comboAddItem(w, t, screen_edit_tip_name);
	}
	addNewTipComboItem = comboAddItem(w, "ADD NEW", screen_edit_tip_name);
	comboAddItem(w, "EXIT", screen_settings);
	sc->current_widget = tipCombo;

	//Screen edit iron tip
	sc = oled_addScreen(screen_edit_tip_name);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &edit_iron_tip_screen_init;
	sc->update = &default_screenUpdate;
	w = screen_addWidget(sc);

	widgetDefaultsInit(w, widget_label);
	s = "TIP";
	strcpy(w->displayString, s);
	w->posX = 0;
	w->posY = 0;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 30;
	w->posY = 17;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getTipStr;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_string;
	w->editable.big_step = 10;
	w->editable.step = 1;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setTipStr;
	w->editable.max_value = 9999;
	w->reservedChars = 4;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_6X8_reduced;
	w->posX = 1;
	w->posY = 56;
	s = "SAVE";
	strcpy(w->displayString, s);
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 1;
	w->buttonWidget.action = &saveTip;
	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_6X8_reduced;
	w->posX = 50;
	w->posY = 56;
	s = "BACK";
	strcpy(w->displayString, s);
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 2;
	w->buttonWidget.action = &cancelTip;

	w = screen_addWidget(sc);
	delTipButton = w;
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_6X8_reduced;
	w->posX = 90;
	w->posY = 56;
	s = "DELETE";
	strcpy(w->displayString, s);
	w->reservedChars = 6;
	w->buttonWidget.selectable.tab = 3;
	w->buttonWidget.action = &delTip;
}
