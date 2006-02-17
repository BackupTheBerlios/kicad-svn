	/******************************************************/
	/* editpads.cpp: Pad editing functions and dialog box */
	/******************************************************/

#include "fctsys.h"
#include "gr_basic.h"
#include "common.h"
#include "pcbnew.h"
#include "autorout.h"
#include "trigo.h"

#include "drag.h"

#include "protos.h"

/* Routines Locales */

/* Variables locales */
static wxString Current_PadNetName;


#define NBORIENT 5
static wxString orient_list[NBORIENT] =
{
 wxT("0"), wxT("90"), wxT("-90"), wxT("180"), wxT("User")
};


#define NBSHAPES 4
int CodeShape[NBSHAPES] = /* forme des pads  */
{
	CIRCLE, OVALE, RECT, TRAPEZE
};


#define NBTYPES 5
static wxString type_list[NBTYPES] =	// Type des pads
{
	wxT("Standard"), wxT("SMD"), wxT("Conn"), wxT("Hole"), wxT("Mechanical")
};

int CodeType[NBTYPES] =
{ STANDARD, SMD, CONN, P_HOLE, MECA
};


static long Std_Pad_Layers[NBTYPES] =
{
	ALL_CU_LAYERS|SILKSCREEN_LAYER_CMP|SOLDERMASK_LAYER_CU|SOLDERMASK_LAYER_CMP,
	CMP_LAYER|SOLDERPASTE_LAYER_CMP|SOLDERMASK_LAYER_CMP,
	CMP_LAYER|SOLDERMASK_LAYER_CMP,
	ALL_CU_LAYERS|SILKSCREEN_LAYER_CU|SILKSCREEN_LAYER_CMP|
		SOLDERMASK_LAYER_CU|SOLDERMASK_LAYER_CMP,
	ALL_CU_LAYERS|SILKSCREEN_LAYER_CU|SILKSCREEN_LAYER_CMP|
		SOLDERMASK_LAYER_CU|SOLDERMASK_LAYER_CMP
};

enum id_pad_properties
{
	ID_ACCEPT_PAD_PROPERTIES = 1900,
	ID_CLOSE_PAD_PROPERTIES,
	ID_LISTBOX_TYPE_PAD,
	ID_LISTBOX_SHAPE_PAD,
	ID_LISTBOX_ORIENT_PAD
};


	/************************************/
	/* class WinEDA_PadPropertiesFrame */
	/************************************/

class WinEDA_PadPropertiesFrame: public wxDialog
{
private:

	WinEDA_BasePcbFrame * m_Parent;
	wxDC * m_DC;
	D_PAD * CurrentPad;
	WinEDA_SizeCtrl * m_PadSizeCtrl;
	WinEDA_SizeCtrl * m_PadDeltaSizeCtrl;
	WinEDA_SizeCtrl * m_PadOffsetCtrl;
	WinEDA_ValueCtrl * m_PadDrillCtrl;
	WinEDA_EnterText * m_PadNumCtrl;
	WinEDA_EnterText * m_PadNetNameCtrl;
	wxRadioBox * m_PadType;
	wxRadioBox * m_PadShape;
	wxRadioBox * m_PadOrient;	// Pour selection rapide des oientations
	WinEDA_ValueCtrl * m_PadOrientCtrl;	// Pour orientation a 0.1 degre pres
	wxCheckBox * m_PadLayerCu;
	wxCheckBox * m_PadLayerCmp;
	wxCheckBox * m_PadLayerAdhCmp;
	wxCheckBox * m_PadLayerAdhCu;
	wxCheckBox * m_PadLayerPateCmp;
	wxCheckBox * m_PadLayerPateCu;
	wxCheckBox * m_PadLayerSilkCmp;
	wxCheckBox * m_PadLayerSilkCu;
	wxCheckBox * m_PadLayerMaskCmp;
	wxCheckBox * m_PadLayerMaskCu;
	wxCheckBox * m_PadLayerECO1;
	wxCheckBox * m_PadLayerECO2;
	wxCheckBox * m_PadLayerDraft;

public:
	// Constructor and destructor
	WinEDA_PadPropertiesFrame(WinEDA_BasePcbFrame *parent,
							D_PAD * Pad, wxDC * DC, const wxPoint & pos);
	~WinEDA_PadPropertiesFrame(void)
		{
		}

private:
	void OnQuit(wxCommandEvent& event);
	void PadPropertiesAccept(wxCommandEvent& event);
	void PadTypeSelected(void);
	void PadTypeSelectedEvent(wxCommandEvent& event);
	void PadOrientEvent(wxCommandEvent& event);
	void SetPadLayersList(long layer_mask);

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WinEDA_PadPropertiesFrame, wxDialog)
	EVT_BUTTON(ID_ACCEPT_PAD_PROPERTIES, WinEDA_PadPropertiesFrame::PadPropertiesAccept)
	EVT_BUTTON(ID_CLOSE_PAD_PROPERTIES, WinEDA_PadPropertiesFrame::OnQuit)
	EVT_RADIOBOX(ID_LISTBOX_TYPE_PAD, WinEDA_PadPropertiesFrame::PadTypeSelectedEvent)
	EVT_RADIOBOX(ID_LISTBOX_ORIENT_PAD, WinEDA_PadPropertiesFrame::PadOrientEvent)
