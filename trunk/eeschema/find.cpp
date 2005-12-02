	/****************************************************************/
	/* EESchema: find.cpp (functions for seraching a schematic item */
	/****************************************************************/
/*
	Search a text (text, value, reference) withing e composent or
	search a composant in libraries, a marker ...,
	in current sheet or whole the project
*/
#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"

#include "protos.h"

/* Variables Locales */
static int ItemsCount, MarkerCount;
static wxString s_OldStringFound;


	/*************************************************/
	/* classe derivee pour la frame de Config de Find */
	/*************************************************/

enum find_id {
	FIND_SHEET = 1800,
	FIND_HIERARCHY,
	FIND_NEXT,
	LOCATE_IN_LIBRARIES,
	FIND_MARKERS,
	FIND_NEXT_MARKER
};

/***************************************/
class WinEDA_FindFrame: public wxDialog
/***************************************/
{
public:

	WinEDA_SchematicFrame * m_Parent;
	WinEDA_EnterText * m_NewTextCtrl;

	// Constructor and destructor
	WinEDA_FindFrame(WinEDA_SchematicFrame *parent, wxPoint& pos);
	~WinEDA_FindFrame(void) {};

	void FindSchematicItem(wxCommandEvent& event);
	void FindMarker(wxCommandEvent& event);
	void LocatePartInLibs(wxCommandEvent& event);
	int ExploreAllLibraries(const wxString & wildmask, wxString & FindList);


	DECLARE_EVENT_TABLE()

};
/* Construction de la table des evenements pour FrameClassMain */
BEGIN_EVENT_TABLE(WinEDA_FindFrame, wxDialog)
	EVT_BUTTON(FIND_SHEET, WinEDA_FindFrame::FindSchematicItem)
	EVT_BUTTON(FIND_HIERARCHY, WinEDA_FindFrame::FindSchematicItem)
	EVT_BUTTON(FIND_MARKERS, WinEDA_FindFrame::FindMarker)
	EVT_BUTTON(FIND_NEXT_MARKER, WinEDA_FindFrame::FindMarker)
	EVT_BUTTON(FIND_NEXT, WinEDA_FindFrame::FindSchematicItem)
	EVT_BUTTON(LOCATE_IN_LIBRARIES, WinEDA_FindFrame::LocatePartInLibs)
END_EVENT_TABLE()

#define H_SIZE 340
#define V_SIZE 80
/*******************************************************************************/
WinEDA_FindFrame::WinEDA_FindFrame(WinEDA_SchematicFrame *parent, wxPoint& framepos):
		wxDialog(parent, -1, _("EESchema Locate"), framepos,
				wxSize(H_SIZE, V_SIZE + 30), DIALOG_STYLE)
/*******************************************************************************/
{
wxPoint pos;
wxSize WinClientSize;
	
	m_Parent = parent;
	SetFont(*g_DialogFont);

	if ( (framepos.x == -1) && (framepos.x == -1) ) Centre();

	/* Creation des boutons de commande */
	pos.x = 5; pos.y = 50;
	wxButton * Button = new wxButton(this, FIND_SHEET,
						_("Item in &Sheet"), pos);

	pos.x += Button->GetSize().x + 5;
	Button = new wxButton(this, FIND_HIERARCHY,
						_("Item in &Hierarchy"), pos);

	pos.x += Button->GetSize().x + 5;
	Button = new wxButton(this, FIND_NEXT,
						_("Find &Next Item"), pos);
	Button->SetForegroundColour(wxColor(0,80,0));
	pos.x += Button->GetSize().x + 5;
	WinClientSize.x = pos.x;

	pos.x = 5;
	pos.y += Button->GetSize().y + 5;
	Button = new wxButton(this, LOCATE_IN_LIBRARIES,
						_("Cmp in &Lib"), pos);
	Button->SetForegroundColour(wxColor(100,0,0) );

	pos.x += Button->GetSize().x + 5;
	Button = new wxButton(this, FIND_MARKERS,
						_("Find Markers"), pos);
	Button->SetForegroundColour(*wxBLUE);

	pos.x += Button->GetSize().x + 5;
	Button = new wxButton(this, FIND_NEXT_MARKER,
						_("Next Marker"), pos);
	Button->SetForegroundColour(*wxBLUE);
	pos.x += Button->GetSize().x + 5;
	WinClientSize.x = MAX(WinClientSize.x, pos.x);

	pos.y += Button->GetSize().y + 5;
	WinClientSize.y = pos.y;

	SetClientSize( WinClientSize );

	pos.y = 15; pos.x = 5;
	wxSize size = GetClientSize();
	m_NewTextCtrl = new WinEDA_EnterText(this,
				_("Item to find:"), s_OldStringFound,
				pos, wxSize(size.x - 15,-1) );

}


