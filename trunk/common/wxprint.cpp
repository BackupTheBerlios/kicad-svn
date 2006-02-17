/*********************/
/* File: wxprint.cpp */
/*********************/

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


#if !wxUSE_PRINTING_ARCHITECTURE
#error You must set wxUSE_PRINTING_ARCHITECTURE to 1 in setup.h to compile this
#endif

// Set this to 1 if you want to test PostScript printing under MSW.
#define wxTEST_POSTSCRIPT_IN_MSW 1

#include <ctype.h>
#include "wx/metafile.h"
#include "wx/print.h"
#include "wx/printdlg.h"

#if wxTEST_POSTSCRIPT_IN_MSW
#include "wx/generic/printps.h"
#include "wx/generic/prntdlgg.h"
#endif

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"

#ifdef EESCHEMA
#include "program.h"
#include "libcmp.h"
#include "general.h"
#endif

#ifdef PCBNEW
#include "pcbnew.h"
extern float Scale_X, Scale_Y;
static long s_SelectedLayers = CUIVRE_LAYER | CMP_LAYER |
					SILKSCREEN_LAYER_CMP | SILKSCREEN_LAYER_CU;
#endif

#include "protos.h"

//#define DEFAULT_ORIENTATION_PAPER wxPORTRAIT
#define DEFAULT_ORIENTATION_PAPER wxLANDSCAPE
#define WIDTH_MAX_VALUE 100
#define WIDTH_MIN_VALUE 10

// static print data and page setup data, to remember settings during the session
static wxPrintData * g_PrintData;

/* exportees dans eeconfig.h */
int PenMinWidth = 20;	/* dim mini (en 1/100 mmm) pour les traits imprimes */

// Variables locales
static int s_PrintMaskLayer;
static int s_OptionPrintPage = 0;
static int s_Print_Black_and_White = TRUE;
#ifdef PCBNEW
#define SCALE_LIST_LEN 9
static int s_Scale_Select = 3;	// default selected scale = ScaleList[3] = 1
static double s_ScaleList[SCALE_LIST_LEN] = { 0, 0.5, 0.7, 0.999, 1.0, 1.4, 2.0, 3.0, 4.0 };
#endif

static bool s_Print_Sheet_Ref = TRUE;

enum print_id {
	ID_PRINT_SETUP = 1100,
	ID_PRINT_PREVIEW,
	ID_PRINT_EXECUTE,
	ID_PRINT_SELECT_LAYERS,
	ID_PRINT_QUIT,
	ID_PEN_WIDTH,
	ID_SET_PRINT_SCALE,
	ID_SET_BW,
	ID_PRINT_REF
};

	/****************************************************************/
	/* frame de Preparation de l'impression (options, selections... */
	/****************************************************************/

class WinEDA_PrintFrame: public wxDialog
{
public:
	WinEDA_DrawFrame * m_Parent;
	wxString m_Buff_Width;
#ifdef PCBNEW
	wxCheckBox * m_BoxSelecLayer[32];
#endif
	wxSpinCtrl * m_ButtPenWidth;
	wxRadioBox * m_ColorOption;
	wxRadioBox * m_PagesOption;
	wxRadioBox * m_ScaleOption;
	wxCheckBox * m_Print_Sheet_Ref;
	WinEDA_DFloatValueCtrl * m_FineAdjustXscaleOpt, * m_FineAdjustYscaleOpt;
	double m_XScaleAdjust, m_YScaleAdjust;

	// Constructor and destructor
	WinEDA_PrintFrame(WinEDA_DrawFrame * parent, wxPoint& pos);
	~WinEDA_PrintFrame(void) {};

	void OnPrintSetup(wxCommandEvent& event);
	void OnPrintPreview(wxCommandEvent& event);
	void EDA_PrintPage(wxCommandEvent& event);
	void SetPenWidth(wxSpinEvent& event);
	void SetColorOrBlack(wxCommandEvent& event);
	void SetScale(wxCommandEvent& event);
	int SetLayerMaskFromListSelection(void);
	void OnClose(wxCloseEvent & event);
	void OnQuit(wxCommandEvent& event);
	wxString BuildPrintTitle(void);

