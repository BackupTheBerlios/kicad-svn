	/***************************************************/
	/* PCBNEW - Gestion des Recherches (fonction Find) */
	/***************************************************/

	/*	 Fichier find.cpp 	*/

/*
 Affichage et modifications des parametres de travail de PcbNew
 Parametres = dimensions des via, pistes, isolements, options...
*/


#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "pcbnew.h"

#include "id.h"

#include "protos.h"

/* Fonctions locales */

/* variables locales */
static wxString s_OldStringFound;
static int s_ItemCount, s_MarkerCount;

enum id_findpcb
{
	ID_FIND_ITEM = 1000,
	ID_FIND_NEXT_ITEM,
	ID_FIND_MARKER,
	ID_FIND_NEXT_MARKER
};
	/*****************************************************/
	/* classe derivee pour la frame de Recherche d'items */
	/*****************************************************/

class WinEDA_PcbFindFrame: public wxDialog
{
private:

	WinEDA_BasePcbFrame * m_Parent;
	wxDC * m_DC;
	WinEDA_EnterText * m_NewText;

	// Constructor and destructor
public:
	WinEDA_PcbFindFrame(WinEDA_BasePcbFrame *parent, wxDC * DC,
					const wxPoint& pos);
	~WinEDA_PcbFindFrame(void) {};

private:
	void FindItem(wxCommandEvent& event);
	void FindMarker(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

};
/* Construction de la table des evenements pour WinEDA_PcbFindFrame */
BEGIN_EVENT_TABLE(WinEDA_PcbFindFrame, wxDialog)
	EVT_BUTTON(ID_FIND_ITEM, WinEDA_PcbFindFrame::FindItem)
	EVT_BUTTON(ID_FIND_NEXT_ITEM, WinEDA_PcbFindFrame::FindItem)
	EVT_BUTTON(ID_FIND_MARKER, WinEDA_PcbFindFrame::FindItem)
	EVT_BUTTON(ID_FIND_NEXT_MARKER, WinEDA_PcbFindFrame::FindItem)
END_EVENT_TABLE()



/*********************************************************************/
void WinEDA_PcbFrame::InstallFindFrame(const wxPoint & pos, wxDC * DC)
/*********************************************************************/
{
	DrawPanel->m_IgnoreMouseEvents = TRUE;
	WinEDA_PcbFindFrame * frame = new WinEDA_PcbFindFrame(this, DC, pos);
	frame->ShowModal(); frame->Destroy();
	DrawPanel->m_IgnoreMouseEvents = FALSE;
}


