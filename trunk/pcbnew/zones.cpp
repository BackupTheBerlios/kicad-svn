		/******************************/
		/* Routines de trace de Zones */
		/******************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "pcbnew.h"

#include "autorout.h"
#include "cell.h"
#include "trigo.h"

#include "protos.h"

/* Routines Locales */
static void Display_Zone_Netname(WinEDA_PcbFrame *frame);
static void Exit_Zones(WinEDA_DrawFrame * frame, wxDC *DC);
static void Show_Zone_Edge_While_MoveMouse(WinEDA_DrawPanel * panel, wxDC * DC, bool erase);
static void Genere_Segments_Zone(WinEDA_PcbFrame *frame, wxDC * DC, int net_code);
static bool Genere_Pad_Connexion(WinEDA_PcbFrame *frame, wxDC * DC, int layer);

/* Variables locales */
static bool Zone_Debug = FALSE;
static bool Zone_45_Only = FALSE;
static bool Zone_Exclude_Pads = TRUE;
static bool Zone_Genere_Freins_Thermiques = TRUE;

static int TimeStamp;			/* signature temporelle pour la zone generee */

	/*****************************/
	/* class WinEDA_NetlistFrame */
	/*****************************/
enum id_zone_functions
{
	ID_CLOSE_ZONE = 1900,
	ID_FILL_ZONE,
	ID_SELECT_GRID,
	ID_SELECT_OPT,
	ID_SET_OPTIONS_ZONE
};


class WinEDA_ZoneFrame: public wxDialog
{
private:

	WinEDA_PcbFrame * m_Parent;
	wxRadioBox * m_Grid;
	wxRadioBox * m_FillOpt;
	wxRadioBox * m_OrientEdgesOpt;
	WinEDA_ValueCtrl * m_ZoneIsolCtrl;

public:
	// Constructor and destructor
	WinEDA_ZoneFrame(WinEDA_PcbFrame *parent);
	~WinEDA_ZoneFrame(void)
		{
		}

private:
	void OnQuit( wxCommandEvent & event);
	void ExecFillZone( wxCommandEvent & event);

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WinEDA_ZoneFrame, wxDialog)
	EVT_BUTTON(ID_CLOSE_ZONE, WinEDA_ZoneFrame::OnQuit)
	EVT_BUTTON(ID_FILL_ZONE, WinEDA_ZoneFrame::ExecFillZone)
	EVT_BUTTON(ID_SET_OPTIONS_ZONE, WinEDA_ZoneFrame::ExecFillZone)
END_EVENT_TABLE()


