	/*************************************************/
	/* options.cpp - Gestion des Options et Reglages */
	/*************************************************/
/*
 Affichage et modifications des parametres de travail de PcbNew
 Parametres = dimensions des via, pistes, isolements, options...
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

/* Fonctions locales */

/* variables locales */


/*****************************************************************/
void WinEDA_PcbFrame::OnSelectOptionToolbar(wxCommandEvent& event)
/*****************************************************************/
/* Mises a jour de differentes variables de gestion d'options apres action sur
le toolbar d'options
*/
{
int id = event.GetId();
wxClientDC dc(DrawPanel);

	DrawPanel->PrepareGraphicContext(&dc);
	switch ( id )
		{
		case ID_TB_OPTIONS_DRC_OFF:
			Drc_On = m_OptionsToolBar->GetToolState(id) ? FALSE : TRUE;
			break;

		case ID_TB_OPTIONS_SHOW_GRID:
			m_Draw_Grid = g_ShowGrid = m_OptionsToolBar->GetToolState(id);
			DrawPanel->ReDraw(&dc, TRUE);
			break;

		case ID_TB_OPTIONS_SHOW_RATSNEST:
			g_Show_Ratsnest = m_OptionsToolBar->GetToolState(id);
			Ratsnest_On_Off(&dc);
			break;

		case ID_TB_OPTIONS_SHOW_MODULE_RATSNEST:
			g_Show_Module_Ratsnest = m_OptionsToolBar->GetToolState(id);
			break;

		case ID_TB_OPTIONS_SELECT_UNIT_MM:
			UnitMetric = MILLIMETRE;
		case ID_TB_OPTIONS_SELECT_UNIT_INCH:
			if ( id == ID_TB_OPTIONS_SELECT_UNIT_INCH )
				UnitMetric = INCHES;
			m_SelTrackWidthBox_Changed = TRUE;
			Affiche_Status_Box();	 /* Reaffichage des coord curseur */
			ReCreateAuxiliaryToolbar();
			DisplayUnitsMsg();
			break;

		case ID_TB_OPTIONS_SHOW_POLAR_COORD:
			Affiche_Message(wxEmptyString);
			DisplayOpt.DisplayPolarCood = m_OptionsToolBar->GetToolState(id);
			Affiche_Status_Box();	 /* Reaffichage des coord curseur */
			break;

		case ID_TB_OPTIONS_SELECT_CURSOR:
			GetScreen()->Trace_Curseur(DrawPanel, &dc);
			g_CursorShape = m_OptionsToolBar->GetToolState(id);
			GetScreen()->Trace_Curseur(DrawPanel, &dc);
			break;

		case ID_TB_OPTIONS_AUTO_DEL_TRACK:
			g_AutoDeleteOldTrack = m_OptionsToolBar->GetToolState(id);
			break;

		case ID_TB_OPTIONS_SHOW_ZONES:
			DisplayOpt.DisplayZones = m_OptionsToolBar->GetToolState(id);
			DrawPanel->ReDraw(&dc, TRUE);
			break;

		case ID_TB_OPTIONS_SHOW_PADS_SKETCH:
			m_DisplayPadFill = DisplayOpt.DisplayPadFill =
				 ! m_OptionsToolBar->GetToolState(id);
			DrawPanel->ReDraw(&dc, TRUE);
			break;

		case ID_TB_OPTIONS_SHOW_TRACKS_SKETCH:
			m_DisplayPcbTrackFill = DisplayOpt.DisplayPcbTrackFill =
				! m_OptionsToolBar->GetToolState(id);
			DrawPanel->ReDraw(&dc, TRUE);
			break;

		case ID_TB_OPTIONS_SHOW_HIGHT_CONTRAST_MODE:
			DisplayOpt.ContrastModeDisplay =
				m_OptionsToolBar->GetToolState(id);
			DrawPanel->ReDraw(&dc, TRUE);
			break;

		case ID_TB_OPTIONS_SHOW_EXTRA_VERTICAL_TOOLBAR1:
			if ( m_OptionsToolBar->GetToolState(id) ) // show aux V toolbar
				ReCreateAuxVToolbar();
			else
			{
				delete m_AuxVToolBar; m_AuxVToolBar = NULL;
				wxSizeEvent SizeEv;
				OnSize(SizeEv);
			}
			break;

		default:
			DisplayError(this, wxT("WinEDA_PcbFrame::OnSelectOptionToolbar error"));
			break;
		}

	SetToolbars();
}


