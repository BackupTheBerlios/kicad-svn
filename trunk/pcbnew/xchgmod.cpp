	/******************************/
	/* PCBNEW: echange de modules */
	/******************************/

	/* Fichier xchmod.cpp */

#include "fctsys.h"
#include "gr_basic.h"
#include "common.h"
#include "pcbnew.h"
#include "autorout.h"

#include "protos.h"

/* variables locales */

   /***********************************************/
	/* Definition de l'application Change Module   */
	/***********************************************/


enum id_ExchangeModule
{
	ID_EXEC_EXCHANGE_MODULE = 1900,
	ID_EXEC_EXCHANGE_ID_MODULES,
	ID_EXEC_EXCHANGE_ID_MODULE_AND_VALUE,
	ID_EXEC_EXCHANGE_ALL_MODULES,
	ID_CLOSE_EXCHANGE_MODULE,
	ID_BROWSE_LIB_MODULES
};

	/************************************/
	/* class WinEDA_ExchangeModuleFrame */
	/************************************/

class WinEDA_ExchangeModuleFrame: public wxDialog
{
private:

	WinEDA_BasePcbFrame * m_Parent;
	wxDC * m_DC;
	MODULE * m_CurrentModule;
	WinEDA_EnterText * m_OldModule;
	WinEDA_EnterText * m_OldValue;
	WinEDA_EnterText * m_NewModule;
	wxTextCtrl * m_WinMsg;

public:
	// Constructor and destructor
	WinEDA_ExchangeModuleFrame(WinEDA_BasePcbFrame *parent,
							MODULE * Module, wxDC * DC, const wxPoint & pos);
	~WinEDA_ExchangeModuleFrame(void)
		{
		}

private:
	void OnQuit(wxCommandEvent& event);
	void Change_Module(wxCommandEvent& event);
	void Change_ModuleId(wxCommandEvent& event);
	void Change_ModuleAll(wxCommandEvent& event);
	int Maj_ListeCmp(
				const wxString & reference,
				const wxString & old_name,
				const wxString & new_name, bool ShowError);
	MODULE * Change_1_Module(MODULE * Module, const char * new_module, bool ShowError);
	void Sel_NewMod_By_Liste(wxCommandEvent& event);


	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WinEDA_ExchangeModuleFrame, wxDialog)
	EVT_BUTTON(ID_EXEC_EXCHANGE_MODULE, WinEDA_ExchangeModuleFrame::Change_Module)
	EVT_BUTTON(ID_EXEC_EXCHANGE_ID_MODULES, WinEDA_ExchangeModuleFrame::Change_ModuleId)
	EVT_BUTTON(ID_EXEC_EXCHANGE_ID_MODULE_AND_VALUE, WinEDA_ExchangeModuleFrame::Change_ModuleId)
	EVT_BUTTON(ID_EXEC_EXCHANGE_ALL_MODULES, WinEDA_ExchangeModuleFrame::Change_ModuleAll)
	EVT_BUTTON(ID_CLOSE_EXCHANGE_MODULE, WinEDA_ExchangeModuleFrame::OnQuit)
	EVT_BUTTON(ID_BROWSE_LIB_MODULES, WinEDA_ExchangeModuleFrame::Sel_NewMod_By_Liste)
END_EVENT_TABLE()


