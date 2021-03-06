	/******************************************************/
	/** eeconfig.cpp : routines et menus de configuration */
	/*******************************************************/
	
#include "fctsys.h"
#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"
#include "protos.h"
#include "eeconfig.h"
#include "worksheet.h"

#include "id.h"

/* Variables locales */


/*********************************************************************/
void WinEDA_SchematicFrame::Process_Config(wxCommandEvent& event)
/*********************************************************************/
{
int id = event.GetId();
wxPoint pos;

	wxGetMousePosition(&pos.x, &pos.y);

	pos.y += 5;
	switch ( id )
		{
		case ID_COLORS_SETUP :
			DisplayColorSetupFrame(this, pos);
			break;

		case ID_CONFIG_REQ :		// Creation de la fenetre de configuration
			{
			InstallConfigFrame(pos);
			break;
			}

		case ID_OPTIONS_SETUP:
			DisplayOptionFrame(this, pos);
			break;

		case ID_CONFIG_SAVE:
			Save_Config(this);
			break;

		case ID_CONFIG_READ:
			{
			wxString mask("*"); mask += g_Prj_Config_Filename_ext;
			wxString FullFileName = ScreenSch->m_FileName;
			ChangeFileNameExt( FullFileName, g_Prj_Config_Filename_ext );
			
			FullFileName = EDA_FileSelector(_("Read config file"),
					wxGetCwd(),				/* Chemin par defaut */
					FullFileName,			/* nom fichier par defaut */
					g_Prj_Config_Filename_ext,	/* extension par defaut */
					mask,					/* Masque d'affichage */
					this,
					wxOPEN,
					TRUE					/* ne change pas de repertoire courant */
					);
			if ( FullFileName == "") break;
			if ( ! wxFileExists(FullFileName) )
				{
				wxString msg = _("File ") + FullFileName +_("not found");;
				DisplayError(this, msg); break;
				}
			Read_Config(  );
			}
			break;

		default:
			DisplayError(this, "WinEDA_SchematicFrame::Process_Config internal error");
		}
}


/*******************************************************************/
bool Read_Config( void )
/*******************************************************************/
/* lit la configuration, si elle n'a pas deja ete lue
	1 - lit <nom fichier root>.pro
	2 - si non trouve lit <chemin des binaires>../template/kicad.pro
	3 - si non trouve: init des variables aux valeurs par defaut

	Retourne TRUE si lu, FALSE si config non lue
*/
{
wxString FullFileName;
bool success = TRUE;
	
	FullFileName = ScreenSch->m_FileName;
	g_LibName_List.Clear();
	
	if ( ! EDA_Appl->ReadProjectConfig(FullFileName,
		GROUP, ParamCfgList, TRUE) )	// Config non lue
	{
		success = FALSE;
	}

	/* Traitement des variables particulieres: */
	SetRealLibraryPath("library");

	// If the list is void, load the libraries "power.lib" and "device.lib"
	if ( g_LibName_List.GetCount() == 0 )
	{
		g_LibName_List.Add("power");
		g_LibName_List.Add("device");
	}

	if ( EDA_Appl->SchematicFrame )
	{
		EDA_Appl->SchematicFrame->SetDrawBgColor(DrawBgColor);
		EDA_Appl->SchematicFrame->m_Draw_Grid = g_ShowGrid;
	}

	LoadLibraries(EDA_Appl->SchematicFrame);

	return success;
}



/****************************************************************/
void WinEDA_SchematicFrame::Save_Config(wxWindow * displayframe)
/***************************************************************/
{
wxString path;
wxString FullFileName;
wxString mask("*");
	
	mask += g_Prj_Config_Filename_ext;
	FullFileName = ScreenSch->m_FileName.AfterLast('/') /*ConfigFileName*/;
	ChangeFileNameExt( FullFileName, g_Prj_Config_Filename_ext );

	path = wxGetCwd();
	FullFileName = EDA_FileSelector(_("Save config file"),
					path,				/* Chemin par defaut */
					FullFileName,		/* nom fichier par defaut */
					g_Prj_Config_Filename_ext,				/* extension par defaut */
					mask,			/* Masque d'affichage */
					displayframe,
					wxSAVE,
					TRUE
					);
	if ( FullFileName == "") return;

	/* ecriture de la configuration */
	EDA_Appl->WriteProjectConfig(FullFileName, GROUP, ParamCfgList);
}
