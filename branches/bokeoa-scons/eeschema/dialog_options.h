/////////////////////////////////////////////////////////////////////////////
// Name:        dilaog_options.h
// Purpose:     
// Author:      jean-pierre Charras
// Modified by: 
// Created:     31/01/2006 13:27:33
// RCS-ID:      
// Copyright:   GNU License
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 31/01/2006 13:27:33

#ifndef _DILAOG_OPTIONS_H_
#define _DILAOG_OPTIONS_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dilaog_options.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/spinctrl.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxSpinCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DIALOG 10000
#define SYMBOL_WINEDA_SETOPTIONSFRAME_STYLE wxDEFAULT_DIALOG_STYLE|wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxSTAY_ON_TOP|wxCLOSE_BOX
#define SYMBOL_WINEDA_SETOPTIONSFRAME_TITLE _("General Options")
#define SYMBOL_WINEDA_SETOPTIONSFRAME_IDNAME ID_DIALOG
#define SYMBOL_WINEDA_SETOPTIONSFRAME_SIZE wxSize(400, 300)
#define SYMBOL_WINEDA_SETOPTIONSFRAME_POSITION wxDefaultPosition
#define ID_CHECKBOX1 10001
#define ID_RADIOBOX 10003
#define ID_RADIOBOX1 10004
#define ID_CHECKBOX 10002
#define ID_RADIOBOX2 10005
#define ID_RADIOBOX3 10006
#define ID_RADIOBOX4 10007
#define ID_TEXTCTRL 10008
#define ID_TEXTCTRL1 10009
#define ID_SPINCTRL 10010
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * WinEDA_SetOptionsFrame class declaration
 */

class WinEDA_SetOptionsFrame: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( WinEDA_SetOptionsFrame )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WinEDA_SetOptionsFrame( );
    WinEDA_SetOptionsFrame( WinEDA_DrawFrame* parent, wxWindowID id = SYMBOL_WINEDA_SETOPTIONSFRAME_IDNAME, const wxString& caption = SYMBOL_WINEDA_SETOPTIONSFRAME_TITLE, const wxPoint& pos = SYMBOL_WINEDA_SETOPTIONSFRAME_POSITION, const wxSize& size = SYMBOL_WINEDA_SETOPTIONSFRAME_SIZE, long style = SYMBOL_WINEDA_SETOPTIONSFRAME_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_WINEDA_SETOPTIONSFRAME_IDNAME, const wxString& caption = SYMBOL_WINEDA_SETOPTIONSFRAME_TITLE, const wxPoint& pos = SYMBOL_WINEDA_SETOPTIONSFRAME_POSITION, const wxSize& size = SYMBOL_WINEDA_SETOPTIONSFRAME_SIZE, long style = SYMBOL_WINEDA_SETOPTIONSFRAME_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin WinEDA_SetOptionsFrame event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOkClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
    void OnCancelClick( wxCommandEvent& event );

////@end WinEDA_SetOptionsFrame event handler declarations

////@begin WinEDA_SetOptionsFrame member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end WinEDA_SetOptionsFrame member function declarations

	void Accept(wxCommandEvent& event);

    /// Should we show tooltips?
    static bool ShowToolTips();

	WinEDA_DrawFrame * m_Parent;
    
////@begin WinEDA_SetOptionsFrame member variables
    wxCheckBox* m_ShowGridOpt;
    wxRadioBox* m_SelGridSize;
    wxRadioBox* m_SelShowPins;
    wxCheckBox* m_AutoPANOpt;
    wxRadioBox* m_Selunits;
    wxRadioBox* m_SelDirWires;
    wxRadioBox* m_Show_Page_Limits;
    wxTextCtrl* m_DeltaStepCtrl_X;
    wxTextCtrl* m_DeltaStepCtrl_Y;
    wxSpinCtrl* m_DeltaLabelCtrl;
////@end WinEDA_SetOptionsFrame member variables
};

#endif
    // _DILAOG_OPTIONS_H_