/**************************************************************/
void InstallFindFrame(WinEDA_SchematicFrame *parent, wxPoint & pos)
/**************************************************************/
{
	parent->DrawPanel->m_IgnoreMouseEvents = TRUE;
	WinEDA_FindFrame * frame = new WinEDA_FindFrame(parent, pos);
	frame->ShowModal(); frame->Destroy();
	parent->DrawPanel->m_IgnoreMouseEvents = FALSE;
}


/**************************************************************/
void WinEDA_FindFrame::FindMarker(wxCommandEvent& event)
/**************************************************************/
/*  Search de markers in whole the hierarchy.
	Mouse cursor is put on the marker
	search the first marker, or next marker
*/
{
int id = event.GetId();
	if( id != FIND_NEXT_MARKER ) m_Parent->FindMarker(0);
	else m_Parent->FindMarker(1);

	Close();
	
}
/*****************************************************************/
EDA_BaseStruct * WinEDA_SchematicFrame::FindMarker(int SearchType)
/*****************************************************************/
/* Search de markers in whole the hierarchy.
	Mouse cursor is put on the marker
	SearchType = 0: searche th first marker, else search next marker
*/
{
SCH_SCREEN * Screen, * FirstScreen = NULL;
EDA_BaseStruct *DrawList, *FirstStruct = NULL, *Struct = NULL;
DrawMarkerStruct * Marker = NULL;
int NotFound, StartCount;
wxPoint firstpos, pos;
wxSize size = DrawPanel->GetClientSize();
wxPoint curpos;
bool force_recadre = FALSE;
wxString msg, WildText;
	
	g_LastSearchIsMarker = TRUE;
	Screen = ScreenSch;
	if( SearchType == 0 ) MarkerCount = 0;

	NotFound = TRUE; StartCount = 0;
	for ( ; Screen != NULL; Screen = Screen->Next() )
	{
		DrawList = Screen->EEDrawList;
		while ( DrawList && NotFound )
		{
			if(DrawList->m_StructType == DRAW_MARKER_STRUCT_TYPE )
			{
			Marker = (DrawMarkerStruct *) DrawList;
				NotFound = FALSE;
				pos = Marker->m_Pos;
				if ( FirstScreen == NULL )	/* First item found */
				{
					FirstScreen = Screen; firstpos = pos;
					FirstStruct = DrawList;
				}
	
				StartCount++;
				if( MarkerCount >= StartCount )
				{
					NotFound = TRUE;	/* Search for the next item */
				}
				else
				{
					Struct = DrawList; MarkerCount++; break ;
				}
			}
			DrawList = DrawList->Pnext;
		}
		if( NotFound == FALSE ) break;
	}

	if( NotFound && FirstScreen )
	{
		NotFound = 0; Screen = FirstScreen; Struct = FirstStruct;
		pos = firstpos; MarkerCount = 1;
	}

	if( NotFound == 0)
	{
		if ( Screen != GetScreen() )
		{
			Screen->SetZoom(GetScreen()->GetZoom() );
			m_CurrentScreen = ActiveScreen = Screen;
			force_recadre = TRUE;
		}

		Screen->m_Curseur = pos;
		curpos = DrawPanel->CursorScreenPosition();
		DrawPanel->GetViewStart(&m_CurrentScreen->m_StartVisu.x,
								&m_CurrentScreen->m_StartVisu.y);

		// calcul des coord curseur avec origine = screen
		curpos.x -= m_CurrentScreen->m_StartVisu.x;
		curpos.y -= m_CurrentScreen->m_StartVisu.y;

		/* Il y a peut-etre necessite de recadrer le dessin: */
		if( (curpos.x <= 0) || (curpos.x >= size.x-1) ||
			(curpos.y <= 0) || (curpos.y >= size.y) || force_recadre )
		{
			Recadre_Trace(TRUE);
		}
		else
		{
			GRMouseWarp(DrawPanel, curpos );
		}

		msg = _("Marker found in ") + Screen->m_FileName;
		Affiche_Message(msg);
	}

	else
	{
		Affiche_Message("");
		msg = _("Marker Not Found");
		DisplayError(NULL,msg, 10);
	}
	
	return Marker;
}