	DECLARE_EVENT_TABLE()

};

/* Construction de la table des evenements */
BEGIN_EVENT_TABLE(WinEDA_PrintFrame, wxDialog)
	EVT_CLOSE(WinEDA_PrintFrame::OnClose)
	EVT_BUTTON(ID_PRINT_SETUP, WinEDA_PrintFrame::OnPrintSetup)
	EVT_BUTTON(ID_PRINT_PREVIEW, WinEDA_PrintFrame::OnPrintPreview)
	EVT_BUTTON(ID_PRINT_EXECUTE, WinEDA_PrintFrame::EDA_PrintPage)
	EVT_BUTTON(ID_PRINT_QUIT, WinEDA_PrintFrame::OnQuit)
	EVT_SPINCTRL(ID_PEN_WIDTH, WinEDA_PrintFrame::SetPenWidth)
	EVT_RADIOBOX(ID_SET_BW, WinEDA_PrintFrame::SetColorOrBlack)
	EVT_RADIOBOX(ID_SET_PRINT_SCALE, WinEDA_PrintFrame::SetScale)
END_EVENT_TABLE()



	/***************************/
	/* Gestion de l'impression */
	/***************************/

class EDA_Printout: public wxPrintout
{
public:
	bool m_Print_Sheet_Ref;

public:
	WinEDA_DrawFrame * m_Parent;
	WinEDA_PrintFrame * m_PrintFrame;

	EDA_Printout(WinEDA_PrintFrame * print_frame, WinEDA_DrawFrame * parent, const wxString & title, bool print_ref):
		wxPrintout(title)
	{
		m_PrintFrame = print_frame;
		m_Parent = parent;
		s_PrintMaskLayer = 0xFFFFFFFF;
		m_Print_Sheet_Ref = print_ref;
	}

	bool OnPrintPage(int page);
	bool HasPage(int page);
	bool OnBeginDocument(int startPage, int endPage);
	void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);

	void DrawPage(void);
};

/*******************************************************/
void WinEDA_DrawFrame::ToPrinter(wxCommandEvent& event)
/*******************************************************/
/* Prepare les structures de données de gestion de l'impression
et affiche la fenetre de dialogue de gestion de l'impression des feuilles
*/
{
wxPoint pos = GetPosition();
bool PrinterError = FALSE;

	// Arret des commandes en cours
	if( m_CurrentScreen->ManageCurseur && m_CurrentScreen->ForceCloseManageCurseur )
		{
		wxClientDC dc(DrawPanel);
		DrawPanel->PrepareDC(dc);
		m_CurrentScreen->ForceCloseManageCurseur(this, &dc);
		}
	SetToolID(0, wxCURSOR_ARROW,wxEmptyString);

	if ( g_PrintData == NULL )	// Premier appel a l'impression
    	{
        g_PrintData = new wxPrintData();
        if ( ! g_PrintData->Ok() )
        	{
            PrinterError = TRUE;
        	DisplayError( this, _("Error Init Printer info") );
            }
		g_PrintData->SetQuality(wxPRINT_QUALITY_HIGH);		// Default resolution = HIGHT;
		g_PrintData->SetOrientation(DEFAULT_ORIENTATION_PAPER);
        }


	pos.x += 10; pos.y += 10;
	WinEDA_PrintFrame * frame = new WinEDA_PrintFrame(this, pos);
	frame->ShowModal(); frame->Destroy();
#ifdef EESCHEMA
	g_PrintFillMask = 0;
#endif
}


