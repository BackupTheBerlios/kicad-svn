	/*************************************************************/
	/** prjconfig.cpp : load and save configuration (file *.pro) */
	/*************************************************************/
	

#include "fctsys.h"
#include "common.h"
#include "kicad.h"
#include "protos.h"
#include "prjconfig.h"

/* Variables locales */


/*******************************************/
void WinEDA_MainFrame::Load_Prj_Config(void)
/*******************************************/
{
FILE * in_file;

	in_file = fopen(m_PrjFileName, "rt");
	if ( in_file == 0 )
		{
		wxString msg = _("Project File <") + m_PrjFileName + _("> not found");
		DisplayError(this, msg);
		return;
		}
	fclose(in_file);

	wxSetWorkingDirectory(wxPathOnly(m_PrjFileName) );
	SetTitle(Main_Title + " " + m_PrjFileName);
	ReCreateMenuBar();
	m_LeftWin->ReCreateTreePrj();

	wxString msg = _("\nWorking dir: ") + wxGetCwd();
	msg << _("\nProject: ") << m_PrjFileName << "\n";
	PrintMsg(msg);
}


/*********************************************/
void WinEDA_MainFrame::Save_Prj_Config(void)
/*********************************************/
{
wxString FullFileName;
wxString mask("*");
	
	g_Prj_Config_Filename_ext = ".pro";
	mask += g_Prj_Config_Filename_ext;
	FullFileName = m_PrjFileName;
	ChangeFileNameExt( FullFileName, g_Prj_Config_Filename_ext);

	FullFileName = EDA_FileSelector(_("Save config file"),
					wxGetCwd(),			/* Chemin par defaut */
					FullFileName,		/* nom fichier par defaut */
					g_Prj_Config_Filename_ext,	/* extension par defaut */
					mask,				/* Masque d'affichage */
					this,
					wxSAVE,
					TRUE
					);
	if ( FullFileName == "") return;

	/* ecriture de la configuration */
	EDA_Appl->WriteProjectConfig(FullFileName, "/general", CfgParamList);
}

