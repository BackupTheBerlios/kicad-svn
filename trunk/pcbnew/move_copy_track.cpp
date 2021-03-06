		/****************************************************/
		/* 				Edition des pistes					*/
		/* Routines de duplication et deplacement de pistes */
		/****************************************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "pcbnew.h"
#include "autorout.h"

#include "drag.h"

#include "protos.h"


/* Routines externes */

static void Show_MoveTrack(WinEDA_DrawPanel * panel, wxDC * DC, bool erase);
static void Exit_MoveTrack(WinEDA_DrawFrame * frame, wxDC *DC);

#if 0
/* Routines Locales */
static void Duplic_Track(COMMAND * Cmd);
static void Place_Dupl_Route(COMMAND * Cmd);
#endif

/* variables locales */
static wxPoint PosInit, LastPos;
static TRACK * NewTrack;	/* Nouvelle piste creee ou piste deplacee */
static int NbPtNewTrack;
static int Old_HightLigth_NetCode;
static bool Old_HightLigt_Status;


/**************************************************************/
static void Exit_MoveTrack(WinEDA_DrawFrame * frame, wxDC *DC)
/***************************************************************/
/* routine d'annulation de la commande drag, copy ou move track  si une piste est en cours
	de tracage, ou de sortie de l'application EDITRACK.
	Appel par la touche ESC
 */
{
TRACK * NextS;
int ii;


	/* Effacement du trace en cours */
	wxPoint oldpos = frame->GetScreen()->m_Curseur;
	frame->GetScreen()->m_Curseur = PosInit;
	frame->GetScreen()->ManageCurseur(frame->DrawPanel, DC, TRUE);
	frame->GetScreen()->m_Curseur = oldpos;
	g_HightLigt_Status = FALSE;
	( (WinEDA_PcbFrame *)frame)->DrawHightLight( DC, g_HightLigth_NetCode) ;

	if( NewTrack )
	{
		if (NewTrack->m_Flags & IS_NEW )
		{
			for( ii = 0; ii < NbPtNewTrack; ii++, NewTrack = NextS)
			{
				if(NewTrack == NULL) break;
				NextS = (TRACK*) NewTrack->Pnext;
				delete NewTrack;
			}
		}
		else	/* Move : remise en ancienne position */
		{
			TRACK * Track = NewTrack;
			int dx = LastPos.x - PosInit.x;
			int dy = LastPos.y - PosInit.y;
			for( ii = 0; ii < NbPtNewTrack; ii++, Track = (TRACK*) Track->Pnext)
			{
				if( Track == NULL ) break;
				Track->m_Start.x -= dx;
				Track->m_Start.y -= dy;
				Track->m_End.x -= dx;
				Track->m_End.y -= dy;
				Track->m_Flags = 0;

			}
			Trace_Une_Piste(frame->DrawPanel, DC, NewTrack,NbPtNewTrack,GR_OR);
		}

		NewTrack = NULL;
	}

	frame->GetScreen()->ManageCurseur = NULL;
	frame->GetScreen()->ForceCloseManageCurseur = NULL;
	frame->GetScreen()->m_CurrentItem = NULL;
	frame->EraseMsgBox();

	/* Annulation deplacement et Redessin des segments dragges */
	DRAG_SEGM * pt_drag = g_DragSegmentList;
	for( ; pt_drag != NULL; pt_drag = pt_drag->Pnext)
	{
		TRACK * Track = pt_drag->m_Segm;
		pt_drag->SetInitialValues();
		Track->SetState(EDIT,OFF);
		Track->m_Flags = 0;
		Track->Draw(frame->DrawPanel, DC, GR_OR);
	}

	g_HightLigth_NetCode = Old_HightLigth_NetCode;
	g_HightLigt_Status = Old_HightLigt_Status;
	if(g_HightLigt_Status)
		( (WinEDA_PcbFrame *)frame)->DrawHightLight( DC, g_HightLigth_NetCode) ;

	EraseDragListe();
}



