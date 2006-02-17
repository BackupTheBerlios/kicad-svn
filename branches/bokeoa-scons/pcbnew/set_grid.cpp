	/***************************************************/
	/* set_grid.cpp - Gestion de la grille utilisateur */
	/***************************************************/
/*
 Affichage et modifications des parametres de travail de PcbNew
 Parametres = dimensions de la grille utilisateur
*/


#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "pcbnew.h"
#include "pcbplot.h"
#include "autorout.h"

#include "id.h"

#include "protos.h"
#include <wx/spinctrl.h>

enum id_optpcb
{
	ID_ACCEPT_OPT = 1000,
	ID_CANCEL_OPT
};


/**************************************************************************/
/* classe derivee pour la frame de Configuration WinEDA_PcbGridFrame */
/**************************************************************************/

class WinEDA_PcbGridFrame: public wxDialog
{
private:
protected:
public:

	WinEDA_BasePcbFrame * m_Parent;

	wxRadioBox * m_UnitGrid;
	wxTextCtrl * m_OptGridSizeX;
	wxTextCtrl * m_OptGridSizeY;

	// Constructor and destructor
	WinEDA_PcbGridFrame(WinEDA_BasePcbFrame *parent,const wxPoint& pos);
	~WinEDA_PcbGridFrame(void) {};

	void OnCloseWindow(wxCloseEvent & event);
	void OnQuit(wxCommandEvent & event);
	void AcceptPcbOptions(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

};
/* Construction de la table des evenements pour WinEDA_PcbGridFrame */
BEGIN_EVENT_TABLE(WinEDA_PcbGridFrame, wxDialog)
	EVT_BUTTON(ID_ACCEPT_OPT, WinEDA_PcbGridFrame::AcceptPcbOptions)
	EVT_BUTTON(ID_CANCEL_OPT, WinEDA_PcbGridFrame::OnQuit)
END_EVENT_TABLE()



	/***************************************/
	/* Constructeur de WinEDA_PcbGridFrame */
	/***************************************/

WinEDA_PcbGridFrame::WinEDA_PcbGridFrame(WinEDA_BasePcbFrame *parent,
		const wxPoint& framepos):
		wxDialog(parent, -1, _("User Grid Size"), framepos, wxDefaultSize,
		DIALOG_STYLE )
{
wxPoint pos;
wxString msg;
int lowy = 0;
PCB_SCREEN * screen;
	
	m_Parent = parent;
	screen = m_Parent->GetScreen();	
	SetFont(*g_DialogFont);

	g_UserGrid = screen->m_UserGrid;
	g_UserGrid_Unit = screen->m_UserGridUnit;

	pos.x = 5; pos.y = 5;
wxString UnitList[2] =
	{ _("Inches"), _("mm")
	};
	
	m_UnitGrid = new wxRadioBox(this, -1, _("Grid Size Units"), pos, wxDefaultSize,
		2, UnitList, 1, wxRA_SPECIFY_COLS);
	if ( screen->m_UserGridUnit != INCHES )
		m_UnitGrid->SetSelection(1);
	
	pos.y += 75;
	wxStaticText * Text = new wxStaticText(this, -1, _("User Grid Size X"),
				pos, wxSize(-1,-1), 0 );
	pos.y += 14;
	msg.Printf( wxT("%.4f"), g_UserGrid.x );
	m_OptGridSizeX = new wxTextCtrl(this, -1, msg, pos );

	pos.y += 25;
	Text = new wxStaticText(this, -1, _("User Grid Size Y"), pos,
				wxSize(-1,-1), 0 );
	pos.y += 14;
	msg.Printf( wxT("%.4f"), g_UserGrid.y );
	m_OptGridSizeY = new wxTextCtrl(this, -1, msg, pos );
	lowy = pos.y + 20;

	pos.x = 200; pos.y =15;
	wxButton * Button = new wxButton(this, ID_ACCEPT_OPT, _("Ok"), pos);
	Button->SetForegroundColour(*wxRED);
	pos.y += Button->GetSize().y  + 5;
	Button = new wxButton(this, ID_CANCEL_OPT, _("Cancel"), pos);
	Button->SetForegroundColour(*wxBLUE);

	/* Size adjust */
	pos.y =10 + lowy;
	SetClientSize(wxSize(300, pos.y));
}



/**********************************************************************/
void  WinEDA_PcbGridFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
/**********************************************************************/
{
    // true is to force the frame to close
    Close(true);
}



/****************************************************************/
void WinEDA_PcbGridFrame::AcceptPcbOptions(wxCommandEvent& event)
/****************************************************************/
{
double dtmp = 0;
	
	g_UserGrid_Unit = m_UnitGrid->GetSelection();
	m_OptGridSizeX->GetValue().ToDouble(&dtmp); g_UserGrid.x = dtmp;
	m_OptGridSizeY->GetValue().ToDouble(&dtmp); g_UserGrid.y = dtmp;

	m_Parent->GetScreen()->m_UserGrid = g_UserGrid;
	m_Parent->GetScreen()->m_UserGridUnit = g_UserGrid_Unit;

	EndModal(1);
	
	m_Parent->DrawPanel->Refresh(TRUE);
}



/************************************************************/
void WinEDA_BasePcbFrame::InstallGridFrame(const wxPoint & pos)
/************************************************************/
{
	WinEDA_PcbGridFrame * GridFrame =
				new WinEDA_PcbGridFrame(this, pos);
	GridFrame->ShowModal(); GridFrame->Destroy();
}

