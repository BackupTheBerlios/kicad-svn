	/*******************************************************/
	/* Module de generation de la Netliste , selon Formats */
	/*******************************************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"
#include "netlist.h"

#include "protos.h"


/* Routines locales */
static void Write_GENERIC_NetList(wxWindow * frame, const wxString & FullFileName);
static void WriteNetListPCBNEW(wxWindow * frame, FILE *f,
					bool with_pcbnew);
static void WriteNetListCADSTAR(wxWindow * frame, FILE *f);
static void WriteListOfNetsCADSTAR( FILE * f, ObjetNetListStruct *ObjNet );
static void WriteNetListPspice(wxWindow * frame, FILE *f, bool use_netnames);

static void WriteGENERICListOfNets( FILE * f, ObjetNetListStruct *ObjNet );
static void AddPinToComponentPinList(EDA_SchComponentStruct *Component,
								LibDrawPin *PinEntry);
static void FindOthersUnits(EDA_SchComponentStruct *Component );
static int SortPinsByNum(ObjetNetListStruct **Pin1, ObjetNetListStruct **Pin2);
static void EraseDuplicatePins(ObjetNetListStruct **TabPin, int NbrPin);

static void ClearUsedFlags(void);


/* Variable locales */
static int s_SortedPinCount;
static ObjetNetListStruct **s_SortedComponentPinList;




/*******************************************************************/
void WriteNetList(WinEDA_DrawFrame * frame, const wxString & FileNameNL,
				bool use_netnames)
/*******************************************************************/
/* Create the netlist file ( Format is given by g_NetFormat )
	bool use_netnames is used only for Spice netlist
*/
{
FILE *f = NULL;

	if ( g_NetFormat < NET_TYPE_CUSTOM1 )
	{
		if ((f = fopen(FileNameNL, "wt")) == NULL)
		{
			wxString msg =  _("Failed to create file ") + FileNameNL;
			DisplayError(frame, msg);
			return;
		}
	}

wxBusyCursor Busy;

	switch ( g_NetFormat )
	{
		case NET_TYPE_PCBNEW :
			WriteNetListPCBNEW(frame, f, TRUE);
			fclose(f);
			break;

		case NET_TYPE_ORCADPCB2 :
			WriteNetListPCBNEW(frame, f, FALSE);
			fclose(f);
			break;

		case NET_TYPE_CADSTAR :
			WriteNetListCADSTAR(frame, f);
			fclose(f);
			break;

		case NET_TYPE_SPICE :
			WriteNetListPspice(frame, f, use_netnames);
			fclose(f);
			break;

		case NET_TYPE_CUSTOM1 :
		case NET_TYPE_CUSTOM2 :
		case NET_TYPE_CUSTOM3 :
		case NET_TYPE_CUSTOM4 :
		case NET_TYPE_CUSTOM5 :
		case NET_TYPE_CUSTOM6 :
		case NET_TYPE_CUSTOM7 :
		case NET_TYPE_CUSTOM8 :
			Write_GENERIC_NetList(frame, FileNameNL);
			break;

		default:
			DisplayError(frame, "WriteNetList() err: Unknown Netlist Format");
			break;
	}
}


/****************************************************************************/
static EDA_SchComponentStruct * FindNextComponentAndCreatPinList(
		EDA_BaseStruct * DrawList)