/*************************************************/
/* Classe de la fenetre de gestion de l'impression */
/*************************************************/
#ifdef PCBNEW
#define Y_SIZE 350
#define X_SIZE 520
#else
#define Y_SIZE 280
#define X_SIZE 360
#endif
WinEDA_PrintFrame::WinEDA_PrintFrame(WinEDA_DrawFrame *parent, wxPoint& framepos):
		wxDialog(parent, -1, _("Print"), framepos,
                wxSize(X_SIZE, Y_SIZE),
		wxDEFAULT_DIALOG_STYLE|wxFRAME_FLOAT_ON_PARENT )
{
wxPoint pos;
wxSize size, win_size;

	m_Parent = parent;
	SetFont(*g_DialogFont);

	m_PagesOption = NULL;
	m_Print_Sheet_Ref = NULL;

	pos.x = 5; pos.y = 5;

	m_XScaleAdjust = m_YScaleAdjust = 1.0;
	m_FineAdjustXscaleOpt = m_FineAdjustYscaleOpt = NULL;

#ifdef PCBNEW
	/* Create layer list */
	int mask = 1, ii, jj = pos.y;
	for ( ii = 0; ii < NB_LAYERS; ii++, mask <<= 1 )
	{
		if ( ii == 16 )
		{
			pos.x = win_size.x + 10; jj = pos.y; pos.y = 5;
		}
		m_BoxSelecLayer[ii] = new wxCheckBox(this, -1,
					ReturnPcbLayerName(ii), pos);
		if ( mask & s_SelectedLayers) m_BoxSelecLayer[ii]->SetValue(TRUE);
		if ( win_size.x < (pos.x + m_BoxSelecLayer[ii]->GetSize().x) )
			win_size.x = pos.x + m_BoxSelecLayer[ii]->GetSize().x;
		if ( win_size.y < (pos.y + m_BoxSelecLayer[ii]->GetSize().y) )
			win_size.y = pos.y + m_BoxSelecLayer[ii]->GetSize().y;
		pos.y += 16;
	}
	win_size.x += 10; pos.y = 5;
	pos.x = win_size.x;
	// Read the scale adjust option
	if ( m_Parent->m_Parent->m_EDA_Config )
	{
		m_Parent->m_Parent->m_EDA_Config->Read( wxT("PrintXFineScaleAdj"), &m_XScaleAdjust);
		m_Parent->m_Parent->m_EDA_Config->Read( wxT("PrintYFineScaleAdj"), &m_YScaleAdjust);
		m_Parent->m_Parent->m_EDA_Config->Read( wxT("PrintScale"), &s_Scale_Select);
	}

wxString msg_scale_list[SCALE_LIST_LEN] = {
	_("fit in page"), _("Scale 0.5"), _("Scale 0.7"), _("Approx. Scale 1"),
	_("Accurate Scale 1"),
	_("Scale 1.4"), _("Scale 2"), _("Scale 3"), _("Scale 4")
	};

	Scale_X = Scale_Y = s_ScaleList[s_Scale_Select];
	m_ScaleOption = new wxRadioBox(this, ID_SET_PRINT_SCALE,
						_("Approx. Scale:"),
						pos,wxSize(-1,-1),
						SCALE_LIST_LEN,msg_scale_list,1,wxRA_SPECIFY_COLS);
	m_ScaleOption->SetSelection(s_Scale_Select);

	// Create scale adjust option
	m_ScaleOption->GetSize(&size.x, &size.y);
	pos.y += size.y + 15;
	m_FineAdjustXscaleOpt = new WinEDA_DFloatValueCtrl(this, _("X Scale Adjust"), m_XScaleAdjust, pos);
	m_FineAdjustXscaleOpt->SetToolTip(_("Set X scale adjust for exact scale plotting"));
	pos.y += m_FineAdjustXscaleOpt->GetDimension().y + 5;
	m_FineAdjustYscaleOpt = new WinEDA_DFloatValueCtrl(this, _("Y Scale Adjust"), m_YScaleAdjust, pos);
	m_FineAdjustYscaleOpt->SetToolTip(_("Set Y scale adjust for exact scale plotting"));

	pos.y += m_FineAdjustXscaleOpt->GetDimension().y + 10;
	if ( win_size.y < pos.y ) win_size.y = pos.y;

	pos.x += 170;
#endif

	pos.y = 5; size.x = 100; size.y = -1;
	new wxStaticText(this,-1, _("Pen width mini"), pos);
	pos.y += 15;
	m_Buff_Width << PenMinWidth;
	m_ButtPenWidth = new wxSpinCtrl(this, ID_PEN_WIDTH, m_Buff_Width, pos);
	m_ButtPenWidth->SetRange(WIDTH_MIN_VALUE,WIDTH_MAX_VALUE);

	pos.y += 30;

	if ( m_Parent->m_Print_Sheet_Ref )
	m_Print_Sheet_Ref = new wxCheckBox(this, ID_PRINT_REF,
                        _("Print Sheet Ref"),
			pos);
	m_Print_Sheet_Ref->SetValue(s_Print_Sheet_Ref);

wxString color_bw[] =
	{_("Color"), _("Black") };

	pos.y += 25;
	m_ColorOption = new wxRadioBox(this, ID_SET_BW,
						_("Color Print:"),
						pos,wxSize(-1,-1),
						2,color_bw,1,wxRA_SPECIFY_COLS);
	if ( s_Print_Black_and_White ) m_ColorOption->SetSelection(1);

	#define OPT_COUNT 2
wxString page_all[OPT_COUNT] =
	{
#ifdef PCBNEW
		_("1 page per layer"), _("Single Page")
#else
		_("Current"), _("All")
#endif
		};

	pos.y += 85;
	m_PagesOption = new wxRadioBox(this, ID_SET_BW,
						_("Page Print:"),
						pos,wxSize(-1,-1),
						OPT_COUNT,page_all,1,wxRA_SPECIFY_COLS);
	m_PagesOption->SetSelection(s_OptionPrintPage);

	pos.y += m_PagesOption->GetSize().y;
	if ( win_size.y < (pos.y+10) ) win_size.y = pos.y + 10;

	/* Creation des boutons de commande */
	pos.x += 175; pos.y = 25;
	wxButton * Button = new wxButton(this,
						ID_PRINT_SETUP,
						_("Print S&etup"), pos);
	Button->SetForegroundColour(wxColour(80,80,0) );
	if ( win_size.x < (pos.x + Button->GetSize().x) )
		win_size.x = pos.x + Button->GetSize().x;

	pos.y += Button->GetSize().y + 15;
	Button = new wxButton(this,	ID_PRINT_PREVIEW,
						_("Pre&view"), pos);
	Button->SetForegroundColour(wxColour(0,0,150) );
	if ( win_size.x < (pos.x + Button->GetSize().x) )
		win_size.x = pos.x + Button->GetSize().x;

	pos.y += Button->GetSize().y + 5;
	Button = new wxButton(this,	ID_PRINT_EXECUTE,
						_("&Print"), pos);
	Button->SetForegroundColour(wxColour(0,100,0) );
	if ( win_size.x < (pos.x + Button->GetSize().x ) )
		win_size.x = pos.x + Button->GetSize().x;
	if ( win_size.x < (pos.x + Button->GetSize().x) )
		win_size.x = pos.x + Button->GetSize().x;

	pos.y += Button->GetSize().y + 15;
	Button = new wxButton(this,	ID_PRINT_QUIT,
						_("&Close"), pos);
	Button->SetForegroundColour(wxColour(0,0,150) );

	if ( win_size.x < (pos.x + Button->GetSize().x) )
		win_size.x = pos.x + Button->GetSize().x;

	win_size.x += 10;
	SetClientSize(win_size);
}