enum id_optpcb
{
	ID_ACCEPT_OPT = 1000,
	ID_CANCEL_OPT
};
	/*************************************************/
	/* classe derivee pour la frame de Configuration */
	/*************************************************/

class WinEDA_PcbGeneralOptionsFrame: public wxDialog
{
private:

	WinEDA_PcbFrame * m_Parent;
	wxRadioBox * m_PolarDisplay;
	wxRadioBox * m_BoxUnits;
	wxRadioBox * m_CursorShape;
	wxSpinCtrl * m_SaveTime;
	wxSpinCtrl * m_LayerNumber;
	wxSpinCtrl * m_MaxShowLinks;
	wxCheckBox * m_DrcOn;
	wxCheckBox * m_ShowGlobalRatsnest;
	wxCheckBox * m_ShowModuleRatsnest;
	wxCheckBox * m_TrackAutodel;
	wxCheckBox * m_Track_45_Only_Ctrl;
	wxCheckBox * m_Segments_45_Only_Ctrl;
	wxCheckBox * m_AutoPANOpt;
	wxDC * m_DC;


	// Constructor and destructor
public:
	WinEDA_PcbGeneralOptionsFrame(WinEDA_PcbFrame *parent, wxDC * DC,
					const wxPoint& pos);
	~WinEDA_PcbGeneralOptionsFrame(void) {};

private:
	void AcceptPcbOptions(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

};
/* Construction de la table des evenements pour WinEDA_PcbGeneralOptionsFrame */
BEGIN_EVENT_TABLE(WinEDA_PcbGeneralOptionsFrame, wxDialog)
	EVT_BUTTON(ID_ACCEPT_OPT, WinEDA_PcbGeneralOptionsFrame::AcceptPcbOptions)
	EVT_BUTTON(ID_CANCEL_OPT, WinEDA_PcbGeneralOptionsFrame::OnQuit)
END_EVENT_TABLE()




