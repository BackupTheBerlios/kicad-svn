	/******************************************************************/
	/* mdiframe.cpp - fonctions de la classe du type WinEDA_MainFrame */
	/******************************************************************/

#ifdef __GNUG__
#pragma implementation
#endif

#include "fctsys.h"

#include "common.h"

#include "bitmaps.h"
#include "protos.h"

#include "id.h"

#include "kicad.h"

#include <wx/fontdlg.h>

/****************************************************************/
void WinEDA_MainFrame::Process_Preferences(wxCommandEvent& event)
/*****************************************************************/
{
int id = event.GetId();

	switch (id)
	{
		case ID_SELECT_PREFERED_EDITOR:
		{
			wxString FullFileName;
			wxString mask(wxT("*"));
#ifdef __WINDOWS__
			mask += wxT(".exe");
#endif
			FullFileName = EDA_FileSelector( _("Prefered Editor:"),
					wxPathOnly(g_EditorName),	/* Default path */
					g_EditorName,			/* default filename */
					wxEmptyString,			/* default filename extension */
					mask,					/* filter for filename list */
					this,					/* parent frame */
					wxOPEN,					/* wxSAVE, wxOPEN ..*/
					TRUE					/* true = keep the current path */
					);
			if ( ( !FullFileName.IsEmpty() ) && EDA_Appl->m_EDA_CommonConfig)
			{
				g_EditorName = FullFileName;
				EDA_Appl->m_EDA_CommonConfig->Write(wxT("Editor"), g_EditorName);
			}
		}

		case ID_PREFERENCES_FONT_INFOSCREEN:
		{
			wxFont font = wxGetFontFromUser(this, *g_StdFont);
			if ( font.Ok() )
			{
				int pointsize = font.GetPointSize();
				*g_StdFont = font;
				g_StdFontPointSize = pointsize;
				g_DialogFontPointSize = pointsize;
				g_FixedFontPointSize = pointsize;
				m_LeftWin->ReCreateTreePrj();
				m_DialogWin->SetFont(* g_StdFont);
				m_DialogWin->Refresh();
			}
			break;
		}

		default: DisplayError(this, wxT("WinEDA_MainFrame::Process_Preferences Internal Error") );
			break;
	}
}

/********************************************************/
void WinEDA_MainFrame::SetLanguage(wxCommandEvent& event)
/********************************************************/
{
int id = event.GetId();

	m_Parent->SetLanguageIdentifier(id );
	m_Parent->SetLanguage();
}

