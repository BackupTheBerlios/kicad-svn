/**************************************************************/
/*	librairy editor: edition of component general properties  */
/**************************************************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"

#include "protos.h"

enum id_libedit {
	ID_PANEL_ALIAS,
	ID_PANEL_FIELD,
	ID_COPY_DOC_TO_ALIAS,
	ID_BROWSE_DOC_FILES,
	ID_ADD_ALIAS,
	ID_DELETE_ONE_ALIAS,
	ID_DELETE_ALL_ALIAS,
	ID_ON_SELECT_FIELD
};



/* Routines locales */

/* Variables locales */

extern int CurrentUnit;

/* Classe de la frame des propri�t�s d'un composant en librairie */

/* Cette classe genere une fenetre type NoteBook, pour l'edition des propri�t�s
d'un composant le librairie.
	On peut �diter:
	Texte dimensions et justification de tous les champs (Ref, Val, et autres champs)
	Documentation et mots clefs
	Nombre de part par boitier
	et autres propri�r�s g�n�rales
*/

#include "dialog_edit_component_in_lib.cpp"


/*****************************************************************/
void WinEDA_LibeditFrame::InstallLibeditFrame(const wxPoint & pos)
/*****************************************************************/
{
wxPoint fpos = pos;

	WinEDA_PartPropertiesFrame * frame =
			new WinEDA_PartPropertiesFrame(this);
	int Changed = frame->ShowModal(); frame->Destroy();
	if ( Changed ) Refresh();

}


/*****************************************************/
void WinEDA_PartPropertiesFrame::InitBuffers(void)
/*****************************************************/
/* Init the buffers to a default value,
	or to values from CurrentLibEntry if CurrentLibEntry != NULL
*/
{
int ii;
	
	m_CurrentFieldId = REFERENCE;
	
	for ( ii = 0; ii < NUMBER_OF_FIELDS; ii++ )
	{
		m_FieldFlags[ii] = 1;
		m_FieldOrient[ii] = 0;
		m_FieldSize[ii] = DEFAULT_TEXT_SIZE;
		m_FieldHJustify[ii] = 1;
		m_FieldVJustify[ii] = 1;
	}

	m_AliasLocation = -1;
	if ( CurrentLibEntry == NULL )
	{
		m_Title = _("Lib Component Properties");
		return;
	}

	wxString msg_text = _("Properties for ");
	if ( ! CurrentAliasName.IsEmpty() )
	{
		m_AliasLocation = LocateAlias( CurrentLibEntry->m_AliasList, CurrentAliasName);
		m_Title = msg_text + CurrentAliasName +
					_("(alias of ") +
					wxString(CurrentLibEntry->m_Name.m_Text)
					+ wxT(")");
	}
	else
	{
		m_Title = msg_text + CurrentLibEntry->m_Name.m_Text;
		CurrentAliasName.Empty();
	}

	m_FieldFlags[REFERENCE] =
		(CurrentLibEntry->m_Prefix.m_Attributs & TEXT_NO_VISIBLE) ? 0 : 1;
	m_FieldOrient[REFERENCE] = CurrentLibEntry->m_Prefix.m_Orient;
	m_FieldText[REFERENCE] = CurrentLibEntry->m_Prefix.m_Text;
	m_FieldPosition[REFERENCE] = CurrentLibEntry->m_Prefix.m_Pos;
	m_FieldSize[REFERENCE] = CurrentLibEntry->m_Prefix.m_Size.x;
	
	m_FieldFlags[VALUE] =
		(CurrentLibEntry->m_Name.m_Attributs & TEXT_NO_VISIBLE) ? 0 : 1;
	m_FieldOrient[VALUE] = CurrentLibEntry->m_Name.m_Orient;
	m_FieldText[VALUE] = CurrentLibEntry->m_Name.m_Text;
	m_FieldPosition[VALUE] = CurrentLibEntry->m_Name.m_Pos;
	m_FieldSize[VALUE] = CurrentLibEntry->m_Name.m_Size.x;

	LibDrawField * Field = CurrentLibEntry->Fields;
	while ( Field )
	{
		int id = Field->m_FieldId;
		m_FieldFlags[id] = (Field->m_Attributs & TEXT_NO_VISIBLE) ? 0 : 1;
		m_FieldOrient[id] = Field->m_Orient;
		if ( Field->m_HJustify == GR_TEXT_HJUSTIFY_LEFT)
			m_FieldHJustify[id] = 0;
		else if ( Field->m_HJustify == GR_TEXT_HJUSTIFY_RIGHT)
			m_FieldHJustify[id] = 2;
		else 
			m_FieldHJustify[id] = 1;
		if ( Field->m_VJustify == GR_TEXT_VJUSTIFY_BOTTOM)
			m_FieldVJustify[id] = 0;
		else if ( Field->m_VJustify == GR_TEXT_VJUSTIFY_TOP)
			m_FieldVJustify[id] = 2;
		else m_FieldVJustify[id] = 1;
		m_FieldText[id] = Field->m_Text;
		m_FieldPosition[id] = Field->m_Pos;
		m_FieldSize[id] = Field->m_Size.x;
		Field = (LibDrawField*)Field->Pnext;
	}
}


