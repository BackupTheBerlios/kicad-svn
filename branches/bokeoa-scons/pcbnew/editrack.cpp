		/*******************************/
		/* Edition des pistes			*/
		/* Routines de trace de pistes */
		/*******************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "pcbnew.h"
#include "autorout.h"

#include "protos.h"


/* Routines Locales */
static void Exit_Editrack(WinEDA_DrawFrame * frame, wxDC *DC);
void Montre_Position_New_Piste(WinEDA_DrawPanel * panel,
				wxDC * DC, bool erase);
static int Met_Coude_a_45(WinEDA_BasePcbFrame * frame, wxDC * DC,
						TRACK * ptfinsegment);

/* variables locales */
static int OldNetCodeSurbrillance;
static int OldEtatSurbrillance;

/***********************************************/
void WinEDA_PcbFrame::DisplayTrackSettings(void)
/***********************************************/
/* Affiche la valeur courante du réglage de l'épaisseur des pistes et diam via
*/
{
wxString msg;
char buftrc[256], bufvia[256];

	valeur_param(g_DesignSettings.m_CurrentTrackWidth, buftrc);
	valeur_param(g_DesignSettings.m_CurrentViaSize, bufvia);
	msg.Printf( _("Track Width: %s   Vias Size : %s"), buftrc, bufvia);
	Affiche_Message( (char*)msg.GetData());
	m_SelTrackWidthBox_Changed = TRUE;
	m_SelViaSizeBox_Changed = TRUE;
}


/************************************************************/
static void Exit_Editrack(WinEDA_DrawFrame * _frame, wxDC *DC)
/************************************************************/
/* routine d'annulation de la Commande Begin_Route si une piste est en cours
	de tracage, ou de sortie de l'application EDITRACK.
 */
{
WinEDA_PcbFrame * frame = (WinEDA_PcbFrame *) _frame;
TRACK * track = (TRACK * ) frame->GetScreen()->m_CurrentItem;

	if( track != NULL )
		{
		/* Effacement du trace en cours */
		Montre_Position_New_Piste(frame->DrawPanel, DC, FALSE);
		if(g_HightLigt_Status) ( (WinEDA_PcbFrame *)frame)->Hight_Light(DC);
		g_HightLigth_NetCode = OldNetCodeSurbrillance;
		if(OldEtatSurbrillance) ( (WinEDA_PcbFrame *)frame)->Hight_Light(DC);

		frame->MsgPanel->EraseMsgBox();
		TRACK * previoustrack;
		// Delete current (new) track
		for(  ;track != NULL; track = previoustrack)
			{
			previoustrack = (TRACK*) track->Pback;
			delete track;
			}
		}
	frame->GetScreen()->ManageCurseur = NULL;
	frame->GetScreen()->ForceCloseManageCurseur = NULL;
	frame->GetScreen()->m_CurrentItem = NULL;
}



/***********************************************/
void WinEDA_PcbFrame::Ratsnest_On_Off(wxDC * DC)
/***********************************************/

/* Affiche ou efface le chevelu selon l'état du bouton d'appel */
{
int ii;
CHEVELU * pt_chevelu;

	if((m_Pcb->m_Status_Pcb & LISTE_CHEVELU_OK) == 0 )
		{
		if ( g_Show_Ratsnest ) Compile_Ratsnest( DC, TRUE );
		return;
		}

	DrawGeneralRatsnest(DC, 0); /* effacement eventuel du chevelu affiche */

	pt_chevelu = m_Pcb->m_Ratsnest;
	if ( pt_chevelu == NULL ) return;

	if(g_Show_Ratsnest)
		{
		for( ii = m_Pcb->GetNumRatsnests(); ii > 0 ; pt_chevelu++, ii--)
			{
			pt_chevelu->status |= CH_VISIBLE;
			}
		DrawGeneralRatsnest(DC, 0);
		}

	else
		{
		for( ii = m_Pcb->GetNumRatsnests(); ii > 0; pt_chevelu++, ii--)
			{
			pt_chevelu->status &= ~CH_VISIBLE;
			}
		}
}


