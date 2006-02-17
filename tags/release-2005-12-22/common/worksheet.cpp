		/**************************************************/
		/* WORKSHEET.CPP : routines de trace du cartouche */
		/**************************************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"

#ifdef EESCHEMA
#include "program.h"
#include "libcmp.h"
#include "general.h"
#endif

#ifdef PCBNEW
#include "pcbnew.h"
#endif

#include "worksheet.h"

#include "protos.h"

/*********************************************************************************/
void WinEDA_DrawFrame::TraceWorkSheet(wxDC * DC, BASE_SCREEN * screen, int marge)
/*********************************************************************************/
/* Trace l'encadrement de la feuille de travail et le cartouche
*/
{
	if ( ! m_Draw_Sheet_Ref ) return;

int ii, jj, xg , yg, ipas, gxpas, gypas;
wxPoint pos;
int refx, refy,Color;
wxString Line;
WorkSheet * WsItem;
int scale = m_InternalUnits/1000;
wxSize size(SIZETEXT*scale,SIZETEXT*scale);
wxSize size_ref(SIZETEXT_REF*scale,SIZETEXT_REF*scale);
wxString msg;

	Color = RED;
	GRSetDrawMode(DC, GR_COPY);
	if(screen->m_CurrentSheet == NULL)
		{
		DisplayError(this,
			wxT("WinEDA_DrawFrame::TraceWorkSheet error: m_CurrentSheet NULL"));
		return;
		}

	/* trace de la bordure */

	refx = refy = marge; /* Start Point */
	xg = screen->m_CurrentSheet->m_Size.x - marge;
	yg = screen->m_CurrentSheet->m_Size.y - marge;

	for ( ii = 0; ii < 2 ; ii++ )
		{
		GRRect(&DrawPanel->m_ClipBox, DC, refx * scale, refy * scale,
			xg * scale, yg * scale, Color);

		refx += GRID_REF_W; refy += GRID_REF_W;
		xg -= GRID_REF_W; yg -= GRID_REF_W;
		}
	/* trace des reperes */
	refx = refy = marge; /* Start Point */
	xg = screen->m_CurrentSheet->m_Size.x - marge;
	yg = screen->m_CurrentSheet->m_Size.y - marge;

	/* Trace des reperes selon l'axe X */
	ipas = (xg - refx) / PAS_REF;
	gxpas = ( xg - refx) / ipas;
	for ( ii = refx + gxpas, jj = 1; ipas > 0 ; ii += gxpas , jj++, ipas--)
		{
		Line.Printf( wxT("%d"),jj);
		if( ii < xg - PAS_REF/2 )
			{
			GRLine(&DrawPanel->m_ClipBox, DC, ii * scale, refy * scale,
					ii * scale, (refy + GRID_REF_W) * scale, Color);
			}
		DrawGraphicText(DrawPanel, DC,
						wxPoint( (ii - gxpas/2) * scale, (refy + GRID_REF_W/2) * scale),
						Color,
						Line, TEXT_ORIENT_HORIZ, size_ref,
						GR_TEXT_HJUSTIFY_CENTER, GR_TEXT_VJUSTIFY_CENTER);
		if( ii < xg - PAS_REF/2 )
			{
			GRLine(&DrawPanel->m_ClipBox, DC,ii * scale, yg * scale,
					ii * scale, (yg - GRID_REF_W) * scale, Color);
			}
		DrawGraphicText(DrawPanel, DC,
					wxPoint( (ii - gxpas/2) * scale, (yg - GRID_REF_W/2) * scale),
					Color,
					Line, TEXT_ORIENT_HORIZ, size_ref,
					GR_TEXT_HJUSTIFY_CENTER, GR_TEXT_VJUSTIFY_CENTER);
		}

	/* Trace des reperes selon l'axe Y */
	ipas = (yg - refy) / PAS_REF;
	gypas = ( yg - refy) / ipas;
	for ( ii = refy + gypas, jj = 0; ipas > 0 ; ii += gypas , jj++, ipas--)
		{
		Line.Empty();
		if( jj < 26 ) Line.Printf(wxT("%c"), jj + 'A');
		else Line.Printf(wxT("%c"), 'a' + jj - 26);
		if( ii < yg - PAS_REF/2 )
			{
			GRLine(&DrawPanel->m_ClipBox, DC, refx * scale, ii * scale,
					(refx + GRID_REF_W) * scale, ii * scale, Color);
			}
		DrawGraphicText(DrawPanel, DC,
					wxPoint((refx + GRID_REF_W/2) * scale, (ii - gypas/2) * scale),
					Color,
					Line, TEXT_ORIENT_HORIZ, size_ref,
					GR_TEXT_HJUSTIFY_CENTER, GR_TEXT_VJUSTIFY_CENTER);
		if( ii < yg - PAS_REF/2 )
			{
			GRLine(&DrawPanel->m_ClipBox, DC, xg * scale, ii * scale,
						(xg - GRID_REF_W) * scale, ii * scale, Color);
			}
		DrawGraphicText(DrawPanel, DC,
					wxPoint((xg - GRID_REF_W/2) * scale, (ii - gxpas/2) * scale),
					Color,
					Line, TEXT_ORIENT_HORIZ, size_ref,
					GR_TEXT_HJUSTIFY_CENTER, GR_TEXT_VJUSTIFY_CENTER);
		}



	/* Trace du cartouche */
	refx = screen->m_CurrentSheet->m_Size.x - GRID_REF_W - marge;
	refy = screen->m_CurrentSheet->m_Size.y - GRID_REF_W - marge;

	for( WsItem = &WS_Date; WsItem != NULL; WsItem = WsItem->Pnext )
		{
		pos.x = (refx - WsItem->posx)* scale;
		pos.y = (refy - WsItem->posy)* scale;
		msg.Empty();
		switch( WsItem->type )
			{
			case WS_DATE:
				if(WsItem->Legende) msg = WsItem->Legende;
				msg += screen->m_Date;
				DrawGraphicText(DrawPanel, DC, pos, Color,
					msg, TEXT_ORIENT_HORIZ, size,
					GR_TEXT_HJUSTIFY_LEFT, GR_TEXT_VJUSTIFY_CENTER);
				break;

			case WS_REV:
				if(WsItem->Legende) msg = WsItem->Legende;
				msg += screen->m_Revision;
				DrawGraphicText(DrawPanel, DC, pos, Color,
					msg, TEXT_ORIENT_HORIZ, size,
					GR_TEXT_HJUSTIFY_LEFT, GR_TEXT_VJUSTIFY_CENTER);
				break;

			case WS_LICENCE:
				if(WsItem->Legende) msg = WsItem->Legende;
				msg += g_ProductName + Main_Title;
				DrawGraphicText(DrawPanel, DC, pos, Color,
					msg, TEXT_ORIENT_HORIZ, size,
					GR_TEXT_HJUSTIFY_LEFT, GR_TEXT_VJUSTIFY_CENTER);
				break;

			case WS_SIZESHEET:
				if(WsItem->Legende) msg = WsItem->Legende;
				msg += screen->m_CurrentSheet->m_Name;
				DrawGraphicText(DrawPanel, DC, pos, Color,
					msg, TEXT_ORIENT_HORIZ, size,
					GR_TEXT_HJUSTIFY_LEFT, GR_TEXT_VJUSTIFY_CENTER);
				break;


			case WS_IDENTSHEET:
				if(WsItem->Legende) msg = WsItem->Legende;
				   msg << screen->m_SheetNumber << wxT("/") <<
									screen->m_NumberOfSheet;
				DrawGraphicText(DrawPanel, DC, pos, Color,
					msg, TEXT_ORIENT_HORIZ, size,
					GR_TEXT_HJUSTIFY_LEFT, GR_TEXT_VJUSTIFY_CENTER);
				break;

			case WS_NAMECOMP:
				if(WsItem->Legende) msg = WsItem->Legende;
				msg += screen->m_Company;
				DrawGraphicText(DrawPanel, DC, pos, Color,
					msg, TEXT_ORIENT_HORIZ, size,
					GR_TEXT_HJUSTIFY_LEFT, GR_TEXT_VJUSTIFY_CENTER);
				break;

			case WS_TITLE:
				if(WsItem->Legende) msg = WsItem->Legende;
				msg += screen->m_Title;
				DrawGraphicText(DrawPanel, DC, pos, Color,
					msg, TEXT_ORIENT_HORIZ, size,
					GR_TEXT_HJUSTIFY_LEFT, GR_TEXT_VJUSTIFY_CENTER);
				break;

			case WS_COMMENT1:
				if(WsItem->Legende) msg = WsItem->Legende;
				msg += screen->m_Commentaire1;
				DrawGraphicText(DrawPanel, DC, pos, Color,
					msg, TEXT_ORIENT_HORIZ, size,
					GR_TEXT_HJUSTIFY_LEFT, GR_TEXT_VJUSTIFY_CENTER);
				break;

			case WS_COMMENT2:
				if(WsItem->Legende) msg = WsItem->Legende;
				msg += screen->m_Commentaire2;
				DrawGraphicText(DrawPanel, DC, pos, Color,
					msg, TEXT_ORIENT_HORIZ, size,
					GR_TEXT_HJUSTIFY_LEFT, GR_TEXT_VJUSTIFY_CENTER);
				break;

			case WS_COMMENT3:
				if(WsItem->Legende) msg = WsItem->Legende;
				msg += screen->m_Commentaire3;
				DrawGraphicText(DrawPanel, DC, pos, Color,
					msg, TEXT_ORIENT_HORIZ, size,
					GR_TEXT_HJUSTIFY_LEFT, GR_TEXT_VJUSTIFY_CENTER);
				break;

			case WS_COMMENT4:
				if(WsItem->Legende) msg = WsItem->Legende;
				msg += screen->m_Commentaire4;
				DrawGraphicText(DrawPanel, DC, pos, Color,
					msg, TEXT_ORIENT_HORIZ, size,
					GR_TEXT_HJUSTIFY_LEFT, GR_TEXT_VJUSTIFY_CENTER);
				break;

			case WS_SEGMENT:
				xg = screen->m_CurrentSheet->m_Size.x -
						GRID_REF_W - marge - WsItem->endx;
				yg = screen->m_CurrentSheet->m_Size.y -
						GRID_REF_W - marge - WsItem->endy;
				GRLine(&DrawPanel->m_ClipBox, DC, pos.x, pos.y,
						xg * scale, yg * scale, Color);
				break;

			}
		}
}
