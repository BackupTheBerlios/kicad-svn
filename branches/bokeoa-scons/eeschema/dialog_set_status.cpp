/////////////////////////////////////////////////////////////////////////////
// Name:        dialog_set_status.cpp
// Purpose:     
// Author:      jean-pierre Charras
// Modified by: 
// Created:     11/02/2006 21:42:23
// RCS-ID:      
// Copyright:   License GNU
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 11/02/2006 21:42:23

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dialog_set_status.h"
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

#include "dialog_set_status.h"

////@begin XPM images
////@end XPM images

/*!
 * WinEDA_ConfigFrame type definition
 */

IMPLEMENT_DYNAMIC_CLASS( WinEDA_ConfigFrame, wxDialog )

/*!
 * WinEDA_ConfigFrame event table definition
 */

BEGIN_EVENT_TABLE( WinEDA_ConfigFrame, wxDialog )

////@begin WinEDA_ConfigFrame event table entries
    EVT_BUTTON( ID_BUTTON, WinEDA_ConfigFrame::OnButtonClick )

    EVT_BUTTON( ID_BUTTON1, WinEDA_ConfigFrame::OnButton1Click )

    EVT_BUTTON( ID_BUTTON2, WinEDA_ConfigFrame::OnButton2Click )

    EVT_BUTTON( ID_BUTTON3, WinEDA_ConfigFrame::OnButton3Click )

////@end WinEDA_ConfigFrame event table entries

END_EVENT_TABLE()

/*!
 * WinEDA_ConfigFrame constructors
 */

WinEDA_ConfigFrame::WinEDA_ConfigFrame( )
{
}

WinEDA_ConfigFrame::WinEDA_ConfigFrame( WinEDA_SchematicFrame* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
	m_Parent = parent;
	m_LibListChanged = FALSE;

	msg = _("from ") + EDA_Appl->m_CurrentOptionFile;
	SetTitle(msg);
    Create(parent, id, caption, pos, size, style);
}

/*!
 * WinEDA_ConfigFrame creator
 */

bool WinEDA_ConfigFrame::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin WinEDA_ConfigFrame member initialisation
    m_CmpfileExtText = NULL;
    m_NetfileExtText = NULL;
    m_LibfileExtText = NULL;
    m_SymbolfileExtText = NULL;
    m_SchfileExtText = NULL;
    m_ListLibr = NULL;
    m_CmpfileExtText = NULL;
    m_LibDirCtrl = NULL;
////@end WinEDA_ConfigFrame member initialisation

////@begin WinEDA_ConfigFrame creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end WinEDA_ConfigFrame creation
    return true;
}

/*!
 * Control creation for WinEDA_ConfigFrame
 */

void WinEDA_ConfigFrame::CreateControls()
{    
	SetFont(*g_DialogFont);
////@begin WinEDA_ConfigFrame content construction
    // Generated by DialogBlocks, 11/02/2006 22:06:21 (unregistered)

    WinEDA_ConfigFrame* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, 0, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, ID_BUTTON, _("Save Cfg"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton5->SetForegroundColour(wxColour(204, 0, 0));
    itemBoxSizer4->Add(itemButton5, 0, wxGROW|wxALL, 5);

    wxString itemRadioBox6Strings[] = {
        _("&PcbNew"),
        _("&OrcadPcb2"),
        _("&CadStar"),
        _("&Spice"),
        _("Other")
    };
    wxRadioBox* itemRadioBox6 = new wxRadioBox( itemDialog1, ID_RADIOBOX, _("NetList Formats:"), wxDefaultPosition, wxDefaultSize, 5, itemRadioBox6Strings, 1, wxRA_SPECIFY_COLS );
    itemBoxSizer4->Add(itemRadioBox6, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Files ext:"));
    wxStaticBoxSizer* itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxVERTICAL);
    itemBoxSizer4->Add(itemStaticBoxSizer7, 0, wxGROW|wxALL, 5);

    m_CmpfileExtText = new wxStaticText( itemDialog1, wxID_STATIC, _("Cmp file Ext: "), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer7->Add(m_CmpfileExtText, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

    m_NetfileExtText = new wxStaticText( itemDialog1, wxID_STATIC, _("Net file Ext: "), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer7->Add(m_NetfileExtText, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

    m_LibfileExtText = new wxStaticText( itemDialog1, wxID_STATIC, _("Library file Ext: "), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer7->Add(m_LibfileExtText, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

    m_SymbolfileExtText = new wxStaticText( itemDialog1, wxID_STATIC, _("Symbol file Ext: "), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer7->Add(m_SymbolfileExtText, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

    m_SchfileExtText = new wxStaticText( itemDialog1, wxID_STATIC, _("Schematic file Ext: "), wxDefaultPosition, wxDefaultSize, 0 );
    m_SchfileExtText->Enable(false);
    itemStaticBoxSizer7->Add(m_SchfileExtText, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer13, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer14 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer13->Add(itemBoxSizer14, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton15 = new wxButton( itemDialog1, ID_BUTTON1, _("Del"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton15->SetForegroundColour(wxColour(204, 0, 0));
    itemBoxSizer14->Add(itemButton15, 0, wxGROW|wxALL, 5);

    wxButton* itemButton16 = new wxButton( itemDialog1, ID_BUTTON2, _("Add"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton16->SetForegroundColour(wxColour(0, 128, 0));
    itemBoxSizer14->Add(itemButton16, 0, wxGROW|wxALL, 5);

    wxButton* itemButton17 = new wxButton( itemDialog1, ID_BUTTON3, _("Ins"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton17->SetForegroundColour(wxColour(0, 0, 255));
    itemBoxSizer14->Add(itemButton17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText18 = new wxStaticText( itemDialog1, wxID_STATIC, _("Libraries:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText18->SetForegroundColour(wxColour(196, 0, 0));
    itemBoxSizer13->Add(itemStaticText18, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    wxString* m_ListLibrStrings = NULL;
    m_ListLibr = new wxListBox( itemDialog1, ID_LISTBOX, wxDefaultPosition, wxSize(-1, 250), 0, m_ListLibrStrings, wxLB_SINGLE );
    itemBoxSizer13->Add(m_ListLibr, 0, wxGROW|wxALL, 5);

    m_CmpfileExtText = new wxStaticText( itemDialog1, wxID_STATIC, _("Library files path:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CmpfileExtText->SetForegroundColour(wxColour(198, 0, 0));
    itemBoxSizer2->Add(m_CmpfileExtText, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_LibDirCtrl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_LibDirCtrl, 0, wxGROW|wxALL, 5);

////@end WinEDA_ConfigFrame content construction
}

/*!
 * Should we show tooltips?
 */

bool WinEDA_ConfigFrame::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap WinEDA_ConfigFrame::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin WinEDA_ConfigFrame bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end WinEDA_ConfigFrame bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon WinEDA_ConfigFrame::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin WinEDA_ConfigFrame icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end WinEDA_ConfigFrame icon retrieval
}
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */

void WinEDA_ConfigFrame::OnButtonClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON in WinEDA_ConfigFrame.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON in WinEDA_ConfigFrame. 
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
 */

void WinEDA_ConfigFrame::OnButton1Click( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1 in WinEDA_ConfigFrame.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1 in WinEDA_ConfigFrame. 
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
 */

void WinEDA_ConfigFrame::OnButton2Click( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2 in WinEDA_ConfigFrame.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2 in WinEDA_ConfigFrame. 
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3
 */

void WinEDA_ConfigFrame::OnButton3Click( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3 in WinEDA_ConfigFrame.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3 in WinEDA_ConfigFrame. 
}


