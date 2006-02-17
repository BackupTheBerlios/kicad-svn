	/********************************************/
	/* sheet.cpp   Module to handle Place Sheet */
	/********************************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"

#include "protos.h"

/* Routines Locales */
static void ExitSheet(WinEDA_DrawFrame * frame, wxDC * DC);
static void DeplaceSheet(WinEDA_DrawPanel * panel, wxDC * DC, bool erase);

/* Variables locales, communes a sheetlab.cpp */

/* Variables locales */
static int SheetMindx, SheetMindy;
static wxPoint OldPos;	/* Ancienne pos pour annulation ReSize ou move */

	/************************************/
	/* class WinEDA_SheetPropertiesFrame */
	/************************************/

enum id_Sheetedit
{
	ID_ACCEPT_SHEET_PROPERTIES = 1980,
	ID_CANCEL_SHEET_PROPERTIES
};


class WinEDA_SheetPropertiesFrame: public wxDialog
{
private:

	WinEDA_SchematicFrame * m_Parent;
	DrawSheetStruct * m_CurrentSheet;
	WinEDA_GraphicTextCtrl * m_FileNameWin;
	WinEDA_GraphicTextCtrl * m_SheetNameWin;

public:
	// Constructor and destructor
	WinEDA_SheetPropertiesFrame(WinEDA_SchematicFrame *parent,
					DrawSheetStruct * currentsheet, const wxPoint & framepos);
	~WinEDA_SheetPropertiesFrame(void){};

private:
	void SheetPropertiesAccept(wxCommandEvent& event);
	void SheetPropertiesCancel(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WinEDA_SheetPropertiesFrame, wxDialog)
	EVT_BUTTON(ID_ACCEPT_SHEET_PROPERTIES,
				WinEDA_SheetPropertiesFrame::SheetPropertiesAccept)
	EVT_BUTTON(ID_CANCEL_SHEET_PROPERTIES,
				WinEDA_SheetPropertiesFrame::SheetPropertiesCancel)
END_EVENT_TABLE()


WinEDA_SheetPropertiesFrame::WinEDA_SheetPropertiesFrame(
				WinEDA_SchematicFrame *parent,
				DrawSheetStruct * currentsheet,
				const wxPoint & framepos):
		wxDialog(parent, -1, _("Sheet properties"), framepos, wxSize(320, 160),
				DIALOG_STYLE)
{
wxPoint pos;
wxString number;

	m_Parent = parent;
	Centre();

	m_CurrentSheet = currentsheet;


	pos.x = 10; pos.y = 20;

	m_FileNameWin = new WinEDA_GraphicTextCtrl(this, _("Filename:"),
				m_CurrentSheet->m_Field[SHEET_FILENAME].m_Text,
				m_CurrentSheet->m_Field[SHEET_FILENAME].m_Size.x,
				UnitMetric , pos, 200);

	pos.y += 15 + m_FileNameWin->GetDimension().y;
	m_SheetNameWin = new WinEDA_GraphicTextCtrl(this, _("Sheetname:"),
				m_CurrentSheet->m_Field[VALUE].m_Text,
				m_CurrentSheet->m_Field[VALUE].m_Size.x,
				UnitMetric , pos, 200);

	/* Creation des boutons de commande */
	pos.x = 60; pos.y += 15 + m_SheetNameWin->GetDimension().y;
	wxButton * Button = new wxButton(this, ID_CANCEL_SHEET_PROPERTIES,
						_("&Cancel"), pos);
	Button->SetForegroundColour(*wxBLUE);
						
	pos.x += Button->GetSize().x + 10;
	Button = new wxButton(this, ID_ACCEPT_SHEET_PROPERTIES,
						_("&OK"), pos);
	Button->SetForegroundColour(*wxRED);
}

/*****************************************************************************/
void WinEDA_SheetPropertiesFrame::SheetPropertiesCancel(wxCommandEvent& event)
/*****************************************************************************/
{
	EndModal(FALSE);
}


/*****************************************************************************/
void WinEDA_SheetPropertiesFrame::SheetPropertiesAccept(wxCommandEvent& event)
/*****************************************************************************/
{
wxString FileName;
	
	FileName = m_FileNameWin->GetText();
	FileName.Trim(FALSE); FileName.Trim(TRUE);
	
	if ( FileName.IsEmpty() )
		{
		DisplayError(this, _("No Filename! Aborted") );
		EndModal (FALSE);
		return;
		}

	ChangeFileNameExt( FileName,g_SchExtBuffer ); 
	m_CurrentSheet->m_Field[SHEET_FILENAME].m_Text = FileName;

	m_CurrentSheet->m_Field[SHEET_FILENAME].m_Size.x = 
		m_CurrentSheet->m_Field[SHEET_FILENAME].m_Size.y =
		m_FileNameWin->GetTextSize();

	m_CurrentSheet->m_Field[VALUE].m_Text = m_SheetNameWin->GetText();
	m_CurrentSheet->m_Field[VALUE].m_Size.x = 
		m_CurrentSheet->m_Field[VALUE].m_Size.y =
		m_SheetNameWin->GetTextSize();

	if( (m_CurrentSheet->m_Field[VALUE].m_Text.IsEmpty() ) )
		m_CurrentSheet->m_Field[VALUE].m_Text =
			m_CurrentSheet->m_Field[SHEET_FILENAME].m_Text;

	EndModal(TRUE);
}

/*************************************************************************/
bool WinEDA_SchematicFrame::EditSheet(DrawSheetStruct * Sheet, wxDC * DC)
/*************************************************************************/
/* Routine de modification des textes (Name et FileName) de la Sheet */
{
BASE_SCREEN * ScreenSheet = NULL;
WinEDA_SheetPropertiesFrame * frame;
bool edit = TRUE;

	if ( Sheet == NULL ) return FALSE;

	ScreenSheet = (BASE_SCREEN*)Sheet->m_Son;

	/* Demande du nouveau texte */
	RedrawOneStruct(DrawPanel, DC, Sheet, XOR_MODE);

	DrawPanel->m_IgnoreMouseEvents = TRUE;
	frame = new WinEDA_SheetPropertiesFrame(this, Sheet,wxPoint(-1,-1) );
	edit = frame->ShowModal(); frame->Destroy();
	DrawPanel->MouseToCursorSchema();
	DrawPanel->m_IgnoreMouseEvents = FALSE;
	if ( edit )
		{
		/* Correction du nom fichier dans la structure SCREEN correspondante */
		if( ScreenSheet )
			{
			ScreenSheet->m_FileName = Sheet->m_Field[SHEET_FILENAME].m_Text;
			}
		SetFlagModify(GetScreen());
		}

	RedrawOneStruct(DrawPanel, DC, Sheet, GR_DEFAULT_DRAWMODE);
	return edit;
}


/****************************************************************/
DrawSheetStruct * WinEDA_SchematicFrame::CreateSheet(wxDC * DC)
/****************************************************************/
/* Routine de Creation d'une feuille de hierarchie (Sheet) */
{
	g_ItemToRepeat = NULL;

	DrawSheetStruct * Sheet = new DrawSheetStruct(m_CurrentScreen->m_Curseur);

	Sheet->m_Flags = IS_NEW | IS_RESIZED;
	Sheet->m_TimeStamp = GetTimeStamp();
	Sheet->m_Parent = m_CurrentScreen;
	SheetMindx = SheetMindy = 0;

	m_CurrentScreen->m_CurrentItem = Sheet;

	m_CurrentScreen->ManageCurseur = DeplaceSheet;
	m_CurrentScreen->ForceCloseManageCurseur = ExitSheet;

	m_CurrentScreen->ManageCurseur(DrawPanel, DC, FALSE);

	return Sheet;
}


/*******************************************************************************/
void WinEDA_SchematicFrame::ReSizeSheet(DrawSheetStruct * Sheet, wxDC * DC)
/*******************************************************************************/
{
DrawSheetLabelStruct* sheetlabel;

	if ( Sheet == NULL ) return;
	if( Sheet->m_Flags & IS_NEW) return;

	if( Sheet->m_StructType != DRAW_SHEET_STRUCT_TYPE)
		{
		DisplayError(this, wxT("WinEDA_SchematicFrame::ReSizeSheet: Bad SructType"));
		return;
		}

	SetFlagModify(GetScreen());
	Sheet->m_Flags |= IS_RESIZED;

	/* sauvegarde des anciennes valeurs */
	OldPos.x = Sheet->m_End.x;
	OldPos.y = Sheet->m_End.y;

	/* Recalcul des dims min de la sheet */
	SheetMindx = SheetMindy = 0;
	sheetlabel = Sheet->m_Label;
	while(sheetlabel)
		{
		SheetMindx = MAX(SheetMindx,
					(int)((sheetlabel->GetLength()+1) * sheetlabel->m_Size.x) );
		SheetMindy = MAX(SheetMindy,sheetlabel->m_Pos.y - Sheet->m_Pos.y);
		sheetlabel = (DrawSheetLabelStruct *) sheetlabel->Pnext;
		}
	m_CurrentScreen->ManageCurseur = DeplaceSheet ;
	m_CurrentScreen->ForceCloseManageCurseur = ExitSheet;
	m_CurrentScreen->ManageCurseur(DrawPanel, DC, TRUE);
}


/*********************************************************************************/
void WinEDA_SchematicFrame::StartMoveSheet(DrawSheetStruct* Sheet, wxDC * DC)
/*********************************************************************************/
{
	if ( (Sheet == NULL) || ( Sheet->m_StructType != DRAW_SHEET_STRUCT_TYPE)  )
		return;

	OldPos = Sheet->m_Pos;
	Sheet->m_Flags |= IS_MOVED;
	m_CurrentScreen->ManageCurseur = DeplaceSheet ;
	m_CurrentScreen->ForceCloseManageCurseur = ExitSheet;
	m_CurrentScreen->ManageCurseur(DrawPanel, DC, TRUE);
}

/********************************************************/
/*** 	Routine de deplacement Sheet, lie au curseur.	*/
/*  Appele par GeneralControle grace a  ManageCurseur.  */
/********************************************************/
static void DeplaceSheet(WinEDA_DrawPanel * panel, wxDC * DC, bool erase)
{
int dx, dy;
DrawSheetLabelStruct * SheetLabel;
BASE_SCREEN * screen = panel->m_Parent->m_CurrentScreen;

DrawSheetStruct * Sheet = (DrawSheetStruct *)
					screen->m_CurrentItem;

	/* Effacement du composant: tj apres depl curseur */
	if( erase )
		RedrawOneStruct(panel, DC, Sheet, XOR_MODE);

	if( Sheet->m_Flags & IS_RESIZED)
		{
		Sheet->m_End.x = MAX(SheetMindx,
							screen->m_Curseur.x	- Sheet->m_Pos.x);
		Sheet->m_End.y = MAX(SheetMindy,
							screen->m_Curseur.y	- Sheet->m_Pos.y);
		SheetLabel = Sheet->m_Label;
		while(SheetLabel)
			{
			if( SheetLabel->m_Edge)
				SheetLabel->m_Pos.x = Sheet->m_Pos.x + Sheet->m_End.x;
			SheetLabel = (DrawSheetLabelStruct *) SheetLabel->Pnext;
			}
		}
	else	/* Move Sheet */
		{
		dx = screen->m_Curseur.x - Sheet->m_Pos.x;
		dy = screen->m_Curseur.y - Sheet->m_Pos.y;
		MoveOneStruct(Sheet, dx, dy);
		}

	RedrawOneStruct(panel, DC, Sheet, XOR_MODE);
}

/****************************************************************/
/* Placement d'un symbole "sheet", apres création ou déplacement */
/****************************************************************/
void DrawSheetStruct::Place( WinEDA_DrawFrame * frame, wxDC * DC)
{
	/* Placement en liste des structures si nouveau composant:*/
	if(m_Flags & IS_NEW)
	{
		if ( ! ((WinEDA_SchematicFrame*)frame)->EditSheet(this, DC) )
		{
			frame->m_CurrentScreen->m_CurrentItem = NULL;
			frame->m_CurrentScreen->ManageCurseur = NULL;
			frame->m_CurrentScreen->ForceCloseManageCurseur = NULL;
			RedrawOneStruct(frame->DrawPanel, DC, this, XOR_MODE);
			delete this;
			return;
		}


		if ( wxFileExists(m_Field[SHEET_FILENAME].m_Text) )
		{
			if ( m_Son == NULL )
			{
				SCH_SCREEN * newscreen = ScreenSch ;
				while(newscreen->Pnext) newscreen = (SCH_SCREEN*) newscreen->Pnext;
				newscreen->Pnext = CreateNewScreen(frame, newscreen, m_TimeStamp);

				newscreen = (SCH_SCREEN*)newscreen->Pnext;
				m_Son = newscreen;
				newscreen->m_Parent = this;
				newscreen->m_FileName = m_Field[SHEET_FILENAME].m_Text;
				newscreen->m_RootSheet = this;
			}
				((WinEDA_SchematicFrame*)frame)->LoadOneSheet(
					(SCH_SCREEN*)m_Son, m_Field[SHEET_FILENAME].m_Text);
		}
	}

	EDA_BaseStruct::Place(frame, DC);
}

/****************************************/
/*  Routine de sortie du Menu de Sheet  */
/****************************************/
static void ExitSheet(WinEDA_DrawFrame * frame, wxDC * DC)
{
DrawSheetStruct * Sheet = (DrawSheetStruct *)
		frame->m_CurrentScreen->m_CurrentItem;

	if ( Sheet == NULL) return;

	/* Deplacement composant en cours */
	if ( Sheet->m_Flags & IS_NEW ) /* Nouveau Placement en cours, on l'efface */
		{
		RedrawOneStruct(frame->DrawPanel, DC, Sheet, XOR_MODE);
		delete Sheet;
		}
	else if ( Sheet->m_Flags & IS_RESIZED )/* resize en cours: on l'annule */
		{
		RedrawOneStruct(frame->DrawPanel, DC, Sheet, XOR_MODE);
		Sheet->m_End = OldPos;
		RedrawOneStruct(frame->DrawPanel, DC, Sheet, GR_DEFAULT_DRAWMODE);
		Sheet->m_Flags = 0;
		}

	else if ( Sheet->m_Flags & IS_MOVED )/* move en cours: on l'annule */
		{
		wxPoint curspos = frame->m_CurrentScreen->m_Curseur;
		frame->m_CurrentScreen->m_Curseur = OldPos;
		DeplaceSheet(frame->DrawPanel, DC, TRUE);
		RedrawOneStruct(frame->DrawPanel, DC, Sheet, GR_DEFAULT_DRAWMODE);
		Sheet->m_Flags = 0;
		frame->m_CurrentScreen->m_Curseur = curspos;
		}

	else Sheet->m_Flags = 0;

	frame->m_CurrentScreen->m_CurrentItem = NULL;
	frame->m_CurrentScreen->ManageCurseur = NULL;
	frame->m_CurrentScreen->ForceCloseManageCurseur =NULL;
}



/********************************************************************/
void DrawSheetStruct::CleanupSheet(WinEDA_SchematicFrame * frame, wxDC *DC)
/********************************************************************/
/* Delete pinsheets which are not corresponding to a Global label
 if DC != NULL, redraw Sheet
*/
{
DrawSheetLabelStruct* Pinsheet, * NextPinsheet;
SCH_SCREEN * Screen = (SCH_SCREEN*) m_Son;
	
	if ( Screen == NULL )
	{
		DisplayError(frame, _("This sheet has no data, cleanup aborted"));
		return;
	}

	if( ! IsOK(frame, _("Ok to cleanup this sheet") ) ) return;
		
	Pinsheet = m_Label;
	while(Pinsheet)
	{
		/* Search Glabel corresponding to this Pinsheet */

		EDA_BaseStruct *DrawStruct = Screen->EEDrawList;
		DrawGlobalLabelStruct * GLabel = NULL;
		for (; DrawStruct != NULL; DrawStruct = DrawStruct->Pnext )
		{
			if( DrawStruct->m_StructType != DRAW_GLOBAL_LABEL_STRUCT_TYPE )
				continue;
			GLabel = (DrawGlobalLabelStruct *) DrawStruct;
			if( Pinsheet->m_Text.CmpNoCase(GLabel->m_Text) == 0 )
				break;	// Found!
			GLabel = NULL;
		}

		NextPinsheet = (DrawSheetLabelStruct *) Pinsheet->Pnext;
		if ( GLabel == NULL )	// Glabel not found: delete pinsheet
		{
			SetFlagModify(frame->GetScreen());
			frame->DeleteSheetLabel(DC, Pinsheet);
		}
		Pinsheet = NextPinsheet;
	}
}
