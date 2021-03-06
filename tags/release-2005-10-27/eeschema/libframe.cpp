	/****************************/
	/*	EESchema - libframe.cpp	*/
	/****************************/

/* Gestion de la frame d'edition des composants en librairie
*/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"

#include "bitmaps.h"
#include "protos.h"

#include "id.h"

#include "libedit.xpm"

	/*****************************/
	/* class WinEDA_LibeditFrame */
	/*****************************/
BEGIN_EVENT_TABLE(WinEDA_LibeditFrame, wxFrame)
	COMMON_EVENTS_DRAWFRAME

	EVT_CLOSE(WinEDA_LibeditFrame::OnCloseWindow)
	EVT_SIZE(WinEDA_LibeditFrame::OnSize)

	EVT_TOOL_RANGE(ID_ZOOM_PLUS_BUTT, ID_ZOOM_PAGE_BUTT,
			WinEDA_LibeditFrame::Process_Zoom)

	// Tools et boutons de Libedit:

	/* Main horizontal toolbar */
	EVT_TOOL_RANGE( ID_LIBEDIT_START_H_TOOL, ID_LIBEDIT_END_H_TOOL,
					WinEDA_LibeditFrame::Process_Special_Functions)
	EVT_COMBOBOX(ID_LIBEDIT_SELECT_PART_NUMBER,
				WinEDA_LibeditFrame::Process_Special_Functions)
	EVT_COMBOBOX(ID_LIBEDIT_SELECT_ALIAS,
				WinEDA_LibeditFrame::Process_Special_Functions)

	/* Right Vertical toolbar */
	EVT_TOOL_RANGE( ID_LIBEDIT_START_V_TOOL, ID_LIBEDIT_END_V_TOOL,
					WinEDA_LibeditFrame::Process_Special_Functions)

	/* PopUp events and commands: */
	EVT_MENU_RANGE(ID_POPUP_START_RANGE, ID_POPUP_END_RANGE,
			WinEDA_LibeditFrame::Process_Special_Functions )

	// Annulation de commande en cours
	EVT_MENU_RANGE(ID_POPUP_GENERAL_START_RANGE, ID_POPUP_GENERAL_END_RANGE,
			WinEDA_LibeditFrame::Process_Special_Functions )

	// PopUp Menus pour Zooms traités dans drawpanel.cpp


END_EVENT_TABLE()


WinEDA_LibeditFrame::WinEDA_LibeditFrame(wxWindow * father, WinEDA_App *parent,
					const wxString & title, const wxPoint& pos, const wxSize& size):
			WinEDA_DrawFrame(father, LIBEDITOR_FRAME, parent, title, pos,  size)
{
	m_FrameName = "LibeditFrame";
	m_Draw_Axes = TRUE;				// TRUE pour avoir les axes dessines
	m_Draw_Grid = TRUE;				// TRUE pour avoir la axes dessinee

	// Give an icon
	SetIcon(wxIcon(libedit_xpm));
	m_CurrentScreen = ScreenLib;
	m_CurrentScreen->SetParentFrame(this);
	GetSettings();
	SetSize(m_FramePos.x, m_FramePos.y, m_FrameSize.x, m_FrameSize.y);
	if ( DrawPanel ) DrawPanel-> m_Block_Enable = TRUE;
	ReCreateHToolbar();
	ReCreateVToolbar();
	DisplayLibInfos();
	Show(TRUE);
}

/**********************************************/
WinEDA_LibeditFrame::~WinEDA_LibeditFrame(void)
/**********************************************/
{
	ScreenLib->SetParentFrame(NULL);
	m_Parent->LibeditFrame = NULL;
	m_CurrentScreen = ScreenSch;

}

/***********************************************************/
void WinEDA_LibeditFrame::OnCloseWindow(wxCloseEvent & Event)
/***********************************************************/
{
LibraryStruct *Lib;

	if( m_CurrentScreen->IsModify() )
	{
		if( ! IsOK(this, _("LibEdit: Part modified!, Continue ?") ) )
		{
			Event.Veto(); return;
		}
	}

	for (Lib = g_LibraryList; Lib != NULL; Lib = Lib->m_Pnext)
	{
		if( Lib->m_Modified )
		{
			wxString msg;
			msg.Printf( _("Library %s modified!, Continue ?"), Lib->m_Name.GetData());
			if( ! IsOK(this, msg) )
			{
				Event.Veto(); return;
			}
		}
	}

	SaveSettings();
	Destroy();
}