/*************************************************************/
TRACK * WinEDA_PcbFrame::Begin_Route(TRACK * track, wxDC * DC)
/*************************************************************/
/*
Routine d'initialisation d'un trace de piste et/ou de mise en place d'un
nouveau point piste

	Si pas de piste en cours de trace:
		- Recherche de netname de la nouvelle piste ( pad de depart out netname
		 de la piste si depart sur une ancienne piste
		- Met en surbrillance tout le net
		- Initilise les divers pointeurs de trace
	Si piste en cours:
		- controle DRC
		- si DRC OK : addition d'un nouveau point piste
*/
{
D_PAD * pt_pad = NULL;
TRACK * adr_buf = NULL, * Track;
int masquelayer = g_TabOneLayerMask[GetScreen()->m_Active_Layer];
EDA_BaseStruct * LockPoint;
wxPoint pos = GetScreen()->m_Curseur;

	GetScreen()->ManageCurseur = Montre_Position_New_Piste;
	GetScreen()->ForceCloseManageCurseur = Exit_Editrack;

	if(track == NULL )	/* debut reel du trace */
		{
		/* effacement surbrillance ancienne */
		OldNetCodeSurbrillance = g_HightLigth_NetCode;
		OldEtatSurbrillance = g_HightLigt_Status;

		if(g_HightLigt_Status) Hight_Light(DC);

		ptstartpiste = ptnewpiste = new TRACK(m_Pcb);
		ptnewpiste->m_Flags = IS_NEW;
		nbptnewpiste = 1;
		g_HightLigth_NetCode = 0;

		/* Localisation de la pastille de reference de la piste: */
		LockPoint = LocateLockPoint(m_Pcb, pos, masquelayer);

		if( LockPoint )
			{
			if( LockPoint->m_StructType == TYPEPAD )
				{
				pt_pad = (D_PAD *) LockPoint;
				/* le debut de la piste est remis sur le centre du pad */
				pos = pt_pad->m_Pos;
				g_HightLigth_NetCode = pt_pad->m_NetCode;
				}

			else /* le point d'accrochage est un segment */
				{
				adr_buf = (TRACK *) LockPoint;
				g_HightLigth_NetCode = adr_buf->m_NetCode;
				CreateLockPoint( &pos.x, &pos.y, adr_buf, NULL);
				}
			}

		build_ratsnest_pad(pt_pad, 0, 0, 1);
		Hight_Light(DC);

		ptnewpiste->m_Flags = IS_NEW;
		ptnewpiste->m_Layer = GetScreen()->m_Active_Layer;
		ptnewpiste->m_Width = g_DesignSettings.m_CurrentTrackWidth ;
		ptnewpiste->m_Start = pos;
		ptnewpiste->m_End = ptnewpiste->m_Start;
		ptnewpiste->m_NetCode = g_HightLigth_NetCode ;
		if(pt_pad)
			{
			ptnewpiste->start = pt_pad ;
			ptnewpiste->SetState(BEGIN_ONPAD,ON);
			}
		else ptnewpiste->start = adr_buf ;

		Affiche_Infos_Piste(this, ptnewpiste) ;

		GetScreen()->ManageCurseur(DrawPanel, DC, FALSE);
		if( Drc_On && (Drc(this, DC,ptnewpiste,m_Pcb->m_Track,1 ) == BAD_DRC) )
			return ptnewpiste;

		}

	else	/* piste en cours : les coord du point d'arrivee ont ete mises
			a jour par la routine Montre_Position_New_Piste*/
		{
		if( Drc_On && (Drc(this, DC,ptnewpiste,m_Pcb->m_Track,1 ) == BAD_DRC) )
			return NULL;

		if( (ptnewpiste->m_Start.x != ptnewpiste->m_End.x) ||
			(ptnewpiste->m_Start.y != ptnewpiste->m_End.y) )
			{
			/* efface ancienne position */
			Montre_Position_New_Piste(DrawPanel, DC, FALSE);

			if( g_Raccord_45_Auto )
				{
				if( Met_Coude_a_45(this, DC, ptnewpiste) != 0 )
					nbptnewpiste++;
				}
			Track = ptnewpiste->Copy();
			Track->Insert(m_Pcb, ptnewpiste);

			Track->SetState(BEGIN_ONPAD|END_ONPAD,OFF);
			ptnewpiste->end = Locate_Pad_Connecte(m_Pcb, ptnewpiste, END);
			if( ptnewpiste->end )
				{
				ptnewpiste->SetState(END_ONPAD,ON);
				Track->SetState(BEGIN_ONPAD,ON);
				}
			Track->start = ptnewpiste->end;

			ptnewpiste = Track;
			ptnewpiste->m_Flags = IS_NEW;
			nbptnewpiste++ ;
			ptnewpiste->m_Start = ptnewpiste->m_End;
			ptnewpiste->m_Layer = GetScreen()->m_Active_Layer;
			ptnewpiste->m_Width = g_DesignSettings.m_CurrentTrackWidth ;
			/* affiche nouvelle position */
			Montre_Position_New_Piste(DrawPanel, DC, FALSE);
			}
		Affiche_Infos_Piste(this, ptnewpiste) ;
		}

	return ptnewpiste;
}


