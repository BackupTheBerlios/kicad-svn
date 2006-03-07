/////////////////////////////////////////////////////////////////////////////
// Name:        dialog_build_BOM.cpp
// Purpose:     
// Author:      jean-pierre Charras
// Modified by: 
// Created:     01/15/06 18:18:44
// RCS-ID:      
// Copyright:   GNU license
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 01/15/06 18:18:44

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "dialog_build_BOM.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "fctsys.h"

////@begin includes
////@end includes

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"
#include "netlist.h"

#include "dialog_build_BOM.h"


#include "protos.h"

////@begin XPM images
////@end XPM images


/* Structures pour memo et liste des elements */
typedef struct ListLabel
{
	int m_StructType;
	void * m_Label;
	int m_SheetNumber;
} ListLabel;


/* fonctions locales */
static void GenereListeOfItems(wxWindow * frame, const wxString & FullFileName);
static int GenListeGLabels( ListLabel * List );
static int ListTriComposantByRef(EDA_SchComponentStruct **Objet1,
									EDA_SchComponentStruct **Objet2);
static int ListTriComposantByVal(EDA_SchComponentStruct **Objet1,
									EDA_SchComponentStruct **Objet2);
static int ListTriGLabelBySheet(ListLabel *Objet1, ListLabel *Objet2);
static int ListTriGLabelByVal(ListLabel *Objet1, ListLabel *Objet2);
static void DeleteSubCmp( EDA_BaseStruct ** List , int NbItems);

static int PrintListeCmpByRef( FILE *f, EDA_BaseStruct **List, int NbItems);
static int PrintListeCmpByVal( FILE *f, EDA_BaseStruct **List, int NbItems);

static int PrintListeGLabel( FILE *f, ListLabel *List, int NbItems);

/* Local variables */
bool s_ListByRef = TRUE;
bool s_ListByValue = TRUE;
bool s_ListWithSubCmponents;
bool s_ListHierarchicalPinByName;
bool s_ListBySheet;
bool s_BrowsList;

/*!
 * WinEDA_Build_BOM_Frame type definition
 */

IMPLEMENT_DYNAMIC_CLASS( WinEDA_Build_BOM_Frame, wxDialog )

/*!
 * WinEDA_Build_BOM_Frame event table definition
 */

BEGIN_EVENT_TABLE( WinEDA_Build_BOM_Frame, wxDialog )

////@begin WinEDA_Build_BOM_Frame event table entries
    EVT_BUTTON( wxID_OK, WinEDA_Build_BOM_Frame::OnOkClick )

    EVT_BUTTON( wxID_EXIT, WinEDA_Build_BOM_Frame::OnExitClick )

////@end WinEDA_Build_BOM_Frame event table entries

END_EVENT_TABLE()

/***************************************************************/
void InstallToolsFrame(WinEDA_DrawFrame *parent, wxPoint & pos)
/***************************************************************/
{
	WinEDA_Build_BOM_Frame * frame = new WinEDA_Build_BOM_Frame(parent);
	frame->ShowModal(); frame->Destroy();
}

/*!
 * WinEDA_Build_BOM_Frame constructors
 */

WinEDA_Build_BOM_Frame::WinEDA_Build_BOM_Frame( )
{
}

WinEDA_Build_BOM_Frame::WinEDA_Build_BOM_Frame( WinEDA_DrawFrame* parent,
			wxWindowID id, const wxString& caption, const wxPoint& pos,
			const wxSize& size, long style )
{
	m_Parent = parent;
    Create(parent, id, caption, pos, size, style);
}

/*!
 * WinEDA_Build_BOM_Frame creator
 */

bool WinEDA_Build_BOM_Frame::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin WinEDA_Build_BOM_Frame member initialisation
    m_ListCmpbyRefItems = NULL;
    m_ListCmpbyValItems = NULL;
    m_ListSubCmpItems = NULL;
    m_GenListLabelsbyVal = NULL;
    m_GenListLabelsbySheet = NULL;
    m_GetListBrowser = NULL;
////@end WinEDA_Build_BOM_Frame member initialisation

