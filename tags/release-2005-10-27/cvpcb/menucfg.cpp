	/***************************************/
	/** menucfg : configuration de CVPCB  **/
	/***************************************/

/* cree et/ou affiche et modifie la configuration de CVPCB */

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWindows headers
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "fctsys.h"
#include "common.h"

#include "cvpcb.h"
#include "protos.h"

enum {
	SAVE_CFG = 1800,
	ID_READ_OLDCFG,
	ID_ACCEPT_CFG,
	DEL_LIB,
	ADD_LIB,
	INSERT_LIB,
	DEL_EQU,
	ADD_EQU,
	INSERT_EQU,
	FORMAT_NETLIST
	};


/* Routines Locales */


	/*****************************************/
	/* classe pour la frame de Configuration */
	/*****************************************/

class ConfigCvpcbFrame: public wxDialog
{
public:
	WinEDA_CvpcbFrame * m_Parent;
	wxListBox * m_ListLibr;
	wxListBox * m_ListEquiv;
	wxRadioBox *m_NetFormatBox;
	WinEDA_EnterText * m_LibDirCtrl;
	WinEDA_EnterText * m_NetInputExtCtrl;
	WinEDA_EnterText * m_PkgExtCtrl;
	WinEDA_EnterText * m_TextHelpModulesFileName;
	bool m_DoUpdate;

public:
	// Constructor and destructor
	ConfigCvpcbFrame(WinEDA_CvpcbFrame *parent, wxPoint& pos);
	~ConfigCvpcbFrame(void);

private:
	bool OnClose(void);
	bool OnQuit(void);
	void Update(void);
	void AcceptCfg(wxCommandEvent& event);

	void CreateListFormatsNetListes(wxPoint pos);

	void SaveCfg(wxCommandEvent& event);
	void ReadOldCfg(wxCommandEvent& event);
	void LibDelFct(wxCommandEvent& event);
	void LibAddFct(wxCommandEvent& event);
	void EquDelFct(wxCommandEvent& event);
	void EquAddFct(wxCommandEvent& event);
	void ReturnNetFormat(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

};
/* Construction de la table des evenements pour FrameClassMain */
BEGIN_EVENT_TABLE(ConfigCvpcbFrame, wxDialog)
	EVT_BUTTON(SAVE_CFG, ConfigCvpcbFrame::SaveCfg)
	EVT_BUTTON(ID_ACCEPT_CFG, ConfigCvpcbFrame::AcceptCfg)
	EVT_BUTTON(ID_READ_OLDCFG, ConfigCvpcbFrame::ReadOldCfg)
	EVT_BUTTON(DEL_LIB, ConfigCvpcbFrame::LibDelFct)
	EVT_BUTTON(ADD_LIB, ConfigCvpcbFrame::LibAddFct)
	EVT_BUTTON(INSERT_LIB, ConfigCvpcbFrame::LibAddFct)
	EVT_BUTTON(DEL_EQU, ConfigCvpcbFrame::EquDelFct)
	EVT_BUTTON(ADD_EQU, ConfigCvpcbFrame::EquAddFct)
	EVT_BUTTON(INSERT_EQU, ConfigCvpcbFrame::EquAddFct)
	EVT_RADIOBOX(FORMAT_NETLIST, ConfigCvpcbFrame::ReturnNetFormat)
END_EVENT_TABLE()



/***************************************************/
void WinEDA_CvpcbFrame::CreateConfigWindow(void)
/***************************************************/
/* Creation de la fenetre de configuration de CVPCB */
{
wxPoint pos = GetPosition();

	pos.x += 5; pos.y += 10;

ConfigCvpcbFrame * ConfigFrame = new ConfigCvpcbFrame(this, pos);

	ConfigFrame->ShowModal(); ConfigFrame->Destroy();
}