	/***************************************/
	/* Constructeur de WinEDA_PcbFindFrame */
	/***************************************/

WinEDA_PcbFindFrame::WinEDA_PcbFindFrame(WinEDA_BasePcbFrame *parent,
		wxDC * DC, const wxPoint& framepos):
		wxDialog(parent, -1, _("Find"), framepos, wxSize(300, 100),
				 DIALOG_STYLE)
{
wxPoint pos;
wxSize WinClientSize;

	m_Parent = parent;
	SetFont(*g_DialogFont);
	m_DC = DC;

	pos.x = 5; pos.y = 60;
	wxButton * Button = new wxButton(this, ID_FIND_ITEM, _("Find Item"), pos);
	Button->SetForegroundColour( wxColor(100,0,0) );
	Button->SetDefault();
	
	pos.x += Button->GetSize().x  + 5;
	Button = new wxButton(this, ID_FIND_NEXT_ITEM, _("Find Next Item"), pos);
	Button->SetForegroundColour( wxColor(100,0,0) );

	pos.x += Button->GetSize().x  + 5;
	Button = new wxButton(this, ID_FIND_MARKER, _("Find Marker"), pos);
	Button->SetForegroundColour(*wxBLUE);
	
	pos.x += Button->GetSize().x  + 5;
	Button = new wxButton(this, ID_FIND_NEXT_MARKER, _("Find Next Marker"), pos);
	Button->SetForegroundColour(*wxBLUE);

	pos.x += Button->GetSize().x  + 5;
	WinClientSize.x = MAX(WinClientSize.x, pos.x);
	pos.y += Button->GetSize().y + 5;
	WinClientSize.y = pos.y;

	pos.y = 15; pos.x = 5;
	m_NewText = new WinEDA_EnterText(this,
				_("Item to find:"), s_OldStringFound,
				pos, wxSize(WinClientSize.x - 10,-1) );
	m_NewText->SetFocus();

	SetClientSize( WinClientSize );
}


/*******************************************************/
void WinEDA_PcbFindFrame::FindItem(wxCommandEvent& event)
/********************************************************/
{
PCB_SCREEN * screen = m_Parent->GetScreen();
wxPoint locate_pos;
wxString msg;
bool succes = FALSE;
bool FindMarker = FALSE;
MODULE * Module;
int StartCount;
	
	switch ( event.GetId() )
	{
		case ID_FIND_ITEM:
			s_ItemCount = 0;
			break;
		
		case ID_FIND_MARKER: s_MarkerCount = 0;
		case ID_FIND_NEXT_MARKER:
			FindMarker = TRUE;
			break;
	}

	s_OldStringFound = m_NewText->GetData();

	m_Parent->DrawPanel->GetViewStart(&screen->m_StartVisu.x, &screen->m_StartVisu.y);
	StartCount = 0;
	
	if( FindMarker )
	{
	MARQUEUR * Marker = (MARQUEUR *) m_Parent->m_Pcb->m_Drawings; 
		for( ; Marker != NULL; Marker = (MARQUEUR *)Marker->Pnext)
		{
			if( Marker->m_StructType != TYPEMARQUEUR ) continue;
			StartCount++;
			if ( StartCount > s_MarkerCount )
			{
				succes = TRUE;
				locate_pos = Marker->m_Pos;
				s_MarkerCount++;
				break;
			}
		}
	}
	
	else for ( Module = m_Parent->m_Pcb->m_Modules; Module != NULL; Module = (MODULE*)Module->Pnext)
	{
		if( WildCompareString( s_OldStringFound, Module->m_Reference->GetText(), FALSE ) )
		{
			StartCount++;
			if ( StartCount > s_ItemCount )
			{
				succes = TRUE;
				locate_pos = Module->m_Pos;
				s_ItemCount++;
				break;
			}
		}
		if( WildCompareString( s_OldStringFound, Module->m_Value->GetText(), FALSE ) )
		{
			StartCount++;
			if ( StartCount > s_ItemCount )
			{
				succes = TRUE;
				locate_pos = Module->m_Pos;
				s_ItemCount++;
				break;
			}
		}
	}
	
	if ( succes )
	{	/* Il y a peut-etre necessite de recadrer le dessin: */		
		if( ! m_Parent->DrawPanel->IsPointOnDisplay(locate_pos) )
		{
			screen->m_Curseur = locate_pos;
			m_Parent->Recadre_Trace(TRUE);
		}
		else
		{	// Positionnement du curseur sur l'item
			screen->Trace_Curseur(m_Parent->DrawPanel, m_DC);
			screen->m_Curseur = locate_pos;
			GRMouseWarp(m_Parent->DrawPanel, screen->m_Curseur );
			m_Parent->DrawPanel->MouseToCursorSchema();
			screen->Trace_Curseur(m_Parent->DrawPanel, m_DC);
		}

		if( FindMarker ) msg = _("Marker found");
		else msg.Printf( _("<%s> Found"), s_OldStringFound.GetData() );
		m_Parent->Affiche_Message(msg);
		EndModal(1);
	}

	else
	{
		m_Parent->Affiche_Message("");
		if( FindMarker ) msg = _("Marker not found");
		else msg.Printf( _("<%s> Not Found"), s_OldStringFound.GetData());
		DisplayError(this,msg, 10);
	}
	EndModal(0);
}



