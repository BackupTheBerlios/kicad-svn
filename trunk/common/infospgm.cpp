		/****************************************************/
		/* Affichage du panneau d'information (copyright..) */
		/* Commun a CVPCB, EESCHEMA et PCBNEW				*/
		/****************************************************/

#include "fctsys.h"
#include "gr_basic.h"
#include "common.h"


// Import:
extern wxString Main_Title;

wxString MsgInfos(
#ifdef GERBVIEW
"** GERBVIEW  (jul 2001 .. 2005) **"
#else
#ifdef PCBNEW
"** PCBNEW  (sept 1992 .. 2005) **"
#endif
#endif

#ifdef CVPCB
"** CVPCB  (sept 1992 .. 2005) **"
#endif

#ifdef KICAD
"** KICAD (jul 2000 .. 2005) **"
#endif

#ifdef EESCHEMA
"** EESCHEMA  (sept 1994 .. 2005) **"
#endif
);

/* Routines Locales */

/******************************************************/
void Affiche_InfosLicence(wxWindow * frame)
/******************************************************/
{
wxString Msg = MsgInfos;
	Msg << "\n\n" << _("Build Version:") << "\n" ;

	Msg << Main_Title << "\n\n" << _("Author:");
	Msg << "JP CHARRAS\n\n" << _("Based on wxWidgets");
	Msg << _("\n\nGPL License");
	Msg << _("\n\nWeb sites:\n");
	Msg << "http://iut-tice.ujf-grenoble.fr/kicad/\n";
	Msg << "http://www.lis.inpg.fr/realise_au_lis/kicad/";

	wxMessageBox(Msg, "", wxICON_INFORMATION, frame);
}

