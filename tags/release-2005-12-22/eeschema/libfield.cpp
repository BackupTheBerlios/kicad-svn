/*********************************************************************/
/*	EESchema - edition des librairies: Edition des champs ( Fields ) */
/*********************************************************************/

	/*	Fichier libfield.cpp	*/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"

#include "protos.h"

#include "wx/spinctrl.h"

/* Routines locales */
static void ShowMoveField(WinEDA_DrawPanel * panel, wxDC *DC, bool erase);

/* Variables locales */

extern int CurrentUnit;
static wxPoint StartCursor, LastTextPosition;


/***********************************************************/
static void ExitMoveField(WinEDA_DrawFrame * frame, wxDC * DC)
/***********************************************************/
{

	frame->m_CurrentScreen->ManageCurseur = NULL;
	frame->m_CurrentScreen->ForceCloseManageCurseur = NULL;
	if(CurrentDrawItem == NULL) return;

	wxPoint curpos;
	curpos = frame->m_CurrentScreen->m_Curseur;
	frame->m_CurrentScreen->m_Curseur = StartCursor;
	ShowMoveField(frame->DrawPanel, DC, TRUE);
	frame->m_CurrentScreen->m_Curseur = curpos;
	CurrentDrawItem->m_Flags = 0;

	CurrentDrawItem = NULL;
}



/****************************************************************************/
void WinEDA_LibeditFrame::StartMoveField(wxDC * DC, LibDrawField *field)
/****************************************************************************/
/* Initialise le deplacement d'un champ ( ref ou Name) */
{

	if( (CurrentLibEntry == NULL) || ( field == NULL ) ) return;
	CurrentDrawItem = field;
	LastTextPosition = field->m_Pos;
	CurrentDrawItem->m_Flags |= IS_MOVED;
	m_CurrentScreen->ManageCurseur = ShowMoveField;
	m_CurrentScreen->ForceCloseManageCurseur = ExitMoveField;
	m_CurrentScreen->ManageCurseur(DrawPanel, DC, TRUE);
	StartCursor = m_CurrentScreen->m_Curseur;
}

/*****************************************************************/
/* Routine d'affichage du texte 'Field' en cours de deplacement. */
/*	Routine normalement attachee au curseur						*/
/*****************************************************************/
static void ShowMoveField(WinEDA_DrawPanel * panel, wxDC *DC, bool erase)
{
int color;
LibDrawField *Field = (LibDrawField *)CurrentDrawItem;

	if( (CurrentLibEntry == NULL) || (Field == NULL) ) return;

	GRSetDrawMode(DC, XOR_MODE);

	switch (Field->m_FieldId)
		{
		case VALUE:
			color = ReturnLayerColor(LAYER_VALUEPART);
			break;

		case REFERENCE:
			color = ReturnLayerColor(LAYER_REFERENCEPART);
			break;

		default:
			color = ReturnLayerColor(LAYER_FIELDS);
			break;
		}

	if( Field->m_Attributs & TEXT_NO_VISIBLE ) color = DARKGRAY;
	if( erase )
		DrawGraphicText(panel, DC,
				wxPoint(LastTextPosition.x, - LastTextPosition.y),
				color, Field->m_Text,
				Field->m_Orient ? TEXT_ORIENT_VERT : TEXT_ORIENT_HORIZ,
				Field->m_Size,
				Field->m_HJustify, Field->m_VJustify);


	LastTextPosition.x = panel->GetScreen()->m_Curseur.x;
	LastTextPosition.y = - panel->GetScreen()->m_Curseur.y;
	if ( (Field->m_Flags & IS_NEW) )
		Field->m_Pos = LastTextPosition;

	DrawGraphicText(panel, DC,
			wxPoint(LastTextPosition.x, - LastTextPosition.y),
			color, Field->m_Text,
			Field->m_Orient ? TEXT_ORIENT_VERT : TEXT_ORIENT_HORIZ,
			Field->m_Size,
			Field->m_HJustify, Field->m_VJustify);
}

