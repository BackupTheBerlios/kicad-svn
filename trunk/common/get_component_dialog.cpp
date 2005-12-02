/*********************************/
/*	get_component_dialog.cpp	 */
/*********************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "macros.h"

/****************************************************************************/
/* Show a dialog frame to choose a name from an history list, or a new name */
/* to select a component or a module                                        */
/****************************************************************************/

static unsigned s_HistoryMaxCount = 8;	// Max number of items displayed in history list
static wxString s_ItemName;


enum selcmp_id {
	ID_ACCEPT_NAME = 3900,
	ID_ACCEPT_KEYWORD,
	ID_ENTER_NAME,
	ID_CANCEL,
	ID_LIST_ALL,
	ID_EXTRA_TOOL,
	ID_SEL_BY_LISTBOX
};

/***************************************/
class WinEDA_SelectCmp: public wxDialog
/***************************************/
{
private:
	WinEDA_DrawFrame * m_Parent;
	bool m_AuxTool;
	wxString * m_Text;
	wxTextCtrl * m_TextCtrl;
	wxListBox * m_List;
public:
	bool m_GetExtraFunction;

public:

	// Constructor and destructor
	WinEDA_SelectCmp(WinEDA_DrawFrame *parent, const wxPoint& framepos,
			wxArrayString & HistoryList, const wxString & Title,
			bool show_extra_tool );
	~WinEDA_SelectCmp(void) {};

private:
	void Accept(wxCommandEvent& event);
	void GetExtraSelection(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

};
BEGIN_EVENT_TABLE(WinEDA_SelectCmp, wxDialog)
	EVT_BUTTON(ID_ACCEPT_NAME, WinEDA_SelectCmp::Accept)
	EVT_BUTTON(ID_ACCEPT_KEYWORD, WinEDA_SelectCmp::Accept)
	EVT_BUTTON(ID_CANCEL, WinEDA_SelectCmp::Accept)
	EVT_BUTTON(ID_LIST_ALL, WinEDA_SelectCmp::Accept)
	EVT_BUTTON(ID_EXTRA_TOOL, WinEDA_SelectCmp::GetExtraSelection)
	EVT_LISTBOX(ID_SEL_BY_LISTBOX, WinEDA_SelectCmp::Accept)
END_EVENT_TABLE()


/****************************************************************************/
WinEDA_SelectCmp::WinEDA_SelectCmp(WinEDA_DrawFrame *parent, const wxPoint& framepos,
			wxArrayString & HistoryList, const wxString & Title,
			bool show_extra_tool ):
		wxDialog(parent, -1, Title, framepos,
			wxSize(350, 330), DIALOG_STYLE)
/****************************************************************************/
/* Dialog frame to choose a component or a footprint
	This dialog shows an history of last selected items
*/
{
#define START_Y 10
wxButton * Button;
wxPoint pos;
wxSize size(220, -1);
wxSize WinSize;
wxStaticText * Text;
	
	m_Parent = parent;
	m_AuxTool = show_extra_tool;
	m_GetExtraFunction = FALSE;
	
	SetFont(*g_DialogFont);
	s_ItemName = "";
	m_Text =  & s_ItemName;
	pos.x = 5; pos.y = START_Y;
	
	Text = new wxStaticText(this, -1, _("Name:"), pos);
	pos.y = Text->GetRect().GetBottom() + 3;
	m_TextCtrl = new wxTextCtrl(this, ID_ENTER_NAME, *m_Text, pos, size);
	m_TextCtrl->SetInsertionPoint(1);
	
	pos.y = m_TextCtrl->GetRect().GetBottom() + 10;
	Text = new wxStaticText(this, -1, _("History list:"), pos);
	pos.y = Text->GetRect().GetBottom() + 3;
	m_List = new wxListBox(this, ID_SEL_BY_LISTBOX, pos, 
		size, HistoryList, wxLB_SINGLE );
	
	pos.x += size.x + 15; pos.y = START_Y;
	Button = new wxButton(this, ID_ACCEPT_NAME, _("OK"), pos);
	Button->SetForegroundColour(*wxRED);
	Button->SetDefault();
	WinSize.x = MAX(WinSize.x, Button->GetRect().GetRight());

	pos.y += 30;
	Button = new wxButton(this, ID_ACCEPT_KEYWORD, _("Search KeyWord"), pos);
	Button->SetForegroundColour(*wxRED);
	WinSize.x = MAX(WinSize.x, Button->GetRect().GetRight());

	pos.y += 30;
	Button = new wxButton(this, ID_CANCEL, _("Cancel"), pos);
	Button->SetForegroundColour(*wxBLUE);
	WinSize.x = MAX(WinSize.x, Button->GetRect().GetRight());

	pos.y += 30;
	Button = new wxButton(this, ID_LIST_ALL, _("List All"), pos);
	Button->SetForegroundColour(wxColor(0, 80, 0));

	if ( m_AuxTool )	/* The selection can be done by an extra function */
	{
		pos.y += 30;
		Button = new wxButton(this, ID_EXTRA_TOOL, _("By Lib Browser"), pos);
		Button->SetForegroundColour(wxColor(80, 0, 80));
	}
	
	
	WinSize.x = MAX(WinSize.x, Button->GetRect().GetRight());
	WinSize.y = MAX(WinSize.y, Button->GetRect().GetBottom());

	WinSize.x += 5;
	WinSize.y = MAX(WinSize.y, m_List->GetRect().GetBottom()) + 10;
	
	SetClientSize(WinSize);
	
}

/*********************************************************/
void WinEDA_SelectCmp::Accept(wxCommandEvent& event)
/*********************************************************/
{
	switch (event.GetId() )
	{
		case ID_SEL_BY_LISTBOX:
			*m_Text = m_List->GetStringSelection();
			break;

		case ID_ACCEPT_NAME:
			*m_Text = m_TextCtrl->GetValue();
			break;

		case ID_ACCEPT_KEYWORD:
			*m_Text = "= " + m_TextCtrl->GetValue();
			break;

		case ID_CANCEL:
			*m_Text = "";
			break;
	
		case ID_LIST_ALL:
			*m_Text = "*";
			break;
	
	}
	m_Text->Trim(FALSE);	// Remove blanks at beginning
	m_Text->Trim(TRUE);	// Remove blanks at end
	Close(TRUE);
}


/**************************************************************/
void WinEDA_SelectCmp::GetExtraSelection(wxCommandEvent& event)
/**************************************************************/
/* Get the component name by the extra function
*/
{
	m_GetExtraFunction = TRUE;
	Close(TRUE);
}



/******************************************************************************/
wxString GetComponentName(WinEDA_DrawFrame * frame,
		wxArrayString & HistoryList, const wxString & Title,
		wxString(*AuxTool)(WinEDA_DrawFrame *parent))
/*******************************************************************************/
/* Dialog frame to choose a component name
*/
{
wxPoint framepos;
int x, y, w, h;
bool GetExtraFunction;
	
	framepos = wxGetMousePosition();
	wxClientDisplayRect(&x, &y, &w, &h);
	framepos.x -= 100; framepos.y -= 50;
	if ( framepos.x < x ) framepos.x = x;
	if ( framepos.y < y ) framepos.y = y;
	if ( framepos.x < x ) framepos.x = x;
	x += w -350; if ( framepos.x > x ) framepos.x = x;
	if ( framepos.y < y ) framepos.y = y;
WinEDA_SelectCmp * selframe =
		new WinEDA_SelectCmp(frame, framepos, HistoryList, Title, AuxTool ? TRUE : FALSE);
	selframe->ShowModal();
	GetExtraFunction = selframe->m_GetExtraFunction;
	selframe->Destroy();
	
	if ( GetExtraFunction ) s_ItemName = AuxTool(frame);
	return s_ItemName;
}


/*******************************************************************************/
void AddHistoryComponentName(wxArrayString & HistoryList, const wxString & Name)
/*******************************************************************************/
/* Add the string "Name" to the history list HistoryList
*/
{
int ii, c_max;

	if ( HistoryList.GetCount() > 0)
	{	
		if (Name == HistoryList[0] ) return;
		
		/* remove an old identical selection if exists */
		for ( ii = 1; (unsigned)ii < HistoryList.GetCount(); ii ++ )
		{
			if ( Name == HistoryList[ii] )
			{
				HistoryList.RemoveAt(ii); ii--;
			}
		}
	
		/* shift the list */
		if ( HistoryList.GetCount() < s_HistoryMaxCount ) HistoryList.Add("");
		
		c_max = HistoryList.GetCount() - 2;
		for ( ii = c_max; ii >= 0 ; ii -- ) HistoryList[ii+1] = HistoryList[ii];
			
		/* Add the new name at the beginning of the history list */
		HistoryList[0] = Name;
	}
	else
		HistoryList.Add(Name);
}
