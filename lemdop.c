/* SPDX-License-Identifier: GPL-3.0-or-later */
/* lemdop v1.0 (December 2022)
 * Copyright (C) 2016-2022 Norbert de Jonge <nlmdejonge@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see [ www.gnu.org/licenses/ ].
 *
 * To properly read this code, set your program's tab stop to: 2.
 */

/*========== Includes ==========*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#if defined WIN32 || _WIN32 || WIN64 || _WIN64
#include <windows.h>
#undef PlaySound
#endif

#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_thread.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
/*========== Includes ==========*/

/*========== Defines ==========*/
#if defined WIN32 || _WIN32 || WIN64 || _WIN64
#define SLASH "\\"
#define DEVNULL "NUL"
#else
#define SLASH "/"
#define DEVNULL "/dev/null"
#endif

#define EXIT_NORMAL 0
#define EXIT_ERROR 1
#define EDITOR_NAME "lemdop"
#define EDITOR_VERSION "v1.0 (December 2022)"
#define COPYRIGHT "Copyright (C) 2022 Norbert de Jonge"
#define TILES 30
#define ROM_DIR "rom"
#define BACKUP ROM_DIR SLASH "rom.bak"
#define MAX_PATHFILE 200
#define MAX_TOWRITE 720
#define WINDOW_WIDTH 640 + 2 + 50 /*** 692 ***/
#define WINDOW_HEIGHT 390 + 2 + 75 /*** 467 ***/
#define MAX_IMG 200
#define MAX_CON 30
#define REFRESH 25 /*** That is 40 frames per second, 1000/25. ***/
#define FONT_SIZE_15 15
#define FONT_SIZE_11 11
#define FONT_SIZE_20 20
#define NUM_SOUNDS 20 /*** Sounds that may play at the same time. ***/
#define MAX_TEXT 100
#define MAX_OPTION 100
#define MAX_ERROR 200
#define MAX_INFO 200
#define BLUE 0
#define BROWN 1

#define NR_LEVELS_US 13
#define NR_LEVELS_EU 17
#define OFFSET_PRINCE_US 0x20140
#define OFFSET_PRINCE_EU 0x504B2
#define OFFSET_LEVELS_US 0x6A5C
#define OFFSET_LEVELS_EU 0x358C2
#define OFFSET_GR_OB_US 0x715A
#define OFFSET_GR_OB_EU 0x36020
#define OFFSET_GUARDS_US 0x1FAC
#define OFFSET_GUARDS_EU 0x2B02
#define OFFSET_DOORS_US 0x41DE
#define OFFSET_DOORS_EU 0x46C2
#define OFFSET_GATES_US 0x5B96
#define OFFSET_GATES_EU 0x61CE
#define OFFSET_LOOSE_US 0x1CD3E
#define OFFSET_LOOSE_EU 0x4CF3C
#define OFFSET_RAISE_US 0x1E592
#define OFFSET_RAISE_EU 0x4E8F6
#define OFFSET_DROP_US 0x1EBFC
#define OFFSET_DROP_EU 0x4F138
#define OFFSET_CHOMPERS_US 0x228DE
#define OFFSET_CHOMPERS_EU 0x52D86
#define OFFSET_SPIKES_US 0x25F94
#define OFFSET_SPIKES_EU 0x56612
#define OFFSET_POTIONS_US 0x27BAE
#define OFFSET_POTIONS_EU 0x581B0

#define MAX_BYTES 0x5000
#define MAX_LEVELS 17 /*** US = 13, EU = 17 ***/
#define MAX_ROOMS 525 /*** 541 - (17 - 1) ***/
#define WIDTH 10
#define HEIGHT 3
#define OFFSET_REGION 0x01F0
#define BAR_FULL 443
#define MAX_GUARDS MAX_ROOMS
#define MAX_DOORS MAX_ROOMS * WIDTH * HEIGHT
#define MAX_GATES MAX_ROOMS * WIDTH * HEIGHT
#define MAX_LOOSE MAX_ROOMS * WIDTH * HEIGHT
#define MAX_RAISE MAX_ROOMS * WIDTH * HEIGHT
#define MAX_DROP MAX_ROOMS * WIDTH * HEIGHT
#define MAX_CHOMPER MAX_ROOMS * WIDTH * HEIGHT
#define MAX_SPIKE MAX_ROOMS * WIDTH * HEIGHT
#define MAX_POTION MAX_ROOMS * WIDTH * HEIGHT
#define MAX_TEMP MAX_ROOMS * WIDTH * HEIGHT

#define VERIFY_OFFSET1 0x120
#define VERIFY_OFFSET2 0x150
#define VERIFY_TEXT "PRINCE OF PERSIA"
#define VERIFY_SIZE 16

#define PNG_VARIOUS "various"
#define PNG_LIVING "living"
#define PNG_SLIVING "sliving"
#define PNG_BUTTONS "buttons"
#define PNG_EXTRAS "extras"
#define PNG_GAMEPAD "gamepad"
#define PNG_DUNGEON "dungeon"
#define PNG_PALACE "palace"
#define PNG_TABS "tabs"
#define PNG_MINIATURES "miniatures"

#define OFFSETD_X 25 /*** Pixels from the left, where tiles are visible. ***/
#define OFFSETD_Y 50 /*** Pixels from the top, where tiles are visible. ***/
#define TTPD_1 -6 /*** Top row, pixels behind interface. ***/
#define TTPD_O 0 /*** Other rows, pixels behind superjacent rows. ***/
#define DD_X 64 /*** Horizontal distance between (overlapping) tiles. ***/
#define DD_Y 128 /*** Vertical distance between (overlapping) tiles. ***/

#define TILEWIDTH 63 /*** On tiles screen. ***/
#define TILEHEIGHT 79 /*** On tiles screen. ***/

#define ALLOWED_US_ROOMS 459
#define ALLOWED_US_GUARDS 38
#define ALLOWED_US_DOORS 22
#define ALLOWED_US_GATES 77
#define ALLOWED_US_LOOSE 94
#define ALLOWED_US_RAISE 90
#define ALLOWED_US_DROP 24
#define ALLOWED_US_CHOMPERS 36
#define ALLOWED_US_SPIKES 102
#define ALLOWED_US_POTIONS 45
#define ALLOWED_EU_ROOMS 541
#define ALLOWED_EU_GUARDS 51
#define ALLOWED_EU_DOORS 30
#define ALLOWED_EU_GATES 105
#define ALLOWED_EU_LOOSE 122
#define ALLOWED_EU_RAISE 122
#define ALLOWED_EU_DROP 39
#define ALLOWED_EU_CHOMPERS 54
#define ALLOWED_EU_SPIKES 138
#define ALLOWED_EU_POTIONS 59

#ifndef O_BINARY
#define O_BINARY 0
#endif
/*========== Defines ==========*/

int iDebug;
char sPathFile[MAX_PATHFILE + 2];
int iChanged;
int iScreen;
TTF_Font *font1;
TTF_Font *font2;
TTF_Font *font3;
SDL_Window *window;
SDL_Renderer *ascreen;
int iScale;
int iFullscreen;
SDL_Cursor *curArrow;
SDL_Cursor *curWait;
SDL_Cursor *curHand;
int iNoAudio;
int iNoController;
int iPreLoaded;
int iNrToPreLoad;
int iCurrentBarHeight;
int iDownAt;
int iSelectedX;
int iSelectedY;
int iCurLevel;
int iExtras;
int iCurX, iCurY;
int iLastObject, iLastGraphics;
int iXPos, iYPos;
int iInfo;
unsigned int gamespeed;
Uint32 looptime;
int iStartRoomsX, iStartRoomsY;
int iOnTile;
int iCloseOn;
int iHelpOK;
int iEXESave;
int iOKOn;
int iYesOn;
int iNoOn;
int iStartLevel;
int iCustomHover, iCustomHoverOld;
int iMednafen;
char sInfo[MAX_INFO + 2];

/*** Set by SetTypeDefaults(), used by LoadLevels() and SaveLevels(). ***/
int iNrLevels;
int iOffsetPrince;
int iOffsetLevels;
int iOffsetGrOb;
int iOffsetGuards;
int iOffsetDoors;
int iOffsetGates;
int iOffsetLoose;
int iOffsetRaise;
int iOffsetDrop;
int iOffsetChompers;
int iOffsetSpikes;
int iOffsetPotions;

int iEXEType;
int iNewObjectLeft, iNewObject, iNewObjectRight;
int iNewGraphicsLeft, iNewGraphics, iNewGraphicsRight;
int iOldGraphicsX, iOldGraphicsY, iOldGraphicsTab;
int iTabObject, iTabGraphics;
int EnvTabRow[2 + 2][15 + 2][2 + 2][15 + 2];
int iModified;
int iNewSkill, iNewHP;

/*** EXE ***/
int iEXEStartingMin;
int iEXEStartingSec;
int iEXEStartingHP;
int iEXEStartingLevel;

/*** for text ***/
SDL_Color color_bl = {0x00, 0x00, 0x00, 255};
SDL_Color color_wh = {0xff, 0xff, 0xff, 255};
SDL_Color color_blue = {0x00, 0x00, 0xff, 255};
SDL_Color color_green = {0x00, 0xff, 0x00, 255};
SDL_Color color_red = {0xff, 0x00, 0x00, 255};
SDL_Surface *message;
SDL_Texture *messaget;
SDL_Rect offset;

/*** controller ***/
int iController;
SDL_GameController *controller;
char sControllerName[MAX_CON + 2];
SDL_Joystick *joystick;
SDL_Haptic *haptic;
Uint32 joyleft, joyright, joyup, joydown;
Uint32 trigleft, trigright;
int iXJoy1, iYJoy1, iXJoy2, iYJoy2;

/*** These are the prince. ***/
int arPrinceYP[MAX_LEVELS + 2];
int arPrinceY[MAX_LEVELS + 2];
int arPrinceXP[MAX_LEVELS + 2];
int arPrinceX[MAX_LEVELS + 2];
int arPrinceDir[MAX_LEVELS + 2];

/*** These are the levels. ***/
int arLevelHeightP[MAX_LEVELS + 2];
int arLevelHeight[MAX_LEVELS + 2];
int arLevelWidthP[MAX_LEVELS + 2];
int arLevelWidth[MAX_LEVELS + 2];
int arLevelNrTiles[MAX_LEVELS + 2];
int arLevelOffsetObjects[MAX_LEVELS + 2];
int arLevelOffsetGraphics[MAX_LEVELS + 2];
int arLevelStartingY[MAX_LEVELS + 2];
int arLevelStartingX[MAX_LEVELS + 2];
int arLevelType[MAX_LEVELS + 2];
unsigned char arLevelObjects[MAX_LEVELS + 2]
	[(MAX_ROOMS * WIDTH) + 2][(MAX_ROOMS * HEIGHT) + 2];
unsigned char arLevelGraphics[MAX_LEVELS + 2]
	[(MAX_ROOMS * WIDTH) + 2][(MAX_ROOMS * HEIGHT) + 2];

/*** These are the guards. ***/
int arNrGuards[MAX_LEVELS + 2];
int arOffsetGuards[MAX_LEVELS + 2];
int arGuardYP[MAX_LEVELS + 2][MAX_GUARDS + 2];
int arGuardY[MAX_LEVELS + 2][MAX_GUARDS + 2];
int arGuardXP[MAX_LEVELS + 2][MAX_GUARDS + 2];
int arGuardX[MAX_LEVELS + 2][MAX_GUARDS + 2];
int arGuardDir[MAX_LEVELS + 2][MAX_GUARDS + 2];
int arGuardSprite[MAX_LEVELS + 2][MAX_GUARDS + 2];
int arGuardType[MAX_LEVELS + 2][MAX_GUARDS + 2];
int arGuardSkill[MAX_LEVELS + 2][MAX_GUARDS + 2];
int arGuardHP[MAX_LEVELS + 2][MAX_GUARDS + 2];

/*** These are the (level) doors. ***/
int arNrDoors[MAX_LEVELS + 2];
int arOffsetDoors[MAX_LEVELS + 2];
int arDoorType[MAX_LEVELS + 2][MAX_DOORS + 2];
int arDoorYP[MAX_LEVELS + 2][MAX_DOORS + 2];
int arDoorY[MAX_LEVELS + 2][MAX_DOORS + 2];
int arDoorXP[MAX_LEVELS + 2][MAX_DOORS + 2];
int arDoorX[MAX_LEVELS + 2][MAX_DOORS + 2];

/*** These are the gates. ***/
int arNrGates[MAX_LEVELS + 2];
int arOffsetGates[MAX_LEVELS + 2];
int arGateState1[MAX_LEVELS + 2][MAX_GATES + 2];
int arGateYP[MAX_LEVELS + 2][MAX_GATES + 2];
int arGateY[MAX_LEVELS + 2][MAX_GATES + 2];
int arGateXP[MAX_LEVELS + 2][MAX_GATES + 2];
int arGateX[MAX_LEVELS + 2][MAX_GATES + 2];
int arGateState2[MAX_LEVELS + 2][MAX_GATES + 2];
int arGateState3[MAX_LEVELS + 2][MAX_GATES + 2];
int arGateUnk[MAX_LEVELS + 2][MAX_GATES + 2];

/*** These are the loose (floors). ***/
int arNrLoose[MAX_LEVELS + 2];
int arOffsetLoose[MAX_LEVELS + 2];
int arLooseYP[MAX_LEVELS + 2][MAX_LOOSE + 2];
int arLooseY[MAX_LEVELS + 2][MAX_LOOSE + 2];
int arLooseXP[MAX_LEVELS + 2][MAX_LOOSE + 2];
int arLooseX[MAX_LEVELS + 2][MAX_LOOSE + 2];

/*** These are the raise (buttons). ***/
int arNrRaise[MAX_LEVELS + 2];
int arOffsetRaise[MAX_LEVELS + 2];
int arRaiseYP[MAX_LEVELS + 2][MAX_RAISE + 2];
int arRaiseY[MAX_LEVELS + 2][MAX_RAISE + 2];
int arRaiseXP[MAX_LEVELS + 2][MAX_RAISE + 2];
int arRaiseX[MAX_LEVELS + 2][MAX_RAISE + 2];
int arRaiseGate1[MAX_LEVELS + 2][MAX_RAISE + 2];
int arRaiseGate2[MAX_LEVELS + 2][MAX_RAISE + 2];
int arRaiseGate3[MAX_LEVELS + 2][MAX_RAISE + 2];

/*** These are the drop (buttons). ***/
int arNrDrop[MAX_LEVELS + 2];
int arOffsetDrop[MAX_LEVELS + 2];
int arDropYP[MAX_LEVELS + 2][MAX_DROP + 2];
int arDropY[MAX_LEVELS + 2][MAX_DROP + 2];
int arDropXP[MAX_LEVELS + 2][MAX_DROP + 2];
int arDropX[MAX_LEVELS + 2][MAX_DROP + 2];
int arDropGate1[MAX_LEVELS + 2][MAX_DROP + 2];
int arDropGate2[MAX_LEVELS + 2][MAX_DROP + 2];
int arDropGate3[MAX_LEVELS + 2][MAX_DROP + 2];

/*** These are the chompers. ***/
int arNrChompers[MAX_LEVELS + 2];
int arOffsetChompers[MAX_LEVELS + 2];
int arChomperYP[MAX_LEVELS + 2][MAX_CHOMPER + 2];
int arChomperY[MAX_LEVELS + 2][MAX_CHOMPER + 2];
int arChomperXP[MAX_LEVELS + 2][MAX_CHOMPER + 2];
int arChomperX[MAX_LEVELS + 2][MAX_CHOMPER + 2];

/*** These are the spikes. ***/
int arNrSpikes[MAX_LEVELS + 2];
int arOffsetSpikes[MAX_LEVELS + 2];
int arSpikeYP[MAX_LEVELS + 2][MAX_SPIKE + 2];
int arSpikeY[MAX_LEVELS + 2][MAX_SPIKE + 2];
int arSpikeXP[MAX_LEVELS + 2][MAX_SPIKE + 2];
int arSpikeX[MAX_LEVELS + 2][MAX_SPIKE + 2];

/*** These are the potions. ***/
int arNrPotions[MAX_LEVELS + 2];
int arOffsetPotions[MAX_LEVELS + 2];
int arPotionColor[MAX_LEVELS + 2][MAX_POTION + 2];
int arPotionYP[MAX_LEVELS + 2][MAX_POTION + 2];
int arPotionY[MAX_LEVELS + 2][MAX_POTION + 2];
int arPotionXP[MAX_LEVELS + 2][MAX_POTION + 2];
int arPotionX[MAX_LEVELS + 2][MAX_POTION + 2];
int arPotionEffect[MAX_LEVELS + 2][MAX_POTION + 2];

/*** These are used for removing and adding attributes. ***/
int iNrTemp;
int iTempAttr1[MAX_TEMP + 2];
int iTempAttr2[MAX_TEMP + 2];
int iTempAttr3[MAX_TEMP + 2];
int iTempAttr4[MAX_TEMP + 2];
int iTempAttr5[MAX_TEMP + 2];
int iTempAttr6[MAX_TEMP + 2];
int iTempAttr7[MAX_TEMP + 2];
int iTempAttr8[MAX_TEMP + 2];
int iTempAttr9[MAX_TEMP + 2];

/*** Values on the object tabs. ***/
int iRaiseGate1Check;
int iRaiseGate1;
int iRaiseGate2Check;
int iRaiseGate2;
int iRaiseGate3Check;
int iRaiseGate3;
int iDropGate1Check;
int iDropGate1;
int iDropGate2Check;
int iDropGate2;
int iDropGate3Check;
int iDropGate3;
int iGateStateCheck;
int iGateState;
int iGateOpenness;
int iGateDelay;
int iPotionColorCheck;
int iPotionColor;
int iPotionEffectCheck;
int iPotionEffect;
int iDoorTypeCheck;
int iDoorType;

int iDX, iDY, iTTP1, iTTPO;
int iHor[10 + 2];
int iVer0, iVer1, iVer2, iVer3, iVer4;

SDL_Texture *imgloading;
SDL_Texture *imgo[0xFF + 2][2 + 2];
SDL_Texture *imgd[0xFF + 2][2 + 2];
SDL_Texture *imgp[0xFF + 2][2 + 2];
SDL_Texture *imgm[12 + 2];
SDL_Texture *imgtabo[12 + 2];
SDL_Texture *imgtabgd[15 + 2];
SDL_Texture *imgtabgp[15 + 2];
SDL_Texture *imgblack;
SDL_Texture *imgprincel[2 + 2], *imgprincer[2 + 2];
SDL_Texture *imgguardl[2 + 2], *imgguardr[2 + 2];
SDL_Texture *imgskeletonl[2 + 2], *imgskeletonr[2 + 2];
SDL_Texture *imgfatl[2 + 2], *imgfatr[2 + 2];
SDL_Texture *imgshadowl[2 + 2], *imgshadowr[2 + 2];
SDL_Texture *imgjaffarl[2 + 2], *imgjaffarr[2 + 2];
SDL_Texture *imgunkobject[2 + 2];
SDL_Texture *imgunkgraphics[2 + 2];
SDL_Texture *imgup_0;
SDL_Texture *imgup_1;
SDL_Texture *imgdown_0;
SDL_Texture *imgdown_1;
SDL_Texture *imgleft_0;
SDL_Texture *imgleft_1;
SDL_Texture *imgright_0;
SDL_Texture *imgright_1;
SDL_Texture *imgudno;
SDL_Texture *imglrno;
SDL_Texture *imgudnonfo;
SDL_Texture *imgprevon_0;
SDL_Texture *imgprevon_1;
SDL_Texture *imgnexton_0;
SDL_Texture *imgnexton_1;
SDL_Texture *imgprevoff;
SDL_Texture *imgnextoff;
SDL_Texture *imgbar;
SDL_Texture *imgextras[10 + 2];
SDL_Texture *imgroomson_0;
SDL_Texture *imgroomson_1;
SDL_Texture *imgroomsoff;
SDL_Texture *imgenumon_0;
SDL_Texture *imgenumon_1;
SDL_Texture *imgenumoff;
SDL_Texture *imgsaveon_0;
SDL_Texture *imgsaveon_1;
SDL_Texture *imgsaveoff;
SDL_Texture *imgquit_0;
SDL_Texture *imgquit_1;
SDL_Texture *imgrooms;
SDL_Texture *imgsrc;
SDL_Texture *imgsrs;
SDL_Texture *imgenumerate;
SDL_Texture *imgchkbl;
SDL_Texture *imgtiles;
SDL_Texture *imgclosebig_0;
SDL_Texture *imgclosebig_1;
SDL_Texture *imgborderb;
SDL_Texture *imgborders;
SDL_Texture *imgbordersl;
SDL_Texture *imgborderbl;
SDL_Texture *imgfadedl;
SDL_Texture *imgpopup;
SDL_Texture *imgok[2 + 2];
SDL_Texture *imgsave[2 + 2];
SDL_Texture *imgpopup_yn;
SDL_Texture *imgyes[2 + 2];
SDL_Texture *imgno[2 + 2];
SDL_Texture *imghelp;
SDL_Texture *imgexe;
SDL_Texture *imgfadeds;
SDL_Texture *imgchover;
SDL_Texture *imgmednafen;
SDL_Texture *imgeuonly;
SDL_Texture *imgstatus0;
SDL_Texture *imgstatus1;

struct sample {
	Uint8 *data;
	Uint32 dpos;
	Uint32 dlen;
} sounds[NUM_SOUNDS];

void ShowUsage (void);
void GetPathFile (void);
void LoadLevels (void);
void StringToUpper (char *sInput, char *sOutput);
void SaveLevels (void);
void PrintTileCombo (int iObjectValue, int iGraphicsValue);
void PrIfDe (char *sString);
void Quit (void);
void InitScreen (void);
void InitPopUpSave (void);
void ShowPopUpSave (void);
void LoadFonts (void);
void MixAudio (void *unused, Uint8 *stream, int iLen);
void PlaySound (char *sFile);
void PreLoadSet (int iTile);
void PreLoad (char *sPath, char *sPNG, SDL_Texture **imgImage);
void ShowScreen (void);
void InitPopUp (void);
void ShowPopUp (void);
void Help (void);
void ShowHelp (void);
void EXE (void);
void ShowEXE (void);
void InitScreenAction (char *sAction);
void RunLevel (int iLevel);
int StartGame (void *unused);
void ClearRoom (int iRoomX, int iRoomY);
void UseTile (int iTile, int iX, int iY);
void Zoom (int iToggleFull);
void ChangeCustom (int iAmount, int *iWhat, int iMin, int iMax);
void Prev (int iDiscard);
void Next (int iDiscard);
void CallSave (void);
void SetLocation (int iX, int iY, int iObject, int iGraphics);
void SetAttribute (int iX, int iY, int *arX, int *arY, int *arAttr,
	int iNrObjects, int iValue);
int HasObject (int iX, int iY, int *arX, int *arY, int iNrObjects);
int InArea (int iUpperLeftX, int iUpperLeftY,
	int iLowerRightX, int iLowerRightY);
int OnLevelBar (void);
void ChangePos (void);
void ShowImage (SDL_Texture *img, int iX, int iY, char *sImageInfo);
void CustomRenderCopy (SDL_Texture* src, SDL_Rect* srcrect,
	SDL_Rect *dstrect, char *sImageInfo);
void CreateBAK (void);
void DisplayText (int iStartX, int iStartY, int iFontSize,
	char arText[9 + 2][MAX_TEXT + 2], int iLines, TTF_Font *font,
	SDL_Color back, SDL_Color fore);
void ShowRooms (void);
void ShowRoom (int iX, int iY);
void ShowChange (void);
int OnTile (void);
void ChangePosAction (char *sAction);
int IsDisabled (int iTile);
void CenterNumber (int iNumber, int iX, int iY,
	SDL_Color fore, int iHex);
int Unused (int iTile);
void OpenURL (char *sURL);
void EXELoad (void);
void EXESave (void);
int PlusMinus (int *iWhat, int iX, int iY,
	int iMin, int iMax, int iChange, int iAddChanged);
void GetOptionValue (char *sArgv, char *sValue);
void ObjectWarn (int iHave, int iNeed);
void LoadingBar (int iBarHeight);
const char* GateAsText (int iGate);
void SetEnvTabRow (int iEnv, int iTab, int iRow,
	int iG1, int iG2, int iG3, int iG4, int iG5, int iG6, int iG7, int iG8,
	int iG9, int iG10, int iG11, int iG12, int iG13, int iG14, int iG15);
void SetEnvTabRows (void);
int OnGraphics (void);
void UpdateOnTile (void);
void GetNrsAndOffsets (int iFd, int *arNr, int *arOffset,
	int iObjectSize, char *sObjectName);
void SetNrsAndOffsets (int iFd, int *arNr, int iStartOffset, int iObjectSize);
const char* ColorAsText (int iColor);
const char* EffectAsText (int iEffect);
const char* StateAsText (int iS1, int iS2, int iS3);
int GetSelectedTileValue (int *arAttr, int *arX, int *arY,
	int iNrObjects, int iDefault);
int GetAttribute (int iX, int iY, int *arX, int *arY, int *arAttr,
	int iNrObjects);
void ModifyStart (int iLevel, int iToFrom);
int Total (char *sType);
void TotalLine (char *sType, int iAllowed, int iVer);
int IsSavingAllowed (void);
void SetTypeDefaults (void);
void WriteByte (int iFd, int iValue);
void WriteWord (int iFd, int iValue);
void WriteDWord (int iFd, int iValue);
int LoadYBottom (int iYCoor);
int SaveYBottom (int iY);
int LoadYTop (int iYCoor);
int SaveYTop (int iY);
void TempAttributes (int iObject);
void AddRemoveAttributes (int iObject, int iX, int iY, int *arNr,
	int *arX, int *arY, int iType);
void AttributeDefaults (int iObject);
void SetCheck (char *sObject, int iX, int iY, int iCheck, int iValue);
void SetCheckGate (int iX, int iY, int iC, int iState, int iOpenn, int iDelay);
void UpdateSkillHP (void);
void RandomizeLevel (void);
int GuardSprite (int iType);
void SingleSword (int iLevel, int iX, int iY);
void SaveSword (int iFd, int iLevel, int iX, int iY);
void LevelResized (int iOldWidth, int iOldHeight);
void RaiseDropUpdate (int iObjectNr, int iDecInc);

/*****************************************************************************/
int main (int argc, char *argv[])
/*****************************************************************************/
{
	int iArgLoop;
	SDL_version verc, verl;
	time_t tm;
	char sStartLevel[MAX_OPTION + 2];

	iDebug = 0;
	iExtras = 0;
	iLastObject = 0x00;
	iLastGraphics = 0x00;
	iInfo = 0;
	iScale = 1;
	/*** Do not set iOnTile here. ***/
	iNoAudio = 0;
	iFullscreen = 0;
	iNoController = 0;
	iStartLevel = 1;
	iMednafen = 0;
	iModified = 0;

	if (argc > 1)
	{
		for (iArgLoop = 1; iArgLoop <= argc - 1; iArgLoop++)
		{
			if ((strcmp (argv[iArgLoop], "-h") == 0) ||
				(strcmp (argv[iArgLoop], "-?") == 0) ||
				(strcmp (argv[iArgLoop], "--help") == 0))
			{
				ShowUsage();
			}
			else if ((strcmp (argv[iArgLoop], "-v") == 0) ||
				(strcmp (argv[iArgLoop], "--version") == 0))
			{
				printf ("%s %s\n", EDITOR_NAME, EDITOR_VERSION);
				exit (EXIT_NORMAL);
			}
			else if ((strcmp (argv[iArgLoop], "-d") == 0) ||
				(strcmp (argv[iArgLoop], "--debug") == 0))
			{
				iDebug = 1;
			}
			else if ((strcmp (argv[iArgLoop], "-n") == 0) ||
				(strcmp (argv[iArgLoop], "--noaudio") == 0))
			{
				iNoAudio = 1;
			}
			else if ((strcmp (argv[iArgLoop], "-z") == 0) ||
				(strcmp (argv[iArgLoop], "--zoom") == 0))
			{
				iScale = 2;
			}
			else if ((strcmp (argv[iArgLoop], "-f") == 0) ||
				(strcmp (argv[iArgLoop], "--fullscreen") == 0))
			{
				iFullscreen = SDL_WINDOW_FULLSCREEN_DESKTOP;
			}
			else if ((strncmp (argv[iArgLoop], "-l=", 3) == 0) ||
				(strncmp (argv[iArgLoop], "--level=", 8) == 0))
			{
				GetOptionValue (argv[iArgLoop], sStartLevel);
				iStartLevel = atoi (sStartLevel);
				if ((iStartLevel < 1) || (iStartLevel > MAX_LEVELS))
				{
					iStartLevel = 1;
				}
			}
			else if ((strcmp (argv[iArgLoop], "-k") == 0) ||
				(strcmp (argv[iArgLoop], "--keyboard") == 0))
			{
				iNoController = 1;
			}
			else
			{
				ShowUsage();
			}
		}
	}

	GetPathFile();

	srand ((unsigned)time(&tm));

	LoadLevels();

	/*** Show the SDL version used for compiling and linking. ***/
	if (iDebug == 1)
	{
		SDL_VERSION (&verc);
		SDL_GetVersion (&verl);
		printf ("[ INFO ] Compiled with SDL %u.%u.%u, linked with SDL %u.%u.%u.\n",
			verc.major, verc.minor, verc.patch, verl.major, verl.minor, verl.patch);
	}

	SetEnvTabRows();
	InitScreen();
	Quit();

	return 0;
}
/*****************************************************************************/
void ShowUsage (void)
/*****************************************************************************/
{
	printf ("%s %s\n%s\n\n", EDITOR_NAME, EDITOR_VERSION, COPYRIGHT);
	printf ("Usage:\n");
	printf ("  %s [OPTIONS]\n\nOptions:\n", EDITOR_NAME);
	printf ("  -h, -?,    --help           display this help and exit\n");
	printf ("  -v,        --version        output version information and"
		" exit\n");
	printf ("  -d,        --debug          also show levels on the console\n");
	printf ("  -n,        --noaudio        do not play sound effects\n");
	printf ("  -z,        --zoom           double the interface size\n");
	printf ("  -f,        --fullscreen     start in fullscreen mode\n");
	printf ("  -l=NR,     --level=NR       start in level NR\n");
	printf ("  -k,        --keyboard       do not use a game controller\n");
	printf ("\n");
	exit (EXIT_NORMAL);
}
/*****************************************************************************/
void GetPathFile (void)
/*****************************************************************************/
{
	int iFound;
	DIR *dDir;
	struct dirent *stDirent;
	char sExtension[100 + 2];
	char sError[MAX_ERROR + 2];
	int iFd;
	char sVerify1[VERIFY_SIZE + 2], sVerify1Up[VERIFY_SIZE + 2];
	char sVerify2[VERIFY_SIZE + 2], sVerify2Up[VERIFY_SIZE + 2];
	char sEXEType[1 + 2];

	iFound = 0;

	dDir = opendir (ROM_DIR);
	if (dDir == NULL)
	{
		printf ("[FAILED] Cannot open directory \"%s\": %s!\n",
			ROM_DIR, strerror (errno));
		exit (EXIT_ERROR);
	}

	while ((stDirent = readdir (dDir)) != NULL)
	{
		if (iFound == 0)
		{
			if ((strcmp (stDirent->d_name, ".") != 0) &&
				(strcmp (stDirent->d_name, "..") != 0))
			{
				snprintf (sExtension, 100, "%s", strrchr (stDirent->d_name, '.'));
				if ((toupper (sExtension[1]) == 'M') &&
					(toupper (sExtension[2]) == 'D'))
				{
					iFound = 1;
					snprintf (sPathFile, MAX_PATHFILE, "%s%s%s", ROM_DIR, SLASH,
						stDirent->d_name);
					if (iDebug == 1)
					{
						printf ("[  OK  ] Found Mega Drive (Sega Genesis) ROM \"%s\".\n",
							sPathFile);
					}
				}
			}
		}
	}

	closedir (dDir);

	if (iFound == 0)
	{
		snprintf (sError, MAX_ERROR, "Cannot find a .md ROM in"
			" directory \"%s\"!", ROM_DIR);
		printf ("[FAILED] %s\n", sError);
		SDL_ShowSimpleMessageBox (SDL_MESSAGEBOX_ERROR,
			"Error", sError, NULL);
		exit (EXIT_ERROR);
	}

	/*** Is the file accessible? ***/
	if (access (sPathFile, R_OK|W_OK) == -1)
	{
		printf ("[FAILED] Cannot access \"%s\": %s!\n",
			sPathFile, strerror (errno));
		exit (EXIT_ERROR);
	}

	iFd = open (sPathFile, O_RDONLY|O_BINARY);
	if (iFd == -1)
	{
		printf ("[FAILED] Could not open \"%s\": %s!\n",
			sPathFile, strerror (errno));
		exit (EXIT_ERROR);
	}

	/*** Is the file a PoP1 for MD ROM file? ***/
	lseek (iFd, VERIFY_OFFSET1, SEEK_SET);
	read (iFd, sVerify1, VERIFY_SIZE);
	lseek (iFd, VERIFY_OFFSET2, SEEK_SET);
	read (iFd, sVerify2, VERIFY_SIZE);
	sVerify1[VERIFY_SIZE] = '\0';
	sVerify2[VERIFY_SIZE] = '\0';
	StringToUpper (sVerify1, sVerify1Up);
	StringToUpper (sVerify2, sVerify2Up);
	if ((strcmp (sVerify1Up, VERIFY_TEXT) != 0) &&
		(strcmp (sVerify2Up, VERIFY_TEXT) != 0))
	{
		snprintf (sError, MAX_ERROR, "File %s is not a Prince of Persia"
			" for MD ROM!", sPathFile);
		printf ("[FAILED] %s\n", sError);
		SDL_ShowSimpleMessageBox (SDL_MESSAGEBOX_ERROR,
			"Error", sError, NULL);
		exit (EXIT_ERROR);
	}

	/*** Store iEXEType. ***/
	lseek (iFd, OFFSET_REGION, SEEK_SET);
	read (iFd, sEXEType, 1);
	switch (sEXEType[0])
	{
		case 'U': iEXEType = 1; break; /*** US ***/
		case 'E': iEXEType = 2; break; /*** EU ***/
		default:
			printf ("[FAILED] Unknown EXE type: %c!\n", sEXEType[0]);
			exit (EXIT_ERROR); break;
	}
	if (iDebug == 1)
	{
		printf ("[ INFO ] Region (1 = US, 2 = EU): %i\n", iEXEType);
	}
	
	close (iFd);
}
/*****************************************************************************/
void LoadLevels (void)
/*****************************************************************************/
{
	int iFd;
	unsigned char sRead[MAX_BYTES + 2];
	char sReadW[10 + 2];
	char sReadDW[10 + 2];
	int iObjects, iGraphics;
	int iObjectValue, iGraphicsValue;
	int iSprite;

	/*** Used for looping. ***/
	int iLevelLoop;
	int iHeightLoop;
	int iWidthLoop;
	int iGuardLoop;
	int iDoorLoop;
	int iGateLoop;
	int iLooseLoop;
	int iRaiseLoop;
	int iDropLoop;
	int iChomperLoop;
	int iSpikeLoop;
	int iPotionLoop;

	iFd = open (sPathFile, O_RDONLY|O_BINARY);
	if (iFd == -1)
	{
		printf ("[FAILED] Could not open \"%s\": %s!\n",
			sPathFile, strerror (errno));
		exit (EXIT_ERROR);
	}

	SetTypeDefaults();

	/*** Prince ***/
	lseek (iFd, iOffsetPrince, SEEK_SET);
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		/*** Y ***/
		read (iFd, sRead, 2);
		snprintf (sReadW, 10, "%02x%02x", sRead[0], sRead[1]);
		arPrinceYP[iLevelLoop] = strtoul (sReadW, NULL, 16);
		arPrinceY[iLevelLoop] = LoadYBottom (arPrinceYP[iLevelLoop]);

		/*** X ***/
		read (iFd, sRead, 2);
		snprintf (sReadW, 10, "%02x%02x", sRead[0], sRead[1]);
		arPrinceXP[iLevelLoop] = strtoul (sReadW, NULL, 16);
		arPrinceX[iLevelLoop] =
			(arPrinceXP[iLevelLoop] / 32) + 1;

		/*** Dir ***/
		read (iFd, sRead, 2);
		snprintf (sReadW, 10, "%02x%02x", sRead[0], sRead[1]);
		arPrinceDir[iLevelLoop] = strtoul (sReadW, NULL, 16);

		if (iDebug == 1)
		{
			printf ("[ INFO ] Level %i, prince: y=%i (%i), x=%i (%i), dir=%i\n",
				iLevelLoop,
				arPrinceY[iLevelLoop],
				arPrinceYP[iLevelLoop],
				arPrinceX[iLevelLoop],
				arPrinceXP[iLevelLoop],
				arPrinceDir[iLevelLoop]);
		}
	}

	lseek (iFd, iOffsetLevels, SEEK_SET);
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		/*** Height ***/
		read (iFd, sRead, 2);
		snprintf (sReadW, 10, "%02x%02x", sRead[0], sRead[1]);
		arLevelHeightP[iLevelLoop] = strtoul (sReadW, NULL, 16);
		arLevelHeight[iLevelLoop] = arLevelHeightP[iLevelLoop] / 192;

		/*** Width ***/
		read (iFd, sRead, 2);
		snprintf (sReadW, 10, "%02x%02x", sRead[0], sRead[1]);
		arLevelWidthP[iLevelLoop] = strtoul (sReadW, NULL, 16);
		arLevelWidth[iLevelLoop] = arLevelWidthP[iLevelLoop] / 320;

		/*** NrTiles ***/
		read (iFd, sRead, 2);
		snprintf (sReadW, 10, "%02x%02x", sRead[0], sRead[1]);
		arLevelNrTiles[iLevelLoop] = strtoul (sReadW, NULL, 16);

		/*** Warn if the number of tiles is off. ***/
		if (arLevelNrTiles[iLevelLoop] != (arLevelHeight[iLevelLoop] *
			arLevelWidth[iLevelLoop] * TILES))
		{
			printf ("[ WARN ] Incorrect number of tiles in level %i!\n",
				iLevelLoop);
		}

		/*** Offset Graphics ***/
		read (iFd, sRead, 4);
		snprintf (sReadDW, 10, "%02x%02x%02x%02x",
			sRead[0], sRead[1], sRead[2], sRead[3]);
		arLevelOffsetGraphics[iLevelLoop] = strtoul (sReadDW, NULL, 16);

		/*** Offset Objects ***/
		read (iFd, sRead, 4);
		snprintf (sReadDW, 10, "%02x%02x%02x%02x",
			sRead[0], sRead[1], sRead[2], sRead[3]);
		arLevelOffsetObjects[iLevelLoop] = strtoul (sReadDW, NULL, 16);

		/*** Starting Y ***/
		read (iFd, sRead, 2);
		snprintf (sReadW, 10, "%02x%02x", sRead[0], sRead[1]);
		arLevelStartingY[iLevelLoop] = (strtoul (sReadW, NULL, 16) / 192) + 1;

		/*** Starting X ***/
		read (iFd, sRead, 2);
		snprintf (sReadW, 10, "%02x%02x", sRead[0], sRead[1]);
		arLevelStartingX[iLevelLoop] = (strtoul (sReadW, NULL, 16) / 320) + 1;

		/*** Type ***/
		read (iFd, sRead, 2);
		snprintf (sReadW, 10, "%02x%02x", sRead[0], sRead[1]);
		arLevelType[iLevelLoop] = strtoul (sReadW, NULL, 16);

		if (iDebug == 1)
		{
			printf ("========== Level %i ==========\n", iLevelLoop);
			printf ("Height: %i (%i pixels)\n",
				arLevelHeight[iLevelLoop], arLevelHeightP[iLevelLoop]);
			printf ("Width: %i (%i pixels)\n",
				arLevelWidth[iLevelLoop], arLevelWidthP[iLevelLoop]);
			printf ("Tiles: %i (%i x %i x %i)\n",
				arLevelNrTiles[iLevelLoop],
				arLevelHeight[iLevelLoop],
				arLevelWidth[iLevelLoop], TILES);
			printf ("Offset Graphics: 0x%02X\n", arLevelOffsetGraphics[iLevelLoop]);
			printf ("Offset Objects: 0x%02X\n", arLevelOffsetObjects[iLevelLoop]);
			printf ("Starting Y: %i\n", arLevelStartingY[iLevelLoop]);
			printf ("Starting X: %i\n", arLevelStartingX[iLevelLoop]);
			printf ("Type (0 = dungeon, 1 = palace): %i\n", arLevelType[iLevelLoop]);
		}
	}

	/* For the objects and graphics below, lseek is used in the for loops.
	 * This is not necessary per se. A single for loop could be used, which,
	 * for each level, first loads graphics and then objects. Then only
	 * one lseek to iOffsetGrOb would be required.
	 */

	/*** Objects. ***/
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		lseek (iFd, arLevelOffsetObjects[iLevelLoop], SEEK_SET);
		iObjects = 0;
		for (iWidthLoop = 1; iWidthLoop <= (arLevelWidth[iLevelLoop] * WIDTH);
			iWidthLoop++)
		{
			for (iHeightLoop = 1; iHeightLoop <= (arLevelHeight[iLevelLoop] * HEIGHT);
				iHeightLoop++)
			{
				read (iFd, sRead, 1);
				arLevelObjects[iLevelLoop][iWidthLoop][iHeightLoop] = sRead[0];
				iObjects++;
			}
		}
		if (iObjects != arLevelNrTiles[iLevelLoop])
		{
			printf ("[ WARN ] Incorrect number of objects in level %i!\n",
				iLevelLoop);
		}
	}

	/*** Graphics. ***/
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		lseek (iFd, arLevelOffsetGraphics[iLevelLoop], SEEK_SET);
		iGraphics = 0;
		for (iWidthLoop = 1; iWidthLoop <= (arLevelWidth[iLevelLoop] * WIDTH);
			iWidthLoop++)
		{
			for (iHeightLoop = 1; iHeightLoop <= (arLevelHeight[iLevelLoop] * HEIGHT);
				iHeightLoop++)
			{
				read (iFd, sRead, 1);
				arLevelGraphics[iLevelLoop][iWidthLoop][iHeightLoop] = sRead[0];
				iGraphics++;
			}
		}
		if (iGraphics != arLevelNrTiles[iLevelLoop])
		{
			printf ("[ WARN ] Incorrect number of graphics in level %i!\n",
				iLevelLoop);
		}
	}

	/*** Show all rooms on the console. ***/
	if (iDebug == 1)
	{
		for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
		{
			printf ("\nLevel %i:\n", iLevelLoop);
			for (iHeightLoop = 1; iHeightLoop <= (arLevelHeight[iLevelLoop] * HEIGHT);
				iHeightLoop++)
			{
				printf ("[l:%i h:%i] ", iLevelLoop, iHeightLoop);
				for (iWidthLoop = 1; iWidthLoop <= (arLevelWidth[iLevelLoop] * WIDTH);
					iWidthLoop++)
				{
					iObjectValue = arLevelObjects[iLevelLoop]
						[iWidthLoop][iHeightLoop];
					iGraphicsValue = arLevelGraphics[iLevelLoop]
						[iWidthLoop][iHeightLoop];
					PrintTileCombo (iObjectValue, iGraphicsValue);
					if (iWidthLoop != (arLevelWidth[iLevelLoop] * WIDTH))
						{ printf ("|"); }
				}
				printf ("\n\n");
			}
		}
	}

	/*** Guards. ***/
	lseek (iFd, iOffsetGuards, SEEK_SET);
	GetNrsAndOffsets (iFd, arNrGuards, arOffsetGuards, 24, "guards");
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrGuards[iLevelLoop] != 0)
		{
			/*** NOT using arOffsetGuards[iLevelLoop] here. ***/
			for (iGuardLoop = 1; iGuardLoop <= arNrGuards[iLevelLoop]; iGuardLoop++)
			{
				read (iFd, sRead, 24);

				ObjectWarn (sRead[0], 0);
				ObjectWarn (sRead[1], 0);

				/*** Y ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[2], sRead[3]);
				arGuardYP[iLevelLoop][iGuardLoop] = strtoul (sReadW, NULL, 16);
				arGuardY[iLevelLoop][iGuardLoop] =
					LoadYBottom (arGuardYP[iLevelLoop][iGuardLoop]);

				/*** X ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[4], sRead[5]);
				arGuardXP[iLevelLoop][iGuardLoop] = strtoul (sReadW, NULL, 16);
				arGuardX[iLevelLoop][iGuardLoop] =
					(arGuardXP[iLevelLoop][iGuardLoop] / 32) + 1;

				/*** Dir ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[6], sRead[7]);
				arGuardDir[iLevelLoop][iGuardLoop] = strtoul (sReadW, NULL, 16);

				ObjectWarn (sRead[8], 0);
				ObjectWarn (sRead[9], 0);

				/*** Type ***/
				snprintf (sReadDW, 10, "%02x%02x%02x%02x",
					sRead[10], sRead[11], sRead[12], sRead[13]);
				arGuardSprite[iLevelLoop][iGuardLoop] = strtoul (sReadDW, NULL, 16);
				iSprite = arGuardSprite[iLevelLoop][iGuardLoop];
				snprintf (sReadW, 10, "%02x%02x", sRead[14], sRead[15]);
				arGuardType[iLevelLoop][iGuardLoop] = strtoul (sReadW, NULL, 16);
				switch (arGuardType[iLevelLoop][iGuardLoop])
				{
					case 0: /*** guard ***/
						if ((iSprite != 0x23838) && (iSprite != 0x53C8C))
							{ printf ("[ WARN ] Unexpected sprite!\n"); }
						break;
					case 1: /*** skeleton ***/
						if ((iSprite != 0x23954) && (iSprite != 0x53DA2))
							{ printf ("[ WARN ] Unexpected sprite!\n"); }
						break;
					case 2: /*** fat ***/
						if ((iSprite != 0x23DD8) && (iSprite != 0x54226))
							{ printf ("[ WARN ] Unexpected sprite!\n"); }
						break;
					case 3: /*** shadow ***/
						if ((iSprite != 0x24000) && (iSprite != 0x54448))
							{ printf ("[ WARN ] Unexpected sprite!\n"); }
						break;
					case 4: /*** Jaffar ***/
						if ((iSprite != 0x241B2) && (iSprite != 0x545FA))
							{ printf ("[ WARN ] Unexpected sprite!\n"); }
						break;
				}

				/*** Skill ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[16], sRead[17]);
				arGuardSkill[iLevelLoop][iGuardLoop] = strtoul (sReadW, NULL, 16);

				/*** HP ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[18], sRead[19]);
				arGuardHP[iLevelLoop][iGuardLoop] = strtoul (sReadW, NULL, 16);

				ObjectWarn (sRead[20], 0);
				ObjectWarn (sRead[21], 0);
				ObjectWarn (sRead[22], 0);
				ObjectWarn (sRead[23], 0);

				if (iDebug == 1)
				{
					printf ("[ INFO ] Level %i, guard %i: x=%i, y=%i, dir=%i, type=%i,"
						" skill=%i, hp=%i\n",
						iLevelLoop,
						iGuardLoop,
						arGuardX[iLevelLoop][iGuardLoop],
						arGuardY[iLevelLoop][iGuardLoop],
						arGuardDir[iLevelLoop][iGuardLoop],
						arGuardType[iLevelLoop][iGuardLoop],
						arGuardSkill[iLevelLoop][iGuardLoop],
						arGuardHP[iLevelLoop][iGuardLoop]);
				}
			}
		}
	}

	/*** Doors. ***/
	lseek (iFd, iOffsetDoors, SEEK_SET);
	GetNrsAndOffsets (iFd, arNrDoors, arOffsetDoors, 10, "doors");
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrDoors[iLevelLoop] != 0)
		{
			/*** NOT using arOffsetDoors[iLevelLoop] here. ***/
			for (iDoorLoop = 1; iDoorLoop <= arNrDoors[iLevelLoop]; iDoorLoop++)
			{
				read (iFd, sRead, 10);

				/*** Type ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[0], sRead[1]);
				arDoorType[iLevelLoop][iDoorLoop] = strtoul (sReadW, NULL, 16);

				/*** Y ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[2], sRead[3]);
				arDoorYP[iLevelLoop][iDoorLoop] = strtoul (sReadW, NULL, 16);
				arDoorY[iLevelLoop][iDoorLoop] =
					LoadYTop (arDoorYP[iLevelLoop][iDoorLoop]);

				/*** X ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[4], sRead[5]);
				arDoorXP[iLevelLoop][iDoorLoop] = strtoul (sReadW, NULL, 16);
				arDoorX[iLevelLoop][iDoorLoop] =
					(arDoorXP[iLevelLoop][iDoorLoop] / 32) + 1;

				ObjectWarn (sRead[6], 0);
				ObjectWarn (sRead[7], 0);
				ObjectWarn (sRead[8], 0);
				ObjectWarn (sRead[9], 0);

				if (iDebug == 1)
				{
					printf ("[ INFO ] Level %i, door %i: type=%i, y:%i, x:%i\n",
						iLevelLoop,
						iDoorLoop,
						arDoorType[iLevelLoop][iDoorLoop],
						arDoorY[iLevelLoop][iDoorLoop],
						arDoorX[iLevelLoop][iDoorLoop]);
				}
			}
		}
	}

	/*** Gates. ***/
	lseek (iFd, iOffsetGates, SEEK_SET);
	GetNrsAndOffsets (iFd, arNrGates, arOffsetGates, 12, "gates");
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrGates[iLevelLoop] != 0)
		{
			/*** NOT using arOffsetGates[iLevelLoop] here. ***/
			for (iGateLoop = 1; iGateLoop <= arNrGates[iLevelLoop]; iGateLoop++)
			{
				read (iFd, sRead, 12);

				/*** State 1 ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[0], sRead[1]);
				arGateState1[iLevelLoop][iGateLoop] = strtoul (sReadW, NULL, 16);

				/*** Y ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[2], sRead[3]);
				arGateYP[iLevelLoop][iGateLoop] = strtoul (sReadW, NULL, 16);
				arGateY[iLevelLoop][iGateLoop] =
					LoadYTop (arGateYP[iLevelLoop][iGateLoop]);

				/*** X ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[4], sRead[5]);
				arGateXP[iLevelLoop][iGateLoop] = strtoul (sReadW, NULL, 16);
				arGateX[iLevelLoop][iGateLoop] =
					(arGateXP[iLevelLoop][iGateLoop] / 32) + 1;

				/*** State 2 ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[6], sRead[7]);
				arGateState2[iLevelLoop][iGateLoop] = strtoul (sReadW, NULL, 16);

				/*** State 3 ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[8], sRead[9]);
				arGateState3[iLevelLoop][iGateLoop] = strtoul (sReadW, NULL, 16);

				/*** Unk ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[10], sRead[11]);
				arGateUnk[iLevelLoop][iGateLoop] = strtoul (sReadW, NULL, 16);

				if (iDebug == 1)
				{
					printf ("[ INFO ] Level %i, gate %i: s1=%i, y=%i, x=%i, s2=%i"
						", s3=%i, unk=%i\n",
						iLevelLoop,
						iGateLoop,
						arGateState1[iLevelLoop][iGateLoop],
						arGateY[iLevelLoop][iGateLoop],
						arGateX[iLevelLoop][iGateLoop],
						arGateState2[iLevelLoop][iGateLoop],
						arGateState3[iLevelLoop][iGateLoop],
						arGateUnk[iLevelLoop][iGateLoop]);
				}
			}
		}
	}

	/*** Loose. ***/
	lseek (iFd, iOffsetLoose, SEEK_SET);
	GetNrsAndOffsets (iFd, arNrLoose, arOffsetLoose, 10, "loose");
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrLoose[iLevelLoop] != 0)
		{
			/*** NOT using arOffsetLoose[iLevelLoop] here. ***/
			for (iLooseLoop = 1; iLooseLoop <= arNrLoose[iLevelLoop]; iLooseLoop++)
			{
				read (iFd, sRead, 10);

				ObjectWarn (sRead[0], 0);
				ObjectWarn (sRead[1], 1);

				/*** Y ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[2], sRead[3]);
				arLooseYP[iLevelLoop][iLooseLoop] = strtoul (sReadW, NULL, 16);
				arLooseY[iLevelLoop][iLooseLoop] =
					LoadYBottom (arLooseYP[iLevelLoop][iLooseLoop]);

				/*** X ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[4], sRead[5]);
				arLooseXP[iLevelLoop][iLooseLoop] = strtoul (sReadW, NULL, 16);
				arLooseX[iLevelLoop][iLooseLoop] =
					(arLooseXP[iLevelLoop][iLooseLoop] / 32) + 1;

				ObjectWarn (sRead[6], 0);
				ObjectWarn (sRead[7], 0);
				ObjectWarn (sRead[8], 0);
				ObjectWarn (sRead[9], 0);

				if (iDebug == 1)
				{
					printf ("[ INFO ] Level %i, loose %i: y=%i, x=%i\n",
						iLevelLoop,
						iLooseLoop,
						arLooseY[iLevelLoop][iLooseLoop],
						arLooseX[iLevelLoop][iLooseLoop]);
				}
			}
		}
	}

	/*** Raise. ***/
	lseek (iFd, iOffsetRaise, SEEK_SET);
	GetNrsAndOffsets (iFd, arNrRaise, arOffsetRaise, 14, "raise");
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrRaise[iLevelLoop] != 0)
		{
			/*** NOT using arOffsetRaise[iLevelLoop] here. ***/
			for (iRaiseLoop = 1; iRaiseLoop <= arNrRaise[iLevelLoop]; iRaiseLoop++)
			{
				read (iFd, sRead, 14);

				ObjectWarn (sRead[0], 0);
				ObjectWarn (sRead[1], 1);

				/*** Y ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[2], sRead[3]);
				arRaiseYP[iLevelLoop][iRaiseLoop] = strtoul (sReadW, NULL, 16);
				arRaiseY[iLevelLoop][iRaiseLoop] =
					LoadYBottom (arRaiseYP[iLevelLoop][iRaiseLoop]);

				/*** X ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[4], sRead[5]);
				arRaiseXP[iLevelLoop][iRaiseLoop] = strtoul (sReadW, NULL, 16);
				arRaiseX[iLevelLoop][iRaiseLoop] =
					(arRaiseXP[iLevelLoop][iRaiseLoop] / 32) + 1;

				ObjectWarn (sRead[6], 0);
				ObjectWarn (sRead[7], 0);

				/*** Gates 1, 2 and 3 ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[8], sRead[9]);
				arRaiseGate1[iLevelLoop][iRaiseLoop] = strtoul (sReadW, NULL, 16);
				snprintf (sReadW, 10, "%02x%02x", sRead[10], sRead[11]);
				arRaiseGate2[iLevelLoop][iRaiseLoop] = strtoul (sReadW, NULL, 16);
				snprintf (sReadW, 10, "%02x%02x", sRead[12], sRead[13]);
				arRaiseGate3[iLevelLoop][iRaiseLoop] = strtoul (sReadW, NULL, 16);

				if (iDebug == 1)
				{
					printf ("[ INFO ] Level %i, raise %i: y=%i, x=%i, g1=%s"
						", g2=%s, g3=%s\n",
						iLevelLoop,
						iRaiseLoop,
						arRaiseY[iLevelLoop][iRaiseLoop],
						arRaiseX[iLevelLoop][iRaiseLoop],
						GateAsText (arRaiseGate1[iLevelLoop][iRaiseLoop]),
						GateAsText (arRaiseGate2[iLevelLoop][iRaiseLoop]),
						GateAsText (arRaiseGate3[iLevelLoop][iRaiseLoop]));
				}
			}
		}
	}

	/*** Drop. ***/
	lseek (iFd, iOffsetDrop, SEEK_SET);
	GetNrsAndOffsets (iFd, arNrDrop, arOffsetDrop, 14, "drop");
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrDrop[iLevelLoop] != 0)
		{
			/*** NOT using arOffsetDrop[iLevelLoop] here. ***/
			for (iDropLoop = 1; iDropLoop <= arNrDrop[iLevelLoop]; iDropLoop++)
			{
				read (iFd, sRead, 14);

				ObjectWarn (sRead[0], 0);
				ObjectWarn (sRead[1], 1);

				/*** Y ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[2], sRead[3]);
				arDropYP[iLevelLoop][iDropLoop] = strtoul (sReadW, NULL, 16);
				arDropY[iLevelLoop][iDropLoop] =
					LoadYBottom (arDropYP[iLevelLoop][iDropLoop]);

				/*** X ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[4], sRead[5]);
				arDropXP[iLevelLoop][iDropLoop] = strtoul (sReadW, NULL, 16);
				arDropX[iLevelLoop][iDropLoop] =
					(arDropXP[iLevelLoop][iDropLoop] / 32) + 1;

				ObjectWarn (sRead[6], 0);
				ObjectWarn (sRead[7], 0);

				/*** Gates 1, 2 and 3 ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[8], sRead[9]);
				arDropGate1[iLevelLoop][iDropLoop] = strtoul (sReadW, NULL, 16);
				snprintf (sReadW, 10, "%02x%02x", sRead[10], sRead[11]);
				arDropGate2[iLevelLoop][iDropLoop] = strtoul (sReadW, NULL, 16);
				snprintf (sReadW, 10, "%02x%02x", sRead[12], sRead[13]);
				arDropGate3[iLevelLoop][iDropLoop] = strtoul (sReadW, NULL, 16);

				if (iDebug == 1)
				{
					printf ("[ INFO ] Level %i, drop %i: y=%i, x=%i, g1=%s"
						", g2=%s, g3=%s\n",
						iLevelLoop,
						iDropLoop,
						arDropY[iLevelLoop][iDropLoop],
						arDropX[iLevelLoop][iDropLoop],
						GateAsText (arDropGate1[iLevelLoop][iDropLoop]),
						GateAsText (arDropGate2[iLevelLoop][iDropLoop]),
						GateAsText (arDropGate3[iLevelLoop][iDropLoop]));
				}
			}
		}
	}

	/*** Chompers. ***/
	lseek (iFd, iOffsetChompers, SEEK_SET);
	GetNrsAndOffsets (iFd, arNrChompers, arOffsetChompers, 10, "chompers");
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrChompers[iLevelLoop] != 0)
		{
			/*** NOT using arOffsetChompers[iLevelLoop] here. ***/
			for (iChomperLoop = 1; iChomperLoop <=
				arNrChompers[iLevelLoop]; iChomperLoop++)
			{
				read (iFd, sRead, 10);

				ObjectWarn (sRead[0], 0);
				ObjectWarn (sRead[1], 1);

				/*** Y ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[2], sRead[3]);
				arChomperYP[iLevelLoop][iChomperLoop] = strtoul (sReadW, NULL, 16);
				arChomperY[iLevelLoop][iChomperLoop] =
					LoadYTop (arChomperYP[iLevelLoop][iChomperLoop]);

				/*** X ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[4], sRead[5]);
				arChomperXP[iLevelLoop][iChomperLoop] = strtoul (sReadW, NULL, 16);
				arChomperX[iLevelLoop][iChomperLoop] =
					(arChomperXP[iLevelLoop][iChomperLoop] / 32) + 1;

				ObjectWarn (sRead[6], 0);
				ObjectWarn (sRead[7], 0);
				ObjectWarn (sRead[8], 0);
				ObjectWarn (sRead[9], 0);

				if (iDebug == 1)
				{
					printf ("[ INFO ] Level %i, chomper %i: y=%i, x=%i\n",
						iLevelLoop,
						iChomperLoop,
						arChomperY[iLevelLoop][iChomperLoop],
						arChomperX[iLevelLoop][iChomperLoop]);
				}
			}
		}
	}

	/*** Spikes. ***/
	lseek (iFd, iOffsetSpikes, SEEK_SET);
	GetNrsAndOffsets (iFd, arNrSpikes, arOffsetSpikes, 8, "spikes");
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrSpikes[iLevelLoop] != 0)
		{
			/*** NOT using arOffsetSpikes[iLevelLoop] here. ***/
			for (iSpikeLoop = 1; iSpikeLoop <= arNrSpikes[iLevelLoop]; iSpikeLoop++)
			{
				read (iFd, sRead, 8);

				ObjectWarn (sRead[0], 0);
				ObjectWarn (sRead[1], 1);

				/*** Y ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[2], sRead[3]);
				arSpikeYP[iLevelLoop][iSpikeLoop] = strtoul (sReadW, NULL, 16);
				arSpikeY[iLevelLoop][iSpikeLoop] =
					LoadYBottom (arSpikeYP[iLevelLoop][iSpikeLoop]);

				/*** X ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[4], sRead[5]);
				arSpikeXP[iLevelLoop][iSpikeLoop] = strtoul (sReadW, NULL, 16);
				arSpikeX[iLevelLoop][iSpikeLoop] =
					(arSpikeXP[iLevelLoop][iSpikeLoop] / 32) + 1;

				ObjectWarn (sRead[6], 0xFF);
				ObjectWarn (sRead[7], 0xFF);

				if (iDebug == 1)
				{
					printf ("[ INFO ] Level %i, spike %i: y=%i, x=%i\n",
						iLevelLoop,
						iSpikeLoop,
						arSpikeY[iLevelLoop][iSpikeLoop],
						arSpikeX[iLevelLoop][iSpikeLoop]);
				}
			}
		}
	}

	/*** Potions. ***/
	lseek (iFd, iOffsetPotions, SEEK_SET);
	GetNrsAndOffsets (iFd, arNrPotions, arOffsetPotions, 10, "potion");
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrPotions[iLevelLoop] != 0)
		{
			/*** NOT using arOffsetPotions[iLevelLoop] here. ***/
			for (iPotionLoop = 1; iPotionLoop <=
				arNrPotions[iLevelLoop]; iPotionLoop++)
			{
				read (iFd, sRead, 10);

				/*** Color ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[0], sRead[1]);
				arPotionColor[iLevelLoop][iPotionLoop] = strtoul (sReadW, NULL, 16);

				/*** Y ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[2], sRead[3]);
				arPotionYP[iLevelLoop][iPotionLoop] = strtoul (sReadW, NULL, 16);
				arPotionY[iLevelLoop][iPotionLoop] =
					LoadYBottom (arPotionYP[iLevelLoop][iPotionLoop]);

				/*** X ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[4], sRead[5]);
				arPotionXP[iLevelLoop][iPotionLoop] = strtoul (sReadW, NULL, 16);
				arPotionX[iLevelLoop][iPotionLoop] =
					(arPotionXP[iLevelLoop][iPotionLoop] / 32) + 1;

				ObjectWarn (sRead[6], 0);
				ObjectWarn (sRead[7], 0);

				/*** Effect ***/
				snprintf (sReadW, 10, "%02x%02x", sRead[8], sRead[9]);
				arPotionEffect[iLevelLoop][iPotionLoop] = strtoul (sReadW, NULL, 16);

				if (iDebug == 1)
				{
					printf ("[ INFO ] Level %i, potion %i: c=%i, y=%i, x=%i, e=%i\n",
						iLevelLoop,
						iPotionLoop,
						arPotionColor[iLevelLoop][iPotionLoop],
						arPotionY[iLevelLoop][iPotionLoop],
						arPotionX[iLevelLoop][iPotionLoop],
						arPotionEffect[iLevelLoop][iPotionLoop]);
				}
			}
		}
	}

	close (iFd);
}
/*****************************************************************************/
void StringToUpper (char *sInput, char *sOutput)
/*****************************************************************************/
{
	/*** Used for looping. ***/
	int iCharLoop;

	for (iCharLoop = 0; sInput[iCharLoop] != '\0'; iCharLoop++)
	{
		sOutput[iCharLoop] = toupper (sInput[iCharLoop]);
	}
	sOutput[iCharLoop] = '\0';
}
/*****************************************************************************/
void SaveLevels (void)
/*****************************************************************************/
{
	int iFd;
	int iToWrite;
	int iNrTiles;
	int iNrTilesTotal;
	int iObject;
	int iSwordLevel, iSwordX, iSwordY;

	/*** Used for looping. ***/
	int iLevelLoop;
	int iWidthLoop, iHeightLoop;
	int iGuardLoop;
	int iDoorLoop;
	int iGateLoop;
	int iLooseLoop;
	int iRaiseLoop;
	int iDropLoop;
	int iChomperLoop;
	int iSpikeLoop;
	int iPotionLoop;

	iFd = open (sPathFile, O_WRONLY|O_BINARY);
	if (iFd == -1)
	{
		printf ("[FAILED] Could not open \"%s\": %s!\n",
			sPathFile, strerror (errno));
		exit (EXIT_ERROR);
	}

	SetTypeDefaults();

	/*** Prince ***/
	lseek (iFd, iOffsetPrince, SEEK_SET);
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		iToWrite = SaveYBottom (arPrinceY[iLevelLoop]);
		if ((iEXEType == 1) && (iLevelLoop == 7)) { iToWrite+=31; }
		if ((iEXEType == 2) && (iLevelLoop == 9)) { iToWrite+=31; }
		WriteWord (iFd, iToWrite); /*** Y ***/

		iToWrite = (arPrinceX[iLevelLoop] - 1) * 32;
		if ((iEXEType == 2) && (iLevelLoop == 1)) { iToWrite+=12; }
		WriteWord (iFd, iToWrite); /*** X ***/

		WriteWord (iFd, arPrinceDir[iLevelLoop]); /*** Dir ***/
	}

	lseek (iFd, iOffsetLevels, SEEK_SET);
	iNrTilesTotal = 0;
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		WriteWord (iFd, arLevelHeight[iLevelLoop] * 192); /*** Height ***/
		WriteWord (iFd, arLevelWidth[iLevelLoop] * 320); /*** Width ***/
		iNrTiles = arLevelWidth[iLevelLoop] *
			arLevelHeight[iLevelLoop] * TILES;
		if (iNrTiles != arLevelNrTiles[iLevelLoop])
		{
			printf ("[ WARN ] Incorrect number of tiles in level %i: %i vs %i!\n",
				iLevelLoop, iNrTiles, arLevelNrTiles[iLevelLoop]);
		}
		WriteWord (iFd, iNrTiles); /*** NrTiles ***/
		WriteDWord (iFd, iOffsetGrOb + iNrTilesTotal); /*** Offset Graphics. ***/
		WriteDWord (iFd, iOffsetGrOb + iNrTilesTotal + iNrTiles); /*** Off. O. ***/
		WriteWord (iFd, (arLevelStartingY[iLevelLoop] - 1) * 192); /*** St. Y ***/
		WriteWord (iFd, (arLevelStartingX[iLevelLoop] - 1) * 320); /*** St. X ***/
		WriteWord (iFd, arLevelType[iLevelLoop]); /*** Type ***/
		iNrTilesTotal = iNrTilesTotal + (2 * iNrTiles);
	}

	/*** Graphics and objects. ***/
	iSwordLevel = MAX_LEVELS + 1; /*** Default. In case there's no s. anywh. ***/
	iSwordX = 1; iSwordY = 1; /*** Defaults. ***/
	lseek (iFd, iOffsetGrOb, SEEK_SET);
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		for (iWidthLoop = 1; iWidthLoop <= (arLevelWidth[iLevelLoop]
			* WIDTH); iWidthLoop++)
		{
			for (iHeightLoop = 1; iHeightLoop <= (arLevelHeight[iLevelLoop]
				* HEIGHT); iHeightLoop++)
			{
				WriteByte (iFd, arLevelGraphics[iLevelLoop][iWidthLoop][iHeightLoop]);
			}
		}
		for (iWidthLoop = 1; iWidthLoop <= (arLevelWidth[iLevelLoop]
			* WIDTH); iWidthLoop++)
		{
			for (iHeightLoop = 1; iHeightLoop <= (arLevelHeight[iLevelLoop]
				* HEIGHT); iHeightLoop++)
			{
				iObject = arLevelObjects[iLevelLoop][iWidthLoop][iHeightLoop];
				WriteByte (iFd, iObject);
				if (iObject == 0x0B)
				{
					iSwordLevel = iLevelLoop;
					iSwordX = iWidthLoop;
					iSwordY = iHeightLoop;
				}
			}
		}
	}

	/*** Save sword information. ***/
	SaveSword (iFd, iSwordLevel, iSwordX, iSwordY);

	/*** Guards. ***/
	lseek (iFd, iOffsetGuards, SEEK_SET);
	SetNrsAndOffsets (iFd, arNrGuards, iOffsetGuards, 24);
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrGuards[iLevelLoop] != 0)
		{
			for (iGuardLoop = 1; iGuardLoop <= arNrGuards[iLevelLoop]; iGuardLoop++)
			{
				WriteByte (iFd, 0);
				WriteByte (iFd, 0);
				WriteWord (iFd, SaveYBottom
					(arGuardY[iLevelLoop][iGuardLoop])); /*** Y ***/
				WriteWord (iFd, (arGuardX[iLevelLoop][iGuardLoop]
					- 1) * 32); /*** X ***/
				WriteWord (iFd, arGuardDir[iLevelLoop][iGuardLoop]); /*** Dir ***/
				WriteByte (iFd, 0);
				WriteByte (iFd, 0);
				WriteDWord (iFd, arGuardSprite[iLevelLoop][iGuardLoop]); /*** Sp. ***/
				WriteWord (iFd, arGuardType[iLevelLoop][iGuardLoop]); /*** Type ***/
				WriteWord (iFd, arGuardSkill[iLevelLoop][iGuardLoop]); /*** Skill ***/
				WriteWord (iFd, arGuardHP[iLevelLoop][iGuardLoop]); /*** HP ***/
				WriteByte (iFd, 0);
				WriteByte (iFd, 0);
				WriteByte (iFd, 0);
				WriteByte (iFd, 0);
			}
		}
	}

	/*** Doors. ***/
	lseek (iFd, iOffsetDoors, SEEK_SET);
	SetNrsAndOffsets (iFd, arNrDoors, iOffsetDoors, 10);
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrDoors[iLevelLoop] != 0)
		{
			for (iDoorLoop = 1; iDoorLoop <= arNrDoors[iLevelLoop]; iDoorLoop++)
			{
				WriteWord (iFd, arDoorType[iLevelLoop][iDoorLoop]); /*** Type ***/
				WriteWord (iFd, SaveYTop (arDoorY[iLevelLoop][iDoorLoop])); /*** Y ***/
				WriteWord (iFd, (arDoorX[iLevelLoop][iDoorLoop] - 1) * 32); /*** X ***/
				WriteByte (iFd, 0);
				WriteByte (iFd, 0);
				WriteByte (iFd, 0);
				WriteByte (iFd, 0);
			}
		}
	}

	/*** Gates. ***/
	lseek (iFd, iOffsetGates, SEEK_SET);
	SetNrsAndOffsets (iFd, arNrGates, iOffsetGates, 12);
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrGates[iLevelLoop] != 0)
		{
			for (iGateLoop = 1; iGateLoop <= arNrGates[iLevelLoop]; iGateLoop++)
			{
				WriteWord (iFd, arGateState1[iLevelLoop][iGateLoop]); /*** State 1 ***/
				WriteWord (iFd, SaveYTop (arGateY[iLevelLoop][iGateLoop])); /*** Y ***/
				WriteWord (iFd, (arGateX[iLevelLoop][iGateLoop] - 1) * 32); /*** X ***/
				WriteWord (iFd, arGateState2[iLevelLoop][iGateLoop]); /*** State 2 ***/
				WriteWord (iFd, arGateState3[iLevelLoop][iGateLoop]); /*** State 3 ***/
				WriteWord (iFd, arGateUnk[iLevelLoop][iGateLoop]); /*** Unk ***/
			}
		}
	}

	/*** Loose. ***/
	lseek (iFd, iOffsetLoose, SEEK_SET);
	SetNrsAndOffsets (iFd, arNrLoose, iOffsetLoose, 10);
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrLoose[iLevelLoop] != 0)
		{
			for (iLooseLoop = 1; iLooseLoop <= arNrLoose[iLevelLoop]; iLooseLoop++)
			{
				WriteByte (iFd, 0);
				WriteByte (iFd, 1);
				WriteWord (iFd, SaveYBottom
					(arLooseY[iLevelLoop][iLooseLoop])); /*** Y ***/
				WriteWord (iFd, (arLooseX[iLevelLoop][iLooseLoop]
					- 1) * 32); /*** X ***/
				WriteByte (iFd, 0);
				WriteByte (iFd, 0);
				WriteByte (iFd, 0);
				WriteByte (iFd, 0);
			}
		}
	}

	/*** Raise. ***/
	lseek (iFd, iOffsetRaise, SEEK_SET);
	SetNrsAndOffsets (iFd, arNrRaise, iOffsetRaise, 14);
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrRaise[iLevelLoop] != 0)
		{
			for (iRaiseLoop = 1; iRaiseLoop <= arNrRaise[iLevelLoop]; iRaiseLoop++)
			{
				WriteByte (iFd, 0);
				WriteByte (iFd, 1);
				WriteWord (iFd, SaveYBottom
					(arRaiseY[iLevelLoop][iRaiseLoop])); /*** Y ***/
				WriteWord (iFd, (arRaiseX[iLevelLoop][iRaiseLoop]
					- 1) * 32); /*** X ***/
				WriteByte (iFd, 0);
				WriteByte (iFd, 0);
				WriteWord (iFd, arRaiseGate1[iLevelLoop][iRaiseLoop]);
				WriteWord (iFd, arRaiseGate2[iLevelLoop][iRaiseLoop]);
				WriteWord (iFd, arRaiseGate3[iLevelLoop][iRaiseLoop]);
			}
		}
	}

	/*** Drop. ***/
	lseek (iFd, iOffsetDrop, SEEK_SET);
	SetNrsAndOffsets (iFd, arNrDrop, iOffsetDrop, 14);
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrDrop[iLevelLoop] != 0)
		{
			for (iDropLoop = 1; iDropLoop <= arNrDrop[iLevelLoop]; iDropLoop++)
			{
				WriteByte (iFd, 0);
				WriteByte (iFd, 1);
				WriteWord (iFd, SaveYBottom
					(arDropY[iLevelLoop][iDropLoop])); /*** Y ***/
				WriteWord (iFd, (arDropX[iLevelLoop][iDropLoop]
					- 1) * 32); /*** X ***/
				WriteByte (iFd, 0);
				WriteByte (iFd, 0);
				WriteWord (iFd, arDropGate1[iLevelLoop][iDropLoop]);
				WriteWord (iFd, arDropGate2[iLevelLoop][iDropLoop]);
				WriteWord (iFd, arDropGate3[iLevelLoop][iDropLoop]);
			}
		}
	}

	/*** Chompers. ***/
	lseek (iFd, iOffsetChompers, SEEK_SET);
	SetNrsAndOffsets (iFd, arNrChompers, iOffsetChompers, 10);
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrChompers[iLevelLoop] != 0)
		{
			for (iChomperLoop = 1; iChomperLoop <=
				arNrChompers[iLevelLoop]; iChomperLoop++)
			{
				WriteByte (iFd, 0);
				WriteByte (iFd, 1);
				WriteWord (iFd, SaveYTop
					(arChomperY[iLevelLoop][iChomperLoop])); /*** Y ***/
				WriteWord (iFd, (arChomperX[iLevelLoop][iChomperLoop]
					- 1) * 32); /*** X ***/
				WriteByte (iFd, 0);
				WriteByte (iFd, 0);
				WriteByte (iFd, 0);
				WriteByte (iFd, 0);
			}
		}
	}

	/*** Spikes. ***/
	lseek (iFd, iOffsetSpikes, SEEK_SET);
	SetNrsAndOffsets (iFd, arNrSpikes, iOffsetSpikes, 8);
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrSpikes[iLevelLoop] != 0)
		{
			for (iSpikeLoop = 1; iSpikeLoop <= arNrSpikes[iLevelLoop]; iSpikeLoop++)
			{
				WriteByte (iFd, 0);
				WriteByte (iFd, 1);
				WriteWord (iFd, SaveYBottom
					(arSpikeY[iLevelLoop][iSpikeLoop])); /*** Y ***/
				WriteWord (iFd, (arSpikeX[iLevelLoop][iSpikeLoop]
					- 1) * 32); /*** X ***/
				WriteByte (iFd, 0xFF);
				WriteByte (iFd, 0xFF);
			}
		}
	}

	/*** Potions. ***/
	lseek (iFd, iOffsetPotions, SEEK_SET);
	SetNrsAndOffsets (iFd, arNrPotions, iOffsetPotions, 10);
	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		if (arNrPotions[iLevelLoop] != 0)
		{
			for (iPotionLoop = 1; iPotionLoop <=
				arNrPotions[iLevelLoop]; iPotionLoop++)
			{
				WriteWord (iFd,
					arPotionColor[iLevelLoop][iPotionLoop]); /*** Color ***/
				WriteWord (iFd, SaveYBottom
					(arPotionY[iLevelLoop][iPotionLoop])); /*** Y ***/
				WriteWord (iFd, (arPotionX[iLevelLoop][iPotionLoop]
					- 1) * 32); /*** X ***/
				WriteByte (iFd, 0);
				WriteByte (iFd, 0);
				WriteWord (iFd,
					arPotionEffect[iLevelLoop][iPotionLoop]); /*** Effect ***/
			}
		}
	}

	close (iFd);

	PlaySound ("wav/save.wav");

	iChanged = 0;
}
/*****************************************************************************/
void PrintTileCombo (int iObjectValue, int iGraphicsValue)
/*****************************************************************************/
{
	char sObject[10 + 2];
	char sGraphics[10 + 2];
	char sTogether[10 + 2];

	switch (iObjectValue)
	{
		case 0x00: snprintf (sObject, 10, "%s", "spc"); break;
		case 0x01: snprintf (sObject, 10, "%s", "wal"); break;
		case 0x02: snprintf (sObject, 10, "%s", "flr"); break;
		case 0x03: snprintf (sObject, 10, "%s", "rbt"); break;
		case 0x04: snprintf (sObject, 10, "%s", "dbt"); break;
		case 0x05: snprintf (sObject, 10, "%s", "gat"); break;
		case 0x06: snprintf (sObject, 10, "%s", "loo"); break;
		case 0x07: snprintf (sObject, 10, "%s", "spk"); break;
		case 0x08: snprintf (sObject, 10, "%s", "cho"); break;
		case 0x09: snprintf (sObject, 10, "%s", "pot"); break;
		case 0x0A: snprintf (sObject, 10, "%s", "dor"); break;
		case 0x0B: snprintf (sObject, 10, "%s", "swd"); break;
		default:
			printf ("[ WARN ] Unknown object: %i!\n", iObjectValue);
			break;
	}

	snprintf (sGraphics, 10, "%02X", iGraphicsValue);

	snprintf (sTogether, 10, "%s %s", sObject, sGraphics);

	printf ("%s", sTogether);
}
/*****************************************************************************/
void PrIfDe (char *sString)
/*****************************************************************************/
{
	if (iDebug == 1) { printf ("%s", sString); }
}
/*****************************************************************************/
void Quit (void)
/*****************************************************************************/
{
	if ((iChanged != 0) && (IsSavingAllowed() == 1)) { InitPopUpSave(); }
	if (iModified != 0) { ModifyStart (iCurLevel, 2); }
	TTF_CloseFont (font1);
	TTF_CloseFont (font2);
	TTF_CloseFont (font3);
	TTF_Quit();
	SDL_Quit();
	exit (EXIT_NORMAL);
}
/*****************************************************************************/
void InitScreen (void)
/*****************************************************************************/
{
	SDL_AudioSpec fmt;
	char sImage[MAX_IMG + 2];
	SDL_Surface *imgicon;
	int iJoyNr;
	SDL_Event event;
	int iOldXPos, iOldYPos;
	const Uint8 *keystate;
	char sFileName[MAX_PATHFILE + 2];
	SDL_Rect barbox;
	int iOldWidth, iOldHeight;
	int iGateX, iGateY;
	int iGraphics;
	int iTorchX;

	/*** Used for looping. ***/
	int iByteLoop;
	int iTabLoop;
	int iMinLoop;

	if (SDL_Init (SDL_INIT_AUDIO|SDL_INIT_VIDEO|
		SDL_INIT_GAMECONTROLLER|SDL_INIT_HAPTIC) < 0)
	{
		printf ("[FAILED] Unable to init SDL: %s!\n", SDL_GetError());
		exit (EXIT_ERROR);
	}
	atexit (SDL_Quit);

	window = SDL_CreateWindow (EDITOR_NAME " " EDITOR_VERSION,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		(WINDOW_WIDTH) * iScale, (WINDOW_HEIGHT) * iScale, iFullscreen);
	if (window == NULL)
	{
		printf ("[FAILED] Unable to create a window: %s!\n", SDL_GetError());
		exit (EXIT_ERROR);
	}
	ascreen = SDL_CreateRenderer (window, -1, 0);
	if (ascreen == NULL)
	{
		printf ("[FAILED] Unable to set video mode: %s!\n", SDL_GetError());
		exit (EXIT_ERROR);
	}
	/*** Some people may prefer linear, but we're going old school. ***/
	SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	if (iFullscreen != 0)
	{
		SDL_RenderSetLogicalSize (ascreen, (WINDOW_WIDTH) * iScale,
			(WINDOW_HEIGHT) * iScale);
	}

	if (TTF_Init() == -1)
	{
		printf ("[FAILED] Could not initialize TTF!\n");
		exit (EXIT_ERROR);
	}

	LoadFonts();

	curArrow = SDL_CreateSystemCursor (SDL_SYSTEM_CURSOR_ARROW);
	curWait = SDL_CreateSystemCursor (SDL_SYSTEM_CURSOR_WAIT);
	curHand = SDL_CreateSystemCursor (SDL_SYSTEM_CURSOR_HAND);

	if (iNoAudio != 1)
	{
		PrIfDe ("[  OK  ] Initializing Audio\n");
		fmt.freq = 44100;
		fmt.format = AUDIO_S16;
		fmt.channels = 2;
		fmt.samples = 512;
		fmt.callback = MixAudio;
		fmt.userdata = NULL;
		if (SDL_OpenAudio (&fmt, NULL) < 0)
		{
			printf ("[FAILED] Unable to open audio: %s!\n", SDL_GetError());
			exit (EXIT_ERROR);
		}
		SDL_PauseAudio (0);
	}

	/*** icon ***/
	snprintf (sImage, MAX_IMG, "png%svarious%slemdop_icon.png",
		SLASH, SLASH);
	imgicon = IMG_Load (sImage);
	if (imgicon == NULL)
	{
		printf ("[ WARN ] Could not load \"%s\": %s!\n", sImage, strerror (errno));
	} else {
		SDL_SetWindowIcon (window, imgicon);
	}

	/*** Open the first available controller. ***/
	iController = 0;
	if (iNoController != 1)
	{
		for (iJoyNr = 0; iJoyNr < SDL_NumJoysticks(); iJoyNr++)
		{
			if (SDL_IsGameController (iJoyNr))
			{
				controller = SDL_GameControllerOpen (iJoyNr);
				if (controller)
				{
					snprintf (sControllerName, MAX_CON, "%s",
						SDL_GameControllerName (controller));
					if (iDebug == 1)
					{
						printf ("[ INFO ] Found a controller \"%s\"; \"%s\".\n",
							sControllerName, SDL_GameControllerNameForIndex (iJoyNr));
					}
					joystick = SDL_GameControllerGetJoystick (controller);
					iController = 1;

					/*** Just for fun, use haptic. ***/
					if (SDL_JoystickIsHaptic (joystick))
					{
						haptic = SDL_HapticOpenFromJoystick (joystick);
						if (SDL_HapticRumbleInit (haptic) == 0)
						{
							SDL_HapticRumblePlay (haptic, 1.0, 1000);
						} else {
							printf ("[ WARN ] Could not initialize the haptic device: %s!\n",
								SDL_GetError());
						}
					} else {
						PrIfDe ("[ INFO ] The game controller is not haptic.\n");
					}
				} else {
					printf ("[ WARN ] Could not open game controller %i: %s!\n",
						iController, SDL_GetError());
				}
			}
		}
		if (iController != 1) { PrIfDe ("[ INFO ] No controller found.\n"); }
	} else {
		PrIfDe ("[ INFO ] Using keyboard and mouse.\n");
	}

	/*******************/
	/* Preload images. */
	/*******************/

	/*** Loading... ***/
	PreLoad (PNG_VARIOUS, "loading.png", &imgloading);
	ShowImage (imgloading, 0, 0, "imgloading");
	SDL_SetRenderDrawColor (ascreen, 0x22, 0x22, 0x22, SDL_ALPHA_OPAQUE);
	barbox.x = 10 * iScale;
	barbox.y = 10 * iScale;
	barbox.w = 20 * iScale;
	barbox.h = 447 * iScale;
	SDL_RenderFillRect (ascreen, &barbox);
	SDL_RenderPresent (ascreen);

	iPreLoaded = 0;
	iCurrentBarHeight = 0;
	iNrToPreLoad = 1203; /*** Value can be obtained via debug mode. ***/
	SDL_SetCursor (curWait);

	/*** Objects. ***/
	PreLoadSet (0x00);
	PreLoadSet (0x01);
	PreLoadSet (0x02);
	PreLoadSet (0x03);
	PreLoadSet (0x04);
	PreLoadSet (0x05);
	PreLoadSet (0x06);
	PreLoadSet (0x07);
	PreLoadSet (0x08);
	PreLoadSet (0x09);
	PreLoadSet (0x0A);
	PreLoadSet (0x0B);

	/*** Dungeon and palace graphics. ***/
	for (iByteLoop = 0x00; iByteLoop <= 0xFF; iByteLoop++)
	{
		snprintf (sFileName, MAX_PATHFILE, "0x%02x.png", iByteLoop);
		PreLoad (PNG_DUNGEON, sFileName, &imgd[iByteLoop][1]);
		PreLoad (PNG_VARIOUS, "sel_graphics.png", &imgd[iByteLoop][2]);
		PreLoad (PNG_PALACE, sFileName, &imgp[iByteLoop][1]);
		PreLoad (PNG_VARIOUS, "sel_graphics.png", &imgp[iByteLoop][2]);
	}

	/*** Object tabs. ***/
	for (iTabLoop = 0x01; iTabLoop <= 12; iTabLoop++)
	{
		snprintf (sFileName, MAX_PATHFILE, "object_%02i.png", iTabLoop);
		PreLoad (PNG_TABS, sFileName, &imgtabo[iTabLoop]);
	}

	/*** Graphics tabs. ***/
	for (iTabLoop = 0x01; iTabLoop <= 15; iTabLoop++)
	{
		snprintf (sFileName, MAX_PATHFILE, "dungeong_%02i.png", iTabLoop);
		PreLoad (PNG_TABS, sFileName, &imgtabgd[iTabLoop]);
		snprintf (sFileName, MAX_PATHFILE, "palaceg_%02i.png", iTabLoop);
		PreLoad (PNG_TABS, sFileName, &imgtabgp[iTabLoop]);
	}

	/*** Object miniatures. ***/
	for (iMinLoop = 0x00; iMinLoop <= 0x0B; iMinLoop++)
	{
		snprintf (sFileName, MAX_PATHFILE, "0x%02x.png", iMinLoop);
		PreLoad (PNG_MINIATURES, sFileName, &imgm[iMinLoop]);
	}
	PreLoad (PNG_MINIATURES, "unknown.png", &imgm[0x0C]);

	/*** various ***/
	PreLoad (PNG_VARIOUS, "black.png", &imgblack);
	PreLoad (PNG_VARIOUS, "unknown.png", &imgunkobject[1]);
	PreLoad (PNG_VARIOUS, "sel_unknown.png", &imgunkobject[2]);
	PreLoad (PNG_VARIOUS, "gunknown.png", &imgunkgraphics[1]);
	PreLoad (PNG_VARIOUS, "sel_gunknown.png", &imgunkgraphics[2]);
	PreLoad (PNG_VARIOUS, "sel_room_current.png", &imgsrc);
	PreLoad (PNG_VARIOUS, "sel_room_start.png", &imgsrs);
	PreLoad (PNG_VARIOUS, "chk_black.png", &imgchkbl);
	PreLoad (PNG_VARIOUS, "border_small_live.png", &imgbordersl);
	PreLoad (PNG_VARIOUS, "border_big_live.png", &imgborderbl);
	PreLoad (PNG_VARIOUS, "faded_l.png", &imgfadedl);
	PreLoad (PNG_VARIOUS, "popup_yn.png", &imgpopup_yn);
	PreLoad (PNG_VARIOUS, "help.png", &imghelp);
	PreLoad (PNG_VARIOUS, "exe.png", &imgexe);
	PreLoad (PNG_VARIOUS, "faded_s.png", &imgfadeds);
	PreLoad (PNG_VARIOUS, "custom_hover.png", &imgchover);
	PreLoad (PNG_VARIOUS, "Mednafen.png", &imgmednafen);
	PreLoad (PNG_VARIOUS, "EU_only.png", &imgeuonly);
	PreLoad (PNG_VARIOUS, "status_0.png", &imgstatus0);
	PreLoad (PNG_VARIOUS, "status_1.png", &imgstatus1);
	if (iController != 1)
	{
		PreLoad (PNG_VARIOUS, "border_big.png", &imgborderb);
		PreLoad (PNG_VARIOUS, "border_small.png", &imgborders);
		PreLoad (PNG_VARIOUS, "enumerate.png", &imgenumerate);
		PreLoad (PNG_VARIOUS, "level_bar.png", &imgbar);
		PreLoad (PNG_VARIOUS, "popup.png", &imgpopup);
		PreLoad (PNG_VARIOUS, "rooms.png", &imgrooms);
		PreLoad (PNG_VARIOUS, "tiles.png", &imgtiles);
	} else {
		PreLoad (PNG_GAMEPAD, "border_big.png", &imgborderb);
		PreLoad (PNG_GAMEPAD, "border_small.png", &imgborders);
		PreLoad (PNG_GAMEPAD, "enumerate.png", &imgenumerate);
		PreLoad (PNG_GAMEPAD, "level_bar.png", &imgbar);
		PreLoad (PNG_GAMEPAD, "popup.png", &imgpopup);
		PreLoad (PNG_GAMEPAD, "rooms.png", &imgrooms);
		PreLoad (PNG_GAMEPAD, "tiles.png", &imgtiles);
	}

	/*** (s)living ***/
	PreLoad (PNG_LIVING, "prince_l.png", &imgprincel[1]);
	PreLoad (PNG_SLIVING, "prince_l.png", &imgprincel[2]);
	PreLoad (PNG_LIVING, "prince_r.png", &imgprincer[1]);
	PreLoad (PNG_SLIVING, "prince_r.png", &imgprincer[2]);
	PreLoad (PNG_LIVING, "guard_l.png", &imgguardl[1]);
	PreLoad (PNG_SLIVING, "guard_l.png", &imgguardl[2]);
	PreLoad (PNG_LIVING, "guard_r.png", &imgguardr[1]);
	PreLoad (PNG_SLIVING, "guard_r.png", &imgguardr[2]);
	PreLoad (PNG_LIVING, "skeleton_l.png", &imgskeletonl[1]);
	PreLoad (PNG_SLIVING, "skeleton_l.png", &imgskeletonl[2]);
	PreLoad (PNG_LIVING, "skeleton_r.png", &imgskeletonr[1]);
	PreLoad (PNG_SLIVING, "skeleton_r.png", &imgskeletonr[2]);
	PreLoad (PNG_LIVING, "fat_l.png", &imgfatl[1]);
	PreLoad (PNG_SLIVING, "fat_l.png", &imgfatl[2]);
	PreLoad (PNG_LIVING, "fat_r.png", &imgfatr[1]);
	PreLoad (PNG_SLIVING, "fat_r.png", &imgfatr[2]);
	PreLoad (PNG_LIVING, "shadow_l.png", &imgshadowl[1]);
	PreLoad (PNG_SLIVING, "shadow_l.png", &imgshadowl[2]);
	PreLoad (PNG_LIVING, "shadow_r.png", &imgshadowr[1]);
	PreLoad (PNG_SLIVING, "shadow_r.png", &imgshadowr[2]);
	PreLoad (PNG_LIVING, "jaffar_l.png", &imgjaffarl[1]);
	PreLoad (PNG_SLIVING, "jaffar_l.png", &imgjaffarl[2]);
	PreLoad (PNG_LIVING, "jaffar_r.png", &imgjaffarr[1]);
	PreLoad (PNG_SLIVING, "jaffar_r.png", &imgjaffarr[2]);

	/*** buttons ***/
	PreLoad (PNG_BUTTONS, "up_0.png", &imgup_0);
	PreLoad (PNG_BUTTONS, "up_1.png", &imgup_1);
	PreLoad (PNG_BUTTONS, "down_0.png", &imgdown_0);
	PreLoad (PNG_BUTTONS, "down_1.png", &imgdown_1);
	PreLoad (PNG_BUTTONS, "left_0.png", &imgleft_0);
	PreLoad (PNG_BUTTONS, "left_1.png", &imgleft_1);
	PreLoad (PNG_BUTTONS, "right_0.png", &imgright_0);
	PreLoad (PNG_BUTTONS, "right_1.png", &imgright_1);
	PreLoad (PNG_BUTTONS, "up_down_no.png", &imgudno);
	PreLoad (PNG_BUTTONS, "left_right_no.png", &imglrno);
	if (iController != 1)
	{
		PreLoad (PNG_BUTTONS, "close_big_0.png", &imgclosebig_0);
		PreLoad (PNG_BUTTONS, "close_big_1.png", &imgclosebig_1);
		PreLoad (PNG_BUTTONS, "enum_off.png", &imgenumoff);
		PreLoad (PNG_BUTTONS, "enum_on_0.png", &imgenumon_0);
		PreLoad (PNG_BUTTONS, "enum_on_1.png", &imgenumon_1);
		PreLoad (PNG_BUTTONS, "next_off.png", &imgnextoff);
		PreLoad (PNG_BUTTONS, "next_on_0.png", &imgnexton_0);
		PreLoad (PNG_BUTTONS, "next_on_1.png", &imgnexton_1);
		PreLoad (PNG_BUTTONS, "No.png", &imgno[1]);
		PreLoad (PNG_BUTTONS, "OK.png", &imgok[1]);
		PreLoad (PNG_BUTTONS, "previous_off.png", &imgprevoff);
		PreLoad (PNG_BUTTONS, "previous_on_0.png", &imgprevon_0);
		PreLoad (PNG_BUTTONS, "previous_on_1.png", &imgprevon_1);
		PreLoad (PNG_BUTTONS, "quit_0.png", &imgquit_0);
		PreLoad (PNG_BUTTONS, "quit_1.png", &imgquit_1);
		PreLoad (PNG_BUTTONS, "rooms_off.png", &imgroomsoff);
		PreLoad (PNG_BUTTONS, "rooms_on_0.png", &imgroomson_0);
		PreLoad (PNG_BUTTONS, "rooms_on_1.png", &imgroomson_1);
		PreLoad (PNG_BUTTONS, "save_off.png", &imgsaveoff);
		PreLoad (PNG_BUTTONS, "save_on_0.png", &imgsaveon_0);
		PreLoad (PNG_BUTTONS, "save_on_1.png", &imgsaveon_1);
		PreLoad (PNG_BUTTONS, "Save.png", &imgsave[1]);
		PreLoad (PNG_BUTTONS, "sel_No.png", &imgno[2]);
		PreLoad (PNG_BUTTONS, "sel_OK.png", &imgok[2]);
		PreLoad (PNG_BUTTONS, "sel_Save.png", &imgsave[2]);
		PreLoad (PNG_BUTTONS, "sel_Yes.png", &imgyes[2]);
		PreLoad (PNG_BUTTONS, "up_down_no_nfo.png", &imgudnonfo);
		PreLoad (PNG_BUTTONS, "Yes.png", &imgyes[1]);
	} else {
		PreLoad (PNG_GAMEPAD, "close_big_0.png", &imgclosebig_0);
		PreLoad (PNG_GAMEPAD, "close_big_1.png", &imgclosebig_1);
		PreLoad (PNG_GAMEPAD, "enum_off.png", &imgenumoff);
		PreLoad (PNG_GAMEPAD, "enum_on_0.png", &imgenumon_0);
		PreLoad (PNG_GAMEPAD, "enum_on_1.png", &imgenumon_1);
		PreLoad (PNG_GAMEPAD, "next_off.png", &imgnextoff);
		PreLoad (PNG_GAMEPAD, "next_on_0.png", &imgnexton_0);
		PreLoad (PNG_GAMEPAD, "next_on_1.png", &imgnexton_1);
		PreLoad (PNG_GAMEPAD, "No.png", &imgno[1]);
		PreLoad (PNG_GAMEPAD, "OK.png", &imgok[1]);
		PreLoad (PNG_GAMEPAD, "previous_off.png", &imgprevoff);
		PreLoad (PNG_GAMEPAD, "previous_on_0.png", &imgprevon_0);
		PreLoad (PNG_GAMEPAD, "previous_on_1.png", &imgprevon_1);
		PreLoad (PNG_GAMEPAD, "quit_0.png", &imgquit_0);
		PreLoad (PNG_GAMEPAD, "quit_1.png", &imgquit_1);
		PreLoad (PNG_GAMEPAD, "rooms_off.png", &imgroomsoff);
		PreLoad (PNG_GAMEPAD, "rooms_on_0.png", &imgroomson_0);
		PreLoad (PNG_GAMEPAD, "rooms_on_1.png", &imgroomson_1);
		PreLoad (PNG_GAMEPAD, "save_off.png", &imgsaveoff);
		PreLoad (PNG_GAMEPAD, "save_on_0.png", &imgsaveon_0);
		PreLoad (PNG_GAMEPAD, "save_on_1.png", &imgsaveon_1);
		PreLoad (PNG_GAMEPAD, "Save.png", &imgsave[1]);
		PreLoad (PNG_GAMEPAD, "sel_No.png", &imgno[2]);
		PreLoad (PNG_GAMEPAD, "sel_OK.png", &imgok[2]);
		PreLoad (PNG_GAMEPAD, "sel_Save.png", &imgsave[2]);
		PreLoad (PNG_GAMEPAD, "sel_Yes.png", &imgyes[2]);
		PreLoad (PNG_GAMEPAD, "up_down_no_nfo.png", &imgudnonfo);
		PreLoad (PNG_GAMEPAD, "Yes.png", &imgyes[1]);
	}

	/*** extras ***/
	PreLoad (PNG_EXTRAS, "extras_00.png", &imgextras[0]);
	PreLoad (PNG_EXTRAS, "extras_01.png", &imgextras[1]);
	PreLoad (PNG_EXTRAS, "extras_02.png", &imgextras[2]);
	PreLoad (PNG_EXTRAS, "extras_03.png", &imgextras[3]);
	PreLoad (PNG_EXTRAS, "extras_04.png", &imgextras[4]);
	PreLoad (PNG_EXTRAS, "extras_05.png", &imgextras[5]);
	PreLoad (PNG_EXTRAS, "extras_06.png", &imgextras[6]);
	PreLoad (PNG_EXTRAS, "extras_07.png", &imgextras[7]);
	PreLoad (PNG_EXTRAS, "extras_08.png", &imgextras[8]);
	PreLoad (PNG_EXTRAS, "extras_09.png", &imgextras[9]);
	PreLoad (PNG_EXTRAS, "extras_10.png", &imgextras[10]);

	/*** One final update of the bar. ***/
	LoadingBar (BAR_FULL);

	if (iDebug == 1)
		{ printf ("[ INFO ] Preloaded images: %i\n", iPreLoaded); }
	SDL_SetCursor (curArrow);

	/*** Defaults. ***/
	iCurLevel = iStartLevel;
	iCurX = arLevelStartingX[iCurLevel];
	iCurY = arLevelStartingY[iCurLevel];
	iDownAt = 0;
	iSelectedX = 1; /*** Start with the upper... ***/
	iSelectedY = 1; /*** ...left selected. ***/
	iScreen = 1;

	iTTP1 = TTPD_1;
	iTTPO = TTPD_O;
	iDX = DD_X;
	iDY = DD_Y;
	iHor[0] = (iDX * -1) + OFFSETD_X;
	iHor[1] = (iDX * 0) + OFFSETD_X;
	iHor[2] = (iDX * 1) + OFFSETD_X;
	iHor[3] = (iDX * 2) + OFFSETD_X;
	iHor[4] = (iDX * 3) + OFFSETD_X;
	iHor[5] = (iDX * 4) + OFFSETD_X;
	iHor[6] = (iDX * 5) + OFFSETD_X;
	iHor[7] = (iDX * 6) + OFFSETD_X;
	iHor[8] = (iDX * 7) + OFFSETD_X;
	iHor[9] = (iDX * 8) + OFFSETD_X;
	iHor[10] = (iDX * 9) + OFFSETD_X;
	iVer0 = OFFSETD_Y - iTTP1 - (iDY * 1);
	iVer1 = OFFSETD_Y - iTTP1 + (iDY * 0);
	iVer2 = OFFSETD_Y - iTTP1 + (iDY * 1);
	iVer3 = OFFSETD_Y - iTTP1 + (iDY * 2);
	iVer4 = OFFSETD_Y - iTTP1 + (iDY * 3);

	ShowScreen();
	InitPopUp();
	while (1)
	{
		while (SDL_PollEvent (&event))
		{
			switch (event.type)
			{
				case SDL_CONTROLLERBUTTONDOWN:
					/*** Nothing for now. ***/
					break;
				case SDL_CONTROLLERBUTTONUP:
					switch (event.cbutton.button)
					{
						case SDL_CONTROLLER_BUTTON_A:
							InitScreenAction ("enter");
							break;
						case SDL_CONTROLLER_BUTTON_B:
							switch (iScreen)
							{
								case 1:
									Quit(); break;
								case 2:
									/*** no break ***/
								case 3:
									iScreen = 1; break;
							}
							break;
						case SDL_CONTROLLER_BUTTON_X:
							if (iScreen != 2)
							{
								iScreen = 2;
								PlaySound ("wav/screen2or3.wav");
							}
							break;
						case SDL_CONTROLLER_BUTTON_Y:
							if (iScreen != 3)
							{
								iScreen = 3;
								PlaySound ("wav/screen2or3.wav");
							}
							break;
						case SDL_CONTROLLER_BUTTON_GUIDE:
							if (iChanged != 0) { CallSave(); } break;
						case SDL_CONTROLLER_BUTTON_START:
							RunLevel (iCurLevel);
							break;
						case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
							if (iCurLevel != 1)
							{
								if ((iChanged != 0) && (IsSavingAllowed() == 1))
									{ InitPopUpSave(); }
								if ((iChanged != 0) && (IsSavingAllowed() == 0))
									{ Prev (0); } else { Prev (1); }
							}
							break;
						case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
							if (iCurLevel != iNrLevels)
							{
								if ((iChanged != 0) && (IsSavingAllowed() == 1))
									{ InitPopUpSave(); }
								if ((iChanged != 0) && (IsSavingAllowed() == 0))
									{ Next (0); } else { Next (1); }
							}
							break;
						case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
							InitScreenAction ("left"); break;
						case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
							InitScreenAction ("right"); break;
						case SDL_CONTROLLER_BUTTON_DPAD_UP:
							InitScreenAction ("up"); break;
						case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
							InitScreenAction ("down"); break;
					}
					ShowScreen();
					break;
				case SDL_CONTROLLERAXISMOTION: /*** triggers and analog sticks ***/
					iXJoy1 = SDL_JoystickGetAxis (joystick, 0);
					iYJoy1 = SDL_JoystickGetAxis (joystick, 1);
					iXJoy2 = SDL_JoystickGetAxis (joystick, 3);
					iYJoy2 = SDL_JoystickGetAxis (joystick, 4);
					if ((iXJoy1 < -30000) || (iXJoy2 < -30000)) /*** left ***/
					{
						if ((SDL_GetTicks() - joyleft) > 300)
						{
							if (iScreen == 1)
							{
								if (iCurX > 1)
								{
									iCurX--;
									PlaySound ("wav/scroll.wav");
								}
							}
							joyleft = SDL_GetTicks();
						}
					}
					if ((iXJoy1 > 30000) || (iXJoy2 > 30000)) /*** right ***/
					{
						if ((SDL_GetTicks() - joyright) > 300)
						{
							if (iScreen == 1)
							{
								if (iCurX < arLevelWidth[iCurLevel])
								{
									iCurX++;
									PlaySound ("wav/scroll.wav");
								}
							}
							joyright = SDL_GetTicks();
						}
					}
					if ((iYJoy1 < -30000) || (iYJoy2 < -30000)) /*** up ***/
					{
						if ((SDL_GetTicks() - joyup) > 300)
						{
							if (iScreen == 1)
							{
								if (iCurY > 1)
								{
									iCurY--;
									PlaySound ("wav/scroll.wav");
								}
							}
							joyup = SDL_GetTicks();
						}
					}
					if ((iYJoy1 > 30000) || (iYJoy2 > 30000)) /*** down ***/
					{
						if ((SDL_GetTicks() - joydown) > 300)
						{
							if (iScreen == 1)
							{
								if (iCurY < arLevelHeight[iCurLevel])
								{
									iCurY++;
									PlaySound ("wav/scroll.wav");
								}
							}
							joydown = SDL_GetTicks();
						}
					}
					if (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
					{
						if ((SDL_GetTicks() - trigleft) > 300)
						{
							if (iScreen == 2)
							{
								iOldWidth = arLevelWidth[iCurLevel];
								iOldHeight = arLevelHeight[iCurLevel];
								ChangeCustom (-1, &arLevelHeight[iCurLevel], 1, 24);
								LevelResized (iOldWidth, iOldHeight);
							}
							trigleft = SDL_GetTicks();
						}
					}
					if (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
					{
						if ((SDL_GetTicks() - trigright) > 300)
						{
							if (iScreen == 2)
							{
								iOldWidth = arLevelWidth[iCurLevel];
								iOldHeight = arLevelHeight[iCurLevel];
								ChangeCustom (1, &arLevelHeight[iCurLevel], 1, 24);
								LevelResized (iOldWidth, iOldHeight);
							}
							trigright = SDL_GetTicks();
						}
					}
					ShowScreen();
					break;
				case SDL_KEYDOWN: /*** https://wiki.libsdl.org/SDL2/SDL_Keycode ***/
					switch (event.key.keysym.sym)
					{
						case SDLK_F1:
							if (iScreen == 1)
							{
								Help(); SDL_SetCursor (curArrow);
							}
							break;
						case SDLK_F2:
							if (iScreen == 1)
							{
								EXE();
							}
							break;
						case SDLK_LEFTBRACKET:
							if (event.key.keysym.mod & KMOD_CTRL)
							{
								InitScreenAction ("lbracket ctrl");
							} else if (event.key.keysym.mod & KMOD_SHIFT) {
								InitScreenAction ("lbracket shift");
							} else {
								InitScreenAction ("lbracket");
							}
							break;
						case SDLK_RIGHTBRACKET:
							if (event.key.keysym.mod & KMOD_CTRL)
							{
								InitScreenAction ("rbracket ctrl");
							} else if (event.key.keysym.mod & KMOD_SHIFT) {
								InitScreenAction ("rbracket shift");
							} else {
								InitScreenAction ("rbracket");
							}
							break;
						case SDLK_COMMA:
							if (event.key.keysym.mod & KMOD_CTRL)
							{
								InitScreenAction ("comma ctrl");
							} else if (event.key.keysym.mod & KMOD_SHIFT) {
								InitScreenAction ("comma shift");
							} else {
								InitScreenAction ("comma");
							}
							break;
						case SDLK_PERIOD:
							if (event.key.keysym.mod & KMOD_CTRL)
							{
								InitScreenAction ("period ctrl");
							} else if (event.key.keysym.mod & KMOD_SHIFT) {
								InitScreenAction ("period shift");
							} else {
								InitScreenAction ("period");
							}
							break;
						case SDLK_d:
							RunLevel (iCurLevel);
							break;
						case SDLK_SLASH:
							if (iScreen == 1)
							{
								ClearRoom (iCurX, iCurY);
								PlaySound ("wav/ok_close.wav");
								iChanged++;
							}
							break;
						case SDLK_BACKSLASH:
							if (iScreen == 1) { RandomizeLevel(); }
							break;
						case SDLK_KP_ENTER:
						case SDLK_RETURN:
						case SDLK_SPACE:
							if (((event.key.keysym.mod & KMOD_LALT) ||
								(event.key.keysym.mod & KMOD_RALT)) && (iScreen == 1))
							{
								Zoom (1);
								iExtras = 0;
								PlaySound ("wav/extras.wav");
							} else {
								InitScreenAction ("enter");
							}
							break;
						case SDLK_ESCAPE:
						case SDLK_q:
							switch (iScreen)
							{
								case 1:
									Quit(); break;
								case 2:
									/*** no break ***/
								case 3:
									iScreen = 1; break;
							}
							break;
						case SDLK_LEFT:
							if ((event.key.keysym.mod & KMOD_LSHIFT) ||
								(event.key.keysym.mod & KMOD_RSHIFT))
							{
								if (iScreen == 1)
								{
									if (iCurX > 1)
									{
										iCurX--;
										PlaySound ("wav/scroll.wav");
									}
								}
							} else if ((event.key.keysym.mod & KMOD_LCTRL) ||
								(event.key.keysym.mod & KMOD_RCTRL))
							{
								/*** Nothing for now. ***/
							} else {
								InitScreenAction ("left");
							}
							break;
						case SDLK_RIGHT:
							if ((event.key.keysym.mod & KMOD_LSHIFT) ||
								(event.key.keysym.mod & KMOD_RSHIFT))
							{
								if (iScreen == 1)
								{
									if (iCurX < arLevelWidth[iCurLevel])
									{
										iCurX++;
										PlaySound ("wav/scroll.wav");
									}
								}
							} else if ((event.key.keysym.mod & KMOD_LCTRL) ||
								(event.key.keysym.mod & KMOD_RCTRL))
							{
								/*** Nothing for now. ***/
							} else {
								InitScreenAction ("right");
							}
							break;
						case SDLK_UP:
							if ((event.key.keysym.mod & KMOD_LSHIFT) ||
								(event.key.keysym.mod & KMOD_RSHIFT))
							{
								if (iScreen == 1)
								{
									if (iCurY > 1)
									{
										iCurY--;
										PlaySound ("wav/scroll.wav");
									}
								}
							} else {
								InitScreenAction ("up");
							}
							break;
						case SDLK_DOWN:
							if ((event.key.keysym.mod & KMOD_LSHIFT) ||
								(event.key.keysym.mod & KMOD_RSHIFT))
							{
								if (iScreen == 1)
								{
									if (iCurY < arLevelHeight[iCurLevel])
									{
										iCurY++;
										PlaySound ("wav/scroll.wav");
									}
								}
							} else {
								InitScreenAction ("down");
							}
							break;
						case SDLK_t:
							if (iScreen == 1) { InitScreenAction ("env"); }
							break;
						case SDLK_MINUS:
						case SDLK_KP_MINUS:
							if (iCurLevel != 1)
							{
								if ((iChanged != 0) && (IsSavingAllowed() == 1))
									{ InitPopUpSave(); }
								if ((iChanged != 0) && (IsSavingAllowed() == 0))
									{ Prev (0); } else { Prev (1); }
							}
							break;
						case SDLK_KP_PLUS:
						case SDLK_EQUALS:
							if (iCurLevel != iNrLevels)
							{
								if ((iChanged != 0) && (IsSavingAllowed() == 1))
									{ InitPopUpSave(); }
								if ((iChanged != 0) && (IsSavingAllowed() == 0))
									{ Next (0); } else { Next (1); }
							}
							break;
						case SDLK_r:
							if (iScreen != 2)
							{
								iScreen = 2;
								PlaySound ("wav/screen2or3.wav");
							}
							break;
						case SDLK_e:
							if (iScreen != 3)
							{
								iScreen = 3;
								PlaySound ("wav/screen2or3.wav");
							}
							break;
						case SDLK_s:
							if (iChanged != 0) { CallSave(); } break;
						case SDLK_z:
							if (iScreen == 1)
							{
								Zoom (0);
								iExtras = 0;
								PlaySound ("wav/extras.wav");
							}
							break;
						case SDLK_f:
							if (iScreen == 1)
							{
								Zoom (1);
								iExtras = 0;
								PlaySound ("wav/extras.wav");
							}
							break;
						case SDLK_QUOTE:
							if (iScreen == 1)
							{
								if ((event.key.keysym.mod & KMOD_LSHIFT) ||
									(event.key.keysym.mod & KMOD_RSHIFT))
								{
									/*** No Sprinkle(). ***/
								} else {
									SetLocation (((iCurX - 1) * WIDTH) + iSelectedX,
										((iCurY - 1) * HEIGHT) + iSelectedY,
										iLastObject, iLastGraphics);
									PlaySound ("wav/ok_close.wav");
									iChanged++;
								}
							}
							break;
						case SDLK_i:
							if (iScreen == 1)
							{
								if (iInfo == 0) { iInfo = 1; } else { iInfo = 0; }
							}
							break;
						case SDLK_0: /*** empty ***/
						case SDLK_KP_0:
							if (iScreen == 1)
							{
								SetLocation (((iCurX - 1) * WIDTH) + iSelectedX,
									((iCurY - 1) * HEIGHT) + iSelectedY, 0x00, -1);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						case SDLK_1: /*** floor ***/
						case SDLK_KP_1:
							if (iScreen == 1)
							{
								SetLocation (((iCurX - 1) * WIDTH) + iSelectedX,
									((iCurY - 1) * HEIGHT) + iSelectedY, 0x02, -1);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						case SDLK_2: /*** loose floor ***/
						case SDLK_KP_2:
							if (iScreen == 1)
							{
								SetLocation (((iCurX - 1) * WIDTH) + iSelectedX,
									((iCurY - 1) * HEIGHT) + iSelectedY, 0x06, -1);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						case SDLK_3: /*** closed gate ***/
						case SDLK_KP_3:
							if (iScreen == 1)
							{
								iGateX = ((iCurX - 1) * WIDTH) + iSelectedX;
								iGateY = ((iCurY - 1) * HEIGHT) + iSelectedY;
								SetLocation (iGateX, iGateY, 0x05, -1);
								SetAttribute (iGateX, iGateY,
									arGateX[iCurLevel], arGateY[iCurLevel],
									arGateState1[iCurLevel], arNrGates[iCurLevel], 0);
								SetAttribute (iGateX, iGateY,
									arGateX[iCurLevel], arGateY[iCurLevel],
									arGateState2[iCurLevel], arNrGates[iCurLevel], 0);
								SetAttribute (iGateX, iGateY,
								arGateX[iCurLevel], arGateY[iCurLevel],
									arGateState3[iCurLevel], arNrGates[iCurLevel], 0);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						case SDLK_4: /*** open gate ***/
						case SDLK_KP_4:
							if (iScreen == 1)
							{
								iGateX = ((iCurX - 1) * WIDTH) + iSelectedX;
								iGateY = ((iCurY - 1) * HEIGHT) + iSelectedY;
								SetLocation (iGateX, iGateY, 0x05, -1);
								SetAttribute (iGateX, iGateY,
									arGateX[iCurLevel], arGateY[iCurLevel],
									arGateState1[iCurLevel], arNrGates[iCurLevel], 3);
								SetAttribute (iGateX, iGateY,
									arGateX[iCurLevel], arGateY[iCurLevel],
									arGateState2[iCurLevel], arNrGates[iCurLevel], 12);
								SetAttribute (iGateX, iGateY,
								arGateX[iCurLevel], arGateY[iCurLevel],
									arGateState3[iCurLevel], arNrGates[iCurLevel], 0xFFFF);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						case SDLK_5: /*** torch ***/
						case SDLK_KP_5:
							if (iScreen == 1)
							{
								iGraphics = 0x00; /*** Default. ***/
								iTorchX = ((iCurX - 1) * WIDTH) + iSelectedX;
								if (arLevelType[iCurLevel] == 0)
								{ /*** dungeon ***/
									if (iTorchX % 2 == 0)
										{ iGraphics = 0x93; } else { iGraphics = 0x92; }
								} else { /*** palace ***/
									iGraphics = 0x08;
								}
								SetLocation (iTorchX,
									((iCurY - 1) * HEIGHT) + iSelectedY, 0x02, iGraphics);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						case SDLK_6: /*** spikes ***/
						case SDLK_KP_6:
							if (iScreen == 1)
							{
								SetLocation (((iCurX - 1) * WIDTH) + iSelectedX,
									((iCurY - 1) * HEIGHT) + iSelectedY, 0x07, -1);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						case SDLK_7: /*** small pillar ***/
						case SDLK_KP_7:
							/*** Nothing for now. ***/
							break;
						case SDLK_8: /*** chomper ***/
						case SDLK_KP_8:
							if (iScreen == 1)
							{
								SetLocation (((iCurX - 1) * WIDTH) + iSelectedX,
									((iCurY - 1) * HEIGHT) + iSelectedY, 0x08, -1);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						case SDLK_9: /*** wall ***/
						case SDLK_KP_9:
							if (iScreen == 1)
							{
								SetLocation (((iCurX - 1) * WIDTH) + iSelectedX,
									((iCurY - 1) * HEIGHT) + iSelectedY, 0x01, -1);
								PlaySound ("wav/ok_close.wav"); iChanged++;
							}
							break;
						default: break;
					}
					ShowScreen();
					break;
				case SDL_MOUSEMOTION:
					iOldXPos = iXPos;
					iOldYPos = iYPos;
					iXPos = event.motion.x;
					iYPos = event.motion.y;
					if ((iOldXPos == iXPos) && (iOldYPos == iYPos)) { break; }

					/*** Mednafen information. ***/
					if (OnLevelBar() == 1)
					{
						if (iMednafen != 1) { iMednafen = 1; ShowScreen(); }
					} else {
						if (iMednafen != 0) { iMednafen = 0; ShowScreen(); }
					}

					if (iScreen == 1)
					{
						/*** User hovers over tiles in the upper row. ***/
						if ((InArea (iHor[1], iVer1 + iTTP1, iHor[2], iVer2 + iTTPO)
							== 1) && ((iSelectedX != 1) || (iSelectedY != 1)))
							{ iSelectedX = 1; iSelectedY = 1; ShowScreen(); }
						else if ((InArea (iHor[2], iVer1 + iTTP1, iHor[3], iVer2 + iTTPO)
							== 1) && ((iSelectedX != 2) || (iSelectedY != 1)))
							{ iSelectedX = 2; iSelectedY = 1; ShowScreen(); }
						else if ((InArea (iHor[3], iVer1 + iTTP1, iHor[4], iVer2 + iTTPO)
							== 1) && ((iSelectedX != 3) || (iSelectedY != 1)))
							{ iSelectedX = 3; iSelectedY = 1; ShowScreen(); }
						else if ((InArea (iHor[4], iVer1 + iTTP1, iHor[5], iVer2 + iTTPO)
							== 1) && ((iSelectedX != 4) || (iSelectedY != 1)))
							{ iSelectedX = 4; iSelectedY = 1; ShowScreen(); }
						else if ((InArea (iHor[5], iVer1 + iTTP1, iHor[6], iVer2 + iTTPO)
							== 1) && ((iSelectedX != 5) || (iSelectedY != 1)))
							{ iSelectedX = 5; iSelectedY = 1; ShowScreen(); }
						else if ((InArea (iHor[6], iVer1 + iTTP1, iHor[7], iVer2 + iTTPO)
							== 1) && ((iSelectedX != 6) || (iSelectedY != 1)))
							{ iSelectedX = 6; iSelectedY = 1; ShowScreen(); }
						else if ((InArea (iHor[7], iVer1 + iTTP1, iHor[8], iVer2 + iTTPO)
							== 1) && ((iSelectedX != 7) || (iSelectedY != 1)))
							{ iSelectedX = 7; iSelectedY = 1; ShowScreen(); }
						else if ((InArea (iHor[8], iVer1 + iTTP1, iHor[9], iVer2 + iTTPO)
							== 1) && ((iSelectedX != 8) || (iSelectedY != 1)))
							{ iSelectedX = 8; iSelectedY = 1; ShowScreen(); }
						else if ((InArea (iHor[9], iVer1 + iTTP1, iHor[10], iVer2 + iTTPO)
							== 1) && ((iSelectedX != 9) || (iSelectedY != 1)))
							{ iSelectedX = 9; iSelectedY = 1; ShowScreen(); }
						else if ((InArea (iHor[10], iVer1 + iTTP1, iHor[10] + iDX,
							iVer2 + iTTPO) == 1) &&
							((iSelectedX != 10) || (iSelectedY != 1)))
						{ iSelectedX = 10; iSelectedY = 1; ShowScreen(); }

						/*** User hovers over tiles in the middle row. ***/
						else if ((InArea (iHor[1], iVer2 + iTTPO, iHor[2], iVer3 + iTTPO)
							== 1) && ((iSelectedX != 1) || (iSelectedY != 2)))
							{ iSelectedX = 1; iSelectedY = 2; ShowScreen(); }
						else if ((InArea (iHor[2], iVer2 + iTTPO, iHor[3], iVer3 + iTTPO)
							== 1) && ((iSelectedX != 2) || (iSelectedY != 2)))
							{ iSelectedX = 2; iSelectedY = 2; ShowScreen(); }
						else if ((InArea (iHor[3], iVer2 + iTTPO, iHor[4], iVer3 + iTTPO)
							== 1) && ((iSelectedX != 3) || (iSelectedY != 2)))
							{ iSelectedX = 3; iSelectedY = 2; ShowScreen(); }
						else if ((InArea (iHor[4], iVer2 + iTTPO, iHor[5], iVer3 + iTTPO)
							== 1) && ((iSelectedX != 4) || (iSelectedY != 2)))
							{ iSelectedX = 4; iSelectedY = 2; ShowScreen(); }
						else if ((InArea (iHor[5], iVer2 + iTTPO, iHor[6], iVer3 + iTTPO)
							== 1) && ((iSelectedX != 5) || (iSelectedY != 2)))
							{ iSelectedX = 5; iSelectedY = 2; ShowScreen(); }
						else if ((InArea (iHor[6], iVer2 + iTTPO, iHor[7], iVer3 + iTTPO)
							== 1) && ((iSelectedX != 6) || (iSelectedY != 2)))
							{ iSelectedX = 6; iSelectedY = 2; ShowScreen(); }
						else if ((InArea (iHor[7], iVer2 + iTTPO, iHor[8], iVer3 + iTTPO)
							== 1) && ((iSelectedX != 7) || (iSelectedY != 2)))
							{ iSelectedX = 7; iSelectedY = 2; ShowScreen(); }
						else if ((InArea (iHor[8], iVer2 + iTTPO, iHor[9], iVer3 + iTTPO)
							== 1) && ((iSelectedX != 8) || (iSelectedY != 2)))
							{ iSelectedX = 8; iSelectedY = 2; ShowScreen(); }
						else if ((InArea (iHor[9], iVer2 + iTTPO, iHor[10], iVer3 + iTTPO)
							== 1) && ((iSelectedX != 9) || (iSelectedY != 2)))
							{ iSelectedX = 9; iSelectedY = 2; ShowScreen(); }
						else if ((InArea (iHor[10], iVer2 + iTTPO, iHor[10] + iDX,
							iVer3 + iTTPO) == 1) &&
							((iSelectedX != 10) || (iSelectedY != 2)))
						{ iSelectedX = 10; iSelectedY = 2; ShowScreen(); }

						/*** User hovers over tiles in the bottom row. ***/
						else if ((InArea (iHor[1], iVer3 + iTTPO, iHor[2],
							iVer3 + iDY + iTTPO) == 1) &&
							((iSelectedX != 1) || (iSelectedY != 3)))
							{ iSelectedX = 1; iSelectedY = 3; ShowScreen(); }
						else if ((InArea (iHor[2], iVer3 + iTTPO, iHor[3],
							iVer3 + iDY + iTTPO) == 1) &&
							((iSelectedX != 2) || (iSelectedY != 3)))
							{ iSelectedX = 2; iSelectedY = 3; ShowScreen(); }
						else if ((InArea (iHor[3], iVer3 + iTTPO, iHor[4],
							iVer3 + iDY + iTTPO) == 1) &&
							((iSelectedX != 3) || (iSelectedY != 3)))
							{ iSelectedX = 3; iSelectedY = 3; ShowScreen(); }
						else if ((InArea (iHor[4], iVer3 + iTTPO, iHor[5],
							iVer3 + iDY + iTTPO) == 1) &&
							((iSelectedX != 4) || (iSelectedY != 3)))
							{ iSelectedX = 4; iSelectedY = 3; ShowScreen(); }
						else if ((InArea (iHor[5], iVer3 + iTTPO, iHor[6],
							iVer3 + iDY + iTTPO) == 1) &&
							((iSelectedX != 5) || (iSelectedY != 3)))
							{ iSelectedX = 5; iSelectedY = 3; ShowScreen(); }
						else if ((InArea (iHor[6], iVer3 + iTTPO, iHor[7],
							iVer3 + iDY + iTTPO) == 1) &&
							((iSelectedX != 6) || (iSelectedY != 3)))
							{ iSelectedX = 6; iSelectedY = 3; ShowScreen(); }
						else if ((InArea (iHor[7], iVer3 + iTTPO, iHor[8],
							iVer3 + iDY + iTTPO) == 1) &&
							((iSelectedX != 7) || (iSelectedY != 3)))
							{ iSelectedX = 7; iSelectedY = 3; ShowScreen(); }
						else if ((InArea (iHor[8], iVer3 + iTTPO, iHor[9],
							iVer3 + iDY + iTTPO) == 1) &&
							((iSelectedX != 8) || (iSelectedY != 3)))
							{ iSelectedX = 8; iSelectedY = 3; ShowScreen(); }
						else if ((InArea (iHor[9], iVer3 + iTTPO, iHor[10],
							iVer3 + iDY + iTTPO) == 1) &&
							((iSelectedX != 9) || (iSelectedY != 3)))
							{ iSelectedX = 9; iSelectedY = 3; ShowScreen(); }
						else if ((InArea (iHor[10], iVer3 + iTTPO, iHor[10] + iDX,
							iVer3 + iDY + iTTPO) == 1) &&
							((iSelectedX != 10) || (iSelectedY != 3)))
						{ iSelectedX = 10; iSelectedY = 3; ShowScreen(); }

						/*** extras ***/
						if ((InArea (610, 3, 619, 12) == 1) && (iExtras != 1))
							{ iExtras = 1; ShowScreen(); }
						else if ((InArea (620, 3, 629, 12) == 1) && (iExtras != 2))
							{ iExtras = 2; ShowScreen(); }
						else if ((InArea (630, 3, 639, 12) == 1) && (iExtras != 3))
							{ iExtras = 3; ShowScreen(); }
						else if ((InArea (640, 3, 649, 12) == 1) && (iExtras != 4))
							{ iExtras = 4; ShowScreen(); }
						else if ((InArea (650, 3, 659, 12) == 1) && (iExtras != 5))
							{ iExtras = 5; ShowScreen(); }
						else if ((InArea (610, 13, 619, 22) == 1) && (iExtras != 6))
							{ iExtras = 6; ShowScreen(); }
						else if ((InArea (620, 13, 629, 22) == 1) && (iExtras != 7))
							{ iExtras = 7; ShowScreen(); }
						else if ((InArea (630, 13, 639, 22) == 1) && (iExtras != 8))
							{ iExtras = 8; ShowScreen(); }
						else if ((InArea (640, 13, 649, 22) == 1) && (iExtras != 9))
							{ iExtras = 9; ShowScreen(); }
						else if ((InArea (650, 13, 659, 22) == 1) && (iExtras != 10))
							{ iExtras = 10; ShowScreen(); }
						else if ((InArea (610, 3, 659, 22) == 0) && (iExtras != 0))
							{ iExtras = 0; ShowScreen(); }
					}

					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == 1)
					{
						if (InArea (0, 50, 25, 442) == 1) /*** left arrow ***/
						{
							if (iCurX > 1) { iDownAt = 1; }
						}
						if (InArea (667, 50, 692, 442) == 1) /*** right arrow ***/
						{
							if (iCurX < arLevelWidth[iCurLevel]) { iDownAt = 2; }
						}
						if (InArea (25, 25, 667, 50) == 1) /*** up arrow ***/
						{
							if (iCurY > 1) { iDownAt = 3; }
						}
						if (InArea (25, 442, 667, 442 + 25) == 1) /*** down arrow ***/
						{
							if (iCurY < arLevelHeight[iCurLevel]) { iDownAt = 4; }
						}
						if (InArea (0, 25, 25, 50) == 1) /*** rooms ***/
						{
							iDownAt = 5;
						}
						if (InArea (667, 25, 692, 50) == 1) /*** enumerate ***/
						{
							iDownAt = 6;
						}
						if (InArea (0, 442, 25, 442 + 25) == 1) /*** save ***/
						{
							iDownAt = 7;
						}
						if (InArea (667, 442, 692, 442 + 25) == 1) /*** quit ***/
						{
							iDownAt = 8;
						}
						if (InArea (0, 0, 25, 25) == 1) /*** previous ***/
						{
							iDownAt = 9;
						}
						if (InArea (667, 0, 692, 25) == 1) /*** next ***/
						{
							iDownAt = 10;
						}
					}
					ShowScreen();
					break;
				case SDL_MOUSEBUTTONUP:
					iDownAt = 0;
					if (event.button.button == 1) /*** left mouse button, change ***/
					{
						if (InArea (0, 50, 25, 442) == 1) /*** left arrow ***/
						{
							if (iCurX > 1)
							{
								iCurX--;
								PlaySound ("wav/scroll.wav");
							}
						}
						if (InArea (667, 50, 692, 442) == 1) /*** right arrow ***/
						{
							if (iCurX < arLevelWidth[iCurLevel])
							{
								iCurX++;
								PlaySound ("wav/scroll.wav");
							}
						}
						if (InArea (25, 25, 667, 50) == 1) /*** up arrow ***/
						{
							if (iCurY > 1)
							{
								iCurY--;
								PlaySound ("wav/scroll.wav");
							}
						}
						if (InArea (25, 442, 667, 442 + 25) == 1) /*** down arrow ***/
						{
							if (iCurY < arLevelHeight[iCurLevel])
							{
								iCurY++;
								PlaySound ("wav/scroll.wav");
							}
						}
						if (InArea (0, 25, 25, 50) == 1) /*** rooms ***/
						{
							if (iScreen != 2)
							{
								iScreen = 2;
								PlaySound ("wav/screen2or3.wav");
							}
						}
						if (InArea (667, 25, 692, 50) == 1) /*** enumerate ***/
						{
							if (iScreen != 3)
							{
								iScreen = 3;
								PlaySound ("wav/screen2or3.wav");
							}
						}
						if (InArea (0, 442, 25, 442 + 25) == 1) /*** save ***/
						{
							if (iChanged != 0) { CallSave(); }
						}
						if (InArea (667, 442, 692, 442 + 25) == 1) /*** quit ***/
						{
							switch (iScreen)
							{
								case 1:
									Quit(); break;
								case 2:
									/*** no break ***/
								case 3:
									iScreen = 1; break;
							}
						}
						if (InArea (0, 0, 25, 25) == 1) /*** previous ***/
						{
							if ((iChanged != 0) && (IsSavingAllowed() == 1))
								{ InitPopUpSave(); }
							if ((iChanged != 0) && (IsSavingAllowed() == 0))
								{ Prev (0); } else { Prev (1); }
							ShowScreen(); break;
						}
						if (InArea (667, 0, 692, 25) == 1) /*** next ***/
						{
							if ((iChanged != 0) && (IsSavingAllowed() == 1))
								{ InitPopUpSave(); }
							if ((iChanged != 0) && (IsSavingAllowed() == 0))
								{ Next (0); } else { Next (1); }
							ShowScreen(); break;
						}
						if (OnLevelBar() == 1) /*** level bar ***/
						{
							RunLevel (iCurLevel);
						}

						if (iScreen == 1)
						{
							if (InArea (iHor[1], iVer1 + iTTP1, iHor[10] + iDX,
								iVer3 + iDY + iTTPO) == 1) /*** middle field ***/
							{
								keystate = SDL_GetKeyboardState (NULL);
								if ((keystate[SDL_SCANCODE_LSHIFT]) ||
									(keystate[SDL_SCANCODE_RSHIFT]))
								{
									SetLocation (((iCurX - 1) * WIDTH) + iSelectedX,
										((iCurY - 1) * HEIGHT) + iSelectedY,
										iLastObject, iLastGraphics);
									PlaySound ("wav/ok_close.wav"); iChanged++;
								} else {
									ChangePos();
									ShowScreen(); break; /*** ? ***/
								}
							}

							/*** 1 ***/
							if (InArea (610, 3, 619, 12) == 1)
							{
								Zoom (0);
								iExtras = 0;
								PlaySound ("wav/extras.wav");
							}

							/*** 4 ***/
							if (InArea (640, 3, 649, 12) == 1)
							{
								InitScreenAction ("env");
							}

							/*** 6 ***/
							if (InArea (610, 13, 619, 22) == 1)
							{
								/*** No Sprinkle(). ***/
							}

							/*** 8 ***/
							if (InArea (630, 13, 639, 22) == 1)
							{
								/*** No FlipRoom(). ***/
							}

							/*** 3 ***/
							if (InArea (630, 3, 639, 12) == 1)
							{
								/*** No FlipRoom(). ***/
							}

							/*** 2 ***/
							if (InArea (620, 3, 629, 12) == 1)
							{
								/*** No CopyPaste(). ***/
							}

							/*** 7 ***/
							if (InArea (620, 13, 629, 22) == 1)
							{
								/*** No CopyPaste(). ***/
							}

							/*** 5 ***/
							if (InArea (650, 3, 659, 12) == 1)
							{
								Help(); SDL_SetCursor (curArrow);
							}

							/*** 10 ***/
							if (InArea (650, 13, 659, 22) == 1)
							{
								EXE();
							}
						}

						if (iScreen == 2) /*** rooms screen ***/
						{
							iOldWidth = arLevelWidth[iCurLevel];
							iOldHeight = arLevelHeight[iCurLevel];

							/*** width ***/
							PlusMinus (&arLevelWidth[iCurLevel], 321, 75, 1, 24, -10, 1);
							PlusMinus (&arLevelWidth[iCurLevel], 336, 75, 1, 24, -1, 1);
							PlusMinus (&arLevelWidth[iCurLevel], 406, 75, 1, 24, 1, 1);
							PlusMinus (&arLevelWidth[iCurLevel], 421, 75, 1, 24, 10, 1);

							/*** height ***/
							PlusMinus (&arLevelHeight[iCurLevel], 535, 75, 1, 24, -10, 1);
							PlusMinus (&arLevelHeight[iCurLevel], 550, 75, 1, 24, -1, 1);
							PlusMinus (&arLevelHeight[iCurLevel], 620, 75, 1, 24, 1, 1);
							PlusMinus (&arLevelHeight[iCurLevel], 635, 75, 1, 24, 10, 1);

							LevelResized (iOldWidth, iOldHeight);
						}

						if (iScreen == 3) /*** enumerate screen ***/
						{
							/*** Nothing for now. ***/
						}
					}
					if (event.button.button == 2) /*** middle mouse button, clear ***/
					{
						if (iScreen == 1)
						{
							ClearRoom (iCurX, iCurY);
							PlaySound ("wav/ok_close.wav");
							iChanged++;
						}
					}
					if (event.button.button == 3) /*** right mouse button, randomize ***/
					{
						if (iScreen == 1) { RandomizeLevel(); }
					}
					ShowScreen();
					break;
				case SDL_MOUSEWHEEL:
					if (event.wheel.y > 0) /*** scroll wheel up ***/
					{
						if (InArea (iHor[1], iVer1 + iTTP1, iHor[10] + iDX,
							iVer3 + iDY + iTTPO) == 1) /*** middle field ***/
						{
							keystate = SDL_GetKeyboardState (NULL);
							if ((keystate[SDL_SCANCODE_LSHIFT]) ||
								(keystate[SDL_SCANCODE_RSHIFT]))
							{ /*** right ***/
								if (iCurX < arLevelWidth[iCurLevel])
								{
									iCurX++;
									PlaySound ("wav/scroll.wav");
								}
							} else { /*** up ***/
								if (iCurY > 1)
								{
									iCurY--;
									PlaySound ("wav/scroll.wav");
								}
							}
						}
					}
					if (event.wheel.y < 0) /*** scroll wheel down ***/
					{
						if (InArea (iHor[1], iVer1 + iTTP1, iHor[10] + iDX,
							iVer3 + iDY + iTTPO) == 1) /*** middle field ***/
						{
							keystate = SDL_GetKeyboardState (NULL);
							if ((keystate[SDL_SCANCODE_LSHIFT]) ||
								(keystate[SDL_SCANCODE_RSHIFT]))
							{ /*** left ***/
								if (iCurX > 1)
								{
									iCurX--;
									PlaySound ("wav/scroll.wav");
								}
							} else { /*** down ***/
								if (iCurY < arLevelHeight[iCurLevel])
								{
									iCurY++;
									PlaySound ("wav/scroll.wav");
								}
							}
						}
					}
					ShowScreen();
					break;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_EXPOSED)
						{ ShowScreen(); } break;
				case SDL_QUIT:
					Quit(); break;
				default: break;
			}
		}

		/*** prevent CPU eating ***/
		gamespeed = REFRESH;
		while ((SDL_GetTicks() - looptime) < gamespeed)
		{
			SDL_Delay (10);
		}
		looptime = SDL_GetTicks();
	}
}
/*****************************************************************************/
void InitPopUpSave (void)
/*****************************************************************************/
{
	int iPopUp;
	SDL_Event event;

	iPopUp = 1;

	PlaySound ("wav/popup_yn.wav");
	ShowPopUpSave();
	while (iPopUp == 1)
	{
		while (SDL_PollEvent (&event))
		{
			switch (event.type)
			{
				case SDL_CONTROLLERBUTTONDOWN:
					/*** Nothing for now. ***/
					break;
				case SDL_CONTROLLERBUTTONUP:
					switch (event.cbutton.button)
					{
						case SDL_CONTROLLER_BUTTON_A:
							CallSave(); iPopUp = 0; break;
						case SDL_CONTROLLER_BUTTON_B:
							iPopUp = 0; break;
					}
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
						case SDLK_n:
							iPopUp = 0; break;
						case SDLK_y:
							CallSave(); iPopUp = 0; break;
						default: break;
					}
					break;
				case SDL_MOUSEMOTION:
					iXPos = event.motion.x;
					iYPos = event.motion.y;
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == 1)
					{
						if (InArea (440, 323, 440 + 85, 323 + 32) == 1) /*** Yes ***/
						{
							iYesOn = 1;
							ShowPopUpSave();
						}
						if (InArea (167, 323, 167 + 85, 323 + 32) == 1) /*** No ***/
						{
							iNoOn = 1;
							ShowPopUpSave();
						}
					}
					break;
				case SDL_MOUSEBUTTONUP:
					iYesOn = 0;
					iNoOn = 0;
					if (event.button.button == 1)
					{
						if (InArea (440, 323, 440 + 85, 323 + 32) == 1) /*** Yes ***/
						{
							CallSave(); iPopUp = 0;
						}
						if (InArea (167, 323, 167 + 85, 323 + 32) == 1) /*** No ***/
						{
							iPopUp = 0;
						}
					}
					ShowPopUpSave(); break;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_EXPOSED)
						{ ShowScreen(); ShowPopUpSave(); } break;
				case SDL_QUIT:
					Quit(); break;
			}
		}

		/*** prevent CPU eating ***/
		gamespeed = REFRESH;
		while ((SDL_GetTicks() - looptime) < gamespeed)
		{
			SDL_Delay (10);
		}
		looptime = SDL_GetTicks();
	}
	PlaySound ("wav/popup_close.wav");
	ShowScreen();
}
/*****************************************************************************/
void ShowPopUpSave (void)
/*****************************************************************************/
{
	char arText[9 + 2][MAX_TEXT + 2];

	/*** faded background ***/
	ShowImage (imgfadedl, 0, 0, "imgfadedl");

	/*** popup ***/
	ShowImage (imgpopup_yn, 150, 95, "imgpopup_yn");

	/*** Yes ***/
	switch (iYesOn)
	{
		case 0: ShowImage (imgyes[1], 440, 323, "imgyes[1]"); break; /*** off ***/
		case 1: ShowImage (imgyes[2], 440, 323, "imgyes[2]"); break; /*** on ***/
	}

	/*** No ***/
	switch (iNoOn)
	{
		case 0: ShowImage (imgno[1], 167, 323, "imgno[1]"); break; /*** off ***/
		case 1: ShowImage (imgno[2], 167, 323, "imgno[2]"); break; /*** on ***/
	}

	if (iChanged == 1)
	{
		snprintf (arText[0], MAX_TEXT, "%s", "You made an unsaved change.");
		snprintf (arText[1], MAX_TEXT, "%s", "Do you want to save it?");
	} else {
		snprintf (arText[0], MAX_TEXT, "%s", "There are unsaved changes.");
		snprintf (arText[1], MAX_TEXT, "%s", "Do you wish to save these?");
	}

	DisplayText (180, 124, FONT_SIZE_15, arText, 2, font1, color_wh, color_bl);

	/*** refresh screen ***/
	SDL_RenderPresent (ascreen);
}
/*****************************************************************************/
void LoadFonts (void)
/*****************************************************************************/
{
	font1 = TTF_OpenFont ("ttf/Bitstream-Vera-Sans-Bold.ttf",
		FONT_SIZE_15 * iScale);
	if (font1 == NULL) { printf ("[FAILED] Font gone!\n"); exit (EXIT_ERROR); }
	font2 = TTF_OpenFont ("ttf/Bitstream-Vera-Sans-Bold.ttf",
		FONT_SIZE_11 * iScale);
	if (font2 == NULL) { printf ("[FAILED] Font gone!\n"); exit (EXIT_ERROR); }
	font3 = TTF_OpenFont ("ttf/Bitstream-Vera-Sans-Bold.ttf",
		FONT_SIZE_20 * iScale);
	if (font3 == NULL) { printf ("[FAILED] Font gone!\n"); exit (EXIT_ERROR); }
}
/*****************************************************************************/
void MixAudio (void *unused, Uint8 *stream, int iLen)
/*****************************************************************************/
{
	int iTemp;
	int iAmount;

	if (unused != NULL) { } /*** To prevent warnings. ***/

	SDL_memset (stream, 0, iLen); /*** SDL2 ***/
	for (iTemp = 0; iTemp < NUM_SOUNDS; iTemp++)
	{
		iAmount = (sounds[iTemp].dlen-sounds[iTemp].dpos);
		if (iAmount > iLen)
		{
			iAmount = iLen;
		}
		SDL_MixAudio (stream, &sounds[iTemp].data[sounds[iTemp].dpos], iAmount,
			SDL_MIX_MAXVOLUME);
		sounds[iTemp].dpos += iAmount;
	}
}
/*****************************************************************************/
void PlaySound (char *sFile)
/*****************************************************************************/
{
	int iIndex;
	SDL_AudioSpec wave;
	Uint8 *data;
	Uint32 dlen;
	SDL_AudioCVT cvt;

	if (iNoAudio == 1) { return; }
	for (iIndex = 0; iIndex < NUM_SOUNDS; iIndex++)
	{
		if (sounds[iIndex].dpos == sounds[iIndex].dlen)
		{
			break;
		}
	}
	if (iIndex == NUM_SOUNDS) { return; }

	if (SDL_LoadWAV (sFile, &wave, &data, &dlen) == NULL)
	{
		printf ("[FAILED] Could not load %s: %s!\n", sFile, SDL_GetError());
		exit (EXIT_ERROR);
	}
	SDL_BuildAudioCVT (&cvt, wave.format, wave.channels, wave.freq, AUDIO_S16, 2,
		44100);
	/*** The "+ 1" is a workaround for SDL bug #2274. ***/
	cvt.buf = (Uint8 *)malloc (dlen * (cvt.len_mult + 1));
	memcpy (cvt.buf, data, dlen);
	cvt.len = dlen;
	SDL_ConvertAudio (&cvt);
	SDL_FreeWAV (data);

	if (sounds[iIndex].data)
	{
		free (sounds[iIndex].data);
	}
	SDL_LockAudio();
	sounds[iIndex].data = cvt.buf;
	sounds[iIndex].dlen = cvt.len_cvt;
	sounds[iIndex].dpos = 0;
	SDL_UnlockAudio();
}
/*****************************************************************************/
void PreLoadSet (int iTile)
/*****************************************************************************/
{
	char sDir[MAX_PATHFILE + 2];
	char sImage[MAX_IMG + 2];
	int iBarHeight;

	snprintf (sDir, MAX_PATHFILE, "png%sobject%s", SLASH, SLASH);
	snprintf (sImage, MAX_IMG, "%s0x%02x.png", sDir, iTile);
	imgo[iTile][1] = IMG_LoadTexture (ascreen, sImage); /*** regular ***/
	snprintf (sDir, MAX_PATHFILE, "png%ssobject%s", SLASH, SLASH);
	snprintf (sImage, MAX_IMG, "%s0x%02x.png", sDir, iTile);
	imgo[iTile][2] = IMG_LoadTexture (ascreen, sImage);
	if ((!imgo[iTile][1]) || (!imgo[iTile][2]))
	{
		printf ("[FAILED] IMG_LoadTexture: %s!\n", IMG_GetError());
		exit (EXIT_ERROR);
	}

	iPreLoaded+=2;
	iBarHeight = (int)(((float)iPreLoaded/(float)iNrToPreLoad) * BAR_FULL);
	if (iBarHeight >= iCurrentBarHeight + 10) { LoadingBar (iBarHeight); }
}
/*****************************************************************************/
void PreLoad (char *sPath, char *sPNG, SDL_Texture **imgImage)
/*****************************************************************************/
{
	char sImage[MAX_IMG + 2];
	int iBarHeight;

	snprintf (sImage, MAX_IMG, "png%s%s%s%s", SLASH, sPath, SLASH, sPNG);
	*imgImage = IMG_LoadTexture (ascreen, sImage);
	if (!*imgImage)
	{
		printf ("[FAILED] IMG_LoadTexture: %s!\n", IMG_GetError());
		exit (EXIT_ERROR);
	}

	iPreLoaded++;
	iBarHeight = (int)(((float)iPreLoaded/(float)iNrToPreLoad) * BAR_FULL);
	if (iBarHeight >= iCurrentBarHeight + 10) { LoadingBar (iBarHeight); }
}
/*****************************************************************************/
void ShowScreen (void)
/*****************************************************************************/
{
	int iTile;
	int iLocX, iLocY;
	int iHorL, iVerL;
	char sLevelBar[MAX_TEXT + 2];
	char sLevelBarF[MAX_TEXT + 2];
	SDL_Texture *imgprince[2 + 2];
	SDL_Texture *imgguard[2 + 2];
	int iUnknownO, iUnknownG;
	int iObject, iGraphics;
	char arText[9 + 2][MAX_TEXT + 2];
	SDL_Color clr;
	int iPrinceX, iPrinceY;

	/*** Used for looping. ***/
	int iTileLoop;
	int iGuardLoop;
	int iDoorLoop;
	int iGateLoop;
	int iLooseLoop;
	int iRaiseLoop;
	int iDropLoop;
	int iChomperLoop;
	int iSpikeLoop;
	int iPotionLoop;

	/*** black background ***/
	ShowImage (imgblack, 0, 0, "imgblack");

	if (iScreen == 1)
	{
		/*** above this room ***/
		for (iTileLoop = 1; iTileLoop <= WIDTH; iTileLoop++)
		{
			iVerL = iVer0 - 32;
			if (iCurY > 1)
			{
				iGraphics = arLevelGraphics[iCurLevel]
					[((iCurX - 1) * WIDTH) + iTileLoop][((iCurY - 1) * HEIGHT)];
				iObject = arLevelObjects[iCurLevel]
					[((iCurX - 1) * WIDTH) + iTileLoop][((iCurY - 1) * HEIGHT)];
			} else {
				iGraphics = 0x00;
				iObject = 0x00;
			}
			if (imgd[iGraphics][1] == NULL)
				{ iUnknownG = 1; } else { iUnknownG = 0; }
			snprintf (sInfo, MAX_INFO, "gra1=%i", iGraphics);
			if (iUnknownG == 1)
			{
				ShowImage (imgunkgraphics[1], iHor[iTileLoop], iVer0, sInfo);
			} else {
				switch (arLevelType[iCurLevel])
				{
					case 0: /*** dungeon ***/
						ShowImage (imgd[iGraphics][1], iHor[iTileLoop], iVer0, sInfo);
						break;
					case 1: /*** palace ***/
						ShowImage (imgp[iGraphics][1], iHor[iTileLoop], iVer0, sInfo);
						break;
				}
			}
			if (imgo[iObject][1] == NULL)
				{ iUnknownO = 1; } else { iUnknownO = 0; }
			snprintf (sInfo, MAX_INFO, "obj1=%i", iObject);
			if (iUnknownO == 1)
			{
				ShowImage (imgunkobject[1], iHor[iTileLoop], iVerL, sInfo);
			} else {
				ShowImage (imgo[iObject][1], iHor[iTileLoop], iVerL, sInfo);
			}
		}

		/*** Room tiles: graphics. ***/
		for (iTileLoop = 1; iTileLoop <= TILES; iTileLoop++)
		{
			iLocX = 0; /*** To prevent warnings. ***/
			iLocY = 0; /*** To prevent warnings. ***/
			iHorL = iHor[0]; /*** To prevent warnings. ***/
			iVerL = iVer0; /*** To prevent warnings. ***/
			if ((iTileLoop >= 1) && (iTileLoop <= 10))
			{
				/*** Bottom row. ***/
				iLocX = iTileLoop;
				iLocY = 3;
				iHorL = iHor[iTileLoop];
				iVerL = iVer3;
			}
			if ((iTileLoop >= 11) && (iTileLoop <= 20))
			{
				/*** Middle row. ***/
				iLocX = iTileLoop - 10;
				iLocY = 2;
				iHorL = iHor[iTileLoop - 10];
				iVerL = iVer2;
			}
			if ((iTileLoop >= 21) && (iTileLoop <= 30))
			{
				/*** Top row. ***/
				iLocX = iTileLoop - 20;
				iLocY = 1;
				iHorL = iHor[iTileLoop - 20];
				iVerL = iVer1;
			}

			iGraphics = arLevelGraphics[iCurLevel]
				[((iCurX - 1) * WIDTH) + iLocX][((iCurY - 1) * HEIGHT) + iLocY];
			if (imgd[iGraphics][1] == NULL)
				{ iUnknownG = 1; } else { iUnknownG = 0; }
			if (iUnknownG == 1)
			{
				snprintf (sInfo, MAX_INFO, "gra1=%i", iGraphics);
				ShowImage (imgunkgraphics[1], iHorL, iVerL, sInfo);
			} else {
				switch (arLevelType[iCurLevel])
				{
					case 0: /*** dungeon ***/
						snprintf (sInfo, MAX_INFO, "gra1=%i", iGraphics);
						ShowImage (imgd[iGraphics][1], iHorL, iVerL, sInfo);
						break;
					case 1: /*** palace ***/
						snprintf (sInfo, MAX_INFO, "gra1=%i", iGraphics);
						ShowImage (imgp[iGraphics][1], iHorL, iVerL, sInfo);
						break;
				}
			}
			if ((iLocX == iSelectedX) && (iLocY == iSelectedY))
			{
				if (iUnknownG == 1)
				{
					snprintf (sInfo, MAX_INFO, "gra2=%i", iGraphics);
					ShowImage (imgunkgraphics[2], iHorL, iVerL, sInfo);
				} else {
					switch (arLevelType[iCurLevel])
					{
						case 0: /*** dungeon ***/
							snprintf (sInfo, MAX_INFO, "gra2=%i", iGraphics);
							ShowImage (imgd[iGraphics][2], iHorL, iVerL, sInfo);
							break;
						case 1: /*** palace ***/
							snprintf (sInfo, MAX_INFO, "gra2=%i", iGraphics);
							ShowImage (imgp[iGraphics][2], iHorL, iVerL, sInfo);
							break;
					}
				}
			}
		}

		/*** No need to show "under this room". ***/

		/*** One object: top row, room left. ***/
		if (iCurX > 1)
		{
			iTile = arLevelObjects[iCurLevel]
				[((iCurX - 1) * WIDTH)][((iCurY - 1) * HEIGHT) + 1];
		} else {
			iTile = 0x01;
		}
		snprintf (sInfo, MAX_INFO, "obj1=%i", iTile);
		iVerL = iVer1 - 32;
		ShowImage (imgo[iTile][1], iHor[0], iVerL, sInfo);
		ShowImage (imgfadeds, iHor[1], iVer1, "imgfadeds");

		/*** One object: middle row, room left. ***/
		if (iCurX > 1)
		{
			iTile = arLevelObjects[iCurLevel]
				[((iCurX - 1) * WIDTH)][((iCurY - 1) * HEIGHT) + 2];
		} else {
			iTile = 0x01;
		}
		snprintf (sInfo, MAX_INFO, "obj1=%i", iTile);
		iVerL = iVer2 - 32;
		ShowImage (imgo[iTile][1], iHor[0], iVerL, sInfo);
		ShowImage (imgfadeds, iHor[1], iVer2, "imgfadeds");

		/*** One object: 'top' row, room left down. ***/
		if (iCurX > 1) /*** left ***/
		{
			if (iCurY < arLevelHeight[iCurLevel]) /*** down ***/
			{
				iTile = arLevelObjects[iCurLevel]
					[((iCurX - 1) * WIDTH)][((iCurY - 1) * HEIGHT) + 4];
			} else {
				iTile = 0x01;
			}
		}
		snprintf (sInfo, MAX_INFO, "obj1=%i", iTile);
		iVerL = iVer4 - 32;
		/* This works, but, as with "under this room", there's no
		 * need to show these images.
		 */
		/*** ShowImage (imgo[iTile][1], iHor[0], iVerL, sInfo); ***/
		/*** ShowImage (imgfadeds, iHor[1], iVer4, "imgfadeds"); ***/

		/*** One object: bottom row, room left. ***/
		if (iCurX > 1)
		{
			iTile = arLevelObjects[iCurLevel]
				[((iCurX - 1) * WIDTH)][((iCurY - 1) * HEIGHT) + 3];
		} else {
			iTile = 0x01;
		}
		snprintf (sInfo, MAX_INFO, "obj1=%i", iTile);
		iVerL = iVer3 - 32;
		ShowImage (imgo[iTile][1], iHor[0], iVerL, sInfo);
		ShowImage (imgfadeds, iHor[1], iVer3, "imgfadeds");

		/*** Room tiles: objects. ***/
		for (iTileLoop = 1; iTileLoop <= TILES; iTileLoop++)
		{
			iLocX = 0; /*** To prevent warnings. ***/
			iLocY = 0; /*** To prevent warnings. ***/
			iHorL = iHor[0]; /*** To prevent warnings. ***/
			iVerL = iVer0; /*** To prevent warnings. ***/
			if ((iTileLoop >= 1) && (iTileLoop <= 10))
			{
				/*** Bottom row. ***/
				iLocX = iTileLoop;
				iLocY = 3;
				iHorL = iHor[iTileLoop];
				iVerL = iVer3 - 32;
			}
			if ((iTileLoop >= 11) && (iTileLoop <= 20))
			{
				/*** Middle row. ***/
				iLocX = iTileLoop - 10;
				iLocY = 2;
				iHorL = iHor[iTileLoop - 10];
				iVerL = iVer2 - 32;
			}
			if ((iTileLoop >= 21) && (iTileLoop <= 30))
			{
				/*** Top row. ***/
				iLocX = iTileLoop - 20;
				iLocY = 1;
				iHorL = iHor[iTileLoop - 20];
				iVerL = iVer1 - 32;
			}

			iObject = arLevelObjects[iCurLevel]
				[((iCurX - 1) * WIDTH) + iLocX][((iCurY - 1) * HEIGHT) + iLocY];
			if (imgo[iObject][1] == NULL)
				{ iUnknownO = 1; } else { iUnknownO = 0; }
			if (iUnknownO == 1)
			{
				snprintf (sInfo, MAX_INFO, "obj1=%i", iObject);
				ShowImage (imgunkobject[1], iHorL, iVerL, sInfo);
			} else {
				snprintf (sInfo, MAX_INFO, "obj1=%i", iObject);
				ShowImage (imgo[iObject][1], iHorL, iVerL, sInfo);
			}
			if ((iLocX == iSelectedX) && (iLocY == iSelectedY))
			{
				if (iUnknownO == 1)
				{
					snprintf (sInfo, MAX_INFO, "obj2=%i", iObject);
					ShowImage (imgunkobject[2], iHorL, iVerL, sInfo);
				} else {
					snprintf (sInfo, MAX_INFO, "obj2=%i", iObject);
					ShowImage (imgo[iObject][2], iHorL, iVerL, sInfo);
				}
			}

			/*** prince ***/
			if ((arPrinceX[iCurLevel] ==
				((iCurX - 1) * WIDTH) + iLocX) &&
				(arPrinceY[iCurLevel] ==
				((iCurY - 1) * HEIGHT) + iLocY))
			{
				switch (arPrinceDir[iCurLevel])
				{
					case 0x00: /*** l ***/
						imgprince[1] = imgprincel[1];
						imgprince[2] = imgprincel[2];
						break;
					case 0x800: /*** r ***/
						imgprince[1] = imgprincer[1];
						imgprince[2] = imgprincer[2];
						break;
					default:
						printf ("[FAILED] Incorrect prince direction: 0x%02x!\n",
							arPrinceDir[iCurLevel]);
						exit (EXIT_ERROR);
				}
				iPrinceX = iHorL + 7;
				iPrinceY = iVerL + 59;
				if ((iEXEType == 2) && (iCurLevel == 1)) /*** EU ***/
					{ iPrinceX+=12; }
				if (((iEXEType == 1) && (iCurLevel == 7)) || /*** US ***/
					((iEXEType == 2) && (iCurLevel == 9))) /*** EU ***/
					{ iPrinceY+=31; }
				ShowImage (imgprince[1], iPrinceX, iPrinceY, "imgprince[1]");
				if ((iLocX == iSelectedX) && (iLocY == iSelectedY))
					{ ShowImage (imgprince[2], iPrinceX, iPrinceY, "imgprince[2]"); }
			}

			/*** guard(s) ***/
			if (arNrGuards[iCurLevel] > 0)
			{
				for (iGuardLoop = 1; iGuardLoop <= arNrGuards[iCurLevel]; iGuardLoop++)
				{
					if ((arGuardX[iCurLevel][iGuardLoop] ==
						((iCurX - 1) * WIDTH) + iLocX) &&
						(arGuardY[iCurLevel][iGuardLoop] ==
						((iCurY - 1) * HEIGHT) + iLocY))
					{
						switch (arGuardDir[iCurLevel][iGuardLoop])
						{
							case 0x00: /*** l ***/
								switch (arGuardType[iCurLevel][iGuardLoop])
								{
									case 0: /*** guard ***/
										imgguard[1] = imgguardl[1];
										imgguard[2] = imgguardl[2];
										break;
									case 1: /*** skeleton ***/
										imgguard[1] = imgskeletonl[1];
										imgguard[2] = imgskeletonl[2];
										break;
									case 2: /*** fat ***/
										imgguard[1] = imgfatl[1];
										imgguard[2] = imgfatl[2];
										break;
									case 3: /*** shadow ***/
										imgguard[1] = imgshadowl[1];
										imgguard[2] = imgshadowl[2];
										break;
									case 4: /*** Jaffar ***/
										imgguard[1] = imgjaffarl[1];
										imgguard[2] = imgjaffarl[2];
										break;
									default:
										printf ("[FAILED] Incorrect guard type: %i!\n",
											arGuardType[iCurLevel][iGuardLoop]);
										exit (EXIT_ERROR);
								}
								break;
							case 0x800: /*** r ***/
								switch (arGuardType[iCurLevel][iGuardLoop])
								{
									case 0: /*** guard ***/
										imgguard[1] = imgguardr[1];
										imgguard[2] = imgguardr[2];
										break;
									case 1: /*** skeleton ***/
										imgguard[1] = imgskeletonr[1];
										imgguard[2] = imgskeletonr[2];
										break;
									case 2: /*** fat ***/
										imgguard[1] = imgfatr[1];
										imgguard[2] = imgfatr[2];
										break;
									case 3: /*** shadow ***/
										imgguard[1] = imgshadowr[1];
										imgguard[2] = imgshadowr[2];
										break;
									case 4: /*** Jaffar ***/
										imgguard[1] = imgjaffarr[1];
										imgguard[2] = imgjaffarr[2];
										break;
									default:
										printf ("[FAILED] Incorrect guard type: %i!\n",
											arGuardType[iCurLevel][iGuardLoop]);
										exit (EXIT_ERROR);
								}
								break;
							default:
								printf ("[FAILED] Incorrect guard direction: 0x%02x!\n",
									arGuardDir[iCurLevel][iGuardLoop]);
								exit (EXIT_ERROR);
						}
						ShowImage (imgguard[1], iHorL, iVerL + 65, "imgguard[1]");
						if ((iLocX == iSelectedX) && (iLocY == iSelectedY))
							{ ShowImage (imgguard[2], iHorL, iVerL + 65, "imgguard[2]"); }
						snprintf (arText[0], MAX_TEXT, "S:%i H:%i",
							arGuardSkill[iCurLevel][iGuardLoop],
							arGuardHP[iCurLevel][iGuardLoop]);
						DisplayText (iHorL, iVerL + 151 - FONT_SIZE_11,
							FONT_SIZE_11, arText, 1, font2, color_wh, color_bl);
					}
				}
			}

			/*** door ***/
			if (arNrDoors[iCurLevel] > 0)
			{
				for (iDoorLoop = 1; iDoorLoop <= arNrDoors[iCurLevel]; iDoorLoop++)
				{
					if ((arDoorX[iCurLevel][iDoorLoop] ==
						((iCurX - 1) * WIDTH) + iLocX) &&
						(arDoorY[iCurLevel][iDoorLoop] ==
						((iCurY - 1) * HEIGHT) + iLocY))
					{
						switch (arDoorType[iCurLevel][iDoorLoop])
						{
							case 0: /*** entrance ***/
								snprintf (arText[0], MAX_TEXT, "%s", "entrance"); break;
							case 2: /*** exit ***/
								snprintf (arText[0], MAX_TEXT, "%s", "exit"); break;
							default:
								snprintf (arText[0], MAX_TEXT, "%s", "?");
								printf ("[ WARN ] Strange door type: %i!\n",
									arDoorType[iCurLevel][iDoorLoop]);
								break;
						}
						DisplayText (iHorL, iVerL + 151 - FONT_SIZE_11,
							FONT_SIZE_11, arText, 1, font2, color_wh, color_bl);
					}
				}
			}

			/*** gate ***/
			if (arNrGates[iCurLevel] > 0)
			{
				for (iGateLoop = 1; iGateLoop <= arNrGates[iCurLevel]; iGateLoop++)
				{
					if ((arGateX[iCurLevel][iGateLoop] ==
						((iCurX - 1) * WIDTH) + iLocX) &&
						(arGateY[iCurLevel][iGateLoop] ==
						((iCurY - 1) * HEIGHT) + iLocY))
					{
						snprintf (arText[0], MAX_TEXT, "%i", iGateLoop - 1);
						snprintf (arText[1], MAX_TEXT, "%s", StateAsText (
							arGateState1[iCurLevel][iGateLoop],
							arGateState2[iCurLevel][iGateLoop],
							arGateState3[iCurLevel][iGateLoop]));
						DisplayText (iHorL, iVerL + 151 - 26,
							FONT_SIZE_11, arText, 2, font2, color_wh, color_bl);
					}
				}
			}

			/*** loose ***/
			if (arNrLoose[iCurLevel] > 0)
			{
				for (iLooseLoop = 1; iLooseLoop <= arNrLoose[iCurLevel]; iLooseLoop++)
				{
					if ((arLooseX[iCurLevel][iLooseLoop] ==
						((iCurX - 1) * WIDTH) + iLocX) &&
						(arLooseY[iCurLevel][iLooseLoop] ==
						((iCurY - 1) * HEIGHT) + iLocY))
					{
						if (iDebug == 1)
						{
							snprintf (arText[0], MAX_TEXT, "%s", "loose");
							DisplayText (iHorL, iVerL + 151 - FONT_SIZE_11,
								FONT_SIZE_11, arText, 1, font2, color_wh, color_bl);
						}
					}
				}
			}

			/*** raise ***/
			if (arNrRaise[iCurLevel] > 0)
			{
				for (iRaiseLoop = 1; iRaiseLoop <= arNrRaise[iCurLevel]; iRaiseLoop++)
				{
					if ((arRaiseX[iCurLevel][iRaiseLoop] ==
						((iCurX - 1) * WIDTH) + iLocX) &&
						(arRaiseY[iCurLevel][iRaiseLoop] ==
						((iCurY - 1) * HEIGHT) + iLocY))
					{
						snprintf (arText[0], MAX_TEXT, "%s",
							GateAsText (arRaiseGate1[iCurLevel][iRaiseLoop]));
						snprintf (arText[1], MAX_TEXT, "%s",
							GateAsText (arRaiseGate2[iCurLevel][iRaiseLoop]));
						snprintf (arText[2], MAX_TEXT, "%s",
							GateAsText (arRaiseGate3[iCurLevel][iRaiseLoop]));
						DisplayText (iHorL, iVerL + 151 - 41,
							FONT_SIZE_11, arText, 3, font2, color_wh, color_bl);
					}
				}
			}

			/*** drop ***/
			if (arNrDrop[iCurLevel] > 0)
			{
				for (iDropLoop = 1; iDropLoop <= arNrDrop[iCurLevel]; iDropLoop++)
				{
					if ((arDropX[iCurLevel][iDropLoop] ==
						((iCurX - 1) * WIDTH) + iLocX) &&
						(arDropY[iCurLevel][iDropLoop] ==
						((iCurY - 1) * HEIGHT) + iLocY))
					{
						snprintf (arText[0], MAX_TEXT, "%s",
							GateAsText (arDropGate1[iCurLevel][iDropLoop]));
						snprintf (arText[1], MAX_TEXT, "%s",
							GateAsText (arDropGate2[iCurLevel][iDropLoop]));
						snprintf (arText[2], MAX_TEXT, "%s",
							GateAsText (arDropGate3[iCurLevel][iDropLoop]));
						DisplayText (iHorL, iVerL + 151 - 41,
							FONT_SIZE_11, arText, 3, font2, color_wh, color_bl);
					}
				}
			}

			/*** chomper ***/
			if (arNrChompers[iCurLevel] > 0)
			{
				for (iChomperLoop = 1; iChomperLoop <=
					arNrChompers[iCurLevel]; iChomperLoop++)
				{
					if ((arChomperX[iCurLevel][iChomperLoop] ==
						((iCurX - 1) * WIDTH) + iLocX) &&
						(arChomperY[iCurLevel][iChomperLoop] ==
						((iCurY - 1) * HEIGHT) + iLocY))
					{
						if (iDebug == 1)
						{
							snprintf (arText[0], MAX_TEXT, "%s", "chomper");
							DisplayText (iHorL, iVerL + 151 - FONT_SIZE_11,
								FONT_SIZE_11, arText, 1, font2, color_wh, color_bl);
						}
					}
				}
			}

			/*** spike ***/
			if (arNrSpikes[iCurLevel] > 0)
			{
				for (iSpikeLoop = 1; iSpikeLoop <= arNrSpikes[iCurLevel]; iSpikeLoop++)
				{
					if ((arSpikeX[iCurLevel][iSpikeLoop] ==
						((iCurX - 1) * WIDTH) + iLocX) &&
						(arSpikeY[iCurLevel][iSpikeLoop] ==
						((iCurY - 1) * HEIGHT) + iLocY))
					{
						if (iDebug == 1)
						{
							snprintf (arText[0], MAX_TEXT, "%s", "spike");
							DisplayText (iHorL, iVerL + 151 - FONT_SIZE_11,
								FONT_SIZE_11, arText, 1, font2, color_wh, color_bl);
						}
					}
				}
			}

			/*** potion ***/
			if (arNrPotions[iCurLevel] > 0)
			{
				for (iPotionLoop = 1; iPotionLoop <=
					arNrPotions[iCurLevel]; iPotionLoop++)
				{
					if ((arPotionX[iCurLevel][iPotionLoop] ==
						((iCurX - 1) * WIDTH) + iLocX) &&
						(arPotionY[iCurLevel][iPotionLoop] ==
						((iCurY - 1) * HEIGHT) + iLocY))
					{
						snprintf (arText[0], MAX_TEXT, "%s",
							ColorAsText (arPotionColor[iCurLevel][iPotionLoop]));
						snprintf (arText[1], MAX_TEXT, "%s",
							EffectAsText (arPotionEffect[iCurLevel][iPotionLoop]));
						DisplayText (iHorL, iVerL + 125,
							FONT_SIZE_11, arText, 2, font2, color_wh, color_bl);
					}
				}
			}
		}
	}
	if (iScreen == 2) /*** R ***/
	{
		/*** background ***/
		ShowImage (imgrooms, 25, 50, "imgrooms");

		/*** level width ***/
		CenterNumber (arLevelWidth[iCurLevel], 349, 75, color_wh, 0);

		/*** level height ***/
		CenterNumber (arLevelHeight[iCurLevel], 563, 75, color_wh, 0);

		ShowRooms();
	}
	if (iScreen == 3) /*** E ***/
	{
		/*** background ***/
		ShowImage (imgenumerate, 25, 50, "imgenumerate");

		switch (iEXEType)
		{
			case 1: /*** US ***/
				TotalLine ("rooms", ALLOWED_US_ROOMS, 0);
				TotalLine ("guards", ALLOWED_US_GUARDS, 1);
				TotalLine ("doors", ALLOWED_US_DOORS, 2);
				TotalLine ("gates", ALLOWED_US_GATES, 3);
				TotalLine ("loose", ALLOWED_US_LOOSE, 4);
				TotalLine ("raise", ALLOWED_US_RAISE, 5);
				TotalLine ("drop", ALLOWED_US_DROP, 6);
				TotalLine ("chompers", ALLOWED_US_CHOMPERS, 7);
				TotalLine ("spikes", ALLOWED_US_SPIKES, 8);
				TotalLine ("potions", ALLOWED_US_POTIONS, 9);
				break;
			case 2: /*** EU ***/
				TotalLine ("rooms", ALLOWED_EU_ROOMS, 0);
				TotalLine ("guards", ALLOWED_EU_GUARDS, 1);
				TotalLine ("doors", ALLOWED_EU_DOORS, 2);
				TotalLine ("gates", ALLOWED_EU_GATES, 3);
				TotalLine ("loose", ALLOWED_EU_LOOSE, 4);
				TotalLine ("raise", ALLOWED_EU_RAISE, 5);
				TotalLine ("drop", ALLOWED_EU_DROP, 6);
				TotalLine ("chompers", ALLOWED_EU_CHOMPERS, 7);
				TotalLine ("spikes", ALLOWED_EU_SPIKES, 8);
				TotalLine ("potions", ALLOWED_EU_POTIONS, 9);
				break;
		}

		if (IsSavingAllowed() == 1)
		{
			snprintf (arText[0], MAX_TEXT, "%s", "possible");
			clr = color_green;
		} else {
			snprintf (arText[0], MAX_TEXT, "%s", "impossible");
			clr = color_red;
		}
		DisplayText (479, 384, FONT_SIZE_15, arText, 1, font1, color_bl, clr);
	}

	/*** left ***/
	if (iCurX > 1)
	{
		/*** yes ***/
		if (iDownAt == 1)
		{
			ShowImage (imgleft_1, 0, 50, "imgleft_1"); /*** down ***/
		} else {
			ShowImage (imgleft_0, 0, 50, "imgleft_0"); /*** up ***/
		}
	} else {
		/*** no ***/
		ShowImage (imglrno, 0, 50, "imglrno");
	}

	/*** right ***/
	if (iCurX < arLevelWidth[iCurLevel])
	{
		/*** yes ***/
		if (iDownAt == 2)
		{
			ShowImage (imgright_1, 667, 50, "imgright_1"); /*** down ***/
		} else {
			ShowImage (imgright_0, 667, 50, "imgright_0"); /*** up ***/
		}
	} else {
		/*** no ***/
		ShowImage (imglrno, 667, 50, "imglrno");
	}

	/*** up ***/
	if (iCurY > 1)
	{
		/*** yes ***/
		if (iDownAt == 3)
		{
			ShowImage (imgup_1, 25, 25, "imgup_1"); /*** down ***/
		} else {
			ShowImage (imgup_0, 25, 25, "imgup_0"); /*** up ***/
		}
	} else {
		/*** no ***/
		if (iScreen != 1)
		{
			ShowImage (imgudno, 25, 25, "imgudno"); /*** without info ***/
		} else {
			ShowImage (imgudnonfo, 25, 25, "imgudnonfo"); /*** with info ***/
		}
	}

	/*** down ***/
	if (iCurY < arLevelHeight[iCurLevel])
	{
		/*** yes ***/
		if (iDownAt == 4)
		{
			ShowImage (imgdown_1, 25, 442, "imgdown_1"); /*** down ***/
		} else {
			ShowImage (imgdown_0, 25, 442, "imgdown_0"); /*** up ***/
		}
	} else {
		/*** no ***/
		ShowImage (imgudno, 25, 442, "imgudno");
	}

	switch (iScreen)
	{
		case 1:
			/*** rooms on ***/
			if (iDownAt == 5)
			{
				ShowImage (imgroomson_1, 0, 25, "imgroomson_1"); /*** down ***/
			} else {
				ShowImage (imgroomson_0, 0, 25, "imgroomson_0"); /*** up ***/
			}
			/*** enumerate on ***/
			if (iDownAt == 6)
			{
				ShowImage (imgenumon_1, 667, 25, "imgenumon_1"); /*** down ***/
			} else {
				ShowImage (imgenumon_0, 667, 25, "imgenumon_0"); /*** up ***/
			}
			break;
		case 2:
			/*** rooms off ***/
			ShowImage (imgroomsoff, 0, 25, "imgroomsoff");

			/*** enumerate on ***/
			if (iDownAt == 6)
			{
				ShowImage (imgenumon_1, 667, 25, "imgenumon_1"); /*** down ***/
			} else {
				ShowImage (imgenumon_0, 667, 25, "imgenumon_0"); /*** up ***/
			}
			break;
		case 3:
			/*** rooms on ***/
			if (iDownAt == 5)
			{
				ShowImage (imgroomson_1, 0, 25, "imgroomson_1"); /*** down ***/
			} else {
				ShowImage (imgroomson_0, 0, 25, "imgroomson_0"); /*** up ***/
			}
			/*** enumerate off ***/
			ShowImage (imgenumoff, 667, 25, "imgenumoff");
			break;
	}

	/*** save ***/
	if (iChanged != 0)
	{
		/*** on ***/
		if (iDownAt == 7)
		{
			ShowImage (imgsaveon_1, 0, 442, "imgsaveon_1"); /*** down ***/
		} else {
			ShowImage (imgsaveon_0, 0, 442, "imgsaveon_0"); /*** up ***/
		}
	} else {
		/*** off ***/
		ShowImage (imgsaveoff, 0, 442, "imgsaveoff");
	}

	/*** quit ***/
	if (iDownAt == 8)
	{
		ShowImage (imgquit_1, 667, 442, "imgquit_1"); /*** down ***/
	} else {
		ShowImage (imgquit_0, 667, 442, "imgquit_0"); /*** up ***/
	}

	/*** previous ***/
	if (iCurLevel != 1)
	{
		/*** on ***/
		if (iDownAt == 9)
		{
			ShowImage (imgprevon_1, 0, 0, "imgprevon_1"); /*** down ***/
		} else {
			ShowImage (imgprevon_0, 0, 0, "imgprevon_0"); /*** up ***/
		}
	} else {
		/*** off ***/
		ShowImage (imgprevoff, 0, 0, "imgprevoff");
	}

	/*** next ***/
	if (iCurLevel != iNrLevels)
	{
		/*** on ***/
		if (iDownAt == 10)
		{
			ShowImage (imgnexton_1, 667, 0, "imgnexton_1"); /*** down ***/
		} else {
			ShowImage (imgnexton_0, 667, 0, "imgnexton_0"); /*** up ***/
		}
	} else {
		/*** off ***/
		ShowImage (imgnextoff, 667, 0, "imgnextoff");
	}

	/*** level bar ***/
	ShowImage (imgbar, 25, 0, "imgbar");

	/*** Assemble level bar text. ***/
	if (iEXEType == 1) /*** US ***/
	{
		switch (iCurLevel)
		{
			case 1: snprintf (sLevelBar, MAX_TEXT, "level 1 (prison),"); break;
			case 2: snprintf (sLevelBar, MAX_TEXT, "level 2 (guards),"); break;
			case 3: snprintf (sLevelBar, MAX_TEXT, "level 3 (skeleton),"); break;
			case 4: snprintf (sLevelBar, MAX_TEXT, "level 4 (mirror),"); break;
			case 5: snprintf (sLevelBar, MAX_TEXT, "level 5 (thief),"); break;
			case 6: snprintf (sLevelBar, MAX_TEXT, "level 6 (plunge),"); break;
			case 7: snprintf (sLevelBar, MAX_TEXT, "level 7 (weightless),"); break;
			case 8: snprintf (sLevelBar, MAX_TEXT, "level 8 (mouse),"); break;
			case 9: snprintf (sLevelBar, MAX_TEXT, "level 9 (twisty),"); break;
			case 10: snprintf (sLevelBar, MAX_TEXT, "level 10 (quad),"); break;
			case 11: snprintf (sLevelBar, MAX_TEXT, "level 11 (fragile),"); break;
			case 12: snprintf (sLevelBar, MAX_TEXT, "level 12 (twr+jaf),"); break;
			case 13: snprintf (sLevelBar, MAX_TEXT, "level 13 (rescue),"); break;
		}
	} else if (iEXEType == 2) { /*** EU ***/
		switch (iCurLevel)
		{
			case 1: snprintf (sLevelBar, MAX_TEXT, "level 1 (prison),"); break;
			case 2: snprintf (sLevelBar, MAX_TEXT, "level 2 (guards),"); break;
			case 3: snprintf (sLevelBar, MAX_TEXT, "level 3 (skeleton),"); break;
			case 4: snprintf (sLevelBar, MAX_TEXT, "level 4 (mirror),"); break;
			case 5: snprintf (sLevelBar, MAX_TEXT, "level 5 (thief),"); break;
			case 6: snprintf (sLevelBar, MAX_TEXT, "level 6 (back),"); break;
			case 7: snprintf (sLevelBar, MAX_TEXT, "level 7 (freeze),"); break;
			case 8: snprintf (sLevelBar, MAX_TEXT, "level 8 (plunge),"); break;
			case 9: snprintf (sLevelBar, MAX_TEXT, "level 9 (weightless),"); break;
			case 10: snprintf (sLevelBar, MAX_TEXT, "level 10 (mouse),"); break;
			case 11: snprintf (sLevelBar, MAX_TEXT, "level 11 (twisty),"); break;
			case 12: snprintf (sLevelBar, MAX_TEXT, "level 12 (rumble),"); break;
			case 13: snprintf (sLevelBar, MAX_TEXT, "level 13 (quad),"); break;
			case 14: snprintf (sLevelBar, MAX_TEXT, "level 14 (fragile),"); break;
			case 15: snprintf (sLevelBar, MAX_TEXT, "level 15 (pits),"); break;
			case 16: snprintf (sLevelBar, MAX_TEXT, "level 16 (twr+jaf),"); break;
			case 17: snprintf (sLevelBar, MAX_TEXT, "level 17 (rescue),"); break;
		}
	} else {
		printf ("[FAILED] Unknown EXE type: %i!\n", iEXEType);
		exit (EXIT_ERROR);
	}
	switch (iScreen)
	{
		case 1:
			snprintf (sLevelBarF, MAX_TEXT, "%s room x:%i y:%i",
				sLevelBar, iCurX, iCurY);
			ShowImage (imgextras[iExtras], 610, 3, "imgextras[...]");
			break;
		case 2:
			snprintf (sLevelBarF, MAX_TEXT, "%s rooms", sLevelBar); break;
		case 3:
			snprintf (sLevelBarF, MAX_TEXT, "%s enumerate", sLevelBar); break;
	}

	/*** Mednafen information. ***/
	if (iMednafen == 1) { ShowImage (imgmednafen, 25, 50, "imgmednafen"); }

	/*** Display level bar text. ***/
	message = TTF_RenderText_Shaded (font1, sLevelBarF, color_bl, color_wh);
	messaget = SDL_CreateTextureFromSurface (ascreen, message);
	offset.x = 31;
	offset.y = 5;
	offset.w = message->w; offset.h = message->h;
	CustomRenderCopy (messaget, NULL, &offset, "message");
	SDL_DestroyTexture (messaget); SDL_FreeSurface (message);

	/*** refresh screen ***/
	SDL_RenderPresent (ascreen);
}
/*****************************************************************************/
void InitPopUp (void)
/*****************************************************************************/
{
	int iPopUp;
	SDL_Event event;

	iPopUp = 1;

	PlaySound ("wav/popup.wav");
	ShowPopUp();
	while (iPopUp == 1)
	{
		while (SDL_PollEvent (&event))
		{
			switch (event.type)
			{
				case SDL_CONTROLLERBUTTONDOWN:
					/*** Nothing for now. ***/
					break;
				case SDL_CONTROLLERBUTTONUP:
					switch (event.cbutton.button)
					{
						case SDL_CONTROLLER_BUTTON_A:
							iPopUp = 0; break;
					}
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
						case SDLK_KP_ENTER:
						case SDLK_RETURN:
						case SDLK_SPACE:
						case SDLK_o:
							iPopUp = 0;
						default: break;
					}
					break;
				case SDL_MOUSEMOTION:
					iXPos = event.motion.x;
					iYPos = event.motion.y;
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == 1)
					{
						if (InArea (440, 323, 440 + 85, 323 + 32) == 1) /*** OK ***/
						{
							iOKOn = 1;
							ShowPopUp();
						}
					}
					break;
				case SDL_MOUSEBUTTONUP:
					iOKOn = 0;
					if (event.button.button == 1)
					{
						if (InArea (440, 323, 440 + 85, 323 + 32) == 1) /*** OK ***/
						{
							iPopUp = 0;
						}
					}
					ShowPopUp(); break;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_EXPOSED)
						{ ShowScreen(); ShowPopUp(); } break;
				case SDL_QUIT:
					Quit(); break;
			}
		}

		/*** prevent CPU eating ***/
		gamespeed = REFRESH;
		while ((SDL_GetTicks() - looptime) < gamespeed)
		{
			SDL_Delay (10);
		}
		looptime = SDL_GetTicks();
	}
	PlaySound ("wav/popup_close.wav");
	ShowScreen();
}
/*****************************************************************************/
void ShowPopUp (void)
/*****************************************************************************/
{
	char arText[9 + 2][MAX_TEXT + 2];

	/*** faded background ***/
	ShowImage (imgfadedl, 0, 0, "imgfadedl");

	/*** popup ***/
	ShowImage (imgpopup, 83, 11, "imgpopup");

	/*** OK ***/
	switch (iOKOn)
	{
		case 0: ShowImage (imgok[1], 440, 323, "imgok[1]"); break; /*** off ***/
		case 1: ShowImage (imgok[2], 440, 323, "imgok[2]"); break; /*** on ***/
	}

	snprintf (arText[0], MAX_TEXT, "%s %s", EDITOR_NAME, EDITOR_VERSION);
	snprintf (arText[1], MAX_TEXT, "%s", COPYRIGHT);
	snprintf (arText[2], MAX_TEXT, "%s", "");
	if (iController != 1)
	{
		snprintf (arText[3], MAX_TEXT, "%s", "single tile (change or select)");
		snprintf (arText[4], MAX_TEXT, "%s", "entire room (clear or fill)");
		snprintf (arText[5], MAX_TEXT, "%s", "entire level (randomize or fill)");
	} else {
		snprintf (arText[3], MAX_TEXT, "%s", "The detected game controller:");
		snprintf (arText[4], MAX_TEXT, "%s", sControllerName);
		snprintf (arText[5], MAX_TEXT, "%s", "Have fun using it!");
	}
	snprintf (arText[6], MAX_TEXT, "%s", "");
	snprintf (arText[7], MAX_TEXT, "%s", "You may use one guard per room.");
	snprintf (arText[8], MAX_TEXT, "%s", "The tile behavior may differ per"
		" level.");

	DisplayText (180, 124, FONT_SIZE_15, arText, 9, font1, color_wh, color_bl);

	/*** refresh screen ***/
	SDL_RenderPresent (ascreen);
}
/*****************************************************************************/
void Help (void)
/*****************************************************************************/
{
	int iHelp;
	SDL_Event event;

	iHelp = 1;

	PlaySound ("wav/popup.wav");
	ShowHelp();
	while (iHelp == 1)
	{
		while (SDL_PollEvent (&event))
		{
			switch (event.type)
			{
				case SDL_CONTROLLERBUTTONDOWN:
					/*** Nothing for now. ***/
					break;
				case SDL_CONTROLLERBUTTONUP:
					switch (event.cbutton.button)
					{
						case SDL_CONTROLLER_BUTTON_A:
							iHelp = 0; break;
					}
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
						case SDLK_KP_ENTER:
						case SDLK_RETURN:
						case SDLK_SPACE:
						case SDLK_o:
							iHelp = 0;
						default: break;
					}
					break;
				case SDL_MOUSEMOTION:
					iXPos = event.motion.x;
					iYPos = event.motion.y;
					if (InArea (80, 349, 80 + 516, 349 + 19) == 1)
					{
						SDL_SetCursor (curHand);
					} else {
						SDL_SetCursor (curArrow);
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == 1)
					{
						if (InArea (590, 417, 590 + 85, 417 + 32) == 1) /*** OK ***/
						{
							iHelpOK = 1;
							ShowHelp();
						}
					}
					break;
				case SDL_MOUSEBUTTONUP:
					iHelpOK = 0;
					if (event.button.button == 1) /*** left mouse button ***/
					{
						if (InArea (590, 417, 590 + 85, 417 + 32) == 1) /*** OK ***/
							{ iHelp = 0; }
						if (InArea (80, 349, 80 + 516, 349 + 19) == 1)
							{ OpenURL ("https://github.com/EndeavourAccuracy/lemdop"); }
					}
					ShowHelp(); break;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_EXPOSED)
						{ ShowHelp(); } break;
				case SDL_QUIT:
					Quit(); break;
			}
		}

		/*** prevent CPU eating ***/
		gamespeed = REFRESH;
		while ((SDL_GetTicks() - looptime) < gamespeed)
		{
			SDL_Delay (10);
		}
		looptime = SDL_GetTicks();
	}
	PlaySound ("wav/popup_close.wav");
	ShowScreen();
}
/*****************************************************************************/
void ShowHelp (void)
/*****************************************************************************/
{
	/*** background ***/
	ShowImage (imghelp, 0, 0, "imghelp");

	/*** OK ***/
	switch (iHelpOK)
	{
		case 0: ShowImage (imgok[1], 590, 417, "imgok[1]"); break; /*** off ***/
		case 1: ShowImage (imgok[2], 590, 417, "imgok[2]"); break; /*** on ***/
	}

	/*** refresh screen ***/
	SDL_RenderPresent (ascreen);
}
/*****************************************************************************/
void EXE (void)
/*****************************************************************************/
{
	int iEXE;
	SDL_Event event;

	iEXE = 1;

	EXELoad();

	PlaySound ("wav/popup.wav");
	ShowEXE();
	while (iEXE == 1)
	{
		while (SDL_PollEvent (&event))
		{
			switch (event.type)
			{
				case SDL_CONTROLLERBUTTONDOWN:
					/*** Nothing for now. ***/
					break;
				case SDL_CONTROLLERBUTTONUP:
					switch (event.cbutton.button)
					{
						case SDL_CONTROLLER_BUTTON_A:
							EXESave(); iEXE = 0; break;
					}
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							iEXE = 0; break;
						case SDLK_KP_ENTER:
						case SDLK_RETURN:
						case SDLK_SPACE:
						case SDLK_s:
							EXESave(); iEXE = 0;
							break;
						default: break;
					}
					ShowEXE();
					break;
				case SDL_MOUSEMOTION:
					iXPos = event.motion.x;
					iYPos = event.motion.y;
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == 1)
					{
						if (InArea (590, 417, 590 + 85, 417 + 32) == 1) /*** Save ***/
						{
							iEXESave = 1;
							ShowEXE();
						}
					}
					break;
				case SDL_MOUSEBUTTONUP:
					iEXESave = 0;
					if (event.button.button == 1) /*** left mouse button ***/
					{
						if (InArea (590, 417, 590 + 85, 417 + 32) == 1) /*** Save ***/
						{
							EXESave(); iEXE = 0;
						}

						/*** Starting minutes. ***/
						PlusMinus (&iEXEStartingMin, 219, 34, 0, 68, -10, 0);
						PlusMinus (&iEXEStartingMin, 234, 34, 0, 68, -1, 0);
						PlusMinus (&iEXEStartingMin, 304, 34, 0, 68, 1, 0);
						PlusMinus (&iEXEStartingMin, 319, 34, 0, 68, 10, 0);

						/*** Starting seconds. ***/
						PlusMinus (&iEXEStartingSec, 219, 58, 0, 59, -10, 0);
						PlusMinus (&iEXEStartingSec, 234, 58, 0, 59, -1, 0);
						PlusMinus (&iEXEStartingSec, 304, 58, 0, 59, 1, 0);
						PlusMinus (&iEXEStartingSec, 319, 58, 0, 59, 10, 0);

						/*** Starting HP. ***/
						PlusMinus (&iEXEStartingHP, 219, 82, 1, 9, -10, 0);
						PlusMinus (&iEXEStartingHP, 234, 82, 1, 9, -1, 0);
						PlusMinus (&iEXEStartingHP, 304, 82, 1, 9, 1, 0);
						PlusMinus (&iEXEStartingHP, 319, 82, 1, 9, 10, 0);

						/*** Starting level. ***/
						PlusMinus (&iEXEStartingLevel, 219, 106, 1, iNrLevels, -10, 0);
						PlusMinus (&iEXEStartingLevel, 234, 106, 1, iNrLevels, -1, 0);
						PlusMinus (&iEXEStartingLevel, 304, 106, 1, iNrLevels, 1, 0);
						PlusMinus (&iEXEStartingLevel, 319, 106, 1, iNrLevels, 10, 0);
					}
					ShowEXE();
					break;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_EXPOSED)
						{ ShowEXE(); } break;
				case SDL_QUIT:
					Quit(); break;
			}
		}

		/*** prevent CPU eating ***/
		gamespeed = REFRESH;
		while ((SDL_GetTicks() - looptime) < gamespeed)
		{
			SDL_Delay (10);
		}
		looptime = SDL_GetTicks();
	}
	PlaySound ("wav/popup_close.wav");
	ShowScreen();
}
/*****************************************************************************/
void ShowEXE (void)
/*****************************************************************************/
{
	SDL_Color clr;

	/*** background ***/
	ShowImage (imgexe, 0, 0, "imgexe");

	/*** save button ***/
	switch (iEXESave)
	{
		case 0: /*** off ***/
			ShowImage (imgsave[1], 590, 417, "imgsave[1]"); break;
		case 1: /*** on ***/
			ShowImage (imgsave[2], 590, 417, "imgsave[2]"); break;
	}

	/*** Starting minutes. ***/
	if (iEXEStartingMin == 60) { clr = color_bl; } else { clr = color_blue; }
	CenterNumber (iEXEStartingMin, 247, 34, clr, 0);

	/*** Starting seconds. ***/
	if (iEXEStartingSec == 0) { clr = color_bl; } else { clr = color_blue; }
	CenterNumber (iEXEStartingSec, 247, 58, clr, 0);

	/*** Starting HP. ***/
	if (iEXEStartingHP == 3) { clr = color_bl; } else { clr = color_blue; }
	CenterNumber (iEXEStartingHP, 247, 82, clr, 0);

	/*** Starting level. ***/
	if (iEXEStartingLevel == 1) { clr = color_bl; } else { clr = color_blue; }
	CenterNumber (iEXEStartingLevel, 247, 106, clr, 0);

	/*** refresh screen ***/
	SDL_RenderPresent (ascreen);
}
/*****************************************************************************/
void InitScreenAction (char *sAction)
/*****************************************************************************/
{
	int iOldWidth;
	int iOldHeight;

	if (strcmp (sAction, "left") == 0)
	{
		switch (iScreen)
		{
			case 1:
				iSelectedX--;
				if (iSelectedX == 0) { iSelectedX = 10; }
				break;
			case 2:
				if (iController == 1)
				{
					iOldWidth = arLevelWidth[iCurLevel];
					iOldHeight = arLevelHeight[iCurLevel];
					ChangeCustom (-1, &arLevelWidth[iCurLevel], 1, 24);
					LevelResized (iOldWidth, iOldHeight);
				}
				break;
		}
	}

	if (strcmp (sAction, "right") == 0)
	{
		switch (iScreen)
		{
			case 1:
				iSelectedX++;
				if (iSelectedX == 11) { iSelectedX = 1; }
				break;
			case 2:
				if (iController == 1)
				{
					iOldWidth = arLevelWidth[iCurLevel];
					iOldHeight = arLevelHeight[iCurLevel];
					ChangeCustom (1, &arLevelWidth[iCurLevel], 1, 24);
					LevelResized (iOldWidth, iOldHeight);
				}
				break;
		}
	}

	if (strcmp (sAction, "up") == 0)
	{
		switch (iScreen)
		{
			case 1:
				iSelectedY--;
				if (iSelectedY == 0) { iSelectedY = 3; }
				break;
			case 2:
				if (iController == 1)
				{
					iOldWidth = arLevelWidth[iCurLevel];
					iOldHeight = arLevelHeight[iCurLevel];
					ChangeCustom (10, &arLevelWidth[iCurLevel], 1, 24);
					LevelResized (iOldWidth, iOldHeight);
				}
				break;
		}
	}

	if (strcmp (sAction, "down") == 0)
	{
		switch (iScreen)
		{
			case 1:
				iSelectedY++;
				if (iSelectedY == 4) { iSelectedY = 1; }
				break;
			case 2:
				if (iController == 1)
				{
					iOldWidth = arLevelWidth[iCurLevel];
					iOldHeight = arLevelHeight[iCurLevel];
					ChangeCustom (-10, &arLevelWidth[iCurLevel], 1, 24);
					LevelResized (iOldWidth, iOldHeight);
				}
				break;
		}
	}

	if (strcmp (sAction, "enter") == 0)
	{
		if (iScreen == 1) { ChangePos(); }
	}

	if (strcmp (sAction, "env") == 0)
	{
		switch (arLevelType[iCurLevel])
		{
			case 0: arLevelType[iCurLevel] = 1; break;
			case 1: arLevelType[iCurLevel] = 0; break;
		}
		PlaySound ("wav/extras.wav");
		iChanged++;
	}

	if (strcmp (sAction, "lbracket ctrl") == 0)
	{
		if (iScreen == 2)
		{
			iOldWidth = arLevelWidth[iCurLevel];
			iOldHeight = arLevelHeight[iCurLevel];
			ChangeCustom (-10, &arLevelHeight[iCurLevel], 1, 24);
			LevelResized (iOldWidth, iOldHeight);
		}
	}

	if (strcmp (sAction, "lbracket shift") == 0)
	{
		if (iScreen == 2)
		{
			iOldWidth = arLevelWidth[iCurLevel];
			iOldHeight = arLevelHeight[iCurLevel];
			ChangeCustom (-1, &arLevelHeight[iCurLevel], 1, 24);
			LevelResized (iOldWidth, iOldHeight);
		}
	}

	if (strcmp (sAction, "rbracket shift") == 0)
	{
		if (iScreen == 2)
		{
			iOldWidth = arLevelWidth[iCurLevel];
			iOldHeight = arLevelHeight[iCurLevel];
			ChangeCustom (1, &arLevelHeight[iCurLevel], 1, 24);
			LevelResized (iOldWidth, iOldHeight);
		}
	}

	if (strcmp (sAction, "rbracket ctrl") == 0)
	{
		if (iScreen == 2)
		{
			iOldWidth = arLevelWidth[iCurLevel];
			iOldHeight = arLevelHeight[iCurLevel];
			ChangeCustom (10, &arLevelHeight[iCurLevel], 1, 24);
			LevelResized (iOldWidth, iOldHeight);
		}
	}

	if (strcmp (sAction, "comma ctrl") == 0)
	{
		if (iScreen == 2)
		{
			iOldWidth = arLevelWidth[iCurLevel];
			iOldHeight = arLevelHeight[iCurLevel];
			ChangeCustom (-10, &arLevelWidth[iCurLevel], 1, 24);
			LevelResized (iOldWidth, iOldHeight);
		}
	}

	if (strcmp (sAction, "comma shift") == 0)
	{
		if (iScreen == 2)
		{
			iOldWidth = arLevelWidth[iCurLevel];
			iOldHeight = arLevelHeight[iCurLevel];
			ChangeCustom (-1, &arLevelWidth[iCurLevel], 1, 24);
			LevelResized (iOldWidth, iOldHeight);
		}
	}

	if (strcmp (sAction, "period shift") == 0)
	{
		if (iScreen == 2)
		{
			iOldWidth = arLevelWidth[iCurLevel];
			iOldHeight = arLevelHeight[iCurLevel];
			ChangeCustom (1, &arLevelWidth[iCurLevel], 1, 24);
			LevelResized (iOldWidth, iOldHeight);
		}
	}

	if (strcmp (sAction, "period ctrl") == 0)
	{
		if (iScreen == 2)
		{
			iOldWidth = arLevelWidth[iCurLevel];
			iOldHeight = arLevelHeight[iCurLevel];
			ChangeCustom (10, &arLevelWidth[iCurLevel], 1, 24);
			LevelResized (iOldWidth, iOldHeight);
		}
	}
}
/*****************************************************************************/
void RunLevel (int iLevel)
/*****************************************************************************/
{
	SDL_Thread *princethread;

	if (iDebug == 1)
	{
		printf ("[  OK  ] Starting the game in level %i.\n", iLevel);
	}

	ModifyStart (iLevel, 1);

	princethread = SDL_CreateThread (StartGame, "StartGame", NULL);
	if (princethread == NULL)
	{
		printf ("[FAILED] Could not create thread!\n");
		exit (EXIT_ERROR);
	}
}
/*****************************************************************************/
int StartGame (void *unused)
/*****************************************************************************/
{
	char sSystem[200 + 2];
	char sSound[200 + 2];

	if (unused != NULL) { } /*** To prevent warnings. ***/

	PlaySound ("wav/mednafen.wav");

	switch (iNoAudio)
	{
#if defined WIN32 || _WIN32 || WIN64 || _WIN64
		case 0: snprintf (sSound, 200, "%s", " -sound 1"); break;
#else
		case 0: snprintf (sSound, 200, "%s", " -sound 1 -sounddriver sdl"); break;
#endif
		case 1: snprintf (sSound, 200, "%s", " -sound 0"); break;
	}

	snprintf (sSystem, 200, "mednafen -md.videoip 0"
		" -md.stretch aspect -md.xscale 2 -md.yscale 2 %s %s > %s",
		sSound, sPathFile, DEVNULL);
	if (system (sSystem) == -1)
	{
		printf ("[ WARN ] Could not execute mednafen!\n");
	}

	if (iModified != 0) { ModifyStart (iCurLevel, 2); }

	return (EXIT_NORMAL);
}
/*****************************************************************************/
void ClearRoom (int iRoomX, int iRoomY)
/*****************************************************************************/
{
	int iX, iY;
	int iHasGuard;

	/*** Used for looping. ***/
	int iXLoop, iYLoop;

	/*** Remove tiles and guard(s). ***/
	for (iXLoop = 1; iXLoop <= WIDTH; iXLoop++)
	{
		for (iYLoop = 1; iYLoop <= HEIGHT; iYLoop++)
		{
			iX = iXLoop + ((iRoomX - 1) * WIDTH);
			iY = iYLoop + ((iRoomY - 1) * HEIGHT);
			SetLocation (iX, iY, 0x00, 0x00);

			/*** guard ***/
			iHasGuard = HasObject (iX, iY, arGuardX[iCurLevel],
				arGuardY[iCurLevel], arNrGuards[iCurLevel]);
			if (iHasGuard == 1)
			{
				AddRemoveAttributes (0x0C, iX, iY, arNrGuards,
					arGuardX[iCurLevel], arGuardY[iCurLevel], 0);
			}
		}
	}

	/*** No PlaySound() or iChanged++ here. ***/
}
/*****************************************************************************/
void UseTile (int iTile, int iX, int iY)
/*****************************************************************************/
{
	int iS3;
	int iNewDir, iNewType;
	int iOldDir, iOldType;
	int iGuard, iGuardAdd;

	iGuard = 0;
	iGuardAdd = 0;
	switch (iTile)
	{
		case -2: /*** custom ***/
			SetLocation (iX, iY, iNewObject, iNewGraphics);
			break;
		/*** For this port, RandomizeLevel() is used instead of iTile -1. ***/
		case 31: iNewDir = 0x800; iNewType = 0; iGuard = 1; break; /*** guard ***/
		case 32: iNewDir = 0x00; iNewType = 0; iGuard = 1; break; /*** guard ***/
		case 33: iNewDir = 0x800; iNewType = 1; iGuard = 1; break; /*** skel ***/
		case 34: iNewDir = 0x00; iNewType = 1; iGuard = 1; break; /*** skel ***/
		case 35: iNewDir = 0x800; iNewType = 2; iGuard = 1; break; /*** fat ***/
		case 36: iNewDir = 0x00; iNewType = 2; iGuard = 1; break; /*** fat ***/
		case 37: iNewDir = 0x800; iNewType = 3; iGuard = 1; break; /*** shadow ***/
		case 38: iNewDir = 0x00; iNewType = 3; iGuard = 1; break; /*** shadow ***/
		case 39: iNewDir = 0x800; iNewType = 4; iGuard = 1; break; /*** Jaffar ***/
		case 40: iNewDir = 0x00; iNewType = 4; iGuard = 1; break; /*** Jaffar ***/
		case 41: /*** prince, turned right ***/
			if ((arPrinceDir[iCurLevel] != 0x800) ||
				(arPrinceX[iCurLevel] != ((iCurX - 1) * WIDTH) + iSelectedX) ||
				(arPrinceY[iCurLevel] != ((iCurY - 1) * HEIGHT) + iSelectedY) ||
				(arLevelStartingX[iCurLevel] != iCurX) ||
				(arLevelStartingY[iCurLevel] != iCurY))
			{
				arPrinceDir[iCurLevel] = 0x800;
				arPrinceX[iCurLevel] = ((iCurX - 1) * WIDTH) + iSelectedX;
				arPrinceY[iCurLevel] = ((iCurY - 1) * HEIGHT) + iSelectedY;
				arLevelStartingX[iCurLevel] = iCurX;
				arLevelStartingY[iCurLevel] = iCurY;
				PlaySound ("wav/hum_adj.wav");
			}
			break;
		case 42: /*** prince, turned left ***/
			if ((arPrinceDir[iCurLevel] != 0x00) ||
				(arPrinceX[iCurLevel] != ((iCurX - 1) * WIDTH) + iSelectedX) ||
				(arPrinceY[iCurLevel] != ((iCurY - 1) * HEIGHT) + iSelectedY) ||
				(arLevelStartingX[iCurLevel] != iCurX) ||
				(arLevelStartingY[iCurLevel] != iCurY))
			{
				arPrinceDir[iCurLevel] = 0x00;
				arPrinceX[iCurLevel] = ((iCurX - 1) * WIDTH) + iSelectedX;
				arPrinceY[iCurLevel] = ((iCurY - 1) * HEIGHT) + iSelectedY;
				arLevelStartingX[iCurLevel] = iCurX;
				arLevelStartingY[iCurLevel] = iCurY;
				PlaySound ("wav/hum_adj.wav");
			}
			break;
		default: /*** 1-30 ***/
			/*** Set the object and graphics. ***/
			SetLocation (iX, iY, iNewObject, OnGraphics());
			break;
	}

	/*** Set the object attributes. ***/
	if ((iTile == -2) || ((iTile >= 1) && (iTile <= 30)))
	{
		switch (iNewObject)
		{
			case 0x03: /*** raise ***/
				SetAttribute (iX, iY, arRaiseX[iCurLevel], arRaiseY[iCurLevel],
					arRaiseGate1[iCurLevel], arNrRaise[iCurLevel], iRaiseGate1);
				SetAttribute (iX, iY, arRaiseX[iCurLevel], arRaiseY[iCurLevel],
					arRaiseGate2[iCurLevel], arNrRaise[iCurLevel], iRaiseGate2);
				SetAttribute (iX, iY, arRaiseX[iCurLevel], arRaiseY[iCurLevel],
					arRaiseGate3[iCurLevel], arNrRaise[iCurLevel], iRaiseGate3);
				break;
			case 0x04: /*** drop ***/
				SetAttribute (iX, iY, arDropX[iCurLevel], arDropY[iCurLevel],
					arDropGate1[iCurLevel], arNrDrop[iCurLevel], iDropGate1);
				SetAttribute (iX, iY, arDropX[iCurLevel], arDropY[iCurLevel],
					arDropGate2[iCurLevel], arNrDrop[iCurLevel], iDropGate2);
				SetAttribute (iX, iY, arDropX[iCurLevel], arDropY[iCurLevel],
					arDropGate3[iCurLevel], arNrDrop[iCurLevel], iDropGate3);
				break;
			case 0x05: /*** gate ***/
				iS3 = iGateDelay * 257; /*** Because 255 * 257 = 0xFFFF. ***/
				SetAttribute (iX, iY, arGateX[iCurLevel], arGateY[iCurLevel],
					arGateState1[iCurLevel], arNrGates[iCurLevel], iGateState);
				SetAttribute (iX, iY, arGateX[iCurLevel], arGateY[iCurLevel],
					arGateState2[iCurLevel], arNrGates[iCurLevel], iGateOpenness);
				SetAttribute (iX, iY, arGateX[iCurLevel], arGateY[iCurLevel],
					arGateState3[iCurLevel], arNrGates[iCurLevel], iS3);
				break;
			case 0x09: /*** potion ***/
				SetAttribute (iX, iY, arPotionX[iCurLevel], arPotionY[iCurLevel],
					arPotionColor[iCurLevel], arNrPotions[iCurLevel], iPotionColor);
				SetAttribute (iX, iY, arPotionX[iCurLevel], arPotionY[iCurLevel],
					arPotionEffect[iCurLevel], arNrPotions[iCurLevel], iPotionEffect);
				break;
			case 0x0A: /*** door ***/
				SetAttribute (iX, iY, arDoorX[iCurLevel], arDoorY[iCurLevel],
					arDoorType[iCurLevel], arNrDoors[iCurLevel], iDoorType);
				break;
		}
	}

	if (iGuard == 1)
	{
		if (HasObject (iX, iY, arGuardX[iCurLevel], arGuardY[iCurLevel],
			arNrGuards[iCurLevel]) == 1)
		{
			iOldDir = GetAttribute (iX, iY, arGuardX[iCurLevel], arGuardY[iCurLevel],
				arGuardDir[iCurLevel], arNrGuards[iCurLevel]);
			iOldType = GetAttribute (iX, iY, arGuardX[iCurLevel], arGuardY[iCurLevel],
				arGuardType[iCurLevel], arNrGuards[iCurLevel]);
			if ((iOldDir == iNewDir) && (iOldType == iNewType))
			{
				AddRemoveAttributes (0x0C, iX, iY, arNrGuards,
					arGuardX[iCurLevel], arGuardY[iCurLevel], 0);
			} else {
				AddRemoveAttributes (0x0C, iX, iY, arNrGuards,
					arGuardX[iCurLevel], arGuardY[iCurLevel], 0);
				iGuardAdd = 1;
			}
		} else {
			iGuardAdd = 1;
		}
	}

	/*** Add a guard and set his attributes. ***/
	if (iGuardAdd == 1)
	{
		AddRemoveAttributes (0x0C, iX, iY, arNrGuards,
			arGuardX[iCurLevel], arGuardY[iCurLevel], 1);
		SetAttribute (iX, iY, arGuardX[iCurLevel], arGuardY[iCurLevel],
			arGuardDir[iCurLevel], arNrGuards[iCurLevel], iNewDir);
		SetAttribute (iX, iY, arGuardX[iCurLevel], arGuardY[iCurLevel],
			arGuardSprite[iCurLevel], arNrGuards[iCurLevel], GuardSprite (iNewType));
		SetAttribute (iX, iY, arGuardX[iCurLevel], arGuardY[iCurLevel],
			arGuardType[iCurLevel], arNrGuards[iCurLevel], iNewType);
		SetAttribute (iX, iY, arGuardX[iCurLevel], arGuardY[iCurLevel],
			arGuardSkill[iCurLevel], arNrGuards[iCurLevel], iNewSkill);
		SetAttribute (iX, iY, arGuardX[iCurLevel], arGuardY[iCurLevel],
			arGuardHP[iCurLevel], arNrGuards[iCurLevel], iNewHP);
		PlaySound ("wav/hum_adj.wav");
	}
}
/*****************************************************************************/
void Zoom (int iToggleFull)
/*****************************************************************************/
{
	if (iToggleFull == 1)
	{
		if (iFullscreen == 0)
		{ iFullscreen = SDL_WINDOW_FULLSCREEN_DESKTOP; }
			else { iFullscreen = 0; }
	} else {
		if (iFullscreen == SDL_WINDOW_FULLSCREEN_DESKTOP)
		{
			iFullscreen = 0;
			iScale = 1;
		} else if (iScale == 1) {
			iScale = 2;
		} else if (iScale == 2) {
			iFullscreen = SDL_WINDOW_FULLSCREEN_DESKTOP;
		} else {
			printf ("[ WARN ] Unknown window state!\n");
		}
	}

	SDL_SetWindowFullscreen (window, iFullscreen);
	SDL_SetWindowSize (window, (WINDOW_WIDTH) * iScale,
		(WINDOW_HEIGHT) * iScale);
	SDL_RenderSetLogicalSize (ascreen, (WINDOW_WIDTH) * iScale,
		(WINDOW_HEIGHT) * iScale);
	SDL_SetWindowPosition (window, SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED);
	TTF_CloseFont (font1);
	TTF_CloseFont (font2);
	TTF_CloseFont (font3);
	LoadFonts();
}
/*****************************************************************************/
void ChangeCustom (int iAmount, int *iWhat, int iMin, int iMax)
/*****************************************************************************/
{
	if (((iAmount > 0) && (*iWhat < iMax)) ||
		((iAmount < 0) && (*iWhat > iMin)))
	{
		*iWhat+=iAmount;
		if (*iWhat < iMin) { *iWhat = iMin; }
		if (*iWhat > iMax) { *iWhat = iMax; }

		PlaySound ("wav/plus_minus.wav");
	}
}
/*****************************************************************************/
void Prev (int iDiscard)
/*****************************************************************************/
{
	if (iCurLevel != 1)
	{
		iCurLevel--;
		if (iDiscard == 1)
		{
			LoadLevels(); iChanged = 0;
		}
		iCurX = arLevelStartingX[iCurLevel];
		iCurY = arLevelStartingY[iCurLevel];
		PlaySound ("wav/level_change.wav");
	}
}
/*****************************************************************************/
void Next (int iDiscard)
/*****************************************************************************/
{
	if (iCurLevel != iNrLevels)
	{
		iCurLevel++;
		if (iDiscard == 1)
		{
			LoadLevels(); iChanged = 0;
		}
		iCurX = arLevelStartingX[iCurLevel];
		iCurY = arLevelStartingY[iCurLevel];
		PlaySound ("wav/level_change.wav");
	}
}
/*****************************************************************************/
void CallSave (void)
/*****************************************************************************/
{
	if (IsSavingAllowed() == 1)
	{
		CreateBAK();
		SaveLevels();
	} else {
		if (iScreen != 3)
		{
			iScreen = 3;
			PlaySound ("wav/screen2or3.wav");
		}
	}
}
/*****************************************************************************/
void SetLocation (int iX, int iY, int iObject, int iGraphics)
/*****************************************************************************/
{
	int iObjectOld;

	/*** Remember the old object. ***/
	iObjectOld = arLevelObjects[iCurLevel][iX][iY];

	/*** Object. ***/
	arLevelObjects[iCurLevel][iX][iY] = iObject;
	iLastObject = iObject;

	/*** Remove all other swords. ***/
	if (iObject == 0x0B) { SingleSword (iCurLevel, iX, iY); }

	/*** If necessary, remove the old object's attributes. ***/
	switch (iObjectOld)
	{
		case 0x03: /*** raise (buttons) ***/
			AddRemoveAttributes (0x03, iX, iY, arNrRaise,
				arRaiseX[iCurLevel], arRaiseY[iCurLevel], 0); break;
		case 0x04: /*** drop (buttons) ***/
			AddRemoveAttributes (0x04, iX, iY, arNrDrop,
				arDropX[iCurLevel], arDropY[iCurLevel], 0); break;
		case 0x05: /*** gates ***/
			AddRemoveAttributes (0x05, iX, iY, arNrGates,
				arGateX[iCurLevel], arGateY[iCurLevel], 0); break;
		case 0x06: /*** loose (floors) ***/
			AddRemoveAttributes (0x06, iX, iY, arNrLoose,
				arLooseX[iCurLevel], arLooseY[iCurLevel], 0); break;
		case 0x07: /*** spikes ***/
			AddRemoveAttributes (0x07, iX, iY, arNrSpikes,
				arSpikeX[iCurLevel], arSpikeY[iCurLevel], 0); break;
		case 0x08: /*** chompers ***/
			AddRemoveAttributes (0x08, iX, iY, arNrChompers,
				arChomperX[iCurLevel], arChomperY[iCurLevel], 0); break;
		case 0x09: /*** potions ***/
			AddRemoveAttributes (0x09, iX, iY, arNrPotions,
				arPotionX[iCurLevel], arPotionY[iCurLevel], 0); break;
		case 0x0A: /*** (level) doors ***/
			AddRemoveAttributes (0x0A, iX, iY, arNrDoors,
				arDoorX[iCurLevel], arDoorY[iCurLevel], 0); break;
		case 0x0C: /*** guards ***/
			AddRemoveAttributes (0x0C, iX, iY, arNrGuards,
				arGuardX[iCurLevel], arGuardY[iCurLevel], 0); break;
	}

	/*** If necessary, add the new object's (X and Y) attributes. ***/
	switch (iObject)
	{
		case 0x03: /*** raise (buttons) ***/
			AddRemoveAttributes (0x03, iX, iY, arNrRaise,
				arRaiseX[iCurLevel], arRaiseY[iCurLevel], 1); break;
		case 0x04: /*** drop (buttons) ***/
			AddRemoveAttributes (0x04, iX, iY, arNrDrop,
				arDropX[iCurLevel], arDropY[iCurLevel], 1); break;
		case 0x05: /*** gates ***/
			AddRemoveAttributes (0x05, iX, iY, arNrGates,
				arGateX[iCurLevel], arGateY[iCurLevel], 1); break;
		case 0x06: /*** loose (floors) ***/
			AddRemoveAttributes (0x06, iX, iY, arNrLoose,
				arLooseX[iCurLevel], arLooseY[iCurLevel], 1); break;
		case 0x07: /*** spikes ***/
			AddRemoveAttributes (0x07, iX, iY, arNrSpikes,
				arSpikeX[iCurLevel], arSpikeY[iCurLevel], 1); break;
		case 0x08: /*** chompers ***/
			AddRemoveAttributes (0x08, iX, iY, arNrChompers,
				arChomperX[iCurLevel], arChomperY[iCurLevel], 1); break;
		case 0x09: /*** potions ***/
			AddRemoveAttributes (0x09, iX, iY, arNrPotions,
				arPotionX[iCurLevel], arPotionY[iCurLevel], 1); break;
		case 0x0A: /*** (level) doors ***/
			AddRemoveAttributes (0x0A, iX, iY, arNrDoors,
				arDoorX[iCurLevel], arDoorY[iCurLevel], 1); break;
		case 0x0C: /*** guards ***/
			AddRemoveAttributes (0x0C, iX, iY, arNrGuards,
				arGuardX[iCurLevel], arGuardY[iCurLevel], 1); break;
	}

	/*** Graphics. ***/
	if (iGraphics != -1)
	{
		arLevelGraphics[iCurLevel][iX][iY] = iGraphics;
		iLastGraphics = iGraphics;
	} else {
		iLastGraphics = arLevelGraphics[iCurLevel][iX][iY];
	}
}
/*****************************************************************************/
void SetAttribute (int iX, int iY, int *arX, int *arY, int *arAttr,
	int iNrObjects, int iValue)
/*****************************************************************************/
{
	/*** Used for looping. ***/
	int iObjectLoop;

	for (iObjectLoop = 1; iObjectLoop <= iNrObjects; iObjectLoop++)
	{
		if ((arX[iObjectLoop] == iX) && (arY[iObjectLoop] == iY))
		{
			arAttr[iObjectLoop] = iValue;
		}
	}
}
/*****************************************************************************/
int HasObject (int iX, int iY, int *arX, int *arY, int iNrObjects)
/*****************************************************************************/
{
	/*** Used for looping. ***/
	int iObjectLoop;

	for (iObjectLoop = 1; iObjectLoop <= iNrObjects; iObjectLoop++)
	{
		if ((arX[iObjectLoop] == iX) && (arY[iObjectLoop] == iY))
		{
			return (1);
		}
	}

	return (0);
}
/*****************************************************************************/
int InArea (int iUpperLeftX, int iUpperLeftY,
	int iLowerRightX, int iLowerRightY)
/*****************************************************************************/
{
	if ((iUpperLeftX * iScale <= iXPos) &&
		(iLowerRightX * iScale >= iXPos) &&
		(iUpperLeftY * iScale <= iYPos) &&
		(iLowerRightY * iScale >= iYPos))
	{
		return (1);
	} else {
		return (0);
	}
}
/*****************************************************************************/
int OnLevelBar (void)
/*****************************************************************************/
{
	if (InArea (28, 3, 602, 22) == 1) { return (1); } else { return (0); }
}
/*****************************************************************************/
void ChangePos (void)
/*****************************************************************************/
{
	int iChanging;
	SDL_Event event;
	int iOldXPos, iOldYPos;
	int iUseTile;
	int iNowOn;
	int iNewXLeft, iNewXRight;
	int iS1, iS2, iS3;

	/*** Used for looping. ***/
	int iXLoop, iYLoop;
	int iTabLoop;
	int iRowLoop;
	int iTileLoop;

	iChanging = 1;
	iCustomHover = 0;

	/* Store old values. Used for custom tile creation and showing
	 * the new image of the object and graphics combo.
	 */
	iNewObject = arLevelObjects[iCurLevel]
		[((iCurX - 1) * WIDTH) + iSelectedX][((iCurY - 1) * HEIGHT) + iSelectedY];
	iNewGraphics = arLevelGraphics[iCurLevel]
		[((iCurX - 1) * WIDTH) + iSelectedX][((iCurY - 1) * HEIGHT) + iSelectedY];
	iNewXLeft = ((iCurX - 1) * WIDTH) + iSelectedX - 1;
	if (iNewXLeft < 1)
	{
		iNewObjectLeft = 0x01; /*** wall ***/
		iNewGraphicsLeft = 0x00; /*** black ***/
	} else {
		iNewObjectLeft = arLevelObjects[iCurLevel]
			[iNewXLeft][((iCurY - 1) * HEIGHT) + iSelectedY];
		iNewGraphicsLeft = arLevelGraphics[iCurLevel]
			[iNewXLeft][((iCurY - 1) * HEIGHT) + iSelectedY];
	}
	iNewXRight = ((iCurX - 1) * WIDTH) + iSelectedX + 1;
	if (iNewXRight > (arLevelWidth[iCurLevel] * WIDTH))
	{
		iNewObjectRight = 0x01; /*** wall ***/
		iNewGraphicsRight = 0x00; /*** black ***/
	} else {
		iNewObjectRight = arLevelObjects[iCurLevel]
			[iNewXRight][((iCurY - 1) * HEIGHT) + iSelectedY];
		iNewGraphicsRight = arLevelGraphics[iCurLevel]
			[iNewXRight][((iCurY - 1) * HEIGHT) + iSelectedY];
	}

	/*** Old graphics. ***/
	for (iTabLoop = 1; iTabLoop <= 15; iTabLoop++)
	{
		for (iRowLoop = 1; iRowLoop <= 2; iRowLoop++)
		{
			for (iTileLoop = 1; iTileLoop <= 15; iTileLoop++)
			{
				if (EnvTabRow[arLevelType[iCurLevel]]
					[iTabLoop][iRowLoop][iTileLoop] == iNewGraphics)
				{
					iOldGraphicsX = iTileLoop;
					iOldGraphicsY = iRowLoop;
					iOldGraphicsTab = iTabLoop;
				}
			}
		}
	}

	/*** Set the initial tab states. ***/
	iTabObject = iNewObject + 1;
	iTabGraphics = iOldGraphicsTab;
	iOnTile = ((iOldGraphicsY - 1) * 15) + iOldGraphicsX;

	/*** Set the initial object attributes. ***/
	switch (iNewObject)
	{
		case 0x03: /*** raise ***/
			iRaiseGate1 = GetSelectedTileValue (arRaiseGate1[iCurLevel],
				arRaiseX[iCurLevel], arRaiseY[iCurLevel],
				arNrRaise[iCurLevel], 0);
			switch (iRaiseGate1)
			{
				case 0xFFFD: iRaiseGate1Check = 0xFFFD; break; /*** none ***/
				case 0xFFFE: iRaiseGate1Check = 0xFFFE; break; /*** mirror ***/
				case 0xFFFF: iRaiseGate1Check = 0xFFFF; break; /*** exit ***/
				default: iRaiseGate1Check = -1; break; /*** gate no ***/
			}
			iRaiseGate2 = GetSelectedTileValue (arRaiseGate2[iCurLevel],
				arRaiseX[iCurLevel], arRaiseY[iCurLevel],
				arNrRaise[iCurLevel], 0xFFFD);
			switch (iRaiseGate2)
			{
				case 0xFFFD: iRaiseGate2Check = 0xFFFD; break; /*** none ***/
				case 0xFFFE: iRaiseGate2Check = 0xFFFE; break; /*** mirror ***/
				case 0xFFFF: iRaiseGate2Check = 0xFFFF; break; /*** exit ***/
				default: iRaiseGate2Check = -1; break; /*** gate no ***/
			}
			iRaiseGate3 = GetSelectedTileValue (arRaiseGate3[iCurLevel],
				arRaiseX[iCurLevel], arRaiseY[iCurLevel],
				arNrRaise[iCurLevel], 0xFFFD);
			switch (iRaiseGate3)
			{
				case 0xFFFD: iRaiseGate3Check = 0xFFFD; break; /*** none ***/
				case 0xFFFE: iRaiseGate3Check = 0xFFFE; break; /*** mirror ***/
				case 0xFFFF: iRaiseGate3Check = 0xFFFF; break; /*** exit ***/
				default: iRaiseGate3Check = -1; break; /*** gate no ***/
			}
			break;
		case 0x04: /*** drop ***/
			iDropGate1 = GetSelectedTileValue (arDropGate1[iCurLevel],
				arDropX[iCurLevel], arDropY[iCurLevel],
				arNrDrop[iCurLevel], 0);
			switch (iDropGate1)
			{
				case 0xFFFD: iDropGate1Check = 0xFFFD; break; /*** none ***/
				case 0xFFFE: iDropGate1Check = 0xFFFE; break; /*** mirror ***/
				case 0xFFFF: iDropGate1Check = 0xFFFF; break; /*** exit ***/
				default: iDropGate1Check = -1; break; /*** gate no ***/
			}
			iDropGate2 = GetSelectedTileValue (arDropGate2[iCurLevel],
				arDropX[iCurLevel], arDropY[iCurLevel],
				arNrDrop[iCurLevel], 0xFFFD);
			switch (iDropGate2)
			{
				case 0xFFFD: iDropGate2Check = 0xFFFD; break; /*** none ***/
				case 0xFFFE: iDropGate2Check = 0xFFFE; break; /*** mirror ***/
				case 0xFFFF: iDropGate2Check = 0xFFFF; break; /*** exit ***/
				default: iDropGate2Check = -1; break; /*** gate no ***/
			}
			iDropGate3 = GetSelectedTileValue (arDropGate3[iCurLevel],
				arDropX[iCurLevel], arDropY[iCurLevel],
				arNrDrop[iCurLevel], 0xFFFD);
			switch (iDropGate3)
			{
				case 0xFFFD: iDropGate3Check = 0xFFFD; break; /*** none ***/
				case 0xFFFE: iDropGate3Check = 0xFFFE; break; /*** mirror ***/
				case 0xFFFF: iDropGate3Check = 0xFFFF; break; /*** exit ***/
				default: iDropGate3Check = -1; break; /*** gate no ***/
			}
			break;
		case 0x05: /*** gate ***/
			iS1 = GetSelectedTileValue (arGateState1[iCurLevel],
				arGateX[iCurLevel], arGateY[iCurLevel],
				arNrGates[iCurLevel], 0);
			iS2 = GetSelectedTileValue (arGateState2[iCurLevel],
				arGateX[iCurLevel], arGateY[iCurLevel],
				arNrGates[iCurLevel], 0);
			iS3 = GetSelectedTileValue (arGateState3[iCurLevel],
				arGateX[iCurLevel], arGateY[iCurLevel],
				arNrGates[iCurLevel], 0);
			iS3 = iS3 / 257; /*** Because 0xFFFF / 257 = 255. ***/
			if ((iS1 == 0) && (iS2 == 0) && (iS3 == 0)) /*** closed ***/
			{
				iGateState = 0;
				iGateStateCheck = 0;
			} else if ((iS1 == 7) && (iS2 == 12) && (iS3 == 0)) { /*** c. s. ***/
				iGateState = 7;
				iGateStateCheck = 7;
			} else if ((iS1 == 3) && (iS2 == 12) && (iS3 == 255)) { /*** open ***/
				iGateState = 3;
				iGateStateCheck = 3;
			} else if ((iS1 == 1) && (iS2 == 0) && (iS3 == 0)) { /*** o. s. ***/
				iGateState = 1;
				iGateStateCheck = 1;
			} else switch (iS1) {
				case 0: /*** closed, custom ***/
					iGateState = 0;
					iGateStateCheck = 10;
					break;
				case 7: /*** close on start, custom ***/
					iGateState = 7;
					iGateStateCheck = 17;
					break;
				case 3: /*** open, custom ***/
					iGateState = 3;
					iGateStateCheck = 13;
					break;
				case 1: /*** open on start, custom ***/
					iGateState = 1;
					iGateStateCheck = 11;
					break;
			}
			iGateOpenness = iS2;
			iGateDelay = iS3;
			break;
		case 0x09: /*** potion ***/
			iPotionColor = GetSelectedTileValue (arPotionColor[iCurLevel],
				arPotionX[iCurLevel], arPotionY[iCurLevel],
				arNrPotions[iCurLevel], 1);
			switch (iPotionColor)
			{
				case 1: iPotionColorCheck = 1; break; /*** red ***/
				case 3: iPotionColorCheck = 3; break; /*** green ***/
				default: iPotionColorCheck = 10; break; /*** color no ***/
			}
			iPotionEffect = GetSelectedTileValue (arPotionEffect[iCurLevel],
				arPotionX[iCurLevel], arPotionY[iCurLevel],
				arNrPotions[iCurLevel], 0);
			switch (iPotionEffect)
			{
				case 0: iPotionEffectCheck = 0; break; /*** heal ***/
				case 1: iPotionEffectCheck = 1; break; /*** life ***/
				case 2: iPotionEffectCheck = 2; break; /*** hurt ***/
				case 3: iPotionEffectCheck = 3; break; /*** float ***/
				case 4: iPotionEffectCheck = 4; break; /*** loose ***/
				case 5: iPotionEffectCheck = 5; break; /*** stop ***/
				case 6: iPotionEffectCheck = 6; break; /*** time ***/
				case 7: iPotionEffectCheck = 7; break; /*** time ***/
				case 8: iPotionEffectCheck = 8; break; /*** time ***/
				case 9: iPotionEffectCheck = 9; break; /*** time ***/
				default: iPotionEffectCheck = 10; break; /*** effect no ***/
			}
			break;
		case 0x0A: /*** door ***/
			iDoorType = GetSelectedTileValue (arDoorType[iCurLevel],
				arDoorX[iCurLevel], arDoorY[iCurLevel],
				arNrDoors[iCurLevel], 0);
			switch (iDoorType)
			{
				case 0: iDoorTypeCheck = 0; break; /*** entrance ***/
				case 2: iDoorTypeCheck = 2; break; /*** exit ***/
			}
			break;
	}

	/*** Set the initial guard attributes. ***/
	iNewSkill = GetSelectedTileValue (arGuardSkill[iCurLevel],
		arGuardX[iCurLevel], arGuardY[iCurLevel],
		arNrGuards[iCurLevel], 0);
	iNewHP = GetSelectedTileValue (arGuardHP[iCurLevel],
		arGuardX[iCurLevel], arGuardY[iCurLevel],
		arNrGuards[iCurLevel], 3);

	ShowChange();
	while (iChanging == 1)
	{
		while (SDL_PollEvent (&event))
		{
			switch (event.type)
			{
				case SDL_CONTROLLERBUTTONDOWN:
					/*** Nothing for now. ***/
					break;
				case SDL_CONTROLLERBUTTONUP:
					switch (event.cbutton.button)
					{
						case SDL_CONTROLLER_BUTTON_A:
							if (iOnTile != 0)
							{
								UseTile (iOnTile, iSelectedX + ((iCurX - 1) * WIDTH),
									iSelectedY + ((iCurY - 1) * HEIGHT));
								if (iOnTile <= 30) { iChanging = 0; }
								iChanged++;
							}
							break;
						case SDL_CONTROLLER_BUTTON_B:
							iChanging = 0; break;
						case SDL_CONTROLLER_BUTTON_X:
							UseTile (-2, iSelectedX + ((iCurX - 1) * WIDTH),
								iSelectedY + ((iCurY - 1) & HEIGHT));
							iChanging = 0;
							iChanged++;
							break;
						case SDL_CONTROLLER_BUTTON_Y:
							iTabObject++;
							if (iTabObject > 12) { iTabObject = 1; }
							iNewObject = iTabObject - 1;
							AttributeDefaults (iNewObject);
							PlaySound ("wav/hum_adj.wav");
							break;
						case SDL_CONTROLLER_BUTTON_GUIDE:
							iTabGraphics++;
							if (iTabGraphics > 15) { iTabGraphics = 1; }
							iOnTile = 1;
							iNewGraphics = OnGraphics();
							PlaySound ("wav/hum_adj.wav");
							break;
						case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
							ChangePosAction ("left"); break;
						case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
							ChangePosAction ("right"); break;
						case SDL_CONTROLLER_BUTTON_DPAD_UP:
							ChangePosAction ("up"); break;
						case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
							ChangePosAction ("down"); break;
						case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
							ChangeCustom (-1, &iNewObject, 0x00, 0x0B);
							iTabObject = iNewObject + 1;
							AttributeDefaults (iNewObject);
							break;
						case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
							ChangeCustom (1, &iNewObject, 0x00, 0x0B);
							iTabObject = iNewObject + 1;
							AttributeDefaults (iNewObject);
							break;
						case SDL_CONTROLLER_BUTTON_BACK:
							ChangeCustom (-1, &iNewGraphics, 0x00, 0xFF);
							UpdateOnTile();
							break;
						case SDL_CONTROLLER_BUTTON_START:
							ChangeCustom (1, &iNewGraphics, 0x00, 0xFF);
							UpdateOnTile();
							break;
					}
					ShowChange();
					break;
				case SDL_CONTROLLERAXISMOTION: /*** triggers and analog sticks ***/
					iXJoy1 = SDL_JoystickGetAxis (joystick, 0);
					iYJoy1 = SDL_JoystickGetAxis (joystick, 1);
					iXJoy2 = SDL_JoystickGetAxis (joystick, 3);
					iYJoy2 = SDL_JoystickGetAxis (joystick, 4);
					if ((iXJoy1 < -30000) || (iXJoy2 < -30000)) /*** left ***/
					{
						if ((SDL_GetTicks() - joyleft) > 300)
						{
							ChangeCustom (-1, &iNewHP, 0, 999); UpdateSkillHP();
							joyleft = SDL_GetTicks();
						}
					}
					if ((iXJoy1 > 30000) || (iXJoy2 > 30000)) /*** right ***/
					{
						if ((SDL_GetTicks() - joyright) > 300)
						{
							ChangeCustom (1, &iNewHP, 0, 999); UpdateSkillHP();
							joyright = SDL_GetTicks();
						}
					}
					if ((iYJoy1 < -30000) || (iYJoy2 < -30000)) /*** up ***/
					{
						if ((SDL_GetTicks() - joyup) > 300)
						{
							ChangeCustom (10, &iNewHP, 0, 999); UpdateSkillHP();
							joyup = SDL_GetTicks();
						}
					}
					if ((iYJoy1 > 30000) || (iYJoy2 > 30000)) /*** down ***/
					{
						if ((SDL_GetTicks() - joydown) > 300)
						{
							ChangeCustom (-10, &iNewHP, 0, 999); UpdateSkillHP();
							joydown = SDL_GetTicks();
						}
					}
					if (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
					{
						if ((SDL_GetTicks() - trigleft) > 300)
						{
							ChangeCustom (-1, &iNewSkill, 0, 999); UpdateSkillHP();
							trigleft = SDL_GetTicks();
						}
					}
					if (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
					{
						if ((SDL_GetTicks() - trigright) > 300)
						{
							ChangeCustom (1, &iNewSkill, 0, 999); UpdateSkillHP();
							trigright = SDL_GetTicks();
						}
					}
					ShowChange();
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_KP_ENTER:
						case SDLK_RETURN:
						case SDLK_SPACE:
							if (event.key.keysym.mod & KMOD_CTRL)
							{
								if ((iOnTile >= 1) && (iOnTile <= 30))
								{
									for (iXLoop = 1; iXLoop <= arLevelWidth[iCurLevel]
										* WIDTH; iXLoop++)
									{
										for (iYLoop = 1; iYLoop <= arLevelHeight[iCurLevel]
											* HEIGHT; iYLoop++)
										{
											UseTile (iOnTile, iXLoop, iYLoop);
										}
									}
									iChanging = 0;
									iChanged++;
								}
							} else if (event.key.keysym.mod & KMOD_SHIFT) {
								if ((iOnTile >= 1) && (iOnTile <= 30))
								{
									for (iXLoop = 1; iXLoop <= WIDTH; iXLoop++)
									{
										for (iYLoop = 1; iYLoop <= HEIGHT; iYLoop++)
										{
											UseTile (iOnTile, iXLoop + ((iCurX - 1) * WIDTH),
												iYLoop + ((iCurY - 1) * HEIGHT));
										}
									}
									iChanging = 0;
									iChanged++;
								}
							} else if (iOnTile != 0) {
								UseTile (iOnTile, iSelectedX + ((iCurX - 1) * WIDTH),
									iSelectedY + ((iCurY - 1) * HEIGHT));
								if (iOnTile <= 30) { iChanging = 0; }
								iChanged++;
							}
							break;
						case SDLK_ESCAPE:
						case SDLK_q:
						case SDLK_c:
							iChanging = 0; break;
						case SDLK_LEFT:
							if (event.key.keysym.mod & KMOD_CTRL)
							{
								ChangeCustom (-10, &iNewSkill, 0, 999); UpdateSkillHP();
							} else if (event.key.keysym.mod & KMOD_SHIFT) {
								ChangeCustom (-1, &iNewSkill, 0, 999); UpdateSkillHP();
							} else {
								ChangePosAction ("left");
							}
							break;
						case SDLK_RIGHT:
							if (event.key.keysym.mod & KMOD_CTRL)
							{
								ChangeCustom (10, &iNewSkill, 0, 999); UpdateSkillHP();
							} else if (event.key.keysym.mod & KMOD_SHIFT) {
								ChangeCustom (1, &iNewSkill, 0, 999); UpdateSkillHP();
							} else {
								ChangePosAction ("right");
							}
							break;
						case SDLK_UP:
							if (event.key.keysym.mod & KMOD_CTRL)
							{
								ChangeCustom (10, &iNewHP, 0, 999); UpdateSkillHP();
							} else if (event.key.keysym.mod & KMOD_SHIFT) {
								ChangeCustom (1, &iNewHP, 0, 999); UpdateSkillHP();
							} else {
								ChangePosAction ("up");
							}
							break;
						case SDLK_DOWN:
							if (event.key.keysym.mod & KMOD_CTRL)
							{
								ChangeCustom (-10, &iNewHP, 0, 999); UpdateSkillHP();
							} else if (event.key.keysym.mod & KMOD_SHIFT) {
								ChangeCustom (-1, &iNewHP, 0, 999); UpdateSkillHP();
							} else {
								ChangePosAction ("down");
							}
							break;
						case SDLK_LEFTBRACKET:
							if (event.key.keysym.mod & KMOD_SHIFT)
							{ /*** { ***/
								if (iTabGraphics > 1)
								{
									iTabGraphics--;
									iOnTile = 1;
									iNewGraphics = OnGraphics();
									PlaySound ("wav/hum_adj.wav");
								}
							} else { /*** [ ***/
								if (iTabObject > 1)
								{
									iTabObject--;
									iNewObject = iTabObject - 1;
									AttributeDefaults (iNewObject);
									PlaySound ("wav/hum_adj.wav");
								}
							}
							break;
						case SDLK_RIGHTBRACKET:
							if (event.key.keysym.mod & KMOD_SHIFT)
							{ /*** } ***/
								if (iTabGraphics < 15)
								{
									iTabGraphics++;
									iOnTile = 1;
									iNewGraphics = OnGraphics();
									PlaySound ("wav/hum_adj.wav");
								}
							} else { /*** ] ***/
								if (iTabObject < 12)
								{
									iTabObject++;
									iNewObject = iTabObject - 1;
									AttributeDefaults (iNewObject);
									PlaySound ("wav/hum_adj.wav");
								}
							}
							break;
						default: break;
					}
					ShowChange();
					break;
				case SDL_MOUSEMOTION:
					iOldXPos = iXPos;
					iOldYPos = iYPos;
					iXPos = event.motion.x;
					iYPos = event.motion.y;
					if ((iOldXPos == iXPos) && (iOldYPos == iYPos)) { break; }

					/*** custom hover ***/
					iCustomHoverOld = iCustomHover;
					if (InArea (271, 399, 271 + 252, 399 + 33) == 1)
						{ iCustomHover = 1; } else { iCustomHover = 0; }
					if (iCustomHover != iCustomHoverOld) { ShowChange(); }

					iNowOn = OnTile();
					if ((iOnTile != iNowOn) && (iNowOn != 0))
					{
						if (IsDisabled (iNowOn) == 0)
						{
							iOnTile = iNowOn;
							iNewGraphics = OnGraphics();
							ShowChange();
						}
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == 1) /*** left mouse button ***/
					{
						if (InArea (656, 0, 656 + 36, 0 + 467) == 1) /*** close ***/
						{
							iCloseOn = 1;
							ShowChange();
						}
					}
					break;
				case SDL_MOUSEBUTTONUP:
					iCloseOn = 0;

					/*** On graphics or living. ***/
					iUseTile = 0;
					if ((InArea (10, 193, 10 + 512, 193 + 134) == 1) ||
						(InArea (204, 329, 204 + 322, 329 + 69) == 1) ||
						(InArea (204, 396, 204 + 66, 396 + 69) == 1)) { iUseTile = 1; }

					/*** On the custom tile area. ***/
					if (InArea (271, 399, 271 + 252, 399 + 33) == 1)
					{
						iOnTile = -2;
						iUseTile = 1;
					}

					if (event.button.button == 1) /*** left mouse button ***/
					{
						/*** Guard skill. ***/
						if (InArea (532, 393, 532 + 13, 393 + 20) == 1)
							{ ChangeCustom (-10, &iNewSkill, 0, 999); UpdateSkillHP(); }
						if (InArea (547, 393, 547 + 13, 393 + 20) == 1)
							{ ChangeCustom (-1, &iNewSkill, 0, 999); UpdateSkillHP(); }
						if (InArea (617, 393, 617 + 13, 393 + 20) == 1)
							{ ChangeCustom (1, &iNewSkill, 0, 999); UpdateSkillHP(); }
						if (InArea (632, 393, 632 + 13, 393 + 20) == 1)
							{ ChangeCustom (10, &iNewSkill, 0, 999); UpdateSkillHP(); }

						/*** Guard HP. ***/
						if (InArea (532, 437, 532 + 13, 437 + 20) == 1)
							{ ChangeCustom (-10, &iNewHP, 0, 999); UpdateSkillHP(); }
						if (InArea (547, 437, 547 + 13, 437 + 20) == 1)
							{ ChangeCustom (-1, &iNewHP, 0, 999); UpdateSkillHP(); }
						if (InArea (617, 437, 617 + 13, 437 + 20) == 1)
							{ ChangeCustom (1, &iNewHP, 0, 999); UpdateSkillHP(); }
						if (InArea (632, 437, 632 + 13, 437 + 20) == 1)
							{ ChangeCustom (10, &iNewHP, 0, 999); UpdateSkillHP(); }

						/*** Changing the object tab. ***/
						for (iTabLoop = 1; iTabLoop <= 12; iTabLoop++)
						{
							if ((InArea (28 + ((iTabLoop - 1) * 52), 6,
								28 + ((iTabLoop - 1) * 52) + 50, 6 + 55) == 1) &&
								(iTabObject != iTabLoop))
							{
								iTabObject = iTabLoop;
								iNewObject = iTabLoop - 1;
								AttributeDefaults (iNewObject);
								PlaySound ("wav/hum_adj.wav");
							}
						}

						/*** Changing the graphics tab. ***/
						for (iTabLoop = 1; iTabLoop <= 15; iTabLoop++)
						{
							if ((InArea (12 + ((iTabLoop - 1) * 34), 127,
								12 + ((iTabLoop - 1) * 34) + 32, 127 + 64) == 1) &&
								(iTabGraphics != iTabLoop))
							{
								iTabGraphics = iTabLoop;
								iOnTile = 1;
								iNewGraphics = OnGraphics();
								PlaySound ("wav/hum_adj.wav");
							}
						}

						/*** Custom: changing the object. ***/
						if (InArea (276, 437, 276 + 13, 437 + 20) == 1)
						{
							ChangeCustom (-16, &iNewObject, 0x00, 0x0B);
							iTabObject = iNewObject + 1;
							AttributeDefaults (iNewObject);
						}
						if (InArea (291, 437, 291 + 13, 437 + 20) == 1)
						{
							ChangeCustom (-1, &iNewObject, 0x00, 0x0B);
							iTabObject = iNewObject + 1;
							AttributeDefaults (iNewObject);
						}
						if (InArea (361, 437, 361 + 13, 437 + 20) == 1)
						{
							ChangeCustom (1, &iNewObject, 0x00, 0x0B);
							iTabObject = iNewObject + 1;
							AttributeDefaults (iNewObject);
						}
						if (InArea (376, 437, 376 + 13, 437 + 20) == 1)
						{
							ChangeCustom (16, &iNewObject, 0x00, 0x0B);
							iTabObject = iNewObject + 1;
							AttributeDefaults (iNewObject);
						}

						/*** Custom: changing the graphics. ***/
						if (InArea (405, 437, 405 + 13, 437 + 20) == 1)
						{
							ChangeCustom (-16, &iNewGraphics, 0x00, 0xFF);
							UpdateOnTile();
						}
						if (InArea (420, 437, 420 + 13, 437 + 20) == 1)
						{
							ChangeCustom (-1, &iNewGraphics, 0x00, 0xFF);
							UpdateOnTile();
						}
						if (InArea (490, 437, 490 + 13, 437 + 20) == 1)
						{
							ChangeCustom (1, &iNewGraphics, 0x00, 0xFF);
							UpdateOnTile();
						}
						if (InArea (505, 437, 505 + 13, 437 + 20) == 1)
						{
							ChangeCustom (16, &iNewGraphics, 0x00, 0xFF);
							UpdateOnTile();
						}

						/*** On close. ***/
						if (InArea (656, 0, 656 + 36, 0 + 467) == 1) { iChanging = 0; }

						if (iUseTile == 1)
						{
							UseTile (iOnTile, iSelectedX + ((iCurX - 1) * WIDTH),
								iSelectedY + ((iCurY - 1) * HEIGHT));
							if (iOnTile <= 30) { iChanging = 0; }
							iChanged++;
						}

						/*** Changing object attributes. ***/
						switch (iNewObject)
						{
							case 0x03: /*** raise ***/
								/*========== 1 ==========*/
								SetCheck ("raisegate1", 35, 72, -1, iRaiseGate1);
								if (InArea (35, 72, 35 + 14, 72 + 14) == 1)
								{
									if (iRaiseGate1 > 999) { iRaiseGate1 = 0; }
								}
								SetCheck ("raisegate1", 35, 98, 0xFFFD, 0xFFFD); /*** none ***/
								SetCheck ("raisegate1", 95, 98, 0xFFFE, 0xFFFE); /*** mir. ***/
								SetCheck ("raisegate1", 155, 98, 0xFFFF, 0xFFFF); /*** ex. ***/
								if (InArea (113, 69, 113 + 13, 69 + 20) == 1) /*** -10 ***/
								{
									iRaiseGate1Check = -1;
									if (iRaiseGate1 > 999) { iRaiseGate1 = 0; }
									ChangeCustom (-10, &iRaiseGate1, 0x00, 0xFF);
								}
								if (InArea (128, 69, 128 + 13, 69 + 20) == 1) /*** -1 ***/
								{
									iRaiseGate1Check = -1;
									if (iRaiseGate1 > 999) { iRaiseGate1 = 0; }
									ChangeCustom (-1, &iRaiseGate1, 0x00, 0xFF);
								}
								if (InArea (198, 69, 198 + 13, 69 + 20) == 1) /*** 1 ***/
								{
									iRaiseGate1Check = -1;
									if (iRaiseGate1 > 999) { iRaiseGate1 = 0; }
									ChangeCustom (1, &iRaiseGate1, 0x00, 0xFF);
								}
								if (InArea (213, 69, 213 + 13, 69 + 20) == 1) /*** 10 ***/
								{
									iRaiseGate1Check = -1;
									if (iRaiseGate1 > 999) { iRaiseGate1 = 0; }
									ChangeCustom (10, &iRaiseGate1, 0x00, 0xFF);
								}
								/*========== 2 ==========*/
								SetCheck ("raisegate2", 244, 72, -1, iRaiseGate2);
								if (InArea (244, 72, 244 + 14, 72 + 14) == 1)
								{
									if (iRaiseGate2 > 999) { iRaiseGate2 = 0; }
								}
								SetCheck ("raisegate2", 244, 98, 0xFFFD, 0xFFFD); /*** n. ***/
								SetCheck ("raisegate2", 304, 98, 0xFFFE, 0xFFFE); /*** m. ***/
								SetCheck ("raisegate2", 364, 98, 0xFFFF, 0xFFFF); /*** ex. ***/
								if (InArea (322, 69, 322 + 13, 69 + 20) == 1) /*** -10 ***/
								{
									iRaiseGate2Check = -1;
									if (iRaiseGate2 > 999) { iRaiseGate2 = 0; }
									ChangeCustom (-10, &iRaiseGate2, 0x00, 0xFF);
								}
								if (InArea (337, 69, 337 + 13, 69 + 20) == 1) /*** -1 ***/
								{
									iRaiseGate2Check = -1;
									if (iRaiseGate2 > 999) { iRaiseGate2 = 0; }
									ChangeCustom (-1, &iRaiseGate2, 0x00, 0xFF);
								}
								if (InArea (407, 69, 407 + 13, 69 + 20) == 1) /*** 1 ***/
								{
									iRaiseGate2Check = -1;
									if (iRaiseGate2 > 999) { iRaiseGate2 = 0; }
									ChangeCustom (1, &iRaiseGate2, 0x00, 0xFF);
								}
								if (InArea (422, 69, 422 + 13, 69 + 20) == 1) /*** 10 ***/
								{
									iRaiseGate2Check = -1;
									if (iRaiseGate2 > 999) { iRaiseGate2 = 0; }
									ChangeCustom (10, &iRaiseGate2, 0x00, 0xFF);
								}
								/*========== 3 ==========*/
								SetCheck ("raisegate3", 453, 72, -1, iRaiseGate3);
								if (InArea (453, 72, 453 + 14, 72 + 14) == 1)
								{
									if (iRaiseGate3 > 999) { iRaiseGate3 = 0; }
								}
								SetCheck ("raisegate3", 453, 98, 0xFFFD, 0xFFFD); /*** n. ***/
								SetCheck ("raisegate3", 513, 98, 0xFFFE, 0xFFFE); /*** m. ***/
								SetCheck ("raisegate3", 573, 98, 0xFFFF, 0xFFFF); /*** ex. ***/
								if (InArea (531, 69, 531 + 13, 69 + 20) == 1) /*** -10 ***/
								{
									iRaiseGate3Check = -1;
									if (iRaiseGate3 > 999) { iRaiseGate3 = 0; }
									ChangeCustom (-10, &iRaiseGate3, 0x00, 0xFF);
								}
								if (InArea (546, 69, 546 + 13, 69 + 20) == 1) /*** -1 ***/
								{
									iRaiseGate3Check = -1;
									if (iRaiseGate3 > 999) { iRaiseGate3 = 0; }
									ChangeCustom (-1, &iRaiseGate3, 0x00, 0xFF);
								}
								if (InArea (616, 69, 616 + 13, 69 + 20) == 1) /*** 1 ***/
								{
									iRaiseGate3Check = -1;
									if (iRaiseGate3 > 999) { iRaiseGate3 = 0; }
									ChangeCustom (1, &iRaiseGate3, 0x00, 0xFF);
								}
								if (InArea (631, 69, 631 + 13, 69 + 20) == 1) /*** 10 ***/
								{
									iRaiseGate3Check = -1;
									if (iRaiseGate3 > 999) { iRaiseGate3 = 0; }
									ChangeCustom (10, &iRaiseGate3, 0x00, 0xFF);
								}
								break;
							case 0x04: /*** drop ***/
								/*========== 1 ==========*/
								SetCheck ("dropgate1", 35, 72, -1, iDropGate1);
								if (InArea (35, 72, 35 + 14, 72 + 14) == 1)
								{
									if (iDropGate1 > 999) { iDropGate1 = 0; }
								}
								SetCheck ("dropgate1", 35, 98, 0xFFFD, 0xFFFD); /*** none ***/
								SetCheck ("dropgate1", 95, 98, 0xFFFE, 0xFFFE); /*** mir. ***/
								SetCheck ("dropgate1", 155, 98, 0xFFFF, 0xFFFF); /*** ex. ***/
								if (InArea (113, 69, 113 + 13, 69 + 20) == 1) /*** -10 ***/
								{
									iDropGate1Check = -1;
									if (iDropGate1 > 999) { iDropGate1 = 0; }
									ChangeCustom (-10, &iDropGate1, 0x00, 0xFF);
								}
								if (InArea (128, 69, 128 + 13, 69 + 20) == 1) /*** -1 ***/
								{
									iDropGate1Check = -1;
									if (iDropGate1 > 999) { iDropGate1 = 0; }
									ChangeCustom (-1, &iDropGate1, 0x00, 0xFF);
								}
								if (InArea (198, 69, 198 + 13, 69 + 20) == 1) /*** 1 ***/
								{
									iDropGate1Check = -1;
									if (iDropGate1 > 999) { iDropGate1 = 0; }
									ChangeCustom (1, &iDropGate1, 0x00, 0xFF);
								}
								if (InArea (213, 69, 213 + 13, 69 + 20) == 1) /*** 10 ***/
								{
									iDropGate1Check = -1;
									if (iDropGate1 > 999) { iDropGate1 = 0; }
									ChangeCustom (10, &iDropGate1, 0x00, 0xFF);
								}
								/*========== 2 ==========*/
								SetCheck ("dropgate2", 244, 72, -1, iDropGate2);
								if (InArea (244, 72, 244 + 14, 72 + 14) == 1)
								{
									if (iDropGate2 > 999) { iDropGate2 = 0; }
								}
								SetCheck ("dropgate2", 244, 98, 0xFFFD, 0xFFFD); /*** n. ***/
								SetCheck ("dropgate2", 304, 98, 0xFFFE, 0xFFFE); /*** m. ***/
								SetCheck ("dropgate2", 364, 98, 0xFFFF, 0xFFFF); /*** ex. ***/
								if (InArea (322, 69, 322 + 13, 69 + 20) == 1) /*** -10 ***/
								{
									iDropGate2Check = -1;
									if (iDropGate2 > 999) { iDropGate2 = 0; }
									ChangeCustom (-10, &iDropGate2, 0x00, 0xFF);
								}
								if (InArea (337, 69, 337 + 13, 69 + 20) == 1) /*** -1 ***/
								{
									iDropGate2Check = -1;
									if (iDropGate2 > 999) { iDropGate2 = 0; }
									ChangeCustom (-1, &iDropGate2, 0x00, 0xFF);
								}
								if (InArea (407, 69, 407 + 13, 69 + 20) == 1) /*** 1 ***/
								{
									iDropGate2Check = -1;
									if (iDropGate2 > 999) { iDropGate2 = 0; }
									ChangeCustom (1, &iDropGate2, 0x00, 0xFF);
								}
								if (InArea (422, 69, 422 + 13, 69 + 20) == 1) /*** 10 ***/
								{
									iDropGate2Check = -1;
									if (iDropGate2 > 999) { iDropGate2 = 0; }
									ChangeCustom (10, &iDropGate2, 0x00, 0xFF);
								}
								/*========== 3 ==========*/
								SetCheck ("dropgate3", 453, 72, -1, iDropGate3);
								if (InArea (453, 72, 453 + 14, 72 + 14) == 1)
								{
									if (iDropGate3 > 999) { iDropGate3 = 0; }
								}
								SetCheck ("dropgate3", 453, 98, 0xFFFD, 0xFFFD); /*** n. ***/
								SetCheck ("dropgate3", 513, 98, 0xFFFE, 0xFFFE); /*** m. ***/
								SetCheck ("dropgate3", 573, 98, 0xFFFF, 0xFFFF); /*** ex. ***/
								if (InArea (531, 69, 531 + 13, 69 + 20) == 1) /*** -10 ***/
								{
									iDropGate3Check = -1;
									if (iDropGate3 > 999) { iDropGate3 = 0; }
									ChangeCustom (-10, &iDropGate3, 0x00, 0xFF);
								}
								if (InArea (546, 69, 546 + 13, 69 + 20) == 1) /*** -1 ***/
								{
									iDropGate3Check = -1;
									if (iDropGate3 > 999) { iDropGate3 = 0; }
									ChangeCustom (-1, &iDropGate3, 0x00, 0xFF);
								}
								if (InArea (616, 69, 616 + 13, 69 + 20) == 1) /*** 1 ***/
								{
									iDropGate3Check = -1;
									if (iDropGate3 > 999) { iDropGate3 = 0; }
									ChangeCustom (1, &iDropGate3, 0x00, 0xFF);
								}
								if (InArea (631, 69, 631 + 13, 69 + 20) == 1) /*** 10 ***/
								{
									iDropGate3Check = -1;
									if (iDropGate3 > 999) { iDropGate3 = 0; }
									ChangeCustom (10, &iDropGate3, 0x00, 0xFF);
								}
								break;
							case 0x05: /*** gate ***/
								SetCheckGate (35, 72, 0, 0, 0, 0); /*** closed ***/
								SetCheckGate (54, 72, 10, 0, 0, 0); /*** closed, custom ***/
								SetCheckGate (121, 72, 7, 7, 12, 0); /*** close on start ***/
								SetCheckGate (140, 72, 17, 7, 12, 0); /*** c. s., custom ***/
								SetCheckGate (35, 98, 3, 3, 12, 255); /*** open ***/
								SetCheckGate (54, 98, 13, 3, 12, 255); /*** open, custom ***/
								SetCheckGate (121, 98, 1, 1, 0, 0); /*** open on start ***/
								SetCheckGate (140, 98, 11, 1, 0, 0); /*** o. s., custom ***/
								if (InArea (315, 69, 315 + 13, 69 + 20) == 1) /*** -10 ***/
								{
									if (iGateStateCheck < 10) { iGateStateCheck+=10; }
									ChangeCustom (-10, &iGateOpenness, 0, 12);
								}
								if (InArea (330, 69, 330 + 13, 69 + 20) == 1) /*** -1 ***/
								{
									if (iGateStateCheck < 10) { iGateStateCheck+=10; }
									ChangeCustom (-1, &iGateOpenness, 0, 12);
								}
								if (InArea (400, 69, 400 + 13, 69 + 20) == 1) /*** 1 ***/
								{
									if (iGateStateCheck < 10) { iGateStateCheck+=10; }
									ChangeCustom (1, &iGateOpenness, 0, 12);
								}
								if (InArea (415, 69, 415 + 13, 69 + 20) == 1) /*** 10 ***/
								{
									if (iGateStateCheck < 10) { iGateStateCheck+=10; }
									ChangeCustom (10, &iGateOpenness, 0, 12);
								}
								if (InArea (372, 95, 372 + 13, 95 + 20) == 1) /*** -10 ***/
								{
									if (iGateStateCheck < 10) { iGateStateCheck+=10; }
									ChangeCustom (-10, &iGateDelay, 0, 255);
								}
								if (InArea (387, 95, 387 + 13, 95 + 20) == 1) /*** -1 ***/
								{
									if (iGateStateCheck < 10) { iGateStateCheck+=10; }
									ChangeCustom (-1, &iGateDelay, 0, 255);
								}
								if (InArea (457, 95, 457 + 13, 95 + 20) == 1) /*** 1 ***/
								{
									if (iGateStateCheck < 10) { iGateStateCheck+=10; }
									ChangeCustom (1, &iGateDelay, 0, 255);
								}
								if (InArea (472, 95, 472 + 13, 95 + 20) == 1) /*** 10 ***/
								{
									if (iGateStateCheck < 10) { iGateStateCheck+=10; }
									ChangeCustom (10, &iGateDelay, 0, 255);
								}
								break;
							case 0x09: /*** potion ***/
								SetCheck ("potioncolor", 35, 72, 10, iPotionColor);
								SetCheck ("potioncolor", 35, 98, 1, 1); /*** red ***/
								SetCheck ("potioncolor", 95, 98, 3, 3); /*** green ***/
								if (InArea (113, 69, 113 + 13, 69 + 20) == 1) /*** -10 ***/
								{
									iPotionColorCheck = 10;
									ChangeCustom (-10, &iPotionColor, 0x00, 0xFF);
								}
								if (InArea (128, 69, 128 + 13, 69 + 20) == 1) /*** -1 ***/
								{
									iPotionColorCheck = 10;
									ChangeCustom (-1, &iPotionColor, 0x00, 0xFF);
								}
								if (InArea (198, 69, 198 + 13, 69 + 20) == 1) /*** 1 ***/
								{
									iPotionColorCheck = 10;
									ChangeCustom (1, &iPotionColor, 0x00, 0xFF);
								}
								if (InArea (213, 69, 213 + 13, 69 + 20) == 1) /*** 10 ***/
								{
									iPotionColorCheck = 10;
									ChangeCustom (10, &iPotionColor, 0x00, 0xFF);
								}
								SetCheck ("potioneffect", 415, 72, 10, iPotionEffect);
								SetCheck ("potioneffect", 244, 72, 0, 0); /*** heal ***/
								SetCheck ("potioneffect", 301, 72, 1, 1); /*** life ***/
								SetCheck ("potioneffect", 358, 72, 2, 2); /*** hurt ***/
								SetCheck ("potioneffect", 244, 98, 3, 3); /*** float ***/
								if (iEXEType == 2) /*** EU ***/
								{
									SetCheck ("potioneffect", 301, 98, 4, 4); /*** loose ***/
									SetCheck ("potioneffect", 358, 98, 5, 5); /*** stop ***/
									SetCheck ("potioneffect", 415, 98, 6, 6); /*** time ***/
									SetCheck ("potioneffect", 472, 98, 7, 7); /*** time ***/
									SetCheck ("potioneffect", 529, 98, 8, 8); /*** time ***/
									SetCheck ("potioneffect", 586, 98, 9, 9); /*** time ***/
								}
								if (InArea (531, 69, 531 + 13, 69 + 20) == 1) /*** -10 ***/
								{
									iPotionEffectCheck = 10;
									ChangeCustom (-10, &iPotionEffect, 0x00, 0xFF);
								}
								if (InArea (546, 69, 546 + 13, 69 + 20) == 1) /*** -1 ***/
								{
									iPotionEffectCheck = 10;
									ChangeCustom (-1, &iPotionEffect, 0x00, 0xFF);
								}
								if (InArea (616, 69, 616 + 13, 69 + 20) == 1) /*** 1 ***/
								{
									iPotionEffectCheck = 10;
									ChangeCustom (1, &iPotionEffect, 0x00, 0xFF);
								}
								if (InArea (631, 69, 631 + 13, 69 + 20) == 1) /*** 10 ***/
								{
									iPotionEffectCheck = 10;
									ChangeCustom (10, &iPotionEffect, 0x00, 0xFF);
								}
								break;
							case 0x0A: /*** door ***/
								SetCheck ("doortype", 35, 72, 0, 0); /*** entrance ***/
								SetCheck ("doortype", 35, 98, 2, 2); /*** exit ***/
								break;
						}
					}

					if (event.button.button == 2)
					{
						if ((iUseTile == 1) && (iOnTile != 0) && (iOnTile <= 30))
						{
							for (iXLoop = 1; iXLoop <= WIDTH; iXLoop++)
							{
								for (iYLoop = 1; iYLoop <= HEIGHT; iYLoop++)
								{
									UseTile (iOnTile, iXLoop + ((iCurX - 1) * WIDTH),
										iYLoop + ((iCurY - 1) * HEIGHT));
								}
							}
							iChanging = 0;
							iChanged++;
						}
					}

					if (event.button.button == 3)
					{
						if ((iUseTile == 1) && (iOnTile != 0) && (iOnTile <= 30))
						{
							for (iXLoop = 1; iXLoop <= arLevelWidth[iCurLevel]
								* WIDTH; iXLoop++)
							{
								for (iYLoop = 1; iYLoop <= arLevelHeight[iCurLevel]
									* HEIGHT; iYLoop++)
								{
									UseTile (iOnTile, iXLoop, iYLoop);
								}
							}
							iChanging = 0;
							iChanged++;
						}
					}

					ShowChange();
					break;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_EXPOSED)
						{ ShowChange(); } break;
				case SDL_QUIT:
					Quit(); break;
			}
		}

		/*** prevent CPU eating ***/
		gamespeed = REFRESH;
		while ((SDL_GetTicks() - looptime) < gamespeed)
		{
			SDL_Delay (10);
		}
		looptime = SDL_GetTicks();
	}
	PlaySound ("wav/ok_close.wav");
}
/*****************************************************************************/
void ShowImage (SDL_Texture *img, int iX, int iY, char *sImageInfo)
/*****************************************************************************/
{
	SDL_Rect dest;
	SDL_Rect loc;
	int iWidth, iHeight;
	int iInfoC;
	char arText[9 + 2][MAX_TEXT + 2];
	int iTileValue;
	char sTileValue[MAX_OPTION + 2];
	int iNoGraphics, iNoObject;

	iNoGraphics = 0;
	iNoObject = 0;
	iInfoC = 0;

	/*** Graphics ***/
	if (strncmp (sImageInfo, "gra1=", 5) == 0)
	{
		GetOptionValue (sImageInfo, sTileValue);
		iTileValue = atoi (sTileValue);
		snprintf (arText[0], MAX_TEXT, "G:0x%02X", iTileValue);
		snprintf (arText[1], MAX_TEXT, "%s", "");
		if (imgd[iTileValue][1] == NULL) { iInfoC = 1; } /*** Custom tile. ***/
	} else if (strncmp (sImageInfo, "gra2=", 5) == 0) {
		GetOptionValue (sImageInfo, sTileValue);
		iTileValue = atoi (sTileValue);
	} else {
		iNoGraphics = 1;
	}

	/*** Object ***/
	if (strncmp (sImageInfo, "obj1=", 5) == 0)
	{
		GetOptionValue (sImageInfo, sTileValue);
		iTileValue = atoi (sTileValue);
		snprintf (arText[0], MAX_TEXT, "%s", "");
		snprintf (arText[1], MAX_TEXT, "O:0x%02X", iTileValue);
		if (imgo[iTileValue][1] == NULL) { iInfoC = 1; } /*** Custom tile. ***/
	} else if (strncmp (sImageInfo, "obj2=", 5) == 0) {
		GetOptionValue (sImageInfo, sTileValue);
		iTileValue = atoi (sTileValue);
	} else {
		iNoObject = 1;
	}

	SDL_QueryTexture (img, NULL, NULL, &iWidth, &iHeight);
	loc.x = 0;
	loc.y = 0;
	loc.w = iWidth;
	loc.h = iHeight;
	dest.x = iX;
	dest.y = iY;
	dest.w = iWidth;
	dest.h = iHeight;
	CustomRenderCopy (img, &loc, &dest, sImageInfo);

	/*** Info ("i"). ***/
	if (((iNoGraphics != 1) || (iNoObject != 1)) &&
		((iInfo == 1) || (iInfoC == 1)))
	{
		DisplayText (dest.x, dest.y + 108,
			FONT_SIZE_11, arText, 2, font2, color_wh, color_bl);
	}
}
/*****************************************************************************/
void CustomRenderCopy (SDL_Texture* src, SDL_Rect* srcrect,
	SDL_Rect *dstrect, char *sImageInfo)
/*****************************************************************************/
{
	SDL_Rect stuff;

	stuff.x = dstrect->x * iScale;
	stuff.y = dstrect->y * iScale;
	if (srcrect != NULL) /*** image ***/
	{
		stuff.w = dstrect->w * iScale;
		stuff.h = dstrect->h * iScale;
	} else { /*** font ***/
		stuff.w = dstrect->w;
		stuff.h = dstrect->h;
	}
	if (SDL_RenderCopy (ascreen, src, srcrect, &stuff) != 0)
	{
		printf ("[ WARN ] SDL_RenderCopy (%s): %s!\n",
			sImageInfo, SDL_GetError());
	}
}
/*****************************************************************************/
void CreateBAK (void)
/*****************************************************************************/
{
	FILE *fDAT;
	FILE *fBAK;
	int iData;

	fDAT = fopen (sPathFile, "rb");
	if (fDAT == NULL)
		{ printf ("[FAILED] Could not open %s: %s!\n",
			sPathFile, strerror (errno)); }

	fBAK = fopen (BACKUP, "wb");
	if (fBAK == NULL)
		{ printf ("[FAILED] Could not open %s: %s!\n",
			BACKUP, strerror (errno)); }

	while (1)
	{
		iData = fgetc (fDAT);
		if (iData == EOF) { break; }
			else { putc (iData, fBAK); }
	}

	fclose (fDAT);
	fclose (fBAK);
}
/*****************************************************************************/
void DisplayText (int iStartX, int iStartY, int iFontSize,
	char arText[9 + 2][MAX_TEXT + 2], int iLines, TTF_Font *font,
	SDL_Color back, SDL_Color fore)
/*****************************************************************************/
{
	int iTemp;

	for (iTemp = 0; iTemp <= (iLines - 1); iTemp++)
	{
		if (strcmp (arText[iTemp], "") != 0)
		{
			message = TTF_RenderText_Shaded (font,
				arText[iTemp], fore, back);
			messaget = SDL_CreateTextureFromSurface (ascreen, message);
			if ((strcmp (arText[iTemp], "single tile (change or select)") == 0) ||
				(strcmp (arText[iTemp], "entire room (clear or fill)") == 0) ||
				(strcmp (arText[iTemp], "entire level (randomize or fill)") == 0))
			{
				offset.x = iStartX + 20;
			} else {
				offset.x = iStartX;
			}
			offset.y = iStartY + (iTemp * (iFontSize + 4));
			offset.w = message->w; offset.h = message->h;
			CustomRenderCopy (messaget, NULL, &offset, "message");
			SDL_DestroyTexture (messaget); SDL_FreeSurface (message);
		}
	}
}
/*****************************************************************************/
void ShowRooms (void)
/*****************************************************************************/
{
	/*** Used for looping. ***/
	int iWidthLoop;
	int iHeightLoop;

	iStartRoomsX = floor (12 - ((float)arLevelWidth[iCurLevel] / 2));
	iStartRoomsY = floor (12 - ((float)arLevelHeight[iCurLevel] / 2));

	for (iWidthLoop = 1; iWidthLoop <= arLevelWidth[iCurLevel]; iWidthLoop++)
	{
		for (iHeightLoop = 1; iHeightLoop <= arLevelHeight[iCurLevel];
			iHeightLoop++)
		{
			ShowRoom (iWidthLoop, iHeightLoop);
		}
	}
}
/*****************************************************************************/
void ShowRoom (int iX, int iY)
/*****************************************************************************/
{
	int iXBase, iYBase;
	int iXLoop, iYLoop;
	unsigned char cObject;

	iXBase = 144 + ((iStartRoomsX + iX - 1) * ((10 * 2) + 1));
	iYBase = 111 + ((iStartRoomsY + iY - 1) * ((3 * 4) + 1));
	for (iXLoop = 1; iXLoop <= WIDTH; iXLoop++)
	{
		for (iYLoop = 1; iYLoop <= HEIGHT; iYLoop++)
		{
			cObject = arLevelObjects[iCurLevel][((iX - 1) * WIDTH) + iXLoop]
				[((iY - 1) * HEIGHT) + iYLoop];
			if (cObject > 0x0B) { cObject = 0x0C; }
			ShowImage (imgm[cObject], iXBase + ((iXLoop - 1) * 2),
				iYBase + ((iYLoop - 1) * 4), "imgm[...]");
		}
	}

	/*** prince room ***/
	if ((iX == arLevelStartingX[iCurLevel]) &&
		(iY == arLevelStartingY[iCurLevel]))
		{ ShowImage (imgsrs, iXBase, iYBase, "imgsrs"); }

	/*** current room ***/
	if ((iX == iCurX) && (iY == iCurY))
		{ ShowImage (imgsrc, iXBase, iYBase, "imgsrc"); }
}
/*****************************************************************************/
void ShowChange (void)
/*****************************************************************************/
{
	int iX, iY;
	int iGuardX;

	/*** Used for looping. ***/
	int iGuardLoop;

	/*** Three preview graphics. ***/
	switch (arLevelType[iCurLevel])
	{
		case 0:
			ShowImage (imgd[iNewGraphicsLeft][1],
				(0 * DD_X) + 6, 333, "imgd[...][1]");
			ShowImage (imgd[iNewGraphics][1],
				(1 * DD_X) + 6, 333, "imgd[...][1]");
			ShowImage (imgd[iNewGraphicsRight][1],
				(2 * DD_X) + 6, 333, "imgd[...][1]");
			break;
		case 1:
			ShowImage (imgp[iNewGraphicsLeft][1],
				(0 * DD_X) + 6, 333, "imgp[...][1]");
			ShowImage (imgp[iNewGraphics][1],
				(1 * DD_X) + 6, 333, "imgp[...][1]");
			ShowImage (imgp[iNewGraphicsRight][1],
				(2 * DD_X) + 6, 333, "imgp[...][1]");
			break;
	}

	/*** Three preview objects. ***/
	ShowImage (imgo[iNewObjectLeft][1], (0 * DD_X) + 6, 301, "imgo[...][1]");
	ShowImage (imgo[iNewObject][1], (1 * DD_X) + 6, 301, "imgo[...][1]");
	ShowImage (imgo[iNewObjectRight][1], (2 * DD_X) + 6, 301, "imgo[...][1]");

	/*** background ***/
	ShowImage (imgtiles, 0, 0, "imgtiles");

	/*** Object tab. ***/
	ShowImage (imgtabo[iTabObject], 24, 4, "imgtabo[...]");

	/*** Object settings. ***/
	iX = 0; iY = 0; /*** To prevent warnings. ***/
	switch (iNewObject)
	{
		case 0x03: /*** raise ***/
			/*========== 1 ==========*/
			switch (iRaiseGate1Check)
			{
				case 0xFFFD: iX = 35; iY = 98; break; /*** none ***/
				case 0xFFFE: iX = 95; iY = 98; break; /*** mirror ***/
				case 0xFFFF: iX = 155; iY = 98; break; /*** exit ***/
				case -1: iX = 35; iY = 72; break; /*** gate no ***/
			}
			ShowImage (imgchkbl, iX, iY, "imgchkbl");
			CenterNumber (iRaiseGate1, 141, 69, color_bl, 0);
			/*========== 2 ==========*/
			switch (iRaiseGate2Check)
			{
				case 0xFFFD: iX = 244; iY = 98; break; /*** none ***/
				case 0xFFFE: iX = 304; iY = 98; break; /*** mirror ***/
				case 0xFFFF: iX = 364; iY = 98; break; /*** exit ***/
				case -1: iX = 244; iY = 72; break; /*** gate no ***/
			}
			ShowImage (imgchkbl, iX, iY, "imgchkbl");
			CenterNumber (iRaiseGate2, 350, 69, color_bl, 0);
			/*========== 3 ==========*/
			switch (iRaiseGate3Check)
			{
				case 0xFFFD: iX = 453; iY = 98; break; /*** none ***/
				case 0xFFFE: iX = 513; iY = 98; break; /*** mirror ***/
				case 0xFFFF: iX = 573; iY = 98; break; /*** exit ***/
				case -1: iX = 453; iY = 72; break; /*** gate no ***/
			}
			ShowImage (imgchkbl, iX, iY, "imgchkbl");
			CenterNumber (iRaiseGate3, 559, 69, color_bl, 0);
			break;
		case 0x04: /*** drop ***/
			/*========== 1 ==========*/
			switch (iDropGate1Check)
			{
				case 0xFFFD: iX = 35; iY = 98; break; /*** none ***/
				case 0xFFFE: iX = 95; iY = 98; break; /*** mirror ***/
				case 0xFFFF: iX = 155; iY = 98; break; /*** exit ***/
				case -1: iX = 35; iY = 72; break; /*** gate no ***/
			}
			ShowImage (imgchkbl, iX, iY, "imgchkbl");
			CenterNumber (iDropGate1, 141, 69, color_bl, 0);
			/*========== 2 ==========*/
			switch (iDropGate2Check)
			{
				case 0xFFFD: iX = 244; iY = 98; break; /*** none ***/
				case 0xFFFE: iX = 304; iY = 98; break; /*** mirror ***/
				case 0xFFFF: iX = 364; iY = 98; break; /*** exit ***/
				case -1: iX = 244; iY = 72; break; /*** gate no ***/
			}
			ShowImage (imgchkbl, iX, iY, "imgchkbl");
			CenterNumber (iDropGate2, 350, 69, color_bl, 0);
			/*========== 3 ==========*/
			switch (iDropGate3Check)
			{
				case 0xFFFD: iX = 453; iY = 98; break; /*** none ***/
				case 0xFFFE: iX = 513; iY = 98; break; /*** mirror ***/
				case 0xFFFF: iX = 573; iY = 98; break; /*** exit ***/
				case -1: iX = 453; iY = 72; break; /*** gate no ***/
			}
			ShowImage (imgchkbl, iX, iY, "imgchkbl");
			CenterNumber (iDropGate3, 559, 69, color_bl, 0);
			break;
		case 0x05: /*** gate ***/
			switch (iGateStateCheck)
			{
				case 0: iX = 35; iY = 72; break; /*** closed ***/
				case 10: iX = 54; iY = 72; break; /*** closed, custom ***/
				case 7: iX = 121; iY = 72; break; /*** close on start ***/
				case 17: iX = 140; iY = 72; break; /*** close on start, custom ***/
				case 3: iX = 35; iY = 98; break; /*** open ***/
				case 13: iX = 54; iY = 98; break; /*** open, custom ***/
				case 1: iX = 121; iY = 98; break; /*** open on start ***/
				case 11: iX = 140; iY = 98; break; /*** open on start, custom ***/
				default:
					printf ("[ WARN ] Unknown gate state check: %i!\n",
						iGateStateCheck); break;
			}
			ShowImage (imgchkbl, iX, iY, "imgchkbl");
			CenterNumber (iGateOpenness, 343, 69, color_bl, 0);
			CenterNumber (iGateDelay, 400, 95, color_bl, 0);
			break;
		case 0x09: /*** potion ***/
			if (iEXEType != 2) /*** If not EU... ***/
			{
				ShowImage (imgeuonly, 301, 98, "imgeuonly"); /*** loose ***/
				ShowImage (imgeuonly, 358, 98, "imgeuonly"); /*** stop ***/
				ShowImage (imgeuonly, 415, 98, "imgeuonly"); /*** time ***/
				ShowImage (imgeuonly, 472, 98, "imgeuonly"); /*** time ***/
				ShowImage (imgeuonly, 529, 98, "imgeuonly"); /*** time ***/
				ShowImage (imgeuonly, 586, 98, "imgeuonly"); /*** time ***/
			}
			switch (iPotionColorCheck)
			{
				case 1: iX = 35; iY = 98; break; /*** red ***/
				case 3: iX = 95; iY = 98; break; /*** green ***/
				case 10: iX = 35; iY = 72; break; /*** color no ***/
			}
			ShowImage (imgchkbl, iX, iY, "imgchkbl");
			CenterNumber (iPotionColor, 141, 69, color_bl, 0);
			switch (iPotionEffectCheck)
			{
				case 0: iX = 244; iY = 72; break; /*** heal ***/
				case 1: iX = 301; iY = 72; break; /*** life ***/
				case 2: iX = 358; iY = 72; break; /*** hurt ***/
				case 3: iX = 244; iY = 98; break; /*** float ***/
				case 4: iX = 301; iY = 98; break; /*** loose ***/
				case 5: iX = 358; iY = 98; break; /*** stop ***/
				case 6: iX = 415; iY = 98; break; /*** time ***/
				case 7: iX = 472; iY = 98; break; /*** time ***/
				case 8: iX = 529; iY = 98; break; /*** time ***/
				case 9: iX = 586; iY = 98; break; /*** time ***/
				case 10: iX = 415; iY = 72; break; /*** effect no ***/
			}
			ShowImage (imgchkbl, iX, iY, "imgchkbl");
			CenterNumber (iPotionEffect, 559, 69, color_bl, 0);
			break;
		case 0x0A: /*** door ***/
			switch (iDoorTypeCheck)
			{
				case 0: iX = 35; iY = 72; break; /*** entrance ***/
				case 2: iX = 35; iY = 98; break; /*** exit ***/
			}
			ShowImage (imgchkbl, iX, iY, "imgchkbl");
			break;
	}

	/*** Graphics tab. ***/
	switch (arLevelType[iCurLevel])
	{
		case 0:
			ShowImage (imgtabgd[iTabGraphics], 8, 125, "imgtabgd[...]");
			break;
		case 1:
			ShowImage (imgtabgp[iTabGraphics], 8, 125, "imgtabgp[...]");
			break;
	}

	/*** close button ***/
	switch (iCloseOn)
	{
		case 0: /*** off ***/
			ShowImage (imgclosebig_0, 656, 0, "imgclosebig_0"); break;
		case 1: /*** on ***/
			ShowImage (imgclosebig_1, 656, 0, "imgclosebig_1"); break;
	}

	/*** old graphics ***/
	if (iTabGraphics == iOldGraphicsTab)
	{
		ShowImage (imgborderbl, 10 + ((iOldGraphicsX - 1) * 34),
			193 + ((iOldGraphicsY - 1) * 66), "imgborderbl");
	}

	/*** prince, turned right ***/
	if ((arPrinceDir[iCurLevel] == 0x800) &&
		(arPrinceX[iCurLevel] == ((iCurX - 1) * WIDTH) + iSelectedX) &&
		(arPrinceY[iCurLevel] == ((iCurY - 1) * HEIGHT) + iSelectedY) &&
		(arLevelStartingX[iCurLevel] == iCurX) &&
		(arLevelStartingY[iCurLevel] == iCurY))
		{ ShowImage (imgbordersl, 204, 396, "imgbordersl"); }
	/*** prince, turned left ***/
	if ((arPrinceDir[iCurLevel] == 0x00) &&
		(arPrinceX[iCurLevel] == ((iCurX - 1) * WIDTH) + iSelectedX) &&
		(arPrinceY[iCurLevel] == ((iCurY - 1) * HEIGHT) + iSelectedY) &&
		(arLevelStartingX[iCurLevel] == iCurX) &&
		(arLevelStartingY[iCurLevel] == iCurY))
		{ ShowImage (imgbordersl, 236, 396, "imgbordersl"); }

	/*** guard ***/
	for (iGuardLoop = 1; iGuardLoop <= arNrGuards[iCurLevel]; iGuardLoop++)
	{
		if ((arGuardX[iCurLevel][iGuardLoop] ==
			((iCurX - 1) * WIDTH) + iSelectedX) &&
			(arGuardY[iCurLevel][iGuardLoop] ==
			((iCurY - 1) * HEIGHT) + iSelectedY))
		{
			iGuardX = 204; /*** To prevent the "uninitialized" warning. ***/
			if (arGuardDir[iCurLevel][iGuardLoop] == 0x800) /*** right ***/
			{
				switch (arGuardType[iCurLevel][iGuardLoop])
				{
					case 0: iGuardX = 204; break; /*** guard ***/
					case 1: iGuardX = 268; break; /*** skeleton ***/
					case 2: iGuardX = 332; break; /*** fat ***/
					case 3: iGuardX = 396; break; /*** shadow ***/
					case 4: iGuardX = 460; break; /*** Jaffar ***/
				}
			}
			if (arGuardDir[iCurLevel][iGuardLoop] == 0x00) /*** left ***/
			{
				switch (arGuardType[iCurLevel][iGuardLoop])
				{
					case 0: iGuardX = 236; break; /*** guard ***/
					case 1: iGuardX = 300; break; /*** skeleton ***/
					case 2: iGuardX = 364; break; /*** fat ***/
					case 3: iGuardX = 428; break; /*** shadow ***/
					case 4: iGuardX = 492; break; /*** Jaffar ***/
				}
			}
			ShowImage (imgbordersl, iGuardX, 329, "imgbordersl");
		}
	}

	/*** selected (new) tile ***/
	if ((iOnTile != 0) && (IsDisabled (iOnTile) == 0))
	{
		iX = -1; iY = -1; /*** To prevent warnings. ***/

		/*** Row 1. ***/
		if ((iOnTile >= 1) && (iOnTile <= 15))
		{
			iX = 10 + ((iOnTile - 1) * 34);
			iY = 193;
		}
		/*** Row 2. ***/
		if ((iOnTile >= 16) && (iOnTile <= 30))
		{
			iX = 10 + ((iOnTile - 16) * 34);
			iY = 259;
		}
		/*** Row 3. ***/
		if ((iOnTile >= 31) && (iOnTile <= 40))
		{
			iX = 204 + ((iOnTile - 31) * 32);
			iY = 329;
		}
		/*** Row 4. ***/
		if (iOnTile == 41) { iX = 204; iY = 396; }
		if (iOnTile == 42) { iX = 236; iY = 396; }

		if (iOnTile <= 30) /*** graphics ***/
		{
			ShowImage (imgborderb, iX, iY, "imgborderb");
		} else { /*** living ***/
			ShowImage (imgborders, iX, iY, "imgborders");
		}
	}

	/*** Object number. ***/
	CenterNumber (iNewObject, 304, 437, color_bl, 1);

	/*** Graphics number. ***/
	CenterNumber (iNewGraphics, 433, 437, color_bl, 1);

	/*** Skill. ***/
	CenterNumber (iNewSkill, 560, 393, color_bl, 0);

	/*** Hit points. ***/
	CenterNumber (iNewHP, 560, 437, color_bl, 0);

	if (iCustomHover == 1)
		{ ShowImage (imgchover, 271, 399, "imgchover"); }

	/*** refresh screen ***/
	SDL_RenderPresent (ascreen);
}
/*****************************************************************************/
int OnTile (void)
/*****************************************************************************/
{
	/*** Used for looping. ***/
	int iTileLoop;

	/*** Row 1. ***/
	for (iTileLoop = 1; iTileLoop <= 15; iTileLoop++)
	{
		if (InArea (10 + ((iTileLoop - 1) * 34), 193,
			10 + ((iTileLoop - 1) * 34) + 36, 193 + 68) == 1)
			{ return (iTileLoop); }
	}
	/*** Row 2. ***/
	for (iTileLoop = 16; iTileLoop <= 30; iTileLoop++)
	{
		if (InArea (10 + ((iTileLoop - 16) * 34), 259,
			10 + ((iTileLoop - 16) * 34) + 36, 259 + 68) == 1)
			{ return (iTileLoop); }
	}
	/*** Row 3. ***/
	for (iTileLoop = 31; iTileLoop <= 40; iTileLoop++)
	{
		if (InArea (204 + ((iTileLoop - 31) * 32), 329,
			204 + ((iTileLoop - 31) * 32) + 34, 329 + 69) == 1)
			{ return (iTileLoop); }
	}
	/*** Row 4. ***/
	if (InArea (204, 396, 204 + 34, 396 + 69) == 1) { return (41); }
	if (InArea (236, 396, 236 + 34, 396 + 69) == 1) { return (42); }

	return (0);
}
/*****************************************************************************/
void ChangePosAction (char *sAction)
/*****************************************************************************/
{
	if (strcmp (sAction, "left") == 0)
	{
		do {
			switch (iOnTile)
			{
				case 1: iOnTile = 15; break;
				case 16: iOnTile = 30; break;
				case 31: iOnTile = 40; break;
				case 41: iOnTile = 42; break;
				default: iOnTile--; break;
			}
		} while (IsDisabled (iOnTile) == 1);
		iNewGraphics = OnGraphics();
	}

	if (strcmp (sAction, "right") == 0)
	{
		do {
			switch (iOnTile)
			{
				case 15: iOnTile = 1; break;
				case 30: iOnTile = 16; break;
				case 40: iOnTile = 31; break;
				case 42: iOnTile = 41; break;
				default: iOnTile++; break;
			}
		} while (IsDisabled (iOnTile) == 1);
		iNewGraphics = OnGraphics();
	}

	if (strcmp (sAction, "up") == 0)
	{
		do {
			switch (iOnTile)
			{
				/*** Row 1. ***/
				case 1: iOnTile = 16; break;
				case 2: iOnTile = 17; break;
				case 3: iOnTile = 18; break;
				case 4: iOnTile = 19; break;
				case 5: iOnTile = 20; break;
				case 6: iOnTile = 21; break;
				case 7: iOnTile = 41; break;
				case 8: iOnTile = 42; break;
				case 9: iOnTile = 33; break;
				case 10: iOnTile = 34; break;
				case 11: iOnTile = 36; break; /*** Sic. ***/
				case 12: iOnTile = 37; break;
				case 13: iOnTile = 38; break;
				case 14: iOnTile = 39; break;
				case 15: iOnTile = 40; break;
				/*** Row 3. ***/
				case 31: iOnTile = 22; break;
				case 32: iOnTile = 23; break;
				case 33: iOnTile = 24; break;
				case 34: iOnTile = 24; break; /*** Sic. ***/
				case 35: iOnTile = 25; break;
				case 36: iOnTile = 26; break;
				case 37: iOnTile = 27; break;
				case 38: iOnTile = 28; break;
				case 39: iOnTile = 29; break;
				case 40: iOnTile = 30; break;
				/*** Row 4. ***/
				case 41: iOnTile = 31; break;
				case 42: iOnTile = 32; break;
				/*** Rest. ***/
				default: iOnTile-=15; break;
			}
		} while (IsDisabled (iOnTile) == 1);
		iNewGraphics = OnGraphics();
	}

	if (strcmp (sAction, "down") == 0)
	{
		do {
			switch (iOnTile)
			{
				/*** Row 2. ***/
				case 16: iOnTile = 1; break;
				case 17: iOnTile = 2; break;
				case 18: iOnTile = 3; break;
				case 19: iOnTile = 4; break;
				case 20: iOnTile = 5; break;
				case 21: iOnTile = 6; break;
				case 22: iOnTile = 31; break;
				case 23: iOnTile = 32; break;
				case 24: iOnTile = 33; break;
				case 25: iOnTile = 35; break; /*** Sic. ***/
				case 26: iOnTile = 36; break;
				case 27: iOnTile = 37; break;
				case 28: iOnTile = 38; break;
				case 29: iOnTile = 39; break;
				case 30: iOnTile = 40; break;
				/*** Row 3. ***/
				case 31: iOnTile = 41; break;
				case 32: iOnTile = 42; break;
				case 33: iOnTile = 9; break;
				case 34: iOnTile = 10; break;
				case 35: iOnTile = 10; break; /*** Sic. ***/
				case 36: iOnTile = 11; break;
				case 37: iOnTile = 12; break;
				case 38: iOnTile = 13; break;
				case 39: iOnTile = 14; break;
				case 40: iOnTile = 15; break;
				/*** Row 4. ***/
				case 41: iOnTile = 7; break;
				case 42: iOnTile = 8; break;
				/*** Rest. ***/
				default: iOnTile+=15; break;
			}
		} while (IsDisabled (iOnTile) == 1);
		iNewGraphics = OnGraphics();
	}
}
/*****************************************************************************/
int IsDisabled (int iTile)
/*****************************************************************************/
{
	/* With this port, none of the guards are disabled. Thus, a DisableSome()
	 * function is also not being used.
	 */

	if (Unused (iTile) == 1) { return (1); }

	return (0);
}
/*****************************************************************************/
void CenterNumber (int iNumber, int iX, int iY,
	SDL_Color fore, int iHex)
/*****************************************************************************/
{
	char sText[MAX_TEXT + 2];

	if (iHex == 0)
	{
		if (iNumber <= 999)
		{
			snprintf (sText, MAX_TEXT, "%i", iNumber);
		} else {
			snprintf (sText, MAX_TEXT, "%s", "X");
		}
	} else {
		snprintf (sText, MAX_TEXT, "%02X", iNumber);
	}
	/* The 100000 is a workaround for 0 being broken. SDL devs have fixed that
	 * see e.g. https://hg.libsdl.org/SDL_ttf/rev/72b8861dbc01 but
	 * Ubuntu et al. still ship older sdl2-ttf versions.
	 */
	message = TTF_RenderText_Blended_Wrapped (font3, sText, fore, 100000);
	messaget = SDL_CreateTextureFromSurface (ascreen, message);
	if (iHex == 0)
	{
		if ((iNumber >= -9) && (iNumber <= -1))
		{
			offset.x = iX + 16;
		} else if ((iNumber >= 0) && (iNumber <= 9)) {
			offset.x = iX + 21;
		} else if ((iNumber >= 10) && (iNumber <= 99)) {
			offset.x = iX + 14;
		} else if ((iNumber >= 100) && (iNumber <= 999)) {
			offset.x = iX + 7;
		} else {
			offset.x = iX + 21;
		}
	} else {
		offset.x = iX + 14;
	}
	offset.y = iY - 1;
	offset.w = message->w; offset.h = message->h;
	CustomRenderCopy (messaget, NULL, &offset, "message");
	SDL_DestroyTexture (messaget); SDL_FreeSurface (message);
}
/*****************************************************************************/
int Unused (int iTile)
/*****************************************************************************/
{
	/* Returns 1 for unused tile spots. Depends on the active tab and the
	 * environment type.
	 */

	int iUseRow;
	int iUseTile;

	if (iTile <= 15)
	{
		iUseRow = 1;
		iUseTile = iTile;
	} else {
		iUseRow = 2;
		iUseTile = iTile - 15;
	}

	if (EnvTabRow[arLevelType[iCurLevel]][iTabGraphics][iUseRow][iUseTile] == -1)
	{
		return (1);
	}

	return (0);
}
/*****************************************************************************/
void OpenURL (char *sURL)
/*****************************************************************************/
{
#if defined WIN32 || _WIN32 || WIN64 || _WIN64
ShellExecute (NULL, "open", sURL, NULL, NULL, SW_SHOWNORMAL);
#else
pid_t pid;
pid = fork();
if (pid == 0)
{
	execl ("/usr/bin/xdg-open", "xdg-open", sURL, (char *)NULL);
	exit (EXIT_NORMAL);
}
#endif
}
/*****************************************************************************/
void EXELoad (void)
/*****************************************************************************/
{
	int iFdEXE;
	unsigned char sRead[2 + 2];
	char sReadDW[10 + 2];
	int iOffsetTime, iOffsetHP, iOffsetLevel, iFPS;
	int iEXEStartingTime;

	iFdEXE = open (sPathFile, O_RDONLY|O_BINARY);
	if (iFdEXE == -1)
	{
		printf ("[FAILED] Error opening %s: %s!\n",
			sPathFile, strerror (errno));
		exit (EXIT_ERROR);
	}

	switch (iEXEType)
	{
		case 1: /*** US ***/
			iOffsetTime = 0x1DF9C;
			iOffsetHP = 0x1DFAF;
			iOffsetLevel = 0x1DFA7;
			iFPS = 60;
			break;
		case 2: /*** EU ***/
			iOffsetTime = 0x4E29C;
			iOffsetHP = 0x4E2C1;
			iOffsetLevel = 0x4E2A7;
			iFPS = 50;
			break;
		default:
			printf ("[FAILED] Unknown iEXEType: %i!\n", iEXEType);
			exit (EXIT_ERROR); break;
	}

	/*** Starting time. ***/
	lseek (iFdEXE, iOffsetTime, SEEK_SET);
	read (iFdEXE, sRead, 4);
	snprintf (sReadDW, 10, "%02x%02x%02x%02x",
		sRead[0], sRead[1], sRead[2], sRead[3]);
	iEXEStartingTime = (strtoul (sReadDW, NULL, 16) + 1) / iFPS;
	iEXEStartingMin = iEXEStartingTime / 60;
	iEXEStartingSec = iEXEStartingTime % 60;

	/*** Starting HP. ***/
	lseek (iFdEXE, iOffsetHP, SEEK_SET);
	read (iFdEXE, sRead, 1);
	iEXEStartingHP = sRead[0];

	/*** Starting level. ***/
	lseek (iFdEXE, iOffsetLevel, SEEK_SET);
	read (iFdEXE, sRead, 1);
	iEXEStartingLevel = sRead[0] + 1;

	close (iFdEXE);
}
/*****************************************************************************/
void EXESave (void)
/*****************************************************************************/
{
	int iFdEXE;
	int iOffsetTime, iOffsetHP, iOffsetLevel, iFPS;
	int iEXEStartingTime;

	iFdEXE = open (sPathFile, O_RDWR|O_BINARY);
	if (iFdEXE == -1)
	{
		printf ("[FAILED] Error opening %s: %s!\n",
			sPathFile, strerror (errno));
		exit (EXIT_ERROR);
	}

	switch (iEXEType)
	{
		case 1: /*** US ***/
			iOffsetTime = 0x1DF9C;
			iOffsetHP = 0x1DFAF;
			iOffsetLevel = 0x1DFA7;
			iFPS = 60;
			break;
		case 2: /*** EU ***/
			iOffsetTime = 0x4E29C;
			iOffsetHP = 0x4E2C1;
			iOffsetLevel = 0x4E2A7;
			iFPS = 50;
			break;
		default:
			printf ("[FAILED] Unknown iEXEType: %i!\n", iEXEType);
			exit (EXIT_ERROR); break;
	}

	/*** Starting time. ***/
	lseek (iFdEXE, iOffsetTime, SEEK_SET);
	iEXEStartingTime = (((iEXEStartingMin * 60) + iEXEStartingSec) * iFPS) - 1;
	WriteDWord (iFdEXE, iEXEStartingTime);

	/*** Starting HP. ***/
	lseek (iFdEXE, iOffsetHP, SEEK_SET);
	WriteByte (iFdEXE, iEXEStartingHP);

	/*** Starting level. ***/
	lseek (iFdEXE, iOffsetLevel, SEEK_SET);
	WriteByte (iFdEXE, iEXEStartingLevel - 1);

	close (iFdEXE);

	PlaySound ("wav/save.wav");
}
/*****************************************************************************/
int PlusMinus (int *iWhat, int iX, int iY,
	int iMin, int iMax, int iChange, int iAddChanged)
/*****************************************************************************/
{
	if ((InArea (iX, iY, iX + 13, iY + 20) == 1) &&
		(((iChange < 0) && (*iWhat > iMin)) ||
		((iChange > 0) && (*iWhat < iMax))))
	{
		*iWhat = *iWhat + iChange;
		if ((iChange < 0) && (*iWhat < iMin)) { *iWhat = iMin; }
		if ((iChange > 0) && (*iWhat > iMax)) { *iWhat = iMax; }
		if (iAddChanged == 1) { iChanged++; }
		PlaySound ("wav/plus_minus.wav");
		return (1);
	} else { return (0); }
}
/*****************************************************************************/
void GetOptionValue (char *sArgv, char *sValue)
/*****************************************************************************/
{
	int iTemp;
	char sTemp[MAX_OPTION + 2];

	iTemp = strlen (sArgv) - 1;
	snprintf (sValue, MAX_OPTION, "%s", "");
	while (sArgv[iTemp] != '=')
	{
		snprintf (sTemp, MAX_OPTION, "%c%s", sArgv[iTemp], sValue);
		snprintf (sValue, MAX_OPTION, "%s", sTemp);
		iTemp--;
	}
}
/*****************************************************************************/
void ObjectWarn (int iHave, int iNeed)
/*****************************************************************************/
{
	if (iHave != iNeed)
	{
		printf ("[ WARN ] Unexpected object value for %i: %i!\n", iNeed, iHave);
	}
}
/*****************************************************************************/
void LoadingBar (int iBarHeight)
/*****************************************************************************/
{
	SDL_Rect bar;

	bar.x = (10 + 2) * iScale;
	bar.y = (447 + 10 - 2 - iBarHeight) * iScale;
	bar.w = (20 - 2 - 2) * iScale;
	bar.h = iBarHeight * iScale;
	SDL_SetRenderDrawColor (ascreen, 0x44, 0x44, 0x44, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect (ascreen, &bar);
	iCurrentBarHeight = iBarHeight;

	/*** refresh screen ***/
	SDL_RenderPresent (ascreen);
}
/*****************************************************************************/
const char* GateAsText (int iGate)
/*****************************************************************************/
{
	static char sReturn[MAX_TEXT + 2];

	switch (iGate)
	{
		case 65533: return ("X"); break;
		case 65534: return ("mirror"); break;
		case 65535: return ("exit"); break;
		default:
			snprintf (sReturn, MAX_TEXT, "%i", iGate);
			return (sReturn);
	}
}
/*****************************************************************************/
void SetEnvTabRow (int iEnv, int iTab, int iRow,
	int iG1, int iG2, int iG3, int iG4, int iG5, int iG6, int iG7, int iG8,
	int iG9, int iG10, int iG11, int iG12, int iG13, int iG14, int iG15)
/*****************************************************************************/
{
	EnvTabRow[iEnv][iTab][iRow][1] = iG1;
	EnvTabRow[iEnv][iTab][iRow][2] = iG2;
	EnvTabRow[iEnv][iTab][iRow][3] = iG3;
	EnvTabRow[iEnv][iTab][iRow][4] = iG4;
	EnvTabRow[iEnv][iTab][iRow][5] = iG5;
	EnvTabRow[iEnv][iTab][iRow][6] = iG6;
	EnvTabRow[iEnv][iTab][iRow][7] = iG7;
	EnvTabRow[iEnv][iTab][iRow][8] = iG8;
	EnvTabRow[iEnv][iTab][iRow][9] = iG9;
	EnvTabRow[iEnv][iTab][iRow][10] = iG10;
	EnvTabRow[iEnv][iTab][iRow][11] = iG11;
	EnvTabRow[iEnv][iTab][iRow][12] = iG12;
	EnvTabRow[iEnv][iTab][iRow][13] = iG13;
	EnvTabRow[iEnv][iTab][iRow][14] = iG14;
	EnvTabRow[iEnv][iTab][iRow][15] = iG15;
}
/*****************************************************************************/
void SetEnvTabRows (void)
/*****************************************************************************/
{
	/* In theory, a static const initialization could be used for this. However,
	 * with this multidimensional array, it would become an unreadable mess.
	 */

	/*** dungeon (0) ***/
	SetEnvTabRow (0, 1, 1, 0xea, 0xe8, 0xeb, 0xee, 0xe6,
		0xec, 0xe9, -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 1, 2, 0xe7, 0xed, 0xc3, 0xba, 0xc2,
		0xc0, 0xbe, 0xc1, 0xbf, -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 2, 1, 0xca, 0xde, 0xe1, 0xe4, 0xc8,
		0xdc, 0xdf, 0xe2, 0xe3, 0xdd, 0xc9, 0xe0, -1,   -1,   -1);
	SetEnvTabRow (0, 2, 2, 0xb8, 0xb0, 0xb9, 0xb5, 0xb4,
		0xb7, 0xcc, 0xcd, 0xb6, 0xcb, -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 3, 1, 0xda, 0xd7, 0xd4, 0xd0, 0xd9,
		0xd6, 0xd3, 0xd1, 0xcf, 0xce, 0xd2, 0xd8, 0xd5, 0xc4, -1);
	SetEnvTabRow (0, 3, 2, 0xae, 0xaf, 0xc5, 0xaa, 0xad,
		0xc6, 0xc7, 0xab, 0xac, -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 4, 1, 0x63, 0xbd, 0xb3, 0x65, 0x66,
		0xbc, 0xb2, 0x16, 0x17, 0x45, -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 4, 2, -1,   -1,   -1,   -1,   -1,
		-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 5, 1, 0x97, 0x9b, 0x99, 0x9d, 0x96,
		0x9a, 0x9c, 0x98, 0x20, 0x1f, 0x14, -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 5, 2, -1,   -1,   -1,   -1,   -1,
		-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 6, 1, 0x39, 0x7f, 0x37, 0x7d, 0x6b,
		0x5d, 0x4b, 0x3d, 0x4f, 0x6d, -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 6, 2, 0x64, 0x38, 0x7e, 0x7c, 0x36,
		0x6c, 0x4c, -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 7, 1, 0x26, 0x22, 0x04, 0x32, -1,
		0x4a, 0x49, -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 7, 2, 0x27, 0x30, 0x2d, 0x21, 0x03,
		0x2e, -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 8, 1, 0x93, 0x91, 0x8f, 0x8d, 0x5e,
		0x88, 0x87, 0x95, 0x19, 0x1a, -1,   0x35, 0x7b, -1,   -1);
	SetEnvTabRow (0, 8, 2, 0x92, 0x90, 0x94, 0x8c, 0x5f,
		0x86, 0x8e, 0x8a, 0x1b, 0x18, -1,   0x7a, 0x34, -1,   -1);
	SetEnvTabRow (0, 9, 1, 0x73, 0x53, 0x71, 0x75, 0x51,
		0x6f, 0x83, 0x8b, 0x80, 0x41, 0x74, 0x2f, 0x79, 0x85, -1);
	SetEnvTabRow (0, 9, 2, 0x52, 0x58, 0x76, 0x56, 0x72,
		0x6e, 0x89, 0x82, 0x81, 0x42, 0x77, 0x4e, 0x78, 0x84, -1);
	SetEnvTabRow (0, 10, 1, 0xa7, 0xa2, 0xa6, 0xa3, 0x24,
		0x23, 0x3e, -1,   0x9e, 0xa8, 0x25, -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 10, 2, 0xa5, 0xa0, 0xa1, 0xa4, 0x15,
		0x3c, -1,   0xa9, 0x9f, -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 11, 1, 0x68, 0x54, 0x69, -1,   0x5a,
		0x4d, 0x55, -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 11, 2, 0x67, 0x50, 0x61, -1,   0x5b,
		0x46, 0x57, 0x59, -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 12, 1, 0x00, 0xf0, 0x01, 0x02, 0x0a,
		0x13, -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 12, 2, 0x07, 0x1c, 0x08, 0x1d, 0x11,
		0x12, 0x09, -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 13, 1, 0x47, 0x48, 0x0b, 0x0c, 0x3a,
		0x3b, 0x33, 0xb1, 0xbb, 0x44, 0xdb, 0xe5, 0xef, -1,   -1);
	SetEnvTabRow (0, 13, 2, -1,   -1,   -1,   -1,   -1,
		-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 14, 1, 0x5c, 0x60, 0x6a, 0x70, 0x05,
		0x06, 0x0d, 0x0e, 0x0f, 0x10, -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 14, 2, 0x43, 0x40, 0x3f, 0x31, 0x62,
		0x28, 0x1e, 0x2a, 0x29, 0x2c, 0x2b, -1,   -1,   -1,   -1);
	SetEnvTabRow (0, 15, 1, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5,
		0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff);
	SetEnvTabRow (0, 15, 2, -1,   -1,   -1,   -1,   -1,
		-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);

	/*** palace (1) ***/
	SetEnvTabRow (1, 1, 1, 0x5a, 0x5c, 0x6e, 0x70, 0x5d,
		0x71, 0x5b, 0x6f, 0x67, 0x7b, -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 1, 2, 0x65, 0x79, 0x64, 0x78, 0x66,
		0x7a, -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 2, 1, 0x96, 0x97, 0x98, 0x99, 0xa0,
		0xa1, 0xa2, 0xa3, -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 2, 2, -1,   -1,   -1,   -1,   -1,
		-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 3, 1, 0x0d, 0x20, 0x50, 0x0e, 0x1e,
		0x1f, 0x24, 0x21, 0x25, 0x22, 0x23, 0xae, 0xaf, 0xa4, 0xa5);
	SetEnvTabRow (1, 3, 2, -1,   -1,   -1,   -1,   -1,
		-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 4, 1, 0x1b, 0x1c, 0x18, 0x14, 0x15,
		0x86, 0x87, 0x0c, 0x2e, 0x2f, 0x26, 0x27, -1,   -1,   -1);
	SetEnvTabRow (1, 4, 2, 0xb2, 0xb0, 0xb3, 0xb1, 0xc6,
		0xc4, 0xc7, 0xc5, 0x33, 0x32, -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 5, 1, 0x08, 0x09, 0x16, 0x17, -1,
		-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 5, 2, 0xc9, 0xbf, 0x9f, 0x63, 0xca,
		0xc0, 0xc8, 0x9e, 0xc1, 0xcb, 0xbe, 0x62, 0x3c, -1,   -1);
	SetEnvTabRow (1, 6, 1, 0x7c, 0x95, 0x7f, 0x94, 0x7e,
		0x8b, 0x7d, 0x8a, -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 6, 2, 0x51, 0x52, 0x53, 0x54, 0x36,
		0x34, 0x35, 0x37, -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 7, 1, 0x38, 0x39, 0xcd, 0xcc, 0xcf,
		0xce, 0x11, 0x12, 0x2a, 0x2b, -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 7, 2, 0xb7, 0xb6, 0xad, 0xac, 0xb5,
		0xb4, 0xab, 0xaa, -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 8, 1, 0x03, 0x1d, 0x2c, 0x2d, 0x13,
		0x28, 0x29, -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 8, 2, -1,   -1,   -1,   -1,   -1,
		-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 9, 1, 0x4d, 0x4e, 0xde, 0xdf, 0x48,
		0x49, 0xe0, 0xe1, 0xef, 0x8d, -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 9, 2, 0x5e, 0x47, 0x58, 0x4a, 0x3d,
		0xb9, 0x46, -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 10, 1, 0x4c, 0xe3, 0x4f, 0xe2, 0x3f,
		0x80, 0x40, 0x81, 0x5f, 0x60, 0x8c, 0x69, 0x6a, -1,   -1);
	SetEnvTabRow (1, 10, 2, 0x6c, 0x56, 0x6b, 0x55, 0x59,
		0x61, 0xdb, 0x75, 0x73, 0x76, 0x74, 0x4b, 0x88, 0x57, 0x89);
	SetEnvTabRow (1, 11, 1, 0x00, 0xd2, 0xd3, 0xd9, 0xdc,
		0xdd, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec);
	SetEnvTabRow (1, 11, 2, 0xed, -1,   0x0a, 0x01, 0xd6,
		0xd8, 0x0b, 0x02, 0xd5, 0xd7, -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 12, 1, 0xee, 0x68, 0x6d, 0x77, 0xd4,
		0x72, -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 12, 2, 0x8e, 0xa8, 0xa6, 0xa9, 0xa7,
		0xbc, 0xba, 0xbd, 0xbb, -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 13, 1, 0x07, 0x06, 0x82, 0x83, 0x85,
		0x84, 0x8f, 0xb8, 0xc2, 0xc3, 0x90, 0x91, 0x92, 0x93, -1);
	SetEnvTabRow (1, 13, 2, -1,   -1,   -1,   -1,   -1,
		-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 14, 1, 0xd0, 0xd1, 0x30, 0x31, 0x3a,
		0x3b, 0x04, 0x05, 0x0f, 0x10, 0x19, 0x1a, 0xda, -1,   -1);
	SetEnvTabRow (1, 14, 2, 0x41, 0x43, 0x44, 0x42, 0x45,
		0x3e, 0x9a, 0x9b, 0x9c, 0x9d, -1,   -1,   -1,   -1,   -1);
	SetEnvTabRow (1, 15, 1, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4,
		0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe);
	SetEnvTabRow (1, 15, 2, 0xff, -1,   -1,   -1,   -1,
		-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1);
}
/*****************************************************************************/
int OnGraphics (void)
/*****************************************************************************/
{
	int iUseRow;
	int iUseTile;
	int iReturn;

	if (iOnTile <= 15)
	{
		iUseRow = 1;
		iUseTile = iOnTile;
		iReturn = EnvTabRow[arLevelType[iCurLevel]][iTabGraphics]
			[iUseRow][iUseTile];
	} else if (iOnTile <= 30) {
		iUseRow = 2;
		iUseTile = iOnTile - 15;
		iReturn = EnvTabRow[arLevelType[iCurLevel]][iTabGraphics]
			[iUseRow][iUseTile];
	} else {
		iReturn = EnvTabRow[arLevelType[iCurLevel]][iOldGraphicsTab]
			[iOldGraphicsY][iOldGraphicsX];
	}

	return (iReturn);
}
/*****************************************************************************/
void UpdateOnTile (void)
/*****************************************************************************/
{
	/*** Sets iOnTile (and iTabGraphics) based on iNewGraphics. ***/

	/*** Used for looping. ***/
	int iTabLoop;
	int iRowLoop;
	int iTileLoop;

	for (iTabLoop = 1; iTabLoop <= 15; iTabLoop++)
	{
		for (iRowLoop = 1; iRowLoop <= 2; iRowLoop++)
		{
			for (iTileLoop = 1; iTileLoop <= 15; iTileLoop++)
			{
				if (EnvTabRow[arLevelType[iCurLevel]]
					[iTabLoop][iRowLoop][iTileLoop] == iNewGraphics)
				{
					iTabGraphics = iTabLoop;
					iOnTile = ((iRowLoop - 1) * 15) + iTileLoop;
				}
			}
		}
	}
}
/*****************************************************************************/
void GetNrsAndOffsets (int iFd, int *arNr, int *arOffset,
	int iObjectSize, char *sObjectName)
/*****************************************************************************/
{
	unsigned char sRead[MAX_BYTES + 2];
	char sReadW[10 + 2];
	char sReadDW[10 + 2];

	/*** Used for looping. ***/
	int iLevelLoop;

	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		read (iFd, sRead, 6);
		snprintf (sReadW, 10, "%02x%02x", sRead[0], sRead[1]);
		arNr[iLevelLoop] = strtoul (sReadW, NULL, 16);
		snprintf (sReadDW, 10, "%02x%02x%02x%02x",
			sRead[2], sRead[3], sRead[4], sRead[5]);
		arOffset[iLevelLoop] = strtoul (sReadDW, NULL, 16);
		if (iDebug == 1)
		{
			if (arNr[iLevelLoop] > 0)
			{
				printf ("[ INFO ] Offset %s, level %i: 0x%02X(-0x%02X)\n",
					sObjectName, iLevelLoop, arOffset[iLevelLoop],
					arOffset[iLevelLoop] + (arNr[iLevelLoop] * iObjectSize) - 1);
			} else {
				printf ("[ INFO ] Offset %s, level %i: (none)\n",
					sObjectName, iLevelLoop);
			}
		}
	}
}
/*****************************************************************************/
void SetNrsAndOffsets (int iFd, int *arNr, int iStartOffset, int iObjectSize)
/*****************************************************************************/
{
	int iOffset;

	/*** Used for looping. ***/
	int iLevelLoop;

	iOffset = iStartOffset + (iNrLevels * 6);

	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		WriteWord (iFd, arNr[iLevelLoop]);
		if (arNr[iLevelLoop] != 0)
		{
			WriteDWord (iFd, iOffset);
			iOffset+=(arNr[iLevelLoop] * iObjectSize);
		} else {
			WriteDWord (iFd, 0);
		}
	}
}
/*****************************************************************************/
const char* ColorAsText (int iColor)
/*****************************************************************************/
{
	static char sReturn[MAX_TEXT + 2];

	switch (iColor)
	{
		case 1: return ("red"); break;
		case 3: return ("green"); break;
		default:
			snprintf (sReturn, MAX_TEXT, "%i", iColor);
			return (sReturn);
	}
}
/*****************************************************************************/
const char* EffectAsText (int iEffect)
/*****************************************************************************/
{
	static char sReturn[MAX_TEXT + 2];

	switch (iEffect)
	{
		case 0: return ("heal"); break;
		case 1: return ("life"); break;
		case 2: return ("hurt"); break;
		case 3: return ("float"); break;
		case 4: return ("loose"); break;
		case 5: return ("stop"); break;
		case 6: case 7: case 8: case 9: return ("time"); break;
		default:
			snprintf (sReturn, MAX_TEXT, "%i", iEffect);
			return (sReturn);
	}
}
/*****************************************************************************/
const char* StateAsText (int iS1, int iS2, int iS3)
/*****************************************************************************/
{
	if ((iS1 == 0) && (iS2 == 0) && (iS3 == 0))
	{
		return ("closed");
	} else if ((iS1 == 7) && (iS2 == 12) && (iS3 == 0)) {
		return ("c on start");
	} else if ((iS1 == 3) && (iS2 == 12) && (iS3 == 0xFFFF)) {
		return ("open");
	} else if ((iS1 == 1) && (iS2 == 0) && (iS3 == 0)) {
		return ("o on start");
	} else {
		return ("custom");
	}
}
/*****************************************************************************/
int GetSelectedTileValue (int *arAttr, int *arX, int *arY,
	int iNrObjects, int iDefault)
/*****************************************************************************/
{
	int iReturn;

	iReturn = GetAttribute (((iCurX - 1) * WIDTH) + iSelectedX,
		((iCurY - 1) * HEIGHT) + iSelectedY, arX, arY, arAttr, iNrObjects);
	if (iReturn != -1)
	{
		return (iReturn);
	} else {
		return (iDefault);
	}
}
/*****************************************************************************/
int GetAttribute (int iX, int iY, int *arX, int *arY, int *arAttr,
	int iNrObjects)
/*****************************************************************************/
{
	/*** Used for looping. ***/
	int iObjectLoop;

	for (iObjectLoop = 1; iObjectLoop <= iNrObjects; iObjectLoop++)
	{
		if ((arX[iObjectLoop] == iX) &&
			(arY[iObjectLoop] == iY))
		{
			return (arAttr[iObjectLoop]);
		}
	}

	return (-1);
}
/*****************************************************************************/
void ModifyStart (int iLevel, int iToFrom)
/*****************************************************************************/
{
	int iFd;
	int iOffsetLevel;
	char sRead[1 + 2];

	iFd = open (sPathFile, O_RDWR|O_BINARY, 0600);
	if (iFd == -1)
	{
		printf ("[FAILED] Error opening %s: %s!\n", sPathFile, strerror (errno));
		exit (EXIT_ERROR);
	}

	switch (iEXEType)
	{
		case 1: iOffsetLevel = 0x1DFA7; break; /*** US ***/
		case 2: iOffsetLevel = 0x4E2A7; break; /*** EU ***/
		default:
			printf ("[FAILED] Unknown iEXEType: %i!\n", iEXEType);
			exit (EXIT_ERROR); break;
	}

	if (iToFrom == 1)
	{
		lseek (iFd, iOffsetLevel, SEEK_SET);
		read (iFd, sRead, 1);
		iModified = sRead[0] + 1;
		lseek (iFd, iOffsetLevel, SEEK_SET);
		WriteByte (iFd, iLevel - 1);
	} else {
		lseek (iFd, iOffsetLevel, SEEK_SET);
		WriteByte (iFd, iModified - 1);
	}

	close (iFd);
}
/*****************************************************************************/
int Total (char *sType)
/*****************************************************************************/
{
	int iTotal;

	/*** Used for looping. ***/
	int iLevelLoop;

	iTotal = 0;

	if (strcmp (sType, "rooms") == 0)
	{
		for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
			{ iTotal+=(arLevelWidth[iLevelLoop] * arLevelHeight[iLevelLoop]); }
	}

	if (strcmp (sType, "guards") == 0)
	{
		for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
			{ iTotal+=arNrGuards[iLevelLoop]; }
	}

	if (strcmp (sType, "doors") == 0)
	{
		for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
			{ iTotal+=arNrDoors[iLevelLoop]; }
	}

	if (strcmp (sType, "gates") == 0)
	{
		for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
			{ iTotal+=arNrGates[iLevelLoop]; }
	}

	if (strcmp (sType, "loose") == 0)
	{
		for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
			{ iTotal+=arNrLoose[iLevelLoop]; }
	}

	if (strcmp (sType, "raise") == 0)
	{
		for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
			{ iTotal+=arNrRaise[iLevelLoop]; }
	}

	if (strcmp (sType, "drop") == 0)
	{
		for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
			{ iTotal+=arNrDrop[iLevelLoop]; }
	}

	if (strcmp (sType, "chompers") == 0)
	{
		for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
			{ iTotal+=arNrChompers[iLevelLoop]; }
	}

	if (strcmp (sType, "spikes") == 0)
	{
		for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
			{ iTotal+=arNrSpikes[iLevelLoop]; }
	}

	if (strcmp (sType, "potions") == 0)
	{
		for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
			{ iTotal+=arNrPotions[iLevelLoop]; }
	}

	return (iTotal);
}
/*****************************************************************************/
void TotalLine (char *sType, int iAllowed, int iVer)
/*****************************************************************************/
{
	SDL_Color clr;
	SDL_Texture *imgstatus;

	if (Total (sType) <= iAllowed)
		{ clr = color_wh; imgstatus = imgstatus1; }
			else { clr = color_red; imgstatus = imgstatus0; }
	CenterNumber (iAllowed, 250, 129 + (iVer * 24), color_wh, 0);
	CenterNumber (Total (sType), 353, 129 + (iVer * 24), clr, 0);
	ShowImage (imgstatus, 477, 132 + (iVer * 24), "imgstatus");
}
/*****************************************************************************/
int IsSavingAllowed (void)
/*****************************************************************************/
{
	switch (iEXEType)
	{
		case 1: /*** US ***/
			if ((Total ("rooms") <= ALLOWED_US_ROOMS) &&
				(Total ("guards") <= ALLOWED_US_GUARDS) &&
				(Total ("doors") <= ALLOWED_US_DOORS) &&
				(Total ("gates") <= ALLOWED_US_GATES) &&
				(Total ("loose") <= ALLOWED_US_LOOSE) &&
				(Total ("raise") <= ALLOWED_US_RAISE) &&
				(Total ("drop") <= ALLOWED_US_DROP) &&
				(Total ("chompers") <= ALLOWED_US_CHOMPERS) &&
				(Total ("spikes") <= ALLOWED_US_SPIKES) &&
				(Total ("potions") <= ALLOWED_US_POTIONS))
			{ return (1); } else { return (0); }
			break;
		case 2: /*** EU ***/
			if ((Total ("rooms") <= ALLOWED_EU_ROOMS) &&
				(Total ("guards") <= ALLOWED_EU_GUARDS) &&
				(Total ("doors") <= ALLOWED_EU_DOORS) &&
				(Total ("gates") <= ALLOWED_EU_GATES) &&
				(Total ("loose") <= ALLOWED_EU_LOOSE) &&
				(Total ("raise") <= ALLOWED_EU_RAISE) &&
				(Total ("drop") <= ALLOWED_EU_DROP) &&
				(Total ("chompers") <= ALLOWED_EU_CHOMPERS) &&
				(Total ("spikes") <= ALLOWED_EU_SPIKES) &&
				(Total ("potions") <= ALLOWED_EU_POTIONS))
			{ return (1); } else { return (0); }
			break;
	}

	printf ("[ WARN ] IsSavingAllowed() failed!\n");
	return (0);
}
/*****************************************************************************/
void SetTypeDefaults (void)
/*****************************************************************************/
{
	switch (iEXEType)
	{
		case 1: /*** US ***/
			iNrLevels = NR_LEVELS_US;
			iOffsetPrince = OFFSET_PRINCE_US;
			iOffsetLevels = OFFSET_LEVELS_US;
			iOffsetGrOb = OFFSET_GR_OB_US;
			iOffsetGuards = OFFSET_GUARDS_US;
			iOffsetDoors = OFFSET_DOORS_US;
			iOffsetGates = OFFSET_GATES_US;
			iOffsetLoose = OFFSET_LOOSE_US;
			iOffsetRaise = OFFSET_RAISE_US;
			iOffsetDrop = OFFSET_DROP_US;
			iOffsetChompers = OFFSET_CHOMPERS_US;
			iOffsetSpikes = OFFSET_SPIKES_US;
			iOffsetPotions = OFFSET_POTIONS_US;
			break;
		case 2: /*** EU ***/
			iNrLevels = NR_LEVELS_EU;
			iOffsetPrince = OFFSET_PRINCE_EU;
			iOffsetLevels = OFFSET_LEVELS_EU;
			iOffsetGrOb = OFFSET_GR_OB_EU;
			iOffsetGuards = OFFSET_GUARDS_EU;
			iOffsetDoors = OFFSET_DOORS_EU;
			iOffsetGates = OFFSET_GATES_EU;
			iOffsetLoose = OFFSET_LOOSE_EU;
			iOffsetRaise = OFFSET_RAISE_EU;
			iOffsetDrop = OFFSET_DROP_EU;
			iOffsetChompers = OFFSET_CHOMPERS_EU;
			iOffsetSpikes = OFFSET_SPIKES_EU;
			iOffsetPotions = OFFSET_POTIONS_EU;
			break;
	}
}
/*****************************************************************************/
void WriteByte (int iFd, int iValue)
/*****************************************************************************/
{
	char sToWrite[MAX_TOWRITE + 2];

	snprintf (sToWrite, MAX_TOWRITE, "%c", iValue);
	write (iFd, sToWrite, 1);
}
/*****************************************************************************/
void WriteWord (int iFd, int iValue)
/*****************************************************************************/
{
	char sToWrite[MAX_TOWRITE + 2];

	snprintf (sToWrite, MAX_TOWRITE, "%c%c", (iValue >> 8) & 0xFF,
		(iValue >> 0) & 0xFF);
	write (iFd, sToWrite, 2);
}
/*****************************************************************************/
void WriteDWord (int iFd, int iValue)
/*****************************************************************************/
{
	char sToWrite[MAX_TOWRITE + 2];

	snprintf (sToWrite, MAX_TOWRITE, "%c%c%c%c",
		(iValue >> 24) & 0xFF,
		(iValue >> 16) & 0xFF,
		(iValue >> 8) & 0xFF,
		(iValue >> 0) & 0xFF);
	write (iFd, sToWrite, 4);
}
/*****************************************************************************/
int LoadYBottom (int iYCoor)
/*****************************************************************************/
{
	return ((iYCoor + 1) / 64);
}
/*****************************************************************************/
int SaveYBottom (int iY)
/*****************************************************************************/
{
	return ((iY * 64) - 1);
}
/*****************************************************************************/
int LoadYTop (int iYCoor)
/*****************************************************************************/
{
	return ((iYCoor / 64) + 1);
}
/*****************************************************************************/
int SaveYTop (int iY)
/*****************************************************************************/
{
	return ((iY - 1) * 64);
}
/*****************************************************************************/
void TempAttributes (int iObject)
/*****************************************************************************/
{
	/* iObject 0x0C is used for guards.
	 * Also, arOffset...[] is deliberately being ignored.
	 */

	/*** Used for looping. ***/
	int iTempLoop;

	if (iObject == 0x03) /*** raise (buttons) ***/
	{
		iNrTemp = arNrRaise[iCurLevel];
		for (iTempLoop = 1; iTempLoop <= iNrTemp; iTempLoop++)
		{
			iTempAttr1[iTempLoop] = arRaiseYP[iCurLevel][iTempLoop];
			iTempAttr2[iTempLoop] = arRaiseY[iCurLevel][iTempLoop];
			iTempAttr3[iTempLoop] = arRaiseXP[iCurLevel][iTempLoop];
			iTempAttr4[iTempLoop] = arRaiseX[iCurLevel][iTempLoop];
			iTempAttr5[iTempLoop] = arRaiseGate1[iCurLevel][iTempLoop];
			iTempAttr6[iTempLoop] = arRaiseGate2[iCurLevel][iTempLoop];
			iTempAttr7[iTempLoop] = arRaiseGate3[iCurLevel][iTempLoop];
		}
	}

	if (iObject == 0x04) /*** drop (buttons) ***/
	{
		iNrTemp = arNrDrop[iCurLevel];
		for (iTempLoop = 1; iTempLoop <= iNrTemp; iTempLoop++)
		{
			iTempAttr1[iTempLoop] = arDropYP[iCurLevel][iTempLoop];
			iTempAttr2[iTempLoop] = arDropY[iCurLevel][iTempLoop];
			iTempAttr3[iTempLoop] = arDropXP[iCurLevel][iTempLoop];
			iTempAttr4[iTempLoop] = arDropX[iCurLevel][iTempLoop];
			iTempAttr5[iTempLoop] = arDropGate1[iCurLevel][iTempLoop];
			iTempAttr6[iTempLoop] = arDropGate2[iCurLevel][iTempLoop];
			iTempAttr7[iTempLoop] = arDropGate3[iCurLevel][iTempLoop];
		}
	}

	if (iObject == 0x05) /*** gates ***/
	{
		iNrTemp = arNrGates[iCurLevel];
		for (iTempLoop = 1; iTempLoop <= iNrTemp; iTempLoop++)
		{
			iTempAttr1[iTempLoop] = arGateState1[iCurLevel][iTempLoop];
			iTempAttr2[iTempLoop] = arGateYP[iCurLevel][iTempLoop];
			iTempAttr3[iTempLoop] = arGateY[iCurLevel][iTempLoop];
			iTempAttr4[iTempLoop] = arGateXP[iCurLevel][iTempLoop];
			iTempAttr5[iTempLoop] = arGateX[iCurLevel][iTempLoop];
			iTempAttr6[iTempLoop] = arGateState2[iCurLevel][iTempLoop];
			iTempAttr7[iTempLoop] = arGateState3[iCurLevel][iTempLoop];
			iTempAttr8[iTempLoop] = arGateUnk[iCurLevel][iTempLoop];
		}
	}

	if (iObject == 0x06) /*** loose (floors) ***/
	{
		iNrTemp = arNrLoose[iCurLevel];
		for (iTempLoop = 1; iTempLoop <= iNrTemp; iTempLoop++)
		{
			iTempAttr1[iTempLoop] = arLooseYP[iCurLevel][iTempLoop];
			iTempAttr2[iTempLoop] = arLooseY[iCurLevel][iTempLoop];
			iTempAttr3[iTempLoop] = arLooseXP[iCurLevel][iTempLoop];
			iTempAttr4[iTempLoop] = arLooseX[iCurLevel][iTempLoop];
		}
	}

	if (iObject == 0x07) /*** spikes ***/
	{
		iNrTemp = arNrSpikes[iCurLevel];
		for (iTempLoop = 1; iTempLoop <= iNrTemp; iTempLoop++)
		{
			iTempAttr1[iTempLoop] = arSpikeYP[iCurLevel][iTempLoop];
			iTempAttr2[iTempLoop] = arSpikeY[iCurLevel][iTempLoop];
			iTempAttr3[iTempLoop] = arSpikeXP[iCurLevel][iTempLoop];
			iTempAttr4[iTempLoop] = arSpikeX[iCurLevel][iTempLoop];
		}
	}

	if (iObject == 0x08) /*** chompers ***/
	{
		iNrTemp = arNrChompers[iCurLevel];
		for (iTempLoop = 1; iTempLoop <= iNrTemp; iTempLoop++)
		{
			iTempAttr1[iTempLoop] = arChomperYP[iCurLevel][iTempLoop];
			iTempAttr2[iTempLoop] = arChomperY[iCurLevel][iTempLoop];
			iTempAttr3[iTempLoop] = arChomperXP[iCurLevel][iTempLoop];
			iTempAttr4[iTempLoop] = arChomperX[iCurLevel][iTempLoop];
		}
	}

	if (iObject == 0x09) /*** potions ***/
	{
		iNrTemp = arNrPotions[iCurLevel];
		for (iTempLoop = 1; iTempLoop <= iNrTemp; iTempLoop++)
		{
			iTempAttr1[iTempLoop] = arPotionColor[iCurLevel][iTempLoop];
			iTempAttr2[iTempLoop] = arPotionYP[iCurLevel][iTempLoop];
			iTempAttr3[iTempLoop] = arPotionY[iCurLevel][iTempLoop];
			iTempAttr4[iTempLoop] = arPotionXP[iCurLevel][iTempLoop];
			iTempAttr5[iTempLoop] = arPotionX[iCurLevel][iTempLoop];
			iTempAttr6[iTempLoop] = arPotionEffect[iCurLevel][iTempLoop];
		}
	}

	if (iObject == 0x0A) /*** (level) doors ***/
	{
		iNrTemp = arNrDoors[iCurLevel];
		for (iTempLoop = 1; iTempLoop <= iNrTemp; iTempLoop++)
		{
			iTempAttr1[iTempLoop] = arDoorType[iCurLevel][iTempLoop];
			iTempAttr2[iTempLoop] = arDoorYP[iCurLevel][iTempLoop];
			iTempAttr3[iTempLoop] = arDoorY[iCurLevel][iTempLoop];
			iTempAttr4[iTempLoop] = arDoorXP[iCurLevel][iTempLoop];
			iTempAttr5[iTempLoop] = arDoorX[iCurLevel][iTempLoop];
		}
	}

	if (iObject == 0x0C) /*** guards ***/
	{
		iNrTemp = arNrGuards[iCurLevel];
		for (iTempLoop = 1; iTempLoop <= iNrTemp; iTempLoop++)
		{
			iTempAttr1[iTempLoop] = arGuardYP[iCurLevel][iTempLoop];
			iTempAttr2[iTempLoop] = arGuardY[iCurLevel][iTempLoop];
			iTempAttr3[iTempLoop] = arGuardXP[iCurLevel][iTempLoop];
			iTempAttr4[iTempLoop] = arGuardX[iCurLevel][iTempLoop];
			iTempAttr5[iTempLoop] = arGuardDir[iCurLevel][iTempLoop];
			iTempAttr6[iTempLoop] = arGuardSprite[iCurLevel][iTempLoop];
			iTempAttr7[iTempLoop] = arGuardType[iCurLevel][iTempLoop];
			iTempAttr8[iTempLoop] = arGuardSkill[iCurLevel][iTempLoop];
			iTempAttr9[iTempLoop] = arGuardHP[iCurLevel][iTempLoop];
		}
	}
}
/*****************************************************************************/
void AddRemoveAttributes (int iObject, int iX, int iY, int *arNr,
	int *arX, int *arY, int iType)
/*****************************************************************************/
{
	/*** iObject 0x0C is used as the guard 'object'. ***/

	int iObjectNr;
	int iAdded;
	int iSkip, iAdd;
	int iNrLoop;

	/*** Used for looping. ***/
	int iTempLoop;

	TempAttributes (iObject);
	switch (iType)
	{
		case 0: /*** remove ***/
			arNr[iCurLevel]--; iNrLoop = iNrTemp; /*** No -1. ***/ break;
		case 1: /*** add ***/
			arNr[iCurLevel]++; iNrLoop = iNrTemp + 1; break;
		default:
			printf ("[FAILED] Incorrect type: %i!\n", iType);
			exit (EXIT_ERROR); break;
	}
	if (arNr[iCurLevel] > 0)
	{
		iObjectNr = 1;
		iAdded = 0;
		for (iTempLoop = 1; iTempLoop <= iNrLoop; iTempLoop++)
		{
			/*** Checks: is this the object to remove? ***/
			iSkip = 0;
			if ((iType == 0) && (arX[iTempLoop] == iX) &&
				(arY[iTempLoop] == iY))
			{
				iSkip = 1;
				if (iObject == 0x05) { RaiseDropUpdate (iObjectNr, -1); }
			}

			/*** Checks: is this the location to add? ***/
			iAdd = 0;
			if ((iType == 1) && (iAdded == 0))
			{
				if (((arX[iTempLoop] == iX) &&
						(arY[iTempLoop] > iY)) ||
						(arX[iTempLoop] > iX) ||
						(iObjectNr == arNr[iCurLevel]))
				{
					iAdd = 1;
					if (iObject == 0x05) { RaiseDropUpdate (iObjectNr - 1, 1); }
				}
			}

			if (iAdd == 1)
			{
				arY[iObjectNr] = iY;
				arX[iObjectNr] = iX;
				/*** Other attributes are added in UseTile(). ***/
				iObjectNr++;
				iAdded = 1;
			}
			if ((iSkip != 1) && (iObjectNr <= arNr[iCurLevel]))
			{
				switch (iObject)
				{
					case 0x03: /*** raise (buttons) ***/
						arRaiseYP[iCurLevel][iObjectNr] = iTempAttr1[iTempLoop];
						arRaiseY[iCurLevel][iObjectNr] = iTempAttr2[iTempLoop];
						arRaiseXP[iCurLevel][iObjectNr] = iTempAttr3[iTempLoop];
						arRaiseX[iCurLevel][iObjectNr] = iTempAttr4[iTempLoop];
						arRaiseGate1[iCurLevel][iObjectNr] = iTempAttr5[iTempLoop];
						arRaiseGate2[iCurLevel][iObjectNr] = iTempAttr6[iTempLoop];
						arRaiseGate3[iCurLevel][iObjectNr] = iTempAttr7[iTempLoop];
						break;
					case 0x04: /*** drop (buttons) ***/
						arDropYP[iCurLevel][iObjectNr] = iTempAttr1[iTempLoop];
						arDropY[iCurLevel][iObjectNr] = iTempAttr2[iTempLoop];
						arDropXP[iCurLevel][iObjectNr] = iTempAttr3[iTempLoop];
						arDropX[iCurLevel][iObjectNr] = iTempAttr4[iTempLoop];
						arDropGate1[iCurLevel][iObjectNr] = iTempAttr5[iTempLoop];
						arDropGate2[iCurLevel][iObjectNr] = iTempAttr6[iTempLoop];
						arDropGate3[iCurLevel][iObjectNr] = iTempAttr7[iTempLoop];
						break;
					case 0x05: /*** gates ***/
						arGateState1[iCurLevel][iObjectNr] = iTempAttr1[iTempLoop];
						arGateYP[iCurLevel][iObjectNr] = iTempAttr2[iTempLoop];
						arGateY[iCurLevel][iObjectNr] = iTempAttr3[iTempLoop];
						arGateXP[iCurLevel][iObjectNr] = iTempAttr4[iTempLoop];
						arGateX[iCurLevel][iObjectNr] = iTempAttr5[iTempLoop];
						arGateState2[iCurLevel][iObjectNr] = iTempAttr6[iTempLoop];
						arGateState3[iCurLevel][iObjectNr] = iTempAttr7[iTempLoop];
						arGateUnk[iCurLevel][iObjectNr] = iTempAttr8[iTempLoop];
						break;
					case 0x06: /*** loose (floors) ***/
						arLooseYP[iCurLevel][iObjectNr] = iTempAttr1[iTempLoop];
						arLooseY[iCurLevel][iObjectNr] = iTempAttr2[iTempLoop];
						arLooseXP[iCurLevel][iObjectNr] = iTempAttr3[iTempLoop];
						arLooseX[iCurLevel][iObjectNr] = iTempAttr4[iTempLoop];
						break;
					case 0x07: /*** spikes ***/
						arSpikeYP[iCurLevel][iObjectNr] = iTempAttr1[iTempLoop];
						arSpikeY[iCurLevel][iObjectNr] = iTempAttr2[iTempLoop];
						arSpikeXP[iCurLevel][iObjectNr] = iTempAttr3[iTempLoop];
						arSpikeX[iCurLevel][iObjectNr] = iTempAttr4[iTempLoop];
						break;
					case 0x08: /*** chompers ***/
						arChomperYP[iCurLevel][iObjectNr] = iTempAttr1[iTempLoop];
						arChomperY[iCurLevel][iObjectNr] = iTempAttr2[iTempLoop];
						arChomperXP[iCurLevel][iObjectNr] = iTempAttr3[iTempLoop];
						arChomperX[iCurLevel][iObjectNr] = iTempAttr4[iTempLoop];
						break;
					case 0x09: /*** potions ***/
						arPotionColor[iCurLevel][iObjectNr] = iTempAttr1[iTempLoop];
						arPotionYP[iCurLevel][iObjectNr] = iTempAttr2[iTempLoop];
						arPotionY[iCurLevel][iObjectNr] = iTempAttr3[iTempLoop];
						arPotionXP[iCurLevel][iObjectNr] = iTempAttr4[iTempLoop];
						arPotionX[iCurLevel][iObjectNr] = iTempAttr5[iTempLoop];
						arPotionEffect[iCurLevel][iObjectNr] = iTempAttr6[iTempLoop];
						break;
					case 0x0A: /*** (level) doors ***/
						arDoorType[iCurLevel][iObjectNr] = iTempAttr1[iTempLoop];
						arDoorYP[iCurLevel][iObjectNr] = iTempAttr2[iTempLoop];
						arDoorY[iCurLevel][iObjectNr] = iTempAttr3[iTempLoop];
						arDoorXP[iCurLevel][iObjectNr] = iTempAttr4[iTempLoop];
						arDoorX[iCurLevel][iObjectNr] = iTempAttr5[iTempLoop];
						break;
					case 0x0C: /*** guards ***/
						arGuardYP[iCurLevel][iObjectNr] = iTempAttr1[iTempLoop];
						arGuardY[iCurLevel][iObjectNr] = iTempAttr2[iTempLoop];
						arGuardXP[iCurLevel][iObjectNr] = iTempAttr3[iTempLoop];
						arGuardX[iCurLevel][iObjectNr] = iTempAttr4[iTempLoop];
						arGuardDir[iCurLevel][iObjectNr] = iTempAttr5[iTempLoop];
						arGuardSprite[iCurLevel][iObjectNr] = iTempAttr6[iTempLoop];
						arGuardType[iCurLevel][iObjectNr] = iTempAttr7[iTempLoop];
						arGuardSkill[iCurLevel][iObjectNr] = iTempAttr8[iTempLoop];
						arGuardHP[iCurLevel][iObjectNr] = iTempAttr9[iTempLoop];
						break;
				}
				iObjectNr++;
			}
		}
	}
}
/*****************************************************************************/
void AttributeDefaults (int iObject)
/*****************************************************************************/
{
	switch (iObject)
	{
		case 0x03: /*** raise ***/
			iRaiseGate1Check = -1;
			iRaiseGate1 = 0;
			iRaiseGate2Check = 0xFFFD; /*** none ***/
			iRaiseGate2 = 0xFFFD; /*** none ***/
			iRaiseGate3Check = 0xFFFD; /*** none ***/
			iRaiseGate3 = 0xFFFD; /*** none ***/
			break;
		case 0x04: /*** drop ***/
			iDropGate1Check = -1;
			iDropGate1 = 0;
			iDropGate2Check = 0xFFFD; /*** none ***/
			iDropGate2 = 0xFFFD; /*** none ***/
			iDropGate3Check = 0xFFFD; /*** none ***/
			iDropGate3 = 0xFFFD; /*** none ***/
			break;
		case 0x05: /*** gate ***/
			iGateStateCheck = 0;
			iGateState = 0;
			iGateOpenness = 0;
			iGateDelay = 0;
			break;
		case 0x09: /*** potion ***/
			iPotionColorCheck = 1;
			iPotionColor = 1;
			iPotionEffectCheck = 0;
			iPotionEffect = 0;
			break;
		case 0x0A: /*** door ***/
			iDoorTypeCheck = 0;
			iDoorType = 0;
			break;
	}
}
/*****************************************************************************/
void SetCheck (char *sObject, int iX, int iY, int iCheck, int iValue)
/*****************************************************************************/
{
	if (InArea (iX, iY, iX + 14, iY + 14) == 1)
	{
		/*** raise ***/
		if (strcmp (sObject, "raisegate1") == 0)
		{
			iRaiseGate1Check = iCheck;
			iRaiseGate1 = iValue;
		}
		if (strcmp (sObject, "raisegate2") == 0)
		{
			iRaiseGate2Check = iCheck;
			iRaiseGate2 = iValue;
		}
		if (strcmp (sObject, "raisegate3") == 0)
		{
			iRaiseGate3Check = iCheck;
			iRaiseGate3 = iValue;
		}

		/*** drop ***/
		if (strcmp (sObject, "dropgate1") == 0)
		{
			iDropGate1Check = iCheck;
			iDropGate1 = iValue;
		}
		if (strcmp (sObject, "dropgate2") == 0)
		{
			iDropGate2Check = iCheck;
			iDropGate2 = iValue;
		}
		if (strcmp (sObject, "dropgate3") == 0)
		{
			iDropGate3Check = iCheck;
			iDropGate3 = iValue;
		}

		/*** SetCheck() is not used for the gate object. ***/

		/*** potion ***/
		if (strcmp (sObject, "potioncolor") == 0)
		{
			iPotionColorCheck = iCheck;
			iPotionColor = iValue;
		}
		if (strcmp (sObject, "potioneffect") == 0)
		{
			iPotionEffectCheck = iCheck;
			iPotionEffect = iValue;
		}

		/*** door ***/
		if (strcmp (sObject, "doortype") == 0)
		{
			iDoorTypeCheck = iCheck;
			iDoorType = iValue;
		}

		PlaySound ("wav/check_box.wav");
	}
}
/*****************************************************************************/
void SetCheckGate (int iX, int iY, int iC, int iState, int iOpenn, int iDelay)
/*****************************************************************************/
{
	if (InArea (iX, iY, iX + 14, iY + 14) == 1)
	{
		iGateStateCheck = iC;
		iGateState = iState;
		iGateOpenness = iOpenn;
		iGateDelay = iDelay;
		PlaySound ("wav/check_box.wav");
	}
}
/*****************************************************************************/
void UpdateSkillHP (void)
/*****************************************************************************/
{
	/*** If the selected tile has a guard, update his skill and HP. ***/

	int iX, iY;
	int iHasGuard;
	int iOldSkill, iOldHP;

	iX = ((iCurX - 1) * WIDTH) + iSelectedX;
	iY = ((iCurY - 1) * HEIGHT) + iSelectedY;
	iHasGuard = HasObject (iX, iY, arGuardX[iCurLevel],
		arGuardY[iCurLevel], arNrGuards[iCurLevel]);
	if (iHasGuard == 1)
	{
		iOldSkill = GetSelectedTileValue (arGuardSkill[iCurLevel],
			arGuardX[iCurLevel], arGuardY[iCurLevel],
			arNrGuards[iCurLevel], 0);
		iOldHP = GetSelectedTileValue (arGuardHP[iCurLevel],
			arGuardX[iCurLevel], arGuardY[iCurLevel],
			arNrGuards[iCurLevel], 3);
		if ((iOldSkill != iNewSkill) || (iOldHP != iNewHP))
		{
			SetAttribute (iX, iY, arGuardX[iCurLevel], arGuardY[iCurLevel],
				arGuardSkill[iCurLevel], arNrGuards[iCurLevel], iNewSkill);
			SetAttribute (iX, iY, arGuardX[iCurLevel], arGuardY[iCurLevel],
				arGuardHP[iCurLevel], arNrGuards[iCurLevel], iNewHP);
			iChanged++;
		}
	}
}
/*****************************************************************************/
void RandomizeLevel (void)
/*****************************************************************************/
{
	int iRandomO;
	int iRandomG;
	int iRow;
	int iEvenOdd;
	int iObjectLeft, iObjectDown, iObjectLeftDown;
	int iRandom;

	/*** Used for looping. ***/
	int iYLoop, iXLoop;

	for (iYLoop = (arLevelHeight[iCurLevel] * HEIGHT); iYLoop >= 1;
		iYLoop--)
	{
		for (iXLoop = 1; iXLoop <= (arLevelWidth[iCurLevel] * WIDTH);
			iXLoop++)
		{
			iRandomO = 0x00; iRandomG = 0x00; /*** Defaults. ***/
			iRow = iYLoop % 3;
			if (iRow == 0) { iRow = 3; }
			iEvenOdd = iXLoop % 2;

			if (iXLoop > 1)
			{
				iObjectLeft = arLevelObjects[iCurLevel][iXLoop - 1][iYLoop];
			} else {
				iObjectLeft = 0x01; /*** wall ***/
			}

			if (iYLoop < (arLevelHeight[iCurLevel] * HEIGHT))
			{
				iObjectDown = arLevelObjects[iCurLevel][iXLoop][iYLoop + 1];
			} else {
				iObjectDown = 0x01; /*** wall ***/
			}

			if ((iXLoop > 1) && (iYLoop < (arLevelHeight[iCurLevel] * HEIGHT)))
			{
				iObjectLeftDown = arLevelObjects[iCurLevel][iXLoop - 1][iYLoop + 1];
			} else {
				iObjectLeftDown = 0x01; /*** wall ***/
			}

			/*** 1-3 ***/
			iRandom = 1 + (int) (3.0 * rand() / (RAND_MAX + 1.0));

			/*** 0-2 ***/
			do {
				iRandomO = (int) (3.0 * rand() / (RAND_MAX + 1.0));
			/*** Empty above wall not allowed. ***/
			} while ((iObjectDown == 0x01) && (iRandomO == 0x00));

			if (arLevelType[iCurLevel] == 0) /*** dungeon ***/
			{
				switch (iRandomO)
				{
					case 0x00: /*** space ***/
						switch (iObjectLeft)
						{
							case 0x00: /*** space ***/
								switch (iRandom)
								{
									case 1: iRandomG = 0x0A; break;
									case 2: iRandomG = 0x0A; break;
									case 3: iRandomG = 0x47; break;
								}
								break;
							case 0x01: /*** wall ***/
								if (iObjectLeftDown == 0x01) /*** wall ***/
								{
									if (iRow == 1) { iRandomG = 0xDB; }
									if (iRow == 2) { iRandomG = 0xE5; }
									if (iRow == 3) { iRandomG = 0xEF; }
								} else {
									if (iRow == 1) { iRandomG = 0xB1; }
									if (iRow == 2) { iRandomG = 0xBB; }
									if (iRow == 3) { iRandomG = 0x44; }
								}
								break;
							case 0x02: /*** floor ***/
								if (iObjectLeftDown == 0x01) /*** wall ***/
								{
									if (iEvenOdd == BLUE)
									{
										if (iRow == 1) { iRandomG = 0xA3; }
										if (iRow == 2) { iRandomG = 0xA2; }
										if (iRow == 3) { iRandomG = 0xA3; }
									}
									if (iEvenOdd == BROWN)
									{
										if (iRow == 1) { iRandomG = 0xA1; }
										if (iRow == 2) { iRandomG = 0xA0; }
										if (iRow == 3) { iRandomG = 0xA1; }
									}
								} else {
									if (iEvenOdd == BLUE) { iRandomG = 0xA9; }
									if (iEvenOdd == BROWN) { iRandomG = 0xA8; }
								}
								break;
						}
						break;
					case 0x01: /*** wall ***/
						switch (iRandom)
						{
							case 1:
								if (iRow == 1) { iRandomG = 0xD7; }
								if (iRow == 2) { iRandomG = 0xE1; }
								if (iRow == 3) { iRandomG = 0xEA; }
								break;
							case 2:
								if (iRow == 1) { iRandomG = 0xD9; }
								if (iRow == 2) { iRandomG = 0xCD; }
								if (iRow == 3) { iRandomG = 0xC0; }
								break;
							case 3:
								if (iRow == 1) { iRandomG = 0xC5; }
								if (iRow == 2) { iRandomG = 0xB6; }
								if (iRow == 3) { iRandomG = 0xBF; }
								break;
						}
						break;
					case 0x02: /*** floor ***/
						switch (iObjectLeft)
						{
							case 0x00: /*** space ***/
								if (iEvenOdd == BLUE) { iRandomG = 0x96; }
								if (iEvenOdd == BROWN) { iRandomG = 0x97; }
								break;
							case 0x01: /*** wall ***/
								if (iEvenOdd == BLUE)
								{
									if (iRow == 1) { iRandomG = 0xB2; }
									if (iRow == 2) { iRandomG = 0xBC; }
									if (iRow == 3) { iRandomG = 0x65; }
								}
								if (iEvenOdd == BROWN)
								{
									if (iRow == 1) { iRandomG = 0xB3; }
									if (iRow == 2) { iRandomG = 0xBD; }
									if (iRow == 3) { iRandomG = 0x63; }
								}
								break;
							case 0x02: /*** floor ***/
								switch (iRandom)
								{
									case 1:
										if (iEvenOdd == BLUE) { iRandomG = 0x38; }
										if (iEvenOdd == BROWN) { iRandomG = 0x39; }
										break;
									case 2:
										if (iEvenOdd == BLUE) { iRandomG = 0x92; }
										if (iEvenOdd == BROWN) { iRandomG = 0x93; }
										break;
									case 3:
										if (iEvenOdd == BLUE) { iRandomG = 0x34; }
										if (iEvenOdd == BROWN) { iRandomG = 0x35; }
										break;
								}
								break;
						}
						break;
				}
			} else { /*** palace ***/
				switch (iRandomO)
				{
					case 0x00: /*** space ***/
						switch (iObjectLeft)
						{
							case 0x00: /*** space ***/
								switch (iRandom)
								{
									case 1: iRandomG = 0x0A; break;
									case 2: iRandomG = 0x0A; break;
									case 3: iRandomG = 0x07; break;
								}
								break;
							case 0x01: /*** wall ***/
								if (iObjectLeftDown == 0x01) /*** wall ***/
									{ iRandomG = 0x91; } else { iRandomG = 0xB8; }
								break;
							case 0x02: /*** floor ***/
								if (iObjectLeftDown == 0x01) /*** wall ***/
									{ iRandomG = 0x7C; } else { iRandomG = 0x03; }
								break;
						}
						break;
					case 0x01: /*** wall ***/
						/*** Random brick colors would not fit together. ***/
						iRandomG = 0x65;
						break;
					case 0x02: /*** floor ***/
						switch (iObjectLeft)
						{
							case 0x00: /*** space ***/
								iRandomG = 0x0D;
								break;
							case 0x01: /*** wall ***/
								iRandomG = 0x96;
								break;
							case 0x02: /*** floor ***/
								switch (iRandom)
								{
									case 1: iRandomG = 0x1B; break;
									case 2: iRandomG = 0x08; break;
									case 3: iRandomG = 0xC1; break;
								}
								break;
						}
						break;
				}
			}
			SetLocation (iXLoop, iYLoop, iRandomO, iRandomG);
		}
	}
	iChanged++;
	PlaySound ("wav/ok_close.wav");
}
/*****************************************************************************/
int GuardSprite (int iType)
/*****************************************************************************/
{
	int iSprite;

	iSprite = 0x00; /*** To prevent warnings. ***/
	switch (iEXEType)
	{
		case 1: /*** US ***/
			switch (iType)
			{
				case 0: iSprite = 0x23838; break; /*** guard ***/
				case 1: iSprite = 0x23954; break; /*** skeleton ***/
				case 2: iSprite = 0x23DD8; break; /*** fat ***/
				case 3: iSprite = 0x24000; break; /*** shadow ***/
				case 4: iSprite = 0x241B2; break; /*** Jaffar ***/
			}
			break;
		case 2: /*** EU ***/
			switch (iType)
			{
				case 0: iSprite = 0x53C8C; break; /*** guard ***/
				case 1: iSprite = 0x53DA2; break; /*** skeleton ***/
				case 2: iSprite = 0x54226; break; /*** fat ***/
				case 3: iSprite = 0x54448; break; /*** shadow ***/
				case 4: iSprite = 0x545FA; break; /*** Jaffar ***/
			}
			break;
	}

	return (iSprite);
}
/*****************************************************************************/
void SingleSword (int iLevel, int iX, int iY)
/*****************************************************************************/
{
	/*** Used for looping. ***/
	int iLevelLoop;
	int iWidthLoop;
	int iHeightLoop;

	for (iLevelLoop = 1; iLevelLoop <= iNrLevels; iLevelLoop++)
	{
		for (iWidthLoop = 1; iWidthLoop <=
			(arLevelWidth[iLevelLoop] * WIDTH); iWidthLoop++)
		{
			for (iHeightLoop = 1; iHeightLoop <=
				(arLevelHeight[iLevelLoop] * HEIGHT); iHeightLoop++)
			{
				if ((arLevelObjects[iLevelLoop][iWidthLoop][iHeightLoop] == 0x0B) &&
					((iLevel != iLevelLoop) || (iWidthLoop != iX) ||
					(iHeightLoop != iY)))
				{
					if (iDebug == 1)
					{
						printf ("[  OK  ] Removing a sword from level %i: x=%i, y=%i\n",
							iLevelLoop, iWidthLoop, iHeightLoop);
					}
					arLevelObjects[iLevelLoop][iWidthLoop][iHeightLoop] = 0x02;
				}
			}
		}
	}
}
/*****************************************************************************/
void SaveSword (int iFd, int iLevel, int iX, int iY)
/*****************************************************************************/
{
	int iValue;

	/*** Level. ***/
	switch (iEXEType)
	{
		case 1: lseek (iFd, 0x26D88, SEEK_SET); break; /*** US ***/
		case 2: lseek (iFd, 0x57582, SEEK_SET); break; /*** EU ***/
	}
	WriteWord (iFd, iLevel - 1);

	/* Level where the prince starts without his sword (and
	 * thus can pick it up). Similar switch as above.
	 */
	switch (iEXEType)
	{
		case 1: lseek (iFd, 0x200B6, SEEK_SET); break; /*** US ***/
		case 2: lseek (iFd, 0x5040A, SEEK_SET); break; /*** EU ***/
	}
	WriteWord (iFd, iLevel - 1);

	/*** YP room ***/
	switch (iEXEType)
	{
		case 1: lseek (iFd, 0x26D94, SEEK_SET); break; /*** US ***/
		case 2: lseek (iFd, 0x5758E, SEEK_SET); break; /*** EU ***/
	}
	iValue = (ceil ((float)iY / HEIGHT) - 1) * (64 * HEIGHT);
	WriteWord (iFd, iValue);

	/*** XP room ***/
	switch (iEXEType)
	{
		case 1: lseek (iFd, 0x26DA0, SEEK_SET); break; /*** US ***/
		case 2: lseek (iFd, 0x5759A, SEEK_SET); break; /*** EU ***/
	}
	iValue = (ceil ((float)iX / WIDTH) - 1) * (32 * WIDTH);
	WriteWord (iFd, iValue);

	/*** YP tile ***/
	switch (iEXEType)
	{
		case 1: lseek (iFd, 0x26DB6, SEEK_SET); break; /*** US ***/
		case 2: lseek (iFd, 0x575B0, SEEK_SET); break; /*** EU ***/
	}
	WriteWord (iFd, (iY * 64) - 9);

	/*** XP tile ***/
	switch (iEXEType)
	{
		case 1: lseek (iFd, 0x26DBC, SEEK_SET); break; /*** US ***/
		case 2: lseek (iFd, 0x575B6, SEEK_SET); break; /*** EU ***/
	}
	WriteWord (iFd, (iX * 32) - 7);

	/*** Location to change to floor after pickup. ***/
	switch (iEXEType)
	{
		case 1: /*** US ***/
			lseek (iFd, 0x2230C, SEEK_SET);
			iValue = 0xFF15E0 + ((iX - 1) * (arLevelHeight[iLevel] *
				HEIGHT)) + (iY - 1);
			WriteDWord (iFd, iValue);
			break;
		case 2: /*** EU ***/
			lseek (iFd, 0x526EC, SEEK_SET);
			iValue = 0xFF167A + ((iX - 1) * (arLevelHeight[iLevel] *
				HEIGHT)) + (iY - 1);
			WriteDWord (iFd, iValue);
			break;
	}
}
/*****************************************************************************/
void LevelResized (int iOldWidth, int iOldHeight)
/*****************************************************************************/
{
	/* There is no need to modify arLevelOffsetGraphics[] or
	 * arLevelOffsetObjects[] here, but it IS necessary to
	 * remove objects and such from rooms that are no longer
	 * being used.
	 */

	/*** Used for looping. ***/
	int iWidthLoop;
	int iHeightLoop;

	/*** Skip the rest if the level has NOT been resized. ***/
	if ((iOldWidth == arLevelWidth[iCurLevel]) &&
		(iOldHeight == arLevelHeight[iCurLevel])) { return; }

	/*** Update arLevelNrTiles[]. ***/
	arLevelNrTiles[iCurLevel] = arLevelWidth[iCurLevel]
		* arLevelHeight[iCurLevel] * TILES;

	/*** Clear rooms that are no longer in use. ***/
	if ((iOldWidth > arLevelWidth[iCurLevel]) ||
		(iOldHeight > arLevelHeight[iCurLevel]))
	{
		if (iOldWidth > arLevelWidth[iCurLevel])
		{
			for (iWidthLoop = arLevelWidth[iCurLevel] + 1;
				iWidthLoop <= iOldWidth; iWidthLoop++)
			{
				for (iHeightLoop = 1; iHeightLoop <=
					arLevelHeight[iCurLevel]; iHeightLoop++)
				{
					ClearRoom (iWidthLoop, iHeightLoop);
				}
			}
			iChanged++;
		}
		if (iOldHeight > arLevelHeight[iCurLevel])
		{
			for (iWidthLoop = 1; iWidthLoop <=
				arLevelWidth[iCurLevel]; iWidthLoop++)
			{
				for (iHeightLoop = arLevelHeight[iCurLevel] + 1;
					iHeightLoop <= iOldHeight; iHeightLoop++)
				{
					ClearRoom (iWidthLoop, iHeightLoop);
				}
			}
			iChanged++;
		}
		if (iCurX > arLevelWidth[iCurLevel])
			{ iCurX = arLevelWidth[iCurLevel]; }
		if (iCurY > arLevelHeight[iCurLevel])
			{ iCurY = arLevelHeight[iCurLevel]; }
	}
}
/*****************************************************************************/
void RaiseDropUpdate (int iObjectNr, int iDecInc)
/*****************************************************************************/
{
	/* This function is called after a gate was removed or added. It
	 * updates gate numbers raise and drop buttons point to.
	 */

	/*** Used for looping. ***/
	int iRaiseLoop;
	int iDropLoop;

	/*** raise ***/
	if (arNrRaise[iCurLevel] != 0)
	{
		for (iRaiseLoop = 1; iRaiseLoop <= arNrRaise[iCurLevel]; iRaiseLoop++)
		{
			if ((arRaiseGate1[iCurLevel][iRaiseLoop] >= iObjectNr) &&
				(arRaiseGate1[iCurLevel][iRaiseLoop] < 0xFFFD))
				{ arRaiseGate1[iCurLevel][iRaiseLoop]+=iDecInc; }
			if ((arRaiseGate2[iCurLevel][iRaiseLoop] >= iObjectNr) &&
				(arRaiseGate2[iCurLevel][iRaiseLoop] < 0xFFFD))
				{ arRaiseGate2[iCurLevel][iRaiseLoop]+=iDecInc; }
			if ((arRaiseGate3[iCurLevel][iRaiseLoop] >= iObjectNr) &&
				(arRaiseGate3[iCurLevel][iRaiseLoop] < 0xFFFD))
				{ arRaiseGate3[iCurLevel][iRaiseLoop]+=iDecInc; }
		}
	}

	/*** drop ***/
	if (arNrDrop[iCurLevel] != 0)
	{
		for (iDropLoop = 1; iDropLoop <= arNrDrop[iCurLevel]; iDropLoop++)
		{
			if ((arDropGate1[iCurLevel][iDropLoop] >= iObjectNr) &&
				(arDropGate1[iCurLevel][iDropLoop] < 0xFFFD))
				{ arDropGate1[iCurLevel][iDropLoop]+=iDecInc; }
			if ((arDropGate2[iCurLevel][iDropLoop] >= iObjectNr) &&
				(arDropGate2[iCurLevel][iDropLoop] < 0xFFFD))
				{ arDropGate2[iCurLevel][iDropLoop]+=iDecInc; }
			if ((arDropGate3[iCurLevel][iDropLoop] >= iObjectNr) &&
				(arDropGate3[iCurLevel][iDropLoop] < 0xFFFD))
				{ arDropGate3[iCurLevel][iDropLoop]+=iDecInc; }
		}
	}
}
/*****************************************************************************/
