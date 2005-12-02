	/*******************************************************/
	/* delsheet.cpp  Routine d'effacement d'une hierarchie */
	/*******************************************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"

#include "protos.h"


/*********************************************************/
void DeleteSubHierarchy(DrawSheetStruct * FirstSheet)
/*********************************************************/

/*  Supprime tous les elements de la sous hierarchie correspondante a
	la feuille FirstSheet
	FirstSheet n'est pas touche
*/
{
SCH_SCREEN *Screen;
EDA_BaseStruct *DrawStruct;
EDA_BaseStruct *EEDrawList;
WinEDA_SchematicFrame * frame = EDA_Appl->SchematicFrame;
char Line[256];

	if( FirstSheet == NULL ) return;

	if( FirstSheet->m_StructType != DRAW_SHEET_STRUCT_TYPE)
		{
		DisplayError(NULL,
				"DeleteSubHierarchy error: NOT a SubSheet");
		return;
		}

	/* effacement du sous schema correspondant */
	Screen = (SCH_SCREEN*)FirstSheet->m_Son;

	if(Screen == NULL) return;

	if( Screen->IsModify() )
		{
		sprintf(Line,"Sheet %s (file %s) modified. Save ?",
					FirstSheet->m_Field[VALUE].m_Text.GetData(),
					Screen->m_FileName.GetData()); 
		if( IsOK(frame, Line) )
			{
			frame->SaveEEFile(Screen, FILE_SAVE_AS);
			}
		}

	/* Effacement des sous feuilles */
	EEDrawList = Screen->EEDrawList;
	while (EEDrawList != NULL)
		{
		DrawStruct = EEDrawList;
		EEDrawList = EEDrawList->Pnext;
		if( DrawStruct->m_StructType == DRAW_SHEET_STRUCT_TYPE)
			{
			DeleteSubHierarchy((DrawSheetStruct *) DrawStruct);
			}
		}

	/* Effacement des elements autres que les sous feuilles */
	Screen->ClearDrawList();

	/* Modification du chainage des structures ecran
	et suppression de l'ecran */
	SCH_SCREEN * Pscreen, * Nscreen;
	Pscreen = (SCH_SCREEN*) Screen->Pback;
	Nscreen = Screen->Next();
	if( Pscreen ) Pscreen->Pnext = Nscreen;
	if ( Nscreen ) Nscreen->Pback = Pscreen;
	if ( frame->GetScreen() == Screen ) frame->m_CurrentScreen = ScreenSch;
	ActiveScreen = frame->GetScreen();
	if( Screen != ScreenSch ) delete(Screen);
}

/**************************************************/
bool ClearProjectDrawList(SCH_SCREEN * screen)
/**************************************************/
/* Routine d'effacement d'une liste */
{
EDA_BaseStruct *DrawStruct;
EDA_BaseStruct *EEDrawList ;
	
	if ( screen == NULL ) return(TRUE);

	if( screen->EEDrawList == NULL ) return(TRUE);  /* Projet vide */

	/* Effacement des structures */
	EEDrawList = screen->EEDrawList;
	if (EEDrawList != NULL)
		{
		while (EEDrawList != NULL)
			{
			DrawStruct = EEDrawList;
			EEDrawList = EEDrawList->Pnext;

			if( DrawStruct->m_StructType == DRAW_SHEET_STRUCT_TYPE)
				{
				DeleteSubHierarchy((DrawSheetStruct*) DrawStruct);
				}

			delete DrawStruct;
			}
		screen->EEDrawList = NULL;
		}

	/* Init de l'ecran de depart */
	screen->m_SheetNumber = screen->m_NumberOfSheet = 1;
	screen->m_Title = "";
	screen->m_Revision = "";
	screen->m_Company = "";
	screen->m_Commentaire1 = "";
	screen->m_Commentaire2 = "";
	screen->m_Commentaire3 = "";
	screen->m_Commentaire4 = "";
	screen->m_Date = GenDate();

	return TRUE;
}

