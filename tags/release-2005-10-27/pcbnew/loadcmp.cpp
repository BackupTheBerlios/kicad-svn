		/**********************************************/
		/* Footprints selection and loading functions */
		/**********************************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "pcbnew.h"

#include "protos.h"

class ModList
{
public:
	ModList * Next;
	char * Name, *Doc, *KeyWord;

public:
	ModList(void)
	{
		Next = NULL;
		Name = Doc = KeyWord = NULL;
	}
	~ModList(void)
	{
		if(Name) free(Name);
		if(Doc) free(Doc);
		if(KeyWord) free(KeyWord);
	}
};

/* Fonctions locales */
static void DisplayCmpDoc(wxString & Name);
static void ReadDocLib(const wxString & ModLibName );
/*****/

/* variables locales */
static ModList * MList;


/***************************************************************************/
void WinEDA_ModuleEditFrame::Load_Module_Module_From_BOARD( MODULE * Module )
/***************************************************************************/
{
MODULE * NewModule;

	if ( Module == NULL )
		{
		if (m_Parent->m_PcbFrame == NULL) return;
		if (m_Parent->m_PcbFrame->m_Pcb == NULL) return;
		if (m_Parent->m_PcbFrame->m_Pcb->m_Modules == NULL) return;

		Module = Select_1_Module_From_BOARD(m_Parent->m_PcbFrame->m_Pcb);
		}

	if ( Module == NULL ) return;

	m_CurrentScreen->m_CurrentItem = NULL;

	Clear_Pcb(NULL, TRUE);

	m_Pcb->m_Status_Pcb = 0;
	NewModule = new MODULE(m_Pcb);
	NewModule->Copy(Module);
	NewModule->m_Link = Module->m_TimeStamp;

	Module = NewModule;
	Module->m_Parent = m_Pcb;
	Module->Pback = m_Pcb->m_Modules; Module->Pnext = NULL;
	m_Pcb->m_Modules = Module;

	Module->m_Flags = 0;

	build_liste_pads();

	m_CurrentScreen->m_Curseur.x = m_CurrentScreen->m_Curseur.y = 0;
	Place_Module(Module, NULL);
	if( Module->m_Layer != CMP_N) Change_Side_Module(Module, NULL);
	Rotate_Module(NULL, Module, 0, FALSE);
	m_CurrentScreen->ClrModify();
	Zoom_Automatique(TRUE);
}

/****************************************************************************/
MODULE * WinEDA_BasePcbFrame::Load_Module_From_Library(const wxString & library,
			wxDC * DC)
/****************************************************************************/
/* Permet de charger un module directement a partir de la librairie */
{
MODULE * module;
wxPoint curspos = m_CurrentScreen->m_Curseur;
wxString ModuleName, keys;
static wxArrayString HistoryList;

	DrawPanel->m_IgnoreMouseEvents = TRUE;
	
	/* Ask for a component name or key words */
	ModuleName = GetComponentName(this, HistoryList, _("Module name:"), NULL);
	ModuleName.MakeUpper();
	if( ModuleName == "" )	/* Cancel command */
	{
		DrawPanel->m_IgnoreMouseEvents = FALSE;
		DrawPanel->MouseToCursorSchema();
		return NULL;
	}


	if( ModuleName[0] == '=' )	// Selection by keywords
	{
		keys = ModuleName.AfterFirst('=');
		ModuleName = Select_1_Module_From_List(this, library, "", keys);
		if( ModuleName == "" )	/* Cancel command */
		{
			DrawPanel->m_IgnoreMouseEvents = FALSE;
			DrawPanel->MouseToCursorSchema();
			return NULL;
		}
	}

	else if( (ModuleName.Contains("?")) || (ModuleName.Contains("*")) ) // Selection wild card
	{
		ModuleName = Select_1_Module_From_List(this, library, ModuleName, "");
		if( ModuleName == "" )
		{
			DrawPanel->m_IgnoreMouseEvents = FALSE;
			DrawPanel->MouseToCursorSchema();
			return NULL;	/* annulation de commande */
		}
	}

	module = Get_Librairie_Module(this, library, ModuleName, TRUE);

	m_CurrentScreen->m_Curseur = curspos;
	DrawPanel->m_IgnoreMouseEvents = FALSE;
	DrawPanel->MouseToCursorSchema();

	if( module )
	{
		AddHistoryComponentName(HistoryList, ModuleName);

		module->m_Flags = IS_NEW;
		module->m_Link = 0;
		module->m_TimeStamp = GetTimeStamp();
		m_Pcb->m_Status_Pcb = 0 ;
		module->SetPosition(curspos);
		build_liste_pads();

		module->Draw(DrawPanel, DC, wxPoint(0,0), GR_OR);
	}

	return module;
}