/*************************************************************************/
static void Show_MoveTrack(WinEDA_DrawPanel * panel, wxDC * DC, bool erase)
/*************************************************************************/
/* redessin du contour de la piste lors des deplacements de la souris */
{
int ii, dx, dy;
TRACK * Track;
BASE_SCREEN * screen = panel->GetScreen();
int track_fill_copy = DisplayOpt.DisplayPcbTrackFill;

	DisplayOpt.DisplayPcbTrackFill = SKETCH ;

erase = TRUE;

	/* efface ancienne position si elle a ete deja dessinee */
	if( erase )
	{
		if ( NewTrack ) Trace_Une_Piste(panel, DC, NewTrack,NbPtNewTrack,GR_XOR) ;
	}

	/* mise a jour des coordonnees des segments de la piste */
	wxPoint Pos = screen->m_Curseur;
	dx = Pos.x - LastPos.x;
	dy = Pos.y - LastPos.y;
	LastPos = Pos;
	ii = NbPtNewTrack, Track = NewTrack;
	for( ; ii > 0; ii--, Track = Track->Next() )
	{
		if( Track->m_Flags & STARTPOINT)
		{
			Track->m_Start.x += dx; Track->m_Start.y += dy;
		}
		if( Track->m_Flags & ENDPOINT)
		{
			Track->m_End.x += dx; Track->m_End.y += dy;
		}
	}

	/* dessin de la nouvelle piste */
	Trace_Une_Piste(panel, DC, NewTrack,NbPtNewTrack,GR_XOR) ;

	/* Tracage des segments dragges */
	DRAG_SEGM * pt_drag = g_DragSegmentList;
	for( ; pt_drag != NULL; pt_drag = pt_drag->Pnext)
	{
		Track = pt_drag->m_Segm;
		if ( erase ) Track->Draw(panel, DC, GR_XOR);
		if( Track->m_Flags & STARTPOINT)
		{
			Track->m_Start.x += dx; Track->m_Start.y += dy;
		}
		if( Track->m_Flags & ENDPOINT)
		{
			Track->m_End.x += dx; Track->m_End.y += dy;
		}
		Track->Draw(panel, DC, GR_XOR);
	}
	DisplayOpt.DisplayPcbTrackFill = track_fill_copy ;
}



/***********************************************************************************/
void WinEDA_PcbFrame::Start_MoveOneTrackSegment(TRACK * track, wxDC * DC, bool Drag)
/***********************************************************************************/
{
	/* Change hight light net: the new one will be hightlighted */
	Old_HightLigt_Status = g_HightLigt_Status;
	Old_HightLigth_NetCode = g_HightLigth_NetCode;
	if(g_HightLigt_Status) Hight_Light(DC);

	if ( Drag && track->m_StructType == TYPEVIA)
	{
		track->m_Flags = IS_DRAGGED|STARTPOINT|ENDPOINT;
		Collect_TrackSegmentsToDrag(DrawPanel, DC, track->m_Start,
			track->ReturnMaskLayer(), track->m_NetCode);
		NewTrack = track;
		NbPtNewTrack = 1;
		PosInit = track->m_Start;
	}

	else
	{
		int diag = track->IsPointOnEnds(GetScreen()->m_Curseur, -1);
		wxPoint pos = (diag & STARTPOINT) ? track->m_Start : track->m_End;
		Collect_TrackSegmentsToDrag(DrawPanel, DC, pos,
			track->ReturnMaskLayer(), track->m_NetCode);
		track->m_Flags |= IS_DRAGGED;
		NewTrack = NULL;
		NbPtNewTrack = 0;
		PosInit = pos;
	}
	LastPos = PosInit;
	GetScreen()->ManageCurseur = Show_MoveTrack;
	GetScreen()->ForceCloseManageCurseur = Exit_MoveTrack;

	g_HightLigth_NetCode = track->m_NetCode;
	g_HightLigt_Status = TRUE;
	DrawHightLight( DC, g_HightLigth_NetCode) ;
	GetScreen()->ManageCurseur(DrawPanel, DC, TRUE);
}



