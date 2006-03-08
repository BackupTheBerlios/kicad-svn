/////////////////////////////////////////////////////////////////////////////
// Name:        dialog_display_options.cpp
// Purpose:     
// Author:      jean-pierre Charras
// Modified by: 
// Created:     17/02/2006 17:47:55
// RCS-ID:      
// Copyright:   License GNU
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 17/02/2006 17:47:55

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dialog_display_options.h"
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

#include "wxstruct.h"
#include "common.h"
#include "cvpcb.h"
#include "protos.h"

#include "dialog_display_options.h"

////@begin XPM images
////@end XPM images


/*********************************************************************/
void WinEDA_DisplayFrame::InstallOptionsDisplay(wxCommandEvent& event)
/*********************************************************************/
/* Creation de la fenetre d'options de la fenetre de visu */
{
	KiDisplayOptionsFrame * OptionWindow = new KiDisplayOptionsFrame(this);
	OptionWindow->ShowModal(); OptionWindow->Destroy();
}



/*!
 * KiDisplayOptionsFrame type definition
 */

IMPLEMENT_DYNAMIC_CLASS( KiDisplayOptionsFrame, wxDialog )

/*!
 * KiDisplayOptionsFrame event table definition
 */

BEGIN_EVENT_TABLE( KiDisplayOptionsFrame, wxDialog )

////@begin KiDisplayOptionsFrame event table entries
    EVT_CHECKBOX( PADNUM_OPT, KiDisplayOptionsFrame::OnPadnumOptClick )

    EVT_BUTTON( ID_SAVE_CONFIG, KiDisplayOptionsFrame::OnSaveConfigClick )

    EVT_CHECKBOX( PADFILL_OPT, KiDisplayOptionsFrame::OnPadfillOptClick )

    EVT_RADIOBOX( EDGE_SELECT, KiDisplayOptionsFrame::OnEdgeSelectSelected )

    EVT_RADIOBOX( TEXT_SELECT, KiDisplayOptionsFrame::OnTextSelectSelected )

////@end KiDisplayOptionsFrame event table entries

END_EVENT_TABLE()

/*!
 * KiDisplayOptionsFrame constructors
 */

KiDisplayOptionsFrame::KiDisplayOptionsFrame( )
{
}

KiDisplayOptionsFrame::KiDisplayOptionsFrame( WinEDA_BasePcbFrame* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
	m_Parent = parent;

    Create(parent, id, caption, pos, size, style);
}

/*!
 * KiDisplayOptionsFrame creator
 */

bool KiDisplayOptionsFrame::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin KiDisplayOptionsFrame member initialisation
    m_IsShowPadNum = NULL;
    m_IsShowPadFill = NULL;
    m_EdgesDisplayOption = NULL;
    m_TextDisplayOption = NULL;
////@end KiDisplayOptionsFrame member initialisation

////@begin KiDisplayOptionsFrame creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end KiDisplayOptionsFrame creation
    return true;
}

/*!
 * Control creation for KiDisplayOptionsFrame
 */

void KiDisplayOptionsFrame::CreateControls()
{    
	SetFont(*g_DialogFont);

////@begin KiDisplayOptionsFrame content construction
    // Generated by DialogBlocks, 17/02/2006 18:31:55 (unregistered)

    KiDisplayOptionsFrame* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(3, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_IsShowPadNum = new wxCheckBox( itemDialog1, PADNUM_OPT, _("Pad &Num"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_IsShowPadNum->SetValue(false);
    itemFlexGridSizer3->Add(m_IsShowPadNum, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, ID_SAVE_CONFIG, _("Save Cfg"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemButton5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_IsShowPadFill = new wxCheckBox( itemDialog1, PADFILL_OPT, _("&Pad Fill"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_IsShowPadFill->SetValue(false);
    itemFlexGridSizer3->Add(m_IsShowPadFill, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    wxString m_EdgesDisplayOptionStrings[] = {
        _("&Filaire"),
        _("&Filled"),
        _("&Sketch")
    };
    m_EdgesDisplayOption = new wxRadioBox( itemDialog1, EDGE_SELECT, _("Edges:"), wxDefaultPosition, wxDefaultSize, 3, m_EdgesDisplayOptionStrings, 1, wxRA_SPECIFY_COLS );
    itemFlexGridSizer3->Add(m_EdgesDisplayOption, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxString m_TextDisplayOptionStrings[] = {
        _("&Filaire"),
        _("&Filled"),
        _("&Sketch")
    };
    m_TextDisplayOption = new wxRadioBox( itemDialog1, TEXT_SELECT, _("Texts:"), wxDefaultPosition, wxDefaultSize, 3, m_TextDisplayOptionStrings, 1, wxRA_SPECIFY_COLS );
    itemFlexGridSizer3->Add(m_TextDisplayOption, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_IsShowPadNum->SetValidator( wxGenericValidator(& DisplayOpt.DisplayPadNum) );
    m_IsShowPadFill->SetValidator( wxGenericValidator(& DisplayOpt.DisplayPadFill) );
    m_EdgesDisplayOption->SetValidator( wxGenericValidator(& DisplayOpt.DisplayModEdge) );
    m_TextDisplayOption->SetValidator( wxGenericValidator(& DisplayOpt.DisplayModText) );
////@end KiDisplayOptionsFrame content construction
}

/*!
 * Should we show tooltips?
 */

bool KiDisplayOptionsFrame::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap KiDisplayOptionsFrame::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin KiDisplayOptionsFrame bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end KiDisplayOptionsFrame bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon KiDisplayOptionsFrame::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin KiDisplayOptionsFrame icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end KiDisplayOptionsFrame icon retrieval
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for PADFILL_OPT
 */

void KiDisplayOptionsFrame::OnPadfillOptClick( wxCommandEvent& event )
{
	DisplayOpt.DisplayPadFill = m_Parent->m_DisplayPadFill =
		m_IsShowPadFill->GetValue();
	m_Parent->ReDrawPanel();
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for PADNUM_OPT
 */

void KiDisplayOptionsFrame::OnPadnumOptClick( wxCommandEvent& event )
{
    DisplayOpt.DisplayPadNum = m_Parent->m_DisplayPadNum = m_IsShowPadNum->GetValue();
	m_Parent->ReDrawPanel();
}

/*!
 * wxEVT_COMMAND_RADIOBOX_SELECTED event handler for EDGE_SELECT
 */

void KiDisplayOptionsFrame::OnEdgeSelectSelected( wxCommandEvent& event )
{
	DisplayOpt.DisplayModEdge = m_Parent->m_DisplayModEdge =
		m_EdgesDisplayOption->GetSelection();
	m_Parent->ReDrawPanel();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SAVE_CONFIG
 */

void KiDisplayOptionsFrame::OnSaveConfigClick( wxCommandEvent& event )
{
	Save_Config(this);
}

/*!
 * wxEVT_COMMAND_RADIOBOX_SELECTED event handler for TEXT_SELECT
 */

void KiDisplayOptionsFrame::OnTextSelectSelected( wxCommandEvent& event )
{
	DisplayOpt.DisplayModText = m_Parent->m_DisplayModText = 
		m_TextDisplayOption->GetSelection();
	m_Parent->ReDrawPanel();
}