/*******************************************************************/
void WinEDA_LibeditFrame::PlaceField(wxDC * DC, LibDrawField *Field)
/*******************************************************************/
{
int color;

	if(Field == NULL ) return;

	GRSetDrawMode(DC, GR_DEFAULT_DRAWMODE);

	switch (Field->m_FieldId)
		{
		case REFERENCE:
			color = ReturnLayerColor(LAYER_REFERENCEPART);
			break;

		case VALUE:
			color = ReturnLayerColor(LAYER_VALUEPART);
			break;

		default:
			color = ReturnLayerColor(LAYER_FIELDS);
			break;
		}

	if( Field->m_Attributs & TEXT_NO_VISIBLE ) color = DARKGRAY;
	Field->m_Pos.x = GetScreen()->m_Curseur.x;
	Field->m_Pos.y = - GetScreen()->m_Curseur.y;
	DrawGraphicText(DrawPanel, DC, wxPoint(Field->m_Pos.x, - Field->m_Pos.y),
					color, Field->m_Text,
					Field->m_Orient ? TEXT_ORIENT_VERT : TEXT_ORIENT_HORIZ,
					Field->m_Size,
					Field->m_HJustify, Field->m_VJustify);

	Field->m_Flags = 0;

	m_CurrentScreen->SetModify();
	m_CurrentScreen->ManageCurseur = NULL;
	m_CurrentScreen->ForceCloseManageCurseur = NULL;
	CurrentDrawItem = NULL;
}


/******************************************************************/
void WinEDA_LibeditFrame::EditField(wxDC * DC, LibDrawField *Field)
/******************************************************************/
{
wxString Text;
int color;
wxString title = wxT("Text:");

	if( Field == NULL) return;

	switch (Field->m_FieldId)
	{
		case REFERENCE:
			title = wxT("Reference:");
			color = ReturnLayerColor(LAYER_REFERENCEPART);
			break;

		case VALUE:
			title = wxT("Value:");
			color = ReturnLayerColor(LAYER_VALUEPART);
			break;

		default:
			color = ReturnLayerColor(LAYER_FIELDS);
			break;
	}

	if( Field->m_Attributs & TEXT_NO_VISIBLE ) color = DARKGRAY;

	Text = Field->m_Text;
	Get_Message(title,Text, this);
	Text.Replace( wxT(" ") , wxT("_") );

	GRSetDrawMode(DC, XOR_MODE);
	DrawGraphicText(DrawPanel, DC, wxPoint(Field->m_Pos.x, - Field->m_Pos.y),
					color, Field->m_Text,
					Field->m_Orient ? TEXT_ORIENT_VERT : TEXT_ORIENT_HORIZ,
					Field->m_Size,
					Field->m_HJustify, Field->m_VJustify);

	if( ! Text.IsEmpty() )
	{
		SaveCopyInUndoList();
		Field->m_Text = Text;
	}
	else DisplayError(this, _("No new text: no change") );

	if( Field->m_Flags == 0 ) GRSetDrawMode(DC, GR_DEFAULT_DRAWMODE);

	DrawGraphicText(DrawPanel, DC, wxPoint(Field->m_Pos.x, - Field->m_Pos.y),
					color, Field->m_Text,
					Field->m_Orient ? TEXT_ORIENT_VERT : TEXT_ORIENT_HORIZ,
					Field->m_Size,
					Field->m_HJustify, Field->m_VJustify);

	m_CurrentScreen->SetModify();

	if ( Field->m_FieldId == VALUE ) ReCreateHToolbar();
}

/********************************************************************/
void WinEDA_LibeditFrame::RotateField(wxDC * DC, LibDrawField *Field)
/********************************************************************/
/* Routine de modification de l'orientation ( Horiz ou Vert. ) du champ.
	si un champ est en cours d'edition, modif de celui ci.
	sinon Modif du champ pointe par la souris
*/
{
int color;

	if( Field == NULL) return;

	m_CurrentScreen->SetModify();
	switch (Field->m_FieldId)
		{
		case REFERENCE:
			color = ReturnLayerColor(LAYER_REFERENCEPART);
			break;

		case VALUE:
			color = ReturnLayerColor(LAYER_VALUEPART);
			break;

		default:
			color = ReturnLayerColor(LAYER_FIELDS);
			break;
		}

	if( Field->m_Attributs & TEXT_NO_VISIBLE  ) color = DARKGRAY;

	GRSetDrawMode(DC, XOR_MODE);
	DrawGraphicText(DrawPanel, DC, wxPoint(Field->m_Pos.x, - Field->m_Pos.y),
					color, Field->m_Text,
					Field->m_Orient ? TEXT_ORIENT_VERT : TEXT_ORIENT_HORIZ,
					Field->m_Size,
					Field->m_HJustify, Field->m_VJustify);

	if( Field->m_Orient) Field->m_Orient = 0;
	else Field->m_Orient = 1;

	if( Field->m_Flags == 0 ) GRSetDrawMode(DC, GR_DEFAULT_DRAWMODE);

	DrawGraphicText(DrawPanel, DC, wxPoint(Field->m_Pos.x, - Field->m_Pos.y),
					color, Field->m_Text,
					Field->m_Orient ? TEXT_ORIENT_VERT : TEXT_ORIENT_HORIZ,
					Field->m_Size,
					Field->m_HJustify, Field->m_VJustify);
}


