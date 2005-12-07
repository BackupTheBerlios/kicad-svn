	/**********************************/
	/* Dilaog box for netlist outputs */
	/**********************************/

#include "fctsys.h"

//#include "gr_basic.h"
#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"

#include "netlist.h"	/* Definitions generales liees au calcul de netliste */
#include "protos.h"


// ID for configuration:
#define CUSTOM_NETLIST_TITLE wxT("CustomNetlistTitle")
#define CUSTOM_NETLIST_COMMAND wxT("CustomNetlistCommand")

/* Routines locales */

/* Variable locales */

enum id_netlist {
    ID_CREATE_NETLIST = 1550,
	ID_CURRENT_FORMAT_IS_DEFAULT,
	ID_CLOSE_NETLIST,
	ID_RUN_SIMULATOR,
	ID_NETLIST_NOTEBOOK
};

enum panel_netlist_index {
	PANELPCBNEW = 0,	// Create Netlist format Pcbnew
	PANELORCADPCB2,		// Create Netlis format OracdPcb2
	PANELCADSTAR,		// Create Netlis format OracdPcb2
	PANELSPICE,			// Create Netlis format Pspice
	PANELCUSTOMBASE		// Start auxiliary panels (custom netlists)
};

/* wxPanels for creating the NoteBook pages for each netlist format:
*/
class EDA_NoteBookPage: public wxPanel
{
public:
	int m_IdNetType;
	wxCheckBox * m_IsCurrentFormat;
	WinEDA_EnterText * m_CommandStringCtrl;
	WinEDA_EnterText * m_TitleStringCtrl;
	wxButton * m_ButtonCancel;

	EDA_NoteBookPage(wxNotebook* parent, const wxString & title,
			int id_NetType, int idCheckBox, int idCreateFile);
	~EDA_NoteBookPage(void) {};
};


/*****************************************************************************/
EDA_NoteBookPage::EDA_NoteBookPage(wxNotebook* parent, const wxString & title,
		int id_NetType, int idCheckBox, int idCreateFile) :
		wxPanel(parent, -1 )
/*****************************************************************************/
{
	SetFont(*g_DialogFont);
	m_IdNetType = id_NetType;
	m_CommandStringCtrl = NULL;
	m_TitleStringCtrl = NULL;
	m_IsCurrentFormat = NULL;
	m_ButtonCancel = NULL;

	parent->AddPage(this, title, g_NetFormat == m_IdNetType);

	if ( idCheckBox )
	{
		wxPoint pos;
		pos.x = 5; pos.y = 15;
		new wxStaticBox(this, -1, _(" Options : "), pos, wxSize(210, 50));

		pos.x = 10; pos.y += 20;
		m_IsCurrentFormat = new wxCheckBox(this, idCheckBox,
			_("Default format"), pos);

		if ( g_NetFormat == m_IdNetType )
			m_IsCurrentFormat->SetValue(TRUE);

	}

	if ( idCreateFile )	// Create the 2 standard buttons: Create File ans Cancel
	{
		wxPoint pos;
		wxButton * Button;
		pos.x = 230; pos.y = 10;
		Button = new wxButton(this, idCreateFile,
							_("&Netlist"), pos);
		Button->SetForegroundColour(*wxRED);

		pos.y += Button->GetDefaultSize().y + 10;
		m_ButtonCancel =
			Button = new wxButton(this,	ID_CLOSE_NETLIST,
							_("&Close"), pos);
		Button->SetForegroundColour(*wxBLUE);
	}
}

#define CUSTOMPANEL_COUNTMAX 8
/* Dialog frame for creating netlists */
class WinEDA_NetlistFrame: public wxDialog
{
public:
	WinEDA_DrawFrame * m_Parent;
	wxNotebook* m_NoteBook;
	EDA_NoteBookPage * m_PanelNetType[4+CUSTOMPANEL_COUNTMAX];

