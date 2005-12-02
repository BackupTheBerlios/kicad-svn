	/******************************************************************/
	/* mdiframe.cpp - fonctions de la classe du type WinEDA_MainFrame */
	/******************************************************************/

#ifdef __GNUG__
#pragma implementation
#endif

#include "fctsys.h"

#include "common.h"

#include "bitmaps.h"
#include "protos.h"

#include "id.h"

#include "kicad.h"


	/****************/
	/* Constructeur */
	/****************/

WinEDA_MainFrame::WinEDA_MainFrame(WinEDA_App * eda_app,
						wxWindow *parent, const wxString & title,
						const wxPoint& pos, const wxSize& size):
		WinEDA_BasicFrame(parent, KICAD_MAIN_FRAME, eda_app, title, pos, size )
{
wxString msg;
wxSize clientsize;

	m_FrameName = "KicadFrame";
	m_VToolBar = NULL;
	m_LeftWin = NULL;
	m_BottomWin = NULL;
	m_CommandWin = NULL;
	
	GetSettings();
	SetSize(m_FramePos.x, m_FramePos.y, m_FrameSize.x, m_FrameSize.y);

	// ajuste la ligne de status
int dims[3] = { -1, -1, 100};
	CreateStatusBar(3);
	SetStatusWidths(3,dims);

	// Give an icon
	SetIcon(wxICON(kicad_icon));

	clientsize = GetClientSize();

  // Left window: fenetre des arborescences du projet
	m_LeftWin = new WinEDA_PrjFrame(this, wxDefaultPosition, wxDefaultSize);
	m_LeftWin->SetDefaultSize(wxSize(200, clientsize.y));
	m_LeftWin->SetOrientation(wxLAYOUT_VERTICAL);
	m_LeftWin->SetAlignment(wxLAYOUT_LEFT);
	m_LeftWin->SetSashVisible(wxSASH_RIGHT, TRUE);
	m_LeftWin->SetExtraBorderSize(10);

	// Bottom Window: fenetre des messages
	m_BottomWin = new wxSashLayoutWindow(this, ID_BOTTOM_FRAME,
				wxDefaultPosition, wxDefaultSize,
				wxNO_BORDER|wxSW_3D);
	m_BottomWin->SetDefaultSize(wxSize(clientsize.x, 150));
	m_BottomWin->SetOrientation(wxLAYOUT_HORIZONTAL);
	m_BottomWin->SetAlignment(wxLAYOUT_BOTTOM);
	m_BottomWin->SetSashVisible(wxSASH_TOP, TRUE);

	m_DialogWin = new wxTextCtrl(m_BottomWin, ID_MAIN_DIALOG, "",
				wxDefaultPosition, wxDefaultSize,
				wxTE_MULTILINE|
				wxSUNKEN_BORDER|
				wxTE_READONLY);

	m_CommandWin = new WinEDA_CommandFrame(this, ID_MAIN_COMMAND,
				wxPoint(150, 0), wxSize(clientsize.x, 100) );
	m_CommandWin->SetDefaultSize(wxSize(clientsize.x, 100));
	m_CommandWin->SetOrientation(wxLAYOUT_HORIZONTAL);
	m_CommandWin->SetAlignment(wxLAYOUT_TOP);
	m_CommandWin->SetSashVisible(wxSASH_BOTTOM, TRUE);

	CreateCommandToolbar();

char line[1024];
	msg = wxGetCwd();
	sprintf(line,_("Ready\nWorking dir: %s\n"), msg.GetData());
	PrintMsg(line);
}

	/***************/
	/* Destructeur */
	/***************/

WinEDA_MainFrame::~WinEDA_MainFrame(void)
{
}


/*******************************************************/
void WinEDA_MainFrame::PrintMsg(const wxString & text)
/*******************************************************/
/*
imprime le message dans la fenetre des messages
*/
{
	m_DialogWin->AppendText(text);
}

/****************************************************/
void WinEDA_MainFrame::OnSashDrag(wxSashEvent& event)
/****************************************************/
/* Ajuste les dimensions des fenetres lors des drags des bordures
*/
{
int w, h;
int dy = 0;

	if (event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE)
        return;

	GetClientSize(&w, &h);

	switch (event.GetId())
		{
		case ID_LEFT_FRAME:
			{
			m_LeftWin->SetDefaultSize(wxSize(event.GetDragRect().width, -1));
            break;
			}

	  case ID_BOTTOM_FRAME:
			{
			dy = event.GetDragRect().height;
			m_BottomWin->SetDefaultSize(wxSize(-1, dy));
			m_CommandWin->SetDefaultSize(wxSize(-1, h - dy));
			break;
			}

	  case ID_MAIN_COMMAND:
			{
			dy = event.GetDragRect().height;
			m_CommandWin->SetDefaultSize(wxSize(-1, dy));
			m_BottomWin->SetDefaultSize(wxSize(-1, h - dy));
			break;
			}
		}
	wxLayoutAlgorithm layout;
    layout.LayoutFrame(this);

   // Leaves bits of itself behind sometimes
//	GetClientWindow()->Refresh();
}

/************************************************/
void WinEDA_MainFrame::OnSize(wxSizeEvent& event)
/************************************************/
{
	if (m_CommandWin && m_BottomWin)
		{
		int w, h, dy;
		wxSize bsize, hsize;
		GetClientSize(&w, &h);
		bsize = m_BottomWin->GetSize();
		hsize = m_CommandWin->GetSize();
		dy = h - hsize.y;
		if ( dy < 50 )
			{
			dy = 50;
			hsize.y = h - dy;
			}
		m_CommandWin->SetDefaultSize(wxSize(-1, hsize.y));
		m_BottomWin->SetDefaultSize(wxSize(-1, dy));
		};

	wxLayoutAlgorithm layout;
	layout.LayoutFrame(this);
}


/**********************************************************/
void WinEDA_MainFrame::OnCloseWindow(wxCloseEvent & Event)
/**********************************************************/
{
int px, py;

	SetLastProject(m_PrjFileName);

	if ( ! IsIconized() )
		{
		/* Memorisation position sur l'ecran */
		GetPosition(&px, &py);
		m_FramePos.x = px;
		m_FramePos.y = py;

		/* Memorisation dimension de la fenetre */
		GetSize(&px, &py);
		m_FrameSize.x = px;
		m_FrameSize.y = py;
		}

	Event.SetCanVeto(TRUE);

	SaveSettings();

	// Close the help frame
	if ( m_Parent->m_HtmlCtrl )
		{
		if ( m_Parent->m_HtmlCtrl->GetFrame() ) // returns NULL if no help frame active
			m_Parent->m_HtmlCtrl->GetFrame()->Close(TRUE);
		m_Parent->m_HtmlCtrl = NULL;
		}

	Destroy();
}


/**********************************************************/
void WinEDA_MainFrame::OnPaint(wxPaintEvent & event)
/**********************************************************/
{
	event.Skip();
}

/*******************************************/
void WinEDA_MainFrame::ReDraw(wxDC * DC)
/*******************************************/
{
}



/**********************************************************************/
void WinEDA_MainFrame::Process_Special_Functions(wxCommandEvent& event)
/**********************************************************************/
{
int id = event.GetId();

	switch ( id )
		{
		case ID_EXIT :
			Close(TRUE);
			break;

		default:
			DisplayError(this, "WinEDA_MainFrame::Process_Special_Functions error");
			break;
		}
}


/********************************************************/
void WinEDA_MainFrame::Process_Fct(wxCommandEvent& event)
/*********************************************************/
{
int id = event.GetId();
wxString FullFileName = m_PrjFileName;


	switch (id)
		{
		case ID_TO_PCB:
			ChangeFileNameExt(FullFileName, g_BoardExtBuffer);
			AddDelimiterString(FullFileName);
			ExecuteFile(this, PCBNEW_EXE, FullFileName);
			break;

		case ID_TO_CVPCB:
			ChangeFileNameExt(FullFileName, g_NetlistExtBuffer);
			AddDelimiterString(FullFileName);
			ExecuteFile(this, CVPCB_EXE, FullFileName);
			break;

		case ID_TO_EESCHEMA:
			ChangeFileNameExt(FullFileName, g_SchExtBuffer);
			AddDelimiterString(FullFileName);
			ExecuteFile(this, EESCHEMA_EXE, FullFileName);
			break;

		case ID_TO_GERBVIEW:
			FullFileName = wxGetCwd() + STRING_DIR_SEP;
			AddDelimiterString(FullFileName);
			ExecuteFile(this, GERBVIEW_EXE, FullFileName);
			break;

		case ID_TO_EDITOR:
		{
			wxString editorname = GetEditorName();
			if ( editorname != "" )
				ExecuteFile(this, editorname, "");
		}
			break;

		case ID_BROWSE_AN_SELECT_FILE:
			{
			FullFileName = EDA_FileSelector( _("Load file:"),
					".",		  	/* Chemin par defaut */
					"",				/* nom fichier par defaut */
					".*",			/* extension par defaut */
					"*.*",			/* Masque d'affichage */
					this,
					wxOPEN,
					TRUE
					);
			if ( FullFileName != "" )
				{
				wxString fullnamewithquotes;
				fullnamewithquotes = "\"" + FullFileName;
				fullnamewithquotes += "\"";
				wxString editorname = GetEditorName();
				if ( editorname != "" )
					ExecuteFile(this, editorname, fullnamewithquotes);
				}
			}
			break;

		case ID_SELECT_PREFERED_EDITOR:
		{
			wxString mask;
#ifdef __WINDOWS__
			mask = "*.exe";
#endif
			FullFileName = EDA_FileSelector( _("Prefered Editor:"),
					wxPathOnly(g_EditorName),	/* Default path */
					g_EditorName,			/* default filename */
					"",						/* default filename extension */
					mask,						/* filter for filename list */
					NULL,					/* parent frame */
					wxOPEN,					/* wxSAVE, wxOPEN ..*/
					TRUE					/* true = keep the current path */
					);
			if ( (FullFileName != "" ) && EDA_Appl->m_EDA_CommonConfig)
			{
				g_EditorName = FullFileName;
				EDA_Appl->m_EDA_CommonConfig->Write("Editor", g_EditorName);
			}
		}
			break;
		default: DisplayError(this, "WinEDA_MainFrame::Process_Fct Internal Error");
			break;
		}

}

