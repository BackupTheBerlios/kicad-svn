	/*********************************************/
	/*	eesave.cpp  Module to Save EESchema files */
	/*********************************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"

#include "protos.h"

/* Format des fichiers: Voir EELOAD.CC */

/* Fonctions externes */

/* Fonctions Locales */
static int SavePartDescr( FILE *f, EDA_SchComponentStruct * LibItemStruct);
static int SaveSheetDescr( FILE *f, DrawSheetStruct * SheetStruct);
static void SaveLayers(FILE *f);

/* Variables locales */

/*****************************************************************************
* Routine to save an EESchema file.											 *
* FileSave controls how the file is to be saved - under what name.			 *
* Returns TRUE if the file has been saved.									 *
*****************************************************************************/
bool WinEDA_SchematicFrame::SaveEEFile(BASE_SCREEN *Window, int FileSave)
{
wxString msg;
wxString Name, BakName;
const char **LibNames;
char * layer, *width;
int ii, shape;
bool Failed = FALSE;
EDA_BaseStruct *Phead;
W_PLOT * PlotSheet;
FILE *f;
wxString dirbuf;

	if ( Window == NULL ) Window = ActiveScreen;

	/* If no name exists in the window yet - save as new. */
	if( Window->m_FileName ==  "" ) FileSave = FILE_SAVE_NEW;

	switch (FileSave)
	{
		case FILE_SAVE_AS:
			dirbuf = wxGetCwd() + STRING_DIR_SEP;
			Name = MakeFileName(dirbuf, Window->m_FileName, g_SchExtBuffer);
			/* Rename the old file to a '.bak' one: */
			BakName = Name;
			if ( wxFileExists(Name) )
			{
				ChangeFileNameExt(BakName, ".bak");
				remove(BakName);	/* delete Old .bak file */
				if( rename(Name, BakName) )
				{
					DisplayError(this, "Warning: unable to rename old file", 10);
				}
			}
			break;

		case FILE_SAVE_NEW:
		{
			wxString mask = "*" + g_SchExtBuffer;
			Name = EDA_FileSelector(_("Schematic files:"),
					"",					/* Chemin par defaut */
					Window->m_FileName,				/* nom fichier par defaut, et resultat */
					g_SchExtBuffer,		/* extension par defaut */
					mask,				/* Masque d'affichage */
					this,
					wxSAVE,
					FALSE
					);
			if ( Name == "" ) return FALSE;

			Window->m_FileName = Name;
			dirbuf = wxGetCwd() + STRING_DIR_SEP;
			Name = MakeFileName(dirbuf, Name, g_SchExtBuffer);

			break;
		}

		default: break;
	}

	if ((f = fopen(Name.GetData(), "wt")) == NULL)
	{
		msg = _("Failed to create file ") + Name;
		DisplayError(this, msg);
		return FALSE;
	}

	msg = _("Save file ") + Name;
	Affiche_Message(msg);

	LibNames = GetLibNames();
	BakName = "";	// temporary buffer!
	for (ii = 0; LibNames[ii] != NULL; ii++)
	{
		if (ii > 0) BakName += ",";
		BakName += LibNames[ii];
	}
	MyFree( LibNames);

	if (fprintf(f, "%s %s %d\n", EESCHEMA_FILE_STAMP,
		SCHEMATIC_HEAD_STRING, EESCHEMA_VERSION) == EOF ||
		fprintf(f, "LIBS:%s\n", BakName.GetData()) == EOF)
	{
		DisplayError(this, _("File write operation failed."));
		fclose(f);
		return FALSE;
	}

	Window->ClrModify();

	SaveLayers(f);
	/* Sauvegarde des dimensions du schema, des textes du cartouche.. */
	PlotSheet = Window->m_CurrentSheet;
	fprintf(f,"$Descr %s %d %d\n",PlotSheet->m_Name.GetData(),
			PlotSheet->m_Size.x, PlotSheet->m_Size.y);

	fprintf(f,"Sheet %d %d\n",Window->m_SheetNumber, Window->m_NumberOfSheet);
	fprintf(f,"Title \"%s\"\n",Window->m_Title.GetData());
	fprintf(f,"Date \"%s\"\n",Window->m_Date.GetData());
	fprintf(f,"Rev \"%s\"\n",Window->m_Revision.GetData());
	fprintf(f,"Comp \"%s\"\n",Window->m_Company.GetData());
	fprintf(f,"Comment1 \"%s\"\n",Window->m_Commentaire1.GetData());
	fprintf(f,"Comment2 \"%s\"\n",Window->m_Commentaire2.GetData());
	fprintf(f,"Comment3 \"%s\"\n",Window->m_Commentaire3.GetData());
	fprintf(f,"Comment4 \"%s\"\n",Window->m_Commentaire4.GetData());

	fprintf(f,"$EndDescr\n");

	/* Sauvegarde des elements du dessin */
	Phead = Window->EEDrawList;
	while (Phead)
		{
		switch(Phead->m_StructType)
			{
			case DRAW_LIB_ITEM_STRUCT_TYPE:		  /* Its a library item. */
				SavePartDescr( f, (EDA_SchComponentStruct *) Phead);
				break;

			case DRAW_SHEET_STRUCT_TYPE:	   /* Its a Sheet item. */
				SaveSheetDescr( f, (DrawSheetStruct *) Phead);
				break;

			case DRAW_SEGMENT_STRUCT_TYPE:		 /* Its a Segment item. */
				#undef STRUCT
				#define STRUCT ((EDA_DrawLineStruct *) Phead)
				layer = "Notes"; width = "Line";
				if (STRUCT->m_Layer == LAYER_WIRE) layer = "Wire";
				if (STRUCT->m_Layer == LAYER_BUS) layer = "Bus";
				if( STRUCT->m_Width != GR_NORM_WIDTH) layer = "Bus";
				if (fprintf(f, "Wire %s %s\n", layer, width ) == EOF)
					{
					Failed = TRUE; break;
					}
				if (fprintf(f, "\t%-4d %-4d %-4d %-4d\n",
						STRUCT->m_Start.x,STRUCT->m_Start.y,
						STRUCT->m_End.x,STRUCT->m_End.y) == EOF)
					{
					Failed = TRUE; break;
					}
				break;

			case DRAW_BUSENTRY_STRUCT_TYPE:		 /* Its a Raccord item. */
				#undef STRUCT
				#define STRUCT ((DrawBusEntryStruct *) Phead)
				layer = "Wire"; width = "Line";
				if (STRUCT->m_Layer == LAYER_BUS)
					{
					layer = "Bus"; width = "Bus";
					}

				if (fprintf(f, "Entry %s %s\n", layer, width) == EOF)
					{
					Failed = TRUE; break;
					}
				if( fprintf(f, "\t%-4d %-4d %-4d %-4d\n",
						STRUCT->m_Pos.x,STRUCT->m_Pos.y,
						STRUCT->m_End().x,STRUCT->m_End().y) == EOF)
						{
						Failed = TRUE; break;
						}
				break;

			case DRAW_POLYLINE_STRUCT_TYPE:		  /* Its a polyline item. */
				#undef STRUCT
				#define STRUCT ((DrawPolylineStruct *) Phead)
				layer = "Notes"; width = "Line";
				if (STRUCT->m_Layer == LAYER_WIRE) layer = "Wire";
				if (STRUCT->m_Layer == LAYER_BUS) layer = "Bus";
				if( STRUCT->m_Width != GR_NORM_WIDTH) width = "Bus";
				if (fprintf(f, "Poly %s %s %d\n",
							width, layer, STRUCT->m_NumOfPoints) == EOF)
					{
					Failed = TRUE; break;
					}
				for (ii = 0; ii < STRUCT->m_NumOfPoints; ii++)
					{
					if (fprintf(f, "\t%-4d %-4d\n",
						STRUCT->m_Points[ii*2],
						STRUCT->m_Points[ii*2+1]) == EOF)
						{
						Failed = TRUE;
						break;
						}
					}
				break;

			case DRAW_JUNCTION_STRUCT_TYPE:	/* Its a connection item. */
				#undef STRUCT
				#define STRUCT ((DrawJunctionStruct *) Phead)
				if (fprintf(f, "Connection ~ %-4d %-4d\n",
					STRUCT->m_Pos.x, STRUCT->m_Pos.y) == EOF)
					{
					Failed = TRUE;
					}
				break;

			case DRAW_NOCONNECT_STRUCT_TYPE:	/* Its a NoConnection item. */
				#undef STRUCT
				#define STRUCT ((DrawNoConnectStruct *) Phead)
				if (fprintf(f, "NoConn ~ %-4d %-4d\n",
					STRUCT->m_Pos.x, STRUCT->m_Pos.y) == EOF)
					{
					Failed = TRUE;
					}
				break;

			case DRAW_TEXT_STRUCT_TYPE:			/* Its a text item. */
				#undef STRUCT
				#define STRUCT ((DrawTextStruct *) Phead)
				if (fprintf(f, "Text Notes %-4d %-4d %-4d %-4d ~\n%s\n",
						STRUCT->m_Pos.x, STRUCT->m_Pos.y,
						STRUCT->m_Orient, STRUCT->m_Size.x,
						STRUCT->m_Text.GetData()) == EOF)
					Failed = TRUE;
				break;


			case DRAW_LABEL_STRUCT_TYPE:		/* Its a label item. */
				#undef STRUCT
				#define STRUCT ((DrawLabelStruct *) Phead)
				shape = '~';
				if (fprintf(f, "Text Label %-4d %-4d %-4d %-4d %c\n%s\n",
						STRUCT->m_Pos.x, STRUCT->m_Pos.y,
						STRUCT->m_Orient, STRUCT->m_Size.x, shape,
						STRUCT->m_Text.GetData()) == EOF)
					Failed = TRUE;
				break;


			case DRAW_GLOBAL_LABEL_STRUCT_TYPE: /* Its a Global label item. */
				#undef STRUCT
				#define STRUCT ((DrawGlobalLabelStruct *) Phead)
				shape = STRUCT->m_Shape;
			if (fprintf(f, "Text GLabel %-4d %-4d %-4d %-4d %s\n%s\n",
						STRUCT->m_Pos.x, STRUCT->m_Pos.y,
						STRUCT->m_Orient,	STRUCT->m_Size.x,
						SheetLabelType[shape],
						STRUCT->m_Text.GetData()) == EOF)
					Failed = TRUE;
				break;

			case DRAW_MARKER_STRUCT_TYPE:	/* Its a marker item. */
				#undef STRUCT
				#define STRUCT ((DrawMarkerStruct *) Phead)
				if( STRUCT->GetComment() ) msg = STRUCT->GetComment();
				else msg = "";
				if (fprintf(f, "Kmarq %c %-4d %-4d \"%s\" F=%X\n",
								(int) STRUCT->m_Type + 'A',
								STRUCT->m_Pos.x, STRUCT->m_Pos.y,
								msg.GetData(), STRUCT->m_MarkFlags) == EOF)
					{
					Failed = TRUE;
					}
				break;

			case DRAW_SHEETLABEL_STRUCT_TYPE :
			case DRAW_PICK_ITEM_STRUCT_TYPE :
				break;

			default:
				break;
			}

		if (Failed)
			{
			DisplayError(this, _("File write operation failed."));
			break;
			}

		Phead = Phead->Pnext;
		}
	if (fprintf(f, "$EndSCHEMATC\n") == EOF) Failed = TRUE;

	fclose(f);

	if (FileSave == FILE_SAVE_NEW) Window->m_FileName = Name;

	return !Failed;
}