END_EVENT_TABLE()


/*************************************************************/
void WinEDA_BasePcbFrame::InstallPadOptionsFrame(D_PAD * Pad,
					wxDC * DC, const wxPoint & pos)
/*************************************************************/
{
	DrawPanel->m_IgnoreMouseEvents = TRUE;
	WinEDA_PadPropertiesFrame * frame = new WinEDA_PadPropertiesFrame(this,
					 Pad, DC, pos);
	frame->ShowModal(); frame->Destroy();
	DrawPanel->m_IgnoreMouseEvents = FALSE;
}

#define H_SIZE 450
#define V_SIZE 520
/*********************************************************************************/
WinEDA_PadPropertiesFrame::WinEDA_PadPropertiesFrame(WinEDA_BasePcbFrame *parent,
				D_PAD * Pad,wxDC * DC,
				const wxPoint & framepos):
		wxDialog(parent, -1, _("Pad properties"), framepos, wxSize(H_SIZE, V_SIZE),
			DIALOG_STYLE)
/*********************************************************************************/
{
wxPoint pos;
int tmp, ii, xx, yy;
wxString number;
wxButton * Button;
MODULE * Module = NULL;

	m_Parent = parent;
	SetFont(*g_DialogFont);
	m_DC = DC;
	Centre();

	CurrentPad = Pad;

	if ( CurrentPad )
	{
		Current_PadNetName = CurrentPad->m_Netname;
		g_Current_PadName = CurrentPad->ReturnStringPadName();
		Module = (MODULE*) CurrentPad->m_Parent;
	}

	/* Creation des boutons de commande */
	pos.x = 170; pos.y = 10;
	Button = new wxButton(this, ID_ACCEPT_PAD_PROPERTIES,
						_("Ok"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.x += Button->GetDefaultSize().x + 20;
	Button = new wxButton(this, ID_CLOSE_PAD_PROPERTIES,
						_("Cancel"), pos);
	Button->SetForegroundColour(*wxBLUE);

	pos.x = 5; pos.y = 20;
	m_PadNumCtrl = new WinEDA_EnterText(this, _("Pad Num :"),
					g_Current_PadName, pos, wxSize(80, -1));

	pos.y += m_PadNumCtrl->GetDimension().y + 15;
	m_PadNetNameCtrl = new WinEDA_EnterText(this, _("Pad Net Name :"),
					Current_PadNetName, pos, wxSize(120, -1));

	pos.y += m_PadNetNameCtrl->GetDimension().y + 15;
	m_PadSizeCtrl = new WinEDA_SizeCtrl(this, _("Pad Size"),
			CurrentPad ? CurrentPad->m_Size : g_Pad_Master.m_Size,
			UnitMetric, pos, m_Parent->m_InternalUnits);

	pos.y += m_PadSizeCtrl->GetDimension().y + 10;
	m_PadDeltaSizeCtrl = new WinEDA_SizeCtrl(this, _("Delta"),
			CurrentPad ? CurrentPad->m_DeltaSize : g_Pad_Master.m_DeltaSize,
			UnitMetric, pos, m_Parent->m_InternalUnits);

	pos.y += m_PadDeltaSizeCtrl->GetDimension().y + 20;
	m_PadDrillCtrl = new WinEDA_ValueCtrl(this, _("Pad Drill"),
			CurrentPad ? CurrentPad->m_Drill : g_Pad_Master.m_Drill,
			UnitMetric, pos, m_Parent->m_InternalUnits );

	pos.y += m_PadDrillCtrl->GetDimension().y + 10;
	m_PadOffsetCtrl = new WinEDA_SizeCtrl(this, _("Offset"),
			CurrentPad ? CurrentPad->m_Offset : g_Pad_Master.m_Offset,
			UnitMetric, pos, m_Parent->m_InternalUnits);

	pos.y += m_PadOffsetCtrl->GetDimension().y + 10;
	if ( CurrentPad )
		{
		tmp = CurrentPad->m_Orient - Module->m_Orient;
		}
	else tmp = g_Pad_Master.m_Orient;
	m_PadOrientCtrl = new WinEDA_ValueCtrl(this, _("Pad Orient (0.1 deg)"),
			tmp, 2, pos, 1);


	// Pad Orient
	pos.x += 130; pos.y = 50;
	m_PadOrient = new wxRadioBox(this, ID_LISTBOX_ORIENT_PAD, _("Pad Orient:"),
				pos, wxSize(-1,-1), NBORIENT, orient_list, 1);
	switch ( tmp )
	{
		case 0:
			m_PadOrient->SetSelection(0);
			m_PadOrientCtrl->Enable(FALSE);
			break;

		case -2700:
		case 900:
			m_PadOrient->SetSelection(1);
			m_PadOrientCtrl->Enable(FALSE);
			break;

		case -900:
		case 2700:
			m_PadOrient->SetSelection(2);
			m_PadOrientCtrl->Enable(FALSE);
			break;

		case 1800:
		case -1800:
			m_PadOrient->SetSelection(3);
			m_PadOrientCtrl->Enable(FALSE);
			break;

		default:
			m_PadOrient->SetSelection(4);
			break;
	}

	m_PadOrient->GetSize(&xx, &yy);
	pos.y += yy + 10;
wxString shape_list[NBSHAPES] =
	{
	 _("Circle"), _("Oval"), _("Rect"), _("Trapeze")
	};
	m_PadShape = new wxRadioBox(this, ID_LISTBOX_SHAPE_PAD, _("Pad Shape:"),
				pos, wxSize(-1,-1),
				NBSHAPES, shape_list, 1);
	tmp = CurrentPad ? CurrentPad->m_PadShape : g_Pad_Master.m_PadShape;
	switch ( tmp )
		{
		case CIRCLE:
			m_PadShape->SetSelection(0);
			break;

		case OVALE:
			m_PadShape->SetSelection(1);
			break;

		case RECT:
			m_PadShape->SetSelection(2);
			break;

		case TRAPEZE:
			m_PadShape->SetSelection(3);
			break;

		case SPECIAL_PAD:
			m_PadShape->SetSelection(4);
			break;
		}

	// Selection du type
	m_PadShape->GetSize(&xx, &yy);
	pos.y += yy + 10;
	m_PadType = new wxRadioBox(this, ID_LISTBOX_TYPE_PAD, _("Pad Type:"),
				pos, wxSize(-1,-1),
				NBTYPES, type_list, 1);
	tmp = CurrentPad ? CurrentPad->m_Attribut : g_Pad_Master.m_Attribut;
	m_PadType->SetSelection( 0 );
	for ( ii = 0; ii < NBTYPES; ii++ )
	{
		if ( CodeType[ii] == tmp )
		{
			m_PadType->SetSelection( ii ); break ;
		}
	}

	// Selection des couches cuivre :
	pos.x += 130; pos.y = 80;
	m_PadLayerCu = new wxCheckBox(this, -1, wxT("Copper layer"), pos);

	pos.y += 20;
	m_PadLayerCmp = new wxCheckBox(this, -1, wxT("Comp layer"), pos);

	pos.y += 25;
	m_PadLayerAdhCmp = new wxCheckBox(this, -1, wxT("Adhesive Cmp"), pos);

	pos.y += 20;
	m_PadLayerAdhCu = new wxCheckBox(this, -1, wxT("Adhesive Copper"), pos);

	pos.y += 20;
	m_PadLayerPateCmp = new wxCheckBox(this, -1, wxT("Solder paste Cmp"), pos);

	pos.y += 20;
	m_PadLayerPateCu = new wxCheckBox(this, -1, wxT("Solder paste Copper"), pos);

	pos.y += 20;
	m_PadLayerSilkCmp = new wxCheckBox(this, -1, wxT("Silkscreen Cmp"), pos);

	pos.y += 20;
	m_PadLayerSilkCu = new wxCheckBox(this, -1, wxT("Silkscreen Copper"), pos);

	pos.y += 20;
	m_PadLayerMaskCmp = new wxCheckBox(this, -1, wxT("Solder mask Cmp"), pos);

	pos.y += 20;
	m_PadLayerMaskCu = new wxCheckBox(this, -1, wxT("Solder mask Copper"), pos);

	pos.y += 20;
	m_PadLayerECO1 = new wxCheckBox(this, -1, wxT("E.C.O.1 layer"), pos);

	pos.y += 20;
	m_PadLayerECO2 = new wxCheckBox(this, -1, wxT("E.C.O.2 layer"), pos);

	pos.y += 20;
	m_PadLayerDraft = new wxCheckBox(this, -1, wxT("Draft layer"), pos);

	if ( CurrentPad ) SetPadLayersList(CurrentPad->m_Masque_Layer);
	else PadTypeSelected();
}

/**********************************************************************/
void  WinEDA_PadPropertiesFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
/**********************************************************************/
{
    // true is to force the frame to close
    Close(true);
}



/*******************************************************************/
void WinEDA_PadPropertiesFrame::PadOrientEvent(wxCommandEvent& event)
/********************************************************************/
{
	switch ( m_PadOrient->GetSelection() )
		{
		case 0:
			m_PadOrientCtrl->SetValue(0);
			m_PadOrientCtrl->Enable(FALSE);
			break;

		case 1:
			m_PadOrientCtrl->SetValue(900);
			m_PadOrientCtrl->Enable(FALSE);
			break;

		case 2:
			m_PadOrientCtrl->SetValue(2700);
			m_PadOrientCtrl->Enable(FALSE);
			break;

		case 3:
			m_PadOrientCtrl->SetValue(1800);
			m_PadOrientCtrl->Enable(FALSE);
			break;

		default:
			m_PadOrientCtrl->Enable(TRUE);
			break;
		}

}

/**************************************************************************/
void WinEDA_PadPropertiesFrame::PadTypeSelectedEvent(wxCommandEvent& event)
/**************************************************************************/
/* calcule un layer_mask type selon la selection du type du pad
*/
{
	PadTypeSelected();
}

void WinEDA_PadPropertiesFrame::PadTypeSelected(void)
{
long layer_mask;
int ii;

	ii = m_PadType->GetSelection();
	if ( (ii < 0) || ( ii >= NBTYPES) ) ii = 0;

	layer_mask = Std_Pad_Layers[ii];
	SetPadLayersList(layer_mask);
}

/****************************************************************/
void WinEDA_PadPropertiesFrame::SetPadLayersList(long layer_mask)
/****************************************************************/
/* Met a jour l'etat des CheckBoxes de la liste des layers actives,
données bit a bit dans layer_mask
*/
{
	if( layer_mask & CUIVRE_LAYER ) m_PadLayerCu->SetValue(TRUE);
	else  m_PadLayerCu->SetValue(FALSE);

	if( layer_mask & CMP_LAYER ) m_PadLayerCmp->SetValue(TRUE);
	else  m_PadLayerCmp->SetValue(FALSE);

	if( layer_mask & ADHESIVE_LAYER_CMP ) m_PadLayerAdhCmp->SetValue(TRUE);
	else  m_PadLayerAdhCmp->SetValue(FALSE);

	if( layer_mask & ADHESIVE_LAYER_CU ) m_PadLayerAdhCu->SetValue(TRUE);
	else  m_PadLayerAdhCu->SetValue(FALSE);

	if( layer_mask & SOLDERPASTE_LAYER_CMP ) m_PadLayerPateCmp->SetValue(TRUE);
	else  m_PadLayerPateCmp->SetValue(FALSE);

	if( layer_mask & SOLDERPASTE_LAYER_CU ) m_PadLayerPateCu->SetValue(TRUE);
	else  m_PadLayerPateCu->SetValue(FALSE);

	if( layer_mask & SILKSCREEN_LAYER_CMP ) m_PadLayerSilkCmp->SetValue(TRUE);
	else  m_PadLayerSilkCmp->SetValue(FALSE);

	if( layer_mask & SILKSCREEN_LAYER_CU ) m_PadLayerSilkCu->SetValue(TRUE);
	else  m_PadLayerSilkCu->SetValue(FALSE);

	if( layer_mask & SOLDERMASK_LAYER_CMP ) m_PadLayerMaskCmp->SetValue(TRUE);
	else  m_PadLayerMaskCmp->SetValue(FALSE);

	if( layer_mask & SOLDERMASK_LAYER_CU ) m_PadLayerMaskCu->SetValue(TRUE);
	else  m_PadLayerMaskCu->SetValue(FALSE);

	if( layer_mask & ECO1_LAYER ) m_PadLayerECO1->SetValue(TRUE);
	else  m_PadLayerECO1->SetValue(FALSE);

	if( layer_mask & ECO2_LAYER ) m_PadLayerECO2->SetValue(TRUE);
	else m_PadLayerECO2->SetValue(FALSE);

	if( layer_mask & DRAW_LAYER ) m_PadLayerDraft->SetValue(TRUE);
	else m_PadLayerDraft->SetValue(FALSE);
}


/*************************************************************************/
void WinEDA_PadPropertiesFrame::PadPropertiesAccept(wxCommandEvent& event)
/*************************************************************************/
/* Met a jour les differents parametres pour le composant en cours d'édition
*/
{
long PadLayerMask;

	g_Pad_Master.m_Attribut = CodeType[m_PadType->GetSelection()];
	g_Pad_Master.m_PadShape = CodeShape[m_PadShape->GetSelection()];
	g_Pad_Master.m_Size = m_PadSizeCtrl->GetCoord();
	g_Pad_Master.m_DeltaSize = m_PadDeltaSizeCtrl->GetCoord();
	g_Pad_Master.m_Offset = m_PadOffsetCtrl->GetCoord();
	g_Pad_Master.m_Drill = m_PadDrillCtrl->GetValue();
	g_Pad_Master.m_Orient = m_PadOrientCtrl->GetValue();
	g_Current_PadName = m_PadNumCtrl->GetData().Left(4);
	Current_PadNetName = m_PadNetNameCtrl->GetData();

	PadLayerMask = 0;
	if( m_PadLayerCu->GetValue() ) PadLayerMask |= CUIVRE_LAYER;
	if( m_PadLayerCmp->GetValue() ) PadLayerMask |= CMP_LAYER;
	if ( (PadLayerMask & (CUIVRE_LAYER|CMP_LAYER)) == (CUIVRE_LAYER|CMP_LAYER) )
		PadLayerMask |= ALL_CU_LAYERS;
	if( m_PadLayerAdhCmp->GetValue() ) PadLayerMask |= ADHESIVE_LAYER_CMP;
	if( m_PadLayerAdhCu->GetValue() ) PadLayerMask |= ADHESIVE_LAYER_CU;
	if( m_PadLayerPateCmp->GetValue() ) PadLayerMask |= SOLDERPASTE_LAYER_CMP;
	if( m_PadLayerPateCu->GetValue() ) PadLayerMask |= SOLDERPASTE_LAYER_CU;
	if( m_PadLayerSilkCmp->GetValue() ) PadLayerMask |= SILKSCREEN_LAYER_CMP;
	if( m_PadLayerSilkCu->GetValue() ) PadLayerMask |= SILKSCREEN_LAYER_CU;
	if( m_PadLayerMaskCmp->GetValue() ) PadLayerMask |= SOLDERMASK_LAYER_CMP;
	if( m_PadLayerMaskCu->GetValue() ) PadLayerMask |= SOLDERMASK_LAYER_CU;
	if( m_PadLayerECO1->GetValue() ) PadLayerMask |= ECO1_LAYER;
	if( m_PadLayerECO2->GetValue() ) PadLayerMask |= ECO2_LAYER;
	if( m_PadLayerDraft->GetValue() ) PadLayerMask |= DRAW_LAYER;

	g_Pad_Master.m_Masque_Layer = PadLayerMask;

	if ( CurrentPad )   // Set Pad Name & Num
	{
		m_Parent->SaveCopyInUndoList();
		MODULE * Module;
		Module = (MODULE*) CurrentPad->m_Parent;
		Module->m_LastEdit_Time = time(NULL);

		if ( m_DC ) CurrentPad->Draw(m_Parent->DrawPanel, m_DC, wxPoint(0,0), GR_XOR);
		CurrentPad->m_PadShape = g_Pad_Master.m_PadShape;
		CurrentPad->m_Attribut = g_Pad_Master.m_Attribut;
		CurrentPad->m_Size = g_Pad_Master.m_Size;
		CurrentPad->m_DeltaSize = g_Pad_Master.m_DeltaSize;
		CurrentPad->m_Drill = g_Pad_Master.m_Drill;
		CurrentPad->m_Offset = g_Pad_Master.m_Offset;
		CurrentPad->m_Masque_Layer = g_Pad_Master.m_Masque_Layer;
		CurrentPad->m_Orient = g_Pad_Master.m_Orient + Module->m_Orient;
		CurrentPad->SetPadName(g_Current_PadName);
		CurrentPad->m_Netname = Current_PadNetName;
		if ( Current_PadNetName.IsEmpty() ) CurrentPad->m_NetCode = 0;

		switch ( CurrentPad->m_PadShape )
		{
			case CIRCLE:
				CurrentPad->m_DeltaSize = wxSize(0,0);
				CurrentPad->m_Size.y = CurrentPad->m_Size.x;
				break;

			case RECT:
				CurrentPad->m_DeltaSize = wxSize(0,0);
				break;

			case OVALE:
				CurrentPad->m_DeltaSize = wxSize(0,0);
				break;

			case TRAPEZE:
				break;

			case SPECIAL_PAD:
				break;
		}

		switch ( CurrentPad->m_Attribut )
		{
			case STANDARD:
				break;

			case CONN:
			case SMD:
				CurrentPad->m_Offset = wxSize(0,0);
				CurrentPad->m_Drill = 0;
				break;

			case P_HOLE:
			case MECA:
				break;
		}

		CurrentPad->ComputeRayon();

		Module->Set_Rectangle_Encadrement();
		CurrentPad->Display_Infos(m_Parent);
		if ( m_DC ) CurrentPad->Draw(m_Parent->DrawPanel, m_DC, wxPoint(0,0), GR_OR);
		m_Parent->GetScreen()->SetModify();
	}

	Close();
}