/****************************************************************************/
/*	Find a "suitable" component from the DrawList
	build its pin list s_SortedComponentPinList.
	The list is sorted by pin num
	A suitable component is a "new" real component (power symbols are not considered)

	alloc memory for s_SortedComponentPinList if s_SortedComponentPinList == NULL
	Must be deallocated by the user
*/
{
EDA_SchComponentStruct * Component = NULL;
EDA_LibComponentStruct *Entry;
LibEDA_BaseStruct *DEntry;
	
	s_SortedPinCount = 0;

	for ( ; DrawList != NULL; DrawList = DrawList->Next() )
	{
		if ( DrawList->m_StructType != DRAW_LIB_ITEM_STRUCT_TYPE ) continue;
		Component = (EDA_SchComponentStruct *) DrawList;
	
		 /* already tested ? : */
		if( Component->m_FlagControlMulti == 1 ) continue; /* yes */
	
		Entry = FindLibPart(Component->m_ChipName, "", FIND_ROOT);
		if( Entry  == NULL) continue;
			
		/* Power symbol and other component which have the reference starting by
		"#" are not included in netlist (pseudo components) */
		if( Component->m_Field[REFERENCE].m_Text[0] == '#' ) continue;
	
		/* Create the pin table for this component */
		int ii = sizeof(ObjetNetListStruct) * MAXPIN;
		if ( s_SortedComponentPinList == NULL )
			s_SortedComponentPinList = (ObjetNetListStruct **) MyMalloc(ii);
		memset(s_SortedComponentPinList, 0 , ii);
	
		DEntry = Entry->m_Drawings;
		for ( ;DEntry != NULL; DEntry = DEntry->Next())
		{
			if ( DEntry->m_StructType != COMPONENT_PIN_DRAW_TYPE) continue;
			if( DEntry->m_Unit &&
				(DEntry->m_Unit != Component->m_Multi) ) continue;
			if( DEntry->m_Convert &&
				(DEntry->m_Convert != Component->m_Convert)) continue;
			{
				AddPinToComponentPinList(Component, (LibDrawPin*)DEntry);
			}
		}
	
		Component->m_FlagControlMulti = 1;
	
		if (Entry->m_UnitCount > 1) FindOthersUnits(Component);
	
		/* Tri sur le numero de Pin de TabListePin */
		qsort(s_SortedComponentPinList, s_SortedPinCount, sizeof(ObjetNetListStruct*),
				  (int (*)(const void *, const void *)) SortPinsByNum);
	
		/* Elimination des Pins redondantes du s_SortedComponentPinList */
		EraseDuplicatePins(s_SortedComponentPinList, s_SortedPinCount);

		return Component;
	}
	
	return NULL;
}

/**************************************************************************************/
static wxString ReturnPinNetName(ObjetNetListStruct *Pin,
	const wxString& DefaultFormatNetname)
/**************************************************************************************/
/* Return the net name for the pin Pin.
	Net name is:
	"?" if pin not connected
	"netname" for global net (like gnd, vcc ..
	"netname_sheetnumber" for the usual nets
*/
{
int netcode = Pin->m_NetCode;
wxString NetName;
	
	if( (netcode == 0 ) || ( Pin->m_FlagOfConnection != CONNECT ) )
	{
		return NetName;
	}
	else
	{
		int jj;
		for (jj = 0; jj < g_NbrObjNet; jj++)
		{
			if( g_TabObjNet[jj].m_NetCode != netcode) continue;
			if( ( g_TabObjNet[jj].m_Type != NET_GLOBLABEL) &&
				( g_TabObjNet[jj].m_Type != NET_LABEL) &&
				( g_TabObjNet[jj].m_Type != NET_PINLABEL) ) continue;

			NetName = g_TabObjNet[jj].m_Label;
			break;
		}
		
		if( NetName != "" )
		{
			if( g_TabObjNet[jj].m_Type != NET_PINLABEL )
				NetName << "_" << g_TabObjNet[jj].m_SheetNumber;
		}
		else
		{
			NetName.Printf(DefaultFormatNetname.GetData(), netcode);
		}
	}
	return NetName;
}