/*******************************************************************/
static int SavePartDescr( FILE *f, EDA_SchComponentStruct * LibItemStruct)
/*******************************************************************/
/* Routine utilisee dans la routine precedente.
	Assure la sauvegarde de la structure LibItemStruct
*/
{
int ii, Failed = FALSE;
char Name1[256], Name2[256];
int hjustify, vjustify;

	strcpy(Name1, LibItemStruct->m_Field[REFERENCE].m_Text.GetData());
	for (ii = 0; ii < (int)strlen(Name1); ii++)
	if (Name1[ii] <= ' ') Name1[ii] = '~';

	if (LibItemStruct->m_ChipName != NULL)
	{
		strcpy(Name2, LibItemStruct->m_ChipName);
		for (ii = 0; ii < (int)strlen(Name2); ii++)
		if (Name2[ii] <= ' ') Name2[ii] = '~';
	}

	else  strcpy(Name2, NULL_STRING);

	fprintf(f, "$Comp\n");

	if(fprintf (f, "L %s %s\n", Name2, Name1) == EOF)
	{
		Failed = TRUE;
		return(Failed);
	}

	/* Generation de numero d'unit, convert et Time Stamp*/
	if(fprintf(f, "U %d %d %8.8lX\n",
					 LibItemStruct->m_Multi,
					 LibItemStruct->m_Convert,
					 LibItemStruct->m_TimeStamp) == EOF)
		{
		Failed = TRUE; return(Failed);
		}

	/* Sortie de la position */
	if(fprintf(f, "P %d %d\n",
					 LibItemStruct->m_Pos.x, LibItemStruct->m_Pos.y) == EOF)
		{
		Failed = TRUE; return(Failed);
		}

	for( ii = 0; ii < NUMBER_OF_FIELDS; ii++ )
		{
		PartTextStruct * field = & LibItemStruct->m_Field[ii];
		if( field->m_Text == "") continue;
		hjustify = 'C';
		if ( field->m_HJustify == GR_TEXT_HJUSTIFY_LEFT)
			hjustify = 'L';
		else if ( field->m_HJustify == GR_TEXT_HJUSTIFY_RIGHT)
			hjustify = 'R';
		vjustify = 'C';
		if ( field->m_VJustify == GR_TEXT_VJUSTIFY_BOTTOM)
			vjustify = 'B';
		else if ( field->m_VJustify == GR_TEXT_VJUSTIFY_TOP)
			vjustify = 'T';
		if( fprintf(f,"F %d \"%s\" %c %-3d %-3d %-3d %4.4X %c %c\n", ii,
					field->m_Text.GetData(),
					field->m_Orient == TEXT_ORIENT_HORIZ ? 'H' : 'V',
					field->m_Pos.x, field->m_Pos.y,
					field->m_Size.x,
					field->m_Attributs,
					hjustify, vjustify) == EOF)
			{
			Failed = TRUE; break;
			}
		}

	if (Failed)  return(Failed);

	/* Generation du num unit, position, box ( ancienne norme )*/
	if(fprintf(f, "\t%-4d %-4d %-4d\n",
					 LibItemStruct->m_Multi,
					 LibItemStruct->m_Pos.x, LibItemStruct->m_Pos.y) == EOF)
		{
		Failed = TRUE; return(Failed);
		}

	if( fprintf(f, "\t%-4d %-4d %-4d %-4d\n",
					LibItemStruct->m_Transform[0][0],
					LibItemStruct->m_Transform[0][1],
					LibItemStruct->m_Transform[1][0],
					LibItemStruct->m_Transform[1][1]) == EOF)
		{
		Failed = TRUE; return(Failed);
		}

	fprintf(f, "$EndComp\n");
	return(Failed);
}

	/*******************************************************************/
	/* static int SaveSheetDescr( FILE *f, DrawSheetStruct * SheetStruct) */
	/*******************************************************************/