	/********************************************************/
	/* Constructeur de ConfigCvpcbFrame: la fenetre de config */
	/********************************************************/
#define XSIZE 650
#define YSIZE 420
#define LEN_EXT 150
ConfigCvpcbFrame::ConfigCvpcbFrame(WinEDA_CvpcbFrame *parent, wxPoint& winpos):
		wxDialog(parent, -1, "", winpos, wxSize(XSIZE, YSIZE),
		DIALOG_STYLE )
{
int dimy = 39;
wxSize size;
wxPoint pos;
wxString text;
wxString title;
int lowY;
wxSize winsize(XSIZE, YSIZE);
	
	m_Parent = parent;
	m_DoUpdate = TRUE;

	SetFont(*g_DialogFont);

	title = _("from ") + EDA_Appl->m_CurrentOptionFile;
	SetTitle(title);

	pos.x = 5; pos.y = 5;
	/* Creation des boutons de commande */
	size.x = -1; size.y = -1;
	wxButton * Button = new wxButton(this, SAVE_CFG,
						_("Save Cfg"),
						pos, size, 0);
	Button->SetForegroundColour(*wxRED);

	pos.y += Button->GetSize().y + 5;
	Button = new wxButton(this, ID_READ_OLDCFG,
						_("Read Cfg"),
						pos, size, 0);
	Button->SetForegroundColour(*wxBLUE);

	pos.y += Button->GetSize().y + 5;
	Button = new wxButton(this, ID_ACCEPT_CFG,
						_("OK"),
						pos, size, 0);
	Button->SetForegroundColour(wxColor(0,100,0) );
	lowY = pos.y + Button->GetSize().y + 5;

	/*** Creation des listes et boutons de gestion des librairies ***/
#define LISTLIB_POSX 170
#define LISTEQU_POSX 410
	pos.x = LISTLIB_POSX; pos.y = 5;
	Button = new wxButton(this, DEL_LIB, _("Del"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.x += Button->GetSize().x;
	Button = new wxButton(this, ADD_LIB, _("Add"), pos);
	Button->SetForegroundColour(wxColor(0,80,0));

	pos.x += Button->GetSize().x;
	Button = new wxButton(this, INSERT_LIB, _("Ins"), pos);
	Button->SetForegroundColour(*wxBLUE);

	pos.x = LISTEQU_POSX;
	Button = new wxButton(this, DEL_EQU, _("Del"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.x += Button->GetSize().x;
	Button = new wxButton(this, ADD_EQU, _("Add"), pos);
	Button->SetForegroundColour(wxColor(0,80,0));

	pos.x += Button->GetSize().x;
	Button = new wxButton(this, INSERT_EQU, _("Ins"), pos);
	Button->SetForegroundColour(*wxBLUE);

	pos.x = 3; pos.y = lowY;
	CreateListFormatsNetListes(pos);

	pos.x = LISTLIB_POSX; pos.y = 35;
	wxStaticText * Msg = new wxStaticText(this, -1, _("Libraries"),
							pos, wxSize(-1,-1), 0 );
	pos.y += 15;
	m_ListLibr = new wxListBox(this,
							-1,
							pos, wxSize(220,210),
							0,NULL,
							wxLB_ALWAYS_SB|wxLB_SINGLE);
	Msg->SetForegroundColour(wxColour(200,0,0) );
	m_ListLibr->InsertItems(g_LibName_List,0);

	pos.x = LISTEQU_POSX; pos.y = 35;
	Msg = new wxStaticText(this, -1, _("Equiv"),
						 pos, wxSize(-1,-1), 0 );
	pos.y += 15;
	m_ListEquiv = new wxListBox(this, -1,
							pos, wxSize(200,210),
							0,NULL,
							wxLB_ALWAYS_SB|wxLB_SINGLE);
	Msg->SetForegroundColour(wxColour(220,0,0) );
	m_ListEquiv->InsertItems(g_ListName_Equ,0);

	pos.x = LISTLIB_POSX; pos.y = 280;
	size.x = winsize.x - pos.x -10; size.y = -1;
	m_LibDirCtrl = new WinEDA_EnterText(this,
				_("Lib Dir:"), g_UserLibDirBuffer,
				pos, size);

	size.x = LEN_EXT;
	pos.y += dimy;
	m_NetInputExtCtrl = new WinEDA_EnterText(this,
				_("Net Input Ext:"),NetInExtBuffer,
				pos, size);


	pos.x += LEN_EXT + 30;
	m_PkgExtCtrl = new WinEDA_EnterText(this,
				_("Pkg Ext:"), PkgInExtBuffer,
				pos, size);
				
	pos.x = LISTLIB_POSX; pos.y += m_PkgExtCtrl->GetDimension().y + 25;
	size.x = winsize.x - pos.x -10;
	wxString DocModuleFileName =
		EDA_Appl->m_EDA_CommonConfig->Read("module_doc_file", "pcbnew/footprints.pdf");
	m_TextHelpModulesFileName = new WinEDA_EnterText(this,
				_("Module Doc File:"),  DocModuleFileName,
				pos, size);
	winsize.y = pos.y + m_TextHelpModulesFileName->GetDimension().y + 5;

	pos.x = 3; pos.y = lowY + 120;
	size.x = 130; size.y = 120;
	new wxStaticBox(this, -1,_("Files ext:"), pos, size);

	pos.x += 5; pos.y += 15;
	text.Printf("%s     %s", _("Cmp ext:"), g_ExtCmpBuffer.GetData() );
	new wxStaticText(this, -1,text , pos);

	pos.y += 15;
	text.Printf("%s      %s", _("Lib ext:"), LibExtBuffer.GetData());
	new wxStaticText(this, -1,text , pos);

	pos.y += 15;
	text.Printf("%s %s", _("NetOut ext:"), NetExtBuffer.GetData());
	new wxStaticText(this, -1,text , pos);

	pos.y += 15;
	text.Printf("%s  %s", _("Equiv ext:"), g_EquivExtBuffer.GetData());
	new wxStaticText(this, -1,text , pos);

	pos.y += 15;
	text.Printf("%s  %s", _("Retro ext:"), ExtRetroBuffer.GetData());
	new wxStaticText(this, -1,text , pos);

	SetClientSize(winsize);
}


	/*****************************************************/
	/* Destructeur de ConfigCvpcbFrame: la fenetre de config */
	/*****************************************************/

ConfigCvpcbFrame::~ConfigCvpcbFrame(void)
{
}

/***************************************************/
void ConfigCvpcbFrame::AcceptCfg(wxCommandEvent& event)
/**************************************************/
{
	Update();
	Close();
}

/**********************************/
void ConfigCvpcbFrame::Update(void)
/**********************************/
{
wxString msg;
	
	if ( ! m_DoUpdate ) return;
	NetInExtBuffer = m_NetInputExtCtrl->GetData();
	PkgInExtBuffer = m_PkgExtCtrl->GetData();
	EDA_Appl->m_EDA_CommonConfig->Write("module_doc_file",
			m_TextHelpModulesFileName->GetData());

	msg = m_LibDirCtrl->GetData();
	if ( msg != g_UserLibDirBuffer )
	{
		g_UserLibDirBuffer = m_LibDirCtrl->GetData();
		SetRealLibraryPath("modules");
		listlib();
		ListModIsModified = 1;
		m_Parent->BuildModListBox();
	}
}

/***********************************/
bool ConfigCvpcbFrame::OnClose(void)
/***********************************/
{
wxCommandEvent event;
	return TRUE;
}

/****************************************************/
void ConfigCvpcbFrame::SaveCfg(wxCommandEvent& event)
/****************************************************/
{
	Update();
	Save_Config(this);
}

/******************************************************/
void ConfigCvpcbFrame::ReadOldCfg(wxCommandEvent& event)
/******************************************************/
{
char line[1024];

	NetInNameBuffer.Replace("\\", "/");
	
wxString FullFileName = NetInNameBuffer.AfterLast('/');
	
	ChangeFileNameExt( FullFileName, g_Prj_Config_Filename_ext );

	FullFileName = EDA_FileSelector(_("Read config file"),
					wxGetCwd(),					/* Chemin par defaut */
					FullFileName,		/* nom fichier par defaut */
					g_Prj_Config_Filename_ext,				/* extension par defaut */
					FullFileName,				/* Masque d'affichage */
					this,
					wxOPEN,
					TRUE				/* ne change pas de repertoire courant */
					);
	if ( FullFileName == "") return;
	if ( ! wxFileExists(FullFileName) )
		{
		sprintf(line,_("File %s not found"), FullFileName.GetData());
		DisplayError(this, line); return;
		}

	Read_Config( FullFileName );
	m_DoUpdate = FALSE;
	Close(TRUE);
}


/*******************************************************/
void ConfigCvpcbFrame::LibDelFct(wxCommandEvent& event)
/*******************************************************/
{
int ii;
	
	ii = m_ListLibr->GetSelection();
	if ( ii < 0 ) return;

	ListModIsModified = 1;
	g_LibName_List.RemoveAt(ii);

	/* suppression de la reference dans la liste des librairies */
	m_ListLibr->Delete(ii);
	
	g_UserLibDirBuffer = m_LibDirCtrl->GetData();
	SetRealLibraryPath("modules");
	listlib();

	m_Parent->BuildModListBox();

}

/********************************************************/
void ConfigCvpcbFrame::LibAddFct(wxCommandEvent& event)
/********************************************************/
{
int ii;
wxString FullFileName, ShortLibName, mask;

	ii = m_ListLibr->GetSelection();
	if ( event.GetId() == ADD_LIB )	/* Ajout apres selection */
		{
		ii ++;
		}
	if ( ii < 0 ) ii = 0;

	Update();
	mask = "*" + LibExtBuffer;
	FullFileName = EDA_FileSelector( _("Libraries"),
					g_RealLibDirBuffer,		/* Chemin par defaut */
					"",					/* nom fichier par defaut */
					LibExtBuffer,		/* extension par defaut */
					mask,				/* Masque d'affichage */
					this,
					0,
					TRUE				/* ne chage pas de repertoire courant */
					);
	if (FullFileName == "" ) return;

	ShortLibName = MakeReducedFileName(FullFileName,g_RealLibDirBuffer,LibExtBuffer);

	g_LibName_List.Insert(ShortLibName, ii);
	
	g_UserLibDirBuffer = m_LibDirCtrl->GetData();
	SetRealLibraryPath("modules");
	listlib();
	ListModIsModified = 1;

	m_Parent->BuildModListBox();

	m_ListLibr->Clear();
	m_ListLibr->InsertItems(g_LibName_List, 0);

}


/********************************************************/
void ConfigCvpcbFrame::EquDelFct(wxCommandEvent& event)
/********************************************************/
{
int ii;

	ii = m_ListEquiv->GetSelection();
	if ( ii < 0 ) return;

	g_ListName_Equ.RemoveAt(ii);
	m_ListEquiv->Delete(ii);
}

/********************************************************/
void ConfigCvpcbFrame::EquAddFct(wxCommandEvent& event)
/********************************************************/
{
int ii;
wxString FullFileName, ShortLibName, mask;

	ii = m_ListEquiv->GetSelection();
	if ( event.GetId() == ADD_EQU ) ii ++;	/* Ajout apres selection */
	if ( ii < 0 ) ii = 0;

	Update();
	mask = "*" + g_EquivExtBuffer;
	FullFileName = EDA_FileSelector( _("Equiv"),
					g_RealLibDirBuffer,		/* Chemin par defaut */
					"",					/* nom fichier par defaut */
					g_EquivExtBuffer,		/* extension par defaut */
					mask,				/* Masque d'affichage */
					this,
					0,
					TRUE				/* ne chage pas de repertoire courant */
					);

	if (FullFileName == "" ) return;

	ShortLibName = MakeReducedFileName(FullFileName,g_RealLibDirBuffer,g_EquivExtBuffer);

	g_ListName_Equ.Insert(ShortLibName, ii);

	/* Mise a jour de l'affichage */
	g_UserLibDirBuffer = m_LibDirCtrl->GetData();
	SetRealLibraryPath("modules");
	listlib();
	
	m_ListEquiv->Clear();
	m_ListEquiv->InsertItems(g_ListName_Equ, 0);
}




/*****************************************************************/
void ConfigCvpcbFrame::ReturnNetFormat(wxCommandEvent& event)
/*****************************************************************/
{
int ii;

	ii = m_NetFormatBox->GetSelection();
	g_NetType = TYPE_ORCADPCB2;
	if ( ii == 1 ) g_NetType = TYPE_VIEWLOGIC_WIR;
	if ( ii == 2 ) g_NetType = TYPE_VIEWLOGIC_NET;

}


/*************************************************************/
void ConfigCvpcbFrame::CreateListFormatsNetListes(wxPoint pos)
/************************************************************/
{
wxString Net_Select[] =
	{"&PcbNew", "&ViewLogic", "View &Net + Pkg"};

	m_NetFormatBox = new wxRadioBox(this, FORMAT_NETLIST,
						_("NetList Formats:"),
						pos, wxSize(-1,-1),
						3,Net_Select,1,wxRA_SPECIFY_COLS);

	switch( g_NetType )
		{
		case TYPE_NON_SPECIFIE:
		case TYPE_ORCADPCB2:
			m_NetFormatBox->SetSelection(0);
			break;

		case TYPE_PCAD:
			break;

		case TYPE_VIEWLOGIC_WIR:
			m_NetFormatBox->SetSelection(1);
			break;

		case TYPE_VIEWLOGIC_NET:
			m_NetFormatBox->SetSelection(2);
			break;

		default:
			break;
		}
}