	wxRadioBox * m_UseNetNamesInNetlist;

public:
	// Constructor and destructor
	WinEDA_NetlistFrame(WinEDA_DrawFrame *parent, wxPoint& pos);
	~WinEDA_NetlistFrame(void) {};

private:
	void InstallCustomPages(void);
	void InstallPageSpice(void);
	void GenNetlist(wxCommandEvent& event);
	void RunSimulator(wxCommandEvent& event);
	void NetlistUpdateOpt(void);
	void NetlistExit(wxCommandEvent& event);
	void SelectNetlistType(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()};

BEGIN_EVENT_TABLE(WinEDA_NetlistFrame, wxDialog)
	EVT_BUTTON(ID_CLOSE_NETLIST, WinEDA_NetlistFrame::NetlistExit)
	EVT_BUTTON(ID_CREATE_NETLIST, WinEDA_NetlistFrame::GenNetlist)
	EVT_CHECKBOX(ID_CURRENT_FORMAT_IS_DEFAULT, WinEDA_NetlistFrame::SelectNetlistType)
	EVT_BUTTON(ID_RUN_SIMULATOR, WinEDA_NetlistFrame::RunSimulator)
END_EVENT_TABLE()



void InstallNetlistFrame(WinEDA_DrawFrame *parent, wxPoint & pos)
{
	WinEDA_NetlistFrame * frame = new WinEDA_NetlistFrame(parent, pos);
	frame->ShowModal(); frame->Destroy();
}

#define H_SIZE 360
#define V_SIZE 230

/*************************************************************************************/
WinEDA_NetlistFrame::WinEDA_NetlistFrame(WinEDA_DrawFrame *parent, wxPoint& framepos):
		wxDialog(parent, -1, _("Netlist"), framepos, wxSize(H_SIZE, V_SIZE),
				DIALOG_STYLE)
/*************************************************************************************/
{
wxPoint pos;
int ii;

	m_Parent = parent;
	SetFont(*g_DialogFont);
	if ( g_NetFormat == NET_TYPE_NOT_INIT )
			g_NetFormat = NET_TYPE_PCBNEW;

	for ( ii = 0; ii < PANELCUSTOMBASE+CUSTOMPANEL_COUNTMAX; ii ++ )
	{
		m_PanelNetType[ii] = NULL;
	}

	if ( (framepos.x == -1) && (framepos.x == -1) ) Centre();

	m_NoteBook = new wxNotebook(this, ID_NETLIST_NOTEBOOK,
   		wxDefaultPosition,wxSize(H_SIZE-6, V_SIZE - 28));
	m_NoteBook->SetFont(*g_DialogFont);


	wxLayoutConstraints* c = new wxLayoutConstraints;
	c->left.SameAs(this, wxLeft, 4);
	c->right.SameAs(this, wxRight, 4);
	c->top.SameAs(this, wxTop, 4);
	c->bottom.SameAs(this, wxBottom, 10);
	m_NoteBook->SetConstraints(c);

	// Add panels

	// Add Panel FORMAT PCBNEW
	m_PanelNetType[PANELPCBNEW] = new EDA_NoteBookPage(m_NoteBook, wxT("Pcbnew"), NET_TYPE_PCBNEW,
		ID_CURRENT_FORMAT_IS_DEFAULT, ID_CREATE_NETLIST);

	// Add Panel FORMAT ORCADPCB2
	m_PanelNetType[PANELORCADPCB2] = new EDA_NoteBookPage(m_NoteBook, wxT("OrcadPCB2"), NET_TYPE_ORCADPCB2,
		ID_CURRENT_FORMAT_IS_DEFAULT, ID_CREATE_NETLIST);

	// Add Panel FORMAT CADSTAR
	m_PanelNetType[PANELCADSTAR] = new EDA_NoteBookPage(m_NoteBook, wxT("CadStar"), NET_TYPE_CADSTAR,
		ID_CURRENT_FORMAT_IS_DEFAULT, ID_CREATE_NETLIST);

	// Add Panel spice
	InstallPageSpice();
	
	// Add custom panels:
	InstallCustomPages();
}


/*************************************************/
void WinEDA_NetlistFrame::InstallPageSpice(void)
/*************************************************/
/* Create the spice page
*/
{
wxPoint pos;
wxButton * Button;
	
	m_PanelNetType[PANELSPICE] = new EDA_NoteBookPage(m_NoteBook, wxT("Spice"), NET_TYPE_SPICE, 0, 0);

	pos.x = 10; pos.y = 5;
	m_PanelNetType[PANELSPICE]->m_IsCurrentFormat = new wxCheckBox(m_PanelNetType[PANELSPICE],ID_CURRENT_FORMAT_IS_DEFAULT,
					_("Default format"), pos);
	m_PanelNetType[PANELSPICE]->m_IsCurrentFormat->SetValue( g_NetFormat == NET_TYPE_SPICE);

	pos.y += 25;
wxString netlist_opt[2] = { _("Use Net Names"), _("Use Net Numbers") };
	m_UseNetNamesInNetlist = new wxRadioBox(m_PanelNetType[PANELSPICE],-1, _("Netlist Options:"),
			pos, wxDefaultSize,
			2, netlist_opt, 1, wxRA_SPECIFY_COLS);
	if ( ! g_OptNetListUseNames ) m_UseNetNamesInNetlist->SetSelection(1);

	int xx, yy;
	m_UseNetNamesInNetlist->GetSize(&xx, &yy);
	pos.x = 5; pos.y += yy + 20;
	m_PanelNetType[PANELSPICE]->m_CommandStringCtrl = new WinEDA_EnterText(m_PanelNetType[PANELSPICE],
				_("Simulator command:"), g_SimulatorCommandLine,
				pos, wxSize(H_SIZE- 20, -1) );
	// Add buttons
	pos.x = 230; pos.y = 5;
	Button = new wxButton(m_PanelNetType[PANELSPICE], ID_CREATE_NETLIST,
						_("Netlist"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.y += Button->GetSize().y + 5;
	Button = new wxButton(m_PanelNetType[PANELSPICE], ID_RUN_SIMULATOR,
						_("&Run Simulator"), pos);
	Button->SetForegroundColour(wxColour(0,100,0));

	pos.y += Button->GetSize().y + 5;
	Button = new wxButton(m_PanelNetType[PANELSPICE], ID_CLOSE_NETLIST,
						_("&Close"), pos);
	Button->SetForegroundColour(*wxBLUE);
}

/*************************************************/
void WinEDA_NetlistFrame::InstallCustomPages(void)
/*************************************************/
/* create the pages for custom netlist format selection:
*/
{
int ii, CustomCount;
wxString title, previoustitle, msg;
wxPoint pos;
EDA_NoteBookPage * CurrPage;

	CustomCount = CUSTOMPANEL_COUNTMAX;
	previoustitle = wxT("dummy_title");
	for ( ii = 0; ii < CustomCount; ii++ )
	{
		msg = CUSTOM_NETLIST_TITLE; msg << ii+1;
		title = m_Parent->m_Parent->m_EDA_Config->Read(msg);
		
		// Install the panel only if it is the first non initialised
		if ( (title.IsEmpty()) && ( previoustitle.IsEmpty() ) ) break;

		previoustitle = title;
		CurrPage = m_PanelNetType[PANELCUSTOMBASE + ii] =
			new EDA_NoteBookPage(m_NoteBook, title,
			NET_TYPE_CUSTOM1 + ii,
			ID_CURRENT_FORMAT_IS_DEFAULT , ID_CREATE_NETLIST);

		pos.x = 5;
		pos.y = CurrPage->m_ButtonCancel->GetRect().GetBottom() + 10;
		msg = CUSTOM_NETLIST_COMMAND; msg << ii+1;
		wxString Command = m_Parent->m_Parent->m_EDA_Config->Read(msg);
		CurrPage->m_CommandStringCtrl = new WinEDA_EnterText(CurrPage,
					_("Netlist command:"), Command,
					pos, wxSize(H_SIZE- 10, -1) );

		pos.y += CurrPage->m_CommandStringCtrl->GetDimension().y + 20;
		CurrPage->m_TitleStringCtrl = new WinEDA_EnterText(CurrPage,
					_("Title:"), title,
					pos, wxSize(H_SIZE- 10, -1) );
	}
}


// Fonctions de positionnement des variables d'option
/*****************************************************************/
void WinEDA_NetlistFrame::SelectNetlistType(wxCommandEvent& event)
/*****************************************************************/
{
int ii;
EDA_NoteBookPage * CurrPage;

	for ( ii = 0; ii < PANELCUSTOMBASE+CUSTOMPANEL_COUNTMAX; ii++ )
		if ( m_PanelNetType[ii] )
			m_PanelNetType[ii]->m_IsCurrentFormat->SetValue(FALSE);

	CurrPage = (EDA_NoteBookPage *) m_NoteBook->GetCurrentPage();
	if ( CurrPage == NULL ) return;
		
	g_NetFormat = CurrPage->m_IdNetType;
	CurrPage->m_IsCurrentFormat->SetValue(TRUE);

}

/***********************************************/
void WinEDA_NetlistFrame::NetlistUpdateOpt(void)
/***********************************************/
{
int ii;
	
	g_SimulatorCommandLine = m_PanelNetType[PANELSPICE]->m_CommandStringCtrl->GetData();
	g_NetFormat = NET_TYPE_PCBNEW;

	for ( ii = 0; ii < PANELCUSTOMBASE+CUSTOMPANEL_COUNTMAX; ii++ )
	{
		if ( m_PanelNetType[ii] == NULL ) break;
		if ( m_PanelNetType[ii]->m_IsCurrentFormat->GetValue() == TRUE )
			g_NetFormat = m_PanelNetType[ii]->m_IdNetType;
	}

	g_OptNetListUseNames = TRUE;	// Used for pspice, gnucap
	if ( m_UseNetNamesInNetlist->GetSelection() == 1 )
		g_OptNetListUseNames = FALSE;
}

/**********************************************************/
void WinEDA_NetlistFrame::GenNetlist(wxCommandEvent& event)
/**********************************************************/
{
wxString FullFileName, FileExt, Mask;
int netformat_tmp = g_NetFormat;

	NetlistUpdateOpt();

EDA_NoteBookPage * CurrPage;
	
	CurrPage = (EDA_NoteBookPage *) m_NoteBook->GetCurrentPage();
	g_NetFormat = CurrPage->m_IdNetType;
	
	/* Calcul du nom du fichier netlist */
	FullFileName = ScreenSch->m_FileName;

	switch ( g_NetFormat )
	{	
		case NET_TYPE_SPICE:
			FileExt = wxT(".cir");
			break;
		case NET_TYPE_CADSTAR:
			FileExt = wxT(".frp");
			break;
		default:
			FileExt = g_NetExtBuffer;
			break;
	}

	Mask = wxT("*") + FileExt + wxT("*");
	ChangeFileNameExt(FullFileName, FileExt);

	FullFileName = EDA_FileSelector( _("Netlist files:"),
					wxEmptyString,					/* Chemin par defaut */
					FullFileName,		/* nom fichier par defaut */
					FileExt,		  	/* extension par defaut */
					Mask,				/* Masque d'affichage */
					this,
					wxSAVE,
					TRUE
					);
	if ( FullFileName.IsEmpty() ) return;

	m_Parent->MsgPanel->EraseMsgBox();

	if( CheckAnnotate(m_Parent, 0) )
	{
		if( !IsOK( this, _("Must be Annotated, Continue ?"))  )
			return;
	}


	SchematicCleanUp(NULL);
	BuildNetList(m_Parent, ScreenSch);
	if ( CurrPage->m_CommandStringCtrl)
		g_NetListerCommandLine = CurrPage->m_CommandStringCtrl->GetData();
	else g_NetListerCommandLine.Empty();
		
	switch (g_NetFormat)
	{
		default:
			WriteNetList(m_Parent, FullFileName, TRUE);
			break;

		case NET_TYPE_CADSTAR:
		case NET_TYPE_ORCADPCB2:
			WriteNetList(m_Parent, FullFileName, FALSE);

		case NET_TYPE_SPICE:
			g_OptNetListUseNames = TRUE;	// Used for pspice, gnucap
			if ( m_UseNetNamesInNetlist->GetSelection() == 1 )
				g_OptNetListUseNames = FALSE;
			WriteNetList(m_Parent, FullFileName, g_OptNetListUseNames);
			break;
	}
	FreeTabNetList(g_TabObjNet, g_NbrObjNet );
	g_NetFormat = netformat_tmp;

	NetlistExit(event);
}

/***********************************************************/
void WinEDA_NetlistFrame::NetlistExit(wxCommandEvent& event)
/***********************************************************/
{
wxString msg, Command;
	
	NetlistUpdateOpt();
	// Update the new titles
	for ( int ii = 0; ii < CUSTOMPANEL_COUNTMAX; ii++ )
	{
		EDA_NoteBookPage * CurrPage = m_PanelNetType[ii + PANELCUSTOMBASE];
		if ( CurrPage == NULL ) break;
		msg = wxT("Custom"); msg << ii+1;
		if ( CurrPage->m_TitleStringCtrl )
		{
			wxString title = CurrPage->m_TitleStringCtrl->GetData();
			if ( msg != title )	// Title has changed, Update config
			{
				msg = CUSTOM_NETLIST_TITLE; msg << ii+1;
				m_Parent->m_Parent->m_EDA_Config->Write(msg, title);
			}
		}
		
		if ( CurrPage->m_CommandStringCtrl )
		{
			Command = CurrPage->m_CommandStringCtrl->GetData();
			msg = CUSTOM_NETLIST_COMMAND; msg << ii+1;
			m_Parent->m_Parent->m_EDA_Config->Write(msg, Command);
		}
	}
	Close();
}


/***********************************************************/
void WinEDA_NetlistFrame::RunSimulator(wxCommandEvent& event)
/***********************************************************/
{
wxString NetlistFullFileName, ExecFile, CommandLine;

	g_SimulatorCommandLine = m_PanelNetType[PANELSPICE]->m_CommandStringCtrl->GetData();
	g_SimulatorCommandLine.Trim(FALSE);
	g_SimulatorCommandLine.Trim(TRUE);
	ExecFile = g_SimulatorCommandLine.BeforeFirst(' ');

	CommandLine = g_SimulatorCommandLine.AfterFirst(' ');

	/* Calcul du nom du fichier netlist */
	NetlistFullFileName = ScreenSch->m_FileName;
	ChangeFileNameExt(NetlistFullFileName, wxT(".cir"));
	AddDelimiterString(NetlistFullFileName);
	CommandLine += wxT(" ") + NetlistFullFileName;

	ExecuteFile(this, ExecFile, CommandLine);
}

