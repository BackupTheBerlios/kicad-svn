	/**********************************************/
	/* PCBNEW : Routines d'initialisation globale */
	/******* Fichier INITPCB.C ********************/
	/**********************************************/


#include "fctsys.h"

#include "common.h"
#include "pcbnew.h"
#include "autorout.h"

#include "protos.h"

/* Routines Locales */


enum id_optpcb
{
	ID_ACCEPT_OPT = 1000,
	ID_CANCEL_OPT
};
	/*************************************************/
	/* classe derivee pour la frame de Configuration */
	/*************************************************/

class WinEDA_PcbGlobalDeleteFrame: public wxDialog
{
private:

	WinEDA_PcbFrame * m_Parent;
	wxCheckBox * m_DelZones;
	wxCheckBox * m_DelTexts;
	wxCheckBox * m_DelEdges;
	wxCheckBox * m_DelDrawings;
	wxCheckBox * m_DelModules;
	wxCheckBox * m_DelTracks;
	wxCheckBox * m_DelMarkers;
	wxCheckBox * m_DelAlls;
	wxCheckBox * m_TrackFilterAR;
	wxCheckBox * m_TrackFilterLocked;


	// Constructor and destructor
public:
	WinEDA_PcbGlobalDeleteFrame(WinEDA_PcbFrame *parent,const wxPoint& pos);
	~WinEDA_PcbGlobalDeleteFrame(void) {};

private:
	void AcceptPcbDelete(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

};
/* Construction de la table des evenements pour WinEDA_PcbGlobalDeleteFrame */
BEGIN_EVENT_TABLE(WinEDA_PcbGlobalDeleteFrame, wxDialog)
	EVT_BUTTON(ID_ACCEPT_OPT, WinEDA_PcbGlobalDeleteFrame::AcceptPcbDelete)
	EVT_BUTTON(ID_CANCEL_OPT, WinEDA_PcbGlobalDeleteFrame::OnQuit)
END_EVENT_TABLE()


void WinEDA_PcbFrame::InstallPcbGlobalDeleteFrame(const wxPoint & pos)
{
WinEDA_PcbGlobalDeleteFrame * frame =
		new WinEDA_PcbGlobalDeleteFrame(this, pos);
	frame->ShowModal(); frame->Destroy();
}

