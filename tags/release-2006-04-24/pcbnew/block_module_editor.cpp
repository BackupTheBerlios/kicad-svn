/****************************************************/
/*	block_module_editor.cpp							*/
/* Handle block commands for the footprint editor	*/
/****************************************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "pcbnew.h"
#include "autorout.h"
#include "pcbplot.h"
#include "trigo.h"

#include "protos.h"


#define BLOCK_COLOR BROWN
#define IS_SELECTED 1

/* Variables Locales */

/* Fonctions exportees */

/* Fonctions Locales */
static void DrawMovingBlockOutlines(WinEDA_DrawPanel * panel, wxDC * DC, bool erase);
static int MarkItemsInBloc(MODULE * module,
	EDA_Rect & Rect);

static void ClearMarkItems(MODULE * module);
static void CopyMarkedItems(MODULE *module, wxPoint offset);
static void MoveMarkedItems(MODULE *module, wxPoint offset);
static void MirrorMarkedItems(MODULE *module, wxPoint offset);
static void RotateMarkedItems(MODULE *module, wxPoint offset);
static void DeleteMarkedItems(MODULE *module);


/*************************************************************************/
int WinEDA_ModuleEditFrame::ReturnBlockCommand(int key)
/*************************************************************************/
/* Return the block command (BLOCK_MOVE, BLOCK_COPY...) corresponding to
	the key (ALT, SHIFT ALT ..) pressed when dragging mouse and left or middle button
	pressed
*/
{
int cmd;

	switch ( key )
		{
		default:
			cmd = key & 0x255;
		break;

		case -1:
			cmd = BLOCK_PRESELECT_MOVE;
			break;

		case 0:
			cmd = BLOCK_MOVE;
			break;

		case GR_KB_ALT:
			cmd = BLOCK_MIRROR_Y;
			break;

		case GR_KB_SHIFTCTRL:
			cmd = BLOCK_DELETE;
			break;

		case GR_KB_SHIFT:
			cmd = BLOCK_COPY;
			break;

		case GR_KB_CTRL:
			cmd = BLOCK_ROTATE;
			break;

		case MOUSE_MIDDLE:
			cmd = BLOCK_ZOOM;
			break;
		}

	return cmd;
}