	/*************************************************/
	/* Constructeur de WinEDA_PcbGeneralOptionsFrame */
	/************************************************/

WinEDA_PcbGeneralOptionsFrame::WinEDA_PcbGeneralOptionsFrame(WinEDA_PcbFrame *parent,
		wxDC * DC, const wxPoint& framepos):
		wxDialog(parent, -1, _("General Options"), framepos, wxSize(470, 280),
		DIALOG_STYLE )
{
wxPoint pos;
int w, h;
int cb_height = 20;

	m_Parent = parent;
	SetFont(*g_DialogFont);
	m_DC = DC;

	pos.x = 300; pos.y = 180;
	wxButton * Button = new wxButton(this, ID_ACCEPT_OPT, _("Ok"), pos);
	pos.y += Button->GetSize().y  + 5;
	Button->SetForegroundColour(*wxRED);
	Button = new wxButton(this, ID_CANCEL_OPT, _("Cancel"), pos);
	Button->SetForegroundColour(*wxBLUE);

	/* Display Selection affichage des coordonnées polaires */
	pos.x = 5; pos.y = 5;
wxString list_coord[2] = { _("No Display"), _("Display") };
	m_PolarDisplay = new wxRadioBox(this, -1, _("Display Polar Coord"),
					pos, wxDefaultSize,
					2, list_coord, 1);
	m_PolarDisplay->SetSelection(DisplayOpt.DisplayPolarCood ? 1 : 0);

	/* Selection chois des unités d'affichage */
	m_PolarDisplay->GetSize(&w, &h);
	pos.y += h + 5;
wxString list_units[2] = { _("Inches"), _("millimeters") };
	m_BoxUnits = new wxRadioBox(this, -1, _("Units"), pos, wxDefaultSize,
					2, list_units, 1);
	m_BoxUnits->SetSelection( UnitMetric ? 1 : 0);

	/* Selection forme du curseur */
	m_BoxUnits->GetSize(&w, &h);
	pos.y += h + 5;
wxString list_cursors[2] = { _("Small"), _("Big") };
	m_CursorShape = new wxRadioBox(this, -1, _("Cursor"), pos, wxDefaultSize,
					2, list_cursors, 1);
	m_CursorShape->SetSelection( g_CursorShape ? 1 : 0);

	/* Selection periode entre 2 sauvegardes automatiques*/
	pos.x = 120, pos.y = 85;
	wxStaticText * text = new wxStaticText(this, -1,
				_("Auto Save (minuts):"), pos );
	pos.y += text->GetSize().y + 4;
wxString timevalue;
	timevalue << g_TimeOut / 60;
	m_SaveTime = new wxSpinCtrl(this, -1, timevalue, pos, wxSize(60, -1) );
	m_SaveTime->SetRange(0, 60);

	/* Selection du nombre de couches cuivre autorisées */
	pos.y += 35;
	text = new wxStaticText(this, -1, _("Number of Layers:"), pos );
	pos.y += text->GetSize().y + 4;
wxString nblayers;
	nblayers << g_DesignSettings.m_CopperLayerCount;
	m_LayerNumber = new wxSpinCtrl(this, -1, nblayers, pos, wxSize(60, -1) );
	m_LayerNumber->SetRange(1, 16);


wxString value;
	pos.y += 35;
	value << g_MaxLinksShowed;
	text = new wxStaticText(this, -1, _("Max Links:"), pos );
	pos.y += text->GetSize().y + 4;
	m_MaxShowLinks = new wxSpinCtrl(this, -1, value, pos, wxSize(60,-1) );
	m_MaxShowLinks->SetRange(1, 5);

	pos.x = 250; pos.y = 15;
	m_DrcOn = new wxCheckBox(this, -1, _("Drc ON"), pos);
	m_DrcOn->SetValue(Drc_On );
	m_DrcOn->SetForegroundColour(*wxRED);

	pos.y += cb_height;
	m_ShowGlobalRatsnest = new wxCheckBox(this, -1, _("Show Ratsnest"), pos);
	m_ShowGlobalRatsnest->SetValue(g_Show_Ratsnest);

	pos.y += cb_height;
	m_ShowModuleRatsnest = new wxCheckBox(this, -1, _("Show Mod Ratsnest"), pos);
	m_ShowModuleRatsnest->SetValue(g_Show_Module_Ratsnest);

	pos.y += cb_height;
	m_TrackAutodel = new wxCheckBox(this, -1, _("Tracks Auto Del"), pos);
	m_TrackAutodel->SetValue(g_AutoDeleteOldTrack);

	pos.y += cb_height;
 	m_Track_45_Only_Ctrl = new wxCheckBox(this, -1, _("Tracks 45 Only"), pos);
	m_Track_45_Only_Ctrl->SetValue(Track_45_Only);

	pos.y += cb_height;
	m_Segments_45_Only_Ctrl = new wxCheckBox(this, -1, _("Segments 45 Only"), pos);
	m_Segments_45_Only_Ctrl->SetValue(Segments_45_Only);

	pos.y += cb_height;
	m_AutoPANOpt = new wxCheckBox(this, -1, _("Auto PAN"), pos);
	m_AutoPANOpt->SetValue(m_Parent->DrawPanel-> m_AutoPAN_Enable);
	m_AutoPANOpt->SetForegroundColour(*wxBLUE);
}



/**************************************************************************/
void  WinEDA_PcbGeneralOptionsFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
/**************************************************************************/
{
    // true is to force the frame to close
    Close(true);
}



/**************************************************************************/
void WinEDA_PcbGeneralOptionsFrame::AcceptPcbOptions(wxCommandEvent& event)
/**************************************************************************/
{
int ii;

	DisplayOpt.DisplayPolarCood =
		(m_PolarDisplay->GetSelection() == 0) ? FALSE : TRUE;
	ii = UnitMetric;
	UnitMetric = (m_BoxUnits->GetSelection() == 0)  ? 0 : 1;
	if ( ii != UnitMetric ) m_Parent->ReCreateAuxiliaryToolbar();

	g_CursorShape = m_CursorShape->GetSelection();
	g_TimeOut = 60 * m_SaveTime->GetValue();

	/* Mise a jour de la combobox d'affichage de la couche active */
 	g_DesignSettings.m_CopperLayerCount = m_LayerNumber->GetValue();
    m_Parent->ReCreateLayerBox(NULL);

	g_MaxLinksShowed = m_MaxShowLinks->GetValue();
	Drc_On = m_DrcOn->GetValue();
	if ( g_Show_Ratsnest != m_ShowGlobalRatsnest->GetValue() )
		{
		g_Show_Ratsnest = m_ShowGlobalRatsnest->GetValue();
		m_Parent->Ratsnest_On_Off(m_DC);
		}
	g_Show_Module_Ratsnest = m_ShowModuleRatsnest->GetValue();
	g_AutoDeleteOldTrack = m_TrackAutodel->GetValue();
	Segments_45_Only = m_Segments_45_Only_Ctrl->GetValue();
 	Track_45_Only = m_Track_45_Only_Ctrl->GetValue();
	m_Parent->DrawPanel->m_AutoPAN_Enable = m_AutoPANOpt->GetValue();

	EndModal(1);
}



#include "dialog_track_options.cpp"
#include "dialog_display_options.cpp"
#include "dialog_graphic_items_options.cpp"

/*****************************************************************/
void WinEDA_PcbFrame::InstallPcbOptionsFrame(const wxPoint & pos,
			wxDC * DC, int id)
/*****************************************************************/
{

	switch ( id )
		{
		case ID_PCB_TRACK_SIZE_SETUP:
			{
			WinEDA_PcbTracksDialog * OptionsFrame =
				new WinEDA_PcbTracksDialog(this, pos);
			OptionsFrame->ShowModal(); OptionsFrame->Destroy();
			}
			break;

		case ID_PCB_DRAWINGS_WIDTHS_SETUP:
			{
			WinEDA_GraphicItemsOptionsDialog * OptionsFrame =
				new WinEDA_GraphicItemsOptionsDialog(this, pos);
			OptionsFrame->ShowModal(); OptionsFrame->Destroy();
			}
			break;

		case ID_PCB_LOOK_SETUP:
			{
			WinEDA_DisplayOptionsDialog * OptionsFrame =
				new WinEDA_DisplayOptionsDialog(this, pos);
			OptionsFrame->ShowModal(); OptionsFrame->Destroy();
			}
			break;

		case ID_OPTIONS_SETUP:
			{
			WinEDA_PcbGeneralOptionsFrame * OptionsFrame =
				new WinEDA_PcbGeneralOptionsFrame(this, DC, pos);
			OptionsFrame->ShowModal(); OptionsFrame->Destroy();
			}
			break;
		}
}


/*******************************************************************/
void WinEDA_ModuleEditFrame::InstallOptionsFrame(const wxPoint & pos)
/*******************************************************************/
{
WinEDA_GraphicItemsOptionsDialog OptionsFrame (this, pos);
	OptionsFrame.ShowModal();
}

