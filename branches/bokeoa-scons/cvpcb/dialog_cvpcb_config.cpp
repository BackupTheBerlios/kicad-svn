/////////////////////////////////////////////////////////////////////////////
// Name:        dialog_cvpcb_config.cpp
// Purpose:     
// Author:      jean-pierre Charras
// Modified by: 
// Created:     17/02/2006 18:43:13
// RCS-ID:      
// Copyright:   License GNU
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 17/02/2006 18:43:13


////@begin includes
////@end includes

#include "dialog_cvpcb_config.h"

////@begin XPM images
////@end XPM images

/*!
 * KiConfigCvpcbFrame type definition
 */

IMPLEMENT_DYNAMIC_CLASS( KiConfigCvpcbFrame, wxDialog )

/*!
 * KiConfigCvpcbFrame event table definition
 */

BEGIN_EVENT_TABLE( KiConfigCvpcbFrame, wxDialog )

////@begin KiConfigCvpcbFrame event table entries
    EVT_BUTTON( SAVE_CFG, KiConfigCvpcbFrame::OnSaveCfgClick )

    EVT_BUTTON( ID_READ_OLDCFG, KiConfigCvpcbFrame::OnReadOldcfgClick )

    EVT_BUTTON( wxID_OK, KiConfigCvpcbFrame::OnOkClick )

    EVT_RADIOBOX( FORMAT_NETLIST, KiConfigCvpcbFrame::OnFormatNetlistSelected )

    EVT_BUTTON( DEL_LIB, KiConfigCvpcbFrame::OnDelLibClick )

    EVT_BUTTON( ADD_LIB, KiConfigCvpcbFrame::OnAddLibClick )

    EVT_BUTTON( INSERT_LIB, KiConfigCvpcbFrame::OnInsertLibClick )

    EVT_BUTTON( DEL_EQU, KiConfigCvpcbFrame::OnDelEquClick )

    EVT_BUTTON( ADD_EQU, KiConfigCvpcbFrame::OnAddEquClick )

    EVT_BUTTON( INSERT_EQU, KiConfigCvpcbFrame::OnInsertEquClick )

////@end KiConfigCvpcbFrame event table entries

END_EVENT_TABLE()

/*!
 * KiConfigCvpcbFrame constructors
 */

KiConfigCvpcbFrame::KiConfigCvpcbFrame( )
{
}

KiConfigCvpcbFrame::KiConfigCvpcbFrame( WinEDA_CvpcbFrame* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
wxString title;
	m_Parent = parent;
	m_DoUpdate = TRUE;

	title = _("from ") + EDA_Appl->m_CurrentOptionFile;
	SetTitle(title);
    Create(parent, id, caption, pos, size, style);
}

/*!
 * KiConfigCvpcbFrame creator
 */

bool KiConfigCvpcbFrame::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin KiConfigCvpcbFrame member initialisation
    m_NetFormatBox = NULL;
    m_FileExtList = NULL;
    m_RightBoxSizer = NULL;
    m_ListLibr = NULL;
    m_ListEquiv = NULL;
    m_FileExtBoxSizerH = NULL;
    m_NetExtBoxSizer = NULL;
    m_PkgExtBoxSizer = NULL;
////@end KiConfigCvpcbFrame member initialisation

////@begin KiConfigCvpcbFrame creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end KiConfigCvpcbFrame creation
    return true;
}

/*!
 * Control creation for KiConfigCvpcbFrame
 */