/*******************************************************************************/
MODULE * WinEDA_BasePcbFrame::Get_Librairie_Module(wxWindow * winaff,
		const wxString & library, const wxString & ModuleName, bool show_msg_err)
/*******************************************************************************/
/*
	Analyse les LIBRAIRIES pour trouver le module demande
	Si ce module est trouve, le copie en memoire, et le
	chaine en fin de liste des modules
		- Entree:
			name_cmp = nom du module
		- Retour:
			Pointeur sur le nouveau module.
*/
{
int LineNum, Found= 0;
wxString fulllibname;
char Line[512], Name[512];
wxString ComponentName, msg;
MODULE * Module;
MODULE * NewModule;
FILE * lib_module = NULL;
unsigned ii;

	ComponentName = ModuleName;

	/* Calcul de l'adresse du dernier module: */
	Module = m_Pcb->m_Modules;
	if( Module ) while( Module->Pnext ) Module = (MODULE*) Module->Pnext;

	for( ii = 0; ii < g_LibName_List.GetCount(); ii++)
	{
		fulllibname = g_LibName_List[ii];

		/* Calcul du nom complet de la librairie */
		fulllibname = MakeFileName(g_RealLibDirBuffer,fulllibname,LibExtBuffer);

		if ((lib_module = fopen(fulllibname.GetData(),"rt"))  == NULL )
		{
			msg.Printf(_("Library <%s> not found"),fulllibname.GetData());
			Affiche_Message(msg);
			continue ;
		}

		msg.Printf(_("Scan Lib: %s"),fulllibname.GetData());
		Affiche_Message(msg);

		/* lecture entete chaine definie par ENTETE_LIBRAIRIE */
		LineNum = 0;
		GetLine(lib_module, Line, &LineNum) ;
		StrPurge(Line);
		if(strnicmp( Line,ENTETE_LIBRAIRIE, L_ENTETE_LIB) != 0)
		{
			DisplayError(winaff, _("File is Not a library") );
			return(NULL);
		}

		/* Lecture de la liste des composants de la librairie */
		Found = 0;
		while( !Found && GetLine(lib_module,Line, &LineNum) )
		{
			if( strnicmp( Line, "$MODULE",6) == 0 ) break;
			if( strnicmp( Line,"$INDEX",6) == 0 )
			{
				while( GetLine(lib_module,Line, &LineNum) )
				{
					if( strnicmp( Line,"$EndINDEX",9) == 0 ) break;
					StrPurge(Line);
					if( stricmp(Line,ComponentName.GetData()) == 0 )
					{
						Found = 1; break; /* Trouve! */
					}
				}
			}
		}

		/* Lecture de la librairie */
		while( Found && GetLine(lib_module,Line, &LineNum) )
		{
			if( Line[0] != '$' ) continue;
			if( Line[1] != 'M' ) continue;
			if( strnicmp( Line, "$MODULE",7) != 0 ) continue;
			/* Lecture du nom du composant */
			strcpy(Name, StrPurge(Line+8));
			if( stricmp(Name,ComponentName.GetData()) == 0 )  /* composant localise */
			{
				NewModule = new MODULE(m_Pcb);
				NewModule->ReadDescr(lib_module, &LineNum);
				if( Module == NULL )	/* 1er Module */
				{
					m_Pcb->m_Modules = NewModule;
					NewModule->Pback = m_Pcb;
				}

				else
				{
					Module->Pnext = NewModule;
					NewModule->Pback = Module;
				}
				fclose(lib_module) ;
				Affiche_Message("") ;
				return(NewModule) ;
			}
		}
		fclose(lib_module) ; lib_module = 0;
	}

	if( lib_module ) fclose(lib_module) ;

	if ( show_msg_err )
	{
		msg.Printf(_("Module <%s> not found"),ComponentName.GetData());
		DisplayError(winaff, msg);
	}
	return(NULL) ;
}