/**********************************************************/
int WinEDA_PrintFrame::SetLayerMaskFromListSelection(void)
/**********************************************************/
{
int page_count;

	s_PrintMaskLayer = 0;
#ifdef PCBNEW
int ii;
	for ( ii = 0, page_count = 0; ii < NB_LAYERS; ii++ )
	{
		if ( m_BoxSelecLayer[ii]->IsChecked() )
		{
			page_count++;
			s_PrintMaskLayer |= 1 << ii;
		}
	}
#else
	page_count = 1;
#endif
	return page_count;
}

/************************************************************/
void WinEDA_PrintFrame::SetColorOrBlack(wxCommandEvent& event)
/************************************************************/
{
	s_Print_Black_and_White = m_ColorOption->GetSelection();
}

/****************************************************/
void  WinEDA_PrintFrame::OnClose(wxCloseEvent & event)
/****************************************************/
/* called when WinEDA_PrintFrame is closed
*/
{
	if ( m_FineAdjustXscaleOpt ) m_XScaleAdjust = m_FineAdjustXscaleOpt->GetValue();
	if ( m_FineAdjustYscaleOpt ) m_YScaleAdjust = m_FineAdjustYscaleOpt->GetValue();
#ifdef PCBNEW
	if ( m_Parent->m_Parent->m_EDA_Config )
	{
		m_Parent->m_Parent->m_EDA_Config->Write(wxT("PrintXFineScaleAdj"), m_XScaleAdjust);
		m_Parent->m_Parent->m_EDA_Config->Write(wxT("PrintYFineScaleAdj"), m_YScaleAdjust);
		m_Parent->m_Parent->m_EDA_Config->Write(wxT("PrintScale"), s_Scale_Select);
	}
#endif
	EndModal(0);
	}

