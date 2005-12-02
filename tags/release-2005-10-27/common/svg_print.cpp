/***********************/
/* File: svg_print.cpp */
/***********************/


// Print drawings in SVG (Scalable Vector Graphics) format

#ifdef __GNUG__
#pragma implementation
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/spinctrl.h"

#include <ctype.h>
#include "wx/metafile.h"
#include "wx/dcsvg.h"
#include "wx/image.h"

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"

extern BASE_SCREEN * ActiveScreen;
extern int PenMinWidth;	/* dim mini (en 1/100 mmm) pour les traits imprimes */

#define WIDTH_MAX_VALUE 100
#define WIDTH_MIN_VALUE 10

// Variables locales
static int Select_PrintAll = FALSE;
static bool Print_Sheet_Ref = TRUE;

enum print_id {
	ID_PRINT_SETUP = 1100,
	ID_PRINT_EXECUTE,
	ID_PRINT_SELECT_LAYERS,
	ID_PRINT_CLOSE,
	ID_PEN_WIDTH,
	ID_SET_ALLPAGES,
	ID_PRINT_REF
};

	/****************************************************************/
	/* frame de Preparation de l'impression (options, selections... */
	/****************************************************************/

class WinEDA_PrintSVGFrame: public wxDialog
{
public:
	WinEDA_DrawFrame * m_Parent;
	int m_PrintMaskLayer;
	wxString m_Buff_Width;
	wxSpinCtrl * m_ButtPenWidth;
	wxRadioBox * m_PagesOption;
	wxCheckBox * m_Print_Sheet_Ref;
	WinEDA_EnterText * m_FileNameCtrl;
	int m_ImageXSize_mm;

	// Constructor and destructor
	WinEDA_PrintSVGFrame(WinEDA_DrawFrame * parent, wxPoint& pos);
	~WinEDA_PrintSVGFrame(void) {};

	void PrintSVGDoc(wxCommandEvent& event);
	void DrawPage(const wxString & FullFileName);
	void SetPenWidth(wxSpinEvent& event);
	void OnClose(wxCommandEvent& event);
	wxString ReturnFullFileName(void);
	DECLARE_EVENT_TABLE()

};

/* Construction de la table des evenements */
BEGIN_EVENT_TABLE(WinEDA_PrintSVGFrame, wxDialog)
	EVT_BUTTON(ID_PRINT_EXECUTE, WinEDA_PrintSVGFrame::PrintSVGDoc)
	EVT_BUTTON(ID_PRINT_CLOSE, WinEDA_PrintSVGFrame::OnClose)
	EVT_SPINCTRL(ID_PEN_WIDTH, WinEDA_PrintSVGFrame::SetPenWidth)
END_EVENT_TABLE()




/*******************************************************/
void WinEDA_DrawFrame::SVG_Print(wxCommandEvent& event)
/*******************************************************/
/* Prepare les structures de données de gestion de l'impression
et affiche la fenetre de dialogue de gestion de l'impression des feuilles
*/
{
wxPoint pos = GetPosition();

	// Arret des commandes en cours
	if( m_CurrentScreen->ManageCurseur && m_CurrentScreen->ForceCloseManageCurseur )
	{
		wxClientDC dc(DrawPanel);
		DrawPanel->PrepareDC(dc);
		m_CurrentScreen->ForceCloseManageCurseur(this, &dc);
	}
	SetToolID(0, wxCURSOR_ARROW,"");

	pos.x += 10; pos.y += 10;
WinEDA_PrintSVGFrame frame(this, pos);
	frame.ShowModal();
}


/*************************************************/
/* Classe de la fenetre de gestion de l'impression */
/*************************************************/
#define Y_SIZE 280
#define X_SIZE 360
WinEDA_PrintSVGFrame::WinEDA_PrintSVGFrame(WinEDA_DrawFrame *parent, wxPoint& framepos):
		wxDialog(parent, -1, _("Print SVG file"), framepos,
                wxSize(X_SIZE, Y_SIZE),
		wxDEFAULT_DIALOG_STYLE|wxFRAME_FLOAT_ON_PARENT )
{
wxPoint pos;
wxSize size;
wxButton * Button;
	
	m_Parent = parent;
	SetFont(*g_DialogFont);

	m_PagesOption = NULL;
	m_Print_Sheet_Ref = NULL;
	m_ImageXSize_mm = 270;

	pos.x = 10; pos.y = 5;
	size.x = 100; size.y = -1;
	new wxStaticText(this,-1, _("Pen width mini"), pos);
	pos.y += 15;
	m_Buff_Width << PenMinWidth;
	m_ButtPenWidth = new wxSpinCtrl(this, ID_PEN_WIDTH, m_Buff_Width, pos);
	m_ButtPenWidth->SetRange(WIDTH_MIN_VALUE, WIDTH_MAX_VALUE);

	pos.y += 30;

	if ( m_Parent->m_Print_Sheet_Ref )
	m_Print_Sheet_Ref = new wxCheckBox(this, ID_PRINT_REF,
                        _("Print Sheet Ref"), pos);
	m_Print_Sheet_Ref->SetValue(Print_Sheet_Ref);


wxString page_all[] =
	{_("Current"), _("All") };

	pos.y += 35;
	m_PagesOption = new wxRadioBox(this, ID_SET_ALLPAGES,
						_("Page Print:"),
						pos,wxSize(-1,-1),
						2,page_all,1,wxRA_SPECIFY_COLS);
	if ( Select_PrintAll ) m_PagesOption->SetSelection(1);

	pos.y += 105;
wxString FullFileName = ReturnFullFileName();
	m_FileNameCtrl = new WinEDA_EnterText(this, _("Filename:"),
			FullFileName, pos, wxSize(X_SIZE - 20, -1) );

	/* Creation des boutons de commande */
	pos.x += 175; pos.y = 5;
	Button = new wxButton(this,	ID_PRINT_EXECUTE,
						_("&Print"), pos);
	Button->SetForegroundColour(wxColour(0,100,0) );

	pos.y += Button->GetSize().y + 15;
	Button = new wxButton(this,	ID_PRINT_CLOSE,
						_("&Close"), pos);
	Button->SetForegroundColour(wxColour(0,0,150) );
}


