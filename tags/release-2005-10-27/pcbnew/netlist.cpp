	/************************************/
	/* PCBNEW: traitement des netlistes */
	/************************************/

	/* Fichier NETLIST.CPP */

/*
Fonction de lecture de la netliste pour:
	- Chargement modules et nouvelles connexions 
	- Test des modules (modules manquants ou en trop
	- Recalcul du chevelu
	
Remarque importante:
	Lors de la lecture de la netliste pour Chargement modules
	et nouvelles connexions, l'identification des modules peut se faire selon
	2 criteres:
		- la reference (U2, R5 ..): c'est le mode normal
		- le Time Stamp (Signature Temporelle), a utiliser apres reannotation
			d'un schema, donc apres modification des references sans pourtant
			avoir reellement modifie le schema
*/
#include "fctsys.h"
#include "gr_basic.h"
#include "common.h"
#include "pcbnew.h"
#include "autorout.h"

#include "protos.h"


#define TESTONLY 1			/* ctes utilisees lors de l'appel a */
#define READMODULE 0		/*  ReadNetModuleORCADPCB */

/* Structures locales */
class MODULEtoLOAD : public EDA_BaseStruct
{
public:
	wxString m_LibName;
	wxString m_CmpName;

public:
	MODULEtoLOAD(const wxString & libname, const wxString & cmpname, int timestamp);
	~MODULEtoLOAD(void){};
	MODULEtoLOAD * Next(void) { return (MODULEtoLOAD *) Pnext; }
};

/* Fonctions locales : */
static void SortListModulesToLoadByLibname(int NbModules);


/* Variables locales */
static int s_NbNewModules;
static MODULEtoLOAD * s_ModuleToLoad_List;
FILE * source;
static bool ChangeExistantModule;
static bool DisplayWarning  = TRUE;
static int DisplayWarningCount ;

	/*****************************/
	/* class WinEDA_NetlistFrame */
	/*****************************/
enum id_netlist_functions
{
	ID_READ_NETLIST_FILE = 1900,
	ID_TEST_NETLIST,
	ID_CLOSE_NETLIST,
	ID_COMPILE_RATSNEST,
	ID_OPEN_NELIST
};


class WinEDA_NetlistFrame: public wxDialog
{
private:

	WinEDA_PcbFrame * m_Parent;
	wxDC * m_DC;
	wxRadioBox * m_Select_By_Timestamp;
	wxRadioBox * m_DeleteBadTracks;
	wxRadioBox * m_ChangeExistantModuleCtrl;
	wxCheckBox * m_DisplayWarningCtrl;
	wxTextCtrl * m_MessageWindow;

public:
	// Constructor and destructor
	WinEDA_NetlistFrame(WinEDA_PcbFrame *parent, wxDC * DC, const wxPoint & pos);
	~WinEDA_NetlistFrame(void)
		{
		}

private:
	void OnQuit(wxCommandEvent& event);
	void ReadPcbNetlist(wxCommandEvent& event);
	void Set_NetlisteName(wxCommandEvent& event);
	bool OpenNetlistFile(wxCommandEvent& event);
	int BuildListeNetModules(wxCommandEvent& event, char * BufName);
	void ModulesControle(wxCommandEvent& event);
	void RecompileRatsnest(wxCommandEvent& event);
	int ReadListeModules(const char * RefCmp, int TimeStamp, char * NameModule);
	int SetPadNetName( char * Line, MODULE * Module);
	MODULE * ReadNetModule( char * Text,
						int * UseFichCmp, int TstOnly);
	void AddToList(const char * NameLibCmp, const char * NameCmp,int TimeStamp );
	void LoadListeModules(wxDC *DC);

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WinEDA_NetlistFrame, wxDialog)
	EVT_BUTTON(ID_READ_NETLIST_FILE, WinEDA_NetlistFrame::ReadPcbNetlist)
	EVT_BUTTON(ID_CLOSE_NETLIST, WinEDA_NetlistFrame::OnQuit)
	EVT_BUTTON(ID_OPEN_NELIST, WinEDA_NetlistFrame::Set_NetlisteName)
	EVT_BUTTON(ID_TEST_NETLIST, WinEDA_NetlistFrame::ModulesControle)
	EVT_BUTTON(ID_COMPILE_RATSNEST, WinEDA_NetlistFrame::RecompileRatsnest)
END_EVENT_TABLE()


/*************************************************************************/
void WinEDA_PcbFrame::InstallNetlistFrame( wxDC * DC, const wxPoint & pos)
/*************************************************************************/
{
	WinEDA_NetlistFrame * frame = new WinEDA_NetlistFrame(this, DC, pos);
	frame->ShowModal(); frame->Destroy();
}

