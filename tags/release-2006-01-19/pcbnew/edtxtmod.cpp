	/*************************************************************/
	/* Edition des Modules: Routines de modification des textes	 */
	/*			 sur les MODULES								  */
	/*************************************************************/

			/* Fichier EDTXTMOD.CPP */

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "pcbnew.h"
#include "trigo.h"

#include "protos.h"


/* Routines Locales */
static void Show_MoveTexte_Module(WinEDA_DrawPanel * panel, wxDC * DC, bool erase);
static void ExitTextModule(WinEDA_DrawFrame * frame, wxDC *DC);

// variables locales
enum id_TextMod_properties
{
	ID_ACCEPT_TEXTE_MODULE_PROPERTIES = 1900,
	ID_CLOSE_TEXTE_MODULE_PROPERTIES,
};

static wxPoint MoveVector;	// Move vector for move edge
static wxPoint CursorInitialPosition;	// Mouse cursor inital position for move command

	/************************************/
	/* class WinEDA_TextModPropertiesFrame */
	/************************************/

class WinEDA_TextModPropertiesFrame: public wxDialog
{
private:

	WinEDA_BasePcbFrame * m_Parent;
	wxDC * m_DC;
	TEXTE_MODULE * CurrentTextMod;
	WinEDA_EnterText * m_Name;
	WinEDA_PositionCtrl * m_TxtPosCtrl;
	WinEDA_SizeCtrl * m_TxtSizeCtrl;
	WinEDA_ValueCtrl * m_TxtWidthCtlr;
	wxRadioBox * m_Show;
	wxRadioBox * m_Orient;

public:
	// Constructor and destructor
	WinEDA_TextModPropertiesFrame(WinEDA_BasePcbFrame *parent,
							TEXTE_MODULE * TextMod, wxDC * DC, const wxPoint & pos);
	~WinEDA_TextModPropertiesFrame(void)
		{
		}

private:
	void TextModPropertiesAccept(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WinEDA_TextModPropertiesFrame, wxDialog)
	EVT_BUTTON(ID_ACCEPT_TEXTE_MODULE_PROPERTIES,
			WinEDA_TextModPropertiesFrame::TextModPropertiesAccept)
	EVT_BUTTON(ID_CLOSE_TEXTE_MODULE_PROPERTIES,
			WinEDA_TextModPropertiesFrame::OnQuit)
END_EVENT_TABLE()


/***************************************************************************/
void WinEDA_BasePcbFrame::InstallTextModOptionsFrame(TEXTE_MODULE * TextMod,
					wxDC * DC, const wxPoint & pos)
/***************************************************************************/
{
	DrawPanel->m_IgnoreMouseEvents = TRUE;
	WinEDA_TextModPropertiesFrame * frame = new WinEDA_TextModPropertiesFrame(this,
					 TextMod, DC, pos);
	frame->ShowModal(); frame->Destroy();
	DrawPanel->m_IgnoreMouseEvents = FALSE;
}

/***************************************************************************************/
WinEDA_TextModPropertiesFrame::WinEDA_TextModPropertiesFrame(WinEDA_BasePcbFrame *parent,
				TEXTE_MODULE * TextMod,wxDC * DC,
				const wxPoint & framepos):
		wxDialog(parent, -1, _("TextMod properties"), framepos, wxSize(380, 330),
				DIALOG_STYLE)
/***************************************************************************************/
{
wxPoint pos;
int xx, yy;
wxString txt_title, Line;
wxButton * Button;
MODULE * Module = NULL;
#define VPOS0 40

	m_Parent = parent;
	SetFont(*g_DialogFont);
	m_DC = DC;
	Centre();

	CurrentTextMod = TextMod;
	if ( CurrentTextMod )
		{
		Module = (MODULE*)CurrentTextMod->m_Parent;
		}

	pos.x = 20; pos.y = 5;
	if ( Module )
		{
		Line.Printf( _("Module %s (%s) orient %.1f"),
			Module->m_Reference->m_Text.GetData(),
			Module->m_Value->m_Text.GetData(),
			(float)(Module->m_Orient/10) );

		wxStaticText * ModText = new wxStaticText(this, -1, Line, pos );
		ModText->SetForegroundColour(*wxBLUE);
		}

	/* Creation des boutons de commande */
	pos.x = 250; pos.y = VPOS0;
	Button = new wxButton(this, ID_ACCEPT_TEXTE_MODULE_PROPERTIES,
						_("Ok"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.y += Button->GetDefaultSize().y + 10;
	Button = new wxButton(this, ID_CLOSE_TEXTE_MODULE_PROPERTIES,
						_("Cancel"), pos);
	Button->SetForegroundColour(*wxBLUE);

	if(TextMod->m_Type == TEXT_is_REFERENCE) txt_title = _("Reference:");
	else if(TextMod->m_Type == TEXT_is_VALUE) txt_title = _("Value:");
	else if(TextMod->m_Type == TEXT_is_DIVERS) txt_title = _("Text:");
	else txt_title = wxT("???");

	pos.x = 5; pos.y = VPOS0 + 10;
	m_Name = new WinEDA_EnterText(this, txt_title,
			TextMod->m_Text,
			pos, wxSize( 200, -1) );

	pos.y += 25 + m_Name->GetDimension().y;
	m_TxtSizeCtrl = new WinEDA_SizeCtrl(this, _("Size"),
			TextMod->m_Size,
			UnitMetric, pos, m_Parent->m_InternalUnits);

	pos.x += 15 + m_TxtSizeCtrl->GetDimension().x;
	m_TxtWidthCtlr = new WinEDA_ValueCtrl(this, _("Width"),
			TextMod->m_Width,
			UnitMetric, pos, m_Parent->m_InternalUnits);

	pos.x = 5; pos.y += 25 + m_TxtSizeCtrl->GetDimension().y;
	m_TxtPosCtrl = new WinEDA_PositionCtrl(this, _("Offset"),
			TextMod->m_Pos0,
			UnitMetric, pos, m_Parent->m_InternalUnits);

	pos.x = 250; pos.y = VPOS0 + 80;
wxString orient_msg[2] = { _("horiz"), _("vertical") };
	m_Orient = new wxRadioBox(this, -1, _("Orientation"),
							pos, wxSize(-1,-1), 2, orient_msg,
							1, wxRA_SPECIFY_COLS );
	if ( (TextMod->m_Orient != 0) &&
		(TextMod->m_Orient != 1800) &&
		(TextMod->m_Orient != -1800) )
		m_Orient->SetSelection(1);;

	m_Orient->GetSize(&xx, &yy);
	pos.y += 15 + yy;
wxString display_msg[2] = { _("show"), _("no show") };
	m_Show = new wxRadioBox(this, -1, _("Display"),
							pos, wxSize(-1,-1), 2, display_msg,
							1, wxRA_SPECIFY_COLS );
	if ( TextMod->m_NoShow ) m_Show->SetSelection(1);;
}

/***************************************************************************/
void  WinEDA_TextModPropertiesFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
/***************************************************************************/
{
    Close(true);    // true is to force the frame to close
}



/*********************************************************************************/
void WinEDA_TextModPropertiesFrame::TextModPropertiesAccept(wxCommandEvent& event)
/*********************************************************************************/
{
	m_Parent->SaveCopyInUndoList();
	if ( m_DC )		// Effacement ancien texte
	{
		CurrentTextMod->Draw(m_Parent->DrawPanel, m_DC,
			(CurrentTextMod->m_Flags & IS_MOVED) ? MoveVector : wxPoint(0,0), GR_XOR );
	}
	CurrentTextMod->m_Text = m_Name->GetData();
	CurrentTextMod->m_Pos0 = m_TxtPosCtrl->GetCoord();
	CurrentTextMod->m_Size = m_TxtSizeCtrl->GetCoord();
	CurrentTextMod->SetWidth( m_TxtWidthCtlr->GetValue() );
	CurrentTextMod->m_NoShow = (m_Show->GetSelection() == 0) ? 0 : 1;
	CurrentTextMod->m_Orient = (m_Orient->GetSelection() == 0) ? 0 : 900;
	CurrentTextMod->SetDrawCoord();
	if ( m_DC )		// Affichage nouveau texte
		{
		CurrentTextMod->Draw(m_Parent->DrawPanel, m_DC,
			(CurrentTextMod->m_Flags & IS_MOVED) ? MoveVector : wxPoint(0,0), GR_XOR );
		}
	m_Parent->GetScreen()->SetModify();
	((MODULE*)CurrentTextMod->m_Parent)->m_LastEdit_Time = time(NULL);
	Close(TRUE);
}


/******************************************************************************/
TEXTE_MODULE * WinEDA_BasePcbFrame::CreateTextModule(MODULE * Module, wxDC * DC)
/******************************************************************************/
/* Add a new graphical text to the active module (footprint)
	Note there always are 2 texts: reference and value.
	New texts have the member TEXTE_MODULE.m_Type set to TEXT_is_DIVERS
*/
{
TEXTE_MODULE * Text;

	Text = new TEXTE_MODULE(Module);

	/* Chainage de la nouvelle structure en tete de liste drawings */
	Text->Pnext = Module->m_Drawings;
	Text->Pback = Module;

	if( Module->m_Drawings )
		Module->m_Drawings->Pback = Text;
	Module->m_Drawings = Text;
	Text->m_Flags = IS_NEW;

	Text->m_Text = wxT("text");

	Text->m_Size = ModuleTextSize;
	Text->m_Width = ModuleTextWidth;
	Text->m_Pos = GetScreen()->m_Curseur;
	Text->SetLocalCoord();

	InstallTextModOptionsFrame(Text, NULL, wxPoint(-1,-1) );

	Text->m_Flags = 0;
	Text->Draw(DrawPanel, DC, wxPoint(0,0), GR_OR );

	Affiche_Infos_E_Texte(this, Module, Text);

	return Text;
}

/**************************************************************************/
void WinEDA_BasePcbFrame::RotateTextModule(TEXTE_MODULE * Text, wxDC * DC)
/**************************************************************************/
/* Rotation de 90 du texte d'un module */
{
MODULE * Module;

	if ( Text == NULL ) return;

	Module = (MODULE*)Text->m_Parent;

	Text->Draw(DrawPanel, DC, wxPoint(0,0), GR_XOR );

	Text->m_Orient += 900 ;
	while (Text->m_Orient >= 1800) Text->m_Orient -= 1800 ;

	/* Redessin du Texte */
	Text->Draw(DrawPanel, DC, wxPoint(0,0), GR_XOR );

	Affiche_Infos_E_Texte(this, Module,Text);

	((MODULE*)Text->m_Parent)->m_LastEdit_Time = time(NULL);
	GetScreen()->SetModify();
}

/**************************************************************************/
void WinEDA_BasePcbFrame::DeleteTextModule(TEXTE_MODULE * Text, wxDC * DC)
/**************************************************************************/
/*
 Supprime 1 texte sur module (si ce n'est pas la référence ou la valeur)
*/
{
MODULE * Module;

	if (Text == NULL) return;

	Module = (MODULE*)Text->m_Parent;

	if(Text->m_Type == TEXT_is_DIVERS)
	{
		Text->Draw(DrawPanel, DC, wxPoint(0,0), GR_XOR );

		/* liberation de la memoire : */
		DeleteStructure(Text);
		GetScreen()->SetModify();
		Module->m_LastEdit_Time = time(NULL);
	}
}



/*************************************************************/
static void ExitTextModule(WinEDA_DrawFrame * frame, wxDC *DC)
/*************************************************************/
/*
 Routine de sortie du menu edit texte module
Si un texte est selectionne, ses coord initiales sont regenerees
*/
{
BASE_SCREEN * screen = frame->GetScreen();
TEXTE_MODULE * Text = (TEXTE_MODULE *) screen->m_CurrentItem;
MODULE * Module;

	screen->ManageCurseur = NULL;
	screen->ForceCloseManageCurseur = NULL;

	if ( Text == NULL ) return;

	Module = ( MODULE *) Text->m_Parent;
	Text->Draw(frame->DrawPanel, DC, MoveVector, GR_XOR );

	/* Redessin du Texte */
	Text->Draw(frame->DrawPanel, DC, wxPoint(0,0), GR_OR );

	Text->m_Flags = 0;
	Module->m_Flags = 0;

	screen->m_CurrentItem = NULL;
}

/****************************************************************************/
void WinEDA_BasePcbFrame::StartMoveTexteModule(TEXTE_MODULE * Text, wxDC * DC)
/****************************************************************************/
/* Routine d'initialisation du deplacement d'un texte sur module
*/
{
MODULE * Module;

	if( Text == NULL ) return;

	Module = (MODULE*) Text->m_Parent;

	Text->m_Flags |= IS_MOVED;
	Module->m_Flags |= IN_EDIT;

	MoveVector.x = MoveVector.y = 0;
	CursorInitialPosition = Text->m_Pos;

	Affiche_Infos_E_Texte(this, Module, Text);

	GetScreen()->m_CurrentItem = Text;
	GetScreen()->ManageCurseur = Show_MoveTexte_Module;
	GetScreen()->ForceCloseManageCurseur = ExitTextModule;
	
	GetScreen()->ManageCurseur(DrawPanel, DC, TRUE);
}


/*************************************************************************/
void WinEDA_BasePcbFrame::PlaceTexteModule(TEXTE_MODULE * Text, wxDC * DC)
/*************************************************************************/
/* Routine complementaire a StartMoveTexteModule().
	Place le texte en cours de deplacement ou nouvellement cree
*/
{

	if (Text != NULL )
		{
		Text->m_Pos = GetScreen()->m_Curseur;
		/* mise a jour des coordonnées relatives a l'ancre */
		MODULE * Module = ( MODULE *) Text->m_Parent;
		if (Module )
			{
			int px = Text->m_Pos.x - Module->m_Pos.x;
			int py = Text->m_Pos.y - Module->m_Pos.y;
			RotatePoint( &px, &py, - Module->m_Orient);
			Text->m_Pos0.x = px;
			Text->m_Pos0.y = py;
			Text->m_Flags = 0;
			Module->m_Flags = 0;
			Module->m_LastEdit_Time = time(NULL);
			GetScreen()->SetModify();

			/* Redessin du Texte */
			Text->Draw(DrawPanel, DC, wxPoint(0,0), GR_OR );
			}
		}

	GetScreen()->ManageCurseur = NULL;
	GetScreen()->ForceCloseManageCurseur = NULL;
}


/********************************************************************************/
static void Show_MoveTexte_Module(WinEDA_DrawPanel * panel, wxDC * DC, bool erase)
/********************************************************************************/
{
BASE_SCREEN * screen = panel->GetScreen();
TEXTE_MODULE * Text = (TEXTE_MODULE *) screen->m_CurrentItem;
MODULE * Module;

	if (Text == NULL ) return ;

	Module = ( MODULE *) Text->m_Parent;
	/* effacement du texte : */

	if ( erase )
		Text->Draw(panel, DC, MoveVector, GR_XOR );

	MoveVector.x = -(screen->m_Curseur.x - CursorInitialPosition.x);
	MoveVector.y = -(screen->m_Curseur.y - CursorInitialPosition.y);

	/* Redessin du Texte */
	Text->Draw(panel, DC, MoveVector, GR_XOR );
}

