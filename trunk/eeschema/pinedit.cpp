	/***************************/
	/*  EESchema - PinEdit.cpp */
	/***************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"
#include "id.h"

#include "wx/spinctrl.h"

#include "protos.h"

static int CodeOrient[4] =
{
	PIN_RIGHT,
	PIN_LEFT,
	PIN_UP,
	PIN_DOWN
};

#define NBSHAPES 7
static wxString shape_list[NBSHAPES] =
{
 _("line"), _("invert"), _("clock"), _("clock inv"),
	_("low in"), _("low clock"), _("low out")
};

int CodeShape[NBSHAPES] =
{
NONE, INVERT, CLOCK, CLOCK|INVERT, LOWLEVEL_IN, LOWLEVEL_IN|CLOCK, LOWLEVEL_OUT
};


/* Routines locales */
static void CreateImagePins(LibDrawPin * Pin);
static void AbortPinMove(WinEDA_DrawFrame * frame, wxDC * DC);
static void DrawMovePin(WinEDA_DrawPanel * panel, wxDC * DC, bool erase);

/* Variables importees */

/* Variables locales */
static wxPoint OldPos, PinPreviousPos;
static int 	LastPinType = PIN_INPUT,
			LastPinOrient = PIN_RIGHT,
			LastPinShape = NONE,
			LastPinSize = 300,
			LastPinNameSize = 50,
			LastPinNumSize = 50,
			LastPinCommonConvert = FALSE,
			LastPinCommonUnit = FALSE,
			LastPinNoDraw = FALSE;

enum id_pinedit
{
	ID_ACCEPT_PIN_PROPERTIES = 1900,
	ID_CLOSE_PIN_PROPERTIES
};


	/************************************/
	/* class WinEDA_PartPropertiesFrame */
	/************************************/

class WinEDA_PinPropertiesFrame: public wxDialog
{
private:

	WinEDA_LibeditFrame * m_Parent;
	WinEDA_GraphicTextCtrl * m_PinNameCtrl;
	WinEDA_GraphicTextCtrl * m_PinNumCtrl;
	wxRadioBox * m_PinType;
	wxRadioBox * m_PinShape;
	wxRadioBox * m_PinOrient;
	wxCheckBox * m_CommonUnit;
	wxCheckBox * m_CommonConvert;
	wxCheckBox * m_NoDraw;

	wxSpinCtrl * m_PinSize;
	wxSpinCtrl * m_PinNumSize;
	wxSpinCtrl * m_PinNameSize;

public:
	// Constructor and destructor
	WinEDA_PinPropertiesFrame(WinEDA_LibeditFrame *parent, const wxPoint & pos);
	~WinEDA_PinPropertiesFrame(void)
		{
		}

private:
	void OnQuit(wxCommandEvent& event);
	void PinPropertiesAccept(wxCommandEvent& event);
	void SetPinName(const wxString & newname, int newsize);
	void SetPinNum(const wxString & newnum, int newsize);
	void NewSizePin(int newsize);
	void SetPinShape( int newshape);
	void SetPinType(int newtype);
	void SetPinOrient(int neworient);
	void SetAttributsPin(bool draw, bool unit, bool convert);

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WinEDA_PinPropertiesFrame, wxDialog)
	EVT_BUTTON(ID_ACCEPT_PIN_PROPERTIES, WinEDA_PinPropertiesFrame::PinPropertiesAccept)
	EVT_BUTTON(ID_CLOSE_PIN_PROPERTIES, WinEDA_PinPropertiesFrame::OnQuit)
END_EVENT_TABLE()


/**************************************************************************/
void InstallPineditFrame(WinEDA_LibeditFrame * parent, const wxPoint & pos)
/**************************************************************************/
{
wxPoint MousePos = parent->GetScreen()->m_Curseur;
	
	if ( (CurrentDrawItem == NULL) || (CurrentDrawItem->m_StructType == COMPONENT_PIN_DRAW_TYPE) )
		{
		WinEDA_PinPropertiesFrame * frame = new WinEDA_PinPropertiesFrame(parent, pos);
		frame->ShowModal(); frame->Destroy();
		}
	else DisplayError(parent, wxT("Error: Not a Pin!") );
	parent->GetScreen()->m_Curseur = MousePos;
	parent->DrawPanel->MouseToCursorSchema();
}