////@begin WinEDA_Build_BOM_Frame creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end WinEDA_Build_BOM_Frame creation
    return TRUE;
}

/*!
 * Control creation for WinEDA_Build_BOM_Frame
 */

void WinEDA_Build_BOM_Frame::CreateControls()
{    
	SetFont(*g_DialogFont);
	
////@begin WinEDA_Build_BOM_Frame content construction
    // Generated by DialogBlocks, 24/02/2006 20:49:36 (unregistered)

    WinEDA_Build_BOM_Frame* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(itemDialog1, wxID_ANY, _("List items : "));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ListCmpbyRefItems = new wxCheckBox( itemDialog1, ID_CHECKBOX, _("Components by Reference"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ListCmpbyRefItems->SetValue(true);
    itemStaticBoxSizer4->Add(m_ListCmpbyRefItems, 0, wxALIGN_LEFT|wxALL, 5);

    m_ListCmpbyValItems = new wxCheckBox( itemDialog1, ID_CHECKBOX1, _("Components by Value"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ListCmpbyValItems->SetValue(true);
    itemStaticBoxSizer4->Add(m_ListCmpbyValItems, 0, wxALIGN_LEFT|wxALL, 5);

    m_ListSubCmpItems = new wxCheckBox( itemDialog1, ID_CHECKBOX2, _("Sub Components (i.e U2A, U2B..)"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ListSubCmpItems->SetValue(false);
    itemStaticBoxSizer4->Add(m_ListSubCmpItems, 0, wxALIGN_LEFT|wxALL, 5);

    m_GenListLabelsbyVal = new wxCheckBox( itemDialog1, ID_CHECKBOX3, _("Hierachy Pins by name"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_GenListLabelsbyVal->SetValue(false);
    itemStaticBoxSizer4->Add(m_GenListLabelsbyVal, 0, wxALIGN_LEFT|wxALL, 5);

    m_GenListLabelsbySheet = new wxCheckBox( itemDialog1, ID_CHECKBOX4, _("Hierachy Pins by Sheets"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_GenListLabelsbySheet->SetValue(false);
    itemStaticBoxSizer4->Add(m_GenListLabelsbySheet, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer3->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton12 = new wxButton( itemDialog1, wxID_OK, _("&Create List"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton12->SetDefault();
    itemButton12->SetForegroundColour(wxColour(166, 0, 0));
    itemBoxSizer11->Add(itemButton12, 0, wxGROW|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, wxID_EXIT, _("&Quit"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton13->SetForegroundColour(wxColour(0, 0, 210));
    itemBoxSizer11->Add(itemButton13, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer14Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Options"));
    wxStaticBoxSizer* itemStaticBoxSizer14 = new wxStaticBoxSizer(itemStaticBoxSizer14Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer14, 0, wxGROW|wxALL, 10);

    m_GetListBrowser = new wxCheckBox( itemDialog1, ID_CHECKBOX5, _("Launch list browser"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_GetListBrowser->SetValue(false);
    itemStaticBoxSizer14->Add(m_GetListBrowser, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_ListCmpbyRefItems->SetValidator( wxGenericValidator(& s_ListByRef) );
    m_ListCmpbyValItems->SetValidator( wxGenericValidator(& s_ListByValue) );
    m_ListSubCmpItems->SetValidator( wxGenericValidator(& s_ListWithSubCmponents) );
    m_GenListLabelsbyVal->SetValidator( wxGenericValidator(& s_ListHierarchicalPinByName) );
    m_GenListLabelsbySheet->SetValidator( wxGenericValidator(& s_ListBySheet) );
    m_GetListBrowser->SetValidator( wxGenericValidator(& s_BrowsList) );
////@end WinEDA_Build_BOM_Frame content construction
}

/*!
 * Should we show tooltips?
 */

bool WinEDA_Build_BOM_Frame::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap WinEDA_Build_BOM_Frame::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin WinEDA_Build_BOM_Frame bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end WinEDA_Build_BOM_Frame bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon WinEDA_Build_BOM_Frame::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin WinEDA_Build_BOM_Frame icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end WinEDA_Build_BOM_Frame icon retrieval
}
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void WinEDA_Build_BOM_Frame::OnOkClick( wxCommandEvent& event )
{
	GenList();
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in WinEDA_Build_BOM_Frame.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in WinEDA_Build_BOM_Frame. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_EXIT
 */

void WinEDA_Build_BOM_Frame::OnExitClick( wxCommandEvent& event )
{
	EndModal(0);
}

/**********************************************************/
void WinEDA_Build_BOM_Frame::GenList(void)
/**********************************************************/
{
#define EXT_LIST wxT(".lst")
wxString mask, filename;

	s_ListByRef = m_ListCmpbyRefItems->GetValue();
	s_ListByValue = m_ListCmpbyValItems->GetValue();
	s_ListWithSubCmponents = m_ListSubCmpItems->GetValue();
	s_ListHierarchicalPinByName = m_GenListLabelsbyVal->GetValue();
	s_ListBySheet = m_GenListLabelsbySheet->GetValue();
    s_BrowsList = m_GetListBrowser->GetValue();

	m_ListFileName = ScreenSch->m_FileName;
	ChangeFileNameExt(m_ListFileName, EXT_LIST);
	mask = wxT("*"); mask += EXT_LIST;

	filename = EDA_FileSelector(_("Bill of material:"),
					wxEmptyString,				/* Chemin par defaut (ici dir courante) */
					m_ListFileName,	/* nom fichier par defaut, et resultat */
					EXT_LIST,		/* extension par defaut */
					mask,			/* Masque d'affichage */
					this,
					wxSAVE,
					TRUE
					);
	if ( filename.IsEmpty() ) return;
	else m_ListFileName = filename;

	/* Close dialog and show le list, if wanted */
	GenereListeOfItems(this, m_ListFileName);
	Close();
	
	if ( s_BrowsList )
	{
		wxString editorname = GetEditorName();
		AddDelimiterString(filename);
		ExecuteFile(this, editorname, filename);
	}
}

/***************************************************************************/
static void GenereListeOfItems(wxWindow * frame, const wxString & FullFileName)
/***************************************************************************/
/*
	Routine principale pour la creation des listings ( composants et/ou labels
	globaux et "sheet labels" )
*/
{
FILE *f;
EDA_BaseStruct ** List;
ListLabel * ListOfLabels;
int NbItems;
char Line[1024];
wxString msg;
	
	/* Creation de la liste des elements */
	if ((f = wxFopen(FullFileName, wxT("wt"))) == NULL)
	{
		msg = _("Failed to open file "); msg << FullFileName;
		DisplayError(frame, msg);
		return;
	}

	NbItems = GenListeCmp(NULL );
	if ( NbItems )
	{
		List = (EDA_BaseStruct **)
				MyZMalloc( NbItems * sizeof(EDA_BaseStruct **) );
		if (List == NULL )
		{
			fclose(f);  return;
		}

		GenListeCmp(List);

		/* generation du fichier listing */
		DateAndTime(Line);
		fprintf( f, "%s  >> Creation date: %s\n", CONV_TO_UTF8(Main_Title), Line );

		/* Tri et impression de la liste des composants */

		qsort( List, NbItems, sizeof( EDA_BaseStruct * ),
				(int(*)(const void*, const void*))ListTriComposantByRef);

		if( ! s_ListWithSubCmponents ) DeleteSubCmp(List, NbItems);

		if( s_ListByRef )
		{
			PrintListeCmpByRef(f, List, NbItems);
		}

		if( s_ListByValue )
		{
			qsort( List, NbItems, sizeof( EDA_BaseStruct * ),
					(int(*)(const void*, const void*))ListTriComposantByVal);
			PrintListeCmpByVal(f, List, NbItems);
		}
		MyFree( List );
	}

	/***************************************/
	/* Generation liste des Labels globaux */
	/***************************************/

	NbItems = GenListeGLabels( NULL );
	if ( NbItems )
	{
		ListOfLabels = (ListLabel *) MyZMalloc( NbItems * sizeof(ListLabel) );
		if (ListOfLabels == NULL )
		{
			  fclose(f); return;
		}

		GenListeGLabels(ListOfLabels);

		/* Tri de la liste */
		if( s_ListBySheet )
		{
			qsort( ListOfLabels, NbItems, sizeof( ListLabel ),
				(int(*)(const void*, const void*))ListTriGLabelBySheet);

			msg = _("\n#Glob labels ( order = Sheet Number )\n");
			fprintf( f, "%s", CONV_TO_UTF8(msg));
			PrintListeGLabel(f, ListOfLabels, NbItems);
		}

		if( s_ListHierarchicalPinByName )
		{
			qsort( ListOfLabels, NbItems, sizeof( ListLabel ),
				(int(*)(const void*, const void*))ListTriGLabelByVal);

			msg = _("\n#Glob labels ( order = Alphab. )\n");
			fprintf( f, "%s", CONV_TO_UTF8(msg));
			PrintListeGLabel(f, ListOfLabels, NbItems);
		}
		MyFree( ListOfLabels );
	}

	msg = _("\n#End List\n");
	fprintf( f, "%s", CONV_TO_UTF8(msg));
	fclose(f);
}



/****************************************/
int GenListeCmp( EDA_BaseStruct ** List )
/****************************************/
/* Routine de generation de la liste des elements utiles du dessin
	Si List == NULL: comptage des elements
	Sinon remplissage de la liste
	Initialise "FlagControlMulti" a SheetNumber pour la sortie des listes
	et m_Father comme pointeur sur la sheet d'appartenance
*/
{
int ii = 0;
EDA_BaseStruct *DrawList;
EDA_SchComponentStruct *DrawLibItem;
BASE_SCREEN * screen = ScreenSch;

	for( ; screen != NULL ; screen = (BASE_SCREEN*)screen->Pnext )
		{
		DrawList = screen->EEDrawList;
		while ( DrawList )
			{
			switch( DrawList->m_StructType )
				{

				case DRAW_LIB_ITEM_STRUCT_TYPE :
					ii++;
					DrawLibItem = (EDA_SchComponentStruct *) DrawList;
					DrawLibItem->m_FlagControlMulti = screen->m_SheetNumber;
					DrawLibItem->m_Parent = screen;
					if( List )
						{
						*List = DrawList; List++;
						}
					break;


				default: break;
				}
			DrawList = DrawList->Pnext;
			}
		}
	return ( ii );
}

/*********************************************/
static int GenListeGLabels( ListLabel * List )
/*********************************************/
/* Count the Glabels, or fill the list Listwith Glabel pointers 
	If List == NULL: Item count only
	Else fill list of Glabels
*/
{
int ii = 0;
EDA_BaseStruct *DrawList;
DrawSheetLabelStruct *SheetLabel;
BASE_SCREEN * screen = ScreenSch;

	for( ; screen != NULL ; screen = (BASE_SCREEN*)screen->Pnext )
	{
		DrawList = screen->EEDrawList;
		while ( DrawList )
		{
			switch( DrawList->m_StructType )
			{
				case DRAW_GLOBAL_LABEL_STRUCT_TYPE :
					if( List )
					{
						List->m_StructType = DRAW_TEXT_STRUCT_TYPE;
						List->m_SheetNumber = screen->m_SheetNumber;
						List->m_Label = DrawList; List++;
					}
					ii++;
					break;
				
				case DRAW_SHEET_STRUCT_TYPE :
				{
					#define Sheet ((DrawSheetStruct * ) DrawList)
					SheetLabel= Sheet->m_Label;
					while( SheetLabel != NULL )
					{
						if ( List )
						{
							List->m_StructType = DRAW_SHEETLABEL_STRUCT_TYPE;
							List->m_SheetNumber = screen->m_SheetNumber;
							List->m_Label = SheetLabel;
							List++;
						}
						ii++;
						SheetLabel = (DrawSheetLabelStruct*)(SheetLabel->Pnext);
					}
					break;
				}

				default: break;
			}
			DrawList = DrawList->Pnext;
		}
	}
	return ( ii );
}

/**********************************************************/
static int ListTriComposantByVal(EDA_SchComponentStruct **Objet1,
							EDA_SchComponentStruct **Objet2)
/**********************************************************/
 /* Routine de comparaison pour le tri du Tableau par qsort()
	Les composants sont tries
		par valeur
		si meme valeur: par reference
			si meme valeur: par numero d'unite

*/
{
int ii;
const wxString * Text1, *Text2;

	if( (*Objet1 == NULL) && (*Objet2 == NULL ) ) return(0);
	if( *Objet1 == NULL) return(-1);
	if( *Objet2 == NULL) return(1);

	Text1 = &(*Objet1)->m_Field[VALUE].m_Text;
	Text2 = &(*Objet2)->m_Field[VALUE].m_Text;
	ii = Text1->CmpNoCase(*Text2);

	if( ii == 0 )
	{
		Text1 = &(*Objet1)->m_Field[REFERENCE].m_Text;
		Text2 = &(*Objet2)->m_Field[REFERENCE].m_Text;
		ii = Text1->CmpNoCase(*Text2);
	}

	if ( ii == 0 )
	{
		ii = (*Objet1)->m_Multi - (*Objet2)->m_Multi;
	}

	return(ii);
}

/**********************************************************/
static int ListTriComposantByRef(EDA_SchComponentStruct **Objet1,
							EDA_SchComponentStruct **Objet2)
/**********************************************************/
 /* Routine de comparaison pour le tri du Tableau par qsort()
	Les composants sont tries
		par reference
		si meme referenece: par valeur
			si meme valeur: par numero d'unite

*/
{
int ii;
const wxString * Text1, *Text2;

	if( (*Objet1 == NULL) && (*Objet2 == NULL ) ) return(0);
	if( *Objet1 == NULL) return(-1);
	if( *Objet2 == NULL) return(1);

	Text1 = &(*Objet1)->m_Field[REFERENCE].m_Text;
	Text2 = &(*Objet2)->m_Field[REFERENCE].m_Text;
	ii = Text1->CmpNoCase(*Text2);

	if( ii == 0 )
	{
		Text1 = &(*Objet1)->m_Field[VALUE].m_Text;
		Text2 = &(*Objet2)->m_Field[VALUE].m_Text;
		ii = Text1->CmpNoCase(*Text2);
	}

	if ( ii == 0 )
	{
		ii = (*Objet1)->m_Multi - (*Objet2)->m_Multi;
	}

	return(ii);
}

/******************************************************************/
static int ListTriGLabelByVal(ListLabel *Objet1, ListLabel *Objet2)
/*******************************************************************/
/* Routine de comparaison pour le tri du Tableau par qsort()
	Les labels sont tries
		par comparaison ascii
		si meme valeur: par numero de sheet

*/
{
int ii;
const wxString * Text1, *Text2;

	if( Objet1->m_StructType == DRAW_SHEETLABEL_STRUCT_TYPE )
		Text1 = &((DrawSheetLabelStruct *)Objet1->m_Label)->m_Text;
	else
		Text1 = &((DrawTextStruct *)Objet1->m_Label)->m_Text;

	if( Objet2->m_StructType == DRAW_SHEETLABEL_STRUCT_TYPE )
		Text2 = &((DrawSheetLabelStruct *)Objet2->m_Label)->m_Text;
	else
		Text2 = &((DrawTextStruct *)Objet2->m_Label)->m_Text;
	ii = Text1->CmpNoCase(*Text2);

	if ( ii == 0 )
		{
		ii = Objet1->m_SheetNumber - Objet2->m_SheetNumber;
		}

	return(ii);
}

/*******************************************************************/
static int ListTriGLabelBySheet(ListLabel *Objet1, ListLabel *Objet2)
/*******************************************************************/
/* Routine de comparaison pour le tri du Tableau par qsort()
	Les labels sont tries
		par sheet number
		si meme valeur, par ordre alphabetique

*/
{
int ii;
const wxString * Text1, *Text2;

	ii = Objet1->m_SheetNumber - Objet2->m_SheetNumber;

	if ( ii == 0 )
	{
		if( Objet1->m_StructType == DRAW_SHEETLABEL_STRUCT_TYPE )
			Text1 = &((DrawSheetLabelStruct *)Objet1->m_Label)->m_Text;
		else
			Text1 = &((DrawTextStruct *)Objet1->m_Label)->m_Text;

		if( Objet2->m_StructType == DRAW_SHEETLABEL_STRUCT_TYPE )
			Text2 = &((DrawSheetLabelStruct *)Objet2->m_Label)->m_Text;
		else
			Text2 = &((DrawTextStruct *)Objet2->m_Label)->m_Text;
		ii = Text1->CmpNoCase(*Text2);
	}

	return(ii);
}



/**************************************************************/
static void DeleteSubCmp( EDA_BaseStruct ** List, int NbItems )
/**************************************************************/
/* Supprime les sous-composants, c'est a dire les descriptions redonnantes des
boitiers multiples
	La liste des composant doit etre triee par reference et par num d'unite
*/
{
int ii;
EDA_SchComponentStruct * LibItem;
const wxString * OldName = NULL;

	for( ii = 0; ii < NbItems ; ii++ )
	{
		LibItem = (EDA_SchComponentStruct *) List[ii];
		if ( LibItem == NULL ) continue;
		if( OldName )
		{
			if ( OldName->CmpNoCase( LibItem->m_Field[REFERENCE].m_Text ) == 0 )
			{
				List[ii] = NULL;
			}
		}
		OldName = &LibItem->m_Field[REFERENCE].m_Text;
	}
}


/**********************************************************************/
int PrintListeCmpByRef( FILE * f, EDA_BaseStruct ** List, int NbItems )
/**********************************************************************/
/* Impression de la liste des composants tries par reference
*/
{
int ii, Multi, Unit;
EDA_BaseStruct *DrawList;
EDA_SchComponentStruct *DrawLibItem;
EDA_LibComponentStruct *Entry;
char NameCmp[80];
wxString msg;
	
	msg = _("\n#Cmp ( order = Reference )");
	if ( s_ListWithSubCmponents ) msg << _(" (with SubCmp)");
	fprintf( f, "%s\n", CONV_TO_UTF8(msg));

	for ( ii = 0; ii < NbItems; ii++ )
		{
		DrawList = List[ii];

		if( DrawList == NULL ) continue;
		if( DrawList->m_StructType != DRAW_LIB_ITEM_STRUCT_TYPE ) continue;

		DrawLibItem = (EDA_SchComponentStruct *) DrawList;
		if( DrawLibItem->m_Field[REFERENCE].m_Text[0] == '#' ) continue;

		Multi = 0; Unit = ' ';
		Entry = FindLibPart(DrawLibItem->m_ChipName.GetData(), wxEmptyString, FIND_ROOT);
		if( Entry ) Multi = Entry->m_UnitCount;
		if( (Multi > 1 ) && s_ListWithSubCmponents )
			 Unit = DrawLibItem->m_Multi + 'A' - 1;

		sprintf( NameCmp,"%s%c", CONV_TO_UTF8(DrawLibItem->m_Field[REFERENCE].m_Text),
			Unit);
		fprintf(f, "| %-10.10s %-12.12s",
					NameCmp,
					CONV_TO_UTF8(DrawLibItem->m_Field[VALUE].m_Text));

		if ( s_ListWithSubCmponents )
			{
			BASE_SCREEN * screen = (BASE_SCREEN *)(DrawLibItem->m_Parent);
			wxString sheetname;
			if( screen->m_Parent )
				sheetname = ((DrawSheetStruct*)screen->m_Parent)->m_Field[VALUE].m_Text.GetData();
			else sheetname = _("Root");
			fprintf(f, "   (Sheet %.2d: \"%s\")", DrawLibItem->m_FlagControlMulti,
					CONV_TO_UTF8(sheetname));
			}

		fprintf(f,"\n");
		}
	msg = _("#End Cmp\n");
	fprintf(f, CONV_TO_UTF8(msg));
	return(0);
}

/***********************************************************************/
int PrintListeCmpByVal( FILE * f, EDA_BaseStruct ** List, int NbItems )
/**********************************************************************/
{
int ii, Multi;
wxChar Unit;
EDA_BaseStruct *DrawList;
EDA_SchComponentStruct *DrawLibItem;
EDA_LibComponentStruct *Entry;
wxString msg;
	
	msg = _("\n#Cmp ( order = Value )");
	if ( s_ListWithSubCmponents ) msg <<  _(" (with SubCmp)");
	msg << wxT("\n");
	fprintf(f, CONV_TO_UTF8(msg));

	for ( ii = 0; ii < NbItems; ii++ )
		{
		DrawList = List[ii];

		if( DrawList == NULL ) continue;
		if( DrawList->m_StructType != DRAW_LIB_ITEM_STRUCT_TYPE ) continue;

		DrawLibItem = (EDA_SchComponentStruct *) DrawList;
		if( DrawLibItem->m_Field[REFERENCE].m_Text[0] == '#' ) continue;

		Multi = 0; Unit = ' ';
		Entry = FindLibPart(DrawLibItem->m_ChipName.GetData(), wxEmptyString, FIND_ROOT);
		if( Entry ) Multi = Entry->m_UnitCount;
		if( (Multi > 1 ) && s_ListWithSubCmponents )
			 Unit = DrawLibItem->m_Multi + 'A' - 1;
		msg = DrawLibItem->m_Field[REFERENCE].m_Text;
		msg.Append(Unit);

		fprintf(f, "| %-12.12s %-10.10s",
					CONV_TO_UTF8(DrawLibItem->m_Field[VALUE].m_Text),
					CONV_TO_UTF8(msg) );
		if ( s_ListWithSubCmponents )
		{
			fprintf(f, "   (Sheet %.2d)", DrawLibItem->m_FlagControlMulti);
		}

		fprintf(f,"\n");
	}
	msg = _("#End Cmp\n");
	fprintf(f, CONV_TO_UTF8(msg));
	return(0);
}


/******************************************************************/
static int PrintListeGLabel( FILE *f, ListLabel *List, int NbItems)
/******************************************************************/
{
int ii, jj;
DrawGlobalLabelStruct *DrawTextItem;
DrawSheetLabelStruct * DrawSheetLabel;
ListLabel * LabelItem;
wxString msg;
	
	for ( ii = 0; ii < NbItems; ii++ )
	{
		LabelItem = & List[ii];

		switch( LabelItem->m_StructType )
		{
			case DRAW_GLOBAL_LABEL_STRUCT_TYPE :
				DrawTextItem = (DrawGlobalLabelStruct *)(LabelItem->m_Label);
				msg.Printf(
                        _("> %-28.28s Global        (Sheet %.2d) pos: %3.3f, %3.3f\n"),
							DrawTextItem->m_Text.GetData(),
							LabelItem->m_SheetNumber,
							(float)DrawTextItem->m_Pos.x / 1000,
							(float)DrawTextItem->m_Pos.y / 1000);
				
				fprintf(f, CONV_TO_UTF8(msg));
 				break;

			case DRAW_SHEETLABEL_STRUCT_TYPE :
			{
				DrawSheetLabel = (DrawSheetLabelStruct *) LabelItem->m_Label;
				jj = DrawSheetLabel->m_Shape;
				if ( jj < 0 ) jj = NET_TMAX; if ( jj > NET_TMAX ) jj = 4;
				wxString labtype = CONV_FROM_UTF8(SheetLabelType[jj]);
				msg.Printf(
                        _("> %-28.28s Sheet %-7.7s (Sheet %.2d) pos: %3.3f, %3.3f\n"),
							DrawSheetLabel->m_Text.GetData(),
							labtype.GetData(),
							LabelItem->m_SheetNumber,
							(float)DrawSheetLabel->m_Pos.x / 1000,
							(float)DrawSheetLabel->m_Pos.y / 1000);
				fprintf(f, CONV_TO_UTF8(msg));
			}
				break;

			default: break;
		}
	}
	msg = _("#End labels\n");
	fprintf(f, CONV_TO_UTF8(msg));
 	return(0);
}