/**************************************************************/
void WinEDA_FindFrame::FindSchematicItem(wxCommandEvent& event)
/**************************************************************/
/* Find a string in schematic.
	Call to WinEDA_SchematicFrame::FindSchematicItem()
*/
{
int id = event.GetId();

	if( id == FIND_SHEET ) m_Parent->FindSchematicItem(m_NewTextCtrl->GetData(), 0);
	else if( id == FIND_HIERARCHY ) m_Parent->FindSchematicItem(m_NewTextCtrl->GetData(), 1);
	else if( id == FIND_NEXT ) m_Parent->FindSchematicItem("", 2);

	Close();
}

/************************************************************************/
EDA_BaseStruct * WinEDA_SchematicFrame::FindSchematicItem(
			const wxString & pattern, int SearchType)
/************************************************************************/
/* Find a string in schematic.
	Search is made in current sheet (SearchType = 0),
	or the whole hierarchy (SearchType = 1),
	or for the next item  (SearchType = 2).
	Mouse cursor is put on item
*/
{
SCH_SCREEN * Screen, * FirstScreen = NULL;
EDA_BaseStruct *DrawList = NULL, *FirstStruct = NULL, *Struct = NULL;
int NotFound, StartCount, ii, jj;
wxPoint firstpos, pos;
static int FindAll;
wxSize size = DrawPanel->GetClientSize();
wxPoint curpos;
bool force_recadre = FALSE;
wxString msg, WildText;
	
	g_LastSearchIsMarker = FALSE;
	
	Screen = ScreenSch;
	if( SearchType == 0 )
	{
		s_OldStringFound = pattern;
		Screen = (SCH_SCREEN*) m_CurrentScreen; FindAll = FALSE;
	}
	
	if( SearchType == 1 )
	{
		s_OldStringFound = pattern;
		FindAll = TRUE;
	}

	if(  SearchType != 2  ) ItemsCount = 0;

	WildText = s_OldStringFound;
	NotFound = 1; StartCount = 0;
	
	for ( ; Screen != NULL; Screen = Screen->Next() )
	{
		DrawList = Screen->EEDrawList;
		while ( DrawList )
		{
			switch (DrawList->m_StructType)
			{
				case DRAW_LIB_ITEM_STRUCT_TYPE :
					#undef STRUCT
					#define STRUCT ((EDA_SchComponentStruct*)DrawList)
					if( WildCompareString( WildText, STRUCT->m_Field[REFERENCE].m_Text, FALSE ) )
					{
						NotFound = 0;
						pos = STRUCT->m_Field[REFERENCE].m_Pos;
						break;
					}
					if( WildCompareString( WildText, STRUCT->m_Field[VALUE].m_Text, FALSE ) )
					{
						NotFound = 0;
						pos = STRUCT->m_Field[VALUE].m_Pos;
					}
					break;

				case DRAW_LABEL_STRUCT_TYPE :
				case DRAW_GLOBAL_LABEL_STRUCT_TYPE :
				case DRAW_TEXT_STRUCT_TYPE :
					#undef STRUCT
					#define STRUCT ((DrawTextStruct*)DrawList)
					if( WildCompareString( WildText, STRUCT->m_Text, FALSE ) )
					{
						NotFound = 0;
						pos = STRUCT->m_Pos;
					}
					break;

				default:
					break;
			}

			if(NotFound == 0)	/* Element trouve */
			{
				if ( FirstScreen == NULL )	/* 1er element trouve */
				{
					FirstScreen = Screen; firstpos = pos;
					FirstStruct = DrawList;
				}

				StartCount++;
				if( ItemsCount >= StartCount )
				{
					NotFound = 1;	/* Continue recherche de l'element suivant */
				}
				else
				{
					Struct = DrawList; ItemsCount++; break ;
				}
			}
			if( NotFound == 0 ) break;
			DrawList = DrawList->Pnext;
		}
		if( NotFound == 0 ) break;
		if( FindAll == FALSE ) break;
	}

	if( NotFound && FirstScreen )
	{
		NotFound = 0; Screen = FirstScreen; Struct = FirstStruct;
		pos = firstpos; ItemsCount = 1;
	}

	if( NotFound == 0)
	{
		if ( Screen != GetScreen() )
		{
			Screen->SetZoom(GetScreen()->GetZoom() );
			m_CurrentScreen = ActiveScreen = Screen;
			force_recadre = TRUE;
		}

		/* Si la struct localisee est du type DRAW_LIB_ITEM_STRUCT_TYPE,
			Les coordonnes sont a recalculer en fonction de la matrice
			d'orientation */
		if( Struct->m_StructType == DRAW_LIB_ITEM_STRUCT_TYPE )
		{
			#undef STRUCT
			#define STRUCT ((EDA_SchComponentStruct*)Struct)
			pos.x -= STRUCT->m_Pos.x; pos.y -= STRUCT->m_Pos.y;
			ii = STRUCT->m_Transform[0][0] * pos.x + STRUCT->m_Transform[0][1] * pos.y;
			jj = STRUCT->m_Transform[1][0] * pos.x + STRUCT->m_Transform[1][1] * pos.y;
			pos.x = ii + STRUCT->m_Pos.x; pos.y = jj + STRUCT->m_Pos.y;
		}

		Screen->m_Curseur = pos;
		curpos = DrawPanel->CursorScreenPosition();
		DrawPanel->GetViewStart(&m_CurrentScreen->m_StartVisu.x,
								&m_CurrentScreen->m_StartVisu.y);

		// calcul des coord curseur avec origine = screen
		curpos.x -= m_CurrentScreen->m_StartVisu.x;
		curpos.y -= m_CurrentScreen->m_StartVisu.y;

		/* Il y a peut-etre necessite de recadrer le dessin: */
		if( (curpos.x <= 0) || (curpos.x >= size.x-1) ||
			(curpos.y <= 0) || (curpos.y >= size.y) || force_recadre )
		{
			Recadre_Trace(TRUE);
		}
		else
		{
			GRMouseWarp(DrawPanel, curpos );
		}

		msg = WildText + _(" Found in ") + Screen->m_FileName;
		Affiche_Message(msg);
	}

	else
	{
		Affiche_Message("");
		msg = WildText + _(" Not Found");
		DisplayError(this,msg, 10);
	}
	
	return DrawList;
}


