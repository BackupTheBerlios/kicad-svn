/////////////////////////////////////////////////////////////////////////////
// Name:        dialog_cmp_graphic_properties.h
// Purpose:     
// Author:      jean-pierre Charras
// Modified by: 
// Created:     12/02/2006 11:38:02
// RCS-ID:      
// Copyright:   License GNU
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 12/02/2006 11:38:02

#ifndef _DIALOG_CMP_GRAPHIC_PROPERTIES_H_
#define _DIALOG_CMP_GRAPHIC_PROPERTIES_H_


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
#define SYMBOL_WINEDA_BODYGRAPHICS_PROPERTIESFRAME_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_WINEDA_BODYGRAPHICS_PROPERTIESFRAME_TITLE _("Graphic shape properties")
#define SYMBOL_WINEDA_BODYGRAPHICS_PROPERTIESFRAME_IDNAME ID_DIALOG
#define SYMBOL_WINEDA_BODYGRAPHICS_PROPERTIESFRAME_SIZE wxSize(400, 300)
#define SYMBOL_WINEDA_BODYGRAPHICS_PROPERTIESFRAME_POSITION wxDefaultPosition
#define ID_CHECKBOX 10001
#define ID_CHECKBOX1 10002
#define ID_RADIOBOX 10003
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * WinEDA_bodygraphics_PropertiesFrame class declaration
 */

class WinEDA_bodygraphics_PropertiesFrame: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( WinEDA_bodygraphics_PropertiesFrame )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WinEDA_bodygraphics_PropertiesFrame( );
    WinEDA_bodygraphics_PropertiesFrame( WinEDA_LibeditFrame* parent, wxWindowID id = SYMBOL_WINEDA_BODYGRAPHICS_PROPERTIESFRAME_IDNAME, const wxString& caption = SYMBOL_WINEDA_BODYGRAPHICS_PROPERTIESFRAME_TITLE, const wxPoint& pos = SYMBOL_WINEDA_BODYGRAPHICS_PROPERTIESFRAME_POSITION, const wxSize& size = SYMBOL_WINEDA_BODYGRAPHICS_PROPERTIESFRAME_SIZE, long style = SYMBOL_WINEDA_BODYGRAPHICS_PROPERTIESFRAME_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_WINEDA_BODYGRAPHICS_PROPERTIESFRAME_IDNAME,
		const wxString& caption = SYMBOL_WINEDA_BODYGRAPHICS_PROPERTIESFRAME_TITLE,
		const wxPoint& pos = SYMBOL_WINEDA_BODYGRAPHICS_PROPERTIESFRAME_POSITION,
		const wxSize& size = SYMBOL_WINEDA_BODYGRAPHICS_PROPERTIESFRAME_SIZE,
		long style = SYMBOL_WINEDA_BODYGRAPHICS_PROPERTIESFRAME_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin WinEDA_bodygraphics_PropertiesFrame event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOkClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
    void OnCancelClick( wxCommandEvent& event );

////@end WinEDA_bodygraphics_PropertiesFrame event handler declarations

////@begin WinEDA_bodygraphics_PropertiesFrame member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end WinEDA_bodygraphics_PropertiesFrame member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
	void bodygraphics_PropertiesAccept(wxCommandEvent& event);

////@begin WinEDA_bodygraphics_PropertiesFrame member variables
    wxCheckBox* m_CommonUnit;
    wxCheckBox* m_CommonConvert;
    wxRadioBox* m_Filled;
////@end WinEDA_bodygraphics_PropertiesFrame member variables
	WinEDA_LibeditFrame * m_Parent;
};

#endif
    // _DIALOG_CMP_GRAPHIC_PROPERTIES_H_