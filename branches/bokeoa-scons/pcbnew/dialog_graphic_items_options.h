/////////////////////////////////////////////////////////////////////////////
// Name:        dialog_graphic_items_options.h
// Purpose:     
// Author:      jean-pierre Charras
// Modified by: 
// Created:     25/02/2006 09:53:27
// RCS-ID:      
// Copyright:   License GNU
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 25/02/2006 09:53:27

#ifndef _DIALOG_GRAPHIC_ITEMS_OPTIONS_H_
#define _DIALOG_GRAPHIC_ITEMS_OPTIONS_H_

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
#define SYMBOL_WINEDA_GRAPHICITEMSOPTIONSDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_WINEDA_GRAPHICITEMSOPTIONSDIALOG_TITLE _("Texts and Drawings")
#define SYMBOL_WINEDA_GRAPHICITEMSOPTIONSDIALOG_IDNAME ID_DIALOG
#define SYMBOL_WINEDA_GRAPHICITEMSOPTIONSDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_WINEDA_GRAPHICITEMSOPTIONSDIALOG_POSITION wxDefaultPosition
#define ID_TEXTCTRL_SEGW 10001
#define ID_TEXTCTRL_EDGES 10002
#define ID_TEXTCTRL_TEXTW 10003
#define ID_TEXTCTRL_TEXTV 10004
#define ID_TEXTCTRL_TEXTH 10005
#define ID_TEXTCTRL_EDGEMOD_W 10006
#define ID_TEXTCTRL_TXTMOD_W 10007
#define ID_TEXTCTRL_TXTMOD_V 10008
#define ID_TEXTCTRL_TXTMOD_H 10009
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * WinEDA_GraphicItemsOptionsDialog class declaration
 */

class WinEDA_GraphicItemsOptionsDialog: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( WinEDA_GraphicItemsOptionsDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WinEDA_GraphicItemsOptionsDialog( );
    WinEDA_GraphicItemsOptionsDialog( WinEDA_BasePcbFrame* parent, wxWindowID id = SYMBOL_WINEDA_GRAPHICITEMSOPTIONSDIALOG_IDNAME, const wxString& caption = SYMBOL_WINEDA_GRAPHICITEMSOPTIONSDIALOG_TITLE, const wxPoint& pos = SYMBOL_WINEDA_GRAPHICITEMSOPTIONSDIALOG_POSITION, const wxSize& size = SYMBOL_WINEDA_GRAPHICITEMSOPTIONSDIALOG_SIZE, long style = SYMBOL_WINEDA_GRAPHICITEMSOPTIONSDIALOG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_WINEDA_GRAPHICITEMSOPTIONSDIALOG_IDNAME, const wxString& caption = SYMBOL_WINEDA_GRAPHICITEMSOPTIONSDIALOG_TITLE, const wxPoint& pos = SYMBOL_WINEDA_GRAPHICITEMSOPTIONSDIALOG_POSITION, const wxSize& size = SYMBOL_WINEDA_GRAPHICITEMSOPTIONSDIALOG_SIZE, long style = SYMBOL_WINEDA_GRAPHICITEMSOPTIONSDIALOG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin WinEDA_GraphicItemsOptionsDialog event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOkClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
    void OnCancelClick( wxCommandEvent& event );

////@end WinEDA_GraphicItemsOptionsDialog event handler declarations

////@begin WinEDA_GraphicItemsOptionsDialog member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end WinEDA_GraphicItemsOptionsDialog member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
	void AcceptOptions(wxCommandEvent& event);
	void SetDisplayValue( void );

////@begin WinEDA_GraphicItemsOptionsDialog member variables
    wxStaticText* m_GraphicSegmWidthTitle;
    wxTextCtrl* m_OptPcbSegmWidth;
    wxStaticText* m_BoardEdgesWidthTitle;
    wxTextCtrl* m_OptPcbEdgesWidth;
    wxStaticText* m_CopperTextWidthTitle;
    wxTextCtrl* m_OptPcbTextWidth;
    wxStaticText* m_TextSizeVTitle;
    wxTextCtrl* m_OptPcbTextVSize;
    wxStaticText* m_TextSizeHTitle;
    wxTextCtrl* m_OptPcbTextHSize;
    wxStaticText* m_EdgeModWidthTitle;
    wxTextCtrl* m_OptModuleEdgesWidth;
    wxStaticText* m_TextModWidthTitle;
    wxTextCtrl* m_OptModuleTextWidth;
    wxStaticText* m_TextModSizeVTitle;
    wxTextCtrl* m_OptModuleTextVSize;
    wxStaticText* m_TextModSizeHTitle;
    wxTextCtrl* m_OptModuleTextHSize;
////@end WinEDA_GraphicItemsOptionsDialog member variables
	WinEDA_BasePcbFrame * m_Parent;
};

#endif
    // _DIALOG_GRAPHIC_ITEMS_OPTIONS_H_