/**********************************************************************/
bool WinEDA_PcbFrame::PlaceDraggedTrackSegment(TRACK * Track, wxDC * DC)
/**********************************************************************/
/* Place a dragged track segment or via */
{
int errdrc;
DRAG_SEGM * pt_drag;

	if(Track == NULL ) return FALSE;

	// DRC control:
	if(Drc_On)
	{
		errdrc = Drc(this, DC, Track, m_Pcb->m_Track,1);
		if(errdrc == BAD_DRC) return FALSE;
		/* Tracage des segments dragges */
		pt_drag = g_DragSegmentList;
		for( ; pt_drag; pt_drag = pt_drag->Pnext)
		{
			errdrc = Drc(this, DC, pt_drag->m_Segm, m_Pcb->m_Track,1);
			if(errdrc == BAD_DRC) return FALSE;
		}
	}

	// DRC Ok: place track segments
	Track->m_Flags = 0;
	Track->Draw(DrawPanel, DC, GR_OR);

	/* Tracage des segments dragges */
	pt_drag = g_DragSegmentList;
	for( ; pt_drag; pt_drag = pt_drag->Pnext)
		{
		Track = pt_drag->m_Segm;
		Track->SetState(EDIT,OFF);
		Track->m_Flags = 0;
		Track->Draw(DrawPanel, DC, GR_OR);
		}
	EraseDragListe();

	GetScreen()->SetModify();
	GetScreen()->ManageCurseur = NULL;
	GetScreen()->ForceCloseManageCurseur = NULL;

	return TRUE;
}


#if 0