/***********************************************************************/
void Write_GENERIC_NetList(wxWindow * frame, const wxString & FullFileName)
/***********************************************************************/
/* Create a generic netlist, and call an external netlister
	to change the netlist syntax and create the file
*/
{
wxString Line, FootprintName;
BASE_SCREEN *CurrScreen;
EDA_BaseStruct *DrawList;
EDA_SchComponentStruct *Component;
wxString netname;
int ii;
FILE * tmpfile;
wxString TmpFullFileName = FullFileName;	
	
	ChangeFileNameExt(TmpFullFileName, ".tmp");

	if ((tmpfile = fopen(TmpFullFileName, "wt")) == NULL)
	{
		wxString msg =  _("Failed to create file ") + TmpFullFileName;
		DisplayError(frame, msg);
		return;
	}
	
	ClearUsedFlags();	/* Reset the flags FlagControlMulti in all schematic files*/
	fprintf( tmpfile, "$BeginNetlist\n" );

	/* Create netlist module section */
	fprintf( tmpfile, "$BeginComponentList\n" );
	for(CurrScreen = ScreenSch; CurrScreen != NULL; CurrScreen = (BASE_SCREEN*)CurrScreen->Pnext )
	{
		for ( DrawList = CurrScreen->EEDrawList; DrawList != NULL; DrawList = DrawList->Pnext )
		{
			DrawList = Component = FindNextComponentAndCreatPinList(DrawList);
			if ( Component == NULL ) break;	// No component left

			FootprintName.Empty();
			if( ! Component->m_Field[FOOTPRINT].IsVoid() )
			{
				FootprintName = Component->m_Field[FOOTPRINT].m_Text;
				FootprintName.Replace(" ", "_");
			}

			fprintf( tmpfile, "\n$BeginComponent\n" );
			fprintf(tmpfile, "TimeStamp=%8.8lX\n", Component->m_TimeStamp);
			fprintf(tmpfile, "Footprint=%s\n", FootprintName.GetData());
			Line =  "Reference=" + Component->m_Field[REFERENCE].m_Text + "\n";
			Line.Replace(" ", "_");
			fprintf(tmpfile, Line.GetData());

			Line = Component->m_Field[VALUE].m_Text;
			Line.Replace(" ", "_");
			fprintf(tmpfile, "Value=%s\n", Line.GetData());

			Line = Component->m_ChipName;
			Line.Replace(" ", "_");
			fprintf(tmpfile, "Libref=%s\n", Line.GetData());

			// Write pin list:
			fprintf( tmpfile, "$BeginPinList\n" );
			for (ii = 0; ii < s_SortedPinCount; ii++)
			{
				ObjetNetListStruct *Pin = s_SortedComponentPinList[ii];
				if( ! Pin ) continue;
				netname = ReturnPinNetName(Pin, "$-%.6d");
				if ( netname == "" ) netname = "?";
				fprintf( tmpfile,"%.4s=%s\n",(char *)&Pin->m_PinNum, netname.GetData());
			}

			fprintf( tmpfile, "$EndPinList\n" );
			fprintf( tmpfile, "$EndComponent\n" );
		}
	}
	
	MyFree(s_SortedComponentPinList);
	s_SortedComponentPinList = NULL;
	
	fprintf( tmpfile, "$EndComponentList\n" );

	fprintf( tmpfile, "\n$BeginNets\n" );
	WriteGENERICListOfNets( tmpfile, g_TabObjNet );
	fprintf( tmpfile, "$EndNets\n" );
	fprintf( tmpfile, "\n$EndNetlist\n" );
	fclose(tmpfile);
	
	// Call the external module (plug in )
	
	if ( g_NetListerCommandLine == "" ) return;

wxString CommandFile;
	if ( wxIsAbsolutePath(g_NetListerCommandLine) )
		CommandFile = g_NetListerCommandLine;
	else CommandFile = FindKicadFile(g_NetListerCommandLine);
	
	CommandFile += " " + TmpFullFileName;
	CommandFile += " " + FullFileName;

	wxExecute(CommandFile, wxEXEC_SYNC);
	
}


/*******************************/
static void ClearUsedFlags(void)
/*******************************/
/* Clear flag FlagControlMulti, used in netlist generation */
{
BASE_SCREEN *CurrScreen;
EDA_BaseStruct *DrawList;
	
	for(CurrScreen = ScreenSch; CurrScreen != NULL; CurrScreen = CurrScreen->Next() )
	{
		DrawList = CurrScreen->EEDrawList;
		while ( DrawList )
		{
			if( DrawList->m_StructType == DRAW_LIB_ITEM_STRUCT_TYPE)
			{
				EDA_SchComponentStruct * Component = (EDA_SchComponentStruct *) DrawList;
				Component->m_FlagControlMulti = 0;
			}
			DrawList = DrawList->Pnext;
		}
	}
}

/*************************************************************/
static void WriteNetListPspice(wxWindow * frame, FILE *f,
		 bool use_netnames)
/*************************************************************/
/* Routine de generation du fichier netliste ( Format PSPICE )
	si use_netnames = TRUE
		les nodes sont identifies par le netname
	sinon	les nodes sont identifies par le netnumber

	tous les textes graphiques commençant par [.-+]pspice ou  [.-+]gnucap
	sont considérés comme des commandes a placer dans la netliste
		[.-]pspice ou gnucap sont en debut
		+pspice et +gnucap sont en fin de netliste
*/
{
char Line[1024];
BASE_SCREEN *CurrScreen;
EDA_BaseStruct *DrawList;
EDA_SchComponentStruct *Component;
int ii, nbitems;
const char * text;
wxArrayString SpiceCommandAtBeginFile, SpiceCommandAtEndFile;
wxString msg;
#define BUFYPOS_LEN 4
char bufnum[BUFYPOS_LEN+1];

	DateAndTime(Line);
	fprintf( f, "* %s (Spice format) creation date: %s\n\n", NETLIST_HEAD_STRING, Line );

	/* Create text list starting by [.-]pspice , or [.-]gnucap (simulator commands) */
	/* and create text list starting by [+]pspice , or [+]gnucap (simulator commands) */
	bufnum[BUFYPOS_LEN] = 0;
	for(CurrScreen = ScreenSch; CurrScreen != NULL; CurrScreen = (BASE_SCREEN*)CurrScreen->Pnext )
	{
		for ( DrawList = CurrScreen->EEDrawList; DrawList != NULL; DrawList = DrawList->Pnext )
		{
			char ident;
			if ( DrawList->m_StructType != DRAW_TEXT_STRUCT_TYPE ) continue;
			#define DRAWTEXT ((DrawTextStruct *) DrawList)
			text = DRAWTEXT->m_Text.GetData(); if ( ! text ) continue;
			ident = *text;
			if ( ident != '.' && ident != '-' && ident != '+' ) continue;
			if( (strnicmp(text + 1, "pspice",6 ) == 0 ) ||
				(strnicmp(text + 1, "gnucap",6 ) == 0 ) )
			{
				/* Put the Y position as an ascii string, for sort by verical position,
				using usual sort string by alphabetic value */
				int ypos = DRAWTEXT->m_Pos.y;
				for ( ii = 0; ii < BUFYPOS_LEN; ii++ )
				{
					bufnum[BUFYPOS_LEN-1-ii] = (ypos & 63) + ' '; ypos >>= 6;
				}
				msg.Printf("%s %s",bufnum, text + 8);	// First BUFYPOS_LEN char are the Y position
				if ( ident == '+' ) SpiceCommandAtEndFile.Add(msg);
				else SpiceCommandAtBeginFile.Add(msg);
			}
		}
	}

	/* Print texts starting by [.-]pspice , ou [.-]gnucap (of course, without the Y position string)*/
	nbitems = SpiceCommandAtBeginFile.GetCount();
	if ( nbitems )
	{
		SpiceCommandAtBeginFile.Sort();
		for(ii= 0; ii < nbitems; ii++ )
			fprintf( f, "%s\n", SpiceCommandAtBeginFile[ii].GetData() + BUFYPOS_LEN + 1 );
	}
	fprintf( f, "\n" );


	/* Create component list */
	ClearUsedFlags();	/* Reset the flags FlagControlMulti in all schematic files*/
	for(CurrScreen = ScreenSch; CurrScreen != NULL; CurrScreen = (BASE_SCREEN*)CurrScreen->Pnext )
	{
		for( DrawList = CurrScreen->EEDrawList; DrawList != NULL; DrawList = DrawList->Pnext )
		{
			DrawList = Component = FindNextComponentAndCreatPinList(DrawList);
			if ( Component == NULL ) break;

			fprintf(f, "%s ", Component->m_Field[REFERENCE].m_Text.GetData());

			// Write pin list:
			for (ii = 0; ii < s_SortedPinCount; ii++)
			{
				ObjetNetListStruct *Pin = s_SortedComponentPinList[ii];
				if( ! Pin ) continue;
				wxString NetName = ReturnPinNetName(Pin, "N-%.6d");
				if ( NetName == "" ) NetName = "?";
				if( use_netnames) fprintf( f," %s", NetName.GetData());
				else	// Use number for net names (with net number = 0 for "GND"
				{
					// NetName = "0" is "GND" net for Spice
					if( NetName == "0" || NetName == "GND" ) fprintf( f," 0");
					else fprintf( f," %d", Pin->m_NetCode);
				}
			}
			fprintf(f, " %s\n",  Component->m_Field[VALUE].m_Text.GetData());
		}
	}

	MyFree(s_SortedComponentPinList);
	s_SortedComponentPinList = NULL;
	
	/* Print texts starting by [+]pspice , ou [+]gnucap */
	nbitems = SpiceCommandAtEndFile.GetCount();
	if ( nbitems )
	{
		fprintf( f, "\n" );
		SpiceCommandAtEndFile.Sort();
		for(ii= 0; ii < nbitems; ii++ )
			fprintf( f, "%s\n", SpiceCommandAtEndFile[ii].GetData() + BUFYPOS_LEN +1);
	}

	fprintf( f, "\n.end\n" );
}

