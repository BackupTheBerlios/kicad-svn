	/*********************************/
	/* Module de nettoyage du schema */
	/*********************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"
#include "netlist.h"
#include "macros.h"
#include "protos.h"


/* Routines locales */
static int TstAlignSegment(EDA_DrawLineStruct* RefSegm, EDA_DrawLineStruct* TstSegm);

/* Variables definies et initialisees dans netlist.cc */

/* Variable locales */


/*******************************/
void SchematicCleanUp(wxDC * DC)
/*******************************/
/* Routine de nettoyage:
	- regroupe les segments de fils (ou de bus) alignes en 1 seul segment
	- Detecte les objets identiques superposes
*/
{
SCH_SCREEN * screen;
EDA_BaseStruct *DrawList, * TstDrawList;
int flag;


	screen = ScreenSch;
	for ( ; screen != NULL; screen = (SCH_SCREEN*)screen->Pnext )
	{
		DrawList = screen->EEDrawList;
		while ( DrawList )
		{
			if( DrawList->m_StructType == DRAW_SEGMENT_STRUCT_TYPE )
			{
				TstDrawList = DrawList->Pnext;
				while ( TstDrawList )
				{
					if( TstDrawList->m_StructType == DRAW_SEGMENT_STRUCT_TYPE )
					{
						flag = TstAlignSegment( (EDA_DrawLineStruct*)DrawList,
											(EDA_DrawLineStruct*)TstDrawList);
						if (flag )	/* Suppression de TstSegm */
						{
							EraseStruct(TstDrawList, screen);
							screen->SetRefreshReq();
							TstDrawList = screen->EEDrawList;
						}
						else TstDrawList = TstDrawList->Pnext;
					}
					else TstDrawList = TstDrawList->Pnext;
				}
			}
			DrawList = DrawList->Pnext;
		}
		EDA_Appl->SchematicFrame->TestDanglingEnds(screen->EEDrawList, DC);
	}
}


/***********************************************/
void BreakSegmentOnJunction( BASE_SCREEN * Screen )
/************************************************/
/* Routine creant des debuts / fin de segment (BUS ou WIRES) sur les jonctions
et les raccords
	Si Screen = NULL: traitement de la hierarchie complete
*/
{
BASE_SCREEN * screen = Screen;
EDA_BaseStruct *DrawList;

	if( screen == NULL ) screen = ScreenSch;

	for ( ; screen != NULL; screen = (BASE_SCREEN*)screen->Pnext )
		{
		DrawList = screen->EEDrawList;
		while ( DrawList )
			{
			switch( DrawList->m_StructType )
				{
				case DRAW_JUNCTION_STRUCT_TYPE :
					#undef STRUCT
					#define STRUCT ((DrawJunctionStruct*)DrawList)
					BreakSegment(screen, STRUCT->m_Pos);
					break;

				case DRAW_BUSENTRY_STRUCT_TYPE :
					#undef STRUCT
					#define STRUCT ((DrawBusEntryStruct*)DrawList)
					BreakSegment(screen, STRUCT->m_Pos);
					BreakSegment(screen, STRUCT->m_End());
					break;

				case DRAW_SEGMENT_STRUCT_TYPE :
				case DRAW_NOCONNECT_STRUCT_TYPE :
				case DRAW_LABEL_STRUCT_TYPE :
				case DRAW_GLOBAL_LABEL_STRUCT_TYPE :
				case DRAW_LIB_ITEM_STRUCT_TYPE :
				case DRAW_PICK_ITEM_STRUCT_TYPE :
				case DRAW_POLYLINE_STRUCT_TYPE :
				case DRAW_MARKER_STRUCT_TYPE :
				case DRAW_TEXT_STRUCT_TYPE :
				case DRAW_SHEET_STRUCT_TYPE :
				case DRAW_SHEETLABEL_STRUCT_TYPE :
					break;

				default :
					break;
				}
			DrawList = DrawList->Pnext;
			}
		}
}