/***********************************************/
void WinEDA_PcbFrame::Place_Dupl_Route(Track * Track, wxDC * DC)
/******************************************/
/*
	Routine de placement d'une piste (succession de segments)
*/
{
D_PAD * pt_pad;
TRACK * pt_track, *Track, * pt_classe, *NextS;
int masquelayer;
EDA_BaseStruct * LockPoint;
int ii, old_net_code, new_net_code, DRC_error = 0;
wxDC * DC = Cmd->DC;

	ActiveScreen->ManageCurseur = NULL;

	if( NewTrack == NULL ) return ;

	old_net_code = NewTrack->net_code;

	/* Placement du flag BUSY de la piste originelle, qui ne doit
	pas etre vue dans les recherches de raccordement suivantes */
	ii = NbPtNewTrack; pt_track = NewTrack;
	for ( ; ii > 0; ii --, pt_track = (TRACK*) pt_track->Pnext)
		{
		pt_track->SetState(BUSY, ON);
		}

	/* Detection du nouveau net_code */
	ii = NbPtNewTrack; pt_track = NewTrack;
	for ( ; ii > 0; ii --, pt_track = (TRACK*) pt_track->Pnext)
		{
		pt_track->net_code = 0;
		}

	new_net_code = 0;
	ii = 0; pt_track = NewTrack;
	for( ; ii < NbPtNewTrack ; ii++, pt_track = (TRACK*)pt_track->Pnext)
		{
		/* Localisation de la pastille ou segment en debut de segment: */
		masquelayer = tab_layer[pt_track->Layer];
		LockPoint = LocateLockPoint(pt_track->m_Start.x,pt_track->m_Start.y,masquelayer);
		if( LockPoint )
			{
			if ( LockPoint->m_StructType == TYPEPAD )
				{
				pt_pad = (D_PAD*) LockPoint;
				new_net_code = pt_pad->net_code;
				if ( new_net_code > 0 ) break;
				}
			else		/* debut de piste sur un segment de piste */
				{
				Track = (TRACK *) LockPoint;
				new_net_code = Track->net_code;
				if ( new_net_code > 0 ) break;
				}
			}
		LockPoint = LocateLockPoint(pt_track->m_End.x,pt_track->m_End.y,masquelayer);
		if( LockPoint )
			{
			if ( LockPoint->m_StructType == TYPEPAD )
				{
				pt_pad = (D_PAD*) LockPoint;
				new_net_code = pt_pad->net_code;
				if ( new_net_code > 0 ) break;
				}
			else		/* debut de piste sur un segment de piste */
				{
				Track = (TRACK *) LockPoint;
				new_net_code = Track->net_code;
				if ( new_net_code > 0 ) break;
				}
			}
		}

	/* Mise a jour du nouveau net code de la piste */
	ii = 0; pt_track = NewTrack;
	for( ; ii < NbPtNewTrack; ii++, pt_track = (TRACK*)pt_track->Pnext)
		{
		pt_track->net_code = new_net_code;
		}

	/* Controle DRC de la nouvelle piste */
	ii = 0; pt_track = NewTrack;
	for( ; ii < NbPtNewTrack; ii++, pt_track = pt_track->Next() )
		{
		if( Drc_On == RUN )
		  if( drc(DC, pt_track, pt_pcb->Track, 1) == BAD_DRC )
			{
			if( confirmation(" Erreur DRC, Place piste:") == YES ) continue;
			else { DRC_error = 1; break; }
			}
		}

	if( DRC_error == 0)
		{
		if(FlagState == MOVE_ROUTE)
			{
			/* copie nouvelle piste */
			pt_track = NewTrack;
			NewTrack = pt_track->Copy(NbPtNewTrack);
			/* effacement ancienne ( chainage et liens mauvais */
			ii = NbPtNewTrack;
			for ( ; ii > 0; ii --, pt_track = NextS)
				{
				NextS = (TRACK*) pt_track->Pnext;
				DeleteStructure(pt_track);
				}
			test_1_net_connexion(DC, old_net_code );
			}

		pt_classe = NewTrack->GetBestInsertPoint();
		NewTrack->Insert(pt_classe);

		Trace_Une_Piste(DC, NewTrack,NbPtNewTrack,GR_OR) ;

		/* Mise a jour des connexions sur pads et sur pistes */
		ii = 0; pt_track = NewTrack;
		for( ; ii < NbPtNewTrack; ii++, pt_track = NextS)
			{
			NextS = (TRACK*)pt_track->Pnext;
			pt_track->SetState(BEGIN_ONPAD|END_ONPAD, OFF);
			masquelayer = tab_layer[pt_track->Layer];

			/* Localisation de la pastille ou segment sur debut segment: */
			LockPoint = LocateLockPoint(pt_track->m_Start.x,pt_track->m_Start.y,masquelayer);
			if( LockPoint )
				{
				pt_track->start = LockPoint;
				if ( LockPoint->m_StructType == TYPEPAD )
					{	/* fin de piste sur un pad */
					pt_pad = (D_PAD*) LockPoint;
					pt_track->SetState(BEGIN_ONPAD, ON);
					}
				else		/* debut de piste sur un segment de piste */
					{
					Track = (TRACK *) LockPoint;
					CreateLockPoint(&pt_track->m_Start.x,&pt_track->m_Start.y,Track,pt_track);
					}
				}

			/* Localisation de la pastille ou segment sur fin de segment: */
			LockPoint = LocateLockPoint(pt_track->m_End.x,pt_track->m_End.y,masquelayer);
			if( LockPoint )
				{
				pt_track->end = LockPoint;
				if ( LockPoint->m_StructType == TYPEPAD )
					{	/* fin de piste sur un pad */
					pt_pad = (D_PAD*) LockPoint;
					pt_track->SetState(END_ONPAD, ON);
					}
				else		/* debut de piste sur un segment de piste */
					{
					Track = (TRACK *) LockPoint;
					CreateLockPoint(&pt_track->m_Start.x,&pt_track->m_Start.y,Track,pt_track);
					}
				}
			}

		/* Suppression du flag BUSY */
		ii = NbPtNewTrack; pt_track = NewTrack;
		for ( ; ii > 0; ii --, pt_track = (TRACK*) pt_track->Pnext)
			{
			pt_track->SetState(BUSY, OFF);
			}

		test_1_net_connexion(DC, new_net_code );
		ActiveScreen->SetModify();
		}

	else	/* Erreur DRC: Annulation commande */
		{
		DisplayOpt.DisplayPcbTrackFill = SKETCH ;
		Trace_Une_Piste(DC, NewTrack,NbPtNewTrack,GR_XOR);
		DisplayOpt.DisplayPcbTrackFill = Track_fill_copy ;

		if(FlagState == MOVE_ROUTE)
			{	/* Remise en position de la piste deplacee */
			Track = NewTrack;
			PosInitX -= Track->m_Start.x; PosInitY -= Track->m_Start.y;
			for( ii = 0; ii < NbPtNewTrack; ii++, Track = (TRACK*) Track->Pnext)
				{
				if( Track == NULL ) break;
				Track->m_Start.x += PosInitX; Track->m_Start.y += PosInitY;
				Track->m_End.x += PosInitX; Track->m_End.y += PosInitY;
				Track->SetState(BUSY,OFF);
				}
			Trace_Une_Piste(DC, NewTrack,NbPtNewTrack,GR_OR);
			}

		if (FlagState == COPY_ROUTE )
			{	/* Suppression copie */
			for( ii = 0; ii < NbPtNewTrack; NewTrack = NextS)
				{
				if(NewTrack == NULL) break;
				NextS = (TRACK*) NewTrack->Pnext;
				delete NewTrack;
				}
			}
		}
	NewTrack = NULL;
	Affiche_Infos_Status_Pcb(Cmd);
	if(Etat_Surbrillance) Hight_Light(DC);
}