/********************************************************/
void WinEDA_PrintSVGFrame::OnClose(wxCommandEvent& event)
/********************************************************/
{
	Close(TRUE);
}


/******************************************************/
wxString WinEDA_PrintSVGFrame::ReturnFullFileName(void)
/******************************************************/
{
wxString name, ext;

	name = m_Parent->m_CurrentScreen->m_FileName;
	ChangeFileNameExt(name, ".svg");
	return name;
}


/*********************************************************/
void WinEDA_PrintSVGFrame::SetPenWidth(wxSpinEvent& event)
/*********************************************************/
{
	PenMinWidth = m_ButtPenWidth->GetValue();
	if ( PenMinWidth > WIDTH_MAX_VALUE )
		{
		PenMinWidth = WIDTH_MAX_VALUE;
		wxBell();
		}
	if ( PenMinWidth < WIDTH_MIN_VALUE )
		{
		PenMinWidth = WIDTH_MIN_VALUE;
		wxBell();
		}
	m_Buff_Width.Printf("%d", PenMinWidth);
	m_ButtPenWidth->SetValue(m_Buff_Width);
}



/**************************************************************/
void WinEDA_PrintSVGFrame::PrintSVGDoc(wxCommandEvent& event)
/**************************************************************/
/* Called on activate "Print CURRENT" button
*/
{
bool print_ref = TRUE;

	Select_PrintAll = FALSE;
	if ( m_PagesOption && (m_PagesOption->GetSelection() == 1) )
		Select_PrintAll = TRUE;

	if ( (m_Print_Sheet_Ref == NULL) || (m_Print_Sheet_Ref->GetValue() == FALSE) )
		print_ref = FALSE;

wxSpinEvent spinevent;
	SetPenWidth(spinevent);


BASE_SCREEN * screen = m_Parent->m_CurrentScreen;
BASE_SCREEN *oldscreen = screen;

	if( Select_PrintAll )
		while ( screen->Pback ) screen = (BASE_SCREEN *) screen->Pback;

	if ( (m_Parent->m_Ident == PCB_FRAME) || (m_Parent->m_Ident == GERBER_FRAME) )
	{
		if( Select_PrintAll )
		{
			m_PrintMaskLayer = 0xFFFFFFFF;
		}
		else  m_PrintMaskLayer = 1;
	}

	if (screen == NULL) return;
wxString FullFileName = m_FileNameCtrl->GetData();
	ActiveScreen = screen;
	DrawPage(FullFileName);
	ActiveScreen = oldscreen;

	Close(TRUE);
}

/*****************************************************************/
void WinEDA_PrintSVGFrame::DrawPage(const wxString & FullFileName)
/*****************************************************************/
/*
	Routine effective d'impression
*/
{
int tmpzoom;
wxPoint tmp_startvisu;
wxSize PageSize_in_mm;
wxSize SheetSize;		// Sheet size in internal units
wxSize PlotAreaSize;	// Taille de la surface utile de trace (pixels)
wxPoint old_org;
wxPoint DrawOffset;	// Offset de trace
float dpi;
	
	wxBusyCursor dummy;

	/* modification des cadrages et reglages locaux */
	tmp_startvisu = ActiveScreen->m_StartVisu;
	tmpzoom = ActiveScreen->GetZoom();
	old_org = ActiveScreen->m_DrawOrg;
	ActiveScreen->m_DrawOrg.x = ActiveScreen->m_DrawOrg.y = 0;
	ActiveScreen->m_StartVisu.x = ActiveScreen->m_StartVisu.y = 0;
	SheetSize = ActiveScreen->m_CurrentSheet->m_Size;	// size in 1/1000 inch
	SheetSize.x *= m_Parent->m_InternalUnits / 1000;
	SheetSize.y *= m_Parent->m_InternalUnits / 1000;	// size in pixels

	ActiveScreen->SetZoom(1);
	dpi = (float)SheetSize.x * 25.4  /m_ImageXSize_mm;

wxSVGFileDC dc(FullFileName, SheetSize.x, SheetSize.y, dpi) ;

	GRResetPenAndBrush(&dc);
	SetPenMinWidth(1);
	GRForceBlackPen(FALSE);

WinEDA_DrawPanel * panel = m_Parent->DrawPanel;
EDA_Rect tmp = panel->m_ClipBox;

	panel->m_ClipBox.SetX(0);panel->m_ClipBox.SetY(0);
	panel->m_ClipBox.SetWidth(0x7FFFFF0); panel->m_ClipBox.SetY(0x7FFFFF0);

	g_IsPrinting = TRUE;
	panel->PrintPage(&dc, m_Print_Sheet_Ref, m_PrintMaskLayer);
	g_IsPrinting = FALSE;

	panel->m_ClipBox = tmp;

	SetPenMinWidth(1);

	ActiveScreen->m_StartVisu = tmp_startvisu;
	ActiveScreen->m_DrawOrg = old_org;
	ActiveScreen->SetZoom(tmpzoom);
}

