#include "global.h"
#include "gflib.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_gfx_sfx_util.h"
#include "battle_interface.h"
#include "battle_tower.h"
#include "berry_pouch.h"
#include "data.h"
#include "decompress.h"
#include "easy_chat.h"
#include "event_data.h"
#include "evolution_scene.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "field_fadetransition.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "fldeff.h"
#include "graphics.h"
#include "help_message.h"
#include "help_system.h"
#include "item.h"
#include "item_menu.h"
#include "item_use.h"
#include "link.h"
#include "link_rfu.h"
#include "load_save.h"
#include "mail.h"
#include "mail_data.h"
#include "main.h"
#include "menu.h"
#include "menu_helpers.h"
#include "new_menu_helpers.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "party_menu.h"
#include "player_pc.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "pokemon_jump.h"
#include "pokemon_special_anim.h"
#include "pokemon_summary_screen.h"
#include "quest_log.h"
#include "region_map.h"
#include "reshow_battle_screen.h"
#include "scanline_effect.h"
#include "script.h"
#include "start_menu.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "teachy_tv.h"
#include "text_window.h"
#include "tm_case.h"
#include "trade.h"
#include "union_room.h"
#include "constants/battle.h"
#include "constants/easy_chat.h"
#include "constants/field_effects.h"
#include "constants/item_effects.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/moves.h"
#include "constants/pokemon.h"
#include "constants/quest_log.h"
#include "constants/songs.h"
#include "constants/sound.h"

#define MENU_DIR_DOWN     1
#define MENU_DIR_UP      -1
#define MENU_DIR_RIGHT    2
#define MENU_DIR_LEFT    -2

extern void MoveCursorToConfirm(void);
static u16 PartyMenuButtonHandler(s8 *slotPtr);
static s8 *GetCurrentPartySlotPtr(void);
extern void HandleChooseMonSelection(u8 taskId, s8 *slotPtr);
extern void HandleChooseMonCancel(u8 taskId, s8 *slotPtr);
extern EWRAM_DATA struct PartyMenuInternal * sPartyMenuInternal;
extern void CursorCB_Switch(u8 taskId);
extern void UpdateCurrentPartySelection(s8 *slotPtr, s8 movementDir);
struct PartyMenuInternal 
 { 
     TaskFunc task; 
     MainCallback exitCallback; 
     u32 chooseHalf:1; 
     u32 lastSelectedSlot:3;  // Used to return to same slot when going left/right bewtween columns 
     u32 spriteIdConfirmPokeball:7; 
     u32 spriteIdCancelPokeball:7; 
     u32 messageId:14; 
     u8 windowId[3]; 
     u8 actions[8]; 
     u8 numActions; 
     u16 palBuffer[BG_PLTT_SIZE / sizeof(u16)]; 
     s16 data[16]; 
 };

void Task_HandleChooseMonInput(u8 taskId) 
 { 
     if (!gPaletteFade.active && sub_80BF748() != TRUE) 
     { 
         s8 *slotPtr = GetCurrentPartySlotPtr(); 
  
         switch (PartyMenuButtonHandler(slotPtr)) 
         { 
         case 1: // Selected mon 
             HandleChooseMonSelection(taskId, slotPtr); 
             break; 
         case 2: // Selected Cancel 
             HandleChooseMonCancel(taskId, slotPtr); 
             break; 
         case 8: // Start button 
             if (sPartyMenuInternal->chooseHalf) 
             { 
                 PlaySE(SE_SELECT); 
                 MoveCursorToConfirm(); 
             } 
             break; 
         case 9: 
             DestroyTask(taskId); 
             break; 
         } 
     } 
 }


static u16 PartyMenuButtonHandler(s8 *slotPtr) 
 { 
     s8 movementDir; 
     u8 taskId; 
  
     switch (gMain.newAndRepeatedKeys) 
     { 
     case DPAD_UP: 
         movementDir = MENU_DIR_UP; 
         break; 
     case DPAD_DOWN: 
         movementDir = MENU_DIR_DOWN; 
         break; 
     case DPAD_LEFT: 
         movementDir = MENU_DIR_LEFT; 
         break; 
     case DPAD_RIGHT: 
         movementDir = MENU_DIR_RIGHT; 
         break; 
     default: 
         switch (GetLRKeysPressedAndHeld()) 
         { 
         case MENU_L_PRESSED: 
             movementDir = MENU_DIR_UP; 
             break; 
         case MENU_R_PRESSED: 
             movementDir = MENU_DIR_DOWN; 
             break; 
         default: 
             movementDir = 0; 
             break; 
         } 
         break; 
     } 
     if (JOY_NEW(START_BUTTON)) 
         return 8; 
     if (JOY_NEW(SELECT_BUTTON) && CalculatePlayerPartyCount() > 1) 
     { 
         if(gPartyMenu.menuType != PARTY_MENU_TYPE_FIELD) 
             return 8; 
         if(*slotPtr == PARTY_SIZE + 1) 
             return 8; // do nothing if select is pressed on Cancel 
         if(gPartyMenu.action != PARTY_ACTION_SWITCH) 
         { 
             taskId = CreateTask(CursorCB_Switch, 1); 
             return 9; 
         } 
         return 1; //select acts as A button when in switch mode 
     } 
     if (movementDir) 
     { 
         UpdateCurrentPartySelection(slotPtr, movementDir); 
         return 0; 
     } 
     // Pressed Cancel 
     if (JOY_NEW(A_BUTTON) && *slotPtr == PARTY_SIZE + 1) 
         return 2; 
     return JOY_NEW(A_BUTTON | B_BUTTON); 
 }

static s8 *GetCurrentPartySlotPtr(void)
{
    if (gPartyMenu.action == PARTY_ACTION_SWITCH || gPartyMenu.action == PARTY_ACTION_SOFTBOILED)
        return &gPartyMenu.slotId2;
    else
        return &gPartyMenu.slotId;
}