/////////////////////////////////////////////////////////////////////////////
// Name:        dialog_set_status.h
// Purpose:     
// Author:      jean-pierre Charras
// Modified by: 
// Created:     11/02/2006 21:42:23
// RCS-ID:      
// Copyright:   License GNU
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 11/02/2006 21:42:23

#ifndef _DIALOG_SET_STATUS_H_
#define _DIALOG_SET_STATUS_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dialog_set_status.h"
#endif

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DIALOG 10000
#define SYMBOL_WINEDA_CONFIGFRAME_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_WINEDA_CONFIGFRAME_TITLE _("from ")
#define SYMBOL_WINEDA_CONFIGFRAME_IDNAME ID_DIALOG
#define SYMBOL_WINEDA_CONFIGFRAME_SIZE wxSize(400, 300)
#define SYMBOL_WINEDA_CONFIGFRAME_POSITION wxDefaultPosition
#define ID_BUTTON 10001
#define ID_RADIOBOX 10006
#define ID_BUTTON1 10002
#define ID_BUTTON2 10003
#define ID_BUTTON3 10004
#define ID_LISTBOX 10005
#define ID_TEXTCTRL 10007
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * WinEDA_ConfigFrame class declaration
 */

class WinEDA_ConfigFrame: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( WinEDA_ConfigFrame )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WinEDA_ConfigFrame( );
    WinEDA_ConfigFrame( WinEDA_SchematicFrame* parent, wxWindowID id = SYMBOL_WINEDA_CONFIGFRAME_IDNAME, const wxString& caption = SYMBOL_WINEDA_CONFIGFRAME_TITLE, const wxPoint& pos = SYMBOL_WINEDA_CONFIGFRAME_POSITION, const wxSize& size = SYMBOL_WINEDA_CONFIGFRAME_SIZE, long style = SYMBOL_WINEDA_CONFIGFRAME_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_WINEDA_CONFIGFRAME_IDNAME, const wxString& caption = SYMBOL_WINEDA_CONFIGFRAME_TITLE, const wxPoint& pos = SYMBOL_WINEDA_CONFIGFRAME_POSITION, const wxSize& size = SYMBOL_WINEDA_CONFIGFRAME_SIZE, long style = SYMBOL_WINEDA_CONFIGFRAME_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin WinEDA_ConfigFrame event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
    void OnButtonClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
    void OnButton1Click( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
    void OnButton2Click( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3
    void OnButton3Click( wxCommandEvent& event );

////@end WinEDA_ConfigFrame event handler declarations

////@begin WinEDA_ConfigFrame member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end WinEDA_ConfigFrame member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin WinEDA_ConfigFrame member variables
    wxStaticText* m_CmpfileExtText;
    wxStaticText* m_NetfileExtText;
    wxStaticText* m_LibfileExtText;
    wxStaticText* m_SymbolfileExtText;
    wxStaticText* m_SchfileExtText;
    wxListBox* m_ListLibr;
    wxStaticText* m_CmpfileExtText;
    wxTextCtrl* m_LibDirCtrl;
////@end WinEDA_ConfigFrame member variables
};

#endif
    // _DIALOG_SET_STATUS_H_