/************************************************/
void WinEDA_PcbFrame::Start_CopieMove_Route(TRACK * track, wxDC * DC, bool Drag)
/************************************************/
/* Routine permettant la recopie d'une piste (suite de segments) deja tracee
*/
{
int ii;
TRACK *pt_segm, *pt_track;
int masquelayer = tab_layer[ActiveScreen->Active_Layer];
wxDC * DC = Cmd->DC;

	if( NewTrack ) return;

	FlagState = (int)Cmd->Menu->param_inf;

	/* Recherche de la piste sur la couche active (non zone) */
	for(pt_segm = pt_pcb->Track; pt_segm != NULL; pt_segm = (TRACK*)pt_segm->Pnext)
		{
		pt_segm = Locate_Pistes(pt_segm,masquelayer, CURSEUR_OFF_GRILLE);
		if( pt_segm == NULL ) break ;
		break ;
		}

	if( pt_segm != NULL )
		{
		if (FlagState == COPY_ROUTE )
			pt_track = Marque_Une_Piste(DC, pt_segm, &NbPtNewTrack, 0);
		else pt_track = Marque_Une_Piste(DC, pt_segm, &NbPtNewTrack, GR_XOR);

		if(NbPtNewTrack) /* Il y a NbPtNewTrack segments de piste a traiter */
			{
			/* effacement du flag BUSY de la piste originelle */
			ii = NbPtNewTrack; pt_segm = pt_track;
			for ( ; ii > 0; ii --, pt_segm = (TRACK*) pt_segm->Pnext)
				{
				pt_segm->SetState(BUSY, OFF);
				}

			if (FlagState == COPY_ROUTE )
				NewTrack = pt_track->Copy(NbPtNewTrack);
			else NewTrack = pt_track;

			Affiche_Infos_Piste(Cmd, pt_track) ;

			startX = ActiveScreen->Curseur_X;
			startY = ActiveScreen->Curseur_Y;
			Place_Dupl_Route_Item.State = WAIT;
			ActiveScreen->ManageCurseur = Show_Move_Piste;
			DisplayOpt.DisplayPcbTrackFill = SKETCH ;
			Trace_Une_Piste(DC, NewTrack,NbPtNewTrack,GR_XOR) ;
			DisplayOpt.DisplayPcbTrackFill = Track_fill_copy ;
			PosInitX = NewTrack->m_Start.x; PosInitY = NewTrack->m_Start.y;
			}
		}
}


#endif

