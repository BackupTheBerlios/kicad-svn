/*********************************************************************/
/** setvisu() : initialisations de l'ecran d'affichage du composant **/
/*********************************************************************/

#include "fctsys.h"
#include "common.h"

#include "wxstruct.h"
#include "cvpcb.h"
#include "id.h"
#include "bitmaps.h"

#include "protos.h"

#define BITMAP wxBitmap

/*****************************************************************/
/* Construction de la table des evenements pour WinEDA_DrawFrame */
/*****************************************************************/

BEGIN_EVENT_TABLE(WinEDA_DisplayFrame, wxFrame)

	COMMON_EVENTS_DRAWFRAME

	EVT_CLOSE(WinEDA_DisplayFrame::OnCloseWindow)
	EVT_SIZE(WinEDA_DrawFrame::OnSize)
	EVT_TOOL_RANGE(ID_ZOOM_PLUS_BUTT, ID_ZOOM_PAGE_BUTT,
			WinEDA_DisplayFrame::Process_Zoom)
	EVT_TOOL(ID_OPTIONS_SETUP, WinEDA_DisplayFrame::InstallOptionsDisplay)
	EVT_TOOL(ID_CVPCB_SHOW3D_FRAME, WinEDA_BasePcbFrame::Show3D_Frame)
END_EVENT_TABLE()




	/*********************************************************************/
	/* Constructeur de WinEDA_DisplayFrame: la fenetre de visu du composant */
	/*********************************************************************/

WinEDA_DisplayFrame::WinEDA_DisplayFrame( wxWindow * father, WinEDA_App *parent,
			const wxString & title,
			const wxPoint& pos, const wxSize& size):
			WinEDA_BasePcbFrame(father, parent, CVPCB_DISPLAY_FRAME, title, pos, size)
{
	m_FrameName = wxT("CmpFrame");
	m_Draw_Axes = TRUE;			// TRUE pour avoir les axes dessines
	m_Draw_Grid = TRUE;				// TRUE pour avoir la axes dessinee
	m_Draw_Sheet_Ref = FALSE;		// TRUE pour avoir le cartouche dessiné

	SetIcon(wxIcon( wxT("icon_cvpcb")));	// Give an icon
	SetTitle(title);

	m_Pcb = new BOARD(NULL, this);
	m_CurrentScreen = new PCB_SCREEN(NULL, this, CVPCB_DISPLAY_FRAME);
	GetSettings();
	SetSize(m_FramePos.x, m_FramePos.y, m_FrameSize.x, m_FrameSize.y);
	ReCreateHToolbar();
	ReCreateVToolbar();
	Show(TRUE);
}


	/*****************************************/
	/* Fonctions de base de WinEDA_DisplayFrame */
	/*****************************************/
// Destructeur
WinEDA_DisplayFrame::~WinEDA_DisplayFrame(void)
{
	if ( m_Pcb->m_Modules ) DeleteStructure( m_Pcb->m_Modules );
	m_Pcb->m_Modules = NULL;
	delete m_CurrentScreen;
	delete m_Pcb;
	m_Parent->m_CvpcbFrame->DrawFrame = NULL;
}


/************************************************************/
void WinEDA_DisplayFrame::OnCloseWindow(wxCloseEvent& event)
/************************************************************/
/* Fermeture par icone ou systeme
*/
{
wxPoint pos;
wxSize size;

	size = GetSize();
	pos = GetPosition();

	SaveSettings();
	Destroy();
}



/************************************************/
void WinEDA_DisplayFrame::ReCreateVToolbar(void)
/************************************************/
{
}


/************************************************/
void WinEDA_DisplayFrame::ReCreateHToolbar(void)
/************************************************/
{
	if ( m_HToolBar != NULL ) return;

	m_HToolBar = new WinEDA_Toolbar(TOOLBAR_MAIN, this, ID_H_TOOLBAR, TRUE);
	SetToolBar(m_HToolBar);

	m_HToolBar->AddTool(ID_OPTIONS_SETUP, wxEmptyString,
					BITMAP(display_options_xpm),
					_("Display Options"));

	m_HToolBar->AddSeparator();

	m_HToolBar->AddTool(ID_ZOOM_PLUS_BUTT, wxEmptyString, BITMAP(zoom_in_xpm),
					_("zoom + (F1)"));

	m_HToolBar->AddTool(ID_ZOOM_MOINS_BUTT, wxEmptyString, BITMAP(zoom_out_xpm),
					_("zoom - (F2)"));

	m_HToolBar->AddTool(ID_ZOOM_REDRAW_BUTT, wxEmptyString, BITMAP(repaint_xpm),
					_("redraw (F3)"));

	m_HToolBar->AddTool(ID_ZOOM_PAGE_BUTT, wxEmptyString, BITMAP(zoom_optimal_xpm),
					_("1:1 zoom"));

	m_HToolBar->AddSeparator();
	m_HToolBar->AddTool(ID_CVPCB_SHOW3D_FRAME, wxEmptyString, BITMAP(show_3d_xpm),
					_("1:1 zoom"));

	// after adding the buttons to the toolbar, must call Realize() to reflect
	// the changes
	m_HToolBar->Realize();
}


