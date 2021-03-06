/////////////////////////////////////////////////////////////////////////////
// Name:        edtxtmod.cpp
// Purpose:     
// Author:      jean-pierre Charras
// Modified by: 
// Created:     09/02/2006 18:17:15
// RCS-ID:      
// Copyright:   License GNU
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 09/02/2006 18:17:15

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "edtxtmod.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "pcbnew.h"

#include "dialog_edit_mod_text.h"

extern wxPoint MoveVector;	// Move vector for move edge, imported from dialog_edit mod_text.cpp

////@begin XPM images
////@end XPM images

/***************************************************************************/
void WinEDA_BasePcbFrame::InstallTextModOptionsFrame(TEXTE_MODULE * TextMod,
					wxDC * DC, const wxPoint & pos)
/***************************************************************************/
{
	DrawPanel->m_IgnoreMouseEvents = TRUE;
	WinEDA_TextModPropertiesFrame * frame = new WinEDA_TextModPropertiesFrame(this,
					 TextMod, DC);
	frame->ShowModal(); frame->Destroy();
	DrawPanel->m_IgnoreMouseEvents = FALSE;
}

/*!
 * WinEDA_TextModPropertiesFrame type definition
 */

IMPLEMENT_DYNAMIC_CLASS( WinEDA_TextModPropertiesFrame, wxDialog )

/*!
 * WinEDA_TextModPropertiesFrame event table definition
 */

BEGIN_EVENT_TABLE( WinEDA_TextModPropertiesFrame, wxDialog )

////@begin WinEDA_TextModPropertiesFrame event table entries
    EVT_BUTTON( wxID_OK, WinEDA_TextModPropertiesFrame::OnOkClick )

    EVT_BUTTON( wxID_CANCEL, WinEDA_TextModPropertiesFrame::OnCancelClick )

////@end WinEDA_TextModPropertiesFrame event table entries

END_EVENT_TABLE()

/*!
 * WinEDA_TextModPropertiesFrame constructors
 */

WinEDA_TextModPropertiesFrame::WinEDA_TextModPropertiesFrame( )
{
}

WinEDA_TextModPropertiesFrame::WinEDA_TextModPropertiesFrame( WinEDA_BasePcbFrame* parent,
		TEXTE_MODULE * TextMod,wxDC * DC, 
		wxWindowID id, const wxString& caption, const wxPoint& pos,
		const wxSize& size, long style )
{
	m_Parent = parent;
	m_DC = DC;
	m_Module = NULL;
	m_CurrentTextMod = TextMod;
	if ( m_CurrentTextMod )
	{
		m_Module = (MODULE*)m_CurrentTextMod->m_Parent;
	}

    Create(parent, id, caption, pos, size, style);

}

/*!
 * WinEDA_TextModPropertiesFrame creator
 */

bool WinEDA_TextModPropertiesFrame::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin WinEDA_TextModPropertiesFrame member initialisation
    m_ModuleInfoText = NULL;
    m_TextDataTitle = NULL;
    m_Name = NULL;
    m_SizeXTitle = NULL;
    m_TxtSizeCtrlX = NULL;
    m_SizeYTitle = NULL;
    m_TxtSizeCtrlY = NULL;
    m_PosXTitle = NULL;
    m_TxtPosCtrlX = NULL;
    m_PosYTitle = NULL;
    m_TxtPosCtrlY = NULL;
    m_WidthTitle = NULL;
    m_TxtWidthCtlr = NULL;
    m_Orient = NULL;
    m_Show = NULL;
////@end WinEDA_TextModPropertiesFrame member initialisation

////@begin WinEDA_TextModPropertiesFrame creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end WinEDA_TextModPropertiesFrame creation
    return true;
}

/*!
 * Control creation for WinEDA_TextModPropertiesFrame
 */

