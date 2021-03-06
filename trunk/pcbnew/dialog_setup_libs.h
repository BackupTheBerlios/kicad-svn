/////////////////////////////////////////////////////////////////////////////
// Name:        dialog_pcbnew_config.h
// Purpose:     
// Author:      jean-pierre Charras
// Modified by: 
// Created:     18/02/2006 16:41:57
// RCS-ID:      
// Copyright:   License GNU
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 18/02/2006 16:41:57

#ifndef _DIALOG_PCBNEW_CONFIG_H_
#define _DIALOG_PCBNEW_CONFIG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dialog_pcbnew_config.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/valtext.h"
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
#define SYMBOL_KICONFIGPCBNEWFRAME_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_KICONFIGPCBNEWFRAME_TITLE _("Dialog")
#define SYMBOL_KICONFIGPCBNEWFRAME_IDNAME ID_DIALOG
#define SYMBOL_KICONFIGPCBNEWFRAME_SIZE wxSize(400, 300)
#define SYMBOL_KICONFIGPCBNEWFRAME_POSITION wxDefaultPosition
#define SAVE_CFG 10001
#define DEL_LIB 10002
#define ADD_LIB 10003
#define INSERT_LIB 10004
#define ID_LIST_LIBS 10005
#define ID_TEXTCTRL 10007
#define ID_TEXTCTRL1 10008
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * KiConfigPcbnewFrame class declaration
 */

class KiConfigPcbnewFrame: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( KiConfigPcbnewFrame )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    KiConfigPcbnewFrame( );
    KiConfigPcbnewFrame( WinEDA_PcbFrame* parent, wxWindowID id = SYMBOL_KICONFIGPCBNEWFRAME_IDNAME, const wxString& caption = SYMBOL_KICONFIGPCBNEWFRAME_TITLE, const wxPoint& pos = SYMBOL_KICONFIGPCBNEWFRAME_POSITION, const wxSize& size = SYMBOL_KICONFIGPCBNEWFRAME_SIZE, long style = SYMBOL_KICONFIGPCBNEWFRAME_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_KICONFIGPCBNEWFRAME_IDNAME, const wxString& caption = SYMBOL_KICONFIGPCBNEWFRAME_TITLE, const wxPoint& pos = SYMBOL_KICONFIGPCBNEWFRAME_POSITION, const wxSize& size = SYMBOL_KICONFIGPCBNEWFRAME_SIZE, long style = SYMBOL_KICONFIGPCBNEWFRAME_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin KiConfigPcbnewFrame event handler declarations

    /// wxEVT_CLOSE_WINDOW event handler for ID_DIALOG
    void OnCloseWindow( wxCloseEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for SAVE_CFG
    void OnSaveCfgClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for DEL_LIB
    void OnDelLibClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ADD_LIB
    void OnAddLibClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for INSERT_LIB
    void OnInsertLibClick( wxCommandEvent& event );

////@end KiConfigPcbnewFrame event handler declarations

////@begin KiConfigPcbnewFrame member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end KiConfigPcbnewFrame member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

	void SaveCfg(wxCommandEvent& event);
	void LibDelFct(wxCommandEvent& event);
	void LibInsertFct(wxCommandEvent& event);
	void SetNewOptions(void);

////@begin KiConfigPcbnewFrame member variables
    wxStaticBoxSizer* m_FileExtList;
    wxListBox* m_ListLibr;
    wxTextCtrl* m_TextLibDir;
    wxTextCtrl* m_TextHelpModulesFileName;
////@end KiConfigPcbnewFrame member variables

	WinEDA_PcbFrame * m_Parent;
	bool m_LibModified;

};

#endif
    // _DIALOG_PCBNEW_CONFIG_H_
