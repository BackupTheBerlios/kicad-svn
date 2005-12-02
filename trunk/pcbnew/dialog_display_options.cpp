	/****************************************************************/
	/* dialog_display_options.cpp - Gestion des Options d'affichage */
	/****************************************************************/
/*
 Affichage et modifications des parametres de travail de PcbNew
 Parametres = mode d'affichage des textes et elements graphiques
*/


#include "fctsys.h"
#include "gr_basic.h"
#include "macros.h"

#include "common.h"
#include "pcbnew.h"
#include "pcbplot.h"
#include "autorout.h"

#include "id.h"

#include "protos.h"
#include <wx/spinctrl.h>

/* Fonctions locales */

/* variables locales */


class WinEDA_DisplayOptionsDialog: public wxDialog
{
private:
	WinEDA_BasePcbFrame * m_Parent;

	wxRadioBox * m_OptDisplayTracks;
	wxRadioBox * m_OptDisplayTracksClearance;

	wxRadioBox * m_OptDisplayPads;
	wxCheckBox * m_OptDisplayPadClearance;
	wxCheckBox * m_OptDisplayPadNumber;
	wxCheckBox * m_OptDisplayPadNoConn;

	wxRadioBox * m_OptDisplayModTexts;
	wxRadioBox * m_OptDisplayModEdges;

	wxRadioBox * m_OptDisplayDrawings;
	wxRadioBox * m_OptDisplayViaHole;


public:
	// Constructor and destructor
	WinEDA_DisplayOptionsDialog(WinEDA_BasePcbFrame *parent,const wxPoint& pos);
	~WinEDA_DisplayOptionsDialog(void) {};

private:
	void AcceptPcbOptions(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

};
/* Construction de la table des evenements pour WinEDA_DisplayOptionsDialog */
BEGIN_EVENT_TABLE(WinEDA_DisplayOptionsDialog, wxDialog)
	EVT_BUTTON(ID_ACCEPT_OPT, WinEDA_DisplayOptionsDialog::AcceptPcbOptions)
	EVT_BUTTON(ID_CANCEL_OPT, WinEDA_DisplayOptionsDialog::OnQuit)
END_EVENT_TABLE()