/**************************************************************************/
int Met_Coude_a_45(WinEDA_BasePcbFrame * frame, wxDC * DC, TRACK * pt_segm)
/***************************************************************************/
/* rectifie un virage a 90 et le modifie par 2 coudes a 45
	n'opere que sur des segments horizontaux ou verticaux.

	entree : pointeur sur le segment qui vient d'etre trace
			On suppose que le segment precedent est celui qui a ete
			precedement trace
	retourne:
		1 si ok
		0 si impossible
*/
{
TRACK * Previous;
TRACK *NewTrack;
int pas_45;
int dx0, dy0, dx1, dy1 ;

	if(nbptnewpiste < 2 ) return(0) ;	/* il faut au moins 2 segments */

	Previous = (TRACK*)pt_segm->Pback;	  // pointe le segment precedent

	// Test s'il y a 2 segments consecutifs a raccorder
	if( (pt_segm->m_StructType != TYPETRACK ) ||
		(Previous->m_StructType != TYPETRACK) )
		{
		return(0) ;
		}

	pas_45 = frame->GetScreen()->GetGrid().x / 2 ;
	if( pas_45 < pt_segm->m_Width )
		pas_45 = frame->GetScreen()->GetGrid().x;
	while(pas_45 < pt_segm->m_Width) pas_45 *= 2;

	// OK : tst si les segments sont a 90 degre et vertic ou horiz
	dx0 = Previous->m_End.x - Previous->m_Start.x;
	dy0 = Previous->m_End.y - Previous->m_Start.y;
	dx1 = pt_segm->m_End.x - pt_segm->m_Start.x;
	dy1 = pt_segm->m_End.y - pt_segm->m_Start.y;
	// les segments doivent etre de longueur suffisante:
	if(max(abs(dx0),abs(dy0)) < (pas_45*2) ) return(0);
	if(max(abs(dx1),abs(dy1)) < (pas_45*2) ) return(0);

	/* creation du nouveau segment, raccordant des 2 segm: */
	NewTrack = pt_segm->Copy();

	NewTrack->m_Start.x = Previous->m_End.x;
	NewTrack->m_Start.y = Previous->m_End.y;
	NewTrack->m_End.x = pt_segm->m_Start.x;
	NewTrack->m_End.y = pt_segm->m_Start.y;

	if( dx0 == 0 )	// Segment precedent Vertical
		{
		if(dy1 != 0 )	// les 2 segments ne sont pas a 90 ;
			{
			delete NewTrack;
			return(0);
			}

		/* Calcul des coordonnees du point de raccord :
		le nouveau segment raccorde le 1er segment Vertical
		au 2eme segment Horizontal */

		if(dy0 > 0 ) NewTrack->m_Start.y -= pas_45 ;
		else		 NewTrack->m_Start.y += pas_45 ;

		if(dx1 > 0 ) NewTrack->m_End.x += pas_45 ;
		else		 NewTrack->m_End.x -= pas_45 ;

		if ( Drc_On && (Drc(frame, DC, pt_segm, frame->m_Pcb->m_Track, 1) == BAD_DRC) )
			{
			delete NewTrack;
			return(0) ;
			}

		Previous->m_End = NewTrack->m_Start;
		pt_segm->m_Start = NewTrack->m_End;
		NewTrack->Insert(frame->m_Pcb, Previous);
		return(1) ;
		}

	if (dy0 == 0 )		// Segment precedent Horizontal : dy0 = 0
		{
		if (dx1 != 0 )	// les 2 segments ne sont pas a 90 ;
			{
			delete NewTrack;
			return(0);
			}

		// Segments a 90
		/* Modif des coordonnees du point de raccord :
			un nouveau segment a ete cree, raccordant le 1er segment Horizontal
		au 2eme segment Vertical */

		if(dx0 > 0 ) NewTrack->m_Start.x -= pas_45 ;
		else		 NewTrack->m_Start.x += pas_45 ;

		if(dy1 > 0 ) NewTrack->m_End.y += pas_45 ;
		else		 NewTrack->m_End.y -= pas_45 ;

		if ( Drc_On && (Drc(frame, DC, NewTrack, frame->m_Pcb->m_Track, 1) == BAD_DRC) )
			{
			delete NewTrack;
			return(0);
			}

		Previous->m_End = NewTrack->m_Start;
		pt_segm->m_Start = NewTrack->m_End;
		NewTrack->Insert(frame->m_Pcb, Previous);
		return(1) ;
		}

	return(0);
}


