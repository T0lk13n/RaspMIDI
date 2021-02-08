#ifdef __STORM__
int main( int argc, char *argv[] );
void wbmain(struct WBStartup* wb)
{   main(0, (char **) wb);
}
#endif


#include "RaspMidi.h"


//GLOBALS
BYTE  midisig = -1;
UBYTE UserMidiPort[MAXINFOCHARS];
UBYTE CurrentBank = 0;

static struct NewMenu MenuData[] =
{
	{ NM_TITLE, "RaspMIDI"                 ,"P",0 ,0             ,(APTR)MEN_PROJECT  },
	{ NM_ITEM ,  "About"                   ,"?",0 ,0             ,(APTR)MEN_ABOUT    },
	{ NM_ITEM ,  "Quit"                    ,"Q",0 ,0             ,(APTR)MEN_QUIT     },

	{ NM_END,NULL,0,0,0,(APTR)0 },
};

//Estructura para lanzar mensajes en requester
struct EasyStruct ShowRequester = {
		sizeof(struct EasyStruct),
		0,
		"",
		"%s",
		"Ok"
	};

struct TagItem MidiItems[] =
    {
		MIDI_Name, (LONG)"RaspMIDI Node",
        MIDI_MsgQueue, 6048L,
		MIDI_SysExSize, 10000L,
		MIDI_RecvSignal, midisig,
        TAG_DONE, NULL
    };

struct TagItem MidiLinkTags[] =
	{
		MLINK_Location, (LONG)UserMidiPort,
		TAG_DONE, NULL
	};



//MAIN
int main(int argc, char *argv[] )
{
	ULONG signals;
	BOOL running = TRUE;
    struct DiskObject *DiskObj;
	STRPTR temp;
	BPTR oldlock;
	strcpy( UserMidiPort, DEFAULTMIDIPORT);


	//LEE LOS TOOLTYPES
	if(argc == 0)
	{
		struct WBStartup  *WbStartup = (struct WBStartup *) argv;
		struct WBArg *Args = WbStartup->sm_ArgList;

		if(WbStartup == NULL) CleanOut("No wbstartup\n");
		oldlock = CurrentDir(Args[0].wa_Lock);
		DiskObj = GetDiskObject((char *)Args[0].wa_Name);
		temp = FindToolType(DiskObj->do_ToolTypes , "MIDIPORT");
		if(temp != NULL)  strcpy( UserMidiPort, temp );
		CurrentDir(oldlock);
	}

	
	CreateSysExMsg();
	
	//Si no tenemos las libs salimos
	if(OpenLib() == FALSE) return 1;

	//señales para el midi
    midisig = AllocSignal(-1);
	if(midisig == -1) CleanOut("Cant get a signal");

	// MIDI Node
    ournode = CreateMidiA(&MidiItems[0]);
	if(!ournode) CleanOut("Can't Create MidiNode");

	// MIDI Link
	outLink = AddMidiLinkA(ournode, MLTYPE_Sender, &MidiLinkTags[0]);
	if(!outLink) CleanOut("Can't Add Output Link");



	//Creamos App y ventana
	app = ApplicationObject,
		MUIA_Application_Title, "RaspMIDI",
		MUIA_Application_Author, "tolkien/goblins",
		MUIA_Application_Version, ver,

		MUIA_Application_Window, window = WindowObject,
			MUIA_Window_Title, "RaspMIDI tolkien/goblins",
			MUIA_Window_ID, MAKE_ID('M','I','D','1'),
            MUIA_Window_Menustrip, menu = MUI_MakeObject(MUIO_MenustripNM,MenuData,MUIO_MenustripNM_CommandKeyCheck),
			MUIA_Window_Activate, TRUE,
			MUIA_Window_Width, 150,
			MUIA_Window_Height, 100,
			MUIA_Window_CloseGadget, TRUE,
			MUIA_Window_DepthGadget, TRUE,
			MUIA_Window_SizeGadget, TRUE,

			WindowContents, VGroup,
			Child, BTreboot = SimpleButton("Reboot"),
			
			Child, VGroup, GroupFrameT("MT-32"),
			Child, BTmt32mode = SimpleButton("MT-32 mode"),
				Child, HGroup, GroupFrameT("Rom type"),
					Child, BTmt32old = SimpleButton("MT-32 old"),
					Child, BTmt32new = SimpleButton("MT-32 new"),
					Child, BTcm32l   = SimpleButton("CM-32l"),
				End,
			End,
			
			Child, VGroup, GroupFrameT("SoundFont"),
			Child, BTsfmode = SimpleButton("SoundFont mode"),
			Child, HGroup, GroupFrameT("SoundFont bank"),
			Child, BTsfless = SimpleButton("<"),
			Child, BTsfbank = Label2(" 0 "),
			Child, BTsfmore = SimpleButton(">"),
			End,
			End,

			End,
		End,
	End;

	if (!app)
	{
		Show("Cant create MUI app", "What a fucking fail!");
		CleanOut(NULL);
		return 2;
	}

	//METHODS
	DoMethod(window,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
		app,2,MUIM_Application_ReturnID,MUIV_Application_ReturnID_Quit);
	DoMethod(BTreboot,   MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, IDreboot);
	DoMethod(BTmt32mode, MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, IDmt32mode);
	
	DoMethod(BTmt32old, MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, IDmt32old);
	DoMethod(BTmt32new, MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, IDmt32new);
	DoMethod(BTcm32l,   MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, IDcm32l);
	DoMethod(BTsfmode,  MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, IDsfmode);
	DoMethod(BTsfless,  MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, IDsfless);	   
	DoMethod(BTsfmore,  MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, IDsfmore);
	
	set(window,MUIA_Window_Open,TRUE);
	//main loop
	while(running)
	{
		ULONG id = DoMethod(app, MUIM_Application_Input, &signals);

		switch(id)
		{
			case MEN_QUIT:
			case MUIV_Application_ReturnID_Quit:
				running = FALSE;
				break;

			case MEN_ABOUT:
				//MUI_Request(app, window, 0, NULL, "Ok", "RaspMIDI 1.0 \ntolkien/goblins");
				MUI_Request(app, window, 0, NULL, "Ok", about);
				break;
		}

		//los menus tiene ID >= 50
		//los botones del 1 en adelante
		if(id >=1  && id <= 6)
		{
			SendSysExMsg(id);
		}
		//comprobamos el cambio de bancos
		if(id >= IDsfless && id <=IDsfmore ) ChangeBank(id);

		if(signals)
		{
			signals = Wait(signals | SIGBREAKF_CTRL_C);
			if(signals & SIGBREAKF_CTRL_C) running = FALSE;
		}
		
	}

	set(window,MUIA_Window_Open,FALSE);


	//Cerramos librerias
	CleanOut(NULL);

	return 0;
}