/*********************************************************************/
static void WriteNetListPCBNEW(wxWindow * frame, FILE *f, bool with_pcbnew)
/*********************************************************************/
/* Routine de generation du fichier netliste ( Format ORCAD PCB 2 ameliore )
	si with_pcbnew = FALSE
		format PCBNEW (OrcadPcb2 + commentaires et liste des nets)
	si with_pcbnew = FALSE
		Format ORCADPCB2 strict
*/
{
wxString Line, FootprintName;
char Buf[256];
BASE_SCREEN *CurrScreen;
EDA_BaseStruct *DrawList;
EDA_SchComponentStruct *Component;
int ii;

	DateAndTime(Buf);
	if ( with_pcbnew )
		fprintf( f, "# %s generee le  %s\n(\n", NETLIST_HEAD_STRING, Buf);
	else fprintf( f, "( { %s generee le  %s }\n", NETLIST_HEAD_STRING, Buf );


	/* Create netlist module section */
	ClearUsedFlags();	/* Reset the flags FlagControlMulti in all schematic files*/
	for(CurrScreen = ScreenSch; CurrScreen != NULL; CurrScreen = (BASE_SCREEN*)CurrScreen->Pnext )
	{
		for ( DrawList = CurrScreen->EEDrawList; DrawList != NULL; DrawList = DrawList->Pnext )
		{
			DrawList = Component = FindNextComponentAndCreatPinList(DrawList);
			if ( Component == NULL ) break;

			if( ! Component->m_Field[FOOTPRINT].IsVoid() )
			{
				FootprintName = Component->m_Field[FOOTPRINT].m_Text;
				FootprintName.Replace( " ", "_");
			}
			else FootprintName = "$noname";

			Line = Component->m_Field[REFERENCE].m_Text;
			Line.Replace( " ", "_");
			fprintf(f, " ( %8.8lX %s %s",
						Component->m_TimeStamp,
						FootprintName.GetData(), Line.GetData());

			Line = Component->m_Field[VALUE].m_Text;
			Line.Replace( " ", "_");
			fprintf(f, " %s", Line.GetData());

			if ( with_pcbnew )	// Add the lib name for this component
			{
				Line = Component->m_ChipName;
				Line.Replace( " ", "_");
				fprintf(f, " {Lib=%s}", Line.GetData());
			}
			fprintf(f, "\n");

			// Write pin list:
			for (ii = 0; ii < s_SortedPinCount; ii++)
			{
				ObjetNetListStruct *Pin = s_SortedComponentPinList[ii];
				if( ! Pin ) continue;
				wxString netname = ReturnPinNetName(Pin, "N-%.6d");
				if ( netname == "" ) netname = " ?";
				fprintf( f,"  ( %4.4s %s )\n",(char *)&Pin->m_PinNum, netname.GetData());
			}

			fprintf(f, " )\n");
		}
	}
	fprintf( f, ")\n*\n");

	MyFree(s_SortedComponentPinList);
	s_SortedComponentPinList = NULL;

	if ( with_pcbnew )
	{
		fprintf( f, "{ Pin List by Nets\n" );
		WriteGENERICListOfNets( f, g_TabObjNet );
		fprintf( f, "}\n" );
		fprintf( f, "#End\n" );
	}
}


/*************************************************************************************/
static void AddPinToComponentPinList( EDA_SchComponentStruct *Component, LibDrawPin *Pin)
/*************************************************************************************/
/* Add a new pin description in the pin list s_SortedComponentPinList
	a pin description is a pointer to the corresponding structure
	created by BuildNetList() in the table g_TabObjNet
*/
{
int ii;

	/* Search the PIN description for Pin in g_TabObjNet*/
	for (ii = 0; ii < g_NbrObjNet; ii++)
	{
		if ( g_TabObjNet[ii].m_Type != NET_PIN) continue;
		if ( g_TabObjNet[ii].m_Link != Component) continue;
		if ( g_TabObjNet[ii].m_PinNum != Pin->m_PinNum) continue;
		{
			s_SortedComponentPinList[s_SortedPinCount] = & g_TabObjNet[ii];
			s_SortedPinCount++;
			if (s_SortedPinCount >= MAXPIN)
			{
				/* Log message for Internal error */
				DisplayError(NULL, "AddPinToComponentPinList err: MAXPIN reached"); return;
			}
		}
	}
}