/******************************************************************/
WinEDA_NetlistFrame::WinEDA_NetlistFrame(WinEDA_PcbFrame *parent,
				wxDC * DC, const wxPoint & framepos):
		wxDialog(parent, -1, "", framepos, wxSize(-1, -1),
			DIALOG_STYLE)
/******************************************************************/
{
wxPoint pos;
wxString number;
wxString title;
wxButton * Button;

	m_Parent = parent;
	SetFont(*g_DialogFont);
	m_DC = DC;
	Centre();

	/* Mise a jour du Nom du fichier NETLISTE correspondant */
	NetNameBuffer = m_Parent->m_CurrentScreen->m_FileName;
	ChangeFileNameExt(NetNameBuffer, NetExtBuffer);

	title = _("Netlist: ") + NetNameBuffer;
	SetTitle(title);

	/* Creation des boutons de commande */
	pos.x = 170; pos.y = 10;
	Button = new wxButton(this, ID_OPEN_NELIST,
						_("Select"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.y += Button->GetDefaultSize().y + 10;
	Button = new wxButton(this, ID_READ_NETLIST_FILE,
						_("Read"), pos);
	Button->SetForegroundColour(wxColor(0,80,0) );

	pos.y += Button->GetDefaultSize().y + 10;
	Button = new wxButton(this, ID_TEST_NETLIST,
						_("Module Test"), pos);
	Button->SetForegroundColour(wxColor(0,80,80) );

	pos.y += Button->GetDefaultSize().y + 10;
	Button = new wxButton(this, ID_COMPILE_RATSNEST,
						_("Compile"), pos);
	Button->SetForegroundColour(wxColor(0,0,80) );

	pos.y += Button->GetDefaultSize().y + 10;
	Button = new wxButton(this, ID_CLOSE_NETLIST,
						_("Close"), pos);
	Button->SetForegroundColour(*wxBLUE);

	// Options:
wxString opt_select[2] = { _("Reference"), _("Timestamp") };
	pos.x = 15; pos.y = 10;
	m_Select_By_Timestamp = new wxRadioBox( this, -1, _("Module Selection:"),
			pos, wxDefaultSize, 2, opt_select, 1);

wxString track_select[2] = { _("Keep"), _("Delete") };
	int x, y;
	m_Select_By_Timestamp->GetSize(&x, &y);
	pos.y += 5 + y;
	m_DeleteBadTracks = new wxRadioBox(this, -1, _("Bad Tracks Deletion:"),
			pos, wxDefaultSize, 2, track_select, 1);

	m_DeleteBadTracks->GetSize(&x, &y);
	pos.y += 5 + y;
wxString change_module_select[2] = { _("Keep"), _("Change") };
	m_ChangeExistantModuleCtrl = new wxRadioBox(this, -1, _("Exchange Module:"),
			pos, wxDefaultSize, 2, change_module_select, 1);

	m_ChangeExistantModuleCtrl->GetSize(&x, &y);
	pos.y += 15 + y;
	m_DisplayWarningCtrl = new wxCheckBox(this, -1, _("Display Warnings"), pos);
    m_DisplayWarningCtrl->SetValue(DisplayWarning);

	m_DisplayWarningCtrl->GetSize(&x, &y);
	pos.x = 5; pos.y += 10 + y;
	m_MessageWindow = new wxTextCtrl(this, -1, "",
			pos, wxSize(265, 120),wxTE_READONLY|wxTE_MULTILINE);

	m_MessageWindow->GetSize(&x, &y);
	pos.y += 10 + y;
	SetClientSize(280, pos.y);
}


/**********************************************************************/
void  WinEDA_NetlistFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
/**********************************************************************/
{
    // true is to force the frame to close
    Close(true);
}


/*****************************************************************/
void WinEDA_NetlistFrame::RecompileRatsnest(wxCommandEvent& event)
/*****************************************************************/
{
	m_Parent->Compile_Ratsnest(m_DC, TRUE);
}

/***************************************************************/
bool WinEDA_NetlistFrame::OpenNetlistFile(wxCommandEvent& event)
/***************************************************************/
/*
 routine de selection et d'ouverture du fichier Netlist
*/
{
wxString FullFileName;
wxString msg;

	if( NetNameBuffer == "" )	/* Pas de nom specifie */
		Set_NetlisteName(event);

	if( NetNameBuffer == "" )  return(FALSE); /* toujours Pas de nom specifie */

	FullFileName = NetNameBuffer;

	source = fopen(FullFileName.GetData(),"rt");
	if (source == 0)
		{
		msg.Printf(_("Netlist file %s not found"),FullFileName.GetData()) ;
		DisplayError(this, msg) ;
		return FALSE ;
		}

	msg.Printf("Netlist %s",FullFileName.GetData());
	SetTitle(msg);

		return TRUE;
}


/**************************************************************/
void WinEDA_NetlistFrame::ReadPcbNetlist(wxCommandEvent& event)
/**************************************************************/
/* mise a jour des empreintes :
	corrige les Net Names, les textes, les "TIME STAMP"

	Analyse les lignes:
# EESchema Netlist Version 1.0 generee le  18/5/2005-12:30:22
(
 ( 40C08647 $noname R20 4,7K {Lib=R}
  (    1 VCC )
  (    2 MODB_1 )
 )
 ( 40C0863F $noname R18 4,7_k {Lib=R}
  (    1 VCC )
  (    2 MODA_1 )
 )
}
#End
*/
{
int LineNum, State, Comment;
MODULE * Module = NULL;
D_PAD * PtPad;
char Line[256], *Text;
int UseFichCmp = 1;
wxString msg;

    ChangeExistantModule = m_ChangeExistantModuleCtrl->GetSelection() == 1 ? TRUE : FALSE;
    DisplayWarning = m_DisplayWarningCtrl->GetValue();
    if ( DisplayWarning ) DisplayWarningCount = 8;
    else  DisplayWarningCount = 0;

	if( ! OpenNetlistFile(event) ) return;

	msg = _("Read Netlist ") + NetNameBuffer;
	m_MessageWindow->AppendText(msg);

	m_Parent->m_CurrentScreen->SetModify();
	m_Parent->m_Pcb->m_Status_Pcb = 0 ; State = 0; LineNum = 0; Comment = 0;
	s_NbNewModules = 0;

	/* Premiere lecture de la netliste: etablissement de la liste
		des modules a charger
	*/
	while( GetLine( source, Line, &LineNum) )
		{
		Text = StrPurge(Line);

		if ( Comment ) /* Commentaires en cours */
			{
			if( (Text = strchr(Text,'}') ) == NULL )continue;
			Comment = 0;
			}
		if ( *Text == '{' ) /* Commentaires */
			{
			Comment = 1;
			if( (Text = strchr(Text,'}') ) == NULL ) continue;
			}

		if ( *Text == '(' ) State++;
		if ( *Text == ')' ) State--;

		if( State == 2 )
			{
			Module = ReadNetModule(Text, & UseFichCmp, TESTONLY);
			continue;
			}

		if( State >= 3 ) /* la ligne de description d'un pad est ici non analysee */
			{
			State--;
			}
		}

	/* Chargement des nouveaux modules */
	if( s_NbNewModules )
	{
		LoadListeModules(m_DC);
		// Free module list:
		MODULEtoLOAD * item, *next_item;
		for ( item = s_ModuleToLoad_List; item != NULL; item = next_item )
		{
			next_item = item->Next();
			delete item;
		}
		s_ModuleToLoad_List = NULL;
	}

	/* Relecture de la netliste, tous les modules sont ici charges */
	fseek(source, 0, SEEK_SET); LineNum = 0;
	while( GetLine( source, Line, &LineNum) )
		{
		Text = StrPurge(Line);

		if ( Comment ) /* Commentaires en cours */
			{
			if( (Text = strchr(Text,'}') ) == NULL )continue;
			Comment = 0;
			}
		if ( *Text == '{' ) /* Commentaires */
			{
			Comment = 1;
			if( (Text = strchr(Text,'}') ) == NULL ) continue;
			}

		if ( *Text == '(' ) State++;
		if ( *Text == ')' ) State--;

		if( State == 2 )
			{
			Module = ReadNetModule(Text, & UseFichCmp, READMODULE );
			if( Module == NULL )
				{  /* empreinte non trouvee dans la netliste */
				continue ;
				}
			else /* Raz netnames sur pads */
				{
				PtPad = Module->m_Pads;
				for( ; PtPad != NULL; PtPad = (D_PAD *)PtPad->Pnext )
					{
					PtPad->m_Netname = "";
					}
				}
			continue;
			}

		if( State >= 3 )
			{
			if ( Module )
				{
				SetPadNetName( Text, Module);
				}
			State--;
			}
		}

	fclose(source);

	/* Mise a jour et Cleanup du circuit imprime: */
	m_Parent->Compile_Ratsnest(m_DC, TRUE);

	if( m_Parent->m_Pcb->m_Track )
		{
		if( m_DeleteBadTracks->GetSelection() == 1 )
			{
			wxBeginBusyCursor();;
			Netliste_Controle_piste(m_Parent, m_DC, TRUE);
			m_Parent->Compile_Ratsnest(m_DC, TRUE);
			wxEndBusyCursor();;
			}
		}

	Affiche_Infos_Status_Pcb(m_Parent);
}


/****************************************************************************/
MODULE * WinEDA_NetlistFrame::ReadNetModule(char * Text, int * UseFichCmp,
		int TstOnly)
/****************************************************************************/
/* charge la description d'une empreinte ,netliste type PCBNEW
	et met a jour le module correspondant

	Si TstOnly == 0 si le module n'existe pas, il est charge
	Si TstOnly != 0 si le module n'existe pas, il est ajoute a la liste des
		modules a charger
	Text contient la premiere ligne de la description
	* UseFichCmp est un flag
			si != 0, le fichier des composants .CMP sera utilise
			est remis a 0 si le fichier n'existe pas

Analyse les lignes type:
( 40C08647 $noname R20 4,7K {Lib=R}
  (    1 VCC )
  (    2 MODB_1 )
*/
{
MODULE * Module;
char *TextTimeStamp;
char *TextNameLibMod;
char *TextValeur;
char *TextCmpName;
int TimeStamp = -1, Error = 0;
char Line[1024], NameLibCmp[256];
bool Found;

	strcpy(Line,Text);

	if( (TextTimeStamp = strtok(Line, " ()\t\n")) == NULL ) Error = 1;
	if( (TextNameLibMod = strtok(NULL, " ()\t\n")) == NULL ) Error = 1;
	if( (TextCmpName = strtok(NULL, " ()\t\n")) == NULL ) Error = 1;
	if( (TextValeur = strtok(NULL, " ()\t\n")) == NULL ) Error = 1;

	if( Error ) return( NULL );

	sscanf(TextTimeStamp,"%X", &TimeStamp);

	/* Tst si composant deja charge */
	Module = (MODULE*) m_Parent->m_Pcb->m_Modules;
	MODULE * NextModule;
	for( Found = FALSE; Module != NULL; Module = NextModule )
		{
		NextModule = (MODULE*)Module->Pnext;
		if( m_Select_By_Timestamp->GetSelection()  == 1 ) /* Reconnaissance par signature temporelle */
			{
			if( TimeStamp == Module->m_TimeStamp) Found = TRUE;
			}
		else	/* Reconnaissance par Reference */
			{
			if( stricmp(TextCmpName,Module->m_Reference->GetText()) == 0 )
				 Found = TRUE;
			}
		if ( Found ) // Test si module (m_LibRef) et module specifie en netlist concordent
			{
			if( TstOnly != TESTONLY )
				{
				strcpy(NameLibCmp, TextNameLibMod);
				if( *UseFichCmp )
					{
					if( m_Select_By_Timestamp->GetSelection()  == 1 )
						{	/* Reconnaissance par signature temporelle */
						*UseFichCmp = ReadListeModules(NULL, TimeStamp, NameLibCmp);
						}
					else	/* Reconnaissance par Reference */
						{
						*UseFichCmp = ReadListeModules(TextCmpName, 0, NameLibCmp);
						}
					}
				if ( stricmp(Module->m_LibRef.GetData(), NameLibCmp) != 0 )
					{// Module Mismatch: Current module and module specified in netlist are diff.
					if ( ChangeExistantModule )
						{
						MODULE * NewModule =
								m_Parent->Get_Librairie_Module(this, "", NameLibCmp, TRUE);
						if( NewModule )	/* Nouveau module trouve : changement de module */
							Module = m_Parent->Exchange_Module(this, Module, NewModule);
						}
				 	else
				 		{
						wxString msg;
						msg.Printf(
							_("Cmp %s: Mismatch! module is [%s] and netlist said [%s]\n"),
						TextCmpName, Module->m_LibRef.GetData(), NameLibCmp);
						m_MessageWindow->AppendText(msg);
						if ( DisplayWarningCount > 0)
							{
							DisplayError(this, msg, 2);
							DisplayWarningCount--;
							}
						}
					}
				}
			break;
			}
		}

	if( Module == NULL )	/* Module a charger */
		{
		strcpy(NameLibCmp, TextNameLibMod);

		if( *UseFichCmp )
			{
			if( m_Select_By_Timestamp->GetSelection()  == 1)
				{	/* Reconnaissance par signature temporelle */
				*UseFichCmp = ReadListeModules(NULL, TimeStamp, NameLibCmp);
				}
			else	/* Reconnaissance par Reference */
				{
				*UseFichCmp = ReadListeModules(TextCmpName, 0, NameLibCmp);
				}
			}


		if( TstOnly == TESTONLY)
			AddToList(NameLibCmp, TextCmpName, TimeStamp);
		else
			{
			wxString msg;
			msg.Printf(_("Componant [%s] not found"), TextCmpName);
			m_MessageWindow->AppendText(msg+"\n");
			if (DisplayWarningCount> 0)
				{
				DisplayError(this, msg, 2);
				DisplayWarningCount--;
				}
			}
		return(NULL);	/* Le module n'avait pas pu etre charge */
		}

	/* mise a jour des reperes ( nom et ref "Time Stamp") si module charge */
	Module->m_Reference->m_Text = TextCmpName;
	Module->m_Value->m_Text = TextValeur;
	Module->m_TimeStamp = TimeStamp;

	return(Module) ; /* composant trouve */
}

/********************************************************************/
int WinEDA_NetlistFrame::SetPadNetName( char * Text, MODULE * Module)
/********************************************************************/
/*
Met a jour le netname de 1 pastille, Netliste ORCADPCB
	entree :
		Text = ligne de netliste lue ( (pad = net) )
		Module = adresse de la structure MODULE a qui appartient les pads
*/
{
D_PAD * pad;
char * TextPinName, * TextNetName;
int Error = 0;
bool trouve;
char Line[256], Msg[80];

	if( Module == NULL ) return ( 0 );

	strcpy( Line, Text);

	if( (TextPinName = strtok(Line, " ()\t\n")) == NULL ) Error = 1;
	if( (TextNetName = strtok(NULL, " ()\t\n")) == NULL ) Error = 1;
	if(Error) return(0);

	/* recherche du pad */
	pad = Module->m_Pads; trouve = FALSE;
	for( ; pad != NULL; pad = (D_PAD *)pad->Pnext )
		{
		if( strnicmp( TextPinName, pad->m_Padname, 4) == 0 )
			{ /* trouve */
			trouve = TRUE;
			if( *TextNetName != '?' ) pad->m_Netname = TextNetName;
			else pad->m_Netname = "";
			}
		}

	if( !trouve && (DisplayWarningCount > 0) )
		{
		sprintf(Msg,_("Module [%s]: Pad [%s] not found"),
					Module->m_Reference->GetText(), TextPinName);
		DisplayError(this, Msg, 1);
		DisplayWarningCount--;
		}

	return(trouve);
}


/*****************************************************/
MODULE * WinEDA_PcbFrame::ListAndSelectModuleName(void)
/*****************************************************/
/*	liste les noms des modules du PCB
	Retourne:
		un pointeur sur le module selectionne
		NULL si pas de selection
*/
{
int ii, jj, nb_empr;
MODULE * Module;
WinEDAListBox * ListBox;
const char ** ListNames = NULL;

	if( m_Pcb->m_Modules == NULL )
		{
		DisplayError(this, _("No Modules") ) ; return(0);
		}

	/* Calcul du nombre des modules */
	nb_empr = 0; Module = (MODULE*)m_Pcb->m_Modules;
	for( ;Module != NULL; Module = (MODULE*)Module->Pnext) nb_empr++;
	ListNames = (const char**) MyZMalloc( (nb_empr + 1) * sizeof(char*) );
	Module = (MODULE*) m_Pcb->m_Modules;
	for( ii = 0; Module != NULL; Module = (MODULE*)Module->Pnext, ii++)
		{
		ListNames[ii] = Module->m_Reference->GetText();
		}

	ListBox = new WinEDAListBox(this, _("Componants"),
					ListNames, "");
	ii = ListBox->ShowModal(); ListBox->Destroy();

	
	if( ii < 0 )	/* Pas de selection */
		{
		Module = NULL;
		}

	else /* Recherche du module selectionne */
	{
	Module = (MODULE*) m_Pcb->m_Modules;
	for( jj = 0; Module != NULL; Module = (MODULE*)Module->Pnext, jj++)
		{
		if( Module->m_Reference->GetText() == ListNames[ii] ) break;
		}
	}
	
	free(ListNames);
	return(Module);
}

/***************************************************************/
void WinEDA_NetlistFrame::ModulesControle(wxCommandEvent& event)
/***************************************************************/
/* donne la liste :
1 - des empreintes doublées sur le PCB
2 - des empreintes manquantes par rapport a la netliste
3 - des empreintes supplémentaires par rapport a la netliste
*/
#define MAX_LEN_TXT 32
{
int ii, NbModulesPcb;
MODULE * Module, * pt_aux;
int NbModulesNetListe ,nberr = 0 ;
char *ListeNetModules, * NameNetMod;
WinEDA_TextFrame * List;

	/* determination du nombre des modules du PCB*/
	NbModulesPcb = 0; Module = (MODULE*) m_Parent->m_Pcb->m_Modules;
	for( ;Module != NULL; Module = (MODULE*)Module->Pnext)
		{
		NbModulesPcb++;
		}

	if( NbModulesPcb == 0 )
		{
		DisplayError(this, _("No modules"), 10); return;
		}

	/* Construction de la liste des references des modules de la netliste */
	NbModulesNetListe = BuildListeNetModules(event, NULL);
	if( NbModulesNetListe < 0 ) return; /* fichier non trouve */

	if( NbModulesNetListe == 0 )
		{
		DisplayError(this, _("No modules in NetList"), 10); return;
		}

	ii = NbModulesNetListe * (MAX_LEN_TXT + 1);
	ListeNetModules = (char*)MyZMalloc ( ii );
	if( NbModulesNetListe ) BuildListeNetModules(event, ListeNetModules);

	List = new WinEDA_TextFrame(this, _("Check Modules"));

	/* recherche des doubles */
	List->Append(_("Duplicates"));

	Module = (MODULE*) m_Parent->m_Pcb->m_Modules;
	for( ;Module != NULL; Module = (MODULE*)Module->Pnext)
		{
		pt_aux = (MODULE*)Module->Pnext;
		for( ; pt_aux != NULL; pt_aux = (MODULE*)pt_aux->Pnext)
			{
			if( strnicmp(Module->m_Reference->GetText(),
				pt_aux->m_Reference->GetText(),MAX_LEN_TXT) == 0 )
				{
				List->Append(Module->m_Reference->GetText());
				nberr++ ;
				break;
				}
			}
		}

	/* recherche des manquants par rapport a la netliste*/
	List->Append(_("Lack:") );

	NameNetMod = ListeNetModules;
	for ( ii = 0 ; ii < NbModulesNetListe ; ii++, NameNetMod += MAX_LEN_TXT+1 )
		{
		Module = (MODULE*) m_Parent->m_Pcb->m_Modules;
		for( ;Module != NULL; Module = (MODULE*)Module->Pnext)
			{
			if( strnicmp(Module->m_Reference->GetText(), NameNetMod, MAX_LEN_TXT)
					 == 0 )
				{
				break;
				}
			}
		if( Module == NULL )
			{
			List->Append(NameNetMod);
			nberr++ ;
			}
		}

	/* recherche des modules supplementaires (i.e. Non en Netliste) */
	List->Append(_("Not in Netlist:") );

	Module = (MODULE*) m_Parent->m_Pcb->m_Modules;
	for( ;Module != NULL; Module = Module->Next())
		{
		NameNetMod = ListeNetModules;
		for (ii = 0; ii < NbModulesNetListe ; ii++, NameNetMod += MAX_LEN_TXT+1 )
			{
			if( strnicmp(Module->m_Reference->GetText(), NameNetMod, MAX_LEN_TXT)
					 == 0 )
				{
				break ;/* Module trouve en netliste */
				}
			}
		if( ii == NbModulesNetListe )	/* Module en trop */
			{
			List->Append(Module->m_Reference->GetText());
			nberr++ ;
			}
		}

	sprintf(cbuf, _("%d Errors"), nberr);

	List->ShowModal(); List->Destroy();

	MyFree(ListeNetModules);
}


/***********************************************************************************/
int WinEDA_NetlistFrame::BuildListeNetModules(wxCommandEvent& event, char * BufName)
/***********************************************************************************/
/*
	charge en BufName la liste des noms des modules de la netliste,
	( chaque nom est en longueur fixe, sizeof(pt_texte_ref->name) + code 0 )
	retourne le nombre des modules cités dans la netliste
	Si BufName == NULL: determination du nombre des Module uniquement
*/
{
int textlen;
int nb_modules_lus ;
int State, LineNum, Comment;
char Line[256], *Text, * LibModName;

	if( ! OpenNetlistFile(event) ) return -1;

	State = 0; LineNum = 0; Comment = 0;
	nb_modules_lus = 0;
	textlen = MAX_LEN_TXT;

	while( GetLine( source, Line, &LineNum) )
		{
		Text = StrPurge(Line);
		if ( Comment ) /* Commentaires en cours */
			{
			if( (Text = strchr(Text,'}') ) == NULL )continue;
			Comment = 0;
			}
		if ( *Text == '{' ) /* Commentaires */
			{
			Comment = 1;
			if( (Text = strchr(Text,'}') ) == NULL ) continue;
			}

		if ( *Text == '(' ) State++;
		if ( *Text == ')' ) State--;

		if( State == 2 )
			{
			int Error = 0;
			if( strtok(Line, " ()\t\n") == NULL ) Error = 1; /* TimeStamp */
			if( ( LibModName = strtok(NULL, " ()\t\n")) == NULL ) Error = 1; /* nom Lib */
			/* Lecture du nom (reference) du composant: */
			if( (Text = strtok(NULL, " ()\t\n")) == NULL ) Error = 1;
			nb_modules_lus++;
			if( BufName )
				{
				strncpy( BufName, Text, textlen ); BufName[textlen] = 0;
				BufName += textlen+1;
				}
			continue;
			}

		if( State >= 3 )
			{
			State--; /* Lecture 1 ligne relative au Pad */
			}
		}

	fclose(source);
	return(nb_modules_lus);
}



/*****************************************************************************************/
int WinEDA_NetlistFrame::ReadListeModules(const char * RefCmp, int TimeStamp, char * NameModule)
/*****************************************************************************************/
/*
	Lit le fichier .CMP donnant l'equivalence Modules / Composants
	Retourne:
	Si ce fichier existe:
		1 et le nom module dans NameModule
		-1 si module non trouve en fichier
	sinon 0;

	parametres d'appel:
		RefCmp		(NULL si selection par TimeStamp)
		TimeStamp	(signature temporelle si elle existe, NULL sinon)
		pointeur sur le buffer recevant le nom du module

	Exemple de fichier:

Cmp-Mod V01 Genere par PcbNew le 29/10/2003-13:11:6

BeginCmp
TimeStamp = 322D3011;
Reference = BUS1;
ValeurCmp = BUSPC;
IdModule  = BUS_PC;
EndCmp

BeginCmp
TimeStamp = 32307DE2;
Reference = C1;
ValeurCmp = 47uF;
IdModule  = CP6;
EndCmp

*/
{
wxString CmpFullFileName;
char refcurrcmp[512], idmod[512], ia[1024];
int timestamp;
char *ptcar;
FILE * FichCmp;

	if( (RefCmp == NULL) && (TimeStamp == 0) ) return(0);

	CmpFullFileName = NetNameBuffer;
	ChangeFileNameExt(CmpFullFileName,NetCmpExtBuffer) ;

	FichCmp = fopen(CmpFullFileName.GetData(),"rt");
	if (FichCmp == NULL)
		{
		wxString msg;
		msg.Printf( _("File <%s> not found, use Netlist for lib module selection"),
				CmpFullFileName.GetData()) ;
		DisplayError(this, cbuf, 20) ;
		return(0);
		}

	while( fgets(ia,sizeof(ia),FichCmp) != NULL )
		{
		if( strnicmp(ia,"BeginCmp",8) != 0 ) continue;

		/* Ici une description de 1 composant commence */
		*refcurrcmp = *idmod = 0;
		timestamp = -1;
		while( fgets(ia,sizeof(ia),FichCmp) != NULL )
			{
			if( strnicmp(ia,"EndCmp",6) == 0 ) break;

			if( strnicmp(ia,"Reference =",11) == 0 )
				{
				ptcar = ia+11;
				ptcar = strtok(ptcar," =;\t\n");
				if( ptcar ) strcpy(refcurrcmp,ptcar);
				continue;
				}

			if( strnicmp(ia,"IdModule  =",11) == 0 )
				{
				ptcar = ia+11;
				ptcar = strtok(ptcar," =;\t\n");
				if( ptcar ) strcpy(idmod,ptcar);
				continue;
				}
			if( strnicmp(ia,"TimeStamp =",11) == 0 )
				{
				ptcar = ia+11;
				ptcar = strtok(ptcar," =;\t\n");
				if( ptcar ) sscanf(ptcar, "%X", &timestamp);
				}
			}/* Fin lecture 1 descr composant */

		/* Test du Composant lu en fichier: est-il le bon */
		if( RefCmp )
			{
			if( stricmp(RefCmp, refcurrcmp) == 0 )
				{
				fclose(FichCmp);
				strcpy(NameModule, idmod);
				return(1);
				}
			}
		else if( TimeStamp != -1 )
			{
			if( TimeStamp == timestamp )
				{
				fclose(FichCmp);
				strcpy(NameModule, idmod);
				return(1);
				}
			}
		}
	fclose(FichCmp);
	return(-1);
}


/***************************************************************/
void WinEDA_NetlistFrame::Set_NetlisteName(wxCommandEvent& event)
/***************************************************************/
/* Selection un nouveau nom de netliste
	Affiche la liste des fichiers netlistes pour selection sur liste
 */
{
wxString fullfilename, mask("*");

	mask += NetExtBuffer;

	fullfilename = EDA_FileSelector( _("Netlist Selection:"),
					"",						/* Chemin par defaut */
					NetNameBuffer,			/* nom fichier par defaut */
					NetExtBuffer,			/* extension par defaut */
					mask,					/* Masque d'affichage */
					this,
					0,
					TRUE
					);

	if ( fullfilename == "" ) return;
	NetNameBuffer = fullfilename;
	SetTitle(fullfilename);
}

/***********************************************************************************/
void WinEDA_NetlistFrame::AddToList(const char * NameLibCmp, const char * CmpName,int TimeStamp )
/************************************************************************************/
/* Fontion copiant en memoire de travail les caracteristiques
	des nouveaux modules
*/
{
MODULEtoLOAD * NewMod;

	NewMod = new MODULEtoLOAD(NameLibCmp, CmpName, TimeStamp);
	NewMod->Pnext = s_ModuleToLoad_List;
	s_ModuleToLoad_List = NewMod;
	s_NbNewModules++;
}


/***************************************************************/
void WinEDA_NetlistFrame::LoadListeModules(wxDC * DC)
/***************************************************************/
/* Routine de chargement des nouveaux modules en une seule lecture des
	librairies
	Si un module vient d'etre charge il est duplique, ce qui evite une lecture
	inutile de la librairie
*/
{
MODULEtoLOAD * ref, *cmp;
int ii;
MODULE * Module = NULL;
wxPoint OldPos = m_Parent->m_CurrentScreen->m_Curseur;

	if( s_NbNewModules == 0 ) return;

	SortListModulesToLoadByLibname(s_NbNewModules);
	ref = cmp = s_ModuleToLoad_List;

	// Calcul de la coordonnée de placement des modules:
	if ( m_Parent->SetBoardBoundaryBoxFromEdgesOnly() )
	{
		m_Parent->m_CurrentScreen->m_Curseur.x = m_Parent->m_Pcb->m_BoundaryBox.GetRight() + 5000;
		m_Parent->m_CurrentScreen->m_Curseur.y = m_Parent->m_Pcb->m_BoundaryBox.GetBottom() + 10000;
	}
	else
	{
		m_Parent->m_CurrentScreen->m_Curseur = wxPoint(0,0);
	}

	for( ii = 0; ii < s_NbNewModules; ii++, cmp = cmp->Next() )
	{
		if( (ii == 0) || ( ref->m_LibName != cmp->m_LibName) )
		{	/* Nouveau Module a charger */
			Module = m_Parent->Get_Librairie_Module(this, "", cmp->m_LibName, TRUE );
			ref = cmp;
			if ( Module == NULL ) continue;
			m_Parent->Place_Module(Module, DC);
			/* mise a jour des reperes ( nom et ref "Time Stamp")
				si module charge */
			Module->m_Reference->m_Text = cmp->m_CmpName;
			Module->m_TimeStamp = cmp->m_TimeStamp;
		}

		else
		{	/* module deja charge, on peut le dupliquer */
			MODULE * newmodule;
			if ( Module == NULL ) continue;	/* module non existant en libr */
			newmodule = new MODULE(m_Parent->m_Pcb);
			newmodule->Copy(Module);
			newmodule->AddToChain(Module);
			Module = newmodule;
			Module->m_Reference->m_Text = cmp->m_CmpName;
			Module->m_TimeStamp = cmp->m_TimeStamp;
		}
	}
	m_Parent->m_CurrentScreen->m_Curseur = OldPos;
}


/* Routine utilisee par qsort pour le tri des modules a charger
*/
static int SortByLibName( MODULEtoLOAD ** ref, MODULEtoLOAD ** cmp )
{
	return (strcmp( (*ref)->m_LibName.GetData(), (*cmp)->m_LibName.GetData() ) );
}

/*************************************************/
void SortListModulesToLoadByLibname(int NbModules)
/**************************************************/
/* Rearrage la liste des modules List par ordre alphabetique des noms lib des modules
*/
{
MODULEtoLOAD ** base_list, * item;
int ii;

	base_list = (MODULEtoLOAD **) MyMalloc( NbModules * sizeof(MODULEtoLOAD *) );

	for ( ii = 0, item = s_ModuleToLoad_List; ii < NbModules; ii++ )
	{
		base_list[ii] = item;
		item = item->Next();
	}
	qsort( base_list, NbModules, sizeof(MODULEtoLOAD*),
			(int(*)(const void *, const void *) )SortByLibName);
	// Reconstruction du chainage:
	s_ModuleToLoad_List = *base_list;
	for ( ii = 0; ii < NbModules-1; ii++ )
	{
		item = base_list[ii];
		item->Pnext = base_list[ii + 1];
	}
	// Dernier item: Pnext = NULL:
	item = base_list[ii];
	item->Pnext = NULL;
	
	free(base_list);
}


/*****************************************************************************/
MODULEtoLOAD::MODULEtoLOAD(const wxString & libname, const wxString & cmpname,
			int timestamp) : EDA_BaseStruct(TYPE_NOT_INIT)
/*****************************************************************************/
{
	m_LibName = libname;
	m_CmpName = cmpname;
	m_TimeStamp = timestamp;
}
