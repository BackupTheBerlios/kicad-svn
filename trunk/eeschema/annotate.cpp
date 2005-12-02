	/**************************************/
	/* annotate.cpp: component annotation */
	/**************************************/

#include "fctsys.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"
#include "netlist.h"

#include "protos.h"

/* fonctions exportees */
int ListeComposants( CmpListStruct * BaseListeCmp, SCH_SCREEN *Window, int NumSheet);
int AnnotTriComposant(CmpListStruct *Objet1, CmpListStruct *Objet2);
void BreakReference( CmpListStruct * BaseListeCmp,int NbOfCmp );

/* fonctions locales */
static void ReNumComposants( CmpListStruct * BaseListeCmp,int NbOfCmp );
static void ComputeReferenceNumber( CmpListStruct * BaseListeCmp,int NbOfCmp);
static int GetLastReferenceNumber(CmpListStruct *Objet, CmpListStruct * BaseListeCmp,
														int NbOfCmp);
static int ExistUnit(CmpListStruct *Objet, int Unit,
								 CmpListStruct * BaseListeCmp,int NbOfCmp);

/* Variable locales */
static bool AnnotProject = TRUE;

enum id_annotate {
	ID_ANNOTATE_CMP = 1300,
	ID_DEANNOTATE_CMP,
	ID_CLOSE_ANNOTATE,
	ID_SEL_PROJECT,
	ID_SEL_NEWCMP_ONLY
};