WinEDA_ZoneFrame::WinEDA_ZoneFrame(WinEDA_PcbFrame *parent):
		wxDialog(parent, -1, _("Fill Zones Options"), wxPoint(-1,-1), wxSize(380, 230),
				DIALOG_STYLE)
{
wxPoint pos;
wxString number;
wxButton * Button;
int x, y;

	m_Parent = parent;
	SetFont(*g_DialogFont);
	Centre();

	/* Creation des boutons de commande */
	pos.x = 275; pos.y = 10;
	Button = new wxButton(this, ID_CLOSE_ZONE,
						_("Cancel"), pos);
	Button->SetForegroundColour(*wxBLUE);

	pos.y += Button->GetDefaultSize().y + 3;
	Button = new wxButton(this, ID_FILL_ZONE,
						_("Fill"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.y += Button->GetDefaultSize().y + 3;
	Button = new wxButton(this, ID_SET_OPTIONS_ZONE,
						_("Set Options"), pos);
	Button->SetForegroundColour(wxColour(0,100,0));

	// Grid:
	pos.x = 10; pos.y = 10;
wxString grid_select_inches[4] = { "0.005", "0.010", "0.025", "0.050" };
wxString grid_select_mm[4] = { "0.156", "0.3125", "0.625", "1.27" };
	m_Grid = new wxRadioBox( this, ID_SELECT_GRID,
			UnitMetric ? _("Grid (mm):") : _("Grid (inches):"),
			pos, wxDefaultSize, 4,
			UnitMetric ? grid_select_mm :grid_select_inches, 1);
	m_Grid->GetSize(&x, &y);
	if ( pas_route == 100 ) m_Grid->SetSelection(1);
	if ( pas_route == 250 ) m_Grid->SetSelection(2);
	if ( pas_route == 500 ) m_Grid->SetSelection(3);

	// Options:
wxString opt_select[3] = { _("Include Pads"), _("Thermal"), _("Exclude Pads") };
	pos.x += x + 10;
	m_FillOpt = new wxRadioBox( this, ID_SELECT_OPT, _("Grid:"),
			pos, wxDefaultSize, 3, opt_select, 1);
	if ( Zone_Exclude_Pads)
		{
		if ( Zone_Genere_Freins_Thermiques ) m_FillOpt->SetSelection(1);
		else m_FillOpt->SetSelection(2);
		}

	// Isolation
	if ( g_DesignSettings.m_ZoneClearence == 0 ) g_DesignSettings.m_ZoneClearence = g_DesignSettings.m_TrackClearence;
	pos.x = 15; pos.y += y + 10;
	m_ZoneIsolCtrl = new WinEDA_ValueCtrl(this, _("Isolation"),
			g_DesignSettings.m_ZoneClearence, UnitMetric, pos, m_Parent->m_InternalUnits);

	pos.x += m_ZoneIsolCtrl->GetDimension().x + 20; pos.y -= 20;
wxString orient_select[2] = { _("Any"), _("H , V and 45 deg") };
	m_OrientEdgesOpt = new wxRadioBox( this, ID_SELECT_GRID,
			_("Zone edges orient:"),
			pos, wxDefaultSize, 2, orient_select, 1);
	if ( Zone_45_Only ) m_OrientEdgesOpt->SetSelection(1);
}

/**********************************************************************/
void  WinEDA_ZoneFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
/**********************************************************************/
{
    Close(true);    // true is to force the frame to close
}



/***********************************************************/
void WinEDA_ZoneFrame::ExecFillZone( wxCommandEvent & event)
/***********************************************************/
{
	switch ( m_FillOpt->GetSelection() )
		{
		case 0:
			Zone_Exclude_Pads = FALSE;
			Zone_Genere_Freins_Thermiques = FALSE;
			break;

		case 1:
			Zone_Exclude_Pads = TRUE;
			Zone_Genere_Freins_Thermiques = TRUE;
			break;

		case 2:
			Zone_Exclude_Pads = TRUE;
			Zone_Genere_Freins_Thermiques = FALSE;
			break;
		}

	switch ( m_Grid->GetSelection() )
		{
		case 0:
			pas_route = 50;
			break;

		case 1:
			pas_route = 100;
			break;

		case 2:
			pas_route = 250;
			break;

		case 3:
			pas_route = 500;
			break;
		}

	g_DesignSettings.m_ZoneClearence = m_ZoneIsolCtrl->GetValue();
	if ( m_OrientEdgesOpt->GetSelection() == 0) Zone_45_Only = FALSE;
	else Zone_45_Only = TRUE;

	if ( event.GetId() == ID_SET_OPTIONS_ZONE ) EndModal(1);
	else EndModal(0);
}

/**************************************************************/
void WinEDA_PcbFrame::Edit_Zone_Width(wxDC * DC, SEGZONE * Zone)
/**************************************************************/
/* Edite (change la largeur des segments) la zone Zone.
	La zone est constituee des segments zones de meme TimeStamp
*/
{
SEGZONE * pt_segm, * NextS ;
int TimeStamp;
bool modify = FALSE;
double f_new_width;
int w_tmp;
wxString Line;
wxString Msg( _("New zone segment width: ") );

	if ( Zone == NULL ) return;

	f_new_width = To_User_Unit(UnitMetric, Zone->m_Width, GetScreen()->GetInternalUnits());
	Line.Printf("%.4f", f_new_width);
	Msg += UnitMetric ? "(mm)" : "(\")";
	if ( Get_Message(Msg, Line, this) != 0 ) return;

	w_tmp = g_DesignSettings.m_CurrentTrackWidth;
	Line.ToDouble( &f_new_width);
	g_DesignSettings.m_CurrentTrackWidth = From_User_Unit(UnitMetric, f_new_width, GetScreen()->GetInternalUnits());

	TimeStamp = Zone->m_TimeStamp;

	for( pt_segm = (SEGZONE*)m_Pcb->m_Zone; pt_segm != NULL; pt_segm = NextS)
	{
		NextS = (SEGZONE*) pt_segm->Pnext;
		if(pt_segm->m_TimeStamp == TimeStamp)
		{
			modify = TRUE;
			Edit_TrackSegm_Width(DC, pt_segm);
		}
	}

	g_DesignSettings.m_CurrentTrackWidth = w_tmp;
	if ( modify )
	{
		GetScreen()->SetModify();
		DrawPanel->Refresh();
	}
}



/**********************************************************/
void WinEDA_PcbFrame::Delete_Zone(wxDC * DC, SEGZONE * Zone)
/**********************************************************/
/* Efface la zone Zone.
	La zone est constituee des segments zones de meme TimeStamp
*/
{
SEGZONE * pt_segm, * NextS ;
int TimeStamp;
int nb_segm = 0;
bool modify = FALSE;

	TimeStamp = Zone->m_TimeStamp;

	for( pt_segm = (SEGZONE*)m_Pcb->m_Zone; pt_segm != NULL; pt_segm = NextS)
	{
		NextS = (SEGZONE*) pt_segm->Pnext;
		if(pt_segm->m_TimeStamp == TimeStamp)
		{
			modify = TRUE;
			/* effacement des segments a l'ecran */
			Trace_Une_Piste(DrawPanel, DC, pt_segm, nb_segm, GR_XOR);
			DeleteStructure(pt_segm);
		}
	}

	if ( modify )
	{
		GetScreen()->SetModify();
		GetScreen()->SetRefreshReq();
	}
}


/*****************************************************************************/
EDGE_ZONE * WinEDA_PcbFrame::Del_SegmEdgeZone(wxDC * DC, EDGE_ZONE * edge_zone)
/*****************************************************************************/
/* Routine d'effacement du segment de limite zone en cours de trace */
{
EDGE_ZONE * Segm, * previous_segm;

	if (m_Pcb->m_CurrentLimitZone) Segm = m_Pcb->m_CurrentLimitZone;
	else Segm = edge_zone;

	if( Segm == NULL) return NULL;

	Trace_DrawSegmentPcb(DrawPanel, DC, Segm, GR_XOR);

	previous_segm = (EDGE_ZONE *)Segm->Pback;
	delete Segm;

	Segm = previous_segm;
	m_Pcb->m_CurrentLimitZone = Segm;
	GetScreen()->m_CurrentItem = Segm;

	if( Segm )
		{
		Segm->Pnext = NULL;
		if( GetScreen()->ManageCurseur)
			GetScreen()->ManageCurseur(DrawPanel, DC, TRUE);
		}
	else
		{
		GetScreen()->ManageCurseur = NULL;
		GetScreen()->ForceCloseManageCurseur = NULL;
		GetScreen()->m_CurrentItem = NULL;
		}
	return Segm;
}


/*********************************************/
void WinEDA_PcbFrame::CaptureNetName(wxDC * DC)
/*********************************************/
/* routine permettant de capturer le nom net net (netcode) d'un pad
 ou d'une piste pour l'utiliser comme netcode de zone
*/
{
D_PAD* pt_pad = 0;
TRACK * adrpiste ;
MODULE * Module;
int masquelayer = g_TabOneLayerMask[GetScreen()->m_Active_Layer];
int netcode;

	netcode = -1;
	MsgPanel->EraseMsgBox();
	adrpiste = Locate_Pistes(m_Pcb->m_Track, masquelayer,CURSEUR_OFF_GRILLE);
	if ( adrpiste == NULL )
		{
		pt_pad = Locate_Any_Pad(m_Pcb, CURSEUR_OFF_GRILLE);

		if(pt_pad)  /* Verif qu'il est bien sur la couche active */
			{
			Module = (MODULE*) pt_pad->m_Parent;
			pt_pad = Locate_Pads(Module,g_TabOneLayerMask[GetScreen()->m_Active_Layer],
												CURSEUR_OFF_GRILLE);
			}
		if( pt_pad )
			{
			pt_pad->Display_Infos(this);
			netcode = pt_pad->m_NetCode;
			}
		}
	else
		{
		Affiche_Infos_Piste(this, adrpiste) ;
		netcode = adrpiste->m_NetCode;
		}

	// Mise en surbrillance du net
	if(g_HightLigt_Status) Hight_Light(DC);
	g_HightLigth_NetCode = netcode;

	if ( g_HightLigth_NetCode >= 0 )
		{
		Hight_Light(DC);
		}

	/* Affichage du net selectionne pour la zone a tracer */
	Display_Zone_Netname(this);
}

/*******************************************************/
static void Display_Zone_Netname(WinEDA_PcbFrame *frame)
/*******************************************************/
/*
	Affiche le net_code et le nom de net couramment selectionne
*/
{
EQUIPOT * pt_equipot;
char line[1024];

	pt_equipot = frame->m_Pcb->m_Equipots;

	if( g_HightLigth_NetCode > 0 )
		{
		for( ; pt_equipot != NULL; pt_equipot = (EQUIPOT*)pt_equipot->Pnext)
			{
			if( pt_equipot->m_NetCode == g_HightLigth_NetCode) break;
			}
		if( pt_equipot )
			{
			sprintf(line,"Zone: Net[%d] <%s>",g_HightLigth_NetCode,
												pt_equipot->m_Netname.GetData());
			}
		else sprintf(line,"Zone: NetCode[%d], Equipot non trouvee",
												g_HightLigth_NetCode);
		}

	else strcpy(line,"Zone: pas de net selectionne");

	frame->Affiche_Message(line);
}

/********************************************************/
static void Exit_Zones(WinEDA_DrawFrame * frame, wxDC *DC)
/********************************************************/
/* routine d'annulation de la Commande Begin_Zone si une piste est en cours
	de tracage, ou de sortie de l'application SEGZONES.
	Appel par la touche ESC
 */
{
WinEDA_PcbFrame* pcbframe = (WinEDA_PcbFrame*)frame;

	if( pcbframe->m_Pcb->m_CurrentLimitZone )
		{
		if( pcbframe->GetScreen()->ManageCurseur ) /* trace en cours */
			{
			pcbframe->GetScreen()->ManageCurseur(pcbframe->DrawPanel, DC, 0);
			}
		pcbframe->DelLimitesZone(DC, FALSE);
		}

	pcbframe->GetScreen()->ManageCurseur = NULL;
	pcbframe->GetScreen()->ForceCloseManageCurseur = NULL;
	pcbframe->GetScreen()->m_CurrentItem = NULL;

}


/**************************************************************/
void WinEDA_BasePcbFrame::DelLimitesZone(wxDC *DC, bool Redraw)
/**************************************************************/
/* Supprime la liste des segments constituant la frontiere courante
	Libere la memoire correspondante
*/
{
EDGE_ZONE * segment, * Next;

	if( m_Pcb->m_CurrentLimitZone == NULL ) return;

	if ( ! IsOK(this, _("Delete Current Zone Edges")) ) return;
	/* efface ancienne limite de zone */
	segment = m_Pcb->m_CurrentLimitZone;
	for( ; segment != NULL; segment = Next)
		{
		Next = (EDGE_ZONE*) segment->Pback;
		if ( Redraw ) Trace_DrawSegmentPcb(DrawPanel, DC, segment,GR_XOR);
		segment->Pnext = NULL; delete segment;
		}

	GetScreen()->m_CurrentItem = NULL;
	m_Pcb->m_CurrentLimitZone = NULL;
}

/********************************************/
EDGE_ZONE * WinEDA_PcbFrame::Begin_Zone(void)
/********************************************/
/*
Routine d'initialisation d'un trace de Limite de Zone ou
	de placement d'un point intermediaire
*/
{
EDGE_ZONE * oldedge, * newedge = NULL;

	oldedge = m_Pcb->m_CurrentLimitZone;

	if( (m_Pcb->m_CurrentLimitZone == NULL ) || /* debut reel du trace */
		(GetScreen()->ManageCurseur == NULL) ) /* reprise d'un trace complementaire */
		{
		m_Pcb->m_CurrentLimitZone = newedge = new EDGE_ZONE( m_Pcb );

		newedge->m_Flags = IS_NEW | STARTPOINT | IS_MOVED;
		newedge->Pback = oldedge;
		if(oldedge) oldedge->Pnext = newedge;
		newedge->m_Layer = GetScreen()->m_Active_Layer;
		newedge->m_Width = 2 ;		/* Largeur minimum tracable */
		newedge->m_Start = newedge->m_End = GetScreen()->m_Curseur;

		m_Pcb->m_CurrentLimitZone = newedge;
		GetScreen()->ManageCurseur = Show_Zone_Edge_While_MoveMouse;
		GetScreen()->ForceCloseManageCurseur = Exit_Zones;
		}

	else	/* piste en cours : les coord du point d'arrivee ont ete mises
			a jour par la routine Show_Zone_Edge_While_MoveMouse*/
		{
		if( (oldedge->m_Start.x != oldedge->m_End.x) ||
			(oldedge->m_Start.y != oldedge->m_End.y) )
			{
			newedge = new EDGE_ZONE( oldedge);
			newedge->Pback = oldedge;
			oldedge->Pnext = newedge;
			newedge->m_Flags = IS_NEW | IS_MOVED;
			newedge->m_Start = newedge->m_End = oldedge->m_End;
			newedge->m_Layer = GetScreen()->m_Active_Layer;
			m_Pcb->m_CurrentLimitZone = newedge;
			}
		}

	return newedge;
}

/*********************************************/
void WinEDA_PcbFrame::End_Zone(wxDC * DC)
/*********************************************/
/*
	Routine de fin de trace d'une zone (succession de segments)
*/
{
EDGE_ZONE * PtLim;

	if( m_Pcb->m_CurrentLimitZone )
		{
		Begin_Zone();

		/* le dernier point genere est de longueur tj nulle donc inutile. */
		/* il sera raccorde au point de depart */
		PtLim =  m_Pcb->m_CurrentLimitZone;
		PtLim->m_Flags &= ~(IS_NEW|IS_MOVED);
		while( PtLim && PtLim->Pback)
			{
			PtLim = (EDGE_ZONE*) PtLim->Pback;
			if ( PtLim->m_Flags & STARTPOINT) break;
			PtLim->m_Flags &= ~(IS_NEW|IS_MOVED);
			}

		if( PtLim )
			{
			PtLim->m_Flags &= ~(IS_NEW|IS_MOVED);
			m_Pcb->m_CurrentLimitZone->m_End = PtLim->m_Start;
			}
		Trace_DrawSegmentPcb(DrawPanel, DC, m_Pcb->m_CurrentLimitZone,GR_XOR);
		}

	GetScreen()->ManageCurseur = NULL;
	GetScreen()->ForceCloseManageCurseur = NULL;
}


/******************************************************************************************/
static void Show_Zone_Edge_While_MoveMouse(WinEDA_DrawPanel * panel, wxDC * DC, bool erase)
/******************************************************************************************/
/* redessin du contour de la piste  lors des deplacements de la souris
*/
{
EDGE_ZONE * PtLim, * edgezone;
WinEDA_PcbFrame * pcbframe = (WinEDA_PcbFrame *) panel->m_Parent;

	if( pcbframe->m_Pcb->m_CurrentLimitZone == NULL ) return ;

	/* efface ancienne position si elle a ete deja dessinee */
	if( erase )
	{
		PtLim = pcbframe->m_Pcb->m_CurrentLimitZone;
		for( ; PtLim != NULL; PtLim = (EDGE_ZONE*) PtLim->Pback)
		{
			Trace_DrawSegmentPcb(panel, DC, PtLim,GR_XOR);
		}
	}

	/* mise a jour de la couche */
	edgezone = PtLim = pcbframe->m_Pcb->m_CurrentLimitZone;
	for( ; PtLim != NULL; PtLim = (EDGE_ZONE*) PtLim->Pback)
	{
		PtLim->m_Layer = pcbframe->GetScreen()->m_Active_Layer;
	}

	/* dessin de la nouvelle piste : mise a jour du point d'arrivee */
	if (Zone_45_Only)
	{/* Calcul de l'extremite de la piste pour orientations permises:
										horiz,vertical ou 45 degre */
		edgezone->m_End = pcbframe->GetScreen()->m_Curseur;
		Calcule_Coord_Extremite_45(edgezone->m_Start.x, edgezone->m_Start.y,
						&edgezone->m_End.x, &edgezone->m_End.y);
	}
	else	/* ici l'angle d'inclinaison est quelconque */
	{
		edgezone->m_End = pcbframe->GetScreen()->m_Curseur;
	}

	PtLim = edgezone;
	for( ; PtLim != NULL; PtLim = (EDGE_ZONE*) PtLim->Pback)
	{
		Trace_DrawSegmentPcb(panel, DC, PtLim,GR_XOR);
	}
}


/**********************************************/
void WinEDA_PcbFrame::Fill_Zone(wxDC * DC)
/**********************************************/
/*
Fonction generale de creation de zone
	Un contour de zone doit exister, sinon l'ensemble du PCB est utilise
	Il peut y avoir plusieurs contours, toujours fermés,
	ce qui permet de creer des obstacles et donc des parties non remplies.

	Le remplissage s'effectue a partir du point d'ancrage, jusque ves les limites
	données par les contours de zones, edges pcb, ou autres obstacles

	On place la zone sur la couche (layer) active.

	Si la zone est sur couche cuivre, et si un net est selectionné (mis en
	"Hight Light" la zone fera partie de ce net
*/
{
int ii, jj;
EDGE_ZONE* PtLim;
int lp_tmp, lay_tmp_TOP, lay_tmp_BOTTOM;
EQUIPOT* pt_equipot;
int save_isol = g_DesignSettings.m_TrackClearence;
wxPoint ZoneStartFill;
wxString msg;

	MsgPanel->EraseMsgBox();
	if ( m_Pcb->ComputeBoundaryBox() == FALSE )
		{
		DisplayError(this, "Board is empty!", 10);
		return;
		}
wxPoint curs_pos = GetScreen()->m_Curseur;
	WinEDA_ZoneFrame * frame = new WinEDA_ZoneFrame(this);
	ii = frame->ShowModal(); frame->Destroy();
	GetScreen()->m_Curseur = curs_pos;
	DrawPanel->MouseToCursorSchema();
	if ( ii ) return;

	g_DesignSettings.m_TrackClearence = g_DesignSettings.m_ZoneClearence;

	/* mise a jour de la couche */
	PtLim = m_Pcb->m_CurrentLimitZone;
	for( ; PtLim != NULL; PtLim = (EDGE_ZONE*) PtLim->Pback)
		{
		Trace_DrawSegmentPcb(DrawPanel, DC, PtLim, GR_XOR);
		PtLim->m_Layer = GetScreen()->m_Active_Layer;
		Trace_DrawSegmentPcb(DrawPanel, DC, PtLim, GR_XOR);
		}

	TimeStamp = time( NULL );

	/* Calcul du pas de routage fixe a 5 mils et plus */
	if (pas_route < 50 ) pas_route = 50 ;
	E_scale = pas_route / 50 ; if (pas_route < 1 ) pas_route = 1 ;

	/* calcule de Ncols et Nrow, taille de la matrice de routage */
	ComputeMatriceSize(this, pas_route);

	/* Determination de la cellule pointee par la souris */
	ZoneStartFill.x = (GetScreen()->m_Curseur.x - m_Pcb->m_BoundaryBox.m_Pos.x + (pas_route/2) ) / pas_route;
	ZoneStartFill.y = (GetScreen()->m_Curseur.y - m_Pcb->m_BoundaryBox.m_Pos.y + (pas_route/2) ) / pas_route;
	if(ZoneStartFill.x < 0) ZoneStartFill.x = 0;
	if(ZoneStartFill.x >= Ncols) ZoneStartFill.x = Ncols-1;
	if(ZoneStartFill.y < 0) ZoneStartFill.y = 0;
	if(ZoneStartFill.y >= Nrows) ZoneStartFill.y = Nrows-1;

	/* Creation du mapping de la matrice de routage */
	Nb_Sides = ONE_SIDE;
	if( Board.InitBoard() < 0)
		{
		DisplayError(this, "Pas de memoire pour generation de zones");
		return;
		}

	sprintf(cbuf,"%d",Ncols);
	Affiche_1_Parametre(this, 1, "Cols",cbuf,GREEN);
	sprintf(cbuf,"%d",Nrows);
	Affiche_1_Parametre(this, 7, "Lines",cbuf,GREEN);
 	sprintf(cbuf,"%d", Board.m_MemSize / 1024 );
	Affiche_1_Parametre(this, 14, "Mem(Ko)",cbuf,CYAN);

	lay_tmp_BOTTOM = Route_Layer_BOTTOM;
	lay_tmp_TOP = Route_Layer_TOP;
	 /* Init des Parametres utilisés dans les routines de tracé   */
	Route_Layer_BOTTOM = Route_Layer_TOP = GetScreen()->m_Active_Layer;
	lp_tmp = g_DesignSettings.m_CurrentTrackWidth; g_DesignSettings.m_CurrentTrackWidth = pas_route;

	/* Affichage du NetName */
	if(g_HightLigth_NetCode > 0)
		{
		pt_equipot = GetEquipot(m_Pcb, g_HightLigth_NetCode);
		if( pt_equipot == NULL)
			{
			if(g_HightLigth_NetCode > 0 ) DisplayError(this, "Equipot Erreur");
			}
		else msg = pt_equipot->m_Netname;
		}
	else msg = _("No Net");

	Affiche_1_Parametre(this, 22,_("NetName"),msg,RED) ;

	/* Init des points d'accrochage possibles de la zone:
		les pistes du net sont des points d'accrochage convenables*/
	TRACK * pt_segm = m_Pcb->m_Track;
	for( ; pt_segm != NULL; pt_segm = (TRACK*) pt_segm->Pnext)
		{
		if(g_HightLigth_NetCode != pt_segm->m_NetCode) continue;
		if ( pt_segm->m_Layer != GetScreen()->m_Active_Layer ) continue;
		if (pt_segm->m_StructType != TYPETRACK ) continue;
		TraceSegmentPcb(m_Pcb, pt_segm, CELL_is_FRIEND, 0, WRITE_CELL );
		}

	/* Trace des contours du PCB sur la matrice de routage: */
	Route_Layer_BOTTOM = Route_Layer_TOP = EDGE_N;
	PlaceCells(m_Pcb, -1, 0);
	Route_Layer_BOTTOM = Route_Layer_TOP = GetScreen()->m_Active_Layer;

	/* Trace des limites de la zone sur la matrice de routage: */
	PtLim = m_Pcb->m_CurrentLimitZone;
	for( ; PtLim != NULL; PtLim = (EDGE_ZONE*)PtLim->Pback)
		{
		int ux0, uy0, ux1, uy1;
		ux0 = PtLim->m_Start.x - m_Pcb->m_BoundaryBox.m_Pos.x;
		uy0 = PtLim->m_Start.y - m_Pcb->m_BoundaryBox.m_Pos.y;
		ux1 = PtLim->m_End.x - m_Pcb->m_BoundaryBox.m_Pos.x;
		uy1 = PtLim->m_End.y - m_Pcb->m_BoundaryBox.m_Pos.y;
		TraceLignePcb(ux0,uy0,ux1,uy1,-1,HOLE|CELL_is_EDGE,WRITE_CELL);
		}

	/* Init du point d'accrochage de la zone donné par la position souris*/
	OrCell(ZoneStartFill.y,ZoneStartFill.x, BOTTOM, CELL_is_ZONE);

	/* Marquage des cellules faisant partie de la zone*/
	ii = 1; jj = 1;
	while( ii )
		{
		sprintf(cbuf,"%d", jj++ );
		Affiche_1_Parametre(this, 50, "Iter.",cbuf,CYAN);
		ii = Propagation(this);
		}

	/* Selection des cellules convenables pour les points d'ancrage de la zone */
	for( ii = 0; ii < Nrows ; ii++)
		{
		for( jj = 0; jj < Ncols ; jj++)
			{
			long cell = GetCell(ii,jj,BOTTOM);
			if( (cell & CELL_is_ZONE) )
				{
				if ( (cell & CELL_is_FRIEND) == 0)
					AndCell(ii,jj,BOTTOM, ~(CELL_is_FRIEND|CELL_is_ZONE) );
				}
			}
		}

    /* Maintenant, toutes les cellules candidates sont marquees */
	/* Placement des cellules (pads, tracks, vias, edges pcb ou segments)
		faisant des obsctacles sur la matrice de routage */
	ii = 0;
	if( Zone_Exclude_Pads ) ii = FORCE_PADS;
	Affiche_1_Parametre(this, 42, "GenZone","",RED);
	PlaceCells(m_Pcb, g_HightLigth_NetCode, ii);
	Affiche_1_Parametre(this, -1, "",_("Ok"),RED);

	/* Trace des limites de la zone sur la matrice de routage
	(a pu etre detruit par PlaceCells()) : */
	PtLim = m_Pcb->m_CurrentLimitZone;
	for( ; PtLim != NULL; PtLim = (EDGE_ZONE*)PtLim->Pback)
		{
		int ux0, uy0, ux1, uy1;
		ux0 = PtLim->m_Start.x - m_Pcb->m_BoundaryBox.m_Pos.x;
		uy0 = PtLim->m_Start.y - m_Pcb->m_BoundaryBox.m_Pos.y;
		ux1 = PtLim->m_End.x - m_Pcb->m_BoundaryBox.m_Pos.x;
		uy1 = PtLim->m_End.y - m_Pcb->m_BoundaryBox.m_Pos.y;
		TraceLignePcb(ux0,uy0,ux1,uy1,-1,HOLE|CELL_is_EDGE,WRITE_CELL);
		}

	/* Init du point d'accrochage de la zone donné par la position souris
	(a pu etre detruit par PlaceCells()) : */
	OrCell(ZoneStartFill.y,ZoneStartFill.x, BOTTOM, CELL_is_ZONE);

	if(Zone_Debug) DisplayBoard(DrawPanel, DC);

	/* Remplissage des cellules (creation effective de la zone)*/
	ii = 1; jj = 1;
	while( ii )
		{
		sprintf(cbuf,"%d", jj++ );
		Affiche_1_Parametre(this, 50, "Iter.",cbuf,CYAN);
		ii = Propagation(this);
		}

	if(Zone_Debug) DisplayBoard(DrawPanel, DC);

	/* Generation des segments de piste type Zone correspondants*/
	if(g_HightLigth_NetCode < 0 )
			Genere_Segments_Zone(this, DC, 0);
	else	Genere_Segments_Zone(this, DC, g_HightLigth_NetCode);

	/* Trace des connexions type frein thermique */
	g_DesignSettings.m_CurrentTrackWidth = lp_tmp;
	if ( Zone_Exclude_Pads && Zone_Genere_Freins_Thermiques)
			Genere_Pad_Connexion(this, DC, GetScreen()->m_Active_Layer);

	g_DesignSettings.m_TrackClearence = save_isol;

	GetScreen()->SetModify();

	/* Liberation de la memoire */
	Board.UnInitBoard();

	/* Reprise des conditions initiales */
	Route_Layer_TOP = lay_tmp_TOP;
	Route_Layer_BOTTOM = lay_tmp_BOTTOM;
}


/*******************************************************************************/
static void Genere_Segments_Zone(WinEDA_PcbFrame *frame, wxDC * DC, int net_code)
/*******************************************************************************/
/* Genere les segments de piste dans les limites de la zone a remplir
	Algorithme:
		procede en 2 balayages
			- Gauche->droite
			- Haut->Bas
	Parametres:
		net_code = net_code a attribuer au segment de zone
		TimeStamp(global): signature temporelle d'identification
			(mis en .start)
*/
{
int row, col;
long current_cell, old_cell;
int ux0 = 0, uy0 = 0, ux1 = 0, uy1 = 0;
int Xmin = frame->m_Pcb->m_BoundaryBox.m_Pos.x;
int Ymin = frame->m_Pcb->m_BoundaryBox.m_Pos.y;
SEGZONE * pt_track;
int layer = frame->GetScreen()->m_Active_Layer;
int nbsegm = 0;

	/* balayage Gauche-> droite */
	Affiche_1_Parametre(frame, 64, "Segm H","0",BROWN);
	for( row = 0; row < Nrows ; row++)
		{
		old_cell = 0;
		uy0 = uy1 = (row * pas_route) + Ymin;
		for( col = 0; col < Ncols ; col++)
			{
			current_cell = GetCell(row,col,BOTTOM) & CELL_is_ZONE;
			if(current_cell) /* ce point doit faire partie d'un segment */
				{
				ux1 = (col * pas_route) + Xmin;
				if( old_cell == 0 ) ux0 = ux1;
				}

			if( ! current_cell || (col == Ncols-1) ) /* peut etre fin d'un segment */
				{
				if( (old_cell) && (ux0 != ux1) )
					{	  /* un segment avait debute de longueur > 0 */
					pt_track = new SEGZONE(frame->m_Pcb);
					pt_track->m_Layer = layer;
					pt_track->m_NetCode = net_code;
					pt_track->m_Width = pas_route;
					pt_track->m_Start.x = ux0; pt_track->m_Start.y = uy0;
					pt_track->m_End.x = ux1; pt_track->m_End.y = uy1;
					pt_track->m_TimeStamp = TimeStamp;
					pt_track->Insert(frame->m_Pcb, NULL);
					pt_track->Draw(frame->DrawPanel, DC, GR_OR);
					nbsegm++;
					}
				}
			old_cell = current_cell;
			}
		sprintf(cbuf,"%d",nbsegm);
		Affiche_1_Parametre(frame, -1, "",cbuf,BROWN);
		}

	Affiche_1_Parametre(frame, 72, "Segm V", "0",BROWN);
	for( col = 0; col < Ncols ; col++)
		{
		old_cell = 0;
		ux0 = ux1 = (col * pas_route) + Xmin;
		for( row = 0; row < Nrows ; row++)
			{
			current_cell = GetCell(row,col,BOTTOM) & CELL_is_ZONE;
			if(current_cell) /* ce point doit faire partie d'un segment */
				{
				uy1 = (row * pas_route) + Ymin;
				if( old_cell == 0 ) uy0 = uy1;
				}
			if( ! current_cell || (row == Nrows-1) )	/* peut etre fin d'un segment */
				{
				if( (old_cell) && (uy0 != uy1) )
					{	  /* un segment avait debute de longueur > 0 */
					pt_track = new SEGZONE(frame->m_Pcb);
					pt_track->m_Layer = layer;
					pt_track->m_Width = pas_route;
					pt_track->m_NetCode = net_code;
					pt_track->m_Start.x = ux0; pt_track->m_Start.y = uy0;
					pt_track->m_End.x = ux1; pt_track->m_End.y = uy1;
					pt_track->m_TimeStamp = TimeStamp;
					pt_track->Insert(frame->m_Pcb, NULL);
					pt_track->Draw(frame->DrawPanel, DC, GR_OR);
					nbsegm++;
					}
				}
			old_cell = current_cell;
			}
		sprintf(cbuf,"%d",nbsegm);
		Affiche_1_Parametre(frame, -1, "",cbuf,BROWN);
		}
}


/********************************************/
int Propagation(WinEDA_PcbFrame * frame)
/********************************************/
/* Determine les cellules inscrites dans les limites de la zone a remplir
	Algorithme:
	Si une cellule disponible a un voisin faisant partie de la zone, elle
	devient elle meme partie de la zone
	On procede en 4 balayages de la matrice des cellules
			- Gauche->droite de Haut->bas
			- Droite->gauche de Haut->bas
			- Bas->Haut de Droite->gauche
			- Bas->Haut de Gauche->Droite
		et pour chaque balayage, on considere des 2 cellules voisines de
	la cellule courants: cellule precedente sur la ligne et cellule precedente
	sur la colonne.

	La routine peut demander plusieurs iterations
	les iterations doivent continuer juqu'a ce que la routine ne trouve plus
	de cellules a modifier.
	Retourne:
	Nombre de cellules modifiees (c.a.d mises a la valeur CELL_is_ZONE.
*/
{
int row, col, nn;
long current_cell, old_cell_H;
int long * pt_cell_V;
int nbpoints = 0;
#define NO_CELL_ZONE (HOLE | CELL_is_EDGE | CELL_is_ZONE)

	Affiche_1_Parametre(frame, 57, "Passe",cbuf,CYAN);
	/* balayage Gauche-> droite de Haut->bas */
	Affiche_1_Parametre(frame, -1, "","1",CYAN);

	// Reservation memoire pour stockahe de 1 ligne ou une colonne de cellules
	nn = MAX(Nrows, Ncols) * sizeof(*pt_cell_V);
	pt_cell_V = (long *) MyMalloc(nn);
	memset(pt_cell_V, 0, nn);
	for( row = 0; row < Nrows ; row++)
		{
		old_cell_H = 0;
		for( col = 0; col < Ncols ; col++)
			{
			current_cell = GetCell(row,col,BOTTOM) & NO_CELL_ZONE;
			if(current_cell == 0 )  /* une cellule libre a ete trouvee */
				{
				if( (old_cell_H & CELL_is_ZONE)
					|| (pt_cell_V[col] & CELL_is_ZONE) )
					{
					OrCell(row,col,BOTTOM,CELL_is_ZONE);
					current_cell = CELL_is_ZONE;
					nbpoints++;
					}
				}
			pt_cell_V[col] = old_cell_H = current_cell;
			}
		}

	/* balayage Droite-> gauche de Haut->bas */
	Affiche_1_Parametre(frame, -1, "","2",CYAN);
	memset(pt_cell_V, 0, nn);
	for( row = 0; row < Nrows ; row++)
		{
		old_cell_H = 0;
		for( col = Ncols -1 ; col >= 0 ; col--)
			{
			current_cell = GetCell(row,col,BOTTOM) & NO_CELL_ZONE ;
			if(current_cell == 0 )  /* une cellule libre a ete trouvee */
				{
				if( (old_cell_H & CELL_is_ZONE)
					|| (pt_cell_V[col] & CELL_is_ZONE) )
					{
					OrCell(row,col,BOTTOM,CELL_is_ZONE);
					current_cell = CELL_is_ZONE;
					nbpoints++;
					}
				}
			pt_cell_V[col] = old_cell_H = current_cell;
			}
		}

	/* balayage Bas->Haut de Droite->gauche */
	Affiche_1_Parametre(frame, -1, "","3",CYAN);
	memset(pt_cell_V, 0, nn);
	for( col = Ncols -1 ; col >= 0 ; col--)
		{
		old_cell_H = 0;
		for( row = Nrows-1; row >= 0 ; row--)
			{
			current_cell = GetCell(row,col,BOTTOM) & NO_CELL_ZONE ;
			if(current_cell == 0 )  /* une cellule libre a ete trouvee */
				{
				if( (old_cell_H & CELL_is_ZONE)
					|| (pt_cell_V[row] & CELL_is_ZONE) )
					{
					OrCell(row,col,BOTTOM,CELL_is_ZONE);
					current_cell = CELL_is_ZONE;
					nbpoints++;
					}
				}
			pt_cell_V[row] = old_cell_H = current_cell;
			}
		}

	/* balayage  Bas->Haut de Gauche->Droite*/
	Affiche_1_Parametre(frame, -1, "","4",CYAN);
	memset(pt_cell_V, 0, nn);
	for( col = 0 ; col < Ncols ; col++)
		{
		old_cell_H = 0;
		for( row = Nrows-1; row >= 0 ; row--)
			{
			current_cell = GetCell(row,col,BOTTOM) & NO_CELL_ZONE ;
			if(current_cell == 0 )  /* une cellule libre a ete trouvee */
				{
				if( (old_cell_H & CELL_is_ZONE)
					|| (pt_cell_V[row] & CELL_is_ZONE) )
					{
					OrCell(row,col,BOTTOM,CELL_is_ZONE) ;
					current_cell = CELL_is_ZONE;
					nbpoints++;
					}
				}
			pt_cell_V[row] = old_cell_H = current_cell;
			}
		}

	MyFree(pt_cell_V);

	return(nbpoints);
}



/*****************************************************************************/
static bool Genere_Pad_Connexion(WinEDA_PcbFrame *frame, wxDC * DC, int layer)
/*****************************************************************************/
/* Generation des segments de zone de connexion zone / pad pour constitution
de freins thermiques
*/
{
int ii, jj, Npads;
D_PAD * pt_pad;
LISTE_PAD * pt_liste_pad;
TRACK * pt_track, * loctrack;
int angle;
int cX, cY, dx, dy;
int sommet[4][2];

	if( frame->m_Pcb->m_Zone == NULL ) return FALSE;	/* pas de zone */
	if( frame->m_Pcb->m_Zone->m_TimeStamp != TimeStamp ) /* c'est une autre zone */
		return FALSE;

	/* Calcul du nombre de pads a traiter et affichage */
	Affiche_1_Parametre(frame, 50, "NPads","    ",CYAN);
	pt_liste_pad = (LISTE_PAD*) frame->m_Pcb->m_Pads;
	for ( ii = 0, Npads = 0; ii < frame->m_Pcb->m_NbPads; ii++, pt_liste_pad++)
		{
		pt_pad = *pt_liste_pad;
		/* la pastille doit etre du meme net */
		if(pt_pad->m_NetCode != g_HightLigth_NetCode) continue;
		/* la pastille doit exister sur la couche */
		if( (pt_pad->m_Masque_Layer & g_TabOneLayerMask[layer]) == 0 ) continue;
		Npads++;
		}
	sprintf(cbuf,"%d", Npads );
	Affiche_1_Parametre(frame, -1, "",cbuf,CYAN);

	Affiche_1_Parametre(frame, 57, "Pads","     ",CYAN);
	pt_liste_pad = (LISTE_PAD*) frame->m_Pcb->m_Pads;
	for ( ii = 0, Npads = 0; ii < frame->m_Pcb->m_NbPads; ii++, pt_liste_pad++)
		{
		pt_pad = *pt_liste_pad;

		/* la pastille doit etre du meme net */
		if(pt_pad->m_NetCode != g_HightLigth_NetCode) continue;
		/* la pastille doit exister sur la couche */
		if( (pt_pad->m_Masque_Layer & g_TabOneLayerMask[layer]) == 0 ) continue;

		/* traitement du pad en cours */
		Npads++; sprintf(cbuf,"%d", Npads );
		Affiche_1_Parametre(frame, -1, "",cbuf,CYAN);
		cX = pt_pad->m_Pos.x;	cY = pt_pad->m_Pos.y;
		dx = pt_pad->m_Size.x / 2;
		dy = pt_pad->m_Size.y / 2;
		dx += g_DesignSettings.m_TrackClearence + pas_route;
		dy += g_DesignSettings.m_TrackClearence + pas_route;

		if(pt_pad->m_PadShape == TRAPEZE)
			{
			dx += abs(pt_pad->m_DeltaSize.y) / 2;
			dy += abs(pt_pad->m_DeltaSize.x) / 2;
			}

		/* calcul des coord des 4 segments a rajouter a partir du centre cX,cY */
		sommet[0][0] = 0; sommet[0][1] = -dy;
		sommet[1][0] = -dx; sommet[1][1] = 0;
		sommet[2][0] = 0; sommet[2][1] = dy;
		sommet[3][0] = dx; sommet[3][1] = 0;

		angle = pt_pad->m_Orient;
		for(jj = 0; jj < 4; jj++)
			{
			RotatePoint(&sommet[jj][0], &sommet[jj][1], angle);

			pt_track = new SEGZONE(frame->m_Pcb);

			pt_track->m_Layer = layer;
			pt_track->m_Width = g_DesignSettings.m_CurrentTrackWidth;
			pt_track->m_NetCode = g_HightLigth_NetCode;
			pt_track->start = pt_pad;
			pt_track->m_Start.x = cX; pt_track->m_Start.y = cY;
			pt_track->m_End.x = cX + sommet[jj][0];
			pt_track->m_End.y = cY + sommet[jj][1];
			pt_track->m_TimeStamp = TimeStamp;

			/* tst si trace possible */
			if( Drc(frame, DC, pt_track,frame->m_Pcb->m_Track,0) == BAD_DRC )
				{
				delete pt_track; continue;
				}

			/* on doit pouvoir se connecter sur la zone */
			loctrack = Locate_Zone(frame->m_Pcb->m_Zone,pt_track->m_End.x,
									pt_track->m_End.y,layer);
			if( (loctrack == NULL) || (loctrack->m_TimeStamp != TimeStamp) )
				{
				delete pt_track; continue;
				}

			pt_track->Insert(frame->m_Pcb, NULL);
			pt_track->Draw(frame->DrawPanel, DC, GR_OR);
			}
		}
	return TRUE;
}