/*************************************************************/
void WinEDA_FindFrame::LocatePartInLibs(wxCommandEvent& event)
/*************************************************************/
/* Recherche exhaustive d'un composant en librairies, meme non chargees
*/
{
wxString Text, FindList;
const char ** ListNames;
LibraryStruct *Lib = NULL;
EDA_LibComponentStruct * LibEntry;
bool FoundInLib = FALSE;	// True si reference trouvee ailleurs qu'en cache
	
	Text = m_NewTextCtrl->GetData();
	if ( Text == "" )
	{
		Close(); return;
	}
	s_OldStringFound = Text;

	int ii, nbitems, NumOfLibs = NumOfLibraries();
	if (NumOfLibs == 0)
		{
		DisplayError(this, _("No libraries are loaded"));
		Close(); return;
		}

	ListNames = GetLibNames();
		
	nbitems = 0;
	for (ii = 0; ii < NumOfLibs; ii++ )	/* Recherche de la librairie */
	{
	bool IsLibCache;
		Lib = FindLibrary(ListNames[ii]);
		if ( Lib == NULL ) break;
		if ( Lib->m_Name.Contains(".cache") ) IsLibCache = TRUE;
		else IsLibCache = FALSE;
		LibEntry = (EDA_LibComponentStruct *) PQFirst(&Lib->m_Entries, FALSE);
		while( LibEntry )
		{
			if( WildCompareString(Text, LibEntry->m_Name.m_Text, FALSE) )
			{
				nbitems ++;
				if ( ! IsLibCache ) FoundInLib = TRUE;
				if ( FindList != "" ) FindList += "\n";
				FindList << _("Found ")
						+ LibEntry->m_Name.m_Text
						+ _(" in lib ") + Lib->m_Name;
			}
		LibEntry = (EDA_LibComponentStruct *) PQNext(Lib->m_Entries, LibEntry, NULL);
		}
	}

	free (ListNames);
	
	if ( ! FoundInLib )
	{
		if ( nbitems ) FindList = "\n" + Text + _(" found only in cache");
		else FindList = Text + _(" not found");
		FindList += _("\nExplore All Libraries?");
		if ( IsOK(this, FindList) )
		{
			FindList = "";
			ExploreAllLibraries(Text, FindList);
			if ( FindList == "" ) DisplayInfo(this, _("Nothing found") );
			else DisplayInfo(this, FindList);
		}
	}
	else DisplayInfo(this, FindList);
	
	Close();
}


