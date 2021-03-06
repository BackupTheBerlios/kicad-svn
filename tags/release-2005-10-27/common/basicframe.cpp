	/******************************************************************/
	/* basicframe.cpp - fonctions des classes du type WinEDA_BasicFrame */
	/******************************************************************/

#ifdef __GNUG__
#pragma implementation
#endif

#include "fctsys.h"
#include "common.h"



	/*******************************************************/
	/* Constructeur de WinEDA_BasicFrame: la fenetre generale */
	/*******************************************************/

WinEDA_BasicFrame::WinEDA_BasicFrame( wxWindow * father, int idtype,
						WinEDA_App *parent, const wxString & title,
						const wxPoint& pos, const wxSize& size):
		wxFrame(father, -1, title, pos, size,
			wxDEFAULT_FRAME_STYLE|wxWANTS_CHARS)
{
wxSize minsize;

	m_Ident = idtype;
	m_Parent = parent;
	SetFont(*g_StdFont);
	m_MenuBar = NULL;		// menu du haut d'ecran
	m_HToolBar = NULL;
	m_FrameIsActive = FALSE;
	m_MsgFrameHeight = MSG_PANEL_DEFAULT_HEIGHT;

	minsize.x = 470;
	minsize.y = 350 + m_MsgFrameHeight;
	SetSizeHints( minsize.x, minsize.y, -1,-1, -1,-1);

	/* Verification des parametres de creation */
	if ( (size.x < minsize.x) || (size.y < minsize.y) )
		SetSize(0,0, minsize.x, minsize.y);

	// Create child subwindows.
	GetClientSize(&m_FrameSize.x, &m_FrameSize.y);	/* dimx, dimy = dimensions utiles de la
								zone utilisateur de la fenetre principale */
	m_FramePos.x = m_FramePos.y = 0;
	m_FrameSize.y -= m_MsgFrameHeight;
}

/****************************************/
WinEDA_BasicFrame::~WinEDA_BasicFrame(void)
/****************************************/
{
}

/****************************************/
void WinEDA_BasicFrame::ReCreateMenuBar(void)
/****************************************/
// Virtual function
{
}


/*********************************************/
void WinEDA_BasicFrame::GetSettings(void)
/*********************************************/
{
wxString text;
int Ypos_min;

	if( m_Parent->m_EDA_Config )
	{
		text = m_FrameName + "Pos_x";
		m_Parent->m_EDA_Config->Read(text, &m_FramePos.x);
		text = m_FrameName + "Pos_y";
		m_Parent->m_EDA_Config->Read(text, &m_FramePos.y);
		text = m_FrameName + "Size_x";
		m_Parent->m_EDA_Config->Read(text, &m_FrameSize.x, 600);
		text = m_FrameName + "Size_y";
		m_Parent->m_EDA_Config->Read(text, &m_FrameSize.y, 400);
	}

	// Ensure Window title bar is visible
#ifdef __WXMAC__
	// for macOSX, the window must be below system (macOSX) toolbar
	Ypos_min = GetMBarHeight();
#else
	Ypos_min = 0;
#endif
	if ( m_FramePos.y < Ypos_min ) m_FramePos.y = Ypos_min;
}


/*****************************************/
void WinEDA_BasicFrame::SaveSettings(void)
/*****************************************/
{
wxString text;

	if ( !m_Parent || !m_Parent->m_EDA_Config ) return;

	if( ! m_Parent->m_EDA_Config || IsIconized() ) return;

	m_FrameSize = GetSize();
	m_FramePos = GetPosition();

	text = m_FrameName + "Pos_x";
	m_Parent->m_EDA_Config->Write(text, (long)m_FramePos.x);
	text = m_FrameName + "Pos_y";
	m_Parent->m_EDA_Config->Write(text, (long)m_FramePos.y);
	text = m_FrameName + "Size_x";
	m_Parent->m_EDA_Config->Write(text, (long)m_FrameSize.x);
	text = m_FrameName + "Size_y";
	m_Parent->m_EDA_Config->Write(text, (long)m_FrameSize.y);
}



/******************************************************/
void WinEDA_BasicFrame::PrintMsg(const wxString & text)
/******************************************************/
{
	SetStatusText(text);
}

/************************************************************************/
void WinEDA_BasicFrame::DisplayActivity(int PerCent, const wxString & Text)
/*************************************************************************/
/* Display a bargraph (0 to 50 point length) for a PerCent value from 0 to 100
*/
{
wxString Line;

	Line = Text;

	PerCent = (PerCent < 0) ? 0 : PerCent;
	PerCent = (PerCent > 100) ? 100 : PerCent;
	PerCent /= 2;	// Bargraph is 0 .. 50 points from 0% to 100%
	if (PerCent) Line.Pad(PerCent, '*');

	SetStatusText(Line);
}

/*******************************************************************/
void WinEDA_BasicFrame::SetLastProject(const wxString & FullFileName)
/*******************************************************************/
/* Met a jour la liste des anciens projets
*/
{
unsigned ii;

	if ( FullFileName == "" ) return;

	//suppression d'une ancienne trace eventuelle du meme fichier
	for ( ii = 0; ii < m_Parent->m_LastProject.GetCount(); )
	{
		if(m_Parent->m_LastProject[ii] == "") break;
#ifdef __WINDOWS__
		if (stricmp( m_Parent->m_LastProject[ii].GetData(),FullFileName ) == 0 )
#else
		if ( m_Parent->m_LastProject[ii] == FullFileName )
#endif
		{
#if ( (wxMAJOR_VERSION < 2) || ((wxMAJOR_VERSION == 2)&& (wxMINOR_VERSION <= 4)) )
			m_Parent->m_LastProject.Remove(ii);
#else
			m_Parent->m_LastProject.RemoveAt(ii);
#endif
		}
		else ii++;
	}

    while (m_Parent->m_LastProject.GetCount() >= m_Parent->m_LastProjectMaxCount)
    {
#if ( (wxMAJOR_VERSION < 2) || ((wxMAJOR_VERSION == 2)&& (wxMINOR_VERSION <= 4)) )
        files.Remove(files.GetCount() - 1);
#else
        m_Parent->m_LastProject.RemoveAt(m_Parent->m_LastProject.GetCount() - 1);
#endif
    }
	m_Parent->m_LastProject.Insert(FullFileName, 0);

	ReCreateMenuBar();
}

/**************************************************/
wxString WinEDA_BasicFrame::GetLastProject(int rang)
/**************************************************/
{
	if ( rang < 0 ) rang = 0;
	if ( (unsigned) rang >= m_Parent->m_LastProject.GetCount() )
		return wxString("");
	return m_Parent->m_LastProject[rang];
}


/**************************************************************/
void WinEDA_BasicFrame::GetKicadHelp(wxCommandEvent& event)
/**************************************************************/
{
	if ( m_Parent->m_HtmlCtrl == NULL )
		{
		m_Parent->InitOnLineHelp();
		}


	if ( m_Parent->m_HtmlCtrl )
		{
		m_Parent->m_HtmlCtrl->DisplayContents();
		m_Parent->m_HtmlCtrl->Display(m_Parent->m_HelpFileName);
		}
	else DisplayError(this, _("Help files not found"));
}

/***********************************************************/
void WinEDA_BasicFrame::GetKicadAbout(wxCommandEvent& event)
/**********************************************************/
{
	Affiche_InfosLicence(this);
}