/*******************************************/
void WinEDA_DisplayFrame::SetToolbars(void)
/*******************************************/
{
}


void WinEDA_DisplayFrame::OnLeftClick(wxDC * DC, const wxPoint& MousePos)
{
}

void WinEDA_DisplayFrame::OnLeftDClick(wxDC * DC, const wxPoint& MousePos)
{
}

void WinEDA_DisplayFrame::OnRightClick(const wxPoint& MousePos, wxMenu * PopMenu)
{
}


/****************************************************************/
void WinEDA_BasePcbFrame::GeneralControle(wxDC *DC, wxPoint Mouse)
/****************************************************************/
{
wxSize delta;
int flagcurseur = 0;
int zoom = m_CurrentScreen->GetZoom();
wxPoint curpos, oldpos;

	curpos = DrawPanel->CursorRealPosition(Mouse);
	oldpos = m_CurrentScreen->m_Curseur;

	delta.x = m_CurrentScreen->GetGrid().x / zoom;
	delta.y = m_CurrentScreen->GetGrid().y / zoom;
	if( delta.x <= 0 ) delta.x = 1;
	if( delta.y <= 0 ) delta.y = 1;

	if( g_KeyPressed)
		{
		switch( g_KeyPressed )
			{
			case WXK_F1 :
					OnZoom(ID_ZOOM_PLUS_KEY);
					flagcurseur = 2;
					curpos = m_CurrentScreen->m_Curseur;
					break;
			case WXK_F2 :
					OnZoom(ID_ZOOM_MOINS_KEY);
					flagcurseur = 2;
					curpos = m_CurrentScreen->m_Curseur;
					break;
			case WXK_F3 :
					OnZoom(ID_ZOOM_REDRAW_KEY);
					flagcurseur = 2;
					break;
			case WXK_F4 :
					OnZoom(ID_ZOOM_CENTER_KEY);
					flagcurseur = 2;
					curpos = m_CurrentScreen->m_Curseur;
					break;

			case ' ':
				m_CurrentScreen->m_O_Curseur = m_CurrentScreen->m_Curseur;
				break;

			case WXK_NUMPAD8:	/* Deplacement curseur vers le haut */
			case WXK_UP	:
				DrawPanel->CalcScrolledPosition(Mouse.x, Mouse.y - delta.y,
					&Mouse.x, &Mouse.y);
				GRMouseWarp(DrawPanel, Mouse);
				break ;

			case WXK_NUMPAD2:	/* Deplacement curseur vers le bas */
			case WXK_DOWN:
				DrawPanel->CalcScrolledPosition(Mouse.x, Mouse.y + delta.y,
					&Mouse.x, &Mouse.y);
				GRMouseWarp(DrawPanel, Mouse);
				break ;

			case WXK_NUMPAD4:	/* Deplacement curseur vers la gauche */
			case WXK_LEFT :
				DrawPanel->CalcScrolledPosition(Mouse.x - delta.x, Mouse.y,
					&Mouse.x, &Mouse.y);
				GRMouseWarp(DrawPanel, Mouse);
				break ;
	
			case WXK_NUMPAD6:  /* Deplacement curseur vers la droite */
			case WXK_RIGHT :
				DrawPanel->CalcScrolledPosition(Mouse.x + delta.x, Mouse.y,
					&Mouse.x, &Mouse.y);
				GRMouseWarp(DrawPanel, Mouse);
				break ;
			}
		}

	/* Recalcul de la position du curseur schema */
	m_CurrentScreen->m_Curseur = curpos;
	/* Placement sur la grille generale */
	PutOnGrid( & m_CurrentScreen->m_Curseur);

	if( m_CurrentScreen->IsRefreshReq() )
		{
		flagcurseur = 2;
		RedrawActiveWindow(DC, TRUE);
		}

	if ( (oldpos.x != m_CurrentScreen->m_Curseur.x) ||
		 (oldpos.y != m_CurrentScreen->m_Curseur.y) )
		{
		if ( flagcurseur != 2 )
			{
			curpos = m_CurrentScreen->m_Curseur;
			m_CurrentScreen->m_Curseur = oldpos;
			m_CurrentScreen->CursorOff(DrawPanel, DC);

			m_CurrentScreen->m_Curseur = curpos;
			m_CurrentScreen->CursorOn(DrawPanel, DC);
			}

		if(m_CurrentScreen->ManageCurseur)
			{
			m_CurrentScreen->ManageCurseur(DrawPanel, DC, 0);
			}
		}

	Affiche_Status_Box();	 /* Affichage des coord curseur */
}

/*************************************************************************/
void WinEDA_DisplayFrame::Process_Special_Functions(wxCommandEvent& event)
/*************************************************************************/
/* Traite les selections d'outils et les commandes appelees du menu POPUP
*/
{
int id = event.GetId();
wxPoint pos;
wxClientDC dc(DrawPanel);

	DrawPanel->PrepareGraphicContext(&dc);
	wxGetMousePosition(&pos.x, &pos.y);

	switch ( id )	// Arret eventuel de la commande de déplacement en cours
		{
		default:
			wxMessageBox( wxT("WinEDA_DisplayFrame::Process_Special_Functions error"));
			break;
		}

	SetToolbars();
}

