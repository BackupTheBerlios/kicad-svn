	/**********************************************************/
	/* Routines d'affichage de parametres et caracteristiques */
	/**********************************************************/

		/* Fichier AFFICHE.CPP */

#include "fctsys.h"
#include "gr_basic.h"
#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"

#include "protos.h"


/***********************************************************/
void DrawSheetStruct::Display_Infos(WinEDA_DrawFrame * frame)
/************************************************************/
{
	frame->MsgPanel->EraseMsgBox();
	Affiche_1_Parametre(frame, 1,
		_("Name"), m_Field[VALUE].m_Text, CYAN);
	Affiche_1_Parametre(frame, 30, _("FileName"),
		m_Field[SHEET_FILENAME].m_Text, BROWN);
}

/***************************************************************/
void EDA_SchComponentStruct::Display_Infos(WinEDA_DrawFrame * frame)
/***************************************************************/
{
EDA_LibComponentStruct * Entry = FindLibPart(m_ChipName,"",FIND_ROOT);;
wxString msg;
	
	frame->MsgPanel->EraseMsgBox();

	Affiche_1_Parametre(frame, 1, _("Ref"),
		m_Field[REFERENCE].m_Text, DARKCYAN);

	if (Entry && Entry->m_Options == ENTRY_POWER)
		msg = _("Pwr Symb");
	else msg = _("Val");
	Affiche_1_Parametre(frame, 10, msg, m_Field[VALUE].m_Text, DARKCYAN);

	Affiche_1_Parametre(frame, 28, _("RefLib"), m_ChipName, BROWN);

	msg = FindLibName;
	Affiche_1_Parametre(frame, 40, "Lib", msg, DARKRED);

	if( Entry )
	{
		Affiche_1_Parametre(frame, 52, Entry->m_Doc, "", DARKCYAN);
		Affiche_1_Parametre(frame, 52, "", Entry->m_KeyWord, DARKGREEN);
	}
}



/*******************************************************/
void LibDrawPin::Display_Infos(WinEDA_DrawFrame * frame)
/*******************************************************/
/* Affiche en bas d'ecran les caracteristiques de la pin
*/
{
char Line[256];
wxString Text;
int ii;

	frame->MsgPanel->EraseMsgBox();

	/* Affichage du nom */
	Affiche_1_Parametre(frame, 24, _("PinName"), m_PinName, DARKCYAN);

	/* Affichage du numero */
	if(m_PinNum == 0) strcpy ( Line, "?" );
	else ReturnPinStringNum(Line);

	Affiche_1_Parametre(frame, 40, _("PinNum"), Line, DARKCYAN);

	/* Affichage du type */
	ii = m_PinType;
	Affiche_1_Parametre(frame, 48, _("PinType"), MsgPinElectricType[ii], RED);

	/* Affichage de la visiblite */
	ii = m_Attributs;
	if( ii & 1 ) Text = _("no");
	else Text = _("yes");
	Affiche_1_Parametre(frame, 58, _("Display"), Text, DARKGREEN);

	/* Affichage de la longueur */
	Text.Printf("%d", m_PinLen);
	Affiche_1_Parametre(frame, 66, _("Lengh"), Text, MAGENTA);

	/* Affichage de l'orientation */
	switch(m_Orient)
		{
		case PIN_UP: Text = _("Up"); break;
		case PIN_DOWN: Text = _("Down"); break;
		case PIN_LEFT: Text = _("Left"); break;
		case PIN_RIGHT: Text = _("Right"); break;
		default: Text = "??"; break;
		}

	Affiche_1_Parametre(frame, 72, _("Orient"), Text, MAGENTA);
}


/***********************************************************************/
void LibEDA_BaseStruct::Display_Infos_DrawEntry(WinEDA_DrawFrame * frame)
/***********************************************************************/
/* Affiche en bas d'ecran les caracteristiques de l'element
*/
{
wxString msg;

	frame->MsgPanel->EraseMsgBox();

	/* affichage du type */
	msg = "??";
	switch( m_StructType )
		{
		case COMPONENT_ARC_DRAW_TYPE: msg = "Arc"; break;
		case COMPONENT_CIRCLE_DRAW_TYPE: msg = "Circle"; break;
		case COMPONENT_GRAPHIC_TEXT_DRAW_TYPE: msg = "Text"; break;
		case COMPONENT_RECT_DRAW_TYPE: msg = "Rect"; break;
		case COMPONENT_POLYLINE_DRAW_TYPE: msg = "PolyLine"; break;
		case COMPONENT_LINE_DRAW_TYPE: msg = "Segment"; break;
		case COMPONENT_PIN_DRAW_TYPE:
			((LibDrawPin*) this)->Display_Infos(frame);
			msg = "Pin";
			break;
		}

	Affiche_1_Parametre(frame, 1, "Type", msg, CYAN);


	/* Affichage de l'appartenance */
	if( m_Unit == 0 ) msg = _("All");
	else msg.Printf("%d", m_Unit );
	Affiche_1_Parametre(frame, 10, "Unit", msg, BROWN);

	if( m_Convert == 0 ) msg = _("All" );
	else if( m_Convert == 1 ) msg = _("no");
	else if( m_Convert == 2 ) msg = _("yes");
	else msg = "?";
	Affiche_1_Parametre(frame, 16, "Convert", msg, BROWN);
}