void WinEDA_TextModPropertiesFrame::CreateControls()
{    
	SetFont(*g_DialogFont);
////@begin WinEDA_TextModPropertiesFrame content construction
    // Generated by DialogBlocks, 09/02/2006 19:51:49 (unregistered)

    WinEDA_TextModPropertiesFrame* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_ModuleInfoText = new wxStaticText( itemDialog1, wxID_STATIC, _("Module %s (%s) orient %.1f"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ModuleInfoText->SetForegroundColour(wxColour(0, 0, 255));
    itemBoxSizer2->Add(m_ModuleInfoText, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxADJUST_MINSIZE, 5);

    wxStaticLine* itemStaticLine4 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine4, 0, wxGROW|wxALL, 5);

    m_TextDataTitle = new wxStaticText( itemDialog1, wxID_STATIC, _("Reference:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_TextDataTitle, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_Name = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_Name, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer7->Add(itemBoxSizer8, 0, wxGROW|wxALL, 5);

    m_SizeXTitle = new wxStaticText( itemDialog1, wxID_STATIC, _("Size X"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(m_SizeXTitle, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_TxtSizeCtrlX = new wxTextCtrl( itemDialog1, ID_TEXTCTRL1, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(m_TxtSizeCtrlX, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    m_SizeYTitle = new wxStaticText( itemDialog1, wxID_STATIC, _("Size Y"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(m_SizeYTitle, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_TxtSizeCtrlY = new wxTextCtrl( itemDialog1, ID_TEXTCTRL2, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(m_TxtSizeCtrlY, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    itemBoxSizer8->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_PosXTitle = new wxStaticText( itemDialog1, wxID_STATIC, _("Offset X"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(m_PosXTitle, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_TxtPosCtrlX = new wxTextCtrl( itemDialog1, ID_TEXTCTRL3, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(m_TxtPosCtrlX, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    m_PosYTitle = new wxStaticText( itemDialog1, wxID_STATIC, _("Offset Y"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(m_PosYTitle, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_TxtPosCtrlY = new wxTextCtrl( itemDialog1, ID_TEXTCTRL4, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(m_TxtPosCtrlY, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer7->Add(itemBoxSizer18, 0, wxGROW|wxALL, 5);

    m_WidthTitle = new wxStaticText( itemDialog1, wxID_STATIC, _("Width"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(m_WidthTitle, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_TxtWidthCtlr = new wxTextCtrl( itemDialog1, ID_TEXTCTRL5, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(m_TxtWidthCtlr, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer7->Add(itemBoxSizer21, 0, wxGROW|wxALL, 5);

    wxButton* itemButton22 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton22->SetForegroundColour(wxColour(198, 0, 0));
    itemBoxSizer21->Add(itemButton22, 0, wxGROW|wxALL, 5);

    wxButton* itemButton23 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton23->SetForegroundColour(wxColour(0, 0, 255));
    itemBoxSizer21->Add(itemButton23, 0, wxGROW|wxALL, 5);

    wxString m_OrientStrings[] = {
        _("horiz"),
        _("vertical")
    };
    m_Orient = new wxRadioBox( itemDialog1, ID_RADIOBOX, _("Orientation"), wxDefaultPosition, wxDefaultSize, 2, m_OrientStrings, 1, wxRA_SPECIFY_COLS );
    itemBoxSizer21->Add(m_Orient, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxString m_ShowStrings[] = {
        _("show"),
        _("no show")
    };
    m_Show = new wxRadioBox( itemDialog1, ID_RADIOBOX1, _("Display"), wxDefaultPosition, wxDefaultSize, 2, m_ShowStrings, 1, wxRA_SPECIFY_COLS );
    itemBoxSizer21->Add(m_Show, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end WinEDA_TextModPropertiesFrame content construction

	/* Setup titles and datas */
	SetDisplayValue();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void WinEDA_TextModPropertiesFrame::OnOkClick( wxCommandEvent& event )
{
	TextModPropertiesAccept(event);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void WinEDA_TextModPropertiesFrame::OnCancelClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in WinEDA_TextModPropertiesFrame.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in WinEDA_TextModPropertiesFrame. 
}

/*!
 * Should we show tooltips?
 */

bool WinEDA_TextModPropertiesFrame::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap WinEDA_TextModPropertiesFrame::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin WinEDA_TextModPropertiesFrame bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end WinEDA_TextModPropertiesFrame bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon WinEDA_TextModPropertiesFrame::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin WinEDA_TextModPropertiesFrame icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end WinEDA_TextModPropertiesFrame icon retrieval
}

/********************************************************/
void WinEDA_TextModPropertiesFrame::SetDisplayValue(void)
/********************************************************/
{
wxString msg;
	
	if ( m_Module )
	{
		wxString format = m_ModuleInfoText->GetLabel();
		msg.Printf( format,
			m_Module->m_Reference->m_Text.GetData(),
			m_Module->m_Value->m_Text.GetData(),
			(float)(m_Module->m_Orient/10) );
		m_ModuleInfoText->SetLabel(msg);
	}
	
	if(m_CurrentTextMod->m_Type == TEXT_is_VALUE) m_TextDataTitle->SetLabel(_("Value:"));
	else if(m_CurrentTextMod->m_Type == TEXT_is_DIVERS) m_TextDataTitle->SetLabel(_("Text:"));
	else if(m_CurrentTextMod->m_Type != TEXT_is_REFERENCE) m_TextDataTitle->SetLabel(wxT("???"));

	m_Name->SetValue(m_CurrentTextMod->m_Text);

	AddUnitSymbol(*m_SizeXTitle);
	PutValueInLocalUnits(*m_TxtSizeCtrlX, m_CurrentTextMod->m_Size.x,
		m_Parent->m_InternalUnits);
	
	AddUnitSymbol(*m_SizeYTitle);
	PutValueInLocalUnits(*m_TxtSizeCtrlY, m_CurrentTextMod->m_Size.y,
		m_Parent->m_InternalUnits);
	
	AddUnitSymbol(*m_PosXTitle);
	PutValueInLocalUnits(*m_TxtPosCtrlX, m_CurrentTextMod->m_Pos0.x,
		m_Parent->m_InternalUnits);
	
	AddUnitSymbol(*m_PosYTitle);
	PutValueInLocalUnits(*m_TxtPosCtrlY, m_CurrentTextMod->m_Pos0.y,
		m_Parent->m_InternalUnits);

	AddUnitSymbol(*m_WidthTitle);
	PutValueInLocalUnits(*m_TxtWidthCtlr, m_CurrentTextMod->m_Width,
		m_Parent->m_InternalUnits);

	if ( (m_CurrentTextMod->m_Orient != 0) && (m_CurrentTextMod->m_Orient != 1800) &&
		(m_CurrentTextMod->m_Orient != -1800) )
		m_Orient->SetSelection(1);;

	if ( m_CurrentTextMod->m_NoShow ) m_Show->SetSelection(1);;
}

/*********************************************************************************/
void WinEDA_TextModPropertiesFrame::TextModPropertiesAccept(wxCommandEvent& event)
/*********************************************************************************/
{
wxString msg;

	m_Parent->SaveCopyInUndoList();
	if ( m_DC )		// Effacement ancien texte
	{
		m_CurrentTextMod->Draw(m_Parent->DrawPanel, m_DC,
			(m_CurrentTextMod->m_Flags & IS_MOVED) ? MoveVector : wxPoint(0,0), GR_XOR );
	}
	m_CurrentTextMod->m_Text = m_Name->GetValue();

	msg = m_TxtPosCtrlX->GetValue();
	m_CurrentTextMod->m_Pos0.x = ReturnValueFromString(g_UnitMetric, msg,
		m_Parent->m_InternalUnits);
	msg = m_TxtPosCtrlY->GetValue();
	m_CurrentTextMod->m_Pos0.y = ReturnValueFromString(g_UnitMetric, msg,
		m_Parent->m_InternalUnits);

	msg = m_TxtSizeCtrlX->GetValue();
	m_CurrentTextMod->m_Size.x = ReturnValueFromString(g_UnitMetric, msg,
		m_Parent->m_InternalUnits);
	msg = m_TxtSizeCtrlY->GetValue();
	m_CurrentTextMod->m_Size.y = ReturnValueFromString(g_UnitMetric, msg,
		m_Parent->m_InternalUnits);

	msg = m_TxtWidthCtlr->GetValue();
	m_CurrentTextMod->SetWidth( ReturnValueFromString(g_UnitMetric, msg,
		m_Parent->m_InternalUnits) );

	m_CurrentTextMod->m_NoShow = (m_Show->GetSelection() == 0) ? 0 : 1;
	m_CurrentTextMod->m_Orient = (m_Orient->GetSelection() == 0) ? 0 : 900;
	m_CurrentTextMod->SetDrawCoord();
	if ( m_DC )		// Affichage nouveau texte
	{
		m_CurrentTextMod->Draw(m_Parent->DrawPanel, m_DC,
			(m_CurrentTextMod->m_Flags & IS_MOVED) ? MoveVector : wxPoint(0,0), GR_XOR );
	}
	m_Parent->GetScreen()->SetModify();
	((MODULE*)m_CurrentTextMod->m_Parent)->m_LastEdit_Time = time(NULL);
	Close(TRUE);
}