/****************************************************/
int WinEDA_ModuleEditFrame::HandleBlockEnd(wxDC * DC)
/****************************************************/
/* Command BLOCK END (end of block sizing)
	return :
	0 if command finished (zoom, delete ...) 
	1 if HandleBlockPlace must follow (items found, and a block place command must follow)
*/
{
int ItemsCount = 0, MustDoPlace = 0;
MODULE * Currentmodule = m_Pcb->m_Modules;

	if ( GetScreen()->BlockLocate.m_BlockDrawStruct )
	{
		BlockState state = GetScreen()->BlockLocate.m_State;
		CmdBlockType command = GetScreen()->BlockLocate.m_Command;
		GetScreen()->ForceCloseManageCurseur(this, DC);
		GetScreen()->BlockLocate.m_State =  state;
		GetScreen()->BlockLocate.m_Command = command;
		GetScreen()->ManageCurseur = DrawAndSizingBlockOutlines;
		GetScreen()->ForceCloseManageCurseur = AbortBlockCurrentCommand;
		GetScreen()->m_Curseur.x = GetScreen()->BlockLocate.GetRight();
		GetScreen()->m_Curseur.y = GetScreen()->BlockLocate.GetBottom();
		DrawPanel->MouseToCursorSchema();
	}

	switch( GetScreen()->BlockLocate.m_Command )
		{
		case  BLOCK_IDLE:
			DisplayError(this, wxT("Error in HandleBlockPLace") );
			break;

		case BLOCK_DRAG: /* Drag */
		case BLOCK_MOVE: /* Move */
		case BLOCK_COPY: /* Copy */
			ItemsCount = MarkItemsInBloc(Currentmodule, GetScreen()->BlockLocate);
			if ( ItemsCount )
			{
				MustDoPlace = 1;
				if(GetScreen()->ManageCurseur != NULL)
				{
					GetScreen()->ManageCurseur(DrawPanel, DC, FALSE);
					GetScreen()->ManageCurseur = DrawMovingBlockOutlines;
					GetScreen()->ManageCurseur(DrawPanel, DC, FALSE);
				}
				GetScreen()->BlockLocate.m_State = STATE_BLOCK_MOVE;
			DrawPanel->Refresh(TRUE);
			}
			break;

		case BLOCK_PRESELECT_MOVE: /* Move with preselection list*/
			MustDoPlace = 1;
			GetScreen()->ManageCurseur = DrawMovingBlockOutlines;
			GetScreen()->BlockLocate.m_State = STATE_BLOCK_MOVE;
			break;

		case BLOCK_DELETE: /* Delete */
			ItemsCount = MarkItemsInBloc(Currentmodule, GetScreen()->BlockLocate);
			if ( ItemsCount ) SaveCopyInUndoList();
			DeleteMarkedItems(Currentmodule);
			break;
			
		case BLOCK_SAVE: /* Save */
		case BLOCK_PASTE:
			break;

		case BLOCK_ROTATE:
			ItemsCount = MarkItemsInBloc(Currentmodule, GetScreen()->BlockLocate);
			if ( ItemsCount ) SaveCopyInUndoList();
			RotateMarkedItems(Currentmodule, GetScreen()->BlockLocate.Centre());
			break;


		case BLOCK_MIRROR_X:
		case BLOCK_MIRROR_Y:
		case BLOCK_INVERT: /* mirror */
			ItemsCount = MarkItemsInBloc(Currentmodule, GetScreen()->BlockLocate);
			if ( ItemsCount ) SaveCopyInUndoList();
			MirrorMarkedItems(Currentmodule, GetScreen()->BlockLocate.Centre());
			break;

		case BLOCK_ZOOM: /* Window Zoom */
			Window_Zoom( GetScreen()->BlockLocate );
			break;

		case BLOCK_ABORT:
			break;
		
		case BLOCK_SELECT_ITEMS_ONLY:
			break;
		}

	if ( MustDoPlace <= 0 )
	{
		if ( GetScreen()->BlockLocate.m_Command  != BLOCK_SELECT_ITEMS_ONLY )
		{
			ClearMarkItems(Currentmodule);
		}
		GetScreen()->BlockLocate.m_Flags = 0;
		GetScreen()->BlockLocate.m_State = STATE_NO_BLOCK;
		GetScreen()->BlockLocate.m_Command = BLOCK_IDLE;
		GetScreen()->ManageCurseur = NULL;
		GetScreen()->ForceCloseManageCurseur = NULL;
		GetScreen()->m_CurrentItem = NULL;
		SetToolID(m_ID_current_state, DrawPanel->m_PanelDefaultCursor, wxEmptyString );
		DrawPanel->Refresh(TRUE);
	}


	return(MustDoPlace);
}