/*****************************************************/
void WinEDA_PartPropertiesFrame::BuildPanelAlias(void)
/*****************************************************/
/* create the panel for component alias list editing
*/
{
wxButton * Button;

	m_PanelAlias->SetFont(*g_DialogFont);
    wxBoxSizer * PanelAliasBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    m_PanelAlias->SetSizer(PanelAliasBoxSizer);
    wxBoxSizer * LeftBoxSizer = new wxBoxSizer(wxVERTICAL);
	PanelAliasBoxSizer->Add(LeftBoxSizer, 0 , wxGROW|wxALL, 5);

	wxStaticText * Msg = new wxStaticText(m_PanelAlias, -1, _("Alias"));
	Msg->SetForegroundColour(wxColour(200,0,0) );
	LeftBoxSizer->Add(Msg, 0 , wxGROW|wxLEFT|wxRIGHT|wxTOP, 5);

	m_PartAliasList = new wxListBox(m_PanelAlias,
							-1,
							wxDefaultPosition, wxSize(160,170),
							0,NULL,
							wxLB_ALWAYS_SB|wxLB_SINGLE);
	LeftBoxSizer->Add(m_PartAliasList, 0 , wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxBoxSizer * RightBoxSizer = new wxBoxSizer(wxVERTICAL);
	PanelAliasBoxSizer->Add(RightBoxSizer, 0 , wxALIGN_CENTER_VERTICAL|wxALL, 5);

	Button = new wxButton(m_PanelAlias, ID_ADD_ALIAS, _("Add"));
	Button->SetForegroundColour(*wxBLUE);
	RightBoxSizer->Add(Button, 0 , wxGROW|wxALL, 5);
	
	m_ButtonDeleteOneAlias = new wxButton(m_PanelAlias, ID_DELETE_ONE_ALIAS,
						_("Delete"));
	m_ButtonDeleteOneAlias->SetForegroundColour(*wxRED);
	RightBoxSizer->Add(m_ButtonDeleteOneAlias, 0 , wxGROW|wxALL, 5);

	m_ButtonDeleteAllAlias = new wxButton(m_PanelAlias, ID_DELETE_ALL_ALIAS,
						_("Delete All"));
	m_ButtonDeleteAllAlias->SetForegroundColour(*wxRED);
	if ( ! CurrentAliasName.IsEmpty() ) m_ButtonDeleteAllAlias->Enable(FALSE);
	RightBoxSizer->Add(m_ButtonDeleteAllAlias, 0 , wxGROW|wxALL, 5);


	/* lecture des noms des alias */
	if ( CurrentLibEntry )
	{
		for ( unsigned ii = 0; ii < CurrentLibEntry->m_AliasList.GetCount(); ii += ALIAS_NEXT)
			m_PartAliasList->Append(CurrentLibEntry->m_AliasList[ii+ALIAS_NAME]);
	}
	
	if ( (CurrentLibEntry == NULL) || (CurrentLibEntry->m_AliasList.GetCount() == 0) )
	{
		m_ButtonDeleteAllAlias->Enable(FALSE);
		m_ButtonDeleteOneAlias->Enable(FALSE);
	}
}


/*****************************************************/
void WinEDA_PartPropertiesFrame::BuildPanelDoc(void)
/*****************************************************/
/* create the panel for component doc editing
*/
{
wxString msg_text;
	
	if ( m_AliasLocation >= 0 )
		msg_text = CurrentLibEntry->m_AliasList[m_AliasLocation+ALIAS_DOC];
	else
	{
		if (CurrentLibEntry && CurrentLibEntry->m_Doc)
		msg_text = CurrentLibEntry->m_Doc;
	}
	m_Doc->SetValue( msg_text );

	msg_text.Empty();
	if ( m_AliasLocation >= 0 )
		msg_text = CurrentLibEntry->m_AliasList[m_AliasLocation+ALIAS_KEYWORD];
	else
	{
		if (CurrentLibEntry ) msg_text = CurrentLibEntry->m_KeyWord;
	}
	m_Keywords->SetValue(msg_text);

	msg_text.Empty();
	if ( m_AliasLocation >= 0 )
		msg_text = CurrentLibEntry->m_AliasList[m_AliasLocation+ALIAS_DOC_FILENAME];
	else
	{
		if (CurrentLibEntry) msg_text = CurrentLibEntry->m_DocFile;
	}
	m_Docfile->SetValue( msg_text );
	
	if ( m_AliasLocation < 0 ) m_ButtonCopyDoc->Enable(FALSE);

}


/*****************************************************/
void WinEDA_PartPropertiesFrame::BuildPanelBasic(void)
/*****************************************************/
/* create the basic panel for component properties editing
*/
{
	m_PanelBasic->SetFont(*g_DialogFont);
	
	AsConvertButt = new wxCheckBox(m_PanelBasic,-1, _("As Convert"));
	if ( g_AsDeMorgan ) AsConvertButt->SetValue(TRUE);
	m_OptionsBoxSizer->Add(AsConvertButt, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

	ShowPinNumButt = new  wxCheckBox(m_PanelBasic,-1, _("Show Pin Num"));
	if ( CurrentLibEntry )
	{
		if ( CurrentLibEntry->m_DrawPinNum ) ShowPinNumButt->SetValue(TRUE);
	}
	else  ShowPinNumButt->SetValue(TRUE);
	m_OptionsBoxSizer->Add(ShowPinNumButt, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

	ShowPinNameButt = new wxCheckBox(m_PanelBasic,-1, _("Show Pin Name"));
	if ( CurrentLibEntry )
		{
		if( CurrentLibEntry->m_DrawPinName ) ShowPinNameButt->SetValue(TRUE);
		}
	else ShowPinNameButt->SetValue(TRUE);
	m_OptionsBoxSizer->Add(ShowPinNameButt, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

	m_PinsNameInsideButt = new wxCheckBox(m_PanelBasic,-1, _("Pin Name Inside"));
	if ( CurrentLibEntry )
	{
		if ( CurrentLibEntry->m_TextInside ) m_PinsNameInsideButt->SetValue(TRUE);
	}
	else m_PinsNameInsideButt->SetValue(TRUE);
	m_OptionsBoxSizer->Add(m_PinsNameInsideButt, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

	int number, number_of_units;
	if ( CurrentLibEntry ) number_of_units = CurrentLibEntry->m_UnitCount;
	else number_of_units = 1;
	SelNumberOfUnits->SetValue(number_of_units);

	if ( CurrentLibEntry && CurrentLibEntry->m_TextInside)
		number = CurrentLibEntry->m_TextInside;
	else number = 40;
	m_SetSkew->SetValue( number );

	if ( CurrentLibEntry )
	{
		if( CurrentLibEntry->m_Options == ENTRY_POWER )
			m_OptionPower->SetValue(TRUE);
	}
	
	if ( CurrentLibEntry )
	{
		if( CurrentLibEntry->m_UnitSelectionLocked )
			m_OptionPartsLocked->SetValue(TRUE);
	}
}

/*********************************************************/
void WinEDA_PartPropertiesFrame::BuildPanelEditField(void)
/**********************************************************/
/* Create and build the panel managing the fields (REF, VALUE ...)
	of the component
*/
{
wxString Hjustify_list[] =
		{ _("Left justify"),_("Center"),_("Right justify")};
wxString Vjustify_list[] =
		{ _("Bottom justify"),_("Center"),_("Top justify")};
int fieldId = m_CurrentFieldId;

	m_PanelField = new wxPanel(m_NoteBook, ID_PANEL_FIELD);
	m_PanelField->SetFont(*g_DialogFont);
	m_NoteBook->AddPage(m_PanelField, _("Fields"), FALSE);

    wxBoxSizer * PanelFieldBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    m_PanelField->SetSizer(PanelFieldBoxSizer);
    wxBoxSizer * LeftBoxSizer = new wxBoxSizer(wxVERTICAL);
	PanelFieldBoxSizer->Add(LeftBoxSizer, 0 , wxGROW|wxALL, 5);
    wxBoxSizer * MiddleBoxSizer = new wxBoxSizer(wxVERTICAL);
	PanelFieldBoxSizer->Add(MiddleBoxSizer, 0 , wxGROW|wxALL, 5);

	m_ShowFieldTextCtrl = new wxCheckBox(m_PanelField,-1,
			_("Show Text") );
	LeftBoxSizer->Add(m_ShowFieldTextCtrl, 0 , wxGROW|wxALL, 5);

	m_VorientFieldTextCtrl = new wxCheckBox(m_PanelField,-1,
			_("Vertical"));
	LeftBoxSizer->Add(m_VorientFieldTextCtrl, 0 , wxGROW|wxALL, 5);

	m_FieldPositionCtrl = new WinEDA_PositionCtrl( m_PanelField,
				_("Pos"), m_FieldPosition[fieldId],
				g_UnitMetric , LeftBoxSizer);

	wxString msg = _("Current field: XXXXXX");
	m_FieldTextCtrl = new WinEDA_GraphicTextCtrl( m_PanelField,
				msg,
				m_FieldText[fieldId], m_FieldSize[fieldId],
				g_UnitMetric, LeftBoxSizer, 200);

	m_FieldHJustifyCtrl = new wxRadioBox(m_PanelField,-1,
			_("Hor Justify"), wxDefaultPosition, wxDefaultSize,
			3,Hjustify_list, 1 , wxRA_SPECIFY_COLS);
	m_FieldHJustifyCtrl->SetSelection(1);
	MiddleBoxSizer->Add(m_FieldHJustifyCtrl, 0 , wxGROW|wxALL, 5);
	
	m_FieldVJustifyCtrl = new wxRadioBox(m_PanelField,-1,
			_("Vert Justify"), wxDefaultPosition, wxDefaultSize,
			3,Vjustify_list, 1 , wxRA_SPECIFY_COLS);
	m_FieldVJustifyCtrl->SetSelection(1);
	MiddleBoxSizer->Add(m_FieldVJustifyCtrl, 0 , wxGROW|wxALL, 5);
				
wxArrayString FieldListNames;
	for ( int ii = 0; ii < NUMBER_OF_FIELDS; ii++ )
		FieldListNames.Add(DrawPartStruct::ReturnFieldName(ii));

	m_FieldSelection = new wxRadioBox(m_PanelField, ID_ON_SELECT_FIELD,
			_("Field to edit"), wxDefaultPosition, wxDefaultSize,
			FieldListNames, 2 , wxRA_SPECIFY_COLS);
	PanelFieldBoxSizer->Add(m_FieldSelection, 0 , wxGROW|wxALL, 5);
		
	CopyDataToPanelField();
}

/****************************************************************/
void WinEDA_PartPropertiesFrame::CopyDataToPanelField(void)
/****************************************************************/
/* Set the values displayed on the panel field according to
	the current field number
*/
{
int fieldId = m_CurrentFieldId;

	if ( m_FieldFlags[fieldId] )
		m_ShowFieldTextCtrl->SetValue(TRUE);
	else m_ShowFieldTextCtrl->SetValue(FALSE);

	if ( m_FieldOrient[fieldId] ) m_VorientFieldTextCtrl->SetValue(TRUE);
	else  m_VorientFieldTextCtrl->SetValue(FALSE);
		
	m_FieldHJustifyCtrl->SetSelection(m_FieldHJustify[fieldId]);
	
	m_FieldVJustifyCtrl->SetSelection(m_FieldVJustify[fieldId]);

	m_FieldPositionCtrl->SetValue(m_FieldPosition[fieldId].x, m_FieldPosition[fieldId].y );

	wxString msg = _("Current field: ") + DrawPartStruct::ReturnFieldName(fieldId);
	m_FieldTextCtrl->SetTitle( msg );
	m_FieldTextCtrl->SetValue(m_FieldText[fieldId]);
	m_FieldTextCtrl->SetValue(m_FieldSize[fieldId]);
}

/****************************************************************/
void WinEDA_PartPropertiesFrame::CopyPanelFieldToData(void)
/****************************************************************/
/* Copy the values displayed on the panel field to the buffers according to
	the current field number
*/
{
int id = m_CurrentFieldId;

	m_FieldFlags[id] = m_ShowFieldTextCtrl->GetValue();
	m_FieldOrient[id] = m_VorientFieldTextCtrl->GetValue();
	m_FieldHJustify[id] = m_FieldHJustifyCtrl->GetSelection();
	m_FieldVJustify[id] = m_FieldVJustifyCtrl->GetSelection();
	m_FieldText[id] = m_FieldTextCtrl->GetText();
	m_FieldPosition[id] = m_FieldPositionCtrl->GetValue();
	m_FieldSize[id] = m_FieldTextCtrl->GetTextSize();
}


/********************************************************************/
void WinEDA_PartPropertiesFrame::SelectNewField(wxCommandEvent& event)
/********************************************************************/
/* called when changing the current field selected
	Save the current field settings in buffer and display the new one
*/
{
	CopyPanelFieldToData();
	m_CurrentFieldId = m_FieldSelection->GetSelection();
	CopyDataToPanelField();
}

/**************************************************************************/
void WinEDA_PartPropertiesFrame::PartPropertiesAccept(wxCommandEvent& event)
/**************************************************************************/
/* Met a jour les differents parametres pour le composant en cours d'�dition
*/
{
int ii, jj;
int hjustify[3] = {	GR_TEXT_HJUSTIFY_LEFT , GR_TEXT_HJUSTIFY_CENTER,
	GR_TEXT_HJUSTIFY_RIGHT };
int vjustify[3] = {	GR_TEXT_VJUSTIFY_BOTTOM , GR_TEXT_VJUSTIFY_CENTER,
	GR_TEXT_VJUSTIFY_TOP };
	
	if( CurrentLibEntry == NULL )
	{
		Close(); return;
	}

	m_Parent->m_CurrentScreen->SetModify();
	m_Parent->SaveCopyInUndoList();

	CopyPanelFieldToData();

	/* A new name could be entered in VALUE field.
		Must not be an existing alias name in alias list box */ 
	jj = m_PartAliasList->GetCount();
	wxString newvalue = m_FieldText[VALUE];
	for ( ii = 0; ii < jj; ii++ )
	{
		if( newvalue.CmpNoCase(m_PartAliasList->GetString(ii).GetData()) == 0 )
		{
		wxString msg;
		msg.Printf( wxT("Alias %s exists!"), newvalue.GetData());
		DisplayError(this, msg);
		return;
		}
	}

	/* Update the doc, keyword and doc filename strings */
	if ( m_AliasLocation < 0 )
	{
		CurrentLibEntry->m_Doc = m_Doc->GetValue();
		CurrentLibEntry->m_KeyWord = m_Keywords->GetValue();
		CurrentLibEntry->m_DocFile = m_Docfile->GetValue();
	}

	else
	{
		CurrentLibEntry->m_AliasList[m_AliasLocation+ALIAS_DOC] = m_Doc->GetValue();
		CurrentLibEntry->m_AliasList[m_AliasLocation+ALIAS_KEYWORD] = m_Keywords->GetValue();
		CurrentLibEntry->m_AliasList[m_AliasLocation+ALIAS_DOC_FILENAME] = m_Docfile->GetValue();
	}
	
	/* Update the alias list */
	/* 1 - Add names: test for a not existing name in old alias list: */
	jj = m_PartAliasList->GetCount();
	for ( ii = 0; ii < jj; ii++ )
	{
		if ( LocateAlias( CurrentLibEntry->m_AliasList, m_PartAliasList->GetString(ii)) < 0 )
		{	// new alias must be created
			CurrentLibEntry->m_AliasList.Add(m_PartAliasList->GetString(ii));
			CurrentLibEntry->m_AliasList.Add(wxEmptyString);	// Add a void doc string 
			CurrentLibEntry->m_AliasList.Add(wxEmptyString);	// Add a void keyword list string 
			CurrentLibEntry->m_AliasList.Add(wxEmptyString);	// Add a void doc filename string 
		}
	}
	
	/* 2 - Remove delete names: test for an non existing name in new alias list: */
	int kk, kkmax = CurrentLibEntry->m_AliasList.GetCount();
	for ( kk = 0; kk < kkmax; )
	{
		jj = m_PartAliasList->GetCount();
		wxString aliasname = CurrentLibEntry->m_AliasList[kk];
		for ( ii = 0; ii < jj; ii++ )
		{
			if( aliasname.CmpNoCase(m_PartAliasList->GetString(ii).GetData()) == 0 )
			{
				kk += ALIAS_NEXT; // Alias exist in new list. keep it and test next old name
				break;
			}
		}
		
		if ( ii == jj ) // Alias not found in new list, remove it (4 strings in kk position)
		{
			for( ii = 0; ii < ALIAS_NEXT; ii++ ) CurrentLibEntry->m_AliasList.RemoveAt(kk);
			kkmax = CurrentLibEntry->m_AliasList.GetCount();
		}
		
	}
	

	if ( ! m_FieldText[REFERENCE].IsEmpty() )
	{
		CurrentLibEntry->m_Prefix.m_Text = m_FieldText[REFERENCE];
	}

	if ( ! m_FieldText[VALUE].IsEmpty() )
	{
		if ( CurrentLibEntry->m_Name.m_Text != m_FieldText[VALUE] )
		{
			m_RecreateToolbar = TRUE;
			CurrentLibEntry->m_Name.m_Text = m_FieldText[VALUE];
		}
	}

	CurrentLibEntry->m_Prefix.m_Size.x = 
		CurrentLibEntry->m_Prefix.m_Size.y = m_FieldSize[REFERENCE];
	CurrentLibEntry->m_Name.m_Size.x =
		CurrentLibEntry->m_Name.m_Size.y = m_FieldSize[VALUE];

	CurrentLibEntry->m_Prefix.m_Pos = m_FieldPosition[REFERENCE];
	CurrentLibEntry->m_Name.m_Pos = m_FieldPosition[VALUE];

	CurrentLibEntry->m_Prefix.m_Orient = m_FieldOrient[REFERENCE] ? 1 : 0;
	CurrentLibEntry->m_Name.m_Orient = m_FieldOrient[VALUE] ? 1 : 0;

	CurrentLibEntry->m_Prefix.m_HJustify = hjustify[m_FieldHJustify[REFERENCE]];
	CurrentLibEntry->m_Prefix.m_VJustify = vjustify[m_FieldVJustify[REFERENCE]];
	CurrentLibEntry->m_Name.m_HJustify = hjustify[m_FieldHJustify[VALUE]];
	CurrentLibEntry->m_Name.m_VJustify = vjustify[m_FieldVJustify[VALUE]];

	if ( m_FieldFlags[REFERENCE] )
		CurrentLibEntry->m_Prefix.m_Attributs &= ~TEXT_NO_VISIBLE;
	else
		CurrentLibEntry->m_Prefix.m_Attributs |= TEXT_NO_VISIBLE;

	if ( m_FieldFlags[VALUE] )
		CurrentLibEntry->m_Name.m_Attributs &= ~TEXT_NO_VISIBLE;
	else
		CurrentLibEntry->m_Name.m_Attributs |= TEXT_NO_VISIBLE;

	for ( ii = FOOTPRINT; ii < NUMBER_OF_FIELDS; ii++ )
	{
		LibDrawField * Field = CurrentLibEntry->Fields;
		LibDrawField * NextField, * previousField = NULL;
		while ( Field )
		{
			NextField = (LibDrawField*)Field->Pnext;
			if( Field->m_FieldId == ii )
			{
				Field->m_Text = m_FieldText[ii];
				Field->m_Size.x = Field->m_Size.y = m_FieldSize[ii];
				Field->m_HJustify = hjustify[m_FieldHJustify[ii]];
				Field->m_VJustify = vjustify[m_FieldVJustify[ii]];
				if ( m_FieldFlags[ii] )
					Field->m_Attributs &= ~TEXT_NO_VISIBLE;
				else
					Field->m_Attributs |= TEXT_NO_VISIBLE;
				Field->m_Orient = m_FieldOrient[ii] ? 1 : 0;
				Field->m_Pos = m_FieldPosition[ii];
				if( Field->m_Text.IsEmpty() )	// An old field exists; new is void, delete it
				{
					delete Field;
					if ( previousField ) previousField->Pnext = NextField;
					else CurrentLibEntry->Fields = NextField;
				}
				break;
			}

			previousField = Field;
			Field = NextField;
		}

		if ( (Field == NULL) &&	( ! m_FieldText[ii].IsEmpty() ) )
		{	// Do not exists: must be created
			Field = new LibDrawField(ii);
			Field->m_Text = m_FieldText[ii];
			Field->m_Size.x = Field->m_Size.y = m_FieldSize[ii];
			if ( m_FieldFlags[Field->m_FieldId] )
				Field->m_Attributs &= ~TEXT_NO_VISIBLE;
			else
				Field->m_Attributs |= TEXT_NO_VISIBLE;
			Field->m_Orient = m_FieldOrient[Field->m_FieldId] ?1 : 0;
			Field->m_Pos = m_FieldPosition[Field->m_FieldId];
			Field->m_HJustify = hjustify[m_FieldHJustify[Field->m_FieldId]];
			Field->m_VJustify = vjustify[m_FieldVJustify[Field->m_FieldId]];
			Field->Pnext = CurrentLibEntry->Fields;
			CurrentLibEntry->Fields = Field;
		}
	}

	ii = SelNumberOfUnits->GetValue();
	if ( ChangeNbUnitsPerPackage(ii) )
		m_RecreateToolbar = TRUE;

	if ( AsConvertButt->GetValue() )
	{
		if ( ! g_AsDeMorgan )
		{
			g_AsDeMorgan = 1;
			if ( SetUnsetConvert() ) m_RecreateToolbar = TRUE;
		}
	}
	else
	{
		if ( g_AsDeMorgan )
		{
			g_AsDeMorgan = 0;
			if ( SetUnsetConvert() ) m_RecreateToolbar = TRUE;
		}
	}

	CurrentLibEntry->m_DrawPinNum = ShowPinNumButt->GetValue() ? 1 : 0;
	CurrentLibEntry->m_DrawPinName = ShowPinNameButt->GetValue() ? 1 : 0;

	if ( m_PinsNameInsideButt->GetValue() == FALSE)
		CurrentLibEntry->m_TextInside = 0;
	else
		CurrentLibEntry->m_TextInside = m_SetSkew->GetValue();

	if ( m_OptionPower->GetValue() == TRUE)
		CurrentLibEntry->m_Options = ENTRY_POWER;
	else
		CurrentLibEntry->m_Options = ENTRY_NORMAL;

	/* Set the option "Units locked".
	Obviously, cannot be TRUE if there is only one part */
	CurrentLibEntry->m_UnitSelectionLocked = m_OptionPartsLocked->GetValue();
	if ( CurrentLibEntry->m_UnitCount <= 1 )
		CurrentLibEntry->m_UnitSelectionLocked = FALSE;
	
	if ( m_RecreateToolbar ) m_Parent->ReCreateHToolbar();

	m_Parent->DisplayLibInfos();

	EndModal(1);
}


/*******************************************************************************/
void WinEDA_PartPropertiesFrame::CopyDocToAlias(wxCommandEvent& WXUNUSED(event))
/******************************************************************************/
{
	if( CurrentLibEntry == NULL ) return;
	if ( CurrentAliasName.IsEmpty() ) return;

	m_Doc->SetValue(CurrentLibEntry->m_Doc);
	m_Docfile->SetValue(CurrentLibEntry->m_DocFile);
	m_Keywords->SetValue(CurrentLibEntry->m_KeyWord);
}

/**********************************************************/
void WinEDA_PartPropertiesFrame::DeleteAllAliasOfPart(
		wxCommandEvent& WXUNUSED(event))
/**********************************************************/
{

	CurrentAliasName.Empty();
	if( CurrentLibEntry )
	{
		if( IsOK(this, _("Ok to Delete Alias LIST") ) )
		{
			m_PartAliasList->Clear();
			m_RecreateToolbar = TRUE;
			m_ButtonDeleteAllAlias->Enable(FALSE);
			m_ButtonDeleteOneAlias->Enable(FALSE);
		}
	}

}

/*******************************************************************************/
void WinEDA_PartPropertiesFrame::AddAliasOfPart( wxCommandEvent& WXUNUSED(event))
/*******************************************************************************/
/* Add a new name to the alias list box
	New name cannot be the root name, and must not exists
*/
{
wxString Line;
wxString aliasname;

	if(CurrentLibEntry == NULL) return;

	if( Get_Message(_("New alias:"),Line, this) != 0 ) return;

	Line.Replace( wxT(" "), wxT("_") );
	aliasname = Line;

	if ( CurrentLibEntry->m_Name.m_Text.CmpNoCase(Line) == 0 )
	{
		DisplayError(this, _("This is the Root Part"), 10); return;
	}
		
	/* test for an existing name: */
	int ii, jj = m_PartAliasList->GetCount();
	for ( ii = 0; ii < jj; ii++ )
	{
		if( aliasname.CmpNoCase(m_PartAliasList->GetString(ii)) == 0 )
		{
			DisplayError(this, _("Already in use"), 10);
			return;
		}
	}

	m_PartAliasList->Append(aliasname);
	if ( CurrentAliasName.IsEmpty() ) m_ButtonDeleteAllAlias->Enable(TRUE);
	m_ButtonDeleteOneAlias->Enable(TRUE);

	m_RecreateToolbar = TRUE;
}

/********************************************************/
void WinEDA_PartPropertiesFrame::DeleteAliasOfPart(
	wxCommandEvent& WXUNUSED(event))
/********************************************************/
{
wxString aliasname = m_PartAliasList->GetStringSelection();

	if ( aliasname.IsEmpty() ) return;
	if ( aliasname == CurrentAliasName )
	{
		wxString msg = CurrentAliasName + _(" is Current Selected Alias!");
		DisplayError(this, msg );
		return;
	}
	
	int ii = m_PartAliasList->GetSelection();
	m_PartAliasList->Delete(ii);

	if ( ! CurrentLibEntry || (CurrentLibEntry->m_AliasList.GetCount() == 0) )
	{
		m_ButtonDeleteAllAlias->Enable(FALSE);
		m_ButtonDeleteOneAlias->Enable(FALSE);
	}
	m_RecreateToolbar = TRUE;
}




/********************************************************************/
bool WinEDA_PartPropertiesFrame::ChangeNbUnitsPerPackage(int MaxUnit)
/********************************************************************/
/* Routine de modification du nombre d'unites par package pour le
	composant courant;
*/
{
int OldNumUnits, ii, FlagDel = -1;
LibEDA_BaseStruct* DrawItem, * NextDrawItem;

	if( CurrentLibEntry == NULL ) return FALSE;

	/* Si pas de changement: termine */
	if ( CurrentLibEntry->m_UnitCount == MaxUnit ) return FALSE;

	OldNumUnits = CurrentLibEntry->m_UnitCount;
		if ( OldNumUnits < 1 ) OldNumUnits = 1;

	CurrentLibEntry->m_UnitCount = MaxUnit;


	/* Traitement des unites enlevees ou rajoutees */
	if(OldNumUnits > CurrentLibEntry->m_UnitCount )
	{
		DrawItem = CurrentLibEntry->m_Drawings;
		for ( ; DrawItem != NULL; DrawItem = NextDrawItem)
		{
			NextDrawItem = DrawItem->Next();
			if( DrawItem->m_Unit > MaxUnit )  /* Item a effacer */
			{
				if( FlagDel < 0 )
				{
					if( IsOK(this, _("Delete units") ) )
					{
						/* Si part selectee n'existe plus: selection 1ere unit */
						if( CurrentUnit > MaxUnit ) CurrentUnit = 1;
						FlagDel = 1;
					}
					else
					{
						FlagDel = 0;
						MaxUnit = OldNumUnits;
						CurrentLibEntry->m_UnitCount = MaxUnit;
						return FALSE;
					}
				}
				DeleteOneLibraryDrawStruct(m_Parent->DrawPanel, NULL, CurrentLibEntry,
								DrawItem, 0);
			}
		}
		return TRUE;
	}

	if(OldNumUnits < CurrentLibEntry->m_UnitCount )
	{
		DrawItem = CurrentLibEntry->m_Drawings;
		for ( ; DrawItem != NULL; DrawItem = DrawItem->Next() )
		{
			/* Duplication des items pour autres elements */
			if( DrawItem->m_Unit == 1 )
			{
				for ( ii = OldNumUnits +1; ii <= MaxUnit; ii ++ )
				{
					NextDrawItem = CopyDrawEntryStruct(this, DrawItem);
					NextDrawItem->Pnext = CurrentLibEntry->m_Drawings;
					CurrentLibEntry->m_Drawings = NextDrawItem;
					NextDrawItem->m_Unit = ii;
				}
			}
		}
	}
	return TRUE;
}


/*****************************************************/
bool WinEDA_PartPropertiesFrame::SetUnsetConvert(void)
/*****************************************************/
/* cr�e ou efface (selon option AsConvert) les �l�ments
	de la repr�sentation convertie d'un composant
*/
{
int FlagDel = 0;
LibEDA_BaseStruct* DrawItem = NULL, * NextDrawItem;

	if( g_AsDeMorgan )	/* Representation convertie a creer */
	{
		/* Traitement des elements a ajouter ( pins seulement ) */
		if( CurrentLibEntry ) DrawItem = CurrentLibEntry->m_Drawings;
		for ( ; DrawItem != NULL; DrawItem = DrawItem->Next() )
		{
			/* Duplication des items pour autres elements */
			if( DrawItem->m_StructType != COMPONENT_PIN_DRAW_TYPE ) continue;
			if( DrawItem->m_Convert == 1 )
			{
				if( FlagDel == 0 )
				{
					if( IsOK(this, _("Create pins for Convert items")) )
						FlagDel = 1;
					else
					{
						if( IsOK(this, _("Part as \"De Morgan\" anymore")) )
							return TRUE;

						g_AsDeMorgan = 0; return FALSE;
					}
				}
				NextDrawItem = CopyDrawEntryStruct(this, DrawItem);
				NextDrawItem->Pnext = CurrentLibEntry->m_Drawings;
				CurrentLibEntry->m_Drawings = NextDrawItem;
				NextDrawItem->m_Convert = 2;
			}
		}
	}

	else			   /* Representation convertie a supprimer */
	{
		/* Traitement des elements � supprimer */
		if( CurrentLibEntry ) DrawItem = CurrentLibEntry->m_Drawings;
		for ( ; DrawItem != NULL; DrawItem = NextDrawItem)
		{
			NextDrawItem = DrawItem->Next();
			if( DrawItem->m_Convert > 1 )  /* Item a effacer */
			{
				if( FlagDel == 0 )
				{
					if( IsOK(this, _("Delete Convert items") ) )
					{
						CurrentConvert = 1;
						FlagDel = 1;
					}
					else
					{
						g_AsDeMorgan = 1;
						return FALSE;
					}
				}
				m_Parent->GetScreen()->SetModify();
				DeleteOneLibraryDrawStruct(m_Parent->DrawPanel, NULL, CurrentLibEntry, DrawItem, 0);
			}
		}
	}
	return TRUE;
}

/****************************************************************************/
void WinEDA_PartPropertiesFrame::BrowseAndSelectDocFile(wxCommandEvent& event)
/****************************************************************************/
{
wxString FullFileName, mask;
wxString docpath(g_RealLibDirBuffer), filename;
	
	docpath += wxT("doc"); 
	docpath += STRING_DIR_SEP; 
	mask = wxT("*");
	FullFileName = EDA_FileSelector(_("Doc Files"),
					docpath,			/* Chemin par defaut */
					wxEmptyString,					/* nom fichier par defaut */
					wxEmptyString,					/* extension par defaut */
					mask,					/* Masque d'affichage */
					this,
					wxOPEN,
					TRUE
					);
	if ( FullFileName.IsEmpty() ) return;

	// Suppression du chemin par defaut pour le fichier de doc:
	filename = MakeReducedFileName(FullFileName,docpath, wxEmptyString);
	m_Docfile->SetValue(filename);
}	
