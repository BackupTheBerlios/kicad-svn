/////////////////////////////////////////////////////////////////////////////
// Name:        dialog_cvpcb_config.h
// Purpose:     
// Author:      jean-pierre Charras
// Modified by: 
// Created:     17/02/2006 18:43:13
// RCS-ID:      
// Copyright:   License GNU
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 17/02/2006 18:43:13

#ifndef _DIALOG_CVPCB_CONFIG_H_
#define _DIALOG_CVPCB_CONFIG_H_

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DIALOG 10000
#define SYMBOL_KICONFIGCVPCBFRAME_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_KICONFIGCVPCBFRAME_TITLE _("Dialog")
#define SYMBOL_KICONFIGCVPCBFRAME_IDNAME ID_DIALOG
#define SYMBOL_KICONFIGCVPCBFRAME_SIZE wxSize(400, 300)
#define SYMBOL_KICONFIGCVPCBFRAME_POSITION wxDefaultPosition
#define SAVE_CFG 10001
#define ID_READ_OLDCFG 10002
#define FORMAT_NETLIST 10003
#define DEL_LIB 10004
#define ADD_LIB 10005
#define INSERT_LIB 10006
#define ID_LISTBOX 10010
#define DEL_EQU 10007
#define ADD_EQU 10008
#define INSERT_EQU 10009
#define ID_LISTBOX1 10011
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * KiConfigCvpcbFrame class declaration
 */

class KiConfigCvpcbFrame: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( KiConfigCvpcbFrame )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    KiConfigCvpcbFrame( );
    KiConfigCvpcbFrame( WinEDA_CvpcbFrame* parent, wxWindowID id = SYMBOL_KICONFIGCVPCBFRAME_IDNAME, const wxString& caption = SYMBOL_KICONFIGCVPCBFRAME_TITLE, const wxPoint& pos = SYMBOL_KICONFIGCVPCBFRAME_POSITION, const wxSize& size = SYMBOL_KICONFIGCVPCBFRAME_SIZE, long style = SYMBOL_KICONFIGCVPCBFRAME_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_KICONFIGCVPCBFRAME_IDNAME, const wxString& caption = SYMBOL_KICONFIGCVPCBFRAME_TITLE, const wxPoint& pos = SYMBOL_KICONFIGCVPCBFRAME_POSITION, const wxSize& size = SYMBOL_KICONFIGCVPCBFRAME_SIZE, long style = SYMBOL_KICONFIGCVPCBFRAME_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin KiConfigCvpcbFrame event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for SAVE_CFG
    void OnSaveCfgClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_READ_OLDCFG
    void OnReadOldcfgClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOkClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBOX_SELECTED event handler for FORMAT_NETLIST
    void OnFormatNetlistSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for DEL_LIB
    void OnDelLibClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ADD_LIB
    void OnAddLibClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for INSERT_LIB
    void OnInsertLibClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for DEL_EQU
    void OnDelEquClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ADD_EQU
    void OnAddEquClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for INSERT_EQU
    void OnInsertEquClick( wxCommandEvent& event );

////@end KiConfigCvpcbFrame event handler declarations

////@begin KiConfigCvpcbFrame member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end KiConfigCvpcbFrame member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

	void Update(void);
	void AcceptCfg(wxCommandEvent& event);
	void SaveCfg(wxCommandEvent& event);
	void ReadOldCfg(wxCommandEvent& event);
	void LibDelFct(wxCommandEvent& event);
	void LibAddFct(wxCommandEvent& event);
	void EquDelFct(wxCommandEvent& event);
	void EquAddFct(wxCommandEvent& event);
	void ReturnNetFormat(wxCommandEvent& event);
	void SetDialogDatas(void);


////@begin KiConfigCvpcbFrame member variables
    wxRadioBox* m_NetFormatBox;
    wxStaticBoxSizer* m_FileExtList;
    wxBoxSizer* m_RightBoxSizer;
    wxListBox* m_ListLibr;
    wxListBox* m_ListEquiv;
    wxBoxSizer* m_FileExtBoxSizerH;
    wxBoxSizer* m_NetExtBoxSizer;
    wxBoxSizer* m_PkgExtBoxSizer;
////@end KiConfigCvpcbFrame member variables

	WinEDA_CvpcbFrame * m_Parent;
	bool m_DoUpdate;

	WinEDA_EnterText * m_LibDirCtrl;
	WinEDA_EnterText * m_NetInputExtCtrl;
	WinEDA_EnterText * m_PkgExtCtrl;
	WinEDA_EnterText * m_TextHelpModulesFileName;
};

#endif
    // _DIALOG_CVPCB_CONFIG_H_