/******************************************************/
void WinEDA_ModuleEditFrame::HandleBlockPlace(wxDC * DC)
/******************************************************/
/* Routine to handle the BLOCK PLACE commande
	Last routine for block operation for:
	- block move & drag
	- block copie & paste
*/
{
bool err = FALSE;
MODULE * Currentmodule = m_Pcb->m_Modules;

	if(GetScreen()->ManageCurseur == NULL)
	{
		err = TRUE;
		DisplayError(this, wxT("HandleBlockPLace : ManageCurseur = NULL") );
	}

	GetScreen()->BlockLocate.m_State = STATE_BLOCK_STOP;

	switch(GetScreen()->BlockLocate.m_Command )
	{
		case  BLOCK_IDLE:
			err = TRUE;
			break;
		
		case BLOCK_DRAG: /* Drag */
		case BLOCK_MOVE: /* Move */
		case BLOCK_PRESELECT_MOVE: /* Move with preselection list*/
			GetScreen()->BlockLocate.m_BlockDrawStruct = NULL;
			SaveCopyInUndoList();
			MoveMarkedItems(Currentmodule, GetScreen()->BlockLocate.m_MoveVector);
			DrawPanel->Refresh(TRUE);
			break;

		case BLOCK_COPY: /* Copy */
			GetScreen()->BlockLocate.m_BlockDrawStruct = NULL;
			SaveCopyInUndoList();
			CopyMarkedItems(Currentmodule, GetScreen()->BlockLocate.m_MoveVector);
			break;

		case BLOCK_PASTE: /* Paste (recopie du dernier bloc sauve */
			GetScreen()->BlockLocate.m_BlockDrawStruct = NULL;
			break;

		case BLOCK_MIRROR_X:
		case BLOCK_MIRROR_Y:
		case BLOCK_INVERT:	/* Mirror by popup menu, from block move */
			SaveCopyInUndoList();
			MirrorMarkedItems(Currentmodule, GetScreen()->BlockLocate.Centre());
			break;

		case BLOCK_ROTATE:
			SaveCopyInUndoList();
			RotateMarkedItems(Currentmodule, GetScreen()->BlockLocate.Centre());
			break;

		case BLOCK_ZOOM:	// Handled by HandleBlockEnd
		case BLOCK_DELETE:
		case BLOCK_SAVE:
		case BLOCK_ABORT:
		default:
			break;
	}

	GetScreen()->SetModify();

	GetScreen()->ManageCurseur = NULL;
	GetScreen()->ForceCloseManageCurseur = NULL;
	GetScreen()->BlockLocate.m_Flags = 0;
	GetScreen()->BlockLocate.m_State = STATE_NO_BLOCK;
	GetScreen()->BlockLocate.m_Command =  BLOCK_IDLE;
	GetScreen()->m_CurrentItem = NULL;
	DrawPanel->Refresh(TRUE);

	SetToolID(m_ID_current_state, DrawPanel->m_PanelDefaultCursor, wxEmptyString );
}


/************************************************************************/
static void DrawMovingBlockOutlines(WinEDA_DrawPanel * panel, wxDC * DC,
	bool erase )
/************************************************************************/
/* Retrace le contour du block de recherche de structures
	L'ensemble du block suit le curseur
*/
{
DrawBlockStruct * PtBlock;
BASE_SCREEN * screen = panel->m_Parent->GetScreen();
EDA_BaseStruct * item;
wxPoint move_offset;
MODULE * Currentmodule = EDA_Appl->m_ModuleEditFrame->m_Pcb->m_Modules;
	
	PtBlock = &panel->GetScreen()->BlockLocate;
	GRSetDrawMode(DC, g_XorMode);

	/* Effacement ancien cadre */
	if( erase )
	{
		PtBlock->Offset(PtBlock->m_MoveVector);
		PtBlock->Draw(panel, DC);
		PtBlock->Offset( -PtBlock->m_MoveVector.x, -PtBlock->m_MoveVector.y);

		if ( Currentmodule )
		{	
			move_offset.x =  - PtBlock->m_MoveVector.x;
			move_offset.y =  - PtBlock->m_MoveVector.y;
			item = Currentmodule->m_Drawings;
			for ( ; item != NULL; item = item->Next() )
			{
				if ( item->m_Selected == 0 ) continue;
				switch( item->m_StructType )
				{
					case TYPETEXTEMODULE:
						((TEXTE_MODULE *) item)->Draw(panel, DC, move_offset, g_XorMode);
						break;
		
					case TYPEEDGEMODULE:
						((EDGE_MODULE *) item)->Draw(panel, DC, move_offset, g_XorMode);
						break;
		
					default: break;
				}
			}
			D_PAD* pad = Currentmodule->m_Pads;
			for ( ; pad != NULL; pad = pad->Next() )
			{
				if ( pad->m_Selected == 0 ) continue;
				pad->Draw(panel, DC, move_offset, g_XorMode);
			}
		}
	}

	/* Redessin nouvel affichage */
	PtBlock->m_MoveVector.x = screen->m_Curseur.x - PtBlock->m_BlockLastCursorPosition.x;
	PtBlock->m_MoveVector.y = screen->m_Curseur.y - PtBlock->m_BlockLastCursorPosition.y;

	GRSetDrawMode(DC, g_XorMode);
	PtBlock->Offset(PtBlock->m_MoveVector);
	PtBlock->Draw(panel, DC);
	PtBlock->Offset( -PtBlock->m_MoveVector.x, -PtBlock->m_MoveVector.y);

	
	if ( Currentmodule )
	{	
		item = Currentmodule->m_Drawings;
		move_offset.x =  - PtBlock->m_MoveVector.x;
		move_offset.y =  - PtBlock->m_MoveVector.y;
		for ( ; item != NULL; item = item->Next() )
		{
			if ( item->m_Selected == 0 ) continue;
			switch( item->m_StructType )
			{
				case TYPETEXTEMODULE:
					((TEXTE_MODULE *) item)->Draw(panel, DC, move_offset, g_XorMode);
					break;
	
				case TYPEEDGEMODULE:
					((EDGE_MODULE *) item)->Draw(panel, DC, move_offset, g_XorMode);
					break;
	
				default: break;
			}
		}
		D_PAD* pad = Currentmodule->m_Pads;
		for ( ; pad != NULL; pad = pad->Next() )
		{
			if ( pad->m_Selected == 0 ) continue;
			pad->Draw(panel, DC, move_offset, g_XorMode);
		}
	}
}