/**************************************************************/
void WinEDA_PcbFrame::End_Route(TRACK * track, wxDC * DC)
/*************************************************************/
/*
	Routine de fin de trace d'une piste (succession de segments)
*/
// ATTENTION a modifier pour que Track soit la piste à terminer
{
D_PAD * pt_pad;
TRACK * pt_track;
int masquelayer = g_TabOneLayerMask[GetScreen()->m_Active_Layer];
wxPoint pos;
EDA_BaseStruct * LockPoint;
TRACK * adr_buf;

	if( track == NULL ) return;

	if ( Drc_On && ( Drc(this, DC,ptnewpiste,m_Pcb->m_Track,1 ) == BAD_DRC) )
		return ;

	/* Sauvegarde des coord du point terminal de la piste */
	pos = ptnewpiste->m_End;

	if ( Begin_Route(track, DC) == NULL ) return;

	Montre_Position_New_Piste(DrawPanel, DC, TRUE);  /* mise a jour trace reel */
	Montre_Position_New_Piste(DrawPanel, DC, FALSE);  /* efface trace piste*/
	trace_ratsnest_pad(DC);		  /* efface trace chevelu*/

	/* La piste est ici non chainee a la liste des segments de piste.
		Il faut la replacer dans la zone de net,
		le plus pres possible du segment d'attache ( ou de fin ), car
		ceci contribue a la reduction du temps de calcul */

	/* Rappel:
	Il y a en principe au moins 2 segments.
	Le dernier segment genere est de longueur tj nulle donc inutile,
	et sera donc supprime ou utilise pour amener la piste sur un point d'ancrage */
	/* Accrochage de la fin de la piste */
	LockPoint = LocateLockPoint(m_Pcb, pos, masquelayer);

	if ( LockPoint ) /* La fin de la piste est sur un PAD */
		{
		if( LockPoint->m_StructType ==  TYPEPAD )
			{
			pt_pad = (D_PAD *) LockPoint;
			/* la fin de la piste est remise sur le centre du pad si necessaire*/
			if( (ptnewpiste->m_End.x != pt_pad->m_Pos.x ) ||
				(ptnewpiste->m_End.y != pt_pad->m_Pos.y) )
				{ /* nouveau segment cree : le segment nul de fin de piste est utilise*/
				ptnewpiste->m_End.x = pt_pad->m_Pos.x;
				ptnewpiste->m_End.y = pt_pad->m_Pos.y;
				}
			else
				{
				/* le dernier segment sera supprime ( si plusieurs segments ) */
				if( nbptnewpiste > 1 )
					{
					TRACK * Track;
					Track = ptnewpiste;
					ptnewpiste = (TRACK*) ptnewpiste->Pback; nbptnewpiste--;
					DeleteStructure(Track);
					}
				}
			ptnewpiste->end = pt_pad; ptnewpiste->SetState(END_ONPAD, ON);
			}

		else	/* la fin de la piste est sur une autre piste: il faudra
				peut-etre creer un point d'ancrage */
			{
			/* le dernier segment sera supprime */
			if ( nbptnewpiste > 1 )
				{
				TRACK * Track;
				Track = ptnewpiste;
				ptnewpiste = (TRACK*)ptnewpiste->Pback; nbptnewpiste--;
				DeleteStructure(Track);
				}
			adr_buf = (TRACK *) LockPoint;
			g_HightLigth_NetCode = adr_buf->m_NetCode;
			/* creation eventuelle d'un point d'accrochage */
			LockPoint = CreateLockPoint(&ptnewpiste->m_End.x, &ptnewpiste->m_End.y,
								adr_buf, ptnewpiste);
			}
		}

	else  /* Extremite de piste 'en l'air' */
		{
		/* le dernier segment est supprime */
		if ( nbptnewpiste > 1 )
			{
			TRACK * Track;
			Track = ptnewpiste;
			ptnewpiste = (TRACK*)ptnewpiste->Pback; nbptnewpiste--;
			DeleteStructure(Track);
			}
		}

	/* placement en buffer : recherche de la place */
	pt_track = ptstartpiste->GetBestInsertPoint(m_Pcb);

	/* Pt_track pointe ici un point d'insertion possible */
	ptstartpiste->Insert(m_Pcb, pt_track);

	trace_ratsnest_pad(DC);
	Trace_Une_Piste(DrawPanel, DC, ptstartpiste, nbptnewpiste,GR_OR) ;

	// Reset flags:
	TRACK * ptr = ptstartpiste; int ii;
	for ( ii = 0; (ptr != NULL) && (ii < nbptnewpiste) ; ii++ )
		{
		ptr->m_Flags = 0; ptr = (TRACK *) ptr->Pnext;
		}

	/* Effacement automatique de la piste eventuellement redondante */
	if(g_AutoDeleteOldTrack)
		{
		EraseOldTrack(this, m_Pcb, DC, ptstartpiste, nbptnewpiste);
		}

	test_1_net_connexion(DC, ptstartpiste->m_NetCode );

	GetScreen()->SetModify();
	Affiche_Infos_Status_Pcb(this);

	ptstartpiste = NULL;

	if(g_HightLigt_Status) Hight_Light(DC);

	g_HightLigth_NetCode = OldNetCodeSurbrillance;
	if(OldEtatSurbrillance) Hight_Light(DC);

	GetScreen()->ManageCurseur = NULL;
	GetScreen()->ForceCloseManageCurseur = NULL;
	GetScreen()->m_CurrentItem = NULL;
}


