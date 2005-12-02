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
	/* void WinEDA_GerberFrame::Delete_Segment(wxDC * DC, TRACK *Track) */
	/*****************************************************************/

/* Supprime 1 segment de piste.
	2 Cas possibles:
	Si On est en trace de nouvelle piste: Effacement du segment en
		cours de trace
	Sinon : Effacment du segment sous le curseur.
*/

TRACK * WinEDA_GerberFrame::Delete_Segment(wxDC * DC, TRACK *Track)
{
	if ( Track == NULL ) return NULL;

	if(Track->m_Flags & IS_NEW)  // Trace en cours, on peut effacer le dernier segment
		{
		if(nbptnewpiste > 0 )
			{
			// effacement de la piste en cours
//			Montre_Position_New_Piste(DrawPanel, DC, NEW_SCREEN);

			// modification du trace
			Track = ptnewpiste; ptnewpiste = (TRACK*) ptnewpiste->Pback;
			delete Track; nbptnewpiste--;

			if( nbptnewpiste && (ptnewpiste->m_StructType == TYPEVIA))
				{
				Track = ptnewpiste; ptnewpiste = (TRACK*) ptnewpiste->Pback;
				delete Track;
				nbptnewpiste-- ;
				}
			if( ptnewpiste ) ptnewpiste->Pnext = NULL;
			
			// Rectification couche active qui a pu changer si une via
			// a ete effacee
			if( nbptnewpiste )
				GetScreen()->m_Active_Layer = ptnewpiste->m_Layer ;
			else GetScreen()->m_Active_Layer = ptstartpiste->m_Layer ;

			Affiche_Status_Box();
			
			if(nbptnewpiste == 0 )
				{
				GetScreen()->ManageCurseur = NULL;
				GetScreen()->ForceCloseManageCurseur = NULL;
				return NULL;
				}
			else
				{
				if(GetScreen()->ManageCurseur)
					GetScreen()->ManageCurseur(DrawPanel, DC, FALSE);
				return ptnewpiste;
				}
			}
		return NULL;
		} // Fin traitement si trace en cours


	Trace_Segment(DrawPanel, DC, Track,GR_XOR) ;

	SaveItemEfface( Track, 1);
	GetScreen()->SetModify();
	return NULL;
}




