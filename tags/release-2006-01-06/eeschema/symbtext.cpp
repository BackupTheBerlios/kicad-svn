	/**********************************************/
	/* EESchema - symbtext.cpp for Library Editor */
	/**********************************************/

/* Menu et routines de creation, modification, suppression de textes
	du type symbole
	(textes autres que Fields)
*/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"

#include "wx/spinctrl.h"

#include "protos.h"


/* Routines locales */

enum id_body_text_edit
{
	ID_ACCEPT_BODY_TEXT_PROPERTIES = 2000,
	ID_CLOSE_BODY_TEXT_PROPERTIES
};


	/************************************/
	/* class WinEDA_PartPropertiesFrame */
	/************************************/

class WinEDA_bodytext_PropertiesFrame: public wxDialog
{
private:

	WinEDA_LibeditFrame * m_Parent;
	wxCheckBox * m_Orient;
	wxCheckBox * m_CommonUnit;
	wxCheckBox * m_CommonConvert;

	WinEDA_EnterText * NewText;
	wxSpinCtrl * m_Size;

public:
	// Constructor and destructor
	WinEDA_bodytext_PropertiesFrame(WinEDA_LibeditFrame *parent,
								const wxPoint & pos);
	~WinEDA_bodytext_PropertiesFrame(void){};

private:
	void bodytext_PropertiesAccept(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WinEDA_bodytext_PropertiesFrame, wxDialog)
	EVT_BUTTON(ID_ACCEPT_BODY_TEXT_PROPERTIES,
				WinEDA_bodytext_PropertiesFrame::bodytext_PropertiesAccept)
	EVT_BUTTON(ID_CLOSE_BODY_TEXT_PROPERTIES,
			WinEDA_bodytext_PropertiesFrame::OnQuit)
END_EVENT_TABLE()


/********************************************************************/
WinEDA_bodytext_PropertiesFrame::WinEDA_bodytext_PropertiesFrame(
				WinEDA_LibeditFrame *parent, const wxPoint & framepos):
		wxDialog(parent, -1, _("Graphic text properties"),
				framepos, wxSize(400, 270), DIALOG_STYLE )
/********************************************************************/
{
wxPoint pos;
int tmp;
wxString number;
LibDrawText * CurrentText = (LibDrawText *) CurrentDrawItem;
wxButton * Button;
	
	m_Parent = parent;
	SetFont(*g_DialogFont);

	Centre();

	/* Creation des boutons de commande */
	pos.x = 300; pos.y = 10;
	Button = new wxButton(this, ID_ACCEPT_BODY_TEXT_PROPERTIES,
						_("Ok"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.y += Button->GetDefaultSize().y + 10;
	Button = new wxButton(this, ID_CLOSE_BODY_TEXT_PROPERTIES,
						_("Cancel"), pos);
	Button->SetForegroundColour(*wxBLUE);

	pos.x = 5; pos.y = 15;
	new wxStaticBox(this, -1,_(" Text : "), pos, wxSize(250, 60));

	pos.x = 10; pos.y += 26;
	NewText = new WinEDA_EnterText(this, _("Name:"),
						CurrentText ? CurrentText->m_Text.GetData() : wxEmptyString,
				pos, wxSize(150,-1) );
	pos.x = 170;
	number.Printf( wxT("%d"),
				CurrentText ? CurrentText->m_Size.x : g_LastTextSize);
	m_Size = new wxSpinCtrl(this,-1,number, pos,
				wxSize(60, -1), wxSP_ARROW_KEYS | wxSP_WRAP,
				0, 300);


	// Text Options
	pos.x = 5; pos.y += 38; tmp = pos.y;
	new wxStaticBox(this, -1,_(" Text Options : "), pos, wxSize(160, 120));

	pos.x += 5; pos.y += 30;
	m_CommonUnit = new wxCheckBox(this, -1, _("Common to Units"), pos);
	if ( CurrentText )
	{
		if ( CurrentText->m_Unit == 0 ) m_CommonUnit->SetValue(TRUE);
	}
	else if ( ! g_FlDrawSpecificUnit ) m_CommonUnit->SetValue(TRUE);

	pos.y += 20;
	m_CommonConvert = new wxCheckBox(this, -1, _("Common to convert"), pos);
	if ( CurrentText )
		{
		if ( CurrentText->m_Convert == 0 ) m_CommonConvert->SetValue(TRUE);
		}
	else if ( !g_FlDrawSpecificConvert ) m_CommonConvert->SetValue(TRUE);

	// Selection de l'orientation :
	pos.y += 20;
	m_Orient = new wxCheckBox(this, -1, _("Vertical"), pos);
	if ( CurrentText )
	{
		if ( CurrentText->m_Horiz == TEXT_ORIENT_VERT ) m_Orient->SetValue(TRUE);
	}
	else if (g_LastTextOrient == TEXT_ORIENT_VERT ) m_Orient->SetValue(TRUE);
}


/************************************************************************/
void  WinEDA_bodytext_PropertiesFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
/************************************************************************/
{
    // true is to force the frame to close
    Close(true);
}

/***************************************************************************/
void WinEDA_bodytext_PropertiesFrame::bodytext_PropertiesAccept(wxCommandEvent& event)
/***************************************************************************/
/* Met a jour les differents parametres pour le composant en cours d'édition
*/
{
LibDrawText* Text = (LibDrawText*) CurrentDrawItem;
wxString Line;

	Line = NewText->GetData();
	g_LastTextOrient = m_Orient->GetValue() ? TEXT_ORIENT_VERT : TEXT_ORIENT_HORIZ;
	g_LastTextSize = m_Size->GetValue();
	g_FlDrawSpecificConvert = m_CommonConvert->GetValue() ? FALSE : TRUE;
	g_FlDrawSpecificUnit = m_CommonUnit->GetValue() ? FALSE : TRUE;

	if ( Text )   // Set Pin Name & Num
	{
		if ( ! Line.IsEmpty() ) Text->m_Text = Line;
		else Text->m_Text = wxT("[null]");	// **** A REVOIR ***
		Text->m_Size.x = Text->m_Size.y = g_LastTextSize;
		Text->m_Horiz = g_LastTextOrient;
		if( g_FlDrawSpecificUnit ) Text->m_Unit = CurrentUnit;
		else Text->m_Unit = 0;
		if( g_FlDrawSpecificConvert ) Text->m_Convert = CurrentConvert;
		else Text->m_Convert = 0;
	}
	Close();

	if ( CurrentDrawItem )
		CurrentDrawItem->Display_Infos_DrawEntry(m_Parent);
}



/*******************************************************/
void WinEDA_LibeditFrame::EditSymbolText(wxDC * DC,
				LibEDA_BaseStruct * DrawItem)
/*******************************************************/
{
int DrawMode = g_XorMode;

	if ( DrawItem == NULL ) return;
	if ( DrawItem->m_StructType != COMPONENT_GRAPHIC_TEXT_DRAW_TYPE ) return;

	/* Effacement ancien texte */
	if( ((LibDrawText*)DrawItem)->m_Text && DC)
		DrawLibraryDrawStruct(DrawPanel, DC, CurrentLibEntry, 0 , 0,
			DrawItem, CurrentUnit, DrawMode);

	DrawPanel->m_IgnoreMouseEvents = TRUE;

	WinEDA_bodytext_PropertiesFrame * frame =
			new WinEDA_bodytext_PropertiesFrame(this, wxPoint(-1,-1));
	frame->ShowModal(); frame->Destroy();

	DrawPanel->MouseToCursorSchema();
	DrawPanel->m_IgnoreMouseEvents = FALSE;
	
	m_CurrentScreen->SetModify();

	/* Affichage nouveau texte */
	if( ((LibDrawText*)DrawItem)->m_Text && DC)
	{
		if ( (DrawItem->m_Flags & IS_MOVED) == 0 )
			DrawMode = GR_DEFAULT_DRAWMODE;
		DrawLibraryDrawStruct(DrawPanel, DC, CurrentLibEntry, 0 , 0,
				DrawItem, CurrentUnit, DrawMode);
	}
}


/****************************************************/
void WinEDA_LibeditFrame::RotateSymbolText(wxDC * DC)
/****************************************************/
/*
	90 deg Graphic text Rotation .
*/
{
LibDrawText * DrawItem = (LibDrawText *) CurrentDrawItem;

	if(DrawItem == NULL) return;

	/* Erase drawing (can be within a move command) */
	if ( m_CurrentScreen->ManageCurseur == NULL)
		DrawLibraryDrawStruct(DrawPanel, DC, CurrentLibEntry, 0 , 0,
				DrawItem, CurrentUnit, g_XorMode);
	else m_CurrentScreen->ManageCurseur(DrawPanel, DC, FALSE);

	if( DrawItem->m_Horiz == TEXT_ORIENT_HORIZ)
			DrawItem->m_Horiz = TEXT_ORIENT_VERT;
	else DrawItem->m_Horiz = TEXT_ORIENT_HORIZ;

	m_CurrentScreen->SetModify();

	/* Redraw item with new orient */
	if ( m_CurrentScreen->ManageCurseur == NULL)
		DrawLibraryDrawStruct(DrawPanel, DC, CurrentLibEntry, 0 , 0,
				DrawItem, CurrentUnit, GR_DEFAULT_DRAWMODE);
	else m_CurrentScreen->ManageCurseur(DrawPanel, DC, FALSE);

}