/****************************************************************************/
void Montre_Position_New_Piste(WinEDA_DrawPanel * panel,wxDC * DC, bool erase)
/****************************************************************************/
/* redessin du contour de la piste  lors des deplacements de la souris
	Cette routine est utilisee comme .ManageCurseur()
	si ShowIsolDuringCreateTrack_Item.State == RUN la marge d'isolation
	est aussi affichee
*/
{
int IsolTmp, Track_fill_copy;
PCB_SCREEN * screen = (PCB_SCREEN *) panel->GetScreen();

	Track_fill_copy = DisplayOpt.DisplayPcbTrackFill;
	DisplayOpt.DisplayPcbTrackFill = SKETCH;
	IsolTmp = DisplayOpt.DisplayTrackIsol;

	if ( g_ShowIsolDuringCreateTrack)
		DisplayOpt.DisplayTrackIsol = TRUE;

	/* efface ancienne position si elle a ete deja dessinee */
	if( erase )
		{
		Trace_Une_Piste(panel, DC, ptstartpiste,nbptnewpiste,GR_XOR) ;
		((WinEDA_BasePcbFrame*)(panel->m_Parent))->trace_ratsnest_pad( DC);
		}

	/* dessin de la nouvelle piste : mise a jour du point d'arrivee */
	ptnewpiste->m_Layer = screen->m_Active_Layer;
	ptnewpiste->m_Width = g_DesignSettings.m_CurrentTrackWidth;

	if (Track_45_Only)
		{/* Calcul de l'extremite de la piste pour orientations permises:
										horiz,vertical ou 45 degre */
		Calcule_Coord_Extremite_45(ptnewpiste->m_Start.x,ptnewpiste->m_Start.y,
						&ptnewpiste->m_End.x, &ptnewpiste->m_End.y);
		}

	else	/* ici l'angle d'inclinaison est quelconque */
		{
		ptnewpiste->m_End = screen->m_Curseur;
		}

	Trace_Une_Piste(panel, DC, ptstartpiste,nbptnewpiste,GR_XOR) ;

	DisplayOpt.DisplayTrackIsol = IsolTmp;
	DisplayOpt.DisplayPcbTrackFill = Track_fill_copy ;

	((WinEDA_BasePcbFrame*)(panel->m_Parent))->
		build_ratsnest_pad(NULL, ptnewpiste->m_End.x, ptnewpiste->m_End.y, 0);
	((WinEDA_BasePcbFrame*)(panel->m_Parent))->trace_ratsnest_pad(DC);
}