/****************************************************************************/
void CopyMarkedItems(MODULE *module, wxPoint offset)
/****************************************************************************/
/* Copy marked items, at new position = old position + offset 
*/
{
EDA_BaseStruct * item;
EDA_BaseStruct* NewStruct;
	
	if ( module == NULL ) return;
		
	D_PAD * pad = module->m_Pads;
	for ( ; pad != NULL; pad = pad->Next() )
	{
		if ( pad->m_Selected == 0 ) continue;
		pad->m_Selected = 0;
		D_PAD * NewPad = new D_PAD( module );
		NewPad->Copy(pad);
		NewPad->m_Selected = IS_SELECTED;
		NewPad->Pnext = module->m_Pads;
		NewPad->Pback = module;
		module->m_Pads->Pback = NewPad;
		module->m_Pads = NewPad;
  	}

	item = module->m_Drawings;
	for ( ; item != NULL; item = item->Next() )
	{
		if ( item->m_Selected == 0 ) continue;
		item->m_Selected = 0;
		NewStruct = NULL;
		switch(item->m_StructType)
		{
			case TYPETEXTEMODULE:
				NewStruct = new TEXTE_MODULE( module );
				((TEXTE_MODULE*)NewStruct)->Copy((TEXTE_MODULE*)item);
				break;

			case TYPEEDGEMODULE:
				NewStruct = new EDGE_MODULE( module );
				((EDGE_MODULE*)NewStruct)->Copy((EDGE_MODULE*)item);
				break;
			default:
				DisplayError(NULL, wxT("Internal Err: CopyMarkedItems: type indefini"));
				break;
		}
		if( NewStruct == NULL) break;
		NewStruct->m_Selected = IS_SELECTED;
		NewStruct->Pnext = module->m_Drawings;
		NewStruct->Pback = module;
		module->m_Drawings->Pback = module;
		module->m_Drawings = NewStruct;
  	}
	
	MoveMarkedItems(module, offset);
}