/*****************************************************/
void WinEDA_PrintFrame::OnQuit(wxCommandEvent& event)
/*****************************************************/
/* Called when the quit/close button is pressed
*/
{
	Close(TRUE);
}


/************************************************/
wxString WinEDA_PrintFrame::BuildPrintTitle(void)
/************************************************/
/* return a valid filename to create a print file
*/
{
wxString name, ext;

	wxFileName::SplitPath(m_Parent->m_CurrentScreen->m_FileName,
		(wxString*) NULL, & name, & ext);
	name += wxT("-") + ext;
	return name;
}

/******************************************************/
void WinEDA_PrintFrame::SetScale(wxCommandEvent& event)
/******************************************************/
{
#ifdef PCBNEW
	s_Scale_Select = m_ScaleOption->GetSelection();
	Scale_X = Scale_Y = s_ScaleList[s_Scale_Select];
	m_XScaleAdjust = m_FineAdjustXscaleOpt->GetValue();
	m_YScaleAdjust = m_FineAdjustYscaleOpt->GetValue();
	Scale_X *= m_XScaleAdjust;
	Scale_Y *= m_YScaleAdjust;
#endif
}

/*********************************************************/
void WinEDA_PrintFrame::SetPenWidth(wxSpinEvent& event)
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
	m_Buff_Width.Printf(wxT("%d"), PenMinWidth);
	m_ButtPenWidth->SetValue(m_Buff_Width);
}


/**********************************************************/
void WinEDA_PrintFrame::OnPrintSetup(wxCommandEvent& event)
/**********************************************************/
/* Open a dialog box for printer setup (printer options, page size ...)
*/
{
	wxPrintDialogData printDialogData(*g_PrintData);
    if ( printDialogData.Ok() )
    	{
		wxPrintDialog printerDialog(this, & printDialogData);
		printerDialog.ShowModal();

		* g_PrintData = printerDialog.GetPrintDialogData().GetPrintData();
        }
	else DisplayError(this, _("Printer Problem!") );
}


/************************************************************/
void WinEDA_PrintFrame::OnPrintPreview(wxCommandEvent& event)
/************************************************************/
/* Open and display a previewer frame for printing
*/
{
wxSize WSize;
wxPoint WPos;
int x, y;
bool print_ref = TRUE;

wxSpinEvent spinevent;

	SetScale(event);
	SetPenWidth(spinevent);

	if ( m_PagesOption )
		s_OptionPrintPage = m_PagesOption->GetSelection();

	if ( (m_Print_Sheet_Ref == NULL) || (m_Print_Sheet_Ref->GetValue() == FALSE) )
		print_ref = FALSE;

	// Pass two printout objects: for preview, and possible printing.
wxString title = BuildPrintTitle();
	wxPrintPreview *preview =
		new wxPrintPreview(new EDA_Printout(this, m_Parent, title, print_ref),
				new EDA_Printout(this, m_Parent, title, print_ref), g_PrintData);
	if ( preview == NULL )
		{
		DisplayError(this, _("There was a problem previewing"));
		return;
		}
#ifdef PCBNEW
	if ( s_OptionPrintPage ) SetLayerMaskFromListSelection();
#endif

	m_Parent->GetPosition(&x, &y);
	WPos.x = x + 4;
	WPos.y = y + 25;

	WSize = m_Parent->GetSize();
	WSize.x -= 3;
	WSize.y += 6;

	wxPreviewFrame *frame = new wxPreviewFrame(preview, this,
					title, WPos, WSize);
	frame->Initialize();
	frame->Show(TRUE);
}