/******************************************/
void WinEDA_LibeditFrame::SetToolbars(void)
/******************************************/
/* Enable or disable tools of the differents toolbars,
	according to the current conditions or options
*/
{

	if( m_HToolBar == NULL ) return;

	if ( CurrentLib == NULL )
	{
		if ( m_HToolBar ) m_HToolBar->EnableTool(ID_LIBEDIT_SAVE_CURRENT_LIB,FALSE);
	}
	else
	{
		if ( m_HToolBar ) m_HToolBar->EnableTool(ID_LIBEDIT_SAVE_CURRENT_LIB,TRUE);
	}

	if ( CurrentLibEntry == NULL )
	{
		if ( m_HToolBar )
		{
			m_HToolBar->EnableTool(ID_LIBEDIT_IMPORT_PART,TRUE);
			m_HToolBar->EnableTool(ID_LIBEDIT_EXPORT_PART,FALSE);
			m_HToolBar->EnableTool(ID_LIBEDIT_CREATE_NEW_LIB_AND_SAVE_CURRENT_PART,FALSE);
			m_HToolBar->EnableTool(ID_LIBEDIT_SAVE_CURRENT_PART,FALSE);
			m_HToolBar->EnableTool(ID_DE_MORGAN_CONVERT_BUTT, FALSE);
			m_HToolBar->EnableTool(ID_DE_MORGAN_NORMAL_BUTT, FALSE);
			m_HToolBar->EnableTool(ID_LIBEDIT_EDIT_PIN_BY_PIN, FALSE);
			m_HToolBar->EnableTool(ID_LIBEDIT_VIEW_DOC, FALSE);
			m_HToolBar->EnableTool(ID_LIBEDIT_CHECK_PART, FALSE);
			m_SelpartBox->Enable( FALSE);
			m_HToolBar->EnableTool(ID_LIBEDIT_UNDO,FALSE);
			m_HToolBar->EnableTool(ID_LIBEDIT_REDO,FALSE);
		}
		g_EditPinByPinIsOn = FALSE;
		m_HToolBar->ToggleTool(ID_LIBEDIT_EDIT_PIN_BY_PIN, g_EditPinByPinIsOn);

		if ( m_VToolBar )
		{
			m_VToolBar->EnableTool(ID_LIBEDIT_BODY_TEXT_BUTT, FALSE);
			m_VToolBar->EnableTool(ID_LIBEDIT_BODY_LINE_BUTT, FALSE);
			m_VToolBar->EnableTool(ID_LIBEDIT_BODY_RECT_BUTT, FALSE);
			m_VToolBar->EnableTool(ID_LIBEDIT_BODY_CIRCLE_BUTT, FALSE);
			m_VToolBar->EnableTool(ID_LIBEDIT_BODY_ARC_BUTT, FALSE);
			m_VToolBar->EnableTool(ID_LIBEDIT_DELETE_ITEM_BUTT, FALSE);
			m_VToolBar->EnableTool(ID_LIBEDIT_ANCHOR_ITEM_BUTT, FALSE);
			m_VToolBar->EnableTool(ID_LIBEDIT_IMPORT_BODY_BUTT, FALSE);
			m_VToolBar->EnableTool(ID_LIBEDIT_EXPORT_BODY_BUTT, FALSE);
		}
	}

	else
	{
		if ( m_HToolBar )
		{
			m_HToolBar->EnableTool(ID_LIBEDIT_IMPORT_PART,TRUE);
			m_HToolBar->EnableTool(ID_LIBEDIT_EXPORT_PART,TRUE);
			m_HToolBar->EnableTool(ID_LIBEDIT_CREATE_NEW_LIB_AND_SAVE_CURRENT_PART,TRUE);
			m_HToolBar->EnableTool(ID_LIBEDIT_SAVE_CURRENT_PART,TRUE);
			if ( (CurrentLibEntry->m_UnitCount > 1) || g_AsDeMorgan )
				m_HToolBar->EnableTool(ID_LIBEDIT_EDIT_PIN_BY_PIN, TRUE);
			else
				m_HToolBar->EnableTool(ID_LIBEDIT_EDIT_PIN_BY_PIN, FALSE);

			m_HToolBar->ToggleTool(ID_LIBEDIT_EDIT_PIN_BY_PIN, g_EditPinByPinIsOn);

			m_HToolBar->EnableTool(ID_DE_MORGAN_CONVERT_BUTT, g_AsDeMorgan);
			m_HToolBar->EnableTool(ID_DE_MORGAN_NORMAL_BUTT, g_AsDeMorgan);
			/* Enable the "get doc" tool */
			bool enable_dtool = FALSE;
			if ( CurrentAliasName != "" )
			{
				int AliasLocation = LocateAlias( CurrentLibEntry->m_AliasList, CurrentAliasName);
				if ( AliasLocation >= 0 )
					if ( CurrentLibEntry->m_AliasList[AliasLocation+ALIAS_DOC_FILENAME] != "" )
						enable_dtool = TRUE;
			}
			else if ( CurrentLibEntry->m_DocFile != "" ) enable_dtool = TRUE;
			if ( enable_dtool ) m_HToolBar->EnableTool(ID_LIBEDIT_VIEW_DOC, TRUE);
			else m_HToolBar->EnableTool(ID_LIBEDIT_VIEW_DOC, FALSE);
			m_HToolBar->EnableTool(ID_LIBEDIT_CHECK_PART, TRUE);
			m_SelpartBox->Enable( (CurrentLibEntry->m_UnitCount > 1 ) ? TRUE : FALSE);

			if ( GetScreen() )
			{
				m_HToolBar->EnableTool(ID_LIBEDIT_UNDO,GetScreen()->m_UndoList);
				m_HToolBar->EnableTool(ID_LIBEDIT_REDO,GetScreen()->m_RedoList);
			}
		}

		if ( m_VToolBar )
		{
			m_VToolBar->EnableTool(ID_LIBEDIT_BODY_TEXT_BUTT, TRUE);
			m_VToolBar->EnableTool(ID_LIBEDIT_BODY_LINE_BUTT, TRUE);
			m_VToolBar->EnableTool(ID_LIBEDIT_BODY_RECT_BUTT, TRUE);
			m_VToolBar->EnableTool(ID_LIBEDIT_BODY_CIRCLE_BUTT, TRUE);
			m_VToolBar->EnableTool(ID_LIBEDIT_BODY_ARC_BUTT, TRUE);
			m_VToolBar->EnableTool(ID_LIBEDIT_DELETE_ITEM_BUTT, TRUE);
			m_VToolBar->EnableTool(ID_LIBEDIT_ANCHOR_ITEM_BUTT, TRUE);
			m_VToolBar->EnableTool(ID_LIBEDIT_IMPORT_BODY_BUTT, TRUE);
			m_VToolBar->EnableTool(ID_LIBEDIT_EXPORT_BODY_BUTT, TRUE);
		}
	}

	DisplayUnitsMsg();
}

