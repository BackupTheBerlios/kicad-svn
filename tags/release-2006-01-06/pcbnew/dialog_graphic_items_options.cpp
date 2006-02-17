	/*************************************************/
	/* dialog_graphic_items_options.cpp - Gestion des Options et Reglages */
	/*************************************************/
/*
 Affichage et modifications des parametres de travail de PcbNew
 Parametres = dimensions des elemùents graphiques...
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

/**************************************************************************/
/* classe derivee pour la frame de Configuration WinEDA_GraphicItemsOptionsDialog */
/**************************************************************************/

class WinEDA_GraphicItemsOptionsDialog: public wxDialog
{
private:
	WinEDA_BasePcbFrame * m_Parent;

	WinEDA_ValueCtrl * m_OptPcbSegmWidth;
	WinEDA_ValueCtrl * m_OptPcbEdgesWidth;
	WinEDA_ValueCtrl * m_OptPcbTextWidth;
	WinEDA_ValueCtrl * m_OptPcbTextVSize;
	WinEDA_ValueCtrl * m_OptPcbTextHSize;

	WinEDA_ValueCtrl * m_OptModuleEdgesWidth;
	WinEDA_ValueCtrl * m_OptModuleTextWidth;
	WinEDA_ValueCtrl * m_OptModuleTextVSize;
	WinEDA_ValueCtrl * m_OptModuleTextHSize;

public:
	// Constructor and destructor
	WinEDA_GraphicItemsOptionsDialog(WinEDA_BasePcbFrame *parent,const wxPoint& pos);
	~WinEDA_GraphicItemsOptionsDialog(void) {};

private:
	void OnCloseWindow(wxCloseEvent & event);
	void OnQuit(wxCommandEvent& event);
	void AcceptOptions(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

};
/* Construction de la table des evenements pour WinEDA_GraphicItemsOptionsDialog */
BEGIN_EVENT_TABLE(WinEDA_GraphicItemsOptionsDialog, wxDialog)
	EVT_BUTTON(ID_ACCEPT_OPT, WinEDA_GraphicItemsOptionsDialog::AcceptOptions)
	EVT_BUTTON(ID_CANCEL_OPT, WinEDA_GraphicItemsOptionsDialog::OnQuit)
END_EVENT_TABLE()



	/********************************************/
	/* Constructeur de WinEDA_GraphicItemsOptionsDialog */
	/********************************************/

WinEDA_GraphicItemsOptionsDialog::WinEDA_GraphicItemsOptionsDialog(WinEDA_BasePcbFrame *parent,
		const wxPoint& framepos):
		wxDialog(parent, -1, _("Texts and Drawings"), framepos, wxDefaultSize,
		DIALOG_STYLE )
{
wxPoint pos;

	m_Parent = parent;
	SetFont(*g_DialogFont);

	pos.x = 5; pos.y = 5;

	/* Drawings width */
	m_OptPcbSegmWidth = new WinEDA_ValueCtrl(this, _("Segm Width"),
						g_DesignSettings.m_DrawSegmentWidth,
						UnitMetric, pos, PCB_INTERNAL_UNIT );
	/* Edges width */
	pos.y += 5 + m_OptPcbSegmWidth->GetDimension().y;
	m_OptPcbEdgesWidth = new WinEDA_ValueCtrl(this, _("Edges Width"),
						g_DesignSettings.m_EdgeSegmentWidth,
						UnitMetric, pos, PCB_INTERNAL_UNIT );

	/* Pcb Textes (Size & Width) */
	pos.y += 5 + m_OptPcbEdgesWidth->GetDimension().y;
	m_OptPcbTextWidth = new WinEDA_ValueCtrl(this, _("Text Width"),
						g_DesignSettings.m_PcbTextWidth,
						UnitMetric, pos, PCB_INTERNAL_UNIT );

	pos.y += 5 + m_OptPcbTextWidth->GetDimension().y;
	m_OptPcbTextVSize = new WinEDA_ValueCtrl(this, _("Text Size V"),
						g_DesignSettings.m_PcbTextSize.y,
						UnitMetric, pos, PCB_INTERNAL_UNIT );

	pos.y += 5 + m_OptPcbTextVSize->GetDimension().y;
	m_OptPcbTextHSize = new WinEDA_ValueCtrl(this, _("Text Size H"),
						g_DesignSettings.m_PcbTextSize.x,
						UnitMetric, pos, PCB_INTERNAL_UNIT );


	/* Modules: Edges width */
	pos.x += 200; pos.y = 5;
	m_OptModuleEdgesWidth = new WinEDA_ValueCtrl(this, _("Edges Module Width"),
						ModuleSegmentWidth,
						UnitMetric, pos, PCB_INTERNAL_UNIT );

	/* Modules: Texts: Size & width */
	pos.y += 5 + m_OptPcbEdgesWidth->GetDimension().y;
	m_OptModuleTextWidth = new WinEDA_ValueCtrl(this, _("Text Module Width"),
						ModuleTextWidth,
						UnitMetric, pos, PCB_INTERNAL_UNIT );

	pos.y += 5 + m_OptModuleTextWidth->GetDimension().y;
	m_OptModuleTextVSize = new WinEDA_ValueCtrl(this, _("Text Module Size V"),
						ModuleTextSize.y,
						UnitMetric, pos, PCB_INTERNAL_UNIT );

	pos.y += 5 + m_OptModuleTextVSize->GetDimension().y;
	m_OptModuleTextHSize = new WinEDA_ValueCtrl(this, _("Text Module Size H"),
						ModuleTextSize.x,
						UnitMetric, pos, PCB_INTERNAL_UNIT );

	pos.x = 200; pos.y +=20 + m_OptModuleTextHSize->GetDimension().y;
	wxButton * Button = new wxButton(this, ID_ACCEPT_OPT, _("Ok"), pos);
	Button->SetForegroundColour(*wxRED);
	pos.x += Button->GetSize().x  + 5;
	Button = new wxButton(this, ID_CANCEL_OPT, _("Cancel"), pos);
	Button->SetForegroundColour(*wxBLUE);

	/* Size adjust */
	pos.y +=10 + Button->GetSize().y;
	SetClientSize(wxSize(400, pos.y));
}


/**********************************************************************/
void  WinEDA_GraphicItemsOptionsDialog::OnQuit(wxCommandEvent& WXUNUSED(event))
/**********************************************************************/
{
    // true is to force the frame to close
    Close(true);
}



/*********************************************************************/
void WinEDA_GraphicItemsOptionsDialog::AcceptOptions(wxCommandEvent& event)
/*********************************************************************/
{
	g_DesignSettings.m_DrawSegmentWidth = m_OptPcbSegmWidth->GetValue();
	g_DesignSettings.m_EdgeSegmentWidth = m_OptPcbEdgesWidth->GetValue();
	g_DesignSettings.m_PcbTextWidth = m_OptPcbTextWidth->GetValue();
	g_DesignSettings.m_PcbTextSize.y = m_OptPcbTextVSize->GetValue();
	g_DesignSettings.m_PcbTextSize.x = m_OptPcbTextHSize->GetValue();

	ModuleSegmentWidth = m_OptModuleEdgesWidth->GetValue();
	ModuleTextWidth = m_OptModuleTextWidth->GetValue();
	ModuleTextSize.y = m_OptModuleTextVSize->GetValue();
	ModuleTextSize.x = m_OptModuleTextHSize->GetValue();

	EndModal(1);
}