/**********************************************************/
void WinEDA_PrintFrame::EDA_PrintPage(wxCommandEvent& event)
/**********************************************************/
/* Called on activate "Print CURRENT" button
*/
{
bool print_ref = TRUE;

	SetScale(event);

	s_OptionPrintPage = 0;
	if ( m_PagesOption )
		s_OptionPrintPage = m_PagesOption->GetSelection();

	if ( (m_Print_Sheet_Ref == NULL) || (m_Print_Sheet_Ref->GetValue() == FALSE) )
		print_ref = FALSE;

#ifdef PCBNEW
	if ( s_OptionPrintPage ) SetLayerMaskFromListSelection();
#endif

wxSpinEvent spinevent;
	SetPenWidth(spinevent);

	wxPrintDialogData printDialogData( * g_PrintData);

	wxPrinter printer(& printDialogData);

wxString title = BuildPrintTitle();
	EDA_Printout printout(this, m_Parent, title, print_ref);

#ifndef __WINDOWS__
	wxDC * dc = printout.GetDC();
	((wxPostScriptDC*)dc)->SetResolution(600);	// Postscript DC resolution is 600 ppi
#endif

	if (!printer.Print(this, &printout, TRUE))
		{
        if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
	    DisplayError(this, _("There was a problem printing") );
        return;
        }
	else
		{
		* g_PrintData = printer.GetPrintDialogData().GetPrintData();
		}
}


/**************************/
/* methodes de EDA_Printout */
/**************************/

/***************************************/
bool EDA_Printout::OnPrintPage(int page)
/***************************************/
{
wxString msg;

	msg.Printf(_("Print page %d"), page);
	m_Parent->Affiche_Message(msg);


#ifdef EESCHEMA
int ii;
BASE_SCREEN * screen = m_Parent->m_CurrentScreen;
BASE_SCREEN *oldscreen = screen;
	if( s_OptionPrintPage == 1 )
		{
		for ( ii = 1, screen = ScreenSch; ; ii++)
			{
			if ( ii == page ) break;
			screen = (BASE_SCREEN *) screen->Pnext;
			}
		}

	if (screen == NULL) return FALSE;
	ActiveScreen = (SCH_SCREEN*) screen;
	DrawPage();
	ActiveScreen = (SCH_SCREEN*) oldscreen;

#endif

#ifdef PCBNEW
	if ( (m_Parent->m_Ident == PCB_FRAME) || (m_Parent->m_Ident == GERBER_FRAME) )
	{
		if ( s_OptionPrintPage == 0 )
		{
			// compute layer mask from page number
			m_PrintFrame->SetLayerMaskFromListSelection();
			int ii, jj, mask = 1;
			for ( ii = 0, jj = 0; ii < NB_LAYERS; ii++)
			{
				if ( s_PrintMaskLayer & mask ) jj++;
				if ( jj == page )
				{
					s_PrintMaskLayer = mask;
					break;
				}
				mask <<= 1;
			}
			if ( ii == NB_LAYERS ) return FALSE;
		}
	}
	DrawPage();
#endif

	return TRUE;
}

/*********************************************************/
void EDA_Printout::GetPageInfo(int *minPage, int *maxPage,
			      int *selPageFrom, int *selPageTo)
/*********************************************************/
{
int ii = 1;
	*minPage = 1;
	*selPageFrom = 1;

#ifdef EESCHEMA
	if( s_OptionPrintPage == 1 )
	{
		BASE_SCREEN * screen;
		for ( ii = 0, screen = ScreenSch; screen != NULL; ii++)
		{
			screen = (BASE_SCREEN *) screen->Pnext;
		}
	}
#endif

#ifdef PCBNEW
	switch( s_OptionPrintPage )
	{
		case 0 :ii = m_PrintFrame->SetLayerMaskFromListSelection();
			break;

		case 1:
			ii = 1;
			break;
	}
#endif

	*maxPage = ii;
	*selPageTo = ii;
}


