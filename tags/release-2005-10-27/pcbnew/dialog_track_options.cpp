	/*************************************************/
	/* dialog_track_options.cpp  */
	/*************************************************/
/*
 Affichage et modifications des parametres de travail de PcbNew
 Parametres = dimensions des via, pistes, isolements, options...
*/

/*
#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "pcbnew.h"
#include "pcbplot.h"
#include "autorout.h"

#include "id.h"

#include <wx/spinctrl.h>
*/

/* Fonctions locales */

/* variables locales */


/********************************************/
class WinEDA_PcbTracksDialog: public wxDialog
/********************************************/
/* Reglages des caracteristiques des pistes et vias
*/
{
private:
	WinEDA_PcbFrame * m_Parent;
	wxRadioBox * m_OptViaType;
	WinEDA_ValueCtrl * m_OptViaSize;
	WinEDA_ValueCtrl * m_OptViaDrill;
	WinEDA_ValueCtrl * m_OptTrackWidth;
	WinEDA_ValueCtrl * m_OptTrackClearance;
	WinEDA_ValueCtrl * m_OptCustomViaDrill;

	WinEDA_ValueCtrl * m_OptMaskMargin;


public:
	// Constructor and destructor
	WinEDA_PcbTracksDialog(WinEDA_PcbFrame *parent,const wxPoint& pos);
	~WinEDA_PcbTracksDialog(void) {};

private:
	void OnCloseWindow(wxCloseEvent & event);
	void OnQuit(wxCommandEvent& event);
	void AcceptPcbOptions(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

};
/* Construction de la table des evenements pour WinEDA_PcbTracksDialog */
BEGIN_EVENT_TABLE(WinEDA_PcbTracksDialog, wxDialog)
	EVT_BUTTON(ID_ACCEPT_OPT, WinEDA_PcbTracksDialog::AcceptPcbOptions)
	EVT_BUTTON(ID_CANCEL_OPT, WinEDA_PcbTracksDialog::OnQuit)
END_EVENT_TABLE()



	/*************************************************/
	/* Constructeur de WinEDA_PcbTracksDialog */
	/************************************************/

WinEDA_PcbTracksDialog::WinEDA_PcbTracksDialog(WinEDA_PcbFrame *parent,
		const wxPoint& framepos):
		wxDialog(parent, -1, _("Tracks and Vias"), framepos, wxSize(350, 280),
		DIALOG_STYLE )
{
wxPoint pos;

	m_Parent = parent;
	SetFont(*g_DialogFont);

	pos.x = 5; pos.y = 5;
	/* Selection du type et size des vias et pistes */
	m_OptViaSize = new WinEDA_ValueCtrl(this, _("Via Size"), g_DesignSettings.m_CurrentViaSize,
						UnitMetric, pos, PCB_INTERNAL_UNIT );

	pos.y += 5 + m_OptViaSize->GetDimension().y;
	m_OptViaDrill = new WinEDA_ValueCtrl(this, _("Default Via Drill"), g_DesignSettings.m_ViaDrill,
						UnitMetric, pos, PCB_INTERNAL_UNIT );

	pos.y += 5 + m_OptViaSize->GetDimension().y;
	m_OptCustomViaDrill = new WinEDA_ValueCtrl(this, _("Alternate Via Drill"), g_ViaHoleLastValue,
						UnitMetric, pos, PCB_INTERNAL_UNIT );

	pos.y += 15 + m_OptCustomViaDrill->GetDimension().y;
wxString list_via_shapes[3] = { _(" Blind Via"), _("Buried Via"),  _("Standard Via")};
	m_OptViaType = new wxRadioBox(this, -1, _("Via Type"), pos, wxDefaultSize,
					3, list_via_shapes, 1);
	if (g_DesignSettings.m_CurrentViaType < 1) g_DesignSettings.m_CurrentViaType = 1;
	if (g_DesignSettings.m_CurrentViaType > 3) g_DesignSettings.m_CurrentViaType = 3;
	m_OptViaType->SetSelection(g_DesignSettings.m_CurrentViaType - 1);

	/* Track width */
	pos.x += 180; pos.y = 5;
	m_OptTrackWidth = new WinEDA_ValueCtrl(this, _("Track Width"), g_DesignSettings.m_CurrentTrackWidth,
						UnitMetric, pos, PCB_INTERNAL_UNIT );


	/* Clearance ( Isolation) Track / Track ( Pour DRC et autorautage) */
	pos.y += 5 + m_OptTrackWidth->GetDimension().y;
	m_OptTrackClearance = new WinEDA_ValueCtrl(this, _("Clearance"), g_DesignSettings.m_TrackClearence,
						UnitMetric, pos, PCB_INTERNAL_UNIT );


	pos.y += 15 + m_OptTrackClearance->GetDimension().y;
	m_OptMaskMargin = new WinEDA_ValueCtrl(this, _("Mask clearance"),
						g_DesignSettings.m_MaskMargin,
						UnitMetric, pos, PCB_INTERNAL_UNIT );

	pos.y += 20 + m_OptMaskMargin->GetDimension().y;
	wxButton * Button = new wxButton(this, ID_ACCEPT_OPT, _("Ok"), pos);
	Button->SetForegroundColour(*wxRED);
	pos.y += Button->GetSize().y + 10;
	Button = new wxButton(this, ID_CANCEL_OPT, _("Cancel"), pos);
	Button->SetForegroundColour(*wxBLUE);

}

/**********************************************************************/
void  WinEDA_PcbTracksDialog::OnQuit(wxCommandEvent& WXUNUSED(event))
/**********************************************************************/
{
    // true is to force the frame to close
    Close(true);
}


/*******************************************************************/
void WinEDA_PcbTracksDialog::AcceptPcbOptions(wxCommandEvent& event)
/*******************************************************************/
{

	g_DesignSettings.m_CurrentViaType = m_OptViaType->GetSelection() + 1;
	if ( g_DesignSettings.m_CurrentViaType != VIA_NORMALE )
	{
		if( ! IsOK(this,
			_("You have selected VIA Blind or VIA Buried\nWARNING: this feature is EXPERIMENTAL!!! Accept ?") ) )
			g_DesignSettings.m_CurrentViaType = VIA_NORMALE;
	}

	g_DesignSettings.m_CurrentViaSize = m_OptViaSize->GetValue();
	g_DesignSettings.m_ViaDrill = m_OptViaDrill->GetValue();
	g_ViaHoleLastValue = m_OptCustomViaDrill->GetValue();
	g_DesignSettings.m_CurrentTrackWidth = m_OptTrackWidth->GetValue();
	g_DesignSettings.m_TrackClearence = m_OptTrackClearance->GetValue();

	g_DesignSettings.m_MaskMargin = m_OptMaskMargin->GetValue();

	m_Parent->DisplayTrackSettings();

	m_Parent->AddHistory(g_DesignSettings.m_CurrentViaSize, TYPEVIA);
	m_Parent->AddHistory(g_DesignSettings.m_CurrentTrackWidth, TYPETRACK);
	EndModal(1);
}


/*********************************************************************/
void WinEDA_BasePcbFrame::AddHistory(int value, DrawStructureType type)
/**********************************************************************/
// Mise a jour des listes des dernieres epaisseurs de via et track utilisées
{
bool addhistory = TRUE;
int ii;

	switch ( type )
		{
		case TYPETRACK:
			for ( ii = 0; ii < HIST0RY_NUMBER; ii++)
				{
				if ( g_DesignSettings.m_TrackWidhtHistory[ii] == value )
					{
					addhistory = FALSE; break;
					}
				}
			if ( ! addhistory ) break;
			for ( ii = HIST0RY_NUMBER -1; ii > 0 ; ii--)
				{
				g_DesignSettings.m_TrackWidhtHistory[ii] = g_DesignSettings.m_TrackWidhtHistory[ii-1];
				}
			g_DesignSettings.m_TrackWidhtHistory[0] = value;

			// Reclassement par valeur croissante
			for ( ii = 0; ii < HIST0RY_NUMBER -1; ii++)
				{
				if ( g_DesignSettings.m_TrackWidhtHistory[ii+1] == 0 ) break;	// Fin de liste
				if ( g_DesignSettings.m_TrackWidhtHistory[ii] > g_DesignSettings.m_TrackWidhtHistory[ii+1]  )
					{
					EXCHG(g_DesignSettings.m_TrackWidhtHistory[ii], g_DesignSettings.m_TrackWidhtHistory[ii+1]);
					}
				}
			break;

		case TYPEVIA:
			for ( ii = 0; ii < HIST0RY_NUMBER; ii++)
				{
				if (g_DesignSettings.m_ViaSizeHistory[ii] == value)
					{
					addhistory = FALSE; break;
					}
				}
			if ( ! addhistory ) break;

			for ( ii = HIST0RY_NUMBER -1; ii > 0 ; ii--)
				{
				g_DesignSettings.m_ViaSizeHistory[ii] = g_DesignSettings.m_ViaSizeHistory[ii-1];
				}
			g_DesignSettings.m_ViaSizeHistory[0] = value;

			// Reclassement par valeur croissante
			for ( ii = 0; ii < HIST0RY_NUMBER -1; ii++)
				{
				if ( g_DesignSettings.m_ViaSizeHistory[ii+1] == 0 ) break;	// Fin de liste
				if ( g_DesignSettings.m_ViaSizeHistory[ii] > g_DesignSettings.m_ViaSizeHistory[ii+1]  )
					{
					EXCHG(g_DesignSettings.m_ViaSizeHistory[ii], g_DesignSettings.m_ViaSizeHistory[ii+1]);
					}
				}
			break;

		default:
			break;
		}
}


