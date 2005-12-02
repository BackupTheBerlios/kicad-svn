	/**********************************************************/
	/* wxwineda.cpp - fonctions des classes du type WinEDAxxxx */
	/**********************************************************/

#ifdef __GNUG__
#pragma implementation
#endif

#include "fctsys.h"
#include "common.h"



/**********************************************************************************/
/* Classe WinEDA_EnterText pour entrer une ligne texte au clavier dans les frames */
/**********************************************************************************/

WinEDA_EnterText::WinEDA_EnterText(wxWindow *parent, const wxString &Title,
			const wxString & TextToEdit, const wxPoint & Pos, const wxSize & Size )
{
	m_Modify = FALSE;
	if ( TextToEdit ) m_NewText = TextToEdit;

	m_Title = new wxStaticText(parent, -1, Title,
							wxPoint(Pos.x, Pos.y - 14),
							wxSize(-1,-1), 0 );
	m_Title->SetForegroundColour(wxColour(200,0,0) );

	m_FrameText = new wxTextCtrl(parent, -1, TextToEdit, Pos, Size);
	m_FrameText->SetInsertionPoint(1);
}


	/**************************************************/
	/* void WinEDA_EnterText::Getdata() */
	/**************************************************/

wxString WinEDA_EnterText::GetData(void)
{
	m_Modify = m_FrameText->IsModified();
	m_NewText = m_FrameText->GetValue();
	return m_NewText;
}

void WinEDA_EnterText::GetData(char * buffer, int lenmax)
{
	m_Modify = m_FrameText->IsModified();
	if (buffer)
		{
		m_NewText = m_FrameText->GetValue();
		strncpy(buffer, m_NewText.GetData(), lenmax-1);
		buffer[lenmax-1] = 0;
		}
}

	/* retourne la dimension de la zone occupee par la "frame" */
wxSize WinEDA_EnterText::GetDimension(void)
{
	wxSize size = m_FrameText->GetSize();
	return size;
}

void WinEDA_EnterText::SetValue(const wxString & new_text)
{
	m_FrameText->SetValue(new_text);
}

void WinEDA_EnterText::Enable(bool enbl)
{
	m_Title->Enable(enbl);
	m_FrameText->Enable(enbl);
}


/*********************************************************************/
/* Classe pour editer un texte graphique + dimension en INCHES ou MM */
/*********************************************************************/
WinEDA_GraphicTextCtrl::WinEDA_GraphicTextCtrl(wxWindow *parent, const wxString &Title,
				const wxString & TextToEdit, int textsize,
				int units, const wxPoint & pos, int framelen,bool vertical_align,
				int internal_unit)
{
wxPoint aff_pos = pos;

	m_Units = units;
	m_Internal_Unit = internal_unit;
	m_Title = NULL;
	// Limitation de la taille du texte a de valeurs raisonnables
	if ( textsize < 10 ) textsize = 10;
	if ( textsize > 3000 ) textsize = 3000;

	if ( Title )
		{
		m_Title = new wxStaticText(parent, -1, Title,
							wxPoint(aff_pos.x, aff_pos.y - 14),
							wxSize(-1,-1), 0 );
		}

	m_FrameText = new wxTextCtrl(parent, -1, TextToEdit,
								aff_pos, wxSize(framelen, -1));
char TextSize[80];

	if ( vertical_align )
		{
		aff_pos.y += m_FrameText->GetSize().y + 10;
		if ( Title ) aff_pos.y += 14;
		}
	else
		{
		aff_pos.x += m_FrameText->GetSize().x + 10;
		}

	sprintf( TextSize,  ( m_Internal_Unit > 1000 ) ? "%.4f" : "%.3f",
		To_User_Unit(m_Units, textsize, m_Internal_Unit) );
	m_FrameSize = new wxTextCtrl(parent, -1, TextSize, aff_pos ,wxSize(70,-1));
	if ( Title != "")
		{
		new wxStaticText(parent, -1,
                            m_Units ? _("Size (mm):") :  _("Size (\"):"),
							wxPoint(aff_pos.x, aff_pos.y - 14),
							wxSize(-1,-1), 0 );
		}

	if ( vertical_align ) m_WinSize.x = m_FrameText->GetSize().x;
	else m_WinSize.x = aff_pos.x - pos.x + m_FrameSize->GetSize().x;

	m_WinSize.y = aff_pos.y - pos.y + m_FrameSize->GetSize().y;
}


WinEDA_GraphicTextCtrl::~WinEDA_GraphicTextCtrl(void)
{
	delete m_FrameText;
	delete m_Title;
}

/* retourne la dimension de la zone occupee par la "frame"
*/
wxSize WinEDA_GraphicTextCtrl::GetDimension(void)
{
	return m_WinSize;
}

wxString WinEDA_GraphicTextCtrl::GetText(void)
{
wxString text = m_FrameText->GetValue();
	return text;
}