/**************************************/
bool EDA_Printout::HasPage(int pageNum)
/**************************************/
{
#ifdef EESCHEMA
BASE_SCREEN * screen;
int ii = 1;

	for ( screen = ScreenSch; screen != NULL; ii++)
		{
		if( ii == pageNum ) return TRUE;
		screen = (BASE_SCREEN *) screen->Pnext;
		}

	return FALSE;
#endif

#ifdef PCBNEW
	return TRUE;
#endif
}

/*************************************************************/
bool EDA_Printout::OnBeginDocument(int startPage, int endPage)
/*************************************************************/
{
  if (!wxPrintout::OnBeginDocument(startPage, endPage))
	return FALSE;

  return TRUE;
}

/********************************/
void EDA_Printout::DrawPage(void)
/********************************/
/*
	This is the real print function: print the active screen
*/
{
int tmpzoom;
wxPoint tmp_startvisu;
wxSize PageSize_in_mm;
wxSize SheetSize;		// Sheet size in internal units
wxSize PlotAreaSize;	// Taille de la surface utile de trace (pixels)
double scaleX, scaleY, scale;
wxPoint old_org;
wxPoint DrawOffset;	// Offset de trace
double userscale;
wxDC * dc = GetDC();
int DrawZoom = 1;

	wxBusyCursor dummy;

	GetPageSizeMM(&PageSize_in_mm.x, &PageSize_in_mm.y);

	/* Save old draw scale and draw offset */
	tmp_startvisu = ActiveScreen->m_StartVisu;
	tmpzoom = ActiveScreen->GetZoom();
	old_org = ActiveScreen->m_DrawOrg;
	/* Change draw scale and offset to draw the whole page*/
	ActiveScreen->SetZoom(DrawZoom);
	ActiveScreen->m_DrawOrg.x = ActiveScreen->m_DrawOrg.y = 0;
	ActiveScreen->m_StartVisu.x = ActiveScreen->m_StartVisu.y = 0;

	SheetSize = ActiveScreen->m_CurrentSheet->m_Size;	// size in 1/1000 inch
	SheetSize.x *= m_Parent->m_InternalUnits / 1000;
	SheetSize.y *= m_Parent->m_InternalUnits / 1000;	// size in pixels

	// Get the size of the DC in pixels
	dc->GetSize(&PlotAreaSize.x, &PlotAreaSize.y);

#ifdef PCBNEW
	WinEDA_BasePcbFrame * pcbframe = (WinEDA_BasePcbFrame*) m_Parent;
	pcbframe->m_Pcb->ComputeBoundaryBox();
	/* calcul des dimensions du PCB */
	if (s_ScaleList[s_Scale_Select] == 0)	//  fit in page
		{
		int extra_margin = 8000;	// Margin = 8000/2 units pcb = 0,4 inch
		SheetSize.x = pcbframe->m_Pcb->m_BoundaryBox.GetWidth() + extra_margin;
		SheetSize.y = pcbframe->m_Pcb->m_BoundaryBox.GetHeight() + extra_margin;
		userscale = 0.99;
		}
	else userscale = s_ScaleList[s_Scale_Select];

	if ( (s_ScaleList[s_Scale_Select] > 1.0) ||	//  scale > 1 -> Recadrage
			 (s_ScaleList[s_Scale_Select] == 0)	)	//  fit in page
		{
		DrawOffset.x += pcbframe->m_Pcb->m_BoundaryBox.Centre().x;
		DrawOffset.y += pcbframe->m_Pcb->m_BoundaryBox.Centre().y;
		}
#endif

	// Calculate a suitable scaling factor
	scaleX = (double)SheetSize.x / PlotAreaSize.x;
	scaleY = (double)SheetSize.y / PlotAreaSize.y;
	scale = wxMax(scaleX,scaleY) / userscale; // Use x or y scaling factor, whichever fits on the DC

	// ajust the real draw scale
#ifdef PCBNEW
double accurate_Xscale, accurate_Yscale;
	dc->SetUserScale(DrawZoom / scale * m_PrintFrame->m_XScaleAdjust,
					DrawZoom / scale * m_PrintFrame->m_YScaleAdjust);
	// Compute Accurate scale 1
	{
		int w, h;
		GetPPIPrinter(&w, &h);
		accurate_Xscale = ((double)(DrawZoom * w))/ PCB_INTERNAL_UNIT;
		accurate_Yscale = ((double)(DrawZoom * h))/ PCB_INTERNAL_UNIT;
		if ( IsPreview() ) 	// Scale must take in account the DC size in Preview
		{
			// Get the size of the DC in pixels
			dc->GetSize(&PlotAreaSize.x, &PlotAreaSize.y);
			GetPageSizePixels(&w, &h);
			accurate_Xscale *= PlotAreaSize.x; accurate_Xscale /= w;
			accurate_Yscale *= PlotAreaSize.y; accurate_Yscale /= h;
		}
		accurate_Xscale *= m_PrintFrame->m_XScaleAdjust;
		accurate_Yscale *= m_PrintFrame->m_YScaleAdjust;
	}
#else
	dc->SetUserScale(DrawZoom / scale, DrawZoom / scale);
#endif

#ifdef PCBNEW
	if ( (s_ScaleList[s_Scale_Select] > 1.0) ||	//  scale > 1 -> Recadrage
			(s_ScaleList[s_Scale_Select] == 0)	)	//  fit in page
	{
		DrawOffset.x -= (int)( (PlotAreaSize.x/2) * scale);
		DrawOffset.y -= (int)( (PlotAreaSize.y/2) * scale );
	}
	DrawOffset.x += (int)( (SheetSize.x/2) * (m_PrintFrame->m_XScaleAdjust -1.0));
	DrawOffset.y += (int)( (SheetSize.y/2) * (m_PrintFrame->m_YScaleAdjust -1.0));
#endif

	ActiveScreen->m_DrawOrg = DrawOffset;

	GRResetPenAndBrush(dc);
	if( s_Print_Black_and_White ) GRForceBlackPen(TRUE);


#ifdef EESCHEMA
	/* set Pen min width */
float ftmp;
	// PenMinWidth est donné en 1/100 mm, a convertir en pixels
	ftmp = (float)PenMinWidth / 100;	// ftmp est en mm
	ftmp *= (float)PlotAreaSize.x / PageSize_in_mm.x;	/* ftmp est en pixels */
	SetPenMinWidth((int)ftmp);
#else
	SetPenMinWidth(1);
#endif

WinEDA_DrawPanel * panel = m_Parent->DrawPanel;
EDA_Rect tmp = panel->m_ClipBox;

	panel->m_ClipBox.SetOrigin(wxPoint(0,0));
	panel->m_ClipBox.SetSize(wxSize(0x7FFFFF0, 0x7FFFFF0));

	g_IsPrinting = TRUE;
#ifdef EESCHEMA
	if( s_Print_Black_and_White )
		g_PrintFillMask = FILLED_WITH_BG_BODYCOLOR;
#endif
#ifdef PCBNEW
	if ( userscale == 1.0 )	// Draw the Sheet refs at optimum scale, and board at 1.0 scale
	{
		if ( m_Print_Sheet_Ref )
			m_Parent->TraceWorkSheet( dc, ActiveScreen);
		m_Print_Sheet_Ref = FALSE;
		dc->SetUserScale( accurate_Yscale, accurate_Yscale);
	}
#endif
	
	panel->PrintPage(dc, m_Print_Sheet_Ref, s_PrintMaskLayer);

#ifdef EESCHEMA
	g_PrintFillMask = 0;
#endif

	g_IsPrinting = FALSE;
	panel->m_ClipBox = tmp;

	SetPenMinWidth(1);
	GRForceBlackPen(FALSE);

	ActiveScreen->m_StartVisu = tmp_startvisu;
	ActiveScreen->m_DrawOrg = old_org;
	ActiveScreen->SetZoom(tmpzoom);
}