/**********************************************************************/
static void EraseDuplicatePins(ObjetNetListStruct **TabPin, int NbrPin)
/**********************************************************************/
/*
	Routine qui elimine les Pins de meme Numero de la liste des objets
	(Liste des Pins d'un boitier) passee en entree
	Ces pins redondantes proviennent des pins (alims... ) communes a plusieurs
	elements d'un boitier a multiple parts.
*/
{
int ii, jj;

	for (ii = 0; ii < NbrPin-1; ii++)
		{
		if( TabPin[ii] == NULL) continue;	/* Deja supprime */
		if( TabPin[ii]->m_PinNum != TabPin[ii+1]->m_PinNum) continue;
		/* 2 Pins doublees */
		for( jj = ii+1; jj < NbrPin ; jj++ )
			{
			if( TabPin[ii]->m_PinNum != TabPin[jj]->m_PinNum) break;
			TabPin[jj] = NULL;
			}
		}
}



/**********************************************************************/
static void FindOthersUnits(EDA_SchComponentStruct *Component )
/**********************************************************************/

/* Recherche les autres parts du boitier auquel appartient la part Component,
	pour les boitiers a parts multiples.
	Appelle AddPinToComponentPinList() pour classer les pins des parts trouvees
*/
{
EDA_BaseStruct *DrawList;
EDA_SchComponentStruct *Component2;
EDA_LibComponentStruct *Entry;
LibEDA_BaseStruct *DEntry;
BASE_SCREEN * screen;

	for( screen = ScreenSch; screen != NULL; screen = (BASE_SCREEN*)screen->Pnext )
		{
		DrawList = screen->EEDrawList;
		while ( DrawList )
			{
			switch( DrawList->m_StructType )
				{
				case DRAW_LIB_ITEM_STRUCT_TYPE :
					Component2 = (EDA_SchComponentStruct *) DrawList;

					if ( Component2->m_FlagControlMulti == 1 )  break;

					if ( Component2->m_Field[REFERENCE].m_Text.CmpNoCase(
							  Component->m_Field[REFERENCE].m_Text) != 0 )
						break;

					Entry = FindLibPart(Component2->m_ChipName, "", FIND_ROOT);
 					if( Entry  == NULL) break;
					if( Component2->m_Field[REFERENCE].m_Text[0] == '#' ) break;

					if (Entry->m_Drawings != NULL)
						{
						DEntry = Entry->m_Drawings;
						for ( ;DEntry != NULL; DEntry = DEntry->Next())
							{
							if ( DEntry->m_StructType != COMPONENT_PIN_DRAW_TYPE) continue;
							if( DEntry->m_Unit &&
								(DEntry->m_Unit != Component2->m_Multi) ) continue;
							if( DEntry->m_Convert &&
								(DEntry->m_Convert != Component2->m_Convert)) continue;
								{
								AddPinToComponentPinList(Component2, (LibDrawPin*)DEntry);
								}
							}
						}
					Component2->m_FlagControlMulti = 1;
					break;

				default: break;
				}
			DrawList = DrawList->Pnext;
			}
		}
}


/**************************************************************************/
static int SortPinsByNum(ObjetNetListStruct **Pin1, ObjetNetListStruct **Pin2)
/**************************************************************************/
/* Routine de comparaison pour le tri des pins par numero croissant
	du tableau des pins s_SortedComponentPinList par qsort()
*/
{
ObjetNetListStruct * Obj1, * Obj2;
int Num1, Num2;
char Line[5];

	Obj1 = *Pin1; Obj2 = *Pin2;
	Num1 = Obj1->m_PinNum; Num2 = Obj2->m_PinNum;
	Line[4] = 0; memcpy(Line, &Num1, 4 ); Num1 = atoi(Line);
	memcpy(Line, &Num2, 4 ); Num2 = atoi(Line);
	return (Num1 - Num2);
}