/*************************************************************************/
void WinEDA_PcbFrame::ExChange_Track_Layer(TRACK *pt_segm, wxDC * DC)
/*************************************************************************/
/*
 change de couche la piste pointee par la souris :
	la piste doit etre sur une des couches de travail,
	elle est mise sur l'autre couche de travail, si cela est possible
	(ou si DRC = Off ).
*/
{
int ii;
TRACK *pt_track;
int l1, l2 , nb_segm;

	if ( (pt_segm == NULL ) || ( pt_segm->m_StructType == TYPEZONE ) )
		{
		return;
		}

	l1 = Route_Layer_TOP; l2 = Route_Layer_BOTTOM;

	pt_track = Marque_Une_Piste(this, DC, pt_segm, &nb_segm, GR_XOR);

	/* effacement du flag BUSY et sauvegarde en membre .param de la couche
	initiale */
	ii = nb_segm; pt_segm = pt_track;
	for (; ii > 0; ii -- , pt_segm = (TRACK*)pt_segm->Pnext)
		{
		pt_segm->SetState(BUSY,OFF);
		pt_segm->m_Param = pt_segm->m_Layer;	/* pour sauvegarde */
		}

	ii = 0; pt_segm = pt_track;
	for ( ; ii < nb_segm; ii++, pt_segm = (TRACK*) pt_segm->Pnext )
		{
		if( pt_segm->m_StructType == TYPEVIA) continue;

		/* inversion des couches */
		if( pt_segm->m_Layer == l1 ) pt_segm->m_Layer = l2 ;
		else if(pt_segm->m_Layer == l2 ) pt_segm->m_Layer = l1 ;

		if( (Drc_On) && ( Drc(this, DC,pt_segm,m_Pcb->m_Track,1) == BAD_DRC ) )
			{	/* Annulation du changement */
			ii = 0; pt_segm = pt_track;
			for ( ; ii < nb_segm; ii++, pt_segm = (TRACK*) pt_segm->Pnext )
				{
				pt_segm->m_Layer = pt_segm->m_Param;
				}
			Trace_Une_Piste(DrawPanel, DC, pt_track, nb_segm, GR_OR);
			DisplayError(this, "Drc erreur, operation annulee", 10);
			return;
			}
		}
	Trace_Une_Piste(DrawPanel, DC, pt_track, nb_segm, GR_OR | GR_SURBRILL);
	/* controle des extremites de segments: sont-ils sur un pad */
	ii = 0; pt_segm = pt_track;
	for(; ii < nb_segm; pt_segm = (TRACK*)pt_segm->Pnext, ii++)
		{
		pt_segm->start = Locate_Pad_Connecte(m_Pcb, pt_segm, START);
		pt_segm->end = Locate_Pad_Connecte(m_Pcb, pt_segm, END);
		}
	test_1_net_connexion(DC, pt_track->m_NetCode );
	Affiche_Infos_Piste(this, pt_track) ;
	GetScreen()->SetModify();
}

/****************************************************************/
void WinEDA_PcbFrame::Other_Layer_Route(TRACK * track, wxDC * DC)
/****************************************************************/
/*
	Change de couche active pour le routage.
	Si une piste est en cours de trace : placement d'une Via
*/
{
TRACK * pt_segm;
SEGVIA * Via;
int ii;
int itmp;

	if(track == NULL)
		{
		if(GetScreen()->m_Active_Layer != GetScreen()->m_Route_Layer_TOP)
			 GetScreen()->m_Active_Layer = GetScreen()->m_Route_Layer_TOP;
		else GetScreen()->m_Active_Layer = GetScreen()->m_Route_Layer_BOTTOM ;
		Affiche_Status_Box();
		SetToolbars();
		return;
		}

	/* Les vias ne doivent pas etre inutilement empilees: */
	if( Locate_Via(m_Pcb, ptnewpiste->m_End.x,ptnewpiste->m_End.y,ptnewpiste->m_Layer))
		return;
	pt_segm = ptstartpiste;
	for ( ii = 0; ii < nbptnewpiste-1 ; ii++, pt_segm = (TRACK*)pt_segm->Pnext)
		{
		if( (pt_segm->m_StructType == TYPEVIA) &&
			(ptnewpiste->m_End.x == pt_segm->m_Start.x) &&
			(ptnewpiste->m_End.y == pt_segm->m_Start.y) )
				return;
		}
	/* Test si segment possible a placer */
	if ( Drc_On )
		if ( Drc(this, DC,ptnewpiste,m_Pcb->m_Track,1) == BAD_DRC )
			return ;

	/* save etat actuel pour regeneration si via impossible a placer */
	itmp = nbptnewpiste;
	Begin_Route(ptnewpiste, DC);

	GetScreen()->ManageCurseur(DrawPanel, DC, FALSE);

	Via = new SEGVIA(m_Pcb);
	Via->m_Flags = IS_NEW;
	Via->m_Width = g_DesignSettings.m_CurrentViaSize;
	Via->m_Shape = g_DesignSettings.m_CurrentViaType ;
	Via->m_NetCode = g_HightLigth_NetCode ;
	Via->m_Start.x = Via->m_End.x = ptnewpiste->m_Start.x;
	Via->m_Start.y = Via->m_End.y = ptnewpiste->m_Start.y;

	Via->m_Layer = GetScreen()->m_Active_Layer;	// Provisoirement

	if( GetScreen()->m_Active_Layer != GetScreen()->m_Route_Layer_TOP)
			GetScreen()->m_Active_Layer = GetScreen()->m_Route_Layer_TOP ;
	else	GetScreen()->m_Active_Layer = GetScreen()->m_Route_Layer_BOTTOM ;

	if ( (Via->m_Shape & 15) == VIA_ENTERREE )
	{
		Via->m_Layer |= GetScreen()->m_Active_Layer << 4;
	}
	else if ( (Via->m_Shape & 15) == VIA_BORGNE )
	{	// A revoir! ( la via devrai deboucher sur 1 cote )
		Via->m_Layer |= GetScreen()->m_Active_Layer << 4;
	}
	else Via->m_Layer = 0x0F;

	if ( Drc_On &&( Drc(this, DC,Via,m_Pcb->m_Track,1 ) == BAD_DRC ) )
		{ /* Via impossible a placer ici */
		delete Via;
		GetScreen()->m_Active_Layer = ptnewpiste->m_Layer ;
		GetScreen()->ManageCurseur(DrawPanel, DC, FALSE);
		return;
		}

	/* la via est OK et est inseree avant le segment courant */
	Via->Pnext = ptnewpiste;
	Via->Pback = ptnewpiste->Pback;
	ptnewpiste->Pback = Via;
	if ( Via->Pback == NULL ) ptstartpiste = Via;
	else Via->Pback->Pnext = Via;
	nbptnewpiste++;

	GetScreen()->ManageCurseur(DrawPanel, DC, FALSE);
	Affiche_Infos_Piste(this, Via);

	Affiche_Status_Box();
	SetToolbars();
}


