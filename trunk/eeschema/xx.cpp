/////////////////////////////////////////////////////////////////////////////
// Name:        xx.cpp
// Purpose:     
// Author:      jean-pierre Charras
// Modified by: 
// Created:     02/03/2006 09:16:35
// RCS-ID:      
// Copyright:   License GNU
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 02/03/2006 09:16:35

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "xx.h"
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

#include "xx.h"

////@begin XPM images

////@end XPM images

/*!
 * xx type definition
 */

IMPLEMENT_DYNAMIC_CLASS( xx, wxDialog )

/*!
 * xx event table definition
 */

BEGIN_EVENT_TABLE( xx, wxDialog )

////@begin xx event table entries
////@end xx event table entries

END_EVENT_TABLE()

/*!
 * xx constructors
 */

xx::xx( )
{
}

xx::xx( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, id, caption, pos, size, style);
}

/*!
 * xx creator
 */

bool xx::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin xx member initialisation
////@end xx member initialisation

////@begin xx creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end xx creation
    return true;
}

/*!
 * Control creation for xx
 */

void xx::CreateControls()
{    
////@begin xx content construction
    // Generated by DialogBlocks, 02/03/2006 09:16:35 (unregistered)

    xx* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

////@end xx content construction
}

/*!
 * Should we show tooltips?
 */

bool xx::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap xx::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin xx bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end xx bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon xx::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin xx icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end xx icon retrieval
}