 /* Set up the basic primitives for Layer control */

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "gerbview.h"
#include "pcbnew.h"

#include "protos.h"


/* Variables locales */
int CurrentColor;

/* Fonctions locales: */

/* Macro utile : */
#define ADR(numlayer) &g_DesignSettings.m_LayerColor[(numlayer)]

#define BUTT_SIZE_X 25
#define BUTT_SIZE_Y 15


enum col_sel_id {
	ID_COLOR_RESET_SHOW_LAYER_ON = 1800,
	ID_COLOR_RESET_SHOW_LAYER_OFF,
	ID_COLOR_EXIT,
	ID_COLOR_CHECKBOX_ONOFF,
	ID_COLOR_SETUP
};

	/**********************************/
	/* Liste des menus de Menu_Layers */
	/**********************************/
struct ColorButton
	{
	wxString m_Name;
	int * m_Color;		// Pointeur sur la variable couleur
	bool m_NoDisplayIsColor;	// TRUE si bit ITEM_NON_VISIBLE de la variable Color
	bool * m_NoDisplay;	// Pointeur sur la variable Display on/off si ce n'est pas la var
						// Color
	int m_Id;
	wxBitmapButton * m_Button;
	int m_State;
	wxCheckBox * m_CheckBox;	// Option Display ON/OFF
};

#include "set_color.h"	/* include description and list of tools and buttons */


/*************************************************************/
/* classe derivee pour la frame de Configuration des couleurs*/
/*************************************************************/

class WinEDA_SetColorsFrame: public wxDialog
{
private:
	WinEDA_DrawFrame *m_Parent;

public:

	// Constructor and destructor
	WinEDA_SetColorsFrame(WinEDA_DrawFrame *parent, const wxPoint& framepos);
	~WinEDA_SetColorsFrame(void) {};

private:
	void SetColor(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	void SetDisplayOnOff(wxCommandEvent& event);
	void ResetDisplayLayersCu(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()

};
/* Table des evenements pour WinEDA_SetColorsFrame */
BEGIN_EVENT_TABLE(WinEDA_SetColorsFrame, wxDialog)
	EVT_BUTTON(ID_COLOR_RESET_SHOW_LAYER_OFF, WinEDA_SetColorsFrame::ResetDisplayLayersCu)
	EVT_BUTTON(ID_COLOR_RESET_SHOW_LAYER_ON, WinEDA_SetColorsFrame::ResetDisplayLayersCu)
	EVT_BUTTON(ID_COLOR_EXIT, WinEDA_SetColorsFrame::OnQuit)
	EVT_CHECKBOX(ID_COLOR_CHECKBOX_ONOFF, WinEDA_SetColorsFrame::SetDisplayOnOff)
	EVT_BUTTON(ID_COLOR_SETUP, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+1, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+2, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+3, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+4, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+5, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+6, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+7, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+8, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+9, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+10, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+11, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+12, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+13, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+14, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+15, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+16, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+17, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+18, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+19, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+20, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+21, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+22, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+23, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+24, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+25, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+26, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+27, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+28, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+29, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+30, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+31, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+32, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+33, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+34, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+35, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+36, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+37, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+38, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+39, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+40, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+41, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+42, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+43, WinEDA_SetColorsFrame::SetColor)
	EVT_BUTTON(ID_COLOR_SETUP+44, WinEDA_SetColorsFrame::SetColor)
END_EVENT_TABLE()

/**************************************************************/
/* void DisplayColorSetupFrame(WinEDA_DrawFrame * parent, */
/*							const wxPoint & pos)			  */
/**************************************************************/

void DisplayColorSetupFrame(WinEDA_DrawFrame * parent,
							const wxPoint & framepos)
{
	WinEDA_SetColorsFrame * frame =
			new WinEDA_SetColorsFrame(parent, framepos);
	frame->ShowModal(); frame->Destroy();
}



/**********************************************************************/
WinEDA_SetColorsFrame::WinEDA_SetColorsFrame(WinEDA_DrawFrame *parent,
							const wxPoint& framepos):
		wxDialog(parent, -1, _("Gerbview Layer Colors:"), framepos,
			wxSize(390, 380),
			wxDEFAULT_DIALOG_STYLE|wxFRAME_FLOAT_ON_PARENT )
/**********************************************************************/
{
#define START_Y 25
wxBitmapButton * ButtonB;
int ii, yy, xx, butt_ID, buttcolor;
wxPoint pos;
int w = BUTT_SIZE_X, h = BUTT_SIZE_Y;
wxString msg;
	
	m_Parent = parent;
	SetFont(*g_DialogFont);

	pos.x = 5; pos.y = START_Y;
	for ( ii = 0; laytool_list[ii] != NULL; ii++ )
	{
		if( ! laytool_list[ii]->m_Color && ! laytool_list[ii]->m_NoDisplay )
		{
			if( pos.y != START_Y )
			{
				pos.x += w + 100; pos.y = START_Y;
			}
			msg = wxGetTranslation(laytool_list[ii]->m_Name.GetData());
			new wxStaticText(this,-1, msg,
					wxPoint(pos.x + 10, pos.y - 18 ),
					wxSize(-1,-1), 0 );
			continue;
		}

		if ( laytool_list[ii]->m_Id == 0 )
			laytool_list[ii]->m_Id = ID_COLOR_SETUP + ii;
		butt_ID = laytool_list[ii]->m_Id;

		laytool_list[ii]->m_CheckBox = new wxCheckBox(this,
						ID_COLOR_CHECKBOX_ONOFF, "",
						wxPoint(pos.x, pos.y));

		if ( laytool_list[ii]->m_NoDisplayIsColor )
		{
			if ( *laytool_list[ii]->m_Color & ITEM_NOT_SHOW )
				laytool_list[ii]->m_CheckBox->SetValue(FALSE);
			else laytool_list[ii]->m_CheckBox->SetValue(TRUE);
		}

		else if ( laytool_list[ii]->m_NoDisplay )
			laytool_list[ii]->m_CheckBox->SetValue(*laytool_list[ii]->m_NoDisplay);

		xx = 3 + laytool_list[ii]->m_CheckBox->GetSize().x;

		if( laytool_list[ii]->m_Color )
		{
			wxMemoryDC iconDC;
			wxBitmap ButtBitmap(w,h);
			iconDC.SelectObject( ButtBitmap );
			buttcolor = *laytool_list[ii]->m_Color & MASKCOLOR;
			wxBrush Brush;
			iconDC.SelectObject( ButtBitmap );
			iconDC.SetPen(*wxBLACK_PEN);
			Brush.SetColour(
						ColorRefs[buttcolor].m_Red,
						ColorRefs[buttcolor].m_Green,
						ColorRefs[buttcolor].m_Blue
						);
			Brush.SetStyle(wxSOLID);

			iconDC.SetBrush(Brush);
			iconDC.DrawRectangle(0,0, w, h);

			ButtonB = new wxBitmapButton(this, butt_ID,
						ButtBitmap,
						wxPoint(pos.x + xx, pos.y),
						wxSize(w,h) );
			laytool_list[ii]->m_Button = ButtonB;
			xx += 3 + w;
		}

		msg = wxGetTranslation(laytool_list[ii]->m_Name.GetData());
		new wxStaticText(this,-1, msg,
					wxPoint(pos.x + xx , pos.y + 1 ),
					wxSize(-1,-1), 0 );

		yy = h + 5;
		pos.y += yy;
	}

	pos.y = 200;
	wxButton * Button = new wxButton(this,ID_COLOR_RESET_SHOW_LAYER_ON,
						_("Show All"), pos);
	Button->SetForegroundColour(*wxBLUE);

	pos.y += Button->GetSize().y + 2;
	Button = new wxButton(this,ID_COLOR_RESET_SHOW_LAYER_OFF,
						_("Show None"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.y += Button->GetSize().y + 20;
	Button = new wxButton(this,ID_COLOR_EXIT,
						_("Exit"), pos);
	Button->SetForegroundColour(*wxBLACK);
}

/*******************************************************************/
void  WinEDA_SetColorsFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
/*******************************************************************/
{
    // true is to force the frame to close
    Close(true);
}



/***********************************************************/
void WinEDA_SetColorsFrame::SetColor(wxCommandEvent& event)
/***********************************************************/
{
int ii;
int id = event.GetId();
int color;
int w = BUTT_SIZE_X, h = BUTT_SIZE_Y;


	color = DisplayColorFrame(this);
	if ( color < 0) return;

	for ( ii = 0; laytool_list[ii] != NULL; ii++ )
		{
		if( laytool_list[ii]->m_Id != id) continue;
		if( laytool_list[ii]->m_Color == NULL) continue;

		if( *laytool_list[ii]->m_Color == color) break;

		*laytool_list[ii]->m_Color = color;
		wxMemoryDC iconDC;

		wxBitmapButton * Button = laytool_list[ii]->m_Button;

		wxBitmap ButtBitmap = Button->GetBitmapLabel();
		iconDC.SelectObject( ButtBitmap );
		int buttcolor = *laytool_list[ii]->m_Color;
		wxBrush Brush;
		iconDC.SelectObject( ButtBitmap );
		iconDC.SetPen(*wxBLACK_PEN);
		Brush.SetColour(
						ColorRefs[buttcolor].m_Red,
						ColorRefs[buttcolor].m_Green,
						ColorRefs[buttcolor].m_Blue
						);
		Brush.SetStyle(wxSOLID);

		iconDC.SetBrush(Brush);
		iconDC.DrawRectangle(0,0, w, h);
		SetDisplayOnOff(event);
		m_Parent->GetScreen()->SetRefreshReq();
		}
	Refresh(FALSE);
}


/******************************************************************/
void WinEDA_SetColorsFrame::SetDisplayOnOff(wxCommandEvent& event)
/******************************************************************/
{
	for ( int ii = 0; laytool_list[ii] != NULL; ii++ )
		{
		if ( laytool_list[ii]->m_CheckBox == NULL ) continue;
		if ( ! laytool_list[ii]->m_NoDisplayIsColor &&
			 (laytool_list[ii]->m_NoDisplay == NULL) ) continue;

		if ( laytool_list[ii]->m_NoDisplayIsColor )
			{
			if ( laytool_list[ii]->m_CheckBox->GetValue() )
				*laytool_list[ii]->m_Color &= ~ITEM_NOT_SHOW;
			else *laytool_list[ii]->m_Color |= ITEM_NOT_SHOW;
			}

		else
			{
			*laytool_list[ii]->m_NoDisplay = laytool_list[ii]->m_CheckBox->GetValue();
			}

		}
	m_Parent->GetScreen()->SetRefreshReq();
}



/***********************************************************************/
void WinEDA_SetColorsFrame::ResetDisplayLayersCu(wxCommandEvent& event)
/***********************************************************************/
{
bool NewState = (event.GetId() == ID_COLOR_RESET_SHOW_LAYER_ON) ? TRUE : FALSE;

	for ( int ii = 1; ii < 34; ii++ )
		{
		if ( laytool_list[ii]->m_CheckBox == NULL ) continue;
		laytool_list[ii]->m_CheckBox->SetValue(NewState);
		}

	SetDisplayOnOff(event);
}