/****************************************************************************/
LibDrawField * WinEDA_LibeditFrame::LocateField(EDA_LibComponentStruct *LibEntry)
/****************************************************************************/
/* Localise le champ (ref ou name) pointe par la souris
	 retourne:
		pointeur sur le champ (NULL= Pas de champ)
*/
{
int x0, y0, x1, y1;	/* Rectangle d'encadrement des textes a localiser */
int dx, dy;			/* Dimensions du texte */
LibDrawField *Field;
int hjustify, vjustify;

	/* Localisation du Nom */
	x0 = LibEntry->m_Name.m_Pos.x;
	y0 = - LibEntry->m_Name.m_Pos.y;
	dx = LibEntry->m_Name.m_Size.x * LibEntry->m_Name.m_Text.Len(),
	dy = LibEntry->m_Name.m_Size.y;
	hjustify = LibEntry->m_Name.m_HJustify; vjustify = LibEntry->m_Name.m_VJustify;
	if (LibEntry->m_Name.m_Orient) EXCHG(dx, dy);
	if ( hjustify == GR_TEXT_HJUSTIFY_CENTER ) x0 -= dx/2;
	else if ( hjustify == GR_TEXT_HJUSTIFY_RIGHT ) x0 -= dx;
	if ( vjustify == GR_TEXT_VJUSTIFY_CENTER ) y0 -= dy/2;
	else if ( vjustify == GR_TEXT_VJUSTIFY_BOTTOM ) y0 += dy;
	x1 = x0 + dx; y1 = y0 + dy;

	if( (m_CurrentScreen->m_Curseur.x >= x0) && ( m_CurrentScreen->m_Curseur.x <= x1) &&
		(m_CurrentScreen->m_Curseur.y >= y0) && ( m_CurrentScreen->m_Curseur.y <= y1) )
		return &LibEntry->m_Name;

	/* Localisation du Prefix */
	x0 = LibEntry->m_Prefix.m_Pos.x;
	y0 = - LibEntry->m_Prefix.m_Pos.y;
	dx = LibEntry->m_Prefix.m_Size.x *LibEntry->m_Prefix.m_Text.Len(),
	dy = LibEntry->m_Prefix.m_Size.y;
	hjustify = LibEntry->m_Prefix.m_HJustify; vjustify = LibEntry->m_Prefix.m_VJustify;
	if (LibEntry->m_Prefix.m_Orient) EXCHG(dx, dy);
	if ( hjustify == GR_TEXT_HJUSTIFY_CENTER ) x0 -= dx/2;
	else if ( hjustify == GR_TEXT_HJUSTIFY_RIGHT ) x0 -= dx;
	if ( vjustify == GR_TEXT_VJUSTIFY_CENTER ) y0 -= dy/2;
	else if ( vjustify == GR_TEXT_VJUSTIFY_BOTTOM ) y0 -= dy;
	x1 = x0 + dx; y1 = y0 + dy;

	if( (m_CurrentScreen->m_Curseur.x >= x0) && ( m_CurrentScreen->m_Curseur.x <= x1) &&
		(m_CurrentScreen->m_Curseur.y >= y0) && ( m_CurrentScreen->m_Curseur.y <= y1) )
		return &LibEntry->m_Prefix;

	/* Localisation des autres fields */
	for (Field = LibEntry->Fields; Field != NULL;
						Field = (LibDrawField*)Field->Pnext)
		{
		if ( Field->m_Text.IsEmpty() ) continue;
		x0 = Field->m_Pos.x; y0 = - Field->m_Pos.y;
		dx = Field->m_Size.x * Field->m_Text.Len(),
		dy = Field->m_Size.y;
		hjustify = Field->m_HJustify; vjustify = Field->m_VJustify;
		if (Field->m_Orient) EXCHG(dx, dy);
		if (LibEntry->m_Prefix.m_Orient) EXCHG(dx, dy);
		if ( hjustify == GR_TEXT_HJUSTIFY_CENTER ) x0 -= dx/2;
		else if ( hjustify == GR_TEXT_HJUSTIFY_RIGHT ) x0 -= dx;
		if ( vjustify == GR_TEXT_VJUSTIFY_CENTER ) y0 -= dy/2;
		else if ( vjustify == GR_TEXT_VJUSTIFY_BOTTOM ) y0 -= dy;
		x1 = x0 + dx; y1 = y0 + dy;
		if( (m_CurrentScreen->m_Curseur.x >= x0) && ( m_CurrentScreen->m_Curseur.x <= x1) &&
			(m_CurrentScreen->m_Curseur.y >= y0) && ( m_CurrentScreen->m_Curseur.y <= y1) )
			return(Field);
		}

	return NULL;
}

