	/****************************************************/
	/* Routines de gestion des commandes sur blocks		*/
	/* (section commune eeschema/pcbnew... 				*/
	/****************************************************/

		/* Fichier common.cpp */

#include "fctsys.h"
#include "gr_basic.h"
#include "wxstruct.h"
#include "common.h"
#include "macros.h"


	/*******************/
	/* DrawBlockStruct */
	/*******************/

/****************************************************************************/
DrawBlockStruct::DrawBlockStruct(void):
		EDA_BaseStruct(BLOCK_LOCATE_STRUCT_TYPE), EDA_Rect()
/****************************************************************************/
{
	m_State = STATE_NO_BLOCK;	/* Etat (enum BlockState) du block */
	m_Command = BLOCK_IDLE;		/* Type (enum CmdBlockType) d'operation */
	m_BlockDrawStruct = NULL;	/* pointeur sur la structure */
	m_Color = BROWN;
}


/****************************************/
DrawBlockStruct::~DrawBlockStruct(void)
/****************************************/
{
}



/***************************************************************/
void DrawBlockStruct::SetMessageBlock(WinEDA_DrawFrame * frame)
/***************************************************************/
/*
	Print block command message (Block move, Block copy ...) in status bar
*/
{
wxString msg;

	switch(m_Command)
	{
		case BLOCK_IDLE:
			break;

		case BLOCK_MOVE: /* Move */
		case BLOCK_PRESELECT_MOVE: /* Move with preselection list*/
			msg = _("Block Move");
			break;

		case BLOCK_DRAG: /* Drag */
			msg = _("Block Drag");
			break;

		case BLOCK_COPY: /* Copy */
			msg = _("Block Copy");
			break;

		case BLOCK_DELETE: /* Delete */
			msg = _("Block Delete");
			break;

		case BLOCK_SAVE: /* Save */
			msg = _("Block Save");
			break;

		case BLOCK_PASTE:
			msg = _("Block Paste");
			break;

		case BLOCK_ZOOM: /* Window Zoom */
			msg = _("Win Zoom");
			break;

		case BLOCK_ROTATE: /* Rotate 90 deg */
			msg = _("Block Rotate");
			break;

		case BLOCK_INVERT: /* Flip */
			msg = _("Block Invert");
			break;

		case BLOCK_ABORT:
			break;

		default:
			msg = wxT("????");
			break;
	}

	frame->DisplayToolMsg(msg);
}


/**************************************************************/
void DrawBlockStruct::Draw(WinEDA_DrawPanel * panel, wxDC * DC)
/**************************************************************/
{
	int w = GetWidth()/panel->GetZoom();
	int h = GetHeight()/panel->GetZoom();
	if (  w == 0 || h == 0 )
		GRLine(&panel->m_ClipBox, DC, GetX(), GetY(),
				GetRight(), GetBottom(), m_Color);
	else
		GRRect(&panel->m_ClipBox, DC,  GetX(), GetY(),
				GetRight(), GetBottom(), m_Color);
}

/*************************************************************************/
bool WinEDA_DrawFrame::HandleBlockBegin(wxDC * DC, int key,
		const wxPoint & startpos)
/*************************************************************************/
/* 	First command block function:
	Init the Block infos: command type, initial position, and other variables..
*/
{
DrawBlockStruct * Block = & GetScreen()->BlockLocate;
	
	if ( (Block->m_Command != BLOCK_IDLE) ||
		 ( Block->m_State != STATE_NO_BLOCK) )
		return FALSE;

	Block->m_Flags = 0;
	Block->m_Command = (CmdBlockType) ReturnBlockCommand(key);
	if ( Block->m_Command == 0 )
		return FALSE;
		
	switch(Block->m_Command)
		{
		case BLOCK_IDLE: break;

		case BLOCK_MOVE: /* Move */
		case BLOCK_DRAG: /* Drag */
		case BLOCK_COPY: /* Copy */
		case BLOCK_DELETE: /* Delete */
		case BLOCK_SAVE: /* Save */
		case BLOCK_ROTATE: /* Rotate 90 deg */
		case BLOCK_INVERT: /* Flip */
		case BLOCK_ZOOM: /* Window Zoom */
		case BLOCK_PRESELECT_MOVE: /* Move with preselection list*/
			InitBlockLocateDatas(GetScreen(),startpos);
			break;

		case BLOCK_PASTE:
			InitBlockLocateDatas(GetScreen(),startpos);
			Block->m_BlockLastCursorPosition.x = 0;
			Block->m_BlockLastCursorPosition.y = 0;
			InitBlockPasteInfos();
			if( Block->m_BlockDrawStruct == NULL )	/* No data to paste */
			{
				DisplayError(this, wxT("No Block to paste"), 20);
				GetScreen()->BlockLocate.m_Command =  BLOCK_IDLE;
				GetScreen()->ManageCurseur = NULL;
				return TRUE;
			}
			if ( GetScreen()->ManageCurseur == NULL )
			{
				Block->m_BlockDrawStruct = NULL;
				DisplayError(this,
					wxT("WinEDA_DrawFrame::HandleBlockBegin() Err: ManageCurseur NULL"));
				return TRUE;
			}
			Block->m_State = STATE_BLOCK_MOVE;
			GetScreen()->ManageCurseur(DrawPanel, DC, FALSE);
			break;

		default:
			DisplayError(this,
				wxT("WinEDA_DrawFrame::HandleBlockBegin() error: Unknown command"));
			break;
		}

	Block->SetMessageBlock(this);
	return TRUE;
}


