	/******************************************/
	/** cfg.cpp : configuration pour Gerbview */
	/******************************************/

/* lit ou met a jour la configuration de PCBNEW */

#include "fctsys.h"

#include "common.h"
#include "gerbview.h"
#include "pcbplot.h"
#include "cfg.h"
#include "id.h"

#include "protos.h"

/* Routines Locales */

/* Variables locales */

/*************************************************************/
void WinEDA_GerberFrame::Process_Config(wxCommandEvent& event)
/*************************************************************/
{
int id = event.GetId();
wxPoint pos;

	pos = GetPosition();
	pos.x += 20; pos.y += 20;

	switch( id )
		{
		case ID_COLORS_SETUP :
			DisplayColorSetupFrame(this, pos);
			break;

		case ID_CONFIG_REQ :		// Creation de la fenetre de configuration
			{
			InstallConfigFrame(pos);
			break;
			}

		case ID_PCB_TRACK_SIZE_SETUP:
		case ID_PCB_LOOK_SETUP:
		case ID_OPTIONS_SETUP:
			InstallPcbOptionsFrame(pos, id);
			break;

		case ID_CONFIG_SAVE:
			Update_config();
			break;

		default:
			DisplayError(this,"WinEDA_GerberFrame::Process_Config internal error");
		}
}


/*****************************************************/
bool Read_Config(void)
/*****************************************************/
/* lit la configuration, si elle n'a pas deja etee lue
	1 - lit gerbview.cnf
	2 - si non trouve lit <chemin de gerbview.exe>/gerbview.cnf
	3 - si non trouve: init des variables aux valeurs par defaut

	Retourne un pointeur su le message d'erreur a afficher
*/
{
	g_Prj_Config_Filename_ext = ".cnf";
	EDA_Appl->ReadProjectConfig("gerbview", GROUP, ParamCfgList, FALSE);

	/* Inits autres variables */
	if (ScreenPcb) ScreenPcb->SetGrid(TmpGrid);
	if ( g_PhotoFilenameExt == "") g_PhotoFilenameExt = ".pho";
	if ( g_DrillFilenameExt == "") g_DrillFilenameExt = ".drl";
	if ( g_PenFilenameExt == "") g_PenFilenameExt = ".pen";

	return TRUE;
}



/******************************************/
void WinEDA_GerberFrame::Update_config(void)
/******************************************/
/*
 creation du fichier de config
*/
{
wxString FullFileName;
wxString mask("*"),
	
	g_Prj_Config_Filename_ext = ".cnf";
	mask += g_Prj_Config_Filename_ext;
	FullFileName = "gerbview";
	ChangeFileNameExt( FullFileName, g_Prj_Config_Filename_ext );

	FullFileName = EDA_FileSelector(_("Save config file"),
					"",				/* Chemin par defaut */
					FullFileName,		/* nom fichier par defaut */
					g_Prj_Config_Filename_ext,	/* extension par defaut */
					mask,				/* Masque d'affichage */
					this,
					wxSAVE,
					TRUE
					);
	if ( FullFileName == "") return;

	/* ecriture de la configuration */
	EDA_Appl->WriteProjectConfig(FullFileName, GROUP, ParamCfgList);
}

