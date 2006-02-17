			/************************************/
			/*	Edition des pistes			*/
			/* Gestion des attributs des pistes */
			/************************************/

#include "fctsys.h"

#include "common.h"
#include "pcbnew.h"
#include "autorout.h"

#include "protos.h"


/*****************************************************************************/
void WinEDA_PcbFrame::Attribut_Segment(TRACK * track, wxDC * DC, bool Flag_On)
/*****************************************************************************/
/* Changement d'attribut de 1 segment de piste.
Les attributs sont
SEGM_FIXE		protection contre les effacements globaux
SEGM_AR			Segment AutoRouté
*/
{
	if ( track )
		{
		GetScreen()->SetModify();
		track->SetState(SEGM_FIXE, Flag_On);
		track->Draw(DrawPanel, DC, GR_OR | GR_SURBRILL) ;
		Affiche_Infos_Piste(this, track);
		}
}


/***************************************************************************/
void WinEDA_PcbFrame::Attribut_Track(TRACK * track, wxDC * DC, bool Flag_On)
/***************************************************************************/
/* Modification d'attribut de 1 piste */
{
TRACK *Track ;
int nb_segm;

	if( (track == NULL ) || (track->m_StructType == TYPEZONE) ) return;

	Track = Marque_Une_Piste(this, DC, track, & nb_segm, GR_OR | GR_SURBRILL) ;

	for( ; (Track != NULL) && (nb_segm > 0) ; nb_segm-- )
		{
		Track->SetState(SEGM_FIXE, Flag_On);
		Track->SetState(BUSY,OFF);
		Track = (TRACK*)Track->Pnext;
		}

	GetScreen()->SetModify();
}

/***********************************************************************/
void WinEDA_PcbFrame::Attribut_net(wxDC * DC, int net_code, bool Flag_On)
/***********************************************************************/
/* Change le flag SEGM_FIXE du net net_code.
si net_code < 0 tous les nets sont modifiés
*/
{
TRACK *Track = m_Pcb->m_Track;

	/* Recherche du debut de la zone des pistes du net_code courant */
	if ( net_code >= 0 )
		{
		for ( ;Track != NULL; Track = (TRACK*) Track->Pnext )
			{
			if ( net_code == Track->m_NetCode ) break;
			}
		}

	while ( Track )	/* Mise a jour des flags */
		{		
		if ( (net_code >= 0 ) && (net_code != Track->m_NetCode) ) break;
		GetScreen()->SetModify();
		Track->SetState(SEGM_FIXE, Flag_On);
		Track->Draw(DrawPanel, DC, GR_OR | GR_SURBRILL);
		Track = Track->Next();
		}
	GetScreen()->SetModify();
}