/****************************************************************************************/
int WinEDA_FindFrame::ExploreAllLibraries(const wxString & wildmask, wxString & FindList)
/****************************************************************************************/
{
wxString FullFileName;
FILE * file;
int nbitems = 0, LineNum = 0;
char Line[2048], *name;
	
	FullFileName = MakeFileName(g_RealLibDirBuffer, "*", g_LibExtBuffer);
	
	FullFileName = wxFindFirstFile(FullFileName);
	while ( FullFileName != "" )
	{
		file = fopen(FullFileName.GetData(), "rt");
		if (file == NULL) continue;
 
		while (GetLine(file, Line, &LineNum, sizeof(Line)) )
		{
			if (strnicmp(Line, "DEF", 3) == 0)
			{ /* Read one DEF part from library: DEF 74LS00 U 0 30 Y Y 4 0 N */
				strtok(Line, " \t\r\n");
				name = strtok(NULL, " \t\r\n");
				if( WildCompareString(wildmask, name, FALSE) )
				{
					nbitems ++;
					if ( FindList != "" ) FindList += "\n";
					FindList << _("Found ") << name
							<< _(" in lib ") << FullFileName;
				}
			}
			else if (strnicmp(Line, "ALIAS", 5) == 0)
			{ /* Read one ALIAS part from library: ALIAS 74HC00 74HCT00 7400 74LS37 */
				strtok(Line, " \t\r\n");
				while ( (name = strtok(NULL, " \t\r\n")) != NULL )
				{
					if( WildCompareString(wildmask, name, FALSE) )
					{
						nbitems ++;
						if ( FindList != "" ) FindList += "\n";
						FindList << _("Found ") << name
								<< _(" in lib ") << FullFileName;
					}
				}
			}
		}
		fclose(file);
		FullFileName = wxFindNextFile();
	}

	return nbitems;
}