void KiConfigCvpcbFrame::CreateControls()
{    
	SetFont(*g_DialogFont);

////@begin KiConfigCvpcbFrame content construction
    // Generated by DialogBlocks, 28/02/2006 16:11:39 (unregistered)

    KiConfigCvpcbFrame* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    wxButton* itemButton4 = new wxButton( itemDialog1, SAVE_CFG, _("Save Cfg"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton4->SetForegroundColour(wxColour(198, 0, 0));
    itemBoxSizer3->Add(itemButton4, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, ID_READ_OLDCFG, _("Read Cfg"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton5->SetForegroundColour(wxColour(0, 0, 255));
    itemBoxSizer3->Add(itemButton5, 0, wxGROW|wxLEFT|wxRIGHT, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton6->SetForegroundColour(wxColour(0, 128, 0));
    itemBoxSizer3->Add(itemButton6, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxString m_NetFormatBoxStrings[] = {
        _("&PcbNew"),
        _("&ViewLogic"),
        _("View &Net + Pkg")
    };
    m_NetFormatBox = new wxRadioBox( itemDialog1, FORMAT_NETLIST, _("NetList Formats:"), wxDefaultPosition, wxDefaultSize, 3, m_NetFormatBoxStrings, 1, wxRA_SPECIFY_COLS );
    itemBoxSizer3->Add(m_NetFormatBox, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer8Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Files ext:"));
    m_FileExtList = new wxStaticBoxSizer(itemStaticBoxSizer8Static, wxVERTICAL);
    itemBoxSizer3->Add(m_FileExtList, 0, wxGROW|wxALL, 5);

    m_RightBoxSizer = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(m_RightBoxSizer, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    m_RightBoxSizer->Add(itemBoxSizer10, 0, wxALIGN_CENTER_HORIZONTAL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer10->Add(itemBoxSizer11, 0, wxGROW|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer11->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL, 5);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer12->Add(itemBoxSizer13, 0, wxALIGN_CENTER_HORIZONTAL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, DEL_LIB, _("Del"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton14->SetForegroundColour(wxColour(191, 0, 0));
    itemBoxSizer13->Add(itemButton14, 0, wxGROW|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxButton* itemButton15 = new wxButton( itemDialog1, ADD_LIB, _("Add"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton15->SetForegroundColour(wxColour(0, 128, 0));
    itemBoxSizer13->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    wxButton* itemButton16 = new wxButton( itemDialog1, INSERT_LIB, _("Ins"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton16->SetForegroundColour(wxColour(0, 0, 255));
    itemBoxSizer13->Add(itemButton16, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer12->Add(itemBoxSizer17, 0, wxGROW|wxTOP, 5);

    wxStaticText* itemStaticText18 = new wxStaticText( itemDialog1, wxID_STATIC, _("Libraries"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText18->SetForegroundColour(wxColour(204, 0, 0));
    itemBoxSizer17->Add(itemStaticText18, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    wxString* m_ListLibrStrings = NULL;
    m_ListLibr = new wxListBox( itemDialog1, ID_LISTBOX, wxDefaultPosition, wxSize(-1, 200), 0, m_ListLibrStrings, wxLB_SINGLE );
    itemBoxSizer17->Add(m_ListLibr, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer20 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer10->Add(itemBoxSizer20, 0, wxGROW|wxTOP|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer20->Add(itemBoxSizer21, 0, wxALIGN_CENTER_HORIZONTAL, 5);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer21->Add(itemBoxSizer22, 0, wxALIGN_CENTER_HORIZONTAL, 5);

    wxButton* itemButton23 = new wxButton( itemDialog1, DEL_EQU, _("Del"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton23->SetForegroundColour(wxColour(196, 0, 0));
    itemBoxSizer22->Add(itemButton23, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxButton* itemButton24 = new wxButton( itemDialog1, ADD_EQU, _("Add"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton24->SetForegroundColour(wxColour(0, 128, 0));
    itemBoxSizer22->Add(itemButton24, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    wxButton* itemButton25 = new wxButton( itemDialog1, INSERT_EQU, _("Ins"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton25->SetForegroundColour(wxColour(0, 0, 255));
    itemBoxSizer22->Add(itemButton25, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer26 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer21->Add(itemBoxSizer26, 0, wxGROW|wxTOP, 5);

    wxStaticText* itemStaticText27 = new wxStaticText( itemDialog1, wxID_STATIC, _("Equiv"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText27->SetForegroundColour(wxColour(204, 0, 0));
    itemBoxSizer26->Add(itemStaticText27, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    wxString* m_ListEquivStrings = NULL;
    m_ListEquiv = new wxListBox( itemDialog1, ID_LISTBOX1, wxDefaultPosition, wxSize(-1, 200), 0, m_ListEquivStrings, wxLB_SINGLE );
    itemBoxSizer26->Add(m_ListEquiv, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    m_FileExtBoxSizerH = new wxBoxSizer(wxHORIZONTAL);
    m_RightBoxSizer->Add(m_FileExtBoxSizerH, 0, wxGROW, 5);

    m_NetExtBoxSizer = new wxBoxSizer(wxVERTICAL);
    m_FileExtBoxSizerH->Add(m_NetExtBoxSizer, 0, wxGROW|wxRIGHT, 5);

    m_PkgExtBoxSizer = new wxBoxSizer(wxVERTICAL);
    m_FileExtBoxSizerH->Add(m_PkgExtBoxSizer, 0, wxGROW|wxLEFT|wxRIGHT, 5);

////@end KiConfigCvpcbFrame content construction

	SetDialogDatas();
}

/*!
 * Should we show tooltips?
 */

bool KiConfigCvpcbFrame::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap KiConfigCvpcbFrame::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin KiConfigCvpcbFrame bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end KiConfigCvpcbFrame bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon KiConfigCvpcbFrame::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin KiConfigCvpcbFrame icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end KiConfigCvpcbFrame icon retrieval
}
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for SAVE_CFG
 */

void KiConfigCvpcbFrame::OnSaveCfgClick( wxCommandEvent& event )
{
	SaveCfg(event);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_READ_OLDCFG
 */

void KiConfigCvpcbFrame::OnReadOldcfgClick( wxCommandEvent& event )
{
	ReadOldCfg(event);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void KiConfigCvpcbFrame::OnOkClick( wxCommandEvent& event )
{
	AcceptCfg(event);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for DEL_LIB
 */

void KiConfigCvpcbFrame::OnDelLibClick( wxCommandEvent& event )
{
	LibDelFct(event);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ADD_LIB
 */

void KiConfigCvpcbFrame::OnAddLibClick( wxCommandEvent& event )
{
	LibAddFct(event);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for INSERT_LIB
 */

void KiConfigCvpcbFrame::OnInsertLibClick( wxCommandEvent& event )
{
	LibAddFct(event);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for DEL_EQU
 */

void KiConfigCvpcbFrame::OnDelEquClick( wxCommandEvent& event )
{
	EquDelFct(event);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ADD_EQU
 */

void KiConfigCvpcbFrame::OnAddEquClick( wxCommandEvent& event )
{
	EquAddFct(event);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for INSERT_EQU
 */

void KiConfigCvpcbFrame::OnInsertEquClick( wxCommandEvent& event )
{
	EquAddFct(event);
}


/*!
 * wxEVT_COMMAND_RADIOBOX_SELECTED event handler for FORMAT_NETLIST
 */

void KiConfigCvpcbFrame::OnFormatNetlistSelected( wxCommandEvent& event )
{
	ReturnNetFormat(event);
}


