	/*******************/
	/* File: cvpcb.cpp */
	/*******************/
#define MAIN
#define eda_global

#include "fctsys.h"
#include "common.h"

#include "cvpcb.h"
#include "trigo.h"
#include "gr_basic.h"

#include "bitmaps.h"

#include "protos.h"

#include "id.h"

wxString Main_Title = "Cvpcb 16-sep-05";

// Create a new application object
IMPLEMENT_APP(WinEDA_App)

/* fonctions locales */

	/************************************/
	/* Called to initialize the program */
	/************************************/

bool WinEDA_App::OnInit(void)
{
wxString msg;
wxString currCWD = wxGetCwd();

	EDA_Appl = this;
	InitEDA_Appl("cvpcb");

    if ( m_Checker && m_Checker->IsAnotherRunning() ) 
    { 
        if ( ! IsOK(NULL, _("Cvpcb is already running, Continue?") ) )
			return false; 
    }
	
	GetSettings();					// read current setup

	wxSetWorkingDirectory(currCWD); // mofifie par GetSetting
	SetRealLibraryPath("modules");

	if(argc > 1 )
		{
		NetInNameBuffer = argv[1];
		NetNameBuffer = argv[1];
		}

	if (NetInNameBuffer != "" )
		wxSetWorkingDirectory( wxPathOnly(NetInNameBuffer) );
	DrawBgColor = BLACK;

	Read_Config(NetInNameBuffer);

	m_CvpcbFrame = new WinEDA_CvpcbFrame(this, Main_Title);

	msg.Printf( "Modules: %d", nblib);
	m_CvpcbFrame->SetStatusText(msg,2);

	// Show the frame
	SetTopWindow(m_CvpcbFrame);

	m_CvpcbFrame->Show(TRUE);

	listlib();
	m_CvpcbFrame->BuildModListBox();

	if( NetInNameBuffer != "" ) /* nom de fichier passe a la commande */
		{
		FFileName = MakeFileName(NetDirBuffer,
							NetInNameBuffer, NetInExtBuffer);

		m_CvpcbFrame->ReadNetListe();
		}
	else		/* Mise a jour du titre de la fenetre principale */
		{
 		msg.Printf( "%s {%s%c} [no file]",
			Main_Title.GetData(), wxGetCwd().GetData(), DIR_SEP);
		m_CvpcbFrame->SetTitle(msg);
		}

  return TRUE;
}