/**************************************/
int WinEDA_LibeditFrame::BestZoom(void)
/**************************************/
{
int dx, dy, ii,jj ;
int bestzoom;
wxSize size;
EDA_Rect BoundaryBox;

	if ( CurrentLibEntry )
	{
		BoundaryBox = CurrentLibEntry->GetBoundaryBox(CurrentUnit, CurrentConvert);
		dx = BoundaryBox.GetWidth();
		dy = BoundaryBox.GetHeight();
	}

	else
	{
		dx = m_CurrentScreen->m_CurrentSheet->m_Size.x;
		dy = m_CurrentScreen->m_CurrentSheet->m_Size.y;
	}

	size =  DrawPanel->GetClientSize();
	size.x -= 60;	// Pour marges haut et bas
	ii = abs (dx / size.x);
	jj = abs (dy / size.y);

	/* determination du zoom existant le plus proche */
	ii = MAX(ii, jj);
	for ( bestzoom = 1 ;  bestzoom < 512 ;  bestzoom <<= 1 )
	{
		if(bestzoom > ii ) break;
	}

	if ( CurrentLibEntry )
	{
		m_CurrentScreen->m_Curseur = BoundaryBox.Centre();
		m_CurrentScreen->m_Curseur.y = - m_CurrentScreen->m_Curseur.y;
	}
	else
	{
		m_CurrentScreen->m_Curseur.x =  0;
		m_CurrentScreen->m_Curseur.y =  0;
	}

	return(bestzoom);
}



/*************************************************************************/
void WinEDA_LibeditFrame::Process_Special_Functions(wxCommandEvent& event)
/*************************************************************************/
{
int id = event.GetId();
wxPoint pos;
wxClientDC dc(DrawPanel);

	DrawPanel->PrepareGraphicContext(&dc);

	wxGetMousePosition(&pos.x, &pos.y);
	pos.y += 20;

	switch ( id )	// Arret de la commande de déplacement en cours
	{
		case ID_POPUP_LIBEDIT_END_CREATE_ITEM:
		case ID_POPUP_LIBEDIT_PIN_EDIT:
		case ID_POPUP_LIBEDIT_BODY_EDIT_ITEM:
		case ID_POPUP_LIBEDIT_FIELD_ROTATE_ITEM:
		case ID_POPUP_LIBEDIT_FIELD_EDIT_ITEM:
		case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINSIZE_ITEM:
		case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINNAMESIZE_ITEM:
		case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINNUMSIZE_ITEM:
		case ID_POPUP_LIBEDIT_CANCEL_EDITING :
		case ID_POPUP_ZOOM_BLOCK:
		case ID_POPUP_DELETE_BLOCK:
		case ID_POPUP_COPY_BLOCK:
		case ID_POPUP_SELECT_ITEMS_BLOCK:
		case ID_POPUP_INVERT_BLOCK:
		case ID_POPUP_PLACE_BLOCK:
		case ID_POPUP_LIBEDIT_DELETE_CURRENT_POLY_SEGMENT:
		case ID_POPUP_LIBEDIT_ROTATE_GRAPHIC_TEXT:
			break;

		case ID_POPUP_LIBEDIT_DELETE_ITEM:
			if ( m_CurrentScreen->ManageCurseur && m_CurrentScreen->ForceCloseManageCurseur )
					m_CurrentScreen->ForceCloseManageCurseur(this, &dc);
			break;

		default:
			if ( m_CurrentScreen->ManageCurseur && m_CurrentScreen->ForceCloseManageCurseur )
				m_CurrentScreen->ForceCloseManageCurseur(this, &dc);
			SetToolID( 0, wxCURSOR_ARROW, "" );
			break;
	}

	switch ( id )
	{
		case ID_LIBEDIT_SAVE_CURRENT_LIB:
			SaveActiveLibrary();
			break;

		case ID_LIBEDIT_NEW_PART:
		{
			g_EditPinByPinIsOn = FALSE;
			LibItemToRepeat = NULL;
			CreateNewLibraryPart();
			GetScreen()->ClearUndoRedoList();
			ReDrawPanel();
			SetToolbars();
			break;
		}

		case ID_LIBEDIT_SELECT_CURRENT_LIB:
			SelectActiveLibrary();
			break;

		case ID_LIBEDIT_SELECT_PART:
			LibItemToRepeat = NULL;
			if ( LoadOneLibraryPart() )
			{
				g_EditPinByPinIsOn = FALSE;
				GetScreen()->ClearUndoRedoList();
				SetToolbars();
			}
			ReDrawPanel();
			break;

		case ID_LIBEDIT_SAVE_CURRENT_PART:
			SaveOnePartInMemory();
			break;

		case ID_LIBEDIT_GET_FRAME_EDIT_PART:
			InstallLibeditFrame(pos);
			break;

		case ID_LIBEDIT_DELETE_PART:
			LibItemToRepeat = NULL;
			DeleteOnePart();
			break;

		case ID_LIBEDIT_IMPORT_PART:
			LibItemToRepeat = NULL;
			ImportOnePart();
			GetScreen()->ClearUndoRedoList();
			ReDrawPanel();
			break;

		case ID_LIBEDIT_EXPORT_PART:
			ExportOnePart(FALSE);
			break;

		case ID_LIBEDIT_CREATE_NEW_LIB_AND_SAVE_CURRENT_PART:
			ExportOnePart(TRUE);
			break;

		case ID_LIBEDIT_CHECK_PART :
			if ( CurrentLibEntry )
				if ( TestPins(CurrentLibEntry) == FALSE )
					DisplayInfo(this, _(" Tst Pins OK!") );
			break;

		case ID_DE_MORGAN_NORMAL_BUTT:
			m_HToolBar->ToggleTool(ID_DE_MORGAN_NORMAL_BUTT, TRUE);
			m_HToolBar->ToggleTool(ID_DE_MORGAN_CONVERT_BUTT, FALSE);
			LibItemToRepeat = NULL;
			CurrentConvert = 1;
			ReDrawPanel();
			break;

		case ID_DE_MORGAN_CONVERT_BUTT:
			m_HToolBar->ToggleTool(ID_DE_MORGAN_NORMAL_BUTT, FALSE);
			m_HToolBar->ToggleTool(ID_DE_MORGAN_CONVERT_BUTT, TRUE);
			LibItemToRepeat = NULL;
			CurrentConvert = 2;
			ReDrawPanel();
			break;

		case ID_LIBEDIT_VIEW_DOC:
			if ( CurrentLibEntry )
			{
			wxString docfilename;
				if ( CurrentAliasName != "" )
				{
					int AliasLocation = LocateAlias( CurrentLibEntry->m_AliasList, CurrentAliasName);
					if ( AliasLocation >= 0 )
						docfilename = CurrentLibEntry->m_AliasList[AliasLocation+ALIAS_DOC_FILENAME];
				}
				else docfilename = CurrentLibEntry->m_DocFile;

				if ( docfilename != "" )
					GetAssociatedDocument(this, g_RealLibDirBuffer, docfilename);
			}
			break;

		case ID_LIBEDIT_EDIT_PIN_BY_PIN:
			g_EditPinByPinIsOn = g_EditPinByPinIsOn ? FALSE : TRUE;
			m_HToolBar->ToggleTool(ID_LIBEDIT_EDIT_PIN_BY_PIN, g_EditPinByPinIsOn);
			break;

		case ID_LIBEDIT_SELECT_PART_NUMBER:
			{
			int ii = m_SelpartBox->GetSelection();
			if ( ii < 0 ) return;
			LibItemToRepeat = NULL;
			CurrentUnit = ii + 1;
			ReDrawPanel();
			}
			break;

		case ID_LIBEDIT_SELECT_ALIAS:
			{
			int ii = m_SelAliasBox->GetSelection();
			if ( ii < 0 ) return;
			LibItemToRepeat = NULL;
			if ( ii > 0 ) CurrentAliasName = m_SelAliasBox->GetValue();
			else CurrentAliasName = "";
			ReDrawPanel();
			}
			break;

		case ID_POPUP_LIBEDIT_PIN_EDIT:
			DrawPanel->m_IgnoreMouseEvents = TRUE;
			InstallPineditFrame(this, pos);
			DrawPanel->MouseToCursorSchema();
			DrawPanel->m_IgnoreMouseEvents = FALSE;
			break;

		case ID_LIBEDIT_PIN_BUTT:
			if ( CurrentLibEntry )
			{
				SetToolID( id, wxCURSOR_PENCIL, _("Add Pin"));
			}
			else
			{
				SetToolID( id, wxCURSOR_ARROW, _("Set Pin Opt"));
				InstallPineditFrame(this, pos);
				SetToolID( 0, wxCURSOR_ARROW, "");
			}
			break;

		case ID_POPUP_LIBEDIT_CANCEL_EDITING :
			if ( m_CurrentScreen->ManageCurseur && m_CurrentScreen->ForceCloseManageCurseur )
					m_CurrentScreen->ForceCloseManageCurseur(this, &dc);
			else
			SetToolID( 0, wxCURSOR_ARROW, "");
			break;

		case ID_NO_SELECT_BUTT:
			SetToolID( 0, wxCURSOR_ARROW, "");
			break;

		case ID_LIBEDIT_BODY_TEXT_BUTT:
			SetToolID( id, wxCURSOR_PENCIL, _("Add Text"));
			break;

		case ID_LIBEDIT_BODY_RECT_BUTT:
			SetToolID( id, wxCURSOR_PENCIL, _("Add Rectangle"));
			break;

		case ID_LIBEDIT_BODY_CIRCLE_BUTT:
			SetToolID( id, wxCURSOR_PENCIL, _("Add Circle"));
			break;

		case ID_LIBEDIT_BODY_ARC_BUTT:
			SetToolID( id, wxCURSOR_PENCIL, _("Add Arc"));
			break;

		case ID_LIBEDIT_BODY_LINE_BUTT:
			SetToolID( id, wxCURSOR_PENCIL, _("Add Line"));
			break;

		case ID_LIBEDIT_ANCHOR_ITEM_BUTT :
			SetToolID( id, wxCURSOR_HAND, _("Anchor"));
			break;

		case ID_LIBEDIT_IMPORT_BODY_BUTT :
			SetToolID( id, wxCURSOR_ARROW, _("Import"));
			LoadOneSymbol(&dc);
			SetToolID( 0, wxCURSOR_ARROW, "");
			break;

		case ID_LIBEDIT_EXPORT_BODY_BUTT :
			SetToolID( id, wxCURSOR_ARROW, _("Export"));
			SaveOneSymbol();
			SetToolID( 0, wxCURSOR_ARROW, "");
			break;

		case ID_POPUP_LIBEDIT_END_CREATE_ITEM:
			DrawPanel->MouseToCursorSchema();
			if ( CurrentDrawItem )
			{
				EndDrawGraphicItem(&dc);
			}
			break;

		case ID_POPUP_LIBEDIT_BODY_EDIT_ITEM:
			if ( CurrentDrawItem )
				{
				switch ( CurrentDrawItem->m_StructType )
					{
					case COMPONENT_ARC_DRAW_TYPE:
					case COMPONENT_CIRCLE_DRAW_TYPE:
					case COMPONENT_RECT_DRAW_TYPE:
					case COMPONENT_POLYLINE_DRAW_TYPE:
					case COMPONENT_LINE_DRAW_TYPE:
						DrawPanel->m_IgnoreMouseEvents = TRUE;
						EditGraphicSymbol(&dc, CurrentDrawItem);
						DrawPanel->MouseToCursorSchema();
						DrawPanel->m_IgnoreMouseEvents = FALSE;
						break;
					case COMPONENT_GRAPHIC_TEXT_DRAW_TYPE:
						DrawPanel->m_IgnoreMouseEvents = TRUE;
						EditSymbolText(&dc, CurrentDrawItem);
						DrawPanel->MouseToCursorSchema();
						DrawPanel->m_IgnoreMouseEvents = FALSE;
						break;
					}
				}
			break;


		case ID_LIBEDIT_DELETE_ITEM_BUTT:
			if ( CurrentLibEntry == NULL) { wxBell(); break; }
			SetToolID( id, wxCURSOR_BULLSEYE, _("Delete item"));
			break;


		case ID_POPUP_LIBEDIT_DELETE_CURRENT_POLY_SEGMENT:
			// Delete the last created segment, while creating a polyline draw item
			if ( CurrentDrawItem == NULL) break;
			DrawPanel->MouseToCursorSchema();
			DeleteDrawPoly(&dc);
			break;

		case ID_POPUP_LIBEDIT_DELETE_ITEM:
			if ( CurrentDrawItem == NULL) break;
			DrawPanel->MouseToCursorSchema();
			SaveCopyInUndoList();
			if ( CurrentDrawItem->m_StructType == COMPONENT_PIN_DRAW_TYPE )
			{
				DeletePin(&dc, CurrentLibEntry, (LibDrawPin*)CurrentDrawItem);
			}

			else
			{
				if ( m_CurrentScreen->ManageCurseur && m_CurrentScreen->ForceCloseManageCurseur)
					m_CurrentScreen->ForceCloseManageCurseur(this, &dc);
				else DeleteOneLibraryDrawStruct(DrawPanel, &dc, CurrentLibEntry,CurrentDrawItem, TRUE);
			}

			CurrentDrawItem = NULL;
			m_CurrentScreen->SetModify();
			break;

		case ID_POPUP_LIBEDIT_MOVE_ITEM_REQUEST:
			if ( CurrentDrawItem == NULL) break;
			DrawPanel->MouseToCursorSchema();
			if ( CurrentDrawItem->m_StructType == COMPONENT_PIN_DRAW_TYPE )
				StartMovePin(&dc);
			else if ( CurrentDrawItem->m_StructType == COMPONENT_FIELD_DRAW_TYPE )
				StartMoveField(&dc, (LibDrawField *) CurrentDrawItem);
			else StartMoveDrawSymbol(&dc);
			break;

		case ID_POPUP_LIBEDIT_ROTATE_GRAPHIC_TEXT:
			if ( CurrentDrawItem == NULL) break;
			DrawPanel->MouseToCursorSchema();
			if ( (CurrentDrawItem->m_Flags & IS_NEW) == 0 )
				SaveCopyInUndoList();
			RotateSymbolText(&dc);
			break;

		case ID_POPUP_LIBEDIT_FIELD_ROTATE_ITEM:
			if ( CurrentDrawItem == NULL) break;
			DrawPanel->MouseToCursorSchema();
			if ( CurrentDrawItem->m_StructType == COMPONENT_FIELD_DRAW_TYPE )
			{
				SaveCopyInUndoList();
				RotateField(&dc, (LibDrawField *) CurrentDrawItem);
			}
			break;

		case ID_POPUP_LIBEDIT_FIELD_EDIT_ITEM:
			if ( CurrentDrawItem == NULL) break;
			DrawPanel->m_IgnoreMouseEvents = TRUE;
			if ( CurrentDrawItem->m_StructType == COMPONENT_FIELD_DRAW_TYPE )
			{
				EditField(&dc, (LibDrawField *) CurrentDrawItem);
			}
			DrawPanel->MouseToCursorSchema();
			DrawPanel->m_IgnoreMouseEvents = FALSE;
			break;

		case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINSIZE_ITEM:
		case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINNAMESIZE_ITEM:
		case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINNUMSIZE_ITEM:
			if ( (CurrentDrawItem == NULL) ||
				 (CurrentDrawItem->m_StructType != COMPONENT_PIN_DRAW_TYPE) )
				break;
			SaveCopyInUndoList();
			GlobalSetPins(&dc, (LibDrawPin *) CurrentDrawItem, id);
			DrawPanel->MouseToCursorSchema();
			break;

		case ID_POPUP_ZOOM_BLOCK:
			DrawPanel->m_AutoPAN_Request = FALSE;
			GetScreen()->BlockLocate.m_Command = BLOCK_ZOOM;
			HandleBlockEnd(&dc);
			break;

		case ID_POPUP_DELETE_BLOCK:
			DrawPanel->m_AutoPAN_Request = FALSE;
			GetScreen()->BlockLocate.m_Command = BLOCK_DELETE;
			DrawPanel->MouseToCursorSchema();
			HandleBlockEnd(&dc);
			break;

		case ID_POPUP_COPY_BLOCK:
			DrawPanel->m_AutoPAN_Request = FALSE;
			GetScreen()->BlockLocate.m_Command = BLOCK_COPY;
			DrawPanel->MouseToCursorSchema();
			HandleBlockPlace(&dc);
			break;

		case ID_POPUP_SELECT_ITEMS_BLOCK:
			DrawPanel->m_AutoPAN_Request = FALSE;
			GetScreen()->BlockLocate.m_Command = BLOCK_SELECT_ITEMS_ONLY;
			DrawPanel->MouseToCursorSchema();
			HandleBlockEnd(&dc);
			break;

		case ID_POPUP_INVERT_BLOCK:
			DrawPanel->m_AutoPAN_Request = FALSE;
			GetScreen()->BlockLocate.m_Command = BLOCK_INVERT;
			DrawPanel->MouseToCursorSchema();
			HandleBlockPlace(&dc);
			break;

		case ID_POPUP_PLACE_BLOCK:
			DrawPanel->m_AutoPAN_Request = FALSE;
			DrawPanel->MouseToCursorSchema();
			HandleBlockPlace(&dc);
			break;

		case ID_LIBEDIT_UNDO:
			GetComponentFromUndoList();
			DrawPanel->Refresh(TRUE);
			break;

		case ID_LIBEDIT_REDO:
			GetComponentFromRedoList();
			DrawPanel->Refresh(TRUE);
			break;

		default:
			DisplayError(this, "WinEDA_LibeditFrame::Process_Special_Functions error");
			break;
	}

	if ( m_ID_current_state == 0 ) LibItemToRepeat = NULL;
}