/* Dialog frame for annotation control */
class WinEDA_AnnotateFrame: public wxDialog
{
private:
	WinEDA_DrawFrame * m_Parent;
	wxRadioBox * m_AnnotProjetCtrl;
	wxRadioBox * m_AnnotNewCmpCtrl;
	bool m_Abort;

public:
	// Constructor and destructor
	WinEDA_AnnotateFrame(WinEDA_DrawFrame *parent, wxPoint& pos);
	~WinEDA_AnnotateFrame(void) {};
private:
	void AnnotateComponents(wxCommandEvent& event);
	void DeleteAnnotation(wxCommandEvent& event);
	void SelLocal(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};
BEGIN_EVENT_TABLE(WinEDA_AnnotateFrame, wxDialog)
	EVT_BUTTON(ID_CLOSE_ANNOTATE, WinEDA_AnnotateFrame::OnQuit)
	EVT_BUTTON(ID_ANNOTATE_CMP, WinEDA_AnnotateFrame::AnnotateComponents)
	EVT_BUTTON(ID_DEANNOTATE_CMP, WinEDA_AnnotateFrame::DeleteAnnotation)
	EVT_RADIOBOX(ID_SEL_PROJECT, WinEDA_AnnotateFrame::SelLocal)
END_EVENT_TABLE()


void InstallAnnotateFrame(WinEDA_DrawFrame *parent, wxPoint & pos)
{
	WinEDA_AnnotateFrame * frame = new WinEDA_AnnotateFrame(parent, pos);
	frame->ShowModal(); frame->Destroy();
}


#define X_Size 290
#define Y_Size 230

/***************************************************************************************/
WinEDA_AnnotateFrame::WinEDA_AnnotateFrame(WinEDA_DrawFrame *parent, wxPoint& framepos):
		wxDialog(parent, -1, _("EESchema Annotation"), framepos, wxSize(X_Size, Y_Size),
				 DIALOG_STYLE)
/***************************************************************************************/
{
wxPoint pos;
int w, h;

	m_Parent = parent;
	m_Abort = FALSE;

	SetFont(*g_DialogFont);

	pos.x = 5; pos.y = 5;
	if ( (framepos.x == -1) && (framepos.y == -1) ) Centre();

wxString choice_project_current[] =
	{_("Hierarchy"), _("Current sheet") };
	m_AnnotProjetCtrl = new wxRadioBox(this, ID_SEL_PROJECT,
						_("annotate:"),
						pos,wxSize(-1,-1),
						2,choice_project_current,1,wxRA_SPECIFY_COLS);
	if ( AnnotProject == FALSE )m_AnnotProjetCtrl->SetSelection(1);

	m_AnnotProjetCtrl->GetSize(&w, &h);

	/* Create the command buttons */
	pos.x = 160; pos.y = 10;
	wxButton * Button = new wxButton(this, ID_ANNOTATE_CMP,
						_("&Annotate"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.y += Button->GetSize().y + 5;
	Button = new wxButton(this,	ID_DEANNOTATE_CMP,
						_("&Del Annotate"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.y += Button->GetSize().y + 5;
	new wxButton(this,	ID_CLOSE_ANNOTATE,
						_("&Close"), pos);
	Button->SetForegroundColour(*wxBLUE);

	pos.x = 5; pos.y = 150;
wxString choice_all_cmp[] =
	{_("all components"), _("new components only") };
	m_AnnotNewCmpCtrl = new wxRadioBox(this, ID_SEL_NEWCMP_ONLY,
						_("select items:"),
						pos,wxSize(-1,-1),
						2,choice_all_cmp,1,wxRA_SPECIFY_COLS);
	m_AnnotNewCmpCtrl->SetSelection(1);

	pos.y += m_AnnotNewCmpCtrl->GetSize().y + 10;

	SetClientSize(wxSize(X_Size, pos.y));
}


/*******************************************************************/
/* Fonctions de positionnement des variables d'option d'annotation */
/*******************************************************************/
void WinEDA_AnnotateFrame::SelLocal(wxCommandEvent& event)
{
int ii = m_AnnotProjetCtrl->GetSelection();
	AnnotProject = TRUE;
	if ( ii == 1 ) AnnotProject = FALSE;
}


/******************************************************************/
void WinEDA_AnnotateFrame::AnnotateComponents(wxCommandEvent& event)
/******************************************************************/
/*
	Compute the annotation of the components for the whole projeect, or the current sheet only.
	All the components or the new ones only will be annotated.
*/
{
int NbSheet, ii , NbOfCmp;
SCH_SCREEN *Window;
CmpListStruct * BaseListeCmp;

	wxBusyCursor dummy;

	/* If it is an annotation for all the components, reset previous annotation: */
	if( m_AnnotNewCmpCtrl->GetSelection() == 0 ) DeleteAnnotation(event);
	if (m_Abort ) return;

	/* Update sheet numbers */
	Window = ScreenSch;
	Window->SetModify();

	/* Count hierarchy sheet number*/
	for ( NbSheet = 0 ; Window != NULL; Window = (SCH_SCREEN*)Window->Pnext )
		NbSheet++;

	Window = ScreenSch;
	for ( ii = 1; Window != NULL; Window = (SCH_SCREEN*)Window->Pnext, ii++ )
	{
		Window->m_SheetNumber = ii; Window->m_NumberOfSheet = NbSheet;
		Window->m_Date = GenDate();		/* Update the sheet date */
	}

	/* First pass: Component counting */

	Window = (SCH_SCREEN*) m_Parent->m_CurrentScreen;
	if( AnnotProject == TRUE )
	{
		NbOfCmp = 0; Window = ScreenSch;
		for ( ; Window != NULL; Window = (SCH_SCREEN*)Window->Pnext )
		{
			NbOfCmp += ListeComposants(NULL, Window, Window->m_SheetNumber);
		}
	}
	else NbOfCmp = ListeComposants(NULL, Window, Window->m_SheetNumber);

	if( NbOfCmp == 0 )
	{
		wxBell();
		return;
	}

	ii = sizeof(CmpListStruct) * NbOfCmp;
	BaseListeCmp = (CmpListStruct *) MyZMalloc(ii);

	/* Second pass : Int data tables */
	if( AnnotProject == TRUE )
	{
		ii = 0; Window = ScreenSch;
		for ( ; Window != NULL; Window = (SCH_SCREEN*)Window->Pnext )
		{
			ii += ListeComposants(BaseListeCmp + ii,
									Window, Window->m_SheetNumber);
		}
	}

	else ii = ListeComposants(BaseListeCmp, Window, Window->m_SheetNumber);

	if( ii != NbOfCmp )
		DisplayError(this, "Internal error in AnnotateComponents()");

	/* Separation des Numeros de la reference: IC1 -> IC, et 1 dans .m_NumRef */
	BreakReference(BaseListeCmp, NbOfCmp);

	qsort( BaseListeCmp, NbOfCmp, sizeof(CmpListStruct),
				(int(*)(const void*, const void*))AnnotTriComposant);

	/* Recalcul des numeros de reference */
	ComputeReferenceNumber(BaseListeCmp, NbOfCmp);
	ReNumComposants(BaseListeCmp, NbOfCmp);

	MyFree(BaseListeCmp); BaseListeCmp = NULL;

	/* Final control */
	CheckAnnotate(m_Parent, AnnotProject ? FALSE : TRUE);

	m_Parent->DrawPanel->Refresh(TRUE);	/* Refresh screen */
	Close();
}

/************************************************************************/
void  WinEDA_AnnotateFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
/************************************************************************/
{
     Close(true);   // true is to force the frame to close
}


/********************************************************************/
void WinEDA_AnnotateFrame::DeleteAnnotation(wxCommandEvent& event)
/********************************************************************/
/* Clear the current annotation for the whole project or only for the current sheet
	Update sheet number and number of sheets
*/
{
int NbSheet, ii;
SCH_SCREEN * screen;
EDA_SchComponentStruct *DrawLibItem;

	if( !IsOK(this, _("Previous Annotation will be deleted. Continue ?") ) )
	{
		m_Abort = TRUE; return;
	}

	m_Abort = FALSE;

	/* Update the sheet number and number of sheets*/
	screen = ScreenSch;
	screen->SetModify();

	/* Count hierarchy sheets*/
	for ( NbSheet = 0 ; screen != NULL; screen = (SCH_SCREEN*)screen->Pnext )
		NbSheet++;

	screen = ScreenSch;
	for ( ii = 1; screen != NULL; screen = (SCH_SCREEN*)screen->Pnext, ii++ )
	{
		screen->m_SheetNumber = ii;			/* Update the sheet number */
		screen->m_NumberOfSheet = NbSheet;	/* Update the number of sheets */
		screen->m_Date = GenDate();			/* Update the sheet date */
	}

	if( AnnotProject == TRUE ) screen = ScreenSch;
	else screen = (SCH_SCREEN*) m_Parent->m_CurrentScreen;

	for ( ; screen != NULL; screen = (SCH_SCREEN*)screen->Pnext )
	{
		EDA_BaseStruct *DrawList = screen->EEDrawList;
		for ( ; DrawList != NULL ; DrawList = DrawList->Pnext )
		{
			if ( DrawList->m_StructType == DRAW_LIB_ITEM_STRUCT_TYPE )
			{
				DrawLibItem = (EDA_SchComponentStruct *) DrawList;
				DrawLibItem->ClearAnnotation();
			}
		}
		if( ! AnnotProject ) break;
	}

	m_Parent->DrawPanel->Refresh(TRUE);
	Close();
}


/************************************************************************************/
int ListeComposants(CmpListStruct * BaseListeCmp, SCH_SCREEN *Window, int NumSheet)
/***********************************************************************************/
/*	if BaseListeCmp == NULL : Components counting
	else update data table BaseListeCmp
*/
{
int NbrCmp = 0;
EDA_BaseStruct *DrawList = Window->EEDrawList;
EDA_SchComponentStruct *DrawLibItem;
EDA_LibComponentStruct *Entry;

	DrawList = Window->EEDrawList;

	while ( DrawList )
		{
		switch( DrawList->m_StructType )
			{
			case DRAW_SEGMENT_STRUCT_TYPE :
			case DRAW_JUNCTION_STRUCT_TYPE :
			case DRAW_TEXT_STRUCT_TYPE :
			case DRAW_LABEL_STRUCT_TYPE :
			case DRAW_GLOBAL_LABEL_STRUCT_TYPE :
				break;

			case DRAW_LIB_ITEM_STRUCT_TYPE :
				DrawLibItem = (EDA_SchComponentStruct *) DrawList;
  				Entry = FindLibPart(DrawLibItem->m_ChipName, "", FIND_ROOT);
 				if( Entry == NULL) break;
				if ( BaseListeCmp == NULL )	/* Items counting only */
				{
					NbrCmp++; break;
				}
				BaseListeCmp[NbrCmp].m_Cmp = DrawLibItem;
				BaseListeCmp[NbrCmp].m_NbParts = Entry->m_UnitCount;
				BaseListeCmp[NbrCmp].m_Unit = DrawLibItem->m_Multi;
				BaseListeCmp[NbrCmp].m_PartsLocked = Entry->m_UnitSelectionLocked;
				BaseListeCmp[NbrCmp].m_Sheet = NumSheet;
				BaseListeCmp[NbrCmp].m_IsNew = FALSE;
				BaseListeCmp[NbrCmp].m_TimeStamp = DrawLibItem->m_TimeStamp;
				if( DrawLibItem->m_Field[REFERENCE].m_Text == "" )
					DrawLibItem->m_Field[REFERENCE].m_Text = "DefRef?";
				strncpy( BaseListeCmp[NbrCmp].m_TextRef,
						DrawLibItem->m_Field[REFERENCE].m_Text.GetData(), 32);

				BaseListeCmp[NbrCmp].m_NumRef = -1;

				if( DrawLibItem->m_Field[VALUE].m_Text == "" )
					DrawLibItem->m_Field[VALUE].m_Text = "~";
				strncpy( BaseListeCmp[NbrCmp].m_TextValue,
						DrawLibItem->m_Field[VALUE].m_Text.GetData(), 32);
				NbrCmp++;
				break;

			case DRAW_PICK_ITEM_STRUCT_TYPE :
			case DRAW_POLYLINE_STRUCT_TYPE :
			case DRAW_BUSENTRY_STRUCT_TYPE :
			case DRAW_SHEET_STRUCT_TYPE :
			case DRAW_SHEETLABEL_STRUCT_TYPE :
			case DRAW_MARKER_STRUCT_TYPE :
			case DRAW_NOCONNECT_STRUCT_TYPE :
				break;

			default :
				break;
			}

		DrawList = DrawList->Pnext;
		}

return(NbrCmp);
}

/*****************************************************************/
int AnnotTriComposant(CmpListStruct *Objet1, CmpListStruct *Objet2)
/****************************************************************/
/* function used par qsort() for sorting the list
	Composants are sorted
		by reference
		if same reference: by value
			if same value: by unit number
				if same unit number, by sheet
					if same sheet, by time stamp
**/
{
int ii;

	ii = strnicmp( Objet1->m_TextRef, Objet2->m_TextRef, 32 );

	if ( ii == 0 ) ii = strnicmp( Objet1->m_TextValue, Objet2->m_TextValue, 32 );
	if ( ii == 0 ) ii = Objet1->m_Unit - Objet2->m_Unit;
	if ( ii == 0 ) ii = Objet1->m_Sheet - Objet2->m_Sheet;
	if ( ii == 0 ) ii = Objet1->m_TimeStamp - Objet2->m_TimeStamp;

	return(ii);
}


/********************************************************************/
static void ReNumComposants( CmpListStruct * BaseListeCmp, int NbOfCmp)
/********************************************************************/
/* Update the reference component for the schematic project (or the current sheet)
*/
{
int ii;
char *Text;
EDA_SchComponentStruct *DrawLibItem;

	/* Reattribution des numeros */
	for ( ii = 0; ii < NbOfCmp ; ii++ )
	{
		Text = BaseListeCmp[ii].m_TextRef;
		DrawLibItem = BaseListeCmp[ii].m_Cmp;

		if ( BaseListeCmp[ii].m_NumRef < 0 ) strcat( Text, "?" );
		else sprintf( Text + strlen(Text),"%d",BaseListeCmp[ii].m_NumRef );

		DrawLibItem->m_Field[REFERENCE].m_Text = Text;
		DrawLibItem->m_Multi = BaseListeCmp[ii].m_Unit;
	}
}



/**************************************************************/
void BreakReference( CmpListStruct * BaseListeCmp, int NbOfCmp)
/**************************************************************/

/* Modifie dans BaseListeCmp la reference des composants en supprimant la
	partie nombre de la partie texte.
Place le nombre dans .m_NumRef
Pour les composants multiples non encore annotes, met .m_Unit a sa valeur max
	Utilise:
		BaseListeCmp
		NbOfCmp
*/
{
int ii, ll;
char * Text;

	/* Separation des Numeros de la reference: IC1 -> IC, et 1 dans .m_NumRef */
	for ( ii = 0; ii < NbOfCmp ; ii++ )
		{
		BaseListeCmp[ii].m_NumRef = - 1;
		Text = BaseListeCmp[ii].m_TextRef;
		ll = strlen( Text ) - 1;
		if( Text[ll] == '?' )
		{
			BaseListeCmp[ii].m_IsNew = TRUE;
			if ( ! BaseListeCmp[ii].m_PartsLocked )
				BaseListeCmp[ii].m_Unit = 0x7FFFFFFF;
			Text[ll] = 0; continue;
		}

		if( isdigit(Text[ll]) == 0 )
		{
			BaseListeCmp[ii].m_IsNew = TRUE;
			if ( ! BaseListeCmp[ii].m_PartsLocked )
				BaseListeCmp[ii].m_Unit = 0x7FFFFFFF;
			continue;
		}

		while( ll >= 0 )
		{
			if( (Text[ll] <= ' ' ) || isdigit(Text[ll]) )
				ll--;
			else
			{
				if( isdigit(Text[ll+1]) )
					BaseListeCmp[ii].m_NumRef = atoi(& Text[ll+1]);
				Text[ll+1] = 0;
				break;
			}
		}
	}
}


/*****************************************************************************/
static void ComputeReferenceNumber( CmpListStruct * BaseListeCmp, int NbOfCmp)
/*****************************************************************************/
/* Compute the reference number for components without reference number
	Compute	.m_NumRef member
*/
{
int ii, jj, LastReferenceNumber, NumberOfUnits, Unit;
char * Text, * RefText, *ValText;
CmpListStruct * ObjRef, * ObjToTest;

	/* Components with an invisible reference (power...) always are re-annotated*/
	for ( ii = 0; ii < NbOfCmp ; ii++ )
	{
		Text = BaseListeCmp[ii].m_TextRef;
		if (*Text == '#')
		{
			BaseListeCmp[ii].m_IsNew = TRUE;
			BaseListeCmp[ii].m_NumRef = 0;
		}
	}

	ValText = RefText = ""; LastReferenceNumber = 1;
	for ( ii = 0; ii < NbOfCmp ; ii++ )
	{
		ObjRef = & BaseListeCmp[ii];
		if( BaseListeCmp[ii].m_Flag ) continue;

		Text = BaseListeCmp[ii].m_TextRef;
		if( strnicmp(RefText, Text, 32) != 0 ) /* Nouveau Identificateur */
		{
			RefText = BaseListeCmp[ii].m_TextRef;
			LastReferenceNumber = GetLastReferenceNumber(BaseListeCmp + ii, BaseListeCmp, NbOfCmp);
		}

		/* Annotation of mono-part components ( 1 part per package ) (trivial case)*/
		if( BaseListeCmp[ii].m_NbParts <= 1 )
		{
			if ( BaseListeCmp[ii].m_IsNew )
			{
				LastReferenceNumber++;
				BaseListeCmp[ii].m_NumRef = LastReferenceNumber;
			}
			BaseListeCmp[ii].m_Unit = 1;
			BaseListeCmp[ii].m_Flag = 1;
			BaseListeCmp[ii].m_IsNew = FALSE;
			continue;
		}

		/* Annotation of multi-part components ( n parts per package ) (complex case) */
		ValText = BaseListeCmp[ii].m_TextValue;
		NumberOfUnits = BaseListeCmp[ii].m_NbParts;

		if ( BaseListeCmp[ii].m_IsNew )
		{
			LastReferenceNumber++; BaseListeCmp[ii].m_NumRef = LastReferenceNumber;
			if ( ! BaseListeCmp[ii].m_PartsLocked )
				BaseListeCmp[ii].m_Unit = 1;
			BaseListeCmp[ii].m_Flag = 1;
		}

		for( Unit = 1; Unit <= NumberOfUnits; Unit++ )
		{
			if( BaseListeCmp[ii].m_Unit == Unit ) continue;
			jj = ExistUnit( BaseListeCmp + ii , Unit, BaseListeCmp, NbOfCmp );
			if ( jj >= 0 ) continue;	/* Unit exists for this reference */

			/* Search a component to annotate ( same prefix, same value) */
			for ( jj = ii+1; jj < NbOfCmp ; jj++ )
			{
				ObjToTest = &BaseListeCmp[jj];
				if( BaseListeCmp[jj].m_Flag ) continue;
				Text = BaseListeCmp[jj].m_TextRef;
				if( strnicmp(RefText, Text, 32) != 0 ) break;	// references are different
				Text = BaseListeCmp[jj].m_TextValue;
				if( strnicmp(ValText, Text, 32) != 0 ) break;	// values are different
				if ( ! BaseListeCmp[jj].m_IsNew )
				{
					//BaseListeCmp[jj].m_Flag = 1;
					continue;
				}
				/* Component without reference number found, annotate it if possible */
				if ( ! BaseListeCmp[jj].m_PartsLocked || (BaseListeCmp[jj].m_Unit == Unit) )
				{
					BaseListeCmp[jj].m_NumRef = BaseListeCmp[ii].m_NumRef;
					BaseListeCmp[jj].m_Unit = Unit;
					BaseListeCmp[jj].m_Flag = 1;
					BaseListeCmp[jj].m_IsNew = FALSE;
					break;
				}
			}
		}
	}
}


/*************************************************************************************************/
static int GetLastReferenceNumber(CmpListStruct *Objet, CmpListStruct * BaseListeCmp, int NbOfCmp)
/*************************************************************************************************/
/* Recherche le plus grand numero de reference dans les composants
	de meme prefixe de reference que celui pointe par Objet
	la liste des composants est supposee triee
*/
{
CmpListStruct * LastObjet = BaseListeCmp + NbOfCmp;
int LastNumber = 0;
const char * RefText;

	RefText = Objet->m_TextRef;
	for ( ; Objet < LastObjet; Objet++ )
	{
		if( strnicmp(RefText, Objet->m_TextRef, 32) != 0 ) /* Nouveau Identificateur */
			break;
		if( LastNumber < Objet->m_NumRef ) LastNumber = Objet->m_NumRef;
	}
	return(LastNumber);
}


/*****************************************************************/
static int ExistUnit(CmpListStruct *Objet, int Unit,
						CmpListStruct * BaseListeCmp, int NbOfCmp)
/****************************************************************/
/* Recherche dans la liste triee des composants, pour les composants
	multiples s'il existe pour le composant de reference Objet,
	une unite de numero Unit
		Retourne index dans BaseListeCmp si oui
		retourne -1 si non
*/
{
CmpListStruct * EndList = BaseListeCmp + NbOfCmp;
char * RefText, * ValText;
int NumRef, ii;
CmpListStruct *ItemToTest;

	RefText = Objet->m_TextRef;
	ValText = Objet->m_TextValue;
	NumRef = Objet->m_NumRef;
	for ( ItemToTest = BaseListeCmp, ii = 0; ItemToTest < EndList; ItemToTest++, ii++ )
	{
		if ( Objet == ItemToTest ) continue;
		if ( ItemToTest->m_IsNew ) continue;  /* non affecte */
		if ( ItemToTest->m_NumRef != NumRef ) continue;
		if( strnicmp(RefText, ItemToTest->m_TextRef, 32) != 0 ) /* Nouveau Identificateur */
			continue;
		if( ItemToTest->m_Unit == Unit)
		{
			return(ii);
		}
	}
	return(-1);
}

/************************************************************/
int CheckAnnotate(WinEDA_DrawFrame * frame, bool OneSheetOnly)
/************************************************************/

/* Retourne le nombre de composants non annot�s ou de meme r�f�rence (doubl�s)
	Si OneSheetOnly : recherche sur le schema courant
	sinon: recherche sur toute la hierarchie
*/
{
int NbSheet, ii, NumSheet = 1, error, NbOfCmp;
SCH_SCREEN *Window;
CmpListStruct * ListeCmp = NULL;
char Buff[80], BufLine[1024];

	/* Comptage du nombre de feuilles de hierarchy */
	Window = ScreenSch;
	for ( NbSheet = 0 ; Window != NULL; Window = (SCH_SCREEN*)Window->Pnext )
		NbSheet++;

	Window = ScreenSch;
	for ( ii = 1; Window != NULL; Window = (SCH_SCREEN*)Window->Pnext, ii++ )
		{
		Window->m_SheetNumber = ii; Window->m_NumberOfSheet = NbSheet;
		}

	/* 1ere passe : Comptage du nombre de composants */

	Window = (SCH_SCREEN*) frame->m_CurrentScreen;
	if( OneSheetOnly == 0 )
		{
		NbOfCmp = 0; Window = ScreenSch;
		for ( ; Window != NULL; Window = (SCH_SCREEN*)Window->Pnext )
			{
			NbOfCmp += ListeComposants(NULL, Window, NumSheet);
			}
		}
	else NbOfCmp = ListeComposants(NULL, Window, NumSheet);

	if( NbOfCmp == 0 )
		{
		wxBell();
		return(0);
		}

	ii = sizeof(CmpListStruct) * NbOfCmp;
	ListeCmp = (CmpListStruct *) MyZMalloc(ii);

	/* 2eme passe : Remplissage du tableau des caracteristiques */
	if( OneSheetOnly == 0 )
		{
		ii = 0; Window = ScreenSch;
		for ( ; Window != NULL; Window = (SCH_SCREEN*)Window->Pnext )
			{
			ii += ListeComposants(ListeCmp + ii, Window, NumSheet);
			}
		}
	else ListeComposants(ListeCmp, Window, NumSheet);

	qsort( ListeCmp, NbOfCmp, sizeof(CmpListStruct),
				(int(*)(const void*, const void*))AnnotTriComposant);


	/* Separation des Numeros de la reference: IC1 -> IC, et 1 dans .m_NumRef */
	BreakReference(ListeCmp, NbOfCmp);

	/* comptage des elements non annotes */
	error = 0;
	for( ii = 0; ii < NbOfCmp-1 ; ii++ )
		{
		if ( ListeCmp[ii].m_IsNew )
			{
			if( ListeCmp[ii].m_NumRef >= 0 )
				sprintf( Buff,"%d",ListeCmp[ii].m_NumRef);
			else strcpy(Buff,"?");
			sprintf( BufLine,_("item not annotated: %s%s"), ListeCmp[ii].m_TextRef,Buff);

			if( (ListeCmp[ii].m_Unit > 0) && (ListeCmp[ii].m_Unit < 0x7FFFFFFF) )
				sprintf( BufLine+strlen(BufLine)," (unit %d)",
										ListeCmp[ii].m_Unit);

			DisplayError(NULL, BufLine);
			error++; break;
			}

		if( MAX(ListeCmp[ii].m_NbParts, 1) < ListeCmp[ii].m_Unit  )	// Annotate error
			{
			if( ListeCmp[ii].m_NumRef >= 0 )
					sprintf( Buff,"%d",ListeCmp[ii].m_NumRef);
			else strcpy(Buff,"?");

			sprintf( BufLine,_("Error item %s%s"),
						ListeCmp[ii].m_TextRef,Buff);

			sprintf( BufLine+strlen(BufLine),_(" unit %d and no more than %d parts"),
										ListeCmp[ii].m_Unit, ListeCmp[ii].m_NbParts);

			DisplayError(frame, BufLine);
			error++; break;
			}

		}

	if ( error ) return error;

	/* comptage des elements doubl�s (si tous sont annot�s) */
	for( ii = 0; (ii < NbOfCmp-1) && (error < 4); ii++ )
		{
		if( (stricmp(ListeCmp[ii].m_TextRef,ListeCmp[ii+1].m_TextRef) != 0) ||
			( ListeCmp[ii].m_NumRef != ListeCmp[ii+1].m_NumRef ) )
			continue;
		/* Meme reference trouv�e */

		/* Il y a erreur si meme unite */
		if( ListeCmp[ii].m_Unit == ListeCmp[ii+1].m_Unit )
			{

			if( ListeCmp[ii].m_NumRef >= 0 )
					sprintf( Buff,"%d",ListeCmp[ii].m_NumRef);
			else strcpy(Buff,"?");

			sprintf( BufLine,_("Multiple item %s%s"),
						ListeCmp[ii].m_TextRef,Buff);

			if( (ListeCmp[ii].m_Unit > 0) && (ListeCmp[ii].m_Unit < 0x7FFFFFFF) )
					sprintf( BufLine+strlen(BufLine)," (unit %d)",
										ListeCmp[ii].m_Unit);

			DisplayError(frame, BufLine);
			error++; continue;
			}

		/* Il y a erreur si unites differentes mais nombre de parts differentes
		par boitier (ex U3 ( 1 part) et U3B sont incompatibles) */
		if( ListeCmp[ii].m_NbParts != ListeCmp[ii+1].m_NbParts )
			{

			if( ListeCmp[ii].m_NumRef >= 0 )
					sprintf( Buff,"%d",ListeCmp[ii].m_NumRef);
			else strcpy(Buff,"?");

			sprintf( BufLine,_("Multiple item %s%s"),
						ListeCmp[ii].m_TextRef,Buff);

			if( (ListeCmp[ii].m_Unit > 0) && (ListeCmp[ii].m_Unit < 0x7FFFFFFF) )
					sprintf( BufLine+strlen(BufLine)," (unit %d)",
										ListeCmp[ii].m_Unit);

			DisplayError(frame, BufLine);
			error++;
			}

		/* Il y a erreur si unites differentes ET valeurs diff�rentes */
		if( stricmp(ListeCmp[ii].m_TextValue,ListeCmp[ii+1].m_TextValue) != 0)
			{
			sprintf( BufLine,_("Diff values for %s%d%c (%s) et %s%d%c (%s)"),
						ListeCmp[ii].m_TextRef, ListeCmp[ii].m_NumRef, ListeCmp[ii].m_Unit+'A'-1,
						ListeCmp[ii].m_TextValue,
						ListeCmp[ii+1].m_TextRef, ListeCmp[ii+1].m_NumRef, ListeCmp[ii+1].m_Unit+'A'-1,
						ListeCmp[ii+1].m_TextValue);

			DisplayError(frame, BufLine);
			error++;
			}
		}

	MyFree(ListeCmp);
	return(error);
}