/*************************************************************************/
static void WriteGENERICListOfNets( FILE * f, ObjetNetListStruct *ObjNet )
/*************************************************************************/
/* Ecrit dans le fichier f la liste des nets ( classee par NetCodes ), et des
elements qui y sont connectes
*/
{
int ii, jj;
int NetCode, LastNetCode = -1;
EDA_SchComponentStruct * Cmp;
const char * NetName, * CmpRef;
wxString NetcodeName;

	for (ii = 0; ii < g_NbrObjNet; ii++)
	{
		if( (NetCode = ObjNet[ii].m_NetCode) != LastNetCode )	// New net found, write net id;
		{
			NetName = NULL;
			for (jj = 0; jj < g_NbrObjNet; jj++)	// Find a label (if exists) for this net
			{
				if( ObjNet[jj].m_NetCode != NetCode) continue;
				if( ( ObjNet[jj].m_Type != NET_GLOBLABEL) &&
					( ObjNet[jj].m_Type != NET_LABEL) &&
					( ObjNet[jj].m_Type != NET_PINLABEL) ) continue;

				NetName = g_TabObjNet[jj].m_Label; break;
			}

			NetcodeName.Printf("Net %d ", NetCode);
			NetcodeName += "\"";
			if( NetName)
			{
				NetcodeName += NetName;
				if( g_TabObjNet[jj].m_Type != NET_PINLABEL)	// usual net name, add it the sheet number
					NetcodeName << "_" << g_TabObjNet[jj].m_SheetNumber;
			}
			NetcodeName += "\"";
			fprintf( f, "%s\n", NetcodeName.GetData());

			LastNetCode = NetCode;
		}

		// Print the pin list for this net:
		switch ( ObjNet[ii].m_Type )
		{
			case NET_PIN :
				Cmp = (EDA_SchComponentStruct*) ObjNet[ii].m_Link;
				CmpRef = Cmp->m_Field[REFERENCE].m_Text.GetData();
				if ( *CmpRef == '#' )	//Pseudo component do not wxrite it
					break;
				fprintf( f, " %s %.4s\n", CmpRef, (char*) & ObjNet[ii].m_PinNum);
				break;

			default:
				break;
		}
	}
}



/* Generation des netlistes au format CadStar */
wxString StartLine(".");

/*********************************************************/
static void WriteNetListCADSTAR(wxWindow * frame, FILE *f)
/*********************************************************/
/* Routine de generation du fichier netliste ( Format CADSTAR )
Entete:
..HEA
..TIM 2004 07 29 16 22 17
..APP "Cadstar RINF Output - Version 6.0.2.3"
..UNI INCH 1000.0 in
..TYP FULL

liste des composants:
..ADD_COM    X1         "CNT D41612 (48PTS MC CONTOUR)"
..ADD_COM    U2         "74HCT245D" "74HCT245D"

Connexions:
..ADD_TER    RR2        6          "$42"
..TER        U1         100
            CA         6

..ADD_TER    U2         6          "$59"
..TER        U7         39
            U6         17
            U1         122

..ADD_TER    P2         1          "$9"
..TER        T3         1
            U1         14
*/
{
wxString StartCmpDesc = StartLine + "ADD_COM";
wxString msg;
wxString FootprintName;
char Line[1024];
BASE_SCREEN *CurrScreen;
EDA_BaseStruct *DrawList;
EDA_SchComponentStruct *Component;

	fprintf( f, "%sHEA\n", StartLine.GetData());
	DateAndTime(Line);
	fprintf( f, "%sTIM %s\n", StartLine.GetData(), Line );
	fprintf( f, "%sAPP \"%s\"\n", StartLine.GetData(), Main_Title.GetData() );
	fprintf( f, "\n");

	/* Create netlist module section */
	ClearUsedFlags();	/* Reset the flags FlagControlMulti in all schematic files*/
	for(CurrScreen = ScreenSch; CurrScreen != NULL; CurrScreen = (BASE_SCREEN*)CurrScreen->Pnext )
	{
		for ( DrawList = CurrScreen->EEDrawList; DrawList != NULL; DrawList = DrawList->Pnext )
		{
			DrawList = Component = FindNextComponentAndCreatPinList(DrawList);
			if ( Component == NULL ) break;

			if( ! Component->m_Field[FOOTPRINT].IsVoid() )
			{
				FootprintName = Component->m_Field[FOOTPRINT].m_Text;
				FootprintName.Replace(" ","_");
			}
			else FootprintName = "$noname";

			msg = Component->m_Field[REFERENCE].m_Text;
			msg.Replace( " ", "_");
			fprintf(f, "%s     %s",StartCmpDesc.GetData(), msg.GetData());

			msg = Component->m_Field[VALUE].m_Text;
			msg.Replace(" ","_");
			fprintf(f, "     \"%s\"", msg.GetData());
			fprintf(f, "\n");

			}
		}
	fprintf( f, "\n");

	MyFree(s_SortedComponentPinList);
	s_SortedComponentPinList = NULL;

	WriteListOfNetsCADSTAR( f, g_TabObjNet );

	fprintf( f, "\n%sEND\n", StartLine.GetData() );
}