int WinEDA_GraphicTextCtrl::GetTextSize(void)
{
int textsize;

	textsize = (int)From_User_Unit( m_Units,
							atof(m_FrameSize->GetValue().GetData()),
							m_Internal_Unit);
	// Limitation de la taille du texte a de valeurs raisonnables
	if ( textsize < 10 ) textsize = 10;
	if ( textsize > 3000 ) textsize = 3000;
	return textsize;
}

void WinEDA_GraphicTextCtrl::Enable(bool state)
{
	m_FrameText->Enable(state);
}



/*****************************************************************/
/* Classe pour afficher et editer une coordonnée en INCHES ou MM */
/*****************************************************************/

WinEDA_PositionCtrl::WinEDA_PositionCtrl(wxWindow *parent, const wxString & title,
						const wxPoint & pos_to_edit, int units, const wxPoint & pos,
						int internal_unit )
{
wxPoint aff_pos = pos;
wxString text;

	m_Units = units;
	m_Internal_Unit = internal_unit;
	text = (title == "" ) ? _("Pos ") : title.GetData();
	text << (m_Units ? _("X (mm):") :  _("X (\"):"));
	m_TextX = new wxStaticText(parent, -1, text,
							wxPoint(aff_pos.x, aff_pos.y ),
							wxSize(-1,-1), 0 );
	aff_pos.y += 14;
	m_FramePosX = new wxTextCtrl(parent, -1, "", aff_pos );


	aff_pos.y += m_FramePosX->GetSize().y;
	text = (title == "" ) ? _("Pos ") : title.GetData();
	text << (m_Units ? _("Y (mm):") :  _("Y (\"):"));
	m_TextY = new wxStaticText(parent, -1, text,
							wxPoint(aff_pos.x, aff_pos.y),
							wxSize(-1,-1), 0 );

	aff_pos.y += 14;
	m_FramePosY = new wxTextCtrl(parent, -1, "", aff_pos,
								wxSize(-1,-1), 0 );
	aff_pos.y += m_FramePosY->GetSize().y;

	m_WinSize.x = m_FramePosX->GetSize().x;
	m_WinSize.y = aff_pos.y - pos.y;
	
	SetValue(pos_to_edit.x, pos_to_edit.y);
}

WinEDA_PositionCtrl::~WinEDA_PositionCtrl(void)
{
	delete m_TextX;
	delete m_TextY;
	delete m_FramePosX;
	delete m_FramePosY;
}


/*********************************************/
wxSize WinEDA_PositionCtrl::GetDimension(void)
/*********************************************/
/* retourne la dimension de la zone occupee par la "frame"
*/
{
	return m_WinSize;
}

/******************************************/
wxPoint WinEDA_PositionCtrl::GetCoord(void)
/******************************************/
/* Retourne (en unites internes) les coordonnes entrees (en unites utilisateur)
*/
{
wxPoint coord;
double value;

	value = atof(m_FramePosX->GetValue().GetData());
	coord.x = From_User_Unit(m_Units, value, m_Internal_Unit);
	value = atof(m_FramePosY->GetValue().GetData());
	coord.y = From_User_Unit(m_Units, value, m_Internal_Unit);

	return coord;
}


/*******************************************/
void WinEDA_PositionCtrl::Enable(bool onoff)
/*******************************************/
{
	m_FramePosX->Enable(onoff);
	m_FramePosY->Enable(onoff);
}

/***********************************************************/
void WinEDA_PositionCtrl::SetValue(int x_value, int y_value)
/***********************************************************/
{
wxString msg;
	
	m_Pos_To_Edit.x = x_value;
	m_Pos_To_Edit.y = y_value;
	
	msg.Printf( ( m_Internal_Unit > 1000 ) ? "%.4f" : "%.3f",
		To_User_Unit(m_Units, m_Pos_To_Edit.x,m_Internal_Unit) );
	m_FramePosX->Clear();
	m_FramePosX->SetValue(msg);
	
	msg.Printf( ( m_Internal_Unit > 1000 ) ? "%.4f" : "%.3f",
		To_User_Unit(m_Units, m_Pos_To_Edit.y,m_Internal_Unit) );
	m_FramePosY->Clear();
	m_FramePosY->SetValue(msg);
}

	/*******************/
	/* WinEDA_SizeCtrl */
	/*******************/

WinEDA_SizeCtrl::WinEDA_SizeCtrl(wxWindow *parent, const wxString & title,
						const wxSize & size_to_edit,
						int units, const wxPoint & Pos,
						int internal_unit):
				WinEDA_PositionCtrl(parent, title,
						wxPoint(size_to_edit.x, size_to_edit.y),
						units, Pos, internal_unit)
{
}


wxSize WinEDA_SizeCtrl::GetCoord(void)
{
wxPoint pos = WinEDA_PositionCtrl::GetCoord();
wxSize size;
	size.x = pos.x;
	size.y = pos.y;
	return size;
}