/*************************************************/
void WinEDA_PcbFrame::Affiche_Status_Net(wxDC * DC)
/*************************************************/
/* Affiche:
	le status du net en haut d'ecran du segment pointe par la souris
	ou le status PCB en bas d'ecran si pas de segment pointe
*/
{
TRACK * pt_segm;
int masquelayer = g_TabOneLayerMask[GetScreen()->m_Active_Layer];

	pt_segm = Locate_Pistes(m_Pcb->m_Track, masquelayer, CURSEUR_OFF_GRILLE);
	if(pt_segm == NULL)
		{
		Affiche_Infos_Status_Pcb(this);
		}
	else
		{
		test_1_net_connexion(DC, pt_segm->m_NetCode);
		}
}


/*****************************************************************/
void Calcule_Coord_Extremite_45(int ox, int oy, int* fx, int* fy )
/*****************************************************************/
/* determine les parametres .fx et .fy du segment pointe par pt_segm
	pour avoir un segment oriente a 0, 90 ou 45 degres, selon position
	du oint d'origine et de la souris
*/
{
int deltax, deltay, angle;

	deltax = ActiveScreen->m_Curseur.x - ox ;
	deltay = ActiveScreen->m_Curseur.y - oy ;
	/* calcul de l'angle preferentiel : 0, 45 , 90 degre */
	deltax = abs(deltax) ; deltay = abs(deltay) ; angle = 45 ;
	if( deltax >= deltay )
		{
		if ( deltax == 0 ) angle = 0 ;
		else if( ((deltay << 6 )/ deltax ) < 26 ) angle = 0 ;
		}
	else
		{
		angle = 45 ;
		if (deltay == 0 ) angle = 90 ;
		else if( ((deltax << 6 )/ deltay ) < 26 ) angle = 90 ;
		}

	switch ( angle )
		{
		case 0 :
			*fx = ActiveScreen->m_Curseur.x;
			*fy = oy ;
			break ;

		case 45 :
			deltax = min(deltax,deltay) ; deltay = deltax ;
			/* recalcul des signes de deltax et deltay */
			if( (ActiveScreen->m_Curseur.x - ox) < 0 ) deltax = -deltax;
			if( (ActiveScreen->m_Curseur.y - oy) < 0 ) deltay = -deltay;
			*fx = ox + deltax ;
			*fy = oy + deltay ;
			break ;

		case 90 :
			*fx = ox ;
			*fy = ActiveScreen->m_Curseur.y;
			break ;
		}
}