/*************************************************************************/
static void WriteListOfNetsCADSTAR( FILE * f, ObjetNetListStruct *ObjNet )
/*************************************************************************/
/* Ecrit dans le fichier f la liste des nets ( classee par NetCodes ), et des
pins qui y sont connectes
format:
..ADD_TER    RR2        6          "$42"
..TER        U1         100
            CA         6
*/
{
wxString InitNetDesc = StartLine + "ADD_TER";
wxString StartNetDesc = StartLine + "TER";
wxString NetcodeName, InitNetDescLine;
int ii, jj, print_ter = 0;
int NetCode, LastNetCode = -1;
EDA_SchComponentStruct * Cmp;
wxString NetName;

	for (ii = 0; ii < g_NbrObjNet; ii++) ObjNet[ii].m_Flag = 0;

	for (ii = 0; ii < g_NbrObjNet; ii++)
	{
		// Get the NetName of the current net :
		if( (NetCode = ObjNet[ii].m_NetCode) != LastNetCode )
		{
			NetName.Empty();
			for (jj = 0; jj < g_NbrObjNet; jj++)
			{
				if( ObjNet[jj].m_NetCode != NetCode) continue;
				if( ( ObjNet[jj].m_Type != NET_GLOBLABEL) &&
					( ObjNet[jj].m_Type != NET_LABEL) &&
					( ObjNet[jj].m_Type != NET_PINLABEL) ) continue;

				NetName = ObjNet[jj].m_Label; break;
			}
			NetcodeName = "\"";
			if( ! NetName.IsEmpty() )
			{
				NetcodeName += NetName;
				if( g_TabObjNet[jj].m_Type != NET_PINLABEL)
					NetcodeName << "_" << g_TabObjNet[jj].m_SheetNumber;
			}
			else	// this net has no name: create a default name $<net number>
				NetcodeName <<  "$" << NetCode;
			NetcodeName += "\"";
			LastNetCode = NetCode;
			print_ter = 0;
		}


		if ( ObjNet[ii].m_Type != NET_PIN ) continue;

		if ( ObjNet[ii].m_Flag != 0 ) continue;

		Cmp = (EDA_SchComponentStruct*) ObjNet[ii].m_Link;

		if ( Cmp->m_Field[REFERENCE].m_Text[0] == '#' )
			continue;	// Pseudo composant (symboles d'alims)

		switch ( print_ter )
		{
			case 0:
				InitNetDescLine.Printf( "\n%s   %s   %.4s     %s",
						InitNetDesc.GetData(),
						Cmp->m_Field[REFERENCE].m_Text.GetData(),
						(char*) & ObjNet[ii].m_PinNum,NetcodeName.GetData());
				print_ter++;
				break;

			case 1:
				fprintf( f, "%s\n", InitNetDescLine.GetData());
				fprintf( f, "%s       %s   %.4s\n", StartNetDesc.GetData(),
						Cmp->m_Field[REFERENCE].m_Text.GetData(),
						(char*) & ObjNet[ii].m_PinNum );
				print_ter++;
				break;

			default:
			fprintf( f, "            %s   %.4s\n",
					Cmp->m_Field[REFERENCE].m_Text.GetData(),
					(char*) & ObjNet[ii].m_PinNum );
				break;
		}

		ObjNet[ii].m_Flag = 1;
		// Recherche des pins redondantes et mise a 1 de m_Flag,
		//	pour ne pas generer plusieurs fois la connexion
		for (jj = ii + 1; jj < g_NbrObjNet; jj++)
		{
			if( ObjNet[jj].m_NetCode != NetCode) break;
			if( ObjNet[jj].m_Type != NET_PIN ) continue;
		EDA_SchComponentStruct * tstcmp = (EDA_SchComponentStruct *) ObjNet[jj].m_Link;
			if( Cmp->m_Field[REFERENCE].m_Text != tstcmp->m_Field[REFERENCE].m_Text ) continue;

			if ( ObjNet[jj].m_PinNum == ObjNet[ii].m_PinNum ) ObjNet[jj].m_Flag = 1;
		}
	}
}