/***********************************************************************/
/* Classe pour afficher et editer une dimension en INCHES  MM ou autres*/
/***********************************************************************/

/* Unites:
	si units = 0 : unite = inch
	si units = 1 : unite = mm
	si units >1 : affichage direct
*/
WinEDA_ValueCtrl::WinEDA_ValueCtrl(wxWindow *parent, const wxString & title,
						int value, int units, const wxPoint & pos,
						int internal_unit )
{
wxPoint aff_pos = pos;
char buffer[256];
wxString label = title;

	m_Units = units;
	m_Internal_Unit = internal_unit;
	m_Value = value;

	switch ( m_Units )
		{
		case INCHES:
			label += _(" (\"):");
			break;

		case MILLIMETRE:
			label += _(" (mm):");
			break;

		default:
			break;
		}

	m_Text = new wxStaticText(parent, -1, label, aff_pos,
							wxSize(-1,-1), 0 );
	aff_pos.y += 14;
	if ( m_Units >= CENTIMETRE )
		{
		sprintf(buffer , "%d",  m_Value);
		}
	else
		{
		sprintf(buffer , ( m_Internal_Unit > 1000 ) ? "%.4f" : "%.3f",
			To_User_Unit(m_Units, m_Value,m_Internal_Unit) );
		}
	m_ValueCtrl = new wxTextCtrl(parent, -1, buffer, aff_pos );


	m_WinSize.x = m_ValueCtrl->GetSize().x;
	m_WinSize.y = m_ValueCtrl->GetSize().y + 14;
}

WinEDA_ValueCtrl::~WinEDA_ValueCtrl(void)
{
	delete m_ValueCtrl;
	delete m_Text;
}

/* retourne la dimension de la zone occupee par la "frame"
*/
wxSize WinEDA_ValueCtrl::GetDimension(void)
{
	return m_WinSize;
}

int WinEDA_ValueCtrl::GetValue(void)
{
int coord;

	if ( m_Units >= CENTIMETRE )
		{
		coord = (int)(atof(m_ValueCtrl->GetValue().GetData()) + 0.5);
		}
	else
		{
		coord = From_User_Unit(m_Units,
				atof(m_ValueCtrl->GetValue().GetData()),m_Internal_Unit);
		}
	return coord;
}

void WinEDA_ValueCtrl::SetValue(int new_value)
{
wxString buffer;
	m_Value = new_value;

	if ( m_Units >= CENTIMETRE )
		{
		buffer.Printf("%d",  m_Value);
		}
	else
		{
		buffer.Printf( (m_Internal_Unit > 1000 ) ? "%.4f" : "%.3f",
			To_User_Unit(m_Units, m_Value,m_Internal_Unit ) );
		}
	m_ValueCtrl->SetValue(buffer);
}

/* Active ou desactive la frame: */
void WinEDA_ValueCtrl::Enable(bool enbl)
{
	m_ValueCtrl->Enable(enbl);
	m_Text->Enable(enbl);
}

/***************************************************************/
/* Classe pour afficher et editer une valeur en double flottant*/
/***************************************************************/
WinEDA_DFloatValueCtrl::WinEDA_DFloatValueCtrl(wxWindow *parent, const wxString & title,
						double value, const wxPoint & pos )
{
wxPoint aff_pos = pos;
wxString buffer;
wxString label = title;

	m_Value = value;

	m_Text = new wxStaticText(parent, -1, label, aff_pos,
							wxSize(-1,-1), 0 );
	aff_pos.y += 14;
	buffer.Printf("%lf",  m_Value);
	m_ValueCtrl = new wxTextCtrl(parent, -1, buffer, aff_pos );

	m_WinSize.x = m_ValueCtrl->GetSize().x;
	m_WinSize.y = m_ValueCtrl->GetSize().y + 14;
}

WinEDA_DFloatValueCtrl::~WinEDA_DFloatValueCtrl(void)
{
	delete m_ValueCtrl;
	delete m_Text;
}

/* retourne la dimension de la zone occupee par la "frame"
*/
wxSize WinEDA_DFloatValueCtrl::GetDimension(void)
{
	return m_WinSize;
}

double WinEDA_DFloatValueCtrl::GetValue(void)
{
double coord;

	sscanf(m_ValueCtrl->GetValue().GetData(), "%lf", &coord);
	return coord;
}

void WinEDA_DFloatValueCtrl::SetValue(double new_value)
{
wxString buffer;
	m_Value = new_value;

	buffer.Printf("%lf",  m_Value);
	m_ValueCtrl->SetValue(buffer);
}

/* Active ou desactive la frame: */
void WinEDA_DFloatValueCtrl::Enable(bool enbl)
{
	m_ValueCtrl->Enable(enbl);
	m_Text->Enable(enbl);
}