/**********************************************************************/
void WinEDA_PcbFrame::Show_1_Ratsnest(EDA_BaseStruct * item, wxDC * DC)
/**********************************************************************/
/* Affiche le ratsnest relatif
	au net du pad pointe par la souris
	ou au module localise par la souris
	Efface le chevelu affiche si aucun module ou pad n'est selectionne
*/
{
int ii;
CHEVELU * pt_chevelu;
D_PAD * pt_pad = NULL;
MODULE * Module = NULL;

	if (g_Show_Ratsnest) return;	// Deja Affiché!

	if((m_Pcb->m_Status_Pcb & LISTE_CHEVELU_OK) == 0 )
		{
		Compile_Ratsnest( DC, TRUE );
		}

	if ( item && (item->m_StructType == TYPEPAD) )
		{
		pt_pad = (D_PAD*)item;
		Module = (MODULE *) pt_pad->m_Parent;
		}

	if ( pt_pad ) /* Affichage du chevelu du net correspondant */
		{
		pt_pad->Display_Infos(this);
		pt_chevelu = (CHEVELU*) m_Pcb->m_Ratsnest;
		for( ii = m_Pcb->GetNumRatsnests(); ii > 0; pt_chevelu++, ii--)
			{
			if( pt_chevelu->m_NetCode == pt_pad->m_NetCode)
				{
				if( (pt_chevelu->status & CH_VISIBLE) != 0 ) continue;
				pt_chevelu->status |= CH_VISIBLE;
				if( (pt_chevelu->status & CH_ACTIF) == 0 ) continue;

				GRSetDrawMode(DC, GR_XOR);
				GRLine( &DrawPanel->m_ClipBox, DC, pt_chevelu->pad_start->m_Pos.x,
									pt_chevelu->pad_start->m_Pos.y,
									pt_chevelu->pad_end->m_Pos.x,
									pt_chevelu->pad_end->m_Pos.y,
									g_DesignSettings.m_RatsnestColor);
				}
			}
		}
	else
		{
		if ( item && (item->m_StructType == TYPEMODULE) )
			{
			Module = (MODULE*)item;
			}

		if( Module)
			{
			Module->Display_Infos(this);
			pt_pad = Module->m_Pads;
			for( ; pt_pad != NULL; pt_pad = (D_PAD*)pt_pad->Pnext)
				{
				pt_chevelu = (CHEVELU*) m_Pcb->m_Ratsnest;
				for( ii = m_Pcb->GetNumRatsnests(); ii > 0; pt_chevelu++, ii--)
					{
					if( (pt_chevelu->pad_start == pt_pad) ||
						(pt_chevelu->pad_end == pt_pad) )
						{
						if( pt_chevelu->status & CH_VISIBLE ) continue;
						pt_chevelu->status |= CH_VISIBLE;
						if( (pt_chevelu->status & CH_ACTIF) == 0 ) continue;

						GRSetDrawMode(DC, GR_XOR);
						GRLine(&DrawPanel->m_ClipBox, DC, pt_chevelu->pad_start->m_Pos.x,
									pt_chevelu->pad_start->m_Pos.y,
									pt_chevelu->pad_end->m_Pos.x,
									pt_chevelu->pad_end->m_Pos.y,
									g_DesignSettings.m_RatsnestColor);
						}
					}
				}
			pt_pad = NULL;
			}
		}

	/* Effacement complet des selections
		si aucun pad ou module n'a ete localise */
	if( (pt_pad == NULL) && (Module == NULL) )
		{
		MsgPanel->EraseMsgBox();
		DrawGeneralRatsnest(DC);
		pt_chevelu = (CHEVELU*) m_Pcb->m_Ratsnest;
		for( ii = m_Pcb->GetNumRatsnests();(ii > 0) && pt_chevelu; pt_chevelu++, ii--)
			pt_chevelu->status &= ~CH_VISIBLE;
		}
}


/*****************************************************/
void WinEDA_PcbFrame::Affiche_PadsNoConnect(wxDC * DC)
/*****************************************************/
/* Met en surbrillance les pads non encore connectes ( correspondants aux
chevelus actifs
*/
{
int ii;
CHEVELU * pt_chevelu;
D_PAD * pt_pad;

	pt_chevelu = (CHEVELU*)m_Pcb->m_Ratsnest;
	for( ii = m_Pcb->GetNumRatsnests();ii > 0; pt_chevelu++, ii--)
		{
		if( (pt_chevelu->status & CH_ACTIF) == 0 ) continue;
		pt_pad = pt_chevelu->pad_start;

		if (pt_pad)
			pt_pad->Draw(DrawPanel,DC, wxPoint(0,0), GR_OR | GR_SURBRILL);

		pt_pad = pt_chevelu->pad_end;
		if (pt_pad)
			pt_pad->Draw(DrawPanel,DC, wxPoint(0,0), GR_OR | GR_SURBRILL);
		}
}

