/**************************************************************************/
/* EESchema											  					  */
/* editexte.cpp: creation/ editions des textes (labels, textes sur schema) */
/**************************************************************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"

#include "protos.h"

/* Fonctions locales */
static void ShowWhileMoving(WinEDA_DrawPanel * panel, wxDC * DC, bool erase);
static void ExitMoveTexte(WinEDA_DrawFrame * frame, wxDC *DC);

/* Variables locales */
static wxPoint ItemInitialPosition;
static int OldOrient;
static wxSize OldSize;
static int ShapeGLabel = (int) NET_INPUT;
static int TextLabelSize = DEFAULT_SIZE_TEXT;


	/************************************/
	/* class WinEDA_TextPropertiesFrame */
	/************************************/

#define NBSHAPES 5
static wxString shape_list[NBSHAPES] =
{
 "Input", "Output", "Bidi", "TriState", "Passive"
};

enum id_Textdit
{
	ID_ACCEPT_TEXT_PROPERTIES = 1950,
	ID_CLOSE_TEXT_PROPERTIES
};


class WinEDA_TextPropertiesFrame: public wxDialog
{
private:

	WinEDA_SchematicFrame * m_Parent;
	DrawTextStruct * m_CurrentText;
	wxRadioBox * m_TextType;
	wxRadioBox * m_TextShape;
	wxRadioBox * m_TextOrient;
	WinEDA_GraphicTextCtrl * m_TextWin;

public:
	// Constructor and destructor
	WinEDA_TextPropertiesFrame(WinEDA_SchematicFrame *parent,
					DrawTextStruct * CurrentText, const wxPoint & framepos);
	~WinEDA_TextPropertiesFrame(void){};

private:
	void TextPropertiesAccept(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnCloseFrame(wxCloseEvent& event);

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WinEDA_TextPropertiesFrame, wxDialog)
	EVT_CLOSE(WinEDA_TextPropertiesFrame::OnCloseFrame)
	EVT_BUTTON(ID_ACCEPT_TEXT_PROPERTIES, WinEDA_TextPropertiesFrame::TextPropertiesAccept)
	EVT_BUTTON(ID_CLOSE_TEXT_PROPERTIES, WinEDA_TextPropertiesFrame::OnQuit)
END_EVENT_TABLE()


WinEDA_TextPropertiesFrame::WinEDA_TextPropertiesFrame(
				WinEDA_SchematicFrame *parent,
				DrawTextStruct * CurrentText,
				const wxPoint & framepos):
		wxDialog(parent, -1, "", framepos, wxSize(340, 220), DIALOG_STYLE)
{
wxPoint pos;
wxString number;
wxButton * Button;

	m_Parent = parent;
	Centre();

	m_TextShape = NULL;
	m_CurrentText = CurrentText;

	switch( m_CurrentText->m_StructType )
		{
		case DRAW_GLOBAL_LABEL_STRUCT_TYPE:
			SetTitle(_("Global Label properties"));
			break;

		case DRAW_LABEL_STRUCT_TYPE:
			SetTitle(_("Label properties"));
			break;

		default:
			SetTitle(_("Text properties"));
			break;
		}

	/* Creation des boutons de commande */
	pos.x = 240; pos.y = 90;
	Button = new wxButton(this, ID_ACCEPT_TEXT_PROPERTIES,
						_("Ok"), pos);
	Button->SetForegroundColour(*wxRED);
	Button->SetDefault();

	pos.y += Button->GetDefaultSize().y + 10;
	Button = new wxButton(this, ID_CLOSE_TEXT_PROPERTIES,
						_("Close"), pos);
	Button->SetForegroundColour(*wxBLUE);						

	pos.x = 10; pos.y = 35;

	m_TextWin = new WinEDA_GraphicTextCtrl(this, "Text:",
				CurrentText->m_Text, CurrentText->m_Size.x,
				UnitMetric , pos, 200);
	m_TextWin->SetFocus();

	// Selection de l'orientation :
	pos.x = 15;
	pos.y += m_TextWin->GetDimension().y + 10;
	wxString orient_list[4] =
		{ _("Right"), _("Up"), _("Left"), _("Down") };
	m_TextOrient = new wxRadioBox(this, -1, _("Text Orient:"),
				pos, wxSize(-1,-1),
				4, orient_list, 1);
	m_TextOrient->SetSelection(CurrentText->m_Orient);

	// Glabel Shape
	int w, h; m_TextOrient->GetSize(&w, &h);
	pos.x += w + 10;
	if (CurrentText->m_StructType == DRAW_GLOBAL_LABEL_STRUCT_TYPE)
	{
		m_TextShape = new wxRadioBox(this, -1, _("Glabel Shape:"),
				pos, wxSize(-1,-1),
				NBSHAPES, shape_list, 1);
		m_TextShape->SetSelection( CurrentText->m_Shape );
	}
}

/******************************************************************/
void WinEDA_TextPropertiesFrame::OnCloseFrame(wxCloseEvent& event)
/******************************************************************/
{
	m_Parent->DrawPanel->MouseToCursorSchema();
	EndModal(0);
}

/************************************************************************/
void  WinEDA_TextPropertiesFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
/************************************************************************/
{
    // true is to force the frame to close
    Close(true);
}

void WinEDA_TextPropertiesFrame::TextPropertiesAccept(wxCommandEvent& event)
{
wxString newtext;

	newtext = m_TextWin->GetText();

	if ( newtext != "" ) m_CurrentText->m_Text = newtext;
	else if ( (m_CurrentText->m_Flags & IS_NEW) == 0 )
		DisplayError(this, "Empty Text!");

	m_CurrentText->m_Orient = m_TextOrient->GetSelection();
	m_CurrentText->m_Size.x = m_CurrentText->m_Size.y = m_TextWin->GetTextSize();
	if ( m_TextShape ) m_CurrentText->m_Shape = m_TextShape->GetSelection();

	SetFlagModify(m_Parent->GetScreen());

	Close(TRUE);
}



/********************************************************************************/
void WinEDA_SchematicFrame::StartMoveTexte(DrawTextStruct * TextStruct, wxDC *DC)
/********************************************************************************/
{
	if(TextStruct == NULL) return;

	g_ItemToRepeat = NULL;
	TextStruct->m_Flags |= IS_MOVED;

	switch( TextStruct->m_StructType )
		{
		case DRAW_LABEL_STRUCT_TYPE:
		case DRAW_GLOBAL_LABEL_STRUCT_TYPE:
		case DRAW_TEXT_STRUCT_TYPE:
			ItemInitialPosition = TextStruct->m_Pos;
			OldSize = TextStruct->m_Size; OldOrient = TextStruct->m_Orient;
			break;

		default: break;
		}

	SetFlagModify(GetScreen());
	m_CurrentScreen->ManageCurseur = ShowWhileMoving;
	m_CurrentScreen->ForceCloseManageCurseur = ExitMoveTexte;
	m_CurrentScreen->ManageCurseur(DrawPanel, DC, TRUE);
}



/*************************************************************************/
void WinEDA_SchematicFrame::EditSchematicText(DrawTextStruct * TextStruct,
			wxDC * DC)
/*************************************************************************/
/* Changement du texte (Label.. ) pointe par la souris
*/
{
	if(TextStruct == NULL)  return;

	RedrawOneStruct(DrawPanel, DC, TextStruct, XOR_MODE);

	DrawPanel->m_IgnoreMouseEvents = TRUE;
WinEDA_TextPropertiesFrame * frame = new WinEDA_TextPropertiesFrame(this,
	TextStruct, wxPoint(30,30));
	frame->ShowModal(); frame->Destroy();

	DrawPanel->m_IgnoreMouseEvents = FALSE;
	RedrawOneStruct(DrawPanel, DC, TextStruct, GR_DEFAULT_DRAWMODE);
}


/***********************************************************************************/
void WinEDA_SchematicFrame::ChangeTextOrient(DrawTextStruct * TextStruct, wxDC * DC)
/***********************************************************************************/
{
	if( TextStruct == NULL )
		TextStruct = (DrawTextStruct *) PickStruct(GetScreen()->m_Curseur,
				GetScreen()->EEDrawList, TEXTITEM|LABELITEM);
	if( TextStruct == NULL ) return;

	/* Effacement du texte en cours */
	RedrawOneStruct(DrawPanel, DC, TextStruct, XOR_MODE);

	/* Rotation du texte */
	switch( TextStruct->m_StructType )
		{
		case DRAW_LABEL_STRUCT_TYPE:
		case DRAW_GLOBAL_LABEL_STRUCT_TYPE:
		case DRAW_TEXT_STRUCT_TYPE:
			((DrawTextStruct*)TextStruct)->m_Orient++;
			((DrawTextStruct*)TextStruct)->m_Orient &= 3;
			break;

		default: break;
		}

	SetFlagModify(GetScreen());

	/* Reaffichage */
	RedrawOneStruct(DrawPanel, DC, TextStruct, XOR_MODE);
}

/*************************************************************************/
EDA_BaseStruct * WinEDA_SchematicFrame::CreateNewText(wxDC * DC, int type)
/*************************************************************************/
/* Routine to create new text struct (GraphicText, label or Glabel).
*/
{
DrawTextStruct * NewText =  NULL;

	g_ItemToRepeat = NULL;

	switch( type )
		{
		case LAYER_NOTES:
			NewText = new DrawTextStruct(m_CurrentScreen->m_Curseur);
			NewText->m_Size.x = NewText->m_Size.y = TextLabelSize;
			break;

		case LAYER_LOCLABEL:
			{
			NewText = new DrawLabelStruct(m_CurrentScreen->m_Curseur);
			NewText->m_Size.x = NewText->m_Size.y = TextLabelSize;
			}
			break;

		case LAYER_GLOBLABEL:
			NewText = new DrawGlobalLabelStruct(m_CurrentScreen->m_Curseur);
			NewText->m_Size.x = NewText->m_Size.y = TextLabelSize;
			((DrawGlobalLabelStruct*)NewText)->m_Shape = ShapeGLabel;
			break;

		default:
			DisplayError(this, "Editexte: Internal error");
			break;
		}

	NewText->m_Flags = IS_NEW | IS_MOVED;

	EditSchematicText(NewText, DC);

	if ( NewText->m_Text.IsEmpty() )
	{
		delete NewText;
		return NULL;
	}

	RedrawOneStruct(DrawPanel, DC, NewText, GR_DEFAULT_DRAWMODE);
	m_CurrentScreen->ManageCurseur = ShowWhileMoving;
	m_CurrentScreen->ForceCloseManageCurseur = ExitMoveTexte;

	m_CurrentScreen->m_CurrentItem = NewText;

	return NewText;
}



