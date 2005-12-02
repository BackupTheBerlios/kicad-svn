/********************************/
/*		MODULE displlst.cpp		*/
/********************************/

#include "fctsys.h"

#include "wxstruct.h"
#include "gr_basic.h"
#include "macros.h"
#include "common.h"



	/***********************/
	/* class WinEDAListBox */
	/***********************/

enum listbox {
	ID_LISTBOX_CANCEL = 8000,
	ID_LISTBOX_LIST,
	ID_LISTBOX_OK
};

BEGIN_EVENT_TABLE(WinEDAListBox, wxDialog)
	EVT_BUTTON(ID_LISTBOX_OK, WinEDAListBox::Ok)
	EVT_BUTTON(ID_LISTBOX_CANCEL, WinEDAListBox::Cancel)
	EVT_LISTBOX(ID_LISTBOX_LIST, WinEDAListBox::ClickOnList)
	EVT_LISTBOX_DCLICK(ID_LISTBOX_LIST, WinEDAListBox::D_ClickOnList)
	EVT_CLOSE( WinEDAListBox::OnClose )
END_EVENT_TABLE()


	/*******************************/
	/* Constructeur et destructeur */
	/*******************************/
/* Permet l'affichage d'une liste d'elements pour selection.
	itemlist = pointeur sur la liste des pinteurs de noms
	reftext = preselection
	movefct = fonction de création de commentaires a afficher
*/

#define M_LIST_POSX 5
#define M_LIST_POSY 15


WinEDAListBox::WinEDAListBox( wxWindow * parent, const wxString & title,
						const char ** itemlist,
						const wxString & reftext,
						void(* movefct)(wxString & Text),
						const wxColour & colour):
					wxDialog(parent, -1, title, wxPoint(10,10), wxSize(200,300),
					wxDEFAULT_DIALOG_STYLE)
{
wxPoint pos;
wxSize size, Wsize;
const char ** names;
int ii;

	m_ItemList = itemlist;
	m_Parent = parent;
	m_MoveFct = movefct;
	m_WinMsg = NULL;
	SetReturnCode(-1);

	if ( itemlist )
		for ( names = m_ItemList, ii = 0; *names != NULL; names++)
			ii++;
	else ii = 30;

	size.x = 160;
	size.y = (ii * 12) + 40;
	size.y = MIN( 300, size.y);
	pos.x = M_LIST_POSX; pos.y = M_LIST_POSY;
	m_List = new wxListBox(this,
							ID_LISTBOX_LIST,
							pos, size,
							0, NULL,
							wxLB_ALWAYS_SB|wxLB_SINGLE);
	if ( colour != wxNullColour)
	{
		m_List->SetBackgroundColour(colour);
		m_List->SetForegroundColour(*wxBLACK);
	}


	if ( itemlist )
		for ( names = m_ItemList, ii = 0; *names != NULL; names++, ii++ )
		{
		m_List->Append(*names);
		}

	Wsize.y = pos.y + size.y;

	pos.x += 5 + size.x; pos.y = 15;
	wxButton * butt = new wxButton(this, ID_LISTBOX_OK, _("Ok"), pos);

	pos.y += butt->GetDefaultSize().y + 10;
	new wxButton(this, ID_LISTBOX_CANCEL, _("Cancel"), pos);

	Wsize.x = pos.x + butt->GetDefaultSize().x + 10;
	Wsize.y = MAX( Wsize.y, pos.y + butt->GetDefaultSize().y + 10);

	if (m_MoveFct )
	{
		pos.x = 5; pos.y = Wsize.y + 10;
		size.x = 220; size.y = 60;
		Wsize.y = pos.y + size.y;
		m_WinMsg = new wxTextCtrl(this, -1,"", pos, size,
					wxTE_READONLY|wxTE_MULTILINE);
	}

	// Put the window on the parent window left upper corner
	m_Parent->GetPosition(& pos.x, &pos.y);
	// Ensure the window dialog is on screen :
	if ( pos.x < 0 ) pos.x = 0;
	if ( pos.y < 0 ) pos.y = 0;
	pos.x += 20; pos.y += 30;
	Wsize.y += 30;

	SetSize(pos.x, pos.x, Wsize.x, Wsize.y);
	Move(pos);
}


WinEDAListBox:: ~WinEDAListBox(void)
{
}


/******************************************/
void WinEDAListBox::MoveMouseToOrigin(void)
/******************************************/
{
int x, y, w, h;
wxSize list_size = m_List->GetSize();	
	wxClientDisplayRect(&x, &y, &w, &h);
	
	WarpPointer(x + M_LIST_POSX + 20, y + M_LIST_POSY + (list_size.y/2) );
}

/*********************************************/
wxString WinEDAListBox::GetTextSelection(void)
/*********************************************/
{
wxString text = m_List->GetStringSelection();
	return text;
}

/***************************************************************/
void WinEDAListBox::WinEDAListBox::Append(const wxString & item)
/***************************************************************/
{
	m_List->Append(item);
}

/******************************************************************************/
void WinEDAListBox::InsertItems(const wxArrayString & itemlist, int position)
/******************************************************************************/
{
	m_List->InsertItems(itemlist, position);
}

/************************************************/
void WinEDAListBox::Cancel(wxCommandEvent& event)
/************************************************/
{
	EndModal(-1);
}

/*****************************************************/
void WinEDAListBox::ClickOnList(wxCommandEvent& event)
/*****************************************************/
{
wxString text;

	if (m_MoveFct)
		{
		m_WinMsg->Clear();
		text =  m_List->GetStringSelection();
		m_MoveFct(text);
		m_WinMsg->WriteText(text.GetData());
		}
}

/*******************************************************/
void WinEDAListBox::D_ClickOnList(wxCommandEvent& event)
/*******************************************************/
{
int ii = m_List->GetSelection();
	EndModal(ii);
}


/***********************************************/
void WinEDAListBox::Ok(wxCommandEvent& event)
/***********************************************/
{
int ii = m_List->GetSelection();
	EndModal(ii);
}

/***********************************************/
void WinEDAListBox::OnClose(wxCloseEvent& event)
/***********************************************/
{
	EndModal(-1);
}


/********************************************************************/
static int SortItems( const wxString ** ptr1, const wxString ** ptr2 )
/********************************************************************/
/* Routines de comparaison pour le tri tri alphabetique,
	avec traitement des nombres en tant que valeur numerique
*/
{
	return StrNumICmp( (*ptr1)->GetData(), (*ptr2)->GetData() );
}


/************************************/
void WinEDAListBox:: SortList( void )
/************************************/
{
int ii, NbItems = m_List->GetCount();
const wxString ** BufList;

	if( NbItems <= 0 ) return;

	BufList = (const wxString **) MyZMalloc( 100*NbItems * sizeof(wxString*) );
	for( ii = 0; ii < NbItems; ii++ )
		{
		BufList[ii] = new wxString(m_List->GetString(ii));
		}

	qsort(BufList, NbItems, sizeof(wxString *),
			(int(*)(const void *,const void *)) SortItems);

	m_List->Clear();
	for( ii = 0; ii < NbItems; ii++ )
		{
		m_List->Append(*BufList[ii]);
		delete BufList[ii];
		}

	free(BufList);
}