	/*************************************************/
	/* Constructeur de WinEDA_PcbGlobalDeleteFrame */
	/************************************************/

WinEDA_PcbGlobalDeleteFrame::WinEDA_PcbGlobalDeleteFrame(WinEDA_PcbFrame *parent,
		const wxPoint& framepos):
		wxDialog(parent, -1, _("General Delete"), framepos, wxSize(450, 200),
				 DIALOG_STYLE)
{
wxPoint pos;
int ii, jj;

	m_Parent = parent;
	SetFont(*g_DialogFont);

	pos.x = 200; pos.y = 140;
	wxButton * Button = new wxButton(this, ID_ACCEPT_OPT, _("Accept"), pos);
	pos.x += Button->GetSize().x  + 5;
	Button->SetForegroundColour(*wxRED);
	Button = new wxButton(this, ID_CANCEL_OPT, _("Cancel"), pos);
	Button->SetForegroundColour(*wxBLUE);

	/* Display Selection affichage des coordonnées polaires */
	pos.x = 5; pos.y = 5;
	m_DelZones = new wxCheckBox(this, -1, _("Delete Zones"), pos);

	m_DelZones->GetSize(&jj, &ii);
	pos.y += ii + 5;
	m_DelTexts = new wxCheckBox(this, -1, _("Delete Texts"), pos);

	m_DelTexts->GetSize(&jj, &ii);
	pos.y += ii + 5;
	m_DelEdges = new wxCheckBox(this, -1, _("Delete Edges"), pos);

	m_DelEdges->GetSize(&jj, &ii);
	pos.y += ii + 5;
	m_DelDrawings = new wxCheckBox(this, -1, _("Delete Drawings"), pos);

	m_DelEdges->GetSize(&jj, &ii);
	pos.y += ii + 5;
	m_DelModules = new wxCheckBox(this, -1, _("Delete Modules"), pos);

	m_DelModules->GetSize(&jj, &ii);
	pos.y += ii + 5;
	m_DelTracks = new wxCheckBox(this, -1, _("Delete Tracks"), pos);

	m_DelTracks->GetSize(&jj, &ii);
	pos.y += ii + 5;
	m_DelMarkers = new wxCheckBox(this, -1, _("Delete Markers"), pos);

	m_DelMarkers->GetSize(&jj, &ii);
	pos.y += ii + 5;
	m_DelAlls = new wxCheckBox(this, -1, _("Clear Board"), pos);

	pos.x += 200; pos.y = 20;
	new wxStaticBox(this, -1, _("Track Filter"), pos, wxSize(170, 65));
	pos.x += 10; pos.y += 20;
	m_TrackFilterAR = new wxCheckBox(this, -1, _("Include AutoRouted Tracks"), pos);
	m_TrackFilterAR->SetValue(TRUE);

	m_TrackFilterAR->GetSize(&jj, &ii);
	pos.y += ii + 5;
	m_TrackFilterLocked = new wxCheckBox(this, -1, _("Include Locked Tracks"), pos);
}

/**********************************************************************/
void WinEDA_PcbGlobalDeleteFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
/**********************************************************************/
{
    // true is to force the frame to close
    Close(true);
}


/***********************************************************************/
void WinEDA_PcbGlobalDeleteFrame::AcceptPcbDelete(wxCommandEvent& event)
/***********************************************************************/
{
int track_mask;
bool redraw = FALSE;
wxClientDC dc(m_Parent->DrawPanel);

	m_Parent->DrawPanel->PrepareGraphicContext(&dc);

	if ( m_DelAlls->GetValue() )
		{
		m_Parent->Clear_Pcb(&dc, TRUE);
		redraw = TRUE;
		}

	else
		{
		if ( m_DelZones->GetValue() )
			{
			m_Parent->Erase_Zones(&dc, TRUE);
			redraw = TRUE;
			}

		if ( m_DelTexts->GetValue() )
			{
			m_Parent->Erase_Textes_Pcb(&dc, TRUE);
			redraw = TRUE;
			}

		if ( m_DelEdges->GetValue() )
			{
			m_Parent->Erase_Segments_Pcb(&dc, TRUE, TRUE);
			redraw = TRUE;
			}

		if ( m_DelDrawings->GetValue() )
			{
			m_Parent->Erase_Segments_Pcb(&dc, FALSE, TRUE);
			redraw = TRUE;
			}

		if ( m_DelModules->GetValue() )
			{
			m_Parent->Erase_Modules(&dc, TRUE);
			redraw = TRUE;
			}

		if ( m_DelTracks->GetValue() )
			{
			{
			track_mask = 0;
			if ( ! m_TrackFilterLocked->GetValue() ) track_mask |= SEGM_FIXE;
			if ( ! m_TrackFilterAR->GetValue() ) track_mask |= SEGM_AR;

			m_Parent->Erase_Pistes(&dc, track_mask, TRUE);
			redraw = TRUE;
			}
			}

		if ( m_DelMarkers->GetValue() )
			{
			m_Parent->Erase_Marqueurs(&dc, FALSE);
			redraw = TRUE;
			}

		}

	if ( redraw )
		{
		m_Parent->GetScreen()->m_CurrentItem = NULL;
		m_Parent->ReDrawPanel();
		}

	EndModal(1);
}




/*********************************************************/
bool WinEDA_BasePcbFrame::Clear_Pcb(wxDC * DC, bool query)
/*********************************************************/
/* Realise les init des pointeurs et variables
	Si Item == NULL, il n'y aura pas de confirmation
*/
{

	if( m_Pcb == NULL ) return FALSE;

	if ( query && GetScreen()->IsModify() )
		{
		if (m_Pcb->m_Drawings ||m_Pcb->m_Modules ||
			m_Pcb->m_Track || m_Pcb->m_Zone )
			{
			if( ! IsOK(this, _("Current Board will be lost ?")) ) return FALSE;
			}
		}

	/* Suppression des listes chainees */
	DeleteStructList(m_Pcb->m_Equipots);
	m_Pcb->m_Equipots = NULL;

	DeleteStructList(m_Pcb->m_Drawings);
	m_Pcb->m_Drawings = NULL;

	DeleteStructList(m_Pcb->m_Modules);
	m_Pcb->m_Modules = NULL;

	DeleteStructList(m_Pcb->m_Track);
	m_Pcb->m_Track = NULL;
	m_Pcb->m_NbSegmTrack = 0;

	DeleteStructList(m_Pcb->m_Zone);
	m_Pcb->m_Zone = NULL;
	m_Pcb->m_NbSegmZone = 0;
	DelLimitesZone(DC, FALSE);

	for ( ; g_UnDeleteStackPtr != 0; )
		{
		g_UnDeleteStackPtr--;
		DeleteStructList(g_UnDeleteStack[g_UnDeleteStackPtr]);
		}

	/* init pointeurs  et variables */
	GetScreen()->m_FileName.Empty();
	memset (buf_work, 0, BUFMEMSIZE);
	adr_lowmem = adr_max = buf_work;

	if(m_Pcb->m_Pads)
		{
		MyFree(m_Pcb->m_Pads);
		m_Pcb->m_Pads = NULL;
		}
	if( m_Pcb->m_Ratsnest ) MyFree(m_Pcb->m_Ratsnest);
	if( m_Pcb->m_LocalRatsnest ) MyFree(m_Pcb->m_LocalRatsnest);
	m_Pcb->m_Ratsnest = NULL;
	m_Pcb->m_LocalRatsnest = NULL;

	/* remise a 0 ou a une valeur initiale des variables de la structure */
	m_Pcb->m_BoundaryBox.SetOrigin(wxPoint(0,0));
	m_Pcb->m_BoundaryBox.SetSize(wxSize(0,0));
	m_Pcb->m_Status_Pcb = 0;
	m_Pcb->m_NbLoclinks = 0;
	m_Pcb->m_NbLinks = 0;
	m_Pcb->m_NbPads = 0;
	m_Pcb->m_NbNets = 0;
	m_Pcb->m_NbNodes = 0;
	m_Pcb->m_NbNoconnect = 0;
	m_Pcb->m_NbSegmTrack = 0;
	m_Pcb->m_NbSegmZone = 0;
	GetScreen()->m_CurrentItem = NULL;

	/* Init parametres de gestion */
	GetScreen()->Init();

	g_HightLigt_Status = 0 ;

	for(int ii = 1; ii < HIST0RY_NUMBER; ii++)
		{
		g_DesignSettings.m_ViaSizeHistory[ii] =
			g_DesignSettings.m_TrackWidhtHistory[ii] = 0;
		}
	g_DesignSettings.m_TrackWidhtHistory[0] = g_DesignSettings.m_CurrentTrackWidth;
	g_DesignSettings.m_ViaSizeHistory[0] = g_DesignSettings.m_CurrentViaSize;

	Zoom_Automatique(TRUE);
	DrawPanel->Refresh(TRUE);

	return TRUE;
}

/************************************************************/
void WinEDA_PcbFrame::Erase_Zones(wxDC * DC, bool query)
/************************************************************/
{

	if( query && !IsOK(this, _("Delete Zones ?") ) ) return ;

	if( m_Pcb->m_Zone )
		{
		while(m_Pcb->m_Zone ) DeleteStructure(m_Pcb->m_Zone);
		m_Pcb->m_NbSegmZone = 0;
		}
	DelLimitesZone(DC, FALSE);

	GetScreen()->SetModify();
}


/*****************************************************************************/
void WinEDA_PcbFrame::Erase_Segments_Pcb(wxDC * DC, bool is_edges, bool query)
/*****************************************************************************/
{
EDA_BaseStruct * PtStruct, *PtNext;
int masque_layer = (~EDGE_LAYER) & 0x1FFF0000;

	if( is_edges )
		{
		masque_layer = EDGE_LAYER;
		if ( query && ! IsOK(this, _("Delete Board edges ?") ) ) return;
		}
	else
		{
		if ( query && ! IsOK(this, _("Delete draw items?") ) ) return;
		}

	PtStruct = (EDA_BaseStruct *) m_Pcb->m_Drawings;
	for( ; PtStruct != NULL; PtStruct = PtNext)
		{
		PtNext = PtStruct->Pnext;
		switch( PtStruct->m_StructType )
			{
			case TYPEDRAWSEGMENT:
				if(g_TabOneLayerMask[((DRAWSEGMENT*)PtStruct)->m_Layer] & masque_layer)
					 DeleteStructure(PtStruct);
				break;

			case TYPETEXTE:
				if(g_TabOneLayerMask[((TEXTE_PCB*)PtStruct)->m_Layer] & masque_layer)
					DeleteStructure(PtStruct);
				break;

			case TYPECOTATION:
				if(g_TabOneLayerMask[((COTATION*)PtStruct)->m_Layer] & masque_layer)
					DeleteStructure(PtStruct);
				break;

			case TYPEMIRE:
				if(g_TabOneLayerMask[((MIREPCB*)PtStruct)->m_Layer] & masque_layer)
					DeleteStructure(PtStruct);
				break;
			default:
				DisplayError(this, wxT("Unknown/unexpected Draw Type"));
				break;
			}
		}

	GetScreen()->SetModify();
}


/**************************************************************************/
void WinEDA_PcbFrame::Erase_Pistes(wxDC * DC, int masque_type, bool query)
/**************************************************************************/
/* Efface les segments de piste, selon les autorisations affichees
masque_type = masque des options de selection:
SEGM_FIXE, SEGM_AR
	Si un des bits est a 1, il n'y a pas effacement du segment de meme bit a 1
*/
{
TRACK * pt_segm;
EDA_BaseStruct * PtNext;

	if( query && ! IsOK(this, _("Delete Tracks?") ) ) return;

	/* Marquage des pistes a effacer */
	for( pt_segm = m_Pcb->m_Track; pt_segm != NULL; pt_segm = (TRACK*) PtNext)
		{
		PtNext = pt_segm->Pnext;
		if( pt_segm->GetState(SEGM_FIXE|SEGM_AR) & masque_type) continue;
		DeleteStructure(pt_segm);
		}

	GetScreen()->SetModify();
	Compile_Ratsnest(DC, TRUE);
}


/**************************************************************/
void WinEDA_PcbFrame::Erase_Modules(wxDC * DC, bool query)
/**************************************************************/
{
	if( query && ! IsOK(this, _("Delete Modules?") ) ) return;

	while ( m_Pcb->m_Modules ) DeleteStructure(m_Pcb->m_Modules);

	m_Pcb->m_Status_Pcb = 0 ;
	m_Pcb->m_NbNets = 0 ;
	m_Pcb->m_NbPads = 0 ;
	m_Pcb->m_NbNodes = 0 ;
	m_Pcb->m_NbLinks = 0 ;
	m_Pcb->m_NbNoconnect = 0 ;

	GetScreen()->SetModify();
}


/************************************************************/
void WinEDA_PcbFrame::Erase_Textes_Pcb(wxDC * DC, bool query)
/************************************************************/
{
EDA_BaseStruct * PtStruct, *PtNext;

	if( query && ! IsOK(this, _("Delete Pcb Texts") ) ) return;

	PtStruct = (EDA_BaseStruct*) m_Pcb->m_Drawings;
	for( ; PtStruct != NULL; PtStruct = PtNext)
		{
		PtNext = PtStruct->Pnext;
		if(PtStruct->m_StructType == TYPETEXTE ) DeleteStructure(PtStruct);
		}

	GetScreen()->SetModify();
}


/************************************************************/
void WinEDA_PcbFrame::Erase_Marqueurs(wxDC * DC, bool query)
/************************************************************/
{
EDA_BaseStruct * PtStruct, *PtNext;

	PtStruct = m_Pcb->m_Drawings;
	for( ; PtStruct != NULL; PtStruct = PtNext)
		{
		PtNext = PtStruct->Pnext;
		if(PtStruct->m_StructType == TYPEMARQUEUR ) DeleteStructure(PtStruct);
		}

	GetScreen()->SetModify();
}

