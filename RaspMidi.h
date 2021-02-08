#ifndef RaspMIDI_H
#define RaspMIDI_H


//DEFINES
#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif

#define MAXINFOCHARS    128
#define DEFAULTMIDIPORT "out.0"

//INCLUDES
#include <clib/alib_protos.h>
#include <string.h>
#include <stdio.h>

//#include <dos/rdargs.h>

#include <proto/exec.h>
#include <proto/wb.h>
#include <proto/dos.h>
#include <proto/icon.h>
#include <proto/muimaster.h>
#include <proto/camd.h>

#include <workbench/workbench.h>
#include <workbench/startup.h>

#include <libraries/mui.h>
#include <libraries/gadtools.h>
//#include "SDI_hook.h"

//VERSION STRING
//UBYTE ver[]="\0$VER: RaspMIDI " __DATE2__ " tolkien/goblins\0";
UBYTE ver[]="\0$VER: RaspMIDI 1.0 (01-01-2021) tolkien/goblins\0";

UBYTE about[]="\33cRaspMIDI 1.0 (01-01-2021)\n"
			  "Code : tolkien/goblins\n"
			  "lantolkien@gmail.com\n\n"
			  "Thanks to:\n\n"
			  "Dale for mt32-pi\n"
			  "Edu Arana -> Hardware master\n"
			  "Spanish AmiCoders Telegram Group\n"
			  "Estrayk for betatesting and moral support\n";


//STRUCTS
struct s_sysexmsg
{
	UBYTE *name;
	UBYTE msg[5];
}sysexmsg[7];


//ENUMS
//ID buttons
enum {IDreboot=1, IDmt32mode, IDmt32old, IDmt32new, IDcm32l,
			IDsfmode, IDsfless, IDsfbank, IDsfmore};
//Menus
enum { MEN_PROJECT=50, MEN_ABOUT, MEN_QUIT };


//Objetos MUI
APTR app, window, menu, BTreboot, BTmt32mode, BTmt32old,
		BTmt32new, BTcm32l, BTsfmode, BTsfless, BTsfbank, BTsfmore;


//BASES
struct Library *MUIMasterBase;
struct Library *CamdBase;


//PROTOTIPOS
void Show(APTR titulo, APTR cuerpo);
int OpenLib();
void CleanOut(BYTE *txt);
void SendSysExMsg(int ID);
void CreateSysExMsg();
void ChangeBank(LONG id);

//CAMD
struct MidiNode *ournode = NULL;
struct MidiLink *outLink = NULL;


#endif
