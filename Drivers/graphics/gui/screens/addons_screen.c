/*
 * addons_screen.c
 *
 *  Created on: 2022. ápr. 19.
 *      Author: KocsisV
 */

#include "addons_screen.h"
#include "screen_common.h"

#ifdef ENABLE_ADDONS

screen_t Screen_addons;

static void addon_screen_create(screen_t *scr)
{
  widget_t* w;

  newWidget(&w,widget_combo,scr,NULL);

#ifdef ENABLE_ADDON_FUME_EXTRACTOR
  newComboScreen(w, strings[lang].FUME_EXTRACTOR_Title, screen_fume_extractor_settings, NULL);
#endif

#ifdef ENABLE_ADDON_SWITCH_OFF_REMINDER
  newComboScreen(w, strings[lang].SWITCH_OFF_REMINDER_Title, screen_switch_off_reminder_settings, NULL);
#endif

  newComboScreen(w, strings[lang]._BACK, screen_settings, NULL);
}

static void addon_screen_onEnter(screen_t *scr)
{
   if(scr==&Screen_settings){					              // Reset index only if coming from settings screen, otherwise keep the last position
     comboResetIndex(Screen_addons.current_widget);
   }
}

void addons_screen_setup(screen_t *scr)
{
  scr->create       = &addon_screen_create;
  scr->onEnter      = &addon_screen_onEnter;
  scr->processInput = &autoReturn_ProcessInput;
}

#endif
