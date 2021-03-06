	/*****************************/
	/* EESchema - lib_export.cpp */
	/*****************************/

/* Routines de maintenanace des librariries:
	sauvegarde, modification de librairies.
	creation edition suppression de composants
*/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"

#include "protos.h"

#include "id.h"



/*************************************************/
void WinEDA_LibeditFrame::ImportOnePart(void)
/*************************************************/
/* Routine de lecture de 1 description.
	Le format est celui des librairies, mais on ne charge que 1 composant
	ou le 1er composant s'il y en a plusieurs.
	Si le premier composant est un alias, on chargera la racine correspondante
*/
{
wxString Name, mask;
LibraryStruct * LibTmp;
EDA_LibComponentStruct *LibEntry;
int err = 1;

	mask = wxT("*") + g_LibExtBuffer;
	Name = EDA_FileSelector( _("Import part:"),
					wxEmptyString,				/* Chemin par defaut */
					wxEmptyString,		 	/* nom fichier par defaut */
					g_LibExtBuffer,	/* extension par defaut */
					mask,			/* Masque d'affichage */
					this,
					0,
					TRUE
					);
	if ( Name == wxEmptyString ) return;

	LibTmp = g_LibraryList; g_LibraryList = NULL;

	LoadLibraryName(this, Name, wxT("$tmplib$"));

	if ( g_LibraryList )
	{
		LibEntry = (EDA_LibComponentStruct *)PQFirst(&g_LibraryList->m_Entries,FALSE);

		if( LibEntry )
			err = LoadOneLibraryPartAux(LibEntry, g_LibraryList, 1);
		FreeCmpLibrary(this, g_LibraryList->m_Name);

		if ( err == 0 )
		{
			ReCreateHToolbar();
			DisplayLibInfos();
			ReDrawPanel();
		}
	}

	g_LibraryList = LibTmp;
	if (err) DisplayError(this, _("File is empty"), 30);
}


/************************************************************/
void WinEDA_LibeditFrame::ExportOnePart(bool create_lib)
/************************************************************/
/* Routine de creation d'une nouvelle librairie et de sauvegarde du
 composant courant dans cette librarie
	si create_lib == TRUE sauvegarde dans le repertoire des libr
	sinon: sauvegarde sous le nom demande sans modifications.

	Le format du fichier cree est dans tous les cas le meme.
*/
{
wxString Name, mask;
LibraryStruct * NewLib, * LibTmp, * CurLibTmp;
int err;

	if(CurrentLibEntry == NULL)
		{
		DisplayError(this, _("No Part to Save"),10); return;
		}

	Name = CurrentLibEntry->m_Name.m_Text;
	Name.MakeLower();

	mask = wxT("*") + g_LibExtBuffer;
	wxString def_path;
	if ( create_lib ) def_path = g_RealLibDirBuffer;
	Name = EDA_FileSelector( create_lib ? _("New Library") : _("Export part:"),
					def_path,	/* Chemin par defaut */
					Name,		 	/* nom fichier par defaut */
					g_LibExtBuffer,	/* extension par defaut */
					mask,			/* Masque d'affichage */
					this,
					wxSAVE,
					TRUE
					);
	if ( Name == wxEmptyString ) return;


	/* Creation d'une librairie standard pour sauvegarde */
	ChangeFileNameExt(Name, g_LibExtBuffer);

	LibTmp = g_LibraryList;
	CurLibTmp = CurrentLib;

	NewLib = new LibraryStruct(LIBRARY_TYPE_EESCHEMA, wxT("$libTmp$"), Name);
	g_LibraryList = NewLib;

	/* Sauvegarde du composant: */
	CurrentLib = NewLib;
	SaveOnePartInMemory();
	err = SaveOneLibrary(this, Name, NewLib);

	/* Suppression de la librarie temporaire */
	FreeCmpLibrary(this, NewLib->m_Name );
	g_LibraryList = LibTmp;
	CurrentLib = CurLibTmp;

wxString msg;
	if ( create_lib && (err == 0) )
	{
		msg = Name + _("0k");
		DisplayInfo(this,
_("Note: this new library will be available only if it is loaded by eeschema\nModify eeschema config if you want use it") );
	}
	else msg = _("Error while create ") + Name;
	Affiche_Message(msg);
}