/***************************************************************/
wxString WinEDA_BasePcbFrame::Select_1_Module_From_List(
				wxWindow * active_window,
				const wxString & Library,
				const wxString & Mask, const wxString & KeyWord)
/***************************************************************/
/*
 Affiche la liste des modules des librairies
	Recherche dans la librairie Library ou generale si Library == NULL
	Mask = Filtre d'affichage( Mask = "" pour listage non filtré )
	KeyWord = Liste de mots cles, Recherche limitee aux composants
		ayant ces mots cles ( KeyWord = "" pour listage de tous les modules )

	retourne "" si abort ou probleme
	ou le nom du module
*/
{
int LineNum;
unsigned ii, NbModules;
char Line[1024];
wxString FullLibName;
static wxString OldName;	/* Memorise le nom du dernier composant charge */
wxString CmpName;
FILE * lib_module;

WinEDAListBox * ListBox = new WinEDAListBox(active_window, "",
			NULL, OldName, DisplayCmpDoc, wxColour(200, 200, 255) );

	wxBeginBusyCursor();

	/* Recherche des composants en librairies */
	NbModules = 0;
	for( ii = 0; ii < g_LibName_List.GetCount(); ii++)
	{
		/* Calcul du nom complet de la librairie */
		if( Library == "" )
		{
			FullLibName = MakeFileName(g_RealLibDirBuffer,
						g_LibName_List[ii], LibExtBuffer);
		}
		else
			FullLibName = MakeFileName(g_RealLibDirBuffer,Library,LibExtBuffer);

		ReadDocLib(FullLibName );

		if( KeyWord != "")	/* Inutile de lire la librairie si selection
						par mots cles, deja lus */
		{
			if( Library != "" ) break;
			continue ;
		}

		if ((lib_module = fopen(FullLibName.GetData(),"rt"))  == NULL )
		{
			if( Library != "" ) break;
			continue ;
		}

		sprintf( Line,"Librairie: %s", FullLibName.GetData());
		Affiche_Message(Line);

		/* lecture entete */
		LineNum = 0;
		GetLine(lib_module, Line, &LineNum, sizeof(Line) -1);

		if(strnicmp( Line,ENTETE_LIBRAIRIE, L_ENTETE_LIB) != 0)
		{
			DisplayError(this, "Fichier Non Librairie", 20); continue;
		}

		/* Lecture de la librairie */
		while( GetLine(lib_module,Line, &LineNum, sizeof(Line) -1) )
		{
			if( Line[0] != '$' ) continue;
			if( strnicmp( Line, "$MODULE",6) == 0 ) break;
			if( strnicmp( Line,"$INDEX",6) == 0 )
			{
				while( GetLine(lib_module,Line, &LineNum) )
				{
					if( strnicmp( Line,"$EndINDEX",9) == 0 ) break;
                    strupper(Line);
					if ( Mask == "" )
					{
						ListBox->Append( StrPurge(Line) );
						NbModules++;
					}
					else if ( WildCompareString(Mask, Line, FALSE) )
					{
						ListBox->Append( StrPurge(Line) );
						NbModules++;
					}
				}
			} /* Fin Lecture INDEX */
		}  /* Fin lecture 1 Librairie */

		fclose(lib_module) ; lib_module = NULL;
		if( Library != "" ) break;
	}

	/*  creation de la liste des modules si recherche par mots-cles */
	if( KeyWord != "" )
		{
		ModList * ItemMod = MList;
		while( ItemMod != NULL )
			{
			if( KeyWordOk(KeyWord.GetData(), ItemMod->KeyWord) )
				{
				NbModules++;
				ListBox->Append( ItemMod->Name );
				}
			ItemMod = ItemMod->Next;
			}
		}

	sprintf( Line,"Modules (%d items)", NbModules);
	ListBox->SetTitle(Line);

	ListBox->SortList();

	wxEndBusyCursor();
	ii = ListBox->ShowModal();
	if ( ii >= 0 ) CmpName = ListBox->GetTextSelection();
	else CmpName = "";

	ListBox->Destroy();

	/* liberation mem de la liste des textes doc module */
	while( MList != NULL )
		{
		ModList * NewMod = MList->Next;
		delete MList;
		MList = NewMod;
		}

	if( CmpName != "" ) OldName = CmpName;

	return(CmpName);
}