/******************************************************************/
void AbortBlockCurrentCommand(WinEDA_DrawFrame * frame, wxDC * DC)
/******************************************************************/
/*
	Cancel Current block operation.
*/
{
BASE_SCREEN * screen = frame->GetScreen();

	if( screen->ManageCurseur)	/* Erase current drawing on screen */
	{
		screen->ManageCurseur(frame->DrawPanel, DC, FALSE); /* Efface dessin fantome */
		screen->ManageCurseur = NULL;
		screen->ForceCloseManageCurseur = NULL;
		screen->m_CurrentItem = NULL;

		/* Delete the picked wrapper if this is a picked list. */
		if( (screen->BlockLocate.m_Command != BLOCK_PASTE) &&
			screen->BlockLocate.m_BlockDrawStruct )
		{
			if(screen->BlockLocate.m_BlockDrawStruct->m_StructType == DRAW_PICK_ITEM_STRUCT_TYPE)
			{
				DrawPickedStruct * PickedList;
				PickedList = (DrawPickedStruct*)screen->BlockLocate.m_BlockDrawStruct;
				PickedList->DeleteWrapperList();
			}
			screen->BlockLocate.m_BlockDrawStruct = NULL;
		}
	}

	screen->BlockLocate.m_Flags = 0;
	screen->BlockLocate.m_State = STATE_NO_BLOCK;

	screen->BlockLocate.m_Command = BLOCK_ABORT;
	frame->HandleBlockEnd(DC);

	screen->BlockLocate.m_Command = BLOCK_IDLE;
	frame->DisplayToolMsg(wxEmptyString);
}

/*************************************************************************/
void InitBlockLocateDatas( BASE_SCREEN * screen,const wxPoint & startpos )
/*************************************************************************/
/*
	Init the initial values of a BlockLocate, before starting a block command
*/
{
	screen->BlockLocate.m_State = STATE_BLOCK_INIT;
	screen->BlockLocate.SetOrigin(startpos);
	screen->BlockLocate.SetSize(wxSize(0,0));
	screen->BlockLocate.Pnext = NULL;
	screen->BlockLocate.m_BlockDrawStruct = NULL;
	screen->ManageCurseur = DrawAndSizingBlockOutlines;
	screen->ForceCloseManageCurseur = AbortBlockCurrentCommand;
}

/********************************************************************************/
void DrawAndSizingBlockOutlines(WinEDA_DrawPanel * panel, wxDC * DC, bool erase )
/********************************************************************************/
/* Redraw the outlines of the block which shows the search area for block commands
	The first point of the rectangle showing the area is initialised
	by InitBlockLocateDatas().
	The other point of the rectangle is the mouse cursor
 */
{
DrawBlockStruct * PtBlock;

	PtBlock = &panel->GetScreen()->BlockLocate;

	PtBlock->m_MoveVector = wxPoint(0,0);

	GRSetDrawMode(DC, g_XorMode);

	/* Effacement ancien cadre */
	if( erase ) PtBlock->Draw(panel, DC);

	PtBlock->m_BlockLastCursorPosition = panel->GetScreen()->m_Curseur;
	PtBlock->SetEnd(panel->GetScreen()->m_Curseur);

	PtBlock->Draw(panel, DC);

	if ( PtBlock->m_State == STATE_BLOCK_INIT )
	{
		if ( PtBlock->GetWidth() || PtBlock->GetHeight() )
			/* 2ieme point existant: le rectangle n'est pas de surface nulle */
			PtBlock->m_State = STATE_BLOCK_END;
	}
}