	/****************************************/
	/*		Dessin du Texte en deplacement	*/
	/****************************************/
static void ShowWhileMoving(WinEDA_DrawPanel * panel, wxDC * DC, bool erase)
{
EDA_BaseStruct * TextStruct = panel->GetScreen()->m_CurrentItem;

	/* effacement ancienne position */
	if( erase )
		RedrawOneStruct(panel, DC, TextStruct, XOR_MODE);

	/* Redessin du texte */
	switch( TextStruct->m_StructType )
		{
		case DRAW_LABEL_STRUCT_TYPE:
		case DRAW_GLOBAL_LABEL_STRUCT_TYPE:
		case DRAW_TEXT_STRUCT_TYPE:
			((DrawTextStruct*)TextStruct)->m_Pos = panel->GetScreen()->m_Curseur;
			break;

		default: break;
		}

	RedrawOneStruct(panel, DC, TextStruct, XOR_MODE);
}


/*************************************************************/
static void ExitMoveTexte(WinEDA_DrawFrame * frame, wxDC *DC)
/*************************************************************/
/* Routine de sortie des menus de Texte */
{
EDA_BaseStruct * Struct = frame->m_CurrentScreen->m_CurrentItem;

	g_ItemToRepeat = NULL;
	frame->m_CurrentScreen->ManageCurseur = NULL;
	frame->m_CurrentScreen->ForceCloseManageCurseur = NULL;

	if( Struct == NULL)  /* Pas de trace en cours  */
		{
		return;
		}

	/* ici : trace en cours */

	/* Effacement du trace en cours et suppression eventuelle de la structure */
	RedrawOneStruct(frame->DrawPanel, DC, Struct, XOR_MODE);

	if( Struct->m_Flags & IS_NEW )	/* Suppression du nouveau texte en cours de placement */
		{
		delete Struct;
		frame->m_CurrentScreen->m_CurrentItem = NULL;
		}

	else	/* Remise a jour des anciens parametres du texte */
		{
		switch( Struct->m_StructType )
			{
			case DRAW_LABEL_STRUCT_TYPE:
			case DRAW_GLOBAL_LABEL_STRUCT_TYPE:
			case DRAW_TEXT_STRUCT_TYPE:
         	{
				DrawTextStruct * Text = (DrawTextStruct *) Struct;
				Text->m_Pos = ItemInitialPosition;
				Text->m_Size = OldSize;
				Text->m_Orient = OldOrient;
            }
				break;

			default: break;
			}

		RedrawOneStruct(frame->DrawPanel, DC, Struct, GR_DEFAULT_DRAWMODE);
		Struct->m_Flags = 0;
		}

}

/*****************************************************************************/
void WinEDA_SchematicFrame::ChangeTypeText(DrawTextStruct * Text,
				wxDC * DC, int newtype)
/*****************************************************************************/

/* Routine to create new text struct (GraphicText, label or Glabel).
*/
{
	if ( Text == NULL ) return;

	m_CurrentScreen->Trace_Curseur(DrawPanel, DC);	// Erase schematic cursor
	RedrawOneStruct(DrawPanel, DC, Text, XOR_MODE);	// erase drawing
	switch( newtype )
		{
		case DRAW_LABEL_STRUCT_TYPE:
			Text->m_StructType = DRAW_LABEL_STRUCT_TYPE;
			Text->m_Layer = LAYER_LOCLABEL;
			break;

		case DRAW_GLOBAL_LABEL_STRUCT_TYPE:
			Text->m_StructType = DRAW_GLOBAL_LABEL_STRUCT_TYPE;
			Text->m_Layer = LAYER_GLOBLABEL;
			break;

		case DRAW_TEXT_STRUCT_TYPE:
			Text->m_StructType = DRAW_TEXT_STRUCT_TYPE;
			Text->m_Layer = LAYER_NOTES;
			break;

		default:
			DisplayError(this, "ChangeTypeText: Internal error");
			break;
		}

	RedrawOneStruct(DrawPanel, DC, Text, GR_DEFAULT_DRAWMODE);
	m_CurrentScreen->Trace_Curseur(DrawPanel, DC);	// redraw schematic cursor
}