/******************************************/
static void DisplayCmpDoc(wxString &  Name)
/*******************************************/
/* Routine de recherche et d'affichage de la doc du composant Name
	La liste des doc est pointee par MList
*/
{
ModList * Mod = MList;

	if ( ! Mod )
		{
		Name = ""; return;
		}

	/* Recherche de la description */
	while ( Mod )
		{
		if( Mod->Name && (stricmp(Mod->Name, Name.GetData()) == 0) ) break;
		Mod = Mod->Next;
		}

	if ( Mod )
		{
		Name = Mod->Doc ? Mod->Doc  : "No Doc";
		Name += "\nKeyW: ";
		Name += Mod->KeyWord ? Mod->KeyWord : "No Keyword";
		}

	else Name = "";
}

/***************************************************/
static void ReadDocLib(const wxString & ModLibName )
/***************************************************/
/* Routine de lecture du fichier Doc associe a la librairie ModLibName.
  	Cree en memoire la chaine liste des docs pointee par MList
	ModLibName = full file Name de la librairie Modules
*/
{
ModList * NewMod;
char Line[1024];
FILE * LibDoc;
wxString FullModLibName = ModLibName;

	ChangeFileNameExt(FullModLibName, EXT_DOC);

	if( (LibDoc = fopen(FullModLibName.GetData(),"rt")) == NULL ) return;

	GetLine(LibDoc, Line, NULL, sizeof(Line) -1);
	if(strnicmp( Line,ENTETE_LIBDOC, L_ENTETE_LIB) != 0) return;

	/* Lecture de la librairie */
	while( GetLine(LibDoc,Line, NULL, sizeof(Line) -1) )
		{
		if( Line[0] != '$' ) continue;
		if( Line[1] == 'E' ) break;;
		if( Line[1] == 'M' )	/* Debut decription 1 module */
			{
			NewMod = new ModList();
			NewMod->Next = MList;
			MList = NewMod;
			while( GetLine(LibDoc,Line, NULL, sizeof(Line) -1) )
				{
				if( Line[0] ==  '$' )	/* $EndMODULE */
						break;
				switch( Line[0] )
					{
					case 'L':	/* LibName */
						if( ! NewMod->Name )
							NewMod->Name = strdup(StrPurge(Line+3) );
						break;

					case 'K':	/* KeyWords */
						if( ! NewMod->KeyWord )
							NewMod->KeyWord = strdup(StrPurge(Line+3) );
						break;

					case 'C':	/* Doc */
						if( ! NewMod->Doc )
							NewMod->Doc = strdup(StrPurge(Line+3) );
						break;
					}
				}
			} /* lecture 1 descr module */
		}	/* Fin lecture librairie */
	fclose(LibDoc);
}

/********************************************************************/
MODULE * WinEDA_BasePcbFrame::Select_1_Module_From_BOARD(BOARD * Pcb)
/********************************************************************/
/* Affiche la liste des modules du PCB en cours
	Retourne un pointeur si module selectionne
	retourne NULL sinon
*/
{
int ii;
MODULE * Module;
static wxString OldName;	/* Memorise le nom du dernier composant charge */
wxString CmpName, msg;

WinEDAListBox * ListBox = new WinEDAListBox(this, "",
			NULL, "", NULL, wxColour(200, 200, 255) );

	/* Recherche des composants en BOARD */
	ii = 0;
	Module = Pcb->m_Modules;
	for( ; Module != NULL; Module = (MODULE*) Module->Pnext )
		{
		ii++;
		ListBox->Append( Module->m_Reference->GetText() );
		}

	msg.Printf( "Modules (%d items)", ii);
	ListBox->SetTitle(msg);

	ListBox->SortList();

	ii = ListBox->ShowModal();
	if ( ii >= 0 ) CmpName = ListBox->GetTextSelection();
	else CmpName = "";

	ListBox->Destroy();

	if( CmpName == "" ) return NULL;

	OldName = CmpName;

	// Recherche du pointeur sur le module
	Module = Pcb->m_Modules;
	for( ; Module != NULL; Module = (MODULE*) Module->Pnext )
		{
		if ( stricmp(CmpName.GetData(), Module->m_Reference->GetText()) == 0 )
			break;
		}
	return Module;
}