/****************************************************/
void MoveMarkedItems(MODULE *module, wxPoint offset)
/****************************************************/
/* Move marked items, at new position = old position + offset 
*/
{
EDA_BaseStruct * item;
	
	if ( module == NULL ) return;
		
	D_PAD * pad = module->m_Pads;
	for ( ; pad != NULL; pad = pad->Next() )
	{
		if ( pad->m_Selected == 0 ) continue;
		pad->m_Pos.x += offset.x;
		pad->m_Pos.y += offset.y;
		pad->m_Pos0.x += offset.x;
		pad->m_Pos0.y += offset.y;
	}

	item = module->m_Drawings;
	for ( ; item != NULL; item = item->Next() )
	{
		if ( item->m_Selected == 0 ) continue;
 
		switch ( item->m_StructType )
		{
			case TYPETEXTEMODULE:
				((TEXTE_MODULE*)item)->m_Pos.x += offset.x;
				((TEXTE_MODULE*)item)->m_Pos.y += offset.y;
				((TEXTE_MODULE*)item)->m_Pos0.x += offset.x;
				((TEXTE_MODULE*)item)->m_Pos0.y += offset.y;
				break;

			case TYPEEDGEMODULE :
				((EDGE_MODULE*)item)->m_Start.x += offset.x;
				((EDGE_MODULE*)item)->m_Start.y += offset.y;
				((EDGE_MODULE*)item)->m_End.x += offset.x;
				((EDGE_MODULE*)item)->m_End.y += offset.y;
				((EDGE_MODULE*)item)->m_Start0.x += offset.x;
				((EDGE_MODULE*)item)->m_Start0.y += offset.y;
				((EDGE_MODULE*)item)->m_End0.x += offset.x;
				((EDGE_MODULE*)item)->m_End0.y += offset.y;
				break;
		}
		item->m_Flags = item->m_Selected = 0;
 	}
}

/******************************************************/
void DeleteMarkedItems(MODULE *module)
/******************************************************/
/* Delete marked items 
*/
{
EDA_BaseStruct * item, * next_item;
D_PAD * pad, * next_pad;
	
	if ( module == NULL ) return;
		
	pad = module->m_Pads;
	for ( ; pad != NULL; pad = next_pad )
	{
		next_pad = pad->Next();
		if ( pad->m_Selected == 0 ) continue;
		DeleteStructure(pad);
	}

	item = module->m_Drawings;
	for ( ; item != NULL; item = next_item )
	{
		next_item = item->Next();
		if ( item->m_Selected == 0 ) continue;
		DeleteStructure(item);
	}
}


/******************************************************/
void MirrorMarkedItems(MODULE *module, wxPoint offset)
/******************************************************/
/* Mirror marked items, refer to a Vertical axis at position offset
*/
{
#define SETMIRROR(z) (z) -= offset.x; (z) = -(z); (z) += offset.x;
EDA_BaseStruct * item;
	
	if ( module == NULL ) return;
		
	D_PAD * pad = module->m_Pads;
	for ( ; pad != NULL; pad = pad->Next() )
	{
		if ( pad->m_Selected == 0 ) continue;
		SETMIRROR(pad->m_Pos.x);
		pad->m_Pos0.x = pad->m_Pos.x;
		pad->m_Offset.x = -pad->m_Offset.x;
		pad->m_DeltaSize.x = -pad->m_DeltaSize.x;
		pad->m_Orient = 1800 - pad->m_Orient;
		NORMALIZE_ANGLE(pad->m_Orient);
	}

	item = module->m_Drawings;
	for ( ; item != NULL; item = item->Next() )
	{
		if ( item->m_Selected == 0 ) continue;
 
		switch ( item->m_StructType )
		{
			case TYPEEDGEMODULE:
				SETMIRROR(((EDGE_MODULE*)item)->m_Start.x);
				((EDGE_MODULE*)item)->m_Start0.x = ((EDGE_MODULE*)item)->m_Start.x;
				SETMIRROR(((EDGE_MODULE*)item)->m_End.x);
				((EDGE_MODULE*)item)->m_End0.x = ((EDGE_MODULE*)item)->m_End.x;
				((EDGE_MODULE*)item)->m_Angle = - ((EDGE_MODULE*)item)->m_Angle;
				break;
	
			case TYPETEXTEMODULE:
				SETMIRROR(((TEXTE_MODULE*)item)->m_Pos.x);
				((TEXTE_MODULE*)item)->m_Pos0.x = ((TEXTE_MODULE*)item)->m_Pos.x;
				break;
		}
		item->m_Flags = item->m_Selected = 0;
 	}
}