/*********************************************************/
void BreakSegment(BASE_SCREEN * screen, wxPoint breakpoint)
/*********************************************************/
/* Coupe un segment ( BUS, WIRE ) en 2 au point breakpoint,
	- si ce point est sur le segment
	- extremites non comprises
*/
{
EDA_BaseStruct *DrawList;
EDA_DrawLineStruct * segment, * NewSegment;
int ox, oy, fx, fy;

	DrawList = screen->EEDrawList;
	while ( DrawList )
		{
		switch( DrawList->m_StructType )
			{
			case DRAW_SEGMENT_STRUCT_TYPE :
				segment = (EDA_DrawLineStruct*)DrawList;
				ox = segment->m_Start.x; oy = segment->m_Start.y;
				fx = segment->m_End.x; fy = segment->m_End.y;
				if( distance( fx - ox, fy - oy, breakpoint.x - ox, breakpoint.y - oy, 0 ) == 0 )
					break;
				/* Segment connecte: doit etre coupe en 2 si px,py n'est
					pas une extremite */
				if( (ox == breakpoint.x) && (oy == breakpoint.y ) ) break;
				if( (fx == breakpoint.x) && (fy == breakpoint.y ) ) break;
				/* Ici il faut couper le segment en 2 */
				NewSegment = segment->GenCopy();
				NewSegment->m_Start = breakpoint;
				segment->m_End = NewSegment->m_Start;
				NewSegment->Pnext = segment->Pnext;
				segment->Pnext = NewSegment;
				DrawList = NewSegment;
				break;

			case DRAW_JUNCTION_STRUCT_TYPE :
			case DRAW_BUSENTRY_STRUCT_TYPE :
			case DRAW_NOCONNECT_STRUCT_TYPE :
			case DRAW_LABEL_STRUCT_TYPE :
			case DRAW_GLOBAL_LABEL_STRUCT_TYPE :
			case DRAW_LIB_ITEM_STRUCT_TYPE :
			case DRAW_PICK_ITEM_STRUCT_TYPE :
			case DRAW_POLYLINE_STRUCT_TYPE :
			case DRAW_MARKER_STRUCT_TYPE :
			case DRAW_TEXT_STRUCT_TYPE :
			case DRAW_SHEET_STRUCT_TYPE :
			case DRAW_SHEETLABEL_STRUCT_TYPE :
				break;

			default :
				break;
			}
		DrawList = DrawList->Pnext;
		}
}



/***********************************************************/
static int TstAlignSegment( EDA_DrawLineStruct* RefSegm, 
							EDA_DrawLineStruct* TstSegm)
/***********************************************************/