/************************************************************************/
EDA_BaseStruct * LocateLockPoint(BOARD * Pcb, wxPoint pos, int LayerMask)
/************************************************************************/
/* Routine trouvant le point " d'accrochage " d'une extremite de piste.
	Ce point peut etre un PAD ou un autre segment de piste
	Retourne:
		- pointeur sur ce PAD ou:
		- pointeur sur le segment ou:
		- NULL
	Parametres d'appel:
	 coord pX, pY du point tst
	 masque des couches a tester
*/
{
D_PAD * pt_pad;
TRACK * ptsegm;
MODULE * Module;

	/* detection du point type PAD */
	pt_pad = NULL;
	Module = Pcb->m_Modules;
	for( ; Module != NULL; Module = (MODULE*)Module->Pnext )
		{
		pt_pad = Locate_Pads(Module, pos, LayerMask);
		if (pt_pad) return(pt_pad);
		}

	/* ici aucun pad n'a ete localise: detection d'un segment de piste */

	ptsegm = Fast_Locate_Piste( Pcb->m_Track, NULL, pos, LayerMask);
	if( ptsegm == NULL )
		ptsegm = Locate_Pistes( Pcb->m_Track, pos, LayerMask);
	return(ptsegm);
}


/******************************************************************************/
TRACK * CreateLockPoint(int *pX, int *pY, TRACK * ptsegm, TRACK * refsegm)
/******************************************************************************/
/* Routine de creation d'un point intermediaire sur un segment
	le segment ptsegm est casse en 2 segments se raccordant au point pX, pY
	retourne:
		NULL si pas de nouveau point ( c.a.d si pX, pY correspondait deja
		a une extremite ou:
		pointeur sur le segment cree
	si refsegm != NULL refsegm est pointeur sur le segment incident,
	et le point cree est l'intersection des 2 axes des segments ptsegm et
	refsegm
	retourne la valeur exacte de pX et pY
	Si ptsegm pointe sur une via:
		retourne la valeur exacte de pX et pY et ptsegm,
		mais ne cree pas de point supplementaire

*/
{
int cX, cY;
int dx, dy;			/* Coord de l'extremite du segm ptsegm / origine */
int ox, oy, fx , fy;	/* coord de refsegm / origine de prsegm */
TRACK * NewTrack;

	if( (ptsegm->m_Start.x == *pX) && (ptsegm->m_Start.y == *pY) ) return(NULL);
	if( (ptsegm->m_End.x == *pX) && (ptsegm->m_End.y == *pY) ) return(NULL);

	/* le point n'est pas sur une extremite de piste */
	if(ptsegm->m_StructType == TYPEVIA )
		{
		*pX = ptsegm->m_Start.x; *pY = ptsegm->m_Start.y;
		return(ptsegm);
		}
	/* calcul des coord vraies du point intermediaire dans le repere d'origine
	= origine de ptsegm */
	cX = *pX - ptsegm->m_Start.x;
    cY = *pY - ptsegm->m_Start.y;
	dx = ptsegm->m_End.x - ptsegm->m_Start.x;
    dy = ptsegm->m_End.y - ptsegm->m_Start.y;

// ***** A COMPLETER : non utilise
    if ( refsegm )
        {
    	ox = refsegm->m_Start.x - ptsegm->m_Start.x;
        oy = refsegm->m_Start.y - ptsegm->m_Start.y;
    	fx = refsegm->m_End.x - ptsegm->m_Start.x;
        fy = refsegm->m_End.y - ptsegm->m_Start.y;
        }

	/* pour que le point soit sur le segment ptsegm: cY/cX = dy/dx */
	if ( dx == 0 )	cX = 0; /* segm horizontal */
	else cY = (cX * dy) / dx;

	/* creation du point intermediaire ( c'est a dire creation d'un nouveau
	segment, debutant au point intermediaire */

	cX += ptsegm->m_Start.x; cY += ptsegm->m_Start.y;
	NewTrack = ptsegm->Copy();

	NewTrack->Insert(NULL, ptsegm);
	/* correction du pointeur de fin du nouveau segment */
	NewTrack->end = ptsegm->end;

	/* le segment primitif finit au nouveau point : */
	ptsegm->m_End.x = cX; ptsegm->m_End.y = cY;
	ptsegm->SetState(END_ONPAD, OFF);

	/* le nouveau segment debute au nouveau point : */
	ptsegm = NewTrack;;
	ptsegm->m_Start.x = cX; ptsegm->m_Start.y = cY;
	ptsegm->SetState(BEGIN_ONPAD, OFF);
	*pX = cX; *pY = cY;

	return(ptsegm);

}