	/***********************************************/
	/* Constructeur de WinEDA_DisplayOptionsDialog */
	/***********************************************/

WinEDA_DisplayOptionsDialog::WinEDA_DisplayOptionsDialog(WinEDA_BasePcbFrame *parent,
		const wxPoint& framepos):
		wxDialog(parent, -1, _("Display Options"), framepos, wxSize(520, 350),
		DIALOG_STYLE )
{
wxPoint pos;
int ii, jj, h, w;
wxSize size;
wxSize winsize;
	
	m_Parent = parent;
	SetFont(*g_DialogFont);

	pos.x = 5; pos.y = 5;
	// Options for tracks; how to draw tracks and type of Vias
wxString list_opt2[2] = { _("Sketch"), _("Filled") };
	m_OptDisplayTracks = new wxRadioBox(this, -1, _("Tracks:"),
				pos, wxDefaultSize,
				2, list_opt2, 1);
	if ( DisplayOpt.DisplayPcbTrackFill ) m_OptDisplayTracks->SetSelection(1);

wxString list_opt_track_clr[3] = { _("Always"), _("New track"), _("Never") };
	m_OptDisplayTracks->GetSize(&jj, &ii); pos.y += ii + 5;
	m_OptDisplayTracksClearance = new wxRadioBox(this, -1,
			_("Show Track Clearance"), pos, wxDefaultSize,
				3, list_opt_track_clr, 1);
	if ( DisplayOpt.DisplayTrackIsol )
		m_OptDisplayTracksClearance->SetSelection(0);
	else if ( g_ShowIsolDuringCreateTrack )
		m_OptDisplayTracksClearance->SetSelection(1);
	else m_OptDisplayTracksClearance->SetSelection(2);
	
	// Set the draw via hole option:
wxString list_opt_via_hole[3] = { _("Never"), _("defined holes"), _("Always") };
	m_OptDisplayTracksClearance->GetSize(&jj, &ii); pos.y += ii + 5;
	m_OptDisplayViaHole = new wxRadioBox(this, -1,
			_("Show Via Holes"), pos, wxDefaultSize,
				3, list_opt_via_hole, 1);
	m_OptDisplayViaHole->SetSelection(DisplayOpt.m_DisplayViaMode );
	
	winsize.y = m_OptDisplayViaHole->GetRect().GetBottom();

	// Options for Modules; how to draw texts, edges and pads
	pos.x += 150; pos.y = 5;
wxString list_opt3[3] = {_("Line"), _("Filled"), _("Sketch") };
	m_OptDisplayModTexts = new wxRadioBox(this, -1, _("Module Texts"),
				pos, wxDefaultSize,
				3, list_opt3, 1);
	m_OptDisplayModTexts->SetSelection(DisplayOpt.DisplayModText);

	m_OptDisplayModTexts->GetSize(&jj, &ii); pos.y += ii + 5;
	m_OptDisplayModEdges = new wxRadioBox(this, -1, _("Module Edges:"),
				pos, wxDefaultSize,
				3, list_opt3, 1);
	m_OptDisplayModEdges->SetSelection(DisplayOpt.DisplayModEdge);
	m_OptDisplayModEdges->GetSize(&w, &h);
	size.y = pos.y + h;

	pos.x += 110; pos.y = 5;
	m_OptDisplayPads = new wxRadioBox(this, -1, _("Pads:"),
				pos, wxDefaultSize,
				2, list_opt2, 1);
	if ( DisplayOpt.DisplayPadFill ) m_OptDisplayPads->SetSelection(1);

	pos.x += 30; pos.y = 105;
	m_OptDisplayPadClearance = new wxCheckBox(this, -1,
		_("Show Pad Clearance"), pos);
	if ( DisplayOpt.DisplayPadIsol ) m_OptDisplayPadClearance->SetValue(TRUE);

	pos.y += m_OptDisplayPadClearance->GetSize().y + 5;
	m_OptDisplayPadNumber = new wxCheckBox(this, -1,
		_("Show Pad Number"), pos);
	if ( DisplayOpt.DisplayPadNum ) m_OptDisplayPadNumber->SetValue(TRUE);

	pos.y += m_OptDisplayPadNumber->GetSize().y + 5;
	m_OptDisplayPadNoConn = new wxCheckBox(this, -1,
		_("Show Pad NoConnect"), pos);
	if ( DisplayOpt.DisplayPadNoConn ) m_OptDisplayPadNoConn->SetValue(TRUE);

	pos.x += 70; pos.y = 5;
	m_OptDisplayDrawings = new wxRadioBox(this, -1, _("Display other items:"),
				pos, wxDefaultSize,
				3, list_opt3, 1);
	m_OptDisplayDrawings->SetSelection(DisplayOpt.DisplayDrawItems);

	m_OptDisplayDrawings->GetSize(&h, &w);
	size.x = pos.x + h ;

	pos.x = 200; pos.y = size.y + 10;
	wxButton * Button = new wxButton(this, ID_ACCEPT_OPT, _("Ok"), pos);
	Button->SetForegroundColour(*wxRED);
	Button->SetDefault();

	pos.x += Button->GetSize().x  + 15;
	Button = new wxButton(this, ID_CANCEL_OPT, _("Cancel"), pos);
	Button->SetForegroundColour(*wxBLUE);
	size.y = pos.y + Button->GetSize().y;

	winsize.y = MAX(winsize.y, Button->GetRect().GetBottom());
	SetClientSize(size.x + 5, winsize.y + 5);
}


/*************************************************************************/
void  WinEDA_DisplayOptionsDialog::OnQuit(wxCommandEvent& WXUNUSED(event))
/*************************************************************************/
{
    // true is to force the frame to close
    Close(true);
}



/*************************************************************/
void WinEDA_DisplayOptionsDialog::AcceptPcbOptions(wxCommandEvent& event)
/*************************************************************/
/* Met a jour les options
*/
{
	if ( m_OptDisplayTracks->GetSelection() == 1)
		DisplayOpt.DisplayPcbTrackFill = TRUE;
	else DisplayOpt.DisplayPcbTrackFill = FALSE;

	m_Parent->m_DisplayPcbTrackFill = DisplayOpt.DisplayPcbTrackFill;
	DisplayOpt.m_DisplayViaMode = m_OptDisplayViaHole->GetSelection();

	switch ( m_OptDisplayTracksClearance->GetSelection() )
	{
		case 0:
			DisplayOpt.DisplayTrackIsol = TRUE;
			g_ShowIsolDuringCreateTrack = TRUE;
			break;
		case 1:
			DisplayOpt.DisplayTrackIsol = FALSE;
			g_ShowIsolDuringCreateTrack = TRUE;
			break;
		case 2:
			DisplayOpt.DisplayTrackIsol = FALSE;
			g_ShowIsolDuringCreateTrack = FALSE;
			break;
	}

	m_Parent->m_DisplayModText = DisplayOpt.DisplayModText =
			m_OptDisplayModTexts->GetSelection();
	m_Parent->m_DisplayModEdge = DisplayOpt.DisplayModEdge =
			m_OptDisplayModEdges->GetSelection();

	if (m_OptDisplayPads->GetSelection() == 1 )
		 DisplayOpt.DisplayPadFill = TRUE;
	else DisplayOpt.DisplayPadFill = FALSE;

	m_Parent->m_DisplayPadFill = DisplayOpt.DisplayPadFill;

	DisplayOpt.DisplayPadIsol = m_OptDisplayPadClearance->GetValue();

	m_Parent->m_DisplayPadNum = DisplayOpt.DisplayPadNum = m_OptDisplayPadNumber->GetValue();

	DisplayOpt.DisplayPadNoConn = m_OptDisplayPadNoConn->GetValue();

	DisplayOpt.DisplayDrawItems = m_OptDisplayDrawings->GetSelection();

	m_Parent->DrawPanel->Refresh(TRUE);

	EndModal(1);
}