#define X_SIZE 550
#define Y_SIZE 335
WinEDA_PinPropertiesFrame::WinEDA_PinPropertiesFrame(WinEDA_LibeditFrame *parent,
				const wxPoint & framepos):
		wxDialog(parent, -1, _("Pin properties"), framepos, wxSize(X_SIZE, Y_SIZE),
				DIALOG_STYLE)
{
wxPoint pos;
int tmp, ii;
wxString number;
LibDrawPin * CurrentPin = (LibDrawPin *) CurrentDrawItem;
wxString StringPinNum;
wxButton * Button;

	m_Parent = parent;
	SetFont(*g_DialogFont);

	Centre();

	if ( CurrentPin )
	{
		CurrentPin->ReturnPinStringNum(StringPinNum);
		m_Parent->InitEditOnePin();
	}

	/* Creation des boutons de commande */
	pos.x = 300; pos.y = 10;
	Button = new wxButton(this, ID_ACCEPT_PIN_PROPERTIES,
						_("Ok"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.x += Button->GetDefaultSize().x + 10;
	Button = new wxButton(this, ID_CLOSE_PIN_PROPERTIES,
						_("Cancel"), pos);
	Button->SetForegroundColour(*wxBLUE);

	pos.x = 5; pos.y = 30;
	m_PinNameCtrl = new WinEDA_GraphicTextCtrl(this, _("Pin Name :"),
				CurrentPin ? CurrentPin->m_PinName.GetData() : NULL,
				CurrentPin ? CurrentPin->m_SizeName : LastPinNameSize,
				UnitMetric, pos, 190 ,FALSE);


	pos.y += 58;
	m_PinNumCtrl = new WinEDA_GraphicTextCtrl(this, _("Pin Num :"),
				StringPinNum,
				CurrentPin ? CurrentPin->m_SizeNum : LastPinNumSize,
				UnitMetric, pos, 190 ,FALSE);


	// Pin Options
	pos.x = 5; pos.y += 45; tmp = pos.y;
	new wxStaticBox(this, -1,_(" Pin Options :"), pos, wxSize(190, 140));

	pos.x += 5; pos.y += 20;
	wxStaticText * title = new wxStaticText(this, -1,_("Pin lenght :"), pos);
	pos.y += title->GetSize().y + 2;
	number.Printf( wxT("%d"), CurrentPin ? CurrentPin->m_PinLen : LastPinSize);
	m_PinSize = new wxSpinCtrl(this,-1,number, pos,
				wxSize(60, -1), wxSP_ARROW_KEYS | wxSP_WRAP,
				0, 2000);

	pos.y += 30;
	m_CommonUnit = new wxCheckBox(this, -1, _("Common to Units"), pos);
	if ( CurrentPin )
		{
		if ( CurrentPin->m_Unit == 0 ) m_CommonUnit->SetValue(TRUE);
		}
	else m_CommonUnit->SetValue(LastPinCommonUnit);

	pos.y += 20;
	m_CommonConvert = new wxCheckBox(this, -1, _("Common to convert"), pos);
	if ( CurrentPin )
		{
		if ( CurrentPin->m_Convert == 0 ) m_CommonConvert->SetValue(TRUE);
		}
	else m_CommonConvert->SetValue(LastPinCommonConvert);

	pos.y += 20;
	m_NoDraw = new wxCheckBox(this, -1, _("No Draw"), pos);
	if ( CurrentPin )
		{
		if ( CurrentPin->m_Attributs & PINNOTDRAW ) m_NoDraw->SetValue(TRUE);
		}
	else m_NoDraw->SetValue(LastPinNoDraw);

	// Selection de l'orientation :
	pos.x += 195; pos.y = tmp;
	wxString orient_list[4] = { _("Right"), _("Left"), _("Up"), _("Down")};
	m_PinOrient = new wxRadioBox(this, -1, _("Pin Orient:"),
				pos, wxSize(-1,-1),
				4, orient_list, 1);
	tmp = CurrentPin ? CurrentPin->m_Orient : LastPinOrient;
	switch ( tmp )
		{
		case PIN_RIGHT:
			m_PinOrient->SetSelection(0);
			break;

		case PIN_LEFT:
			m_PinOrient->SetSelection(1);
			break;

		case PIN_UP:
			m_PinOrient->SetSelection(2);
			break;

		case PIN_DOWN:
			m_PinOrient->SetSelection(3);
			break;
		}

	// Selection de la forme
	pos.x += m_PinOrient->GetSize().x + 10; pos.y = 80;
	m_PinShape = new wxRadioBox(this, -1, _("Pin Shape:"),
				pos, wxSize(-1,-1),
				NBSHAPES, shape_list, 1);

	tmp = CurrentPin ? CurrentPin->m_PinShape : LastPinShape;
	m_PinShape->SetSelection( 0 );
	for ( ii = 0; ii < NBSHAPES; ii++ )
		{
		if ( CodeShape[ii] == tmp )
			{
			m_PinShape->SetSelection( ii ); break ;
			}
		}

	// Selection du type electrique :
wxString type_list[10] =
		{ _("Input"), _("Output"), _("Bidi"), _("3 States"),
			_("Passive"),  _("Unspecified"),
			_("Power In"), _("Power Out"), _("Open coll"), _("Open emit") };
	pos.x += m_PinShape->GetSize().x + 10; pos.y = 50;
	m_PinType = new wxRadioBox(this, -1, _("Electrical Type:"),
				pos, wxSize(-1,-1),
				10, type_list, 1);
	m_PinType->SetSelection( CurrentPin ? CurrentPin->m_PinType : LastPinType);

	pos.x += m_PinType->GetSize().x + 10; pos.y = 50;
	
	SetClientSize(wxSize(pos.x, Y_SIZE) ) ; 
}


/************************************************************************/
void  WinEDA_PinPropertiesFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
/************************************************************************/
{
    // true is to force the frame to close
    Close(true);
}

/*************************************************************************/
void WinEDA_PinPropertiesFrame::PinPropertiesAccept(wxCommandEvent& event)
/*************************************************************************/
/* Met a jour les differents parametres pour le composant en cours d'édition
*/
{
	LastPinType = m_PinType->GetSelection();
	LastPinShape = CodeShape[m_PinShape->GetSelection()];
	LastPinOrient = CodeOrient[m_PinOrient->GetSelection()];
	LastPinCommonConvert = m_CommonConvert->GetValue();
	LastPinCommonUnit = m_CommonUnit->GetValue();
	LastPinNoDraw = m_NoDraw->GetValue();
	LastPinSize = m_PinSize->GetValue();
	LastPinNameSize = m_PinNameCtrl->GetTextSize();
	LastPinNumSize = m_PinNumCtrl->GetTextSize();

	if ( CurrentDrawItem )   // Set Pin Name & Num
	{
		if ( ! (CurrentDrawItem->m_Flags & IS_NEW) )	// if IS_NEW, copy for undo is done before place
			m_Parent->SaveCopyInUndoList();
		LibDrawPin * CurrentPin = (LibDrawPin *) CurrentDrawItem;
		wxClientDC dc(m_Parent->DrawPanel);
		m_Parent->DrawPanel->PrepareGraphicContext(&dc);
		DrawLibraryDrawStruct(m_Parent->DrawPanel, &dc, CurrentLibEntry,
				0,0, CurrentPin,CurrentUnit, g_XorMode);

		SetPinName(m_PinNameCtrl->GetText(), LastPinNameSize);
		SetPinNum(m_PinNumCtrl->GetText(), LastPinNumSize);
		NewSizePin(LastPinSize);
		SetPinShape(LastPinShape);
		SetPinType(LastPinType);
		SetPinOrient(LastPinOrient);
		SetAttributsPin(TRUE, TRUE, TRUE);
		DrawLibraryDrawStruct(m_Parent->DrawPanel, &dc, CurrentLibEntry,
			0,0, CurrentPin,CurrentUnit, g_XorMode);
	}

	if ( CurrentDrawItem )
		CurrentDrawItem->Display_Infos_DrawEntry(m_Parent);

	Close();
}




/*********************************************/
void WinEDA_LibeditFrame::InitEditOnePin(void)
/*********************************************/
/* Routine d'installation du menu d'edition d'une pin
*/
{
LibDrawPin * Pin;
LibDrawPin  * CurrentPin = (LibDrawPin *) CurrentDrawItem;

	if(CurrentLibEntry == NULL) return;
	if(CurrentPin == NULL) return;

	/* Marquage des pins a traiter,Si edition d'une pin non deja selectionnee */
	Pin = (LibDrawPin *) CurrentLibEntry->m_Drawings;
	for ( ; Pin != NULL; Pin = (LibDrawPin *) Pin->Pnext )
		{
		if (Pin->m_StructType != COMPONENT_PIN_DRAW_TYPE ) continue;
		if ( Pin == CurrentPin ) continue;
		if( (Pin->m_Pos.x == CurrentPin->m_Pos.x) &&
			(Pin->m_Pos.y == CurrentPin->m_Pos.y) &&
			(g_EditPinByPinIsOn == FALSE) )
			Pin->m_Flags |= IS_LINKED | IN_EDIT;
		else Pin->m_Flags = 0;
		}

	CurrentPin->Display_Infos_DrawEntry(this);
}


/*************************************************************/
static void AbortPinMove(WinEDA_DrawFrame * frame, wxDC * DC)
/*************************************************************/
/* Routine de sortie forcee de la commande de deplacement de pins.
*/
{
LibDrawPin * CurrentPin = (LibDrawPin *) CurrentDrawItem;

	if( CurrentPin  && ( CurrentPin->m_Flags & IS_NEW ) )
		DeleteOneLibraryDrawStruct(frame->DrawPanel, DC,
					CurrentLibEntry, CurrentPin, TRUE);

	/* clear edit flags */
	LibEDA_BaseStruct * item = CurrentLibEntry->m_Drawings;
	for ( ; item != NULL; item = item->Next() ) item->m_Flags = 0;

	frame->GetScreen()->ManageCurseur = NULL;
	frame->GetScreen()->ForceCloseManageCurseur = NULL;
	CurrentDrawItem = NULL;
	LibItemToRepeat = NULL;
	frame->DrawPanel->Refresh(true);
}


/********************************************/
void WinEDA_LibeditFrame::PlacePin(wxDC * DC)
/********************************************/
/* Routine de fin de deplacement de la pin selectionnee */
{
LibDrawPin * Pin;
LibDrawPin * CurrentPin = (LibDrawPin *) CurrentDrawItem;
bool ask_for_pin = TRUE;
wxPoint newpos;
	
	if( CurrentPin == NULL ) return;

	newpos.x = GetScreen()->m_Curseur.x;
	newpos.y = - GetScreen()->m_Curseur.y;
	
	Pin = (LibDrawPin *) CurrentLibEntry->m_Drawings;
	// Tst for an other pin in same new position:
	for ( ; Pin != NULL; Pin = (LibDrawPin *) Pin->Pnext)
	{
		if ( Pin->m_StructType != COMPONENT_PIN_DRAW_TYPE ) continue;
		if ( Pin == CurrentPin ) continue;
		if( newpos.x != Pin->m_Pos.x ) continue;
		if( newpos.y != Pin->m_Pos.y ) continue;
		if ( Pin->m_Flags ) continue;
		if ( ask_for_pin )
		{
			if( ! IsOK(this, _("Occupied by other pin, Continue ?")) )
				return;
			else ask_for_pin = FALSE;
		}
	}

	GetScreen()->ManageCurseur = NULL;
	GetScreen()->ForceCloseManageCurseur = NULL;
	GetScreen()->SetModify();

	CurrentPin->m_Pos = newpos;
	if( CurrentPin->m_Flags & IS_NEW )
	{
		LastPinOrient = CurrentPin->m_Orient;
		LastPinType = CurrentPin->m_PinType;
		LastPinShape = CurrentPin->m_PinShape;
		CreateImagePins(CurrentPin);
		LibItemToRepeat = CurrentPin;
	}

	/* Put linked pins in new position, and clear flags */
	Pin = (LibDrawPin *)CurrentLibEntry->m_Drawings;
	for ( ; Pin != NULL; Pin = (LibDrawPin *)Pin->Pnext)
	{
		if(Pin->m_StructType != COMPONENT_PIN_DRAW_TYPE ) continue;
		if( Pin->m_Flags == 0 ) continue;
		Pin->m_Pos = CurrentPin->m_Pos;
		Pin->m_Flags = 0;
	}
	
	DrawLibraryDrawStruct(DrawPanel, DC, CurrentLibEntry,0,0, CurrentPin,CurrentUnit,
					GR_DEFAULT_DRAWMODE);
	
	CurrentDrawItem = NULL;
};

/***********************************************************/
void WinEDA_PinPropertiesFrame::SetPinOrient(int neworient)
/***********************************************************/
/* Routine de Rotation de la pin courante*/
{
LibDrawPin * CurrentPin = (LibDrawPin *) CurrentDrawItem;
LibDrawPin * Pin, *RefPin = CurrentPin;

	if( CurrentLibEntry == NULL ) return;
	if ( RefPin == NULL ) return;

	m_Parent->GetScreen()->SetModify();

	/* Rotation */
	RefPin->m_Orient = neworient;

	Pin = (LibDrawPin *) CurrentLibEntry->m_Drawings;
	for ( ; Pin != NULL; Pin = (LibDrawPin *) Pin->Pnext)
	{
		if( Pin->m_Flags == 0 ) continue;
		Pin->m_Orient = RefPin->m_Orient;
		if(CurrentPin == NULL ) Pin->m_Flags = 0;
	}
}


/*************************************************/
void WinEDA_LibeditFrame::StartMovePin(wxDC * DC)
/*************************************************/
/* Prepare le deplacement d'une pin :
	Localise la pin pointee par le curseur, et si elle existe active
	la fonction de gestion curseur ( DrawMovePin() ).
*/
{
LibDrawPin * Pin;
LibDrawPin * CurrentPin = (LibDrawPin *) CurrentDrawItem;

	/* Marquage des pins a traiter */
	Pin = (LibDrawPin *)CurrentLibEntry->m_Drawings;
	for ( ; Pin != NULL; Pin = (LibDrawPin *)Pin->Pnext )
	{
		Pin->m_Flags = 0;
		if (Pin->m_StructType != COMPONENT_PIN_DRAW_TYPE ) continue;
		if ( Pin == CurrentPin) continue;
		if ( (Pin->m_Pos.x == CurrentPin->m_Pos.x) &&
			 (Pin->m_Pos.y == CurrentPin->m_Pos.y ) &&
			 (g_EditPinByPinIsOn == FALSE ) )
			 Pin->m_Flags |= IS_LINKED | IS_MOVED;
	}
	CurrentPin->m_Flags |= IS_LINKED | IS_MOVED;
	PinPreviousPos = OldPos = CurrentPin->m_Pos;
	CurrentPin->Display_Infos_DrawEntry(this);
	GetScreen()->ManageCurseur = DrawMovePin;
	GetScreen()->ForceCloseManageCurseur = AbortPinMove;
}


/******************************************************************************/
/* Routine de deplacement de la Pin courante selon position du curseur souris */
/* Routine normalement appelee par la routine de gestion du curseur			 */
/******************************************************************************/
static void DrawMovePin(WinEDA_DrawPanel * panel, wxDC * DC, bool erase)
{
LibDrawPin * CurrentPin = (LibDrawPin *) CurrentDrawItem;
wxPoint pinpos = CurrentPin->m_Pos;
	
	/* Erase pin in old position */
	if( erase || (CurrentPin->m_Flags & IS_NEW) )
	{
		CurrentPin->m_Pos = PinPreviousPos;
		DrawLibraryDrawStruct(panel, DC, CurrentLibEntry,0,0,
							CurrentPin,CurrentUnit, g_XorMode);
	}

	/* Redraw pin in new position */
	CurrentPin->m_Pos.x = panel->m_Parent->GetScreen()->m_Curseur.x;
	CurrentPin->m_Pos.y = - panel->m_Parent->GetScreen()->m_Curseur.y;
	DrawLibraryDrawStruct(panel, DC, CurrentLibEntry,0,0, CurrentPin,CurrentUnit, g_XorMode);
	
	PinPreviousPos = CurrentPin->m_Pos;
	/* Keep the original position for existing pin (for Undo command)
	and the current position for a new pin */
	if ( (CurrentPin->m_Flags & IS_NEW) == 0 )
		CurrentPin->m_Pos = pinpos;
}




/**********************************************************/
void WinEDA_PinPropertiesFrame::SetPinShape( int newshape)
/**********************************************************/
/* Changement de la forme de la pin courante.
	Le changement est egalement fait sur les autres pins correspondantes
	des autres unites de la seule forme convert courante
*/
{
LibDrawPin* Pin;
LibDrawPin * CurrentPin = (LibDrawPin *) CurrentDrawItem;

	if(CurrentPin)
	{
		CurrentPin->m_PinShape = newshape;
		m_Parent->GetScreen()->SetModify();
		CurrentPin->Display_Infos_DrawEntry(m_Parent);

		Pin = (LibDrawPin *) CurrentLibEntry->m_Drawings;
		for ( ; Pin != NULL; Pin = (LibDrawPin *) Pin->Pnext)
		{
			if(Pin->m_StructType != COMPONENT_PIN_DRAW_TYPE ) continue;
			if( Pin->m_Flags == 0 ) continue;
			if( Pin->m_Convert != CurrentPin->m_Convert ) continue;
			Pin->m_PinShape = newshape;
		}
	}
}


/******************************************************/
void WinEDA_PinPropertiesFrame::SetPinType(int newtype)
/******************************************************/
/* Changement du type electrique de la pin courante.
	Le changement est egalement fait sur les autres pins correspondantes
	des autres unites du boitier
*/
{
LibDrawPin* Pin;
LibDrawPin * CurrentPin = (LibDrawPin*)CurrentDrawItem;

	if(CurrentPin == NULL) return;

	CurrentPin->m_PinType = newtype;
	m_Parent->GetScreen()->SetModify();

	Pin = (LibDrawPin*)CurrentLibEntry->m_Drawings;
	for ( ; Pin != NULL; Pin = (LibDrawPin *) Pin->Pnext)
	{
		if(Pin->m_StructType != COMPONENT_PIN_DRAW_TYPE ) continue;
		if( Pin->m_Flags == 0 ) continue;
		Pin->m_PinType = newtype;
	}
}

/********************************************************************************/
void WinEDA_PinPropertiesFrame::SetPinName(const wxString & newname, int newsize)
/********************************************************************************/
/* Met a jour le nom et la taille de ce nom de la pin courante
	si newname == NULL, pas de changement de nom
	si newsize < 0 : pas de changement de taille
*/
{
LibDrawPin * Pin;
LibDrawPin * CurrentPin = (LibDrawPin *) CurrentDrawItem;
wxString buf;

	buf = newname;
	buf.Replace( wxT(" "), wxT("_"));

	if ( newsize >= 0 ) CurrentPin->m_SizeName = newsize;

	CurrentPin->m_PinName = buf;

	m_Parent->GetScreen()->SetModify();

	/* Traitement des autres pins */
	Pin = (LibDrawPin *) CurrentLibEntry->m_Drawings;
	for ( ; Pin != NULL; Pin = (LibDrawPin *) Pin->Pnext)
	{
		if (Pin->m_StructType != COMPONENT_PIN_DRAW_TYPE ) continue;
		if( (Pin->m_Flags & IS_LINKED) == 0 ) continue;
		if (newsize >= 0 ) Pin->m_SizeName = newsize;
		Pin->m_PinName = buf;
	}
}

/******************************************************************************/
void WinEDA_PinPropertiesFrame::SetPinNum(const wxString & newnum, int newsize)
/******************************************************************************/
/* Changement du numero de la pin courante.
	Le changement est egalement fait sur les autres pins correspondantes
	a la forme convertie
	Si newnum == NULL: pas de changement de numero
	Si newsize < 0 ) pase de changement de taille
*/
{
LibDrawPin * Pin;
LibDrawPin * CurrentPin = (LibDrawPin *) CurrentDrawItem;
wxString buf;

	buf = newnum;
	buf.Replace( wxT(" "), wxT("_"));

	if(CurrentPin == NULL ) return;

	CurrentPin->m_PinNum = 0;

	if ( newsize >= 0) CurrentPin->m_SizeNum = newsize;
	CurrentPin->SetPinNumFromString(buf);
	m_Parent->GetScreen()->SetModify();

	Pin = (LibDrawPin *) CurrentLibEntry->m_Drawings;
	for ( ; Pin != NULL; Pin = (LibDrawPin *) Pin->Pnext)
	{
		if (Pin->m_StructType != COMPONENT_PIN_DRAW_TYPE ) continue;
		if( (Pin->m_Flags & IS_LINKED) == 0 ) continue;
		if( Pin->m_Unit != CurrentPin->m_Unit ) continue;
		if ( newsize >= 0) Pin->m_SizeNum = newsize;
		if ( newnum ) Pin->m_PinNum = CurrentPin->m_PinNum;
	}
}


/*************************************************/
void WinEDA_LibeditFrame::DeletePin(wxDC * DC,
					EDA_LibComponentStruct* LibEntry,
					LibDrawPin* Pin)
/*************************************************/
/* Routine d'effacement de la pin pointee par la souris
	Si g_EditPinByPinIsOn == FALSE :
		toutes les pins de meme coordonnee seront effacees.
	Sinon seule la pin de l'unite en convert courante sera effacee
*/
{
LibEDA_BaseStruct * DrawItem;
int PinPosX, PinPosY;

	if(LibEntry == NULL ) return;
	if(Pin == NULL ) return;

	PinPosX = Pin->m_Pos.x;
	PinPosY = Pin->m_Pos.y;
	DeleteOneLibraryDrawStruct(DrawPanel, DC, LibEntry, Pin, TRUE);

	/* Effacement des autres pins de meme coordonnees */
	if( g_EditPinByPinIsOn == FALSE )
	{
		DrawItem = LibEntry->m_Drawings;
		for ( ; DrawItem != NULL; )
		{
			if (DrawItem->m_StructType != COMPONENT_PIN_DRAW_TYPE )
			{
				DrawItem = DrawItem->Next(); continue;
			}
			Pin = (LibDrawPin*) DrawItem;
			DrawItem = DrawItem->Next();
			if( Pin->m_Pos.x != PinPosX ) continue;
			if( Pin->m_Pos.y != PinPosY ) continue;
			DeleteOneLibraryDrawStruct(DrawPanel, DC, LibEntry, Pin, 0);
		}
	}
	GetScreen()->SetModify();
}


/*********************************************/
void WinEDA_LibeditFrame::CreatePin(wxDC * DC)
/*********************************************/
/* Creation d'une nouvelle pin */
{
LibEDA_BaseStruct * DrawItem;
LibDrawPin * CurrentPin = (LibDrawPin *) CurrentDrawItem;

	if(CurrentLibEntry == NULL ) return;
	if(CurrentPin != NULL ) return;

	/* Effacement des flags */
	DrawItem = CurrentLibEntry->m_Drawings;
	for( ;DrawItem != NULL; DrawItem = DrawItem->Next())
		DrawItem->m_Flags = 0;

	CurrentPin = new LibDrawPin();
	CurrentDrawItem = CurrentPin;
	if(CurrentPin == NULL ) return;
	CurrentPin->m_Flags = IS_NEW;
	CurrentPin->m_Unit = CurrentUnit;
	CurrentPin->m_Convert = CurrentConvert;

	/* Marquage des pins a traiter */
	if( g_EditPinByPinIsOn == FALSE ) CurrentPin->m_Flags |= IS_LINKED;

	CurrentPin->m_Pos.x = GetScreen()->m_Curseur.x;
	CurrentPin->m_Pos.y = - GetScreen()->m_Curseur.y;
	CurrentPin->m_PinLen = LastPinSize;
	CurrentPin->m_Orient = LastPinOrient;
	CurrentPin->m_PinType = LastPinType;
	CurrentPin->m_PinShape = LastPinShape;
	CurrentPin->m_SizeName = LastPinNameSize;
	CurrentPin->m_SizeNum = LastPinNumSize;
	if ( LastPinCommonConvert ) CurrentPin->m_Convert = 0;
	else CurrentPin->m_Convert = CurrentConvert;
	if ( LastPinCommonUnit ) CurrentPin->m_Unit = 0;
	else CurrentPin->m_Unit = CurrentUnit;
	if( LastPinNoDraw ) CurrentPin->m_Attributs |= PINNOTDRAW;
	else CurrentPin->m_Attributs &= ~PINNOTDRAW;

	CurrentPin->Pnext = CurrentLibEntry->m_Drawings;
	CurrentLibEntry->m_Drawings = CurrentPin;
	CurrentLibEntry->SortDrawItems();

	GetScreen()->ManageCurseur = DrawMovePin;
	GetScreen()->ForceCloseManageCurseur = AbortPinMove;

	CurrentPin->Display_Infos_DrawEntry(this);
	DrawLibraryDrawStruct(DrawPanel, DC, CurrentLibEntry,
			0,0, CurrentPin,CurrentUnit, g_XorMode);
	GetScreen()->SetModify();
}



/*********************************************************/
void WinEDA_PinPropertiesFrame::SetAttributsPin(bool draw,
			bool unit, bool convert)
/*********************************************************/
/*  si draw == TRUE
	- Ajuste le flag visible / invisible (.U.Pin.Flags bit 0 ) de la pin
	editee

	si unit == TRUE
	- Modifie l'attribut Commun / Particulier U.Pin.Unit = 0 ou Num Unite
	de la pin editee

	si convert == TRUE
	- Modifie l'attribut Commun / Particulier U.Pin.Convert = 0 ou Num Unite
	de la pin editee

*/
{
LibEDA_BaseStruct* DrawItem;
LibDrawPin * Pin, * CurrentPin = (LibDrawPin * ) CurrentDrawItem;

	if(CurrentPin == NULL) return;

	m_Parent->GetScreen()->SetModify();

	if( unit  )
	{
		if ( LastPinCommonUnit ) CurrentPin->m_Unit = 0;
		else CurrentPin->m_Unit = CurrentUnit;

		Pin = (LibDrawPin *) CurrentLibEntry->m_Drawings;

		if( CurrentPin->m_Unit == 0 )
		{
			DrawItem = CurrentLibEntry->m_Drawings;
			for ( ; DrawItem != NULL; )
			{
				Pin = (LibDrawPin *)DrawItem;
				DrawItem = DrawItem->Next();
				if( Pin->m_StructType != COMPONENT_PIN_DRAW_TYPE) continue;
				if( Pin->m_Flags == 0 ) continue;
				if( Pin == CurrentPin) continue;
				if(CurrentPin->m_Convert && (CurrentPin->m_Convert != Pin->m_Convert))
					 continue;
				if( CurrentPin->m_Pos.x != Pin->m_Pos.x ) continue;
				if( CurrentPin->m_Pos.y != Pin->m_Pos.y ) continue;
				DeleteOneLibraryDrawStruct(m_Parent->DrawPanel, NULL,
						CurrentLibEntry, Pin, 0);
			}
		}
	}	// end if unit

	if( convert )
		{
		if ( LastPinCommonConvert ) CurrentPin->m_Convert = 0;
		else CurrentPin->m_Convert = CurrentConvert;

		if( CurrentPin->m_Convert == 0 )	/* Effacement des pins redondantes */
			{
			DrawItem = CurrentLibEntry->m_Drawings;
			for ( ; DrawItem != NULL; )
				{
				Pin = (LibDrawPin *)DrawItem; DrawItem = DrawItem->Next();
				if(Pin->m_StructType != COMPONENT_PIN_DRAW_TYPE) continue;
				if( Pin->m_Flags == 0 ) continue;
				if( Pin == CurrentPin) continue;
				if( CurrentPin->m_Unit && (CurrentPin->m_Unit != Pin->m_Unit) )
					 continue;
				if( CurrentPin->m_Pos.x != Pin->m_Pos.x ) continue;
				if( CurrentPin->m_Pos.y != Pin->m_Pos.y ) continue;
				DeleteOneLibraryDrawStruct(m_Parent->DrawPanel, NULL,
						CurrentLibEntry, Pin, 0);
				}
			}
		}	// end if convert

	if( draw )
		{
		if( LastPinNoDraw ) CurrentPin->m_Attributs |= PINNOTDRAW;
		else CurrentPin->m_Attributs &= ~PINNOTDRAW;

		Pin = (LibDrawPin *)CurrentLibEntry->m_Drawings;
		for ( ; Pin != NULL; Pin = (LibDrawPin *)Pin->Pnext)
			{
			if( Pin->m_Flags == 0 ) continue;
			Pin->m_Attributs &= ~PINNOTDRAW;
			if(CurrentPin->m_Attributs & PINNOTDRAW)
				Pin->m_Attributs |= PINNOTDRAW;
			}
		}
}


/******************************************************/
void WinEDA_PinPropertiesFrame::NewSizePin(int newsize)
/******************************************************/
/* Fonction permettant la mise aux dimensions courantes:
	- longueur, dimension des textes
	de la pin courante

*/
{
LibDrawPin * RefPin, * Pin = (LibDrawPin *) CurrentDrawItem;

	if(CurrentLibEntry == NULL ) return;
	if(Pin == NULL ) return;

	m_Parent->GetScreen()->SetModify();

	Pin->m_PinLen = newsize;

	Pin->Display_Infos_DrawEntry(m_Parent);

	RefPin = Pin;

	if( g_EditPinByPinIsOn == FALSE )
	{
		Pin = (LibDrawPin *)CurrentLibEntry->m_Drawings;
		for ( ; Pin != NULL; Pin = (LibDrawPin *)Pin->Pnext )
		{
			if (Pin->m_StructType != COMPONENT_PIN_DRAW_TYPE ) continue;
			if( Pin->m_Pos.x != RefPin->m_Pos.x ) continue;
			if( Pin->m_Pos.y != RefPin->m_Pos.y ) continue;
			if( Pin->m_Convert == RefPin->m_Convert ) Pin->m_PinLen = newsize;
		}
	}
}

/********************************************/
static void CreateImagePins(LibDrawPin* Pin)
/********************************************/
/* Creation des autres pins pour les autres unites et pour convert, apres
	creation d'une pin
*/
{
int ii, CreateConv = FALSE;
LibDrawPin* NewPin;

	if( g_EditPinByPinIsOn ) return;

	if( g_AsDeMorgan && (Pin->m_Convert != 0 ) ) CreateConv = TRUE;

	/* Creation de la pin " convert " pour la part courante */
	if( CreateConv == TRUE)
		{
		NewPin = Pin->GenCopy();
		if( Pin->m_Convert > 1 ) NewPin->m_Convert = 1;
		else NewPin->m_Convert = 2;
		NewPin->Pnext = CurrentLibEntry->m_Drawings;
		CurrentLibEntry->m_Drawings = NewPin;
		}

	for ( ii = 1; ii <= CurrentLibEntry->m_UnitCount ; ii++ )
		{
		if ( ii == CurrentUnit ) continue; 	/* Deja fait */
		if( Pin->m_Unit == 0 ) continue;	/* Pin commune a toutes les unites */

		/* Creation pour la representation "normale" */
		NewPin = Pin->GenCopy();
		if( CurrentConvert != 0 ) NewPin->m_Convert = 1;
		NewPin->m_Unit = ii;
		NewPin->Pnext = CurrentLibEntry->m_Drawings;
		CurrentLibEntry->m_Drawings = NewPin;

		/* Creation pour la representation "Convert" */
		if( CreateConv == FALSE ) continue;

		NewPin = Pin->GenCopy();
		NewPin->m_Convert = 2;
		if( Pin->m_Unit != 0 ) NewPin->m_Unit = ii;
		NewPin->Pnext = CurrentLibEntry->m_Drawings;
		CurrentLibEntry->m_Drawings = NewPin;
		}
}




/*************************************************/
void WinEDA_LibeditFrame::GlobalSetPins(wxDC * DC,
				LibDrawPin * MasterPin, int id)
/*************************************************/
/*  Depending on "id":
	- Change pin text size (name or num) (range 10 .. 1000 mil)
	- Change pin lenght.

	If Pin is selected ( .m_flag == IS_SELECTED ) only the other selected pis are modified
*/
{
LibDrawPin * Pin;
bool selected = (MasterPin->m_Selected & IS_SELECTED) != 0;

	if( (CurrentLibEntry == NULL) || (MasterPin == NULL) ) return;
	if(MasterPin->m_StructType != COMPONENT_PIN_DRAW_TYPE ) return;

	GetScreen()->SetModify();

	Pin = (LibDrawPin *)CurrentLibEntry->m_Drawings;
	for( ; Pin != NULL; Pin = (LibDrawPin *) Pin->Pnext )
		{
		if ( Pin->m_StructType != COMPONENT_PIN_DRAW_TYPE ) continue;
		if ( (Pin->m_Convert) && (Pin->m_Convert != CurrentConvert) ) continue;
		// Is it the "selected mode" ?
		if (selected && (Pin->m_Selected & IS_SELECTED) == 0 ) continue;

		DrawLibraryDrawStruct(DrawPanel, DC, CurrentLibEntry,0,0, Pin, CurrentUnit, g_XorMode);

		switch ( id )
			{
			case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINNUMSIZE_ITEM:
				Pin->m_SizeNum = MasterPin->m_SizeNum;
				break;

			case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINNAMESIZE_ITEM:
				Pin->m_SizeName = MasterPin->m_SizeName;
				break;

			case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINSIZE_ITEM:
				Pin->m_PinLen = MasterPin->m_PinLen;
				break;
			}

		DrawLibraryDrawStruct(DrawPanel, DC, CurrentLibEntry,0,0, Pin, CurrentUnit,
						GR_DEFAULT_DRAWMODE);
		}
}




/************************************************************************/
void WinEDA_LibeditFrame::RepeatPinItem(wxDC * DC, LibDrawPin * SourcePin)
/************************************************************************/
/* Creation d'une nouvelle pin par copie de la précédente ( fct REPEAT) */
{
LibDrawPin * Pin;
wxString msg;
int ox = 0, oy = 0;

	if(CurrentLibEntry == NULL ) return;
	if(SourcePin == NULL ) return;
	if(SourcePin->m_StructType != COMPONENT_PIN_DRAW_TYPE ) return;

	Pin = SourcePin->GenCopy();
	Pin->Pnext = CurrentLibEntry->m_Drawings;
	CurrentLibEntry->m_Drawings = Pin;
	Pin->m_Flags = IS_NEW;

	Pin->m_Pos.x += g_RepeatStep.x; ox = Pin->m_Pos.x;
	Pin->m_Pos.y += - g_RepeatStep.y; oy = Pin->m_Pos.y; // ici axe Y comme en math
	/*** Increment du numero de label ***/
	IncrementLabelMember(Pin->m_PinName);

	Pin->ReturnPinStringNum(msg);
	IncrementLabelMember(msg);
	Pin->SetPinNumFromString(msg);

	CurrentDrawItem = Pin;

	/* Marquage des pins a traiter */
	if( g_EditPinByPinIsOn == FALSE ) Pin->m_Flags |= IS_LINKED;

	PlacePin(DC);
//	DrawPanel->MouseToCursorSchema();

	Pin->Display_Infos_DrawEntry(this);
	DrawLibraryDrawStruct(DrawPanel, DC, CurrentLibEntry,0,0, Pin,CurrentUnit, GR_DEFAULT_DRAWMODE);
	GetScreen()->SetModify();
}

int sort_by_pin_number(const void * ref, const void * tst )
{
const LibDrawPin * Ref = * (LibDrawPin **) ref;
const LibDrawPin * Tst = * (LibDrawPin **) tst;
	return ( Ref->m_PinNum - Tst->m_PinNum);
}
/***************************************************************/
bool WinEDA_LibeditFrame::TestPins(EDA_LibComponentStruct* LibEntry)
/***************************************************************/
// Test des pins ( duplicates...)
{
int nb_pins, ii, error;
LibDrawPin * Pin, **PinList;
wxString msg;

	if( CurrentLibEntry == NULL ) return FALSE;

	// Construction de la liste des pins:
	Pin = (LibDrawPin *)CurrentLibEntry->m_Drawings;
	for( nb_pins  = 0; Pin != NULL; Pin = (LibDrawPin *) Pin->Pnext )
	{
		if ( Pin->m_StructType == COMPONENT_PIN_DRAW_TYPE ) nb_pins++;
	}
	PinList = (LibDrawPin **) MyZMalloc( (nb_pins+1) * sizeof(LibDrawPin *) );
	Pin = (LibDrawPin *)CurrentLibEntry->m_Drawings;
	for( ii = 0; Pin != NULL; Pin = (LibDrawPin *) Pin->Pnext )
	{
		if ( Pin->m_StructType == COMPONENT_PIN_DRAW_TYPE ) PinList[ii++] = Pin;
	}

	// Classement des pins par numero de pin
	qsort(PinList, nb_pins, sizeof(LibDrawPin *), sort_by_pin_number);
	// Controle des duplicates:
	error = 0;
	for( ii = 1; ii < nb_pins; ii++ )
	{
		wxString aux_msg;
		LibDrawPin * curr_pin = PinList[ii];
		Pin = PinList[ii -1];
		if ( Pin->m_PinNum != curr_pin->m_PinNum ) continue;
		if ( Pin->m_Convert != curr_pin->m_Convert ) continue;
		if ( Pin->m_Unit != curr_pin->m_Unit ) continue;
		error ++;
		msg.Printf(_("Duplicate Pin %4.4s (Pin %s loc %d, %d, and Pin %s loc %d, %d)"),
				(char*) &curr_pin->m_PinNum, curr_pin->m_PinName.GetData(), curr_pin->m_Pos.x, -curr_pin->m_Pos.y,
				Pin->m_PinName.GetData(), Pin->m_Pos.x, -Pin->m_Pos.y);
		if ( CurrentLibEntry->m_UnitCount > 1 )
		{
			aux_msg.Printf( _("  Unit %d"), curr_pin->m_Unit);
			msg += aux_msg;
		}
		if ( g_AsDeMorgan )
		{
			if( curr_pin->m_Convert) msg += _("  Convert");
				else msg += _("  Normal");
		}
		DisplayError(this, msg);
	}
	
	free (PinList);
	return error ? TRUE : FALSE;
}