/******************************************************/
void RotateMarkedItems(MODULE *module, wxPoint offset)
/******************************************************/
/* Rotate marked items, refer to a Vertical axis at position offset
*/
{
#define ROTATE(z) RotatePoint((&z), offset, 900)
EDA_BaseStruct * item;
	
	if ( module == NULL ) return;
		
	D_PAD * pad = module->m_Pads;
	for ( ; pad != NULL; pad = pad->Next() )
	{
		if ( pad->m_Selected == 0 ) continue;
		ROTATE(pad->m_Pos);
		pad->m_Pos0 = pad->m_Pos;
		pad->m_Orient += 900;
		NORMALIZE_ANGLE(pad->m_Orient);
	}

	item = module->m_Drawings;
	for ( ; item != NULL; item = item->Next() )
	{
		if ( item->m_Selected == 0 ) continue;
 
		switch ( item->m_StructType )
		{
			case TYPEEDGEMODULE:
				ROTATE(((EDGE_MODULE*)item)->m_Start);
				((EDGE_MODULE*)item)->m_Start0 = ((EDGE_MODULE*)item)->m_Start;
				ROTATE(((EDGE_MODULE*)item)->m_End);
				((EDGE_MODULE*)item)->m_End0 = ((EDGE_MODULE*)item)->m_End;
				break;
	
			case TYPETEXTEMODULE:
				ROTATE(((TEXTE_MODULE*)item)->m_Pos);
				((TEXTE_MODULE*)item)->m_Pos0 = ((TEXTE_MODULE*)item)->m_Pos;
				((TEXTE_MODULE*)item)->m_Orient += 900;
				break;
		}
		item->m_Flags = item->m_Selected = 0;
 	}
}

/*********************************************************/
void ClearMarkItems(MODULE * module)
/*********************************************************/
{
EDA_BaseStruct * item;
	
	if ( module == NULL ) return;

	item = module->m_Drawings;
	for ( ; item != NULL; item = item->Next() )
		item->m_Flags = item->m_Selected = 0;

	item = module->m_Pads;
	for ( ; item != NULL; item = item->Next() )
		item->m_Flags = item->m_Selected = 0;
}

/***************************************************************/
int MarkItemsInBloc(MODULE * module, EDA_Rect & Rect)
/***************************************************************/
/* Mark items inside rect.
	Items are inside rect when an end point is inside rect
*/
{
EDA_BaseStruct * item;
int ItemsCount = 0;
wxPoint pos;
D_PAD * pad;
	
	if ( module == NULL ) return 0;
		
	pad = module->m_Pads;
	for ( ; pad != NULL; pad = pad->Next() )
	{
		pad->m_Selected = 0;
		pos = pad->m_Pos;
		if ( Rect.Inside(pos) )
		{
			pad->m_Selected = IS_SELECTED;
			ItemsCount++;
		}
	}

	item = module->m_Drawings;
	for ( ; item != NULL; item = item->Next() )
	{
		item->m_Selected = 0;
		switch( item->m_StructType )
		{
			case TYPEEDGEMODULE:
				pos = ((EDGE_MODULE*)item)->m_Start;
				if ( Rect.Inside(pos) )
				{
					item->m_Selected = IS_SELECTED;
					ItemsCount++;
				}
				pos = ((EDGE_MODULE*)item)->m_End;
				if ( Rect.Inside(pos) )
				{
					item->m_Selected = IS_SELECTED;
					ItemsCount++;
				}
				break;
		
			case TYPETEXTEMODULE:
				pos = ((TEXTE_MODULE*)item)->m_Pos;
				if ( Rect.Inside(pos) )
				{
					item->m_Selected = IS_SELECTED;
					ItemsCount++;
				}
				break;
			
			default:
				break;
		}
		
	}
	return ItemsCount;
	
}