/* Detecte si 2 segments sont alignes et se touchent.
	Retourne 0 si non
		1 si oui, et RefSegm est alors modifie pour remplacer les 2 segments
		par 1 seul
*/
{
int px, py, ox, oy, fx, fy, flag, nn ;
int cx, cy;

	if( RefSegm == TstSegm ) return(0);
	if( RefSegm->m_Layer != TstSegm->m_Layer ) return(0);

	/* Recalcul des coord avec debut de RefSegm comme origine */
	cx = RefSegm->m_Start.x; cy = RefSegm->m_Start.y;
	px = RefSegm->m_End.x - cx; py = RefSegm->m_End.y - cy;
	ox = TstSegm->m_Start.x - cx; oy = TstSegm->m_Start.y - cy;
	fx = TstSegm->m_End.x - cx; fy = TstSegm->m_End.y - cy;

	/* Tst alignement vertical possible ( X = cte = 0 ): */
	flag = 0;
	if(px == 0)
		{
		if ( ox != fx) return(0);	/* non alignes */
		flag = 1;
		}
	/* Tst alignement horizontal possible ( Y = cte = 0): */
	else if( py == 0)
		{
		if ( oy != fy) return(0);	/* non alignes */
		flag = 2;
		}

	/* tst si il y a alignement d'angle qcq:
		il faut que py/px == (+/-)segmdy/segmdx, c.a.d meme direction */
	else 
		{
		if( (py * (fx - ox)  != px * (fy - oy) ) &&
			(py * (fx - ox) != -px * (fy - oy) ) ) return(0);	/* non alignes */
		flag = 3;
		}

	/* Ici les segments sont alignes, mais il faut qu'ils se touchent */

	switch ( flag )
		{
		case 1:		/* Segments verticaux */
			if( ox != 0 ) return (0);
			if( (py >= oy) && (py <= fy) ) flag = -1;	/* alignes */
			else if( (py >= fy) && (py <= oy) ) flag = -1;  /* alignes */
			else if( (oy >= 0) && (oy <= py) ) flag = -1;	/* alignes */
			else if( (oy <= 0) && (oy >= py) ) flag = -1;	/* alignes */
			else if( (fy >= 0) && (fy <= py) ) flag = -1;	/* alignes */
			else if( (fy <= 0) && (fy >= py) ) flag = -1;	/* alignes */
			break;

		case 2:		/* Segments horizontaux */
			if( oy != 0 ) return (0);
			if( (px >= ox) && (px <= fx) ) flag = -2;	/* alignes */
			else if( (px >= fx) && (px <= ox) ) flag = -2;  /* alignes */
			else if( (ox >= 0) && (ox <= px) ) flag = -2;	/* alignes */
			else if( (ox <= 0) && (ox >= px) ) flag = -2;	/* alignes */
			else if( (fx >= 0) && (fx <= px) ) flag = -2;	/* alignes */
			else if( (fx <= 0) && (fx >= px) ) flag = -2;	/* alignes */
			break;

		case 3:		/* segments quelconques */
			{
			int cx1, cy1, px1, py1, ox1, oy1, fx1, fy1;
			if( distance(px, py, ox, oy, 0 ) )
				{
				flag = -3; break;	/* debut de segment Tst sur Ref */
				}
			if( distance(px, py, fx, fy, 0 ) )
				{
				flag = -3; break;	/* fin de segment Tst sur Ref */
				}

			cx1 = TstSegm->m_Start.x; cy1 = TstSegm->m_Start.y;
			px1 = TstSegm->m_End.x - cx1; py1 = TstSegm->m_End.y - cy1;
			ox1 = RefSegm->m_Start.x - cx1; oy1 = RefSegm->m_Start.y - cy1;
			fx1 = RefSegm->m_End.x - cx1; fy1 = RefSegm->m_End.y - cy1;
			if( distance(px1, py1 , ox1, oy1, 0 ) )
				{
				flag = -3; break;	/* debut de segment Ref sur Tst */
				}
			if( distance(px1, py1 , fx1, fy1, 0 ) )
				{
				flag = -3; break;	/* fin de segment Ref sur Tst */
				}
			}
			break;
		}

	if( flag >= 0 ) return(0);

	/* Segments alignes: recalcul des coordonnees */

	switch ( flag )
		{
		case -1:	 /* Segments verticaux */
			if( py >= 0 )
				{
				py = MAX(py,oy); py = MAX ( py, fy);
				nn = MIN(oy, fy); if( nn > 0 ) nn = 0;
				}
			else {	/* py < 0 */
				py = MIN(py,oy); py = MIN ( py, fy);
				nn = MAX(oy, fy); if( nn < 0 ) nn = 0;
				}
			RefSegm->m_Start.y = nn + cy;
			RefSegm->m_End.y = py + cy;
			return(1);

		case -2:	 /* Segments horizontaux */
			if( px >= 0 )
				{
				px = MAX(px,ox); px = MAX ( px, fx);
				nn = MIN(ox, fx); if( nn > 0 ) nn = 0;
				}
			else {  /* px < 0 */
				px = MIN(px,ox); px = MIN ( px, fx);
				nn = MAX(ox, fx); if( nn < 0 ) nn = 0;
				}
			RefSegm->m_Start.x = nn + cx;
			RefSegm->m_End.x = px + cx;
			return(1);

		case -3:	 /* Segments quelconques */
			if( px >= 0 )
				{
				if( px < ox)
					{
					px = ox;
					RefSegm->m_End.x = px + cx;
					RefSegm->m_End.y = oy + cy;
					}
				if( px < fy )
					{
					px = fx;
					RefSegm->m_End.x = px + cx;
					RefSegm->m_End.y = fy + cy;
					}
				if ( ox < 0 )
					{
					RefSegm->m_Start.x = ox + cx;
					RefSegm->m_Start.y = oy + cy;
					}
				if ( fx < ox )
					{
					RefSegm->m_Start.x = fx + cx;
					RefSegm->m_Start.y = fy + cy;
					}
				}
			else {  /* px < 0 */
				if( px > ox)
					{
					px = ox;
					RefSegm->m_End.x = px + cx;
					RefSegm->m_End.y = oy + cy;
					}
				if( px > fy )
					{
					px = fx;
					RefSegm->m_End.x = px + cx;
					RefSegm->m_End.y = fy + cy;
					}
				if ( ox > 0 )
					{
					RefSegm->m_Start.x = ox + cx;
					RefSegm->m_Start.y = oy + cy;
					}
				if ( fx > ox )
					{
					RefSegm->m_Start.x = fx + cx;
					RefSegm->m_Start.y = fy + cy;
					}
				}
			return(1);
		}
	return(0);
}

