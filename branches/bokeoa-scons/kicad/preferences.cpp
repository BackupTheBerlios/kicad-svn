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
		case ID_PREFERENCES_FONTS1:
		{
			wxFont font = wxGetFontFromUser(this, *g_StdFont);
			if ( font.Ok() )
			{
				int pointsize = font.GetPointSize();
				*g_StdFont = font;
				g_StdFontPointSize = pointsize;
				g_SmallFontPointSize = (pointsize * 70) / 100;
				g_DialogFontPointSize = pointsize;
				g_FixedFontPointSize = pointsize;
				m_LeftWin->ReCreateTreePrj();
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