/* Routine utilisee dans la routine precedente.
	Assure la sauvegarde de la structure LibItemStruct
*/
static int SaveSheetDescr( FILE *f, DrawSheetStruct * SheetStruct)
{
int ii;
int Failed = FALSE;
DrawSheetLabelStruct * SheetLabel;

	fprintf(f, "$Sheet\n");

	if (fprintf(f, "S %-4d %-4d %-4d %-4d\n",
					SheetStruct->m_Pos.x,SheetStruct->m_Pos.y,
					SheetStruct->m_End.x,SheetStruct->m_End.y) == EOF)
		{
		Failed = TRUE; return(Failed);
		}

	/* Generation de la liste des 2 textes (sheetname et filename) */
	if ( ! SheetStruct->m_Field[VALUE].m_Text.IsEmpty())
	{
		if(fprintf(f,"F0 \"%s\" %d\n", SheetStruct->m_Field[VALUE].m_Text.GetData(),
					SheetStruct->m_Field[VALUE].m_Size.x) == EOF)
		{
			Failed = TRUE; return(Failed);
		}
	}

	if( ! SheetStruct->m_Field[SHEET_FILENAME].m_Text.IsEmpty())
		{
		if(fprintf(f,"F1 \"%s\" %d\n",
				SheetStruct->m_Field[SHEET_FILENAME].m_Text.GetData(),
				SheetStruct->m_Field[SHEET_FILENAME].m_Size.x) == EOF)
			{
			Failed = TRUE; return(Failed);
			}
		}

	/* Generation de la liste des labels (entrees) de la sous feuille */
	ii = 2;
	SheetLabel = SheetStruct->m_Label;
	while( SheetLabel != NULL )
	{
		int type = 'U', side = 'L';

		if( SheetLabel->m_Text.IsEmpty() ) continue;
		if( SheetLabel->m_Edge ) side = 'R';

		switch(SheetLabel->m_Shape)
		{
			case NET_INPUT: type = 'I'; break;
			case NET_OUTPUT: type = 'O'; break;
			case NET_BIDI: type = 'B'; break;
			case NET_TRISTATE: type = 'T'; break;
			case NET_UNSPECIFIED: type = 'U'; break;
		}

		if(fprintf(f,"F%d \"%s\" %c %c %-3d %-3d %-3d\n", ii,
			SheetLabel->m_Text.GetData(), type, side,
			SheetLabel->m_Pos.x, SheetLabel->m_Pos.y,
			SheetLabel->m_Size.x) == EOF)
		{
			Failed = TRUE; break;
		}
		ii++;
		SheetLabel = (DrawSheetLabelStruct*)SheetLabel->Pnext;
	}

	fprintf(f, "$EndSheet\n");
	return(Failed);
}

	/****************************/
	/* void SaveLayers(FILE *f)	*/
	/****************************/
/* Save a Layer Structure to a file */
static void SaveLayers(FILE *f)
{
	fprintf(f,"EELAYER %2d %2d\n", g_LayerDescr.NumberOfLayers,g_LayerDescr.CurrentLayer);
	fprintf(f,"EELAYER END\n");
}


