		/*********************************************/
		/* Edition des pistes: Routines d'effacement */
		/* Effacement de segment, piste, net et zone */
		/*********************************************/

#include "fctsys.h"

#include "common.h"
#include "gerbview.h"

#include "protos.h"

/* Routines externes : */

/* Routines Locales */

/* Variables locales */


/*****************************************************************/
TRACK * WinEDA_GerberFrame::Delete_Segment(wxDC * DC, TRACK *Track)
/*****************************************************************/
/* Supprime 1 segment de piste.
	2 Cas possibles:
	Si On est en trace de nouvelle piste: Effacement du segment en
		cours de trace
	Sinon : Effacment du segment sous le curseur.
*/
{
	if ( Track == NULL ) return NULL;

	if(Track->m_Flags & IS_NEW)  // Trace en cours, on peut effacer le dernier segment
		{
		if(g_TrackSegmentCount > 0 )
			{
			// modification du trace
			Track = g_CurrentTrackSegment; g_CurrentTrackSegment = (TRACK*) g_CurrentTrackSegment->Pback;
			delete Track; g_TrackSegmentCount--;

			if( g_TrackSegmentCount && (g_CurrentTrackSegment->m_StructType == TYPEVIA))
				{
				Track = g_CurrentTrackSegment; g_CurrentTrackSegment = (TRACK*) g_CurrentTrackSegment->Pback;
				delete Track;
				g_TrackSegmentCount-- ;
				}
			if( g_CurrentTrackSegment ) g_CurrentTrackSegment->Pnext = NULL;

			Affiche_Status_Box();
			
			if(g_TrackSegmentCount == 0 )
				{
				GetScreen()->ManageCurseur = NULL;
				GetScreen()->ForceCloseManageCurseur = NULL;
				return NULL;
				}
			else
				{
				if(GetScreen()->ManageCurseur)
					GetScreen()->ManageCurseur(DrawPanel, DC, FALSE);
				return g_CurrentTrackSegment;
				}
			}
		return NULL;
		} // Fin traitement si trace en cours


	Trace_Segment(DrawPanel, DC, Track,GR_XOR) ;

	SaveItemEfface( Track, 1);
	GetScreen()->SetModify();
	return NULL;
}