//FUNCIONES
void Show(APTR titulo, APTR cuerpo)
{
	ULONG iflags = 0;
	ShowRequester.es_Title = titulo;
	EasyRequest(NULL, &ShowRequester, &iflags, cuerpo);
}


int OpenLib()
{
	MUIMasterBase = OpenLibrary(MUIMASTER_NAME, MUIMASTER_VMIN);
	if(!MUIMasterBase) CleanOut("Cant open MUI library\n");

	CamdBase = OpenLibrary("camd.library", 36L);
	if(!CamdBase) CleanOut("Cant open CAMD library\n");

	if(!MUIMasterBase || !CamdBase) return FALSE;
	return TRUE;
}

void CleanOut(BYTE *txt)
{
	if(txt) Show("Error", txt);

    if (outLink) RemoveMidiLink(outLink);
	if (ournode) DeleteMidi(ournode);
	if (midisig != -1) FreeSignal(midisig);

	if(app) MUI_DisposeObject(app);

    if(MUIMasterBase) CloseLibrary(MUIMasterBase);
	if(CamdBase) 	  CloseLibrary(CamdBase);
}


void SendSysExMsg(int ID)
{
	ID--;	//para que empiece a contar de 0
	PutSysEx(outLink, (UBYTE*)&sysexmsg[ID].msg);
}


void CreateSysExMsg()
{
	//REBOOT
	sysexmsg[0].msg[0] = 0xf0;
	sysexmsg[0].msg[1] = 0x7d;
	sysexmsg[0].msg[2] = 0x00;
	sysexmsg[0].msg[3] = 0xf7;
	sysexmsg[0].name = "REBOOT sysexmsg";

	//MT32 MODE
	sysexmsg[1].msg[0] = 0xf0;
	sysexmsg[1].msg[1] = 0x7d;
	sysexmsg[1].msg[2] = 0x03;
	sysexmsg[1].msg[3] = 0x00;
	sysexmsg[1].msg[4] = 0xf7;
	sysexmsg[1].name = "MT-32 mode";
	
	//MT32 MODE (old)
	sysexmsg[2].msg[0] = 0xf0;
	sysexmsg[2].msg[1] = 0x7d;
	sysexmsg[2].msg[2] = 0x01;
	sysexmsg[2].msg[3] = 0x00;
	sysexmsg[2].msg[4] = 0xf7;
	sysexmsg[2].name = "MT-32 old rom";

	//MT32 MODE (new)
	sysexmsg[3].msg[0] = 0xf0;
	sysexmsg[3].msg[1] = 0x7d;
	sysexmsg[3].msg[2] = 0x01;
	sysexmsg[3].msg[3] = 0x01;
	sysexmsg[3].msg[4] = 0xf7;
	sysexmsg[3].name = "MT-32 new rom";

	//CM32 MODE
	sysexmsg[4].msg[0] = 0xf0;
	sysexmsg[4].msg[1] = 0x7d;
	sysexmsg[4].msg[2] = 0x01;
	sysexmsg[4].msg[3] = 0x02;
	sysexmsg[4].msg[4] = 0xf7;
	sysexmsg[4].name = "CM-32l rom";

	//SOUNDFONT MODE
	sysexmsg[5].msg[0] = 0xf0;
	sysexmsg[5].msg[1] = 0x7d;
	sysexmsg[5].msg[2] = 0x03;
	sysexmsg[5].msg[3] = 0x01;
	sysexmsg[5].msg[4] = 0xf7;
	sysexmsg[5].name = "SoundFont mode";

	//SOUNDFONT BANK
	sysexmsg[6].msg[0] = 0xf0;
	sysexmsg[6].msg[1] = 0x7d;
	sysexmsg[6].msg[2] = 0x02;
	sysexmsg[6].msg[3] = 0x00;
	sysexmsg[6].msg[4] = 0xf7;
	sysexmsg[6].name = "SoundFont bank";
}

void ChangeBank(LONG id)
{
//f0 7d 02 xx f7  Soundfont Bank xx=00 To ??
	UBYTE temp = CurrentBank;
	UBYTE tonum[4];

	if(id == IDsfless && CurrentBank > 0)   CurrentBank--;
	if(id == IDsfmore && CurrentBank < 127) CurrentBank++;
	
	sprintf(tonum, "%d", CurrentBank);

	if(temp != CurrentBank)
	{
		sysexmsg[6].msg[3] = CurrentBank;
		set(BTsfbank, MUIA_Text_Contents, tonum);
		SendSysExMsg(7);
	}
}