WinEDA_ExchangeModuleFrame::WinEDA_ExchangeModuleFrame(WinEDA_BasePcbFrame *parent,
				MODULE * Module,wxDC * DC,
				const wxPoint & framepos):
		wxDialog(parent, -1, _("Exchange Modules"), framepos, wxSize(360, 460),
					DIALOG_STYLE)
{
wxPoint pos;
wxButton * Button;
int lasty;

	m_Parent = parent;
	SetFont(*g_DialogFont);
	m_DC = DC;
	Centre();

	m_CurrentModule = Module;

	/* Creation des boutons de commande */
	pos.x = 180; pos.y = 5;
	Button = new wxButton(this, ID_EXEC_EXCHANGE_MODULE,
						_("Change module"), pos);
	Button->SetForegroundColour(*wxBLUE);

	pos.y += Button->GetDefaultSize().y + 3;
	Button = new wxButton(this, ID_EXEC_EXCHANGE_ID_MODULES,
						_("Change same modules"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.y += Button->GetDefaultSize().y + 3;
	Button = new wxButton(this, ID_EXEC_EXCHANGE_ID_MODULE_AND_VALUE,
						_("Ch. same module+value"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.y += Button->GetDefaultSize().y + 3;
	Button = new wxButton(this, ID_EXEC_EXCHANGE_ALL_MODULES,
						_("Change all"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.y += Button->GetDefaultSize().y + 14;
	Button = new wxButton(this, ID_BROWSE_LIB_MODULES,
						_("Browse Libs modules"), pos);
	Button->SetForegroundColour(wxColour(0,100,0) );

	pos.y += Button->GetDefaultSize().y + 8;
	Button = new wxButton(this, ID_CLOSE_EXCHANGE_MODULE,
						_("Close"), pos);
	Button->SetForegroundColour(*wxBLUE);
	lasty = pos.y += Button->GetDefaultSize().y;

	pos.x = 5; pos.y = 20;
	m_OldModule = new WinEDA_EnterText(this, _("Current Module"),
			m_CurrentModule ? m_CurrentModule->m_LibRef.GetData() :"",
			pos, wxSize( 150,-1) );
	m_OldModule->Enable(FALSE);

	pos.y += m_OldModule->GetDimension().y + 20;
	m_OldValue = new WinEDA_EnterText(this, _("Current Value"),
			m_CurrentModule ? m_CurrentModule->m_Value->GetText() :"",
			pos, wxSize( 150,-1) );
	m_OldValue->Enable(FALSE);

	pos.y += m_OldValue->GetDimension().y + 20;
	m_NewModule = new WinEDA_EnterText(this, _("New Module"),
			m_OldModule->GetData().GetData(), pos, wxSize( 150,-1) );

	pos.y = lasty + 10;
	m_WinMsg = new wxTextCtrl(this, -1,"", pos, wxSize(340, 230),
					wxTE_READONLY|wxTE_MULTILINE);
}


/*********************************************************************/
void WinEDA_BasePcbFrame::InstallExchangeModuleFrame( MODULE * Module,
					wxDC * DC, const wxPoint & pos)
/*********************************************************************/
{
	WinEDA_ExchangeModuleFrame * frame = new WinEDA_ExchangeModuleFrame(this,
					 Module, DC, pos);
	frame->ShowModal(); frame->Destroy();
}


/**********************************************************************/
void  WinEDA_ExchangeModuleFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
/**********************************************************************/
{
    Close(true);    // true is to force the frame to close
}



/************************************************************************/
int WinEDA_ExchangeModuleFrame::Maj_ListeCmp(
				const wxString & reference,
				const wxString & old_name,
				const wxString & new_name, bool ShowError)
/************************************************************************/
/*
	Met a jour le fichier name.CMP (s'il existe) apres un echange de module
	(par la commande changeMod), si les modules sont geres par ce fichier

	Si ShowError != 0 affiche message d'erreur si le fichier .cmp n'est pas
	trouve.
	Retoure 1 si erreur
*/
{
wxString FileNameCmp, tmpfile;
FILE * FichCmp, *NewFile;
char Line[1024];

	if ( old_name == new_name )	return(0);	/* pas de changement de nom */

	/* Calcul nom fichier CMP par changement de l'extension du nom netliste */
	if ( NetNameBuffer == "" )
		FileNameCmp = m_Parent->m_CurrentScreen->m_FileName;
	else FileNameCmp = NetNameBuffer;
	ChangeFileNameExt(FileNameCmp,NetCmpExtBuffer) ;

	// Modification du fichier .cmp correcpondant
	FichCmp = fopen(FileNameCmp.GetData(),"rt");
	if( FichCmp == NULL )
		{
		if ( ShowError )
			{
			sprintf(Line, _("file %s not found"), FileNameCmp.GetData());
			m_WinMsg->WriteText(Line);
			}
		return(1);
		}

	/* Analyse du fichier et modif */
	tmpfile = FileNameCmp;
	ChangeFileNameExt(tmpfile,".$$$");
	NewFile = fopen(tmpfile.GetData(),"wt");
	if( NewFile == NULL )
		{
		if ( ShowError )
			{
			sprintf(Line, _("Unable to create file %s"), tmpfile.GetData());
			m_WinMsg->WriteText(Line);
			}
		return(1);
		}

	fgets(Line, sizeof(Line), FichCmp);
	fprintf(NewFile,"Cmp-Mod V01 Genere par PcbNew le %s\n", DateAndTime(Line) );

	bool start_descr = FALSE;
	while( fgets(Line, sizeof(Line), FichCmp) != NULL )
		{
		if ( strnicmp( Line, "Reference = ", 9 ) == 0 )
			{
			char buf[1024];
			strcpy( buf, Line + 12);
			strtok(buf,";\n\r");
			if ( stricmp( buf , reference.GetData() ) == 0 )
				{
				start_descr = TRUE;
				}
			}

		if ( (strnicmp( Line, "Begin", 5 ) == 0) ||
			 (strnicmp( Line, "End", 3) == 0) )
			{
			start_descr = FALSE;
			}

		if ( start_descr && strnicmp( Line, "IdModule", 8 ) == 0 )
			{
			char buf[1024];
			sprintf(Line + 8,"  = %s;\n", new_name.GetData() );

			sprintf(buf, " * in %s\n", FileNameCmp.GetData());
			m_WinMsg->WriteText(buf);

			start_descr = FALSE;
			}
		fputs(Line, NewFile);
		}

	fclose(FichCmp);
	fclose(NewFile);
	wxRemoveFile(FileNameCmp);
	wxRenameFile(tmpfile, FileNameCmp);
	return(0);
}



/********************************************************************/
void WinEDA_ExchangeModuleFrame::Change_Module(wxCommandEvent& event)
/********************************************************************/

/* Routine de changement d'un module:
	Change le module pointe par la souris, par un autre en conservant
		- meme orientation
		- meme position
		- memes textes valeur et ref
		- memes netnames pour pads de meme nom
*/
{
wxString newmodulename = m_NewModule->GetData();

	if( newmodulename == "" ) return;

	if( Change_1_Module(m_CurrentModule, newmodulename,TRUE ) )
		{
		m_Parent->m_Pcb->m_Status_Pcb = 0;
		m_Parent->build_liste_pads();
		}
}

/*********************************************************************/
void WinEDA_ExchangeModuleFrame::Change_ModuleId(wxCommandEvent& event)
/**********************************************************************/

/* Routine de changement de tous les modules de meme nom lib que celui
	selectionne, en conservant
		- meme orientation
		- meme position
		- memes textes valeur et ref
		- memes netnames pour pads de meme nom
	et en remplacant l'ancien module par le noveau module
	Attention: m_CurrentModule ne pointe plus sur le module de reference
	puisque celui ci a ete change!!
*/
{
char line[256];
MODULE * PtModule, *PtBack;
bool change = FALSE;
wxString newmodulename = m_NewModule->GetData();
wxString value, lib_reference;	// pour memo Reflib et value de reference
bool check_module_value = FALSE;
int ShowErr = 5;	// Affiche 5 messages d'err maxi

	if( m_Parent->m_Pcb->m_Modules == NULL ) return;
	if( newmodulename == "" ) return;

	lib_reference = m_CurrentModule->m_LibRef;
	if ( event.GetId() == ID_EXEC_EXCHANGE_ID_MODULE_AND_VALUE )
		{
		check_module_value = TRUE;
		value = m_CurrentModule->m_Value->GetText();
		sprintf(line,"Change modules <%s> -> <%s> (val = %s)?",
				m_CurrentModule->m_LibRef.GetData(),
				newmodulename.GetData(), m_CurrentModule->m_Value->GetText() );
		}

	else
		{
		sprintf(line,"Change modules <%s> -> <%s> ?",
				lib_reference.GetData(), newmodulename.GetData() );
		}

	if( !IsOK(this, line) ) return;

	/* Le changement s'effectue a partir du dernier module car la routine
		Change_1_Module() modifie le dernier module de la liste
	*/

	PtModule = m_Parent->m_Pcb->m_Modules;
	for ( ; PtModule != NULL; PtModule = (MODULE*)PtModule->Pnext)
		{
		if(PtModule->Pnext == NULL) break;
		}

	/* Ici PtModule pointe le dernier module de la liste */
	for ( ; PtModule != (MODULE*)m_Parent->m_Pcb; PtModule = PtBack)
		{
		MODULE * module;
		PtBack = (MODULE*)PtModule->Pback;
		if(stricmp( lib_reference.GetData(), PtModule->m_LibRef.GetData() ) != 0 )
			continue;
		if ( check_module_value )
			{
			if(stricmp( value.GetData(), PtModule->m_Value->GetText() ) != 0 )
				continue;
			}
		module = Change_1_Module(PtModule, newmodulename.GetData(), ShowErr);
		if ( module ) change = TRUE;
		else if (ShowErr) ShowErr--;
		}
	if( change )
		{
		m_Parent->m_Pcb->m_Status_Pcb = 0;
		m_Parent->build_liste_pads();
		}
}

/***********************************************************************/
void WinEDA_ExchangeModuleFrame::Change_ModuleAll(wxCommandEvent& event)
/***********************************************************************/
/* Routine de changement de tous les modules par les modules de meme nom lib:
	en conservant
		- meme orientation
		- meme position
		- memes textes valeur et ref
		- memes netnames pour pads de meme nom
*/
{
MODULE * PtModule, *PtBack;
bool change = FALSE;
int ShowErr = 5;	// Affiche 5 messages d'err maxi

	if(m_Parent->m_Pcb->m_Modules == NULL) return;

	if( !IsOK(this, _("Change ALL modules ?")) ) return;

	/* Le changement s'effectue a partir du dernier module car la routine
		Change_1_Module() modifie le dernier module de la liste
	*/

	PtModule = (MODULE*) m_Parent->m_Pcb->m_Modules;
	for ( ; PtModule != NULL; PtModule = (MODULE*)PtModule->Pnext)
	{
		if(PtModule->Pnext == NULL) break;
	}

	/* Ici PtModule pointe le dernier module de la liste */
	for ( ; PtModule != (MODULE*)(m_Parent->m_Pcb); PtModule = PtBack)
	{
		PtBack = (MODULE*)PtModule->Pback;
		if ( Change_1_Module(PtModule, PtModule->m_LibRef.GetData(), ShowErr) )
			change = TRUE;
		else if (ShowErr) ShowErr--;
	}

	if( change)
	{
		m_Parent->m_Pcb->m_Status_Pcb = 0;
		m_Parent->build_liste_pads();
	}
}

/******************************************************************/
MODULE * WinEDA_ExchangeModuleFrame::Change_1_Module(MODULE * PtModule,
		const char * new_module, bool ShowError)
/*******************************************************************/
/* Routine de changement d'un module:
	Change le module de numero empr, avec le module de nom new_module
		- meme orientation
		- meme position
		- memes textes valeur et ref
		- memes netnames pour pads de meme nom
	Retourne :
		0 si pas de changement ( si le nouveau module n'est pas en libr)
		1 si OK
*/
{
char namecmp[1024];
char oldnamecmp[1024];
MODULE * NewModule;
char Line[1024];

	if(PtModule == NULL) return(NULL);

wxBusyCursor dummy;

	/* Memorisation des parametres utiles de l'ancien module */
	strcpy( oldnamecmp, PtModule->m_LibRef.GetData() );
	strcpy( namecmp, new_module);

	/* Chargement du module */
	sprintf(Line,"Change module %s (%s)  ",
		PtModule->m_Reference->GetText(), oldnamecmp);
	m_WinMsg->WriteText(Line);

	StrPurge(namecmp);
	NewModule = m_Parent->Get_Librairie_Module(this, "", namecmp, ShowError);
	if( NewModule == NULL)	/* Nouveau module NON trouve, reaffichage de l'ancien */
		{
		m_WinMsg->WriteText("No\n");
		return(NULL);
		}

	if ( PtModule == m_CurrentModule ) m_CurrentModule = NewModule;
	m_WinMsg->WriteText("Ok\n");

	/* Effacement a l'ecran de l'ancien module */
	PtModule->Draw(m_Parent->DrawPanel, m_DC, wxPoint(0,0), GR_XOR);

	m_Parent->Exchange_Module(this, PtModule, NewModule);

	/* Affichage du nouveau module */
	NewModule->Draw(m_Parent->DrawPanel, m_DC, wxPoint(0,0),GR_OR);

	Maj_ListeCmp(NewModule->m_Reference->GetText(), oldnamecmp, namecmp, ShowError);

	return(NewModule);
}

/***********************************************************************************/
MODULE * WinEDA_BasePcbFrame::Exchange_Module(wxWindow * winaff,
					MODULE * OldModule, MODULE * NewModule)
/***********************************************************************************/
/*
Remplace le module OldModule par le module NewModule (en conservant position, orientation..)
OldModule est supprimé de la memoire.
*/
{
wxPoint oldpos;	/* memorisation temporaire pos curseur */
D_PAD * pt_pad, * pt_old_pad;


	if ( (OldModule->m_StructType != TYPEMODULE) || (NewModule->m_StructType != TYPEMODULE) )
		{
		DisplayError(winaff, "WinEDA_BasePcbFrame::Exchange_Module() StuctType error" );
		}

	NewModule->m_Parent = m_Pcb;

	m_Pcb->m_Status_Pcb = 0 ;
	oldpos = m_CurrentScreen->m_Curseur;
	m_CurrentScreen->m_Curseur = OldModule->m_Pos;
	Place_Module(NewModule, NULL);
	m_CurrentScreen->m_Curseur = oldpos;

	/* Changement eventuel de couche */
	if( OldModule->m_Layer != NewModule->m_Layer)
		{
		Change_Side_Module(NewModule, NULL);
		}

	/* Rotation eventuelle du module */
	if( OldModule->m_Orient != NewModule->m_Orient )
		{
		Rotate_Module(NULL, NewModule,OldModule->m_Orient, FALSE );
		}

	/* Mise a jour des textes ref et val */
	NewModule->m_Reference->m_Text = OldModule->m_Reference->m_Text;
	NewModule->m_Value->m_Text = OldModule->m_Value->m_Text;

	/* Mise a jour des autres parametres */
	NewModule->m_TimeStamp = OldModule->m_TimeStamp;

	/* mise a jour des netnames ( lorsque c'est possible) */
	pt_pad = NewModule->m_Pads;
	for( ; pt_pad != NULL; pt_pad = (D_PAD*)pt_pad->Pnext)
		{
		pt_pad->m_Netname = "";
		pt_pad->m_NetCode = 0;
		pt_old_pad = OldModule->m_Pads;
		for( ; pt_old_pad != NULL; pt_old_pad = (D_PAD*)pt_old_pad->Pnext )
			{
			if(strnicmp( pt_pad->m_Padname, pt_old_pad->m_Padname,sizeof(pt_pad->m_Padname)) == 0)
				{
				pt_pad->m_Netname = pt_old_pad->m_Netname;
				pt_pad->m_NetCode = pt_old_pad->m_NetCode;
				}
			}
		}

	/* Effacement de l'ancien module */
	DeleteStructure(OldModule);

	m_Pcb->m_Status_Pcb = 0;
	NewModule->m_Flags = 0;
	m_CurrentScreen->SetModify();

	return NewModule;
}


	/*****************************************/
	/* static void Sel_NewMod_By_Liste(void) */
	/*****************************************/

/*affiche la liste des modules en librairie et selectione 1 nom */

void WinEDA_ExchangeModuleFrame::Sel_NewMod_By_Liste(wxCommandEvent& event)
{
wxString newname;

	newname = m_Parent->Select_1_Module_From_List(this, "", "", "");
	if ( newname != "" )
		m_NewModule->SetValue(newname);
}


/***************************************************/
bool WinEDA_PcbFrame::RecreateCmpFileFromBoard(void)
/***************************************************/
{
wxString FullFileNameCmp;
FILE * FichCmp;
char Line[1024];
MODULE * Module = m_Pcb->m_Modules;

	if ( Module == NULL)
		{
		DisplayError(this,"No Modules!");
		return FALSE;
		}

	/* Calcul nom fichier CMP par changement de l'extension du nom netliste */
	if ( NetNameBuffer == "" )
		FullFileNameCmp = m_CurrentScreen->m_FileName;
	else FullFileNameCmp = NetNameBuffer;
	ChangeFileNameExt(FullFileNameCmp,NetCmpExtBuffer) ;

	strcpy(Line,"*"); strcat ( Line,NetCmpExtBuffer);
	FullFileNameCmp = EDA_FileSelector("Cmp files:",
				"",						/* Chemin par defaut */
				FullFileNameCmp,	 	/* nom fichier par defaut */
				NetCmpExtBuffer,		/* extension par defaut */
				Line,					/* Masque d'affichage */
				this,
				wxSAVE,
				FALSE
				);
	if ( FullFileNameCmp == "" ) return FALSE;


	FichCmp = fopen(FullFileNameCmp.GetData(),"wt");
	if( FichCmp == NULL )
		{
		sprintf(Line, "Unable to create file %s", FullFileNameCmp.GetData());
		DisplayError(this, Line);
		return FALSE;
		}

	fgets(Line, sizeof(Line), FichCmp);
	fprintf(FichCmp,"Cmp-Mod V01 Genere par PcbNew le %s\n", DateAndTime(Line) );

	for( ; Module != NULL; Module = (MODULE*) Module->Pnext )
		{
		fprintf(FichCmp,"\nBeginCmp\n" );
		fprintf(FichCmp,"TimeStamp = %8.8lX\n", Module->m_TimeStamp);
		fprintf(FichCmp,"Reference = %s;\n",
			strlen(Module->m_Reference->GetText()) ?
					Module->m_Reference->GetText() : "[NoRef]" );
		fprintf(FichCmp,"ValeurCmp = %s;\n",
			strlen(Module->m_Value->GetText()) ?
					Module->m_Value->GetText() : "[NoVal]" );
		fprintf(FichCmp,"IdModule  = %s;\n", Module->m_LibRef.GetData());
		fprintf(FichCmp,"EndCmp\n" );
		}

	fprintf(FichCmp,"\nEndListe\n");
	fclose(FichCmp);

	return TRUE;
}
