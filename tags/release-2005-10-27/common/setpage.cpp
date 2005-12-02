		/****************************************************************************/		/* setpage.cpp : routines de selection de la taille de la feuille de dessin */
		/****************************************************************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"

#ifdef EESCHEMA
#include "program.h"
#include "libcmp.h"
#include "general.h"
#endif

#ifdef PCBNEW
#include "pcbnew.h"
#endif


#ifdef GERBVIEW
#define NB_ITEMS 12
W_PLOT * SheetList[NB_ITEMS+1] =
{
	&g_Sheet_A4, &g_Sheet_A3, &g_Sheet_A2, &g_Sheet_A1, &g_Sheet_A0,
	&g_Sheet_A, &g_Sheet_B, &g_Sheet_C, &g_Sheet_D, &g_Sheet_E, &g_Sheet_GERBER,
	&g_Sheet_user, NULL
};
#else
#define NB_ITEMS 11
W_PLOT * SheetList[NB_ITEMS+1] =
{
	&g_Sheet_A4, &g_Sheet_A3, &g_Sheet_A2, &g_Sheet_A1, &g_Sheet_A0,
	&g_Sheet_A, &g_Sheet_B, &g_Sheet_C, &g_Sheet_D, &g_Sheet_E,
	&g_Sheet_user, NULL
};
#endif

/* Routines internes */


	/*****************************************************/
	/* classe WinEDA_SetPageFrame frame de Configuration */
	/*****************************************************/

enum {
	ID_SAVE_PAGE_SETTINGS,
	ID_CANCEL_PAGE_SETTINGS,
	ID_LIST_PAGE_SIZES
};

	/*************************************************/
	/* classe derivee pour la frame de Configuration */
	/*************************************************/

class WinEDA_SetPageFrame: public wxDialog
{
public:
	WinEDA_DrawFrame *m_Parent;

	wxRadioBox *PageSizeBox;
	int Modified;
	wxString Buf_Size_X;
	wxString Buf_Size_Y;
	W_PLOT * SelectedSheet;
	WinEDA_EnterText * TextRevision;
	WinEDA_EnterText * TextCompany;
	WinEDA_EnterText * TextTitle;
	WinEDA_EnterText * TextComment1;
	WinEDA_EnterText * TextComment2;
	WinEDA_EnterText * TextComment3;
	WinEDA_EnterText * TextComment4;
	WinEDA_EnterText * TextUserSizeX;
	WinEDA_EnterText * TextUserSizeY;
	float UserSizeX, UserSizeY;

	// Constructor and destructor
	WinEDA_SetPageFrame(WinEDA_DrawFrame * parent, wxPoint& pos);
	~WinEDA_SetPageFrame(void);

	void OnClose(wxCommandEvent& event);

	void CreateListSizes(wxPoint boxpos);
	void SavePageSettings(wxCommandEvent& event);
	void ReturnSizeSelected(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

};


/* Construction de la table des evenements pour FrameClassMain */
BEGIN_EVENT_TABLE(WinEDA_SetPageFrame, wxDialog)
	EVT_BUTTON(ID_SAVE_PAGE_SETTINGS, WinEDA_SetPageFrame::SavePageSettings)
	EVT_BUTTON(ID_CANCEL_PAGE_SETTINGS, WinEDA_SetPageFrame::OnClose)
	EVT_RADIOBOX(ID_LIST_PAGE_SIZES, WinEDA_SetPageFrame::ReturnSizeSelected)
END_EVENT_TABLE()


/******************************************************************/
void WinEDA_DrawFrame::Process_PageSettings(wxCommandEvent& event)
/******************************************************************/
/* Creation de la fenetre de configuration
*/
{
wxPoint pos;

	pos = GetPosition();
	pos.x += 10; pos.y += 20;

	WinEDA_SetPageFrame * frame = new WinEDA_SetPageFrame(this, pos);
	frame->ShowModal(); frame->Destroy();
	ReDrawPanel();
}

	/********************************************************/
	/* Constructeur de WinEDA_SetPageFrame: la fenetre de config */
	/********************************************************/

WinEDA_SetPageFrame::WinEDA_SetPageFrame(WinEDA_DrawFrame *parent,
				wxPoint& framepos):
		wxDialog(parent, -1, _("Page Settings"), framepos, wxSize(570,400),
		DIALOG_STYLE )
{
#define LEN_EXT 100
#define LEN_DIR 400

wxPoint pos;
wxSize size;
int dimy = 24+17, POSX;
char Line[80];
BASE_SCREEN * BASE_SCREEN;

	m_Parent = parent;
	BASE_SCREEN = m_Parent->m_CurrentScreen;
	SetFont(*g_DialogFont);

	Modified = FALSE;
	SelectedSheet = NULL;

	pos.x = 5; pos.y = 10;
	CreateListSizes(pos);
	// creation des fenetres d'edition de la dimension de la sheet User
	if ( UnitMetric )
		{
		UserSizeX = (double)g_Sheet_user.m_Size.x * 25.4 / 1000 ;
		UserSizeY = (double)g_Sheet_user.m_Size.y * 25.4 / 1000;
		Buf_Size_X.Printf("%.2f", UserSizeX);
		Buf_Size_Y.Printf("%.2f", UserSizeY);
		}
	else
		{
		UserSizeX = (double)g_Sheet_user.m_Size.x / 1000;
		UserSizeY = (double)g_Sheet_user.m_Size.y / 1000;
		Buf_Size_X.Printf("%.3f", UserSizeX );
		Buf_Size_Y.Printf("%.3f", UserSizeY );
		}

	PageSizeBox->GetSize(&size.x, &size.y);
	pos.x = 5; pos.y += size.y + 20;
	size.y = -1;
	TextUserSizeX = new WinEDA_EnterText(this,
				UnitMetric ? "User Size X: (mm)" : "User Size X: (\")",
				Buf_Size_X, pos, size);

	pos.y += dimy;
	TextUserSizeY = new WinEDA_EnterText(this,
				UnitMetric ? "User Size Y: (mm)" : "User Size Y: (\")",
				Buf_Size_Y, pos, size);


	PageSizeBox->GetSize(&size.x, &size.y);
	POSX = pos.x + size.x + 20;

	/* Creation des boutons de commande */
	pos.x = POSX; pos.y = 5;
	wxButton * Button = new wxButton(this, ID_SAVE_PAGE_SETTINGS,
						_("OK"), pos );
	pos.x += Button->GetDefaultSize().x + 10;
	Button = new wxButton(this, ID_CANCEL_PAGE_SETTINGS,
						_("Cancel"), pos );

	pos.y = Button->GetDefaultSize().y + 15;
	sprintf(Line, _("Number of sheets: %d"), BASE_SCREEN->m_NumberOfSheet);
	new wxStaticText(this, -1, Line, pos);
	pos.x += 150;
	sprintf(Line, _("Sheet number: %d"), BASE_SCREEN->m_SheetNumber);
	new wxStaticText(this, -1, Line, pos);

	size.x = LEN_EXT; size.y = -1;
	pos.x = POSX; pos.y += 35;
	TextRevision = new WinEDA_EnterText(this,
				_("Revision:"), BASE_SCREEN->m_Revision.GetData(),
				pos, size);

	pos.y += dimy; size.x = LEN_DIR;
	TextCompany = new WinEDA_EnterText(this,
				_("Company:"), BASE_SCREEN->m_Company.GetData(),
				pos, size);

	pos.y += dimy;
	TextTitle = new WinEDA_EnterText(this,
				_("Title:"), BASE_SCREEN->m_Title.GetData(),
				pos, size);

	pos.y += dimy;
	TextComment1 = new WinEDA_EnterText(this,
				"Comment1:",  BASE_SCREEN->m_Commentaire1.GetData(),
				pos, size);

	pos.y += dimy;
	TextComment2 = new WinEDA_EnterText(this,
				"Comment2:", BASE_SCREEN->m_Commentaire2.GetData(),
				pos, size);

	pos.y += dimy;
	TextComment3 = new WinEDA_EnterText(this,
				"Comment3:", BASE_SCREEN->m_Commentaire3.GetData(),
				pos, size);

	pos.y += dimy;
	TextComment4 = new WinEDA_EnterText(this,
				"Comment4:", BASE_SCREEN->m_Commentaire4.GetData(),
				pos, size);
}


	/*****************************************************/
	/* Destructeur de WinEDA_SetPageFrame: la fenetre de config */
	/*****************************************************/

WinEDA_SetPageFrame::~WinEDA_SetPageFrame(void)
{
}


	/*****************************************************************/
	/* Fonctions de base de WinEDA_SetPageFrame: la fenetre de config */
	/*****************************************************************/

void WinEDA_SetPageFrame::OnClose(wxCommandEvent& event)
{
	Close(TRUE);
}


/*****************************************************************/
void WinEDA_SetPageFrame::SavePageSettings(wxCommandEvent& event)
/*****************************************************************/
/* Mise a jour effective des textes et dimensions
*/
{
BASE_SCREEN * screen = m_Parent->m_CurrentScreen;

	screen->m_Revision = TextRevision->GetData();
	screen->m_Company = TextCompany->GetData();
	screen->m_Title = TextTitle->GetData();
	screen->m_Commentaire1 = TextComment1->GetData();
	screen->m_Commentaire2 = TextComment2->GetData();
	screen->m_Commentaire3 = TextComment3->GetData();
	screen->m_Commentaire4 = TextComment4->GetData();

	Buf_Size_X = TextUserSizeX->GetData();
	Buf_Size_Y = TextUserSizeY->GetData();
	UserSizeX = atof( Buf_Size_X.GetData() );
	UserSizeY = atof( Buf_Size_Y.GetData() );

	if ( SelectedSheet )
		{
		screen->m_CurrentSheet = SelectedSheet;
		}

	if ( UnitMetric )
		{
		g_Sheet_user.m_Size.x = (int)(UserSizeX * 1000 / 25.4 );
		g_Sheet_user.m_Size.y = (int)(UserSizeY * 1000 / 25.4 );
		}
	else
		{
		g_Sheet_user.m_Size.x = (int)(UserSizeX * 1000 );
		g_Sheet_user.m_Size.y = (int)(UserSizeY * 1000 );
		}

	if ( g_Sheet_user.m_Size.x < 6000 )g_Sheet_user.m_Size.x = 6000;
	if ( g_Sheet_user.m_Size.x > 44000 )g_Sheet_user.m_Size.x = 44000;
	if ( g_Sheet_user.m_Size.y < 4000 )g_Sheet_user.m_Size.y = 4000;
	if ( g_Sheet_user.m_Size.y > 44000 )g_Sheet_user.m_Size.y = 44000;


	screen->SetModify();
	screen->SetRefreshReq();
	Close();
}



/*****************************************************************/
void WinEDA_SetPageFrame::ReturnSizeSelected(wxCommandEvent& event)
/*****************************************************************/
{
int ii;

	ii = PageSizeBox->GetSelection();
	SelectedSheet = SheetList[ii];
}


/********************************************************/
void WinEDA_SetPageFrame::CreateListSizes(wxPoint boxpos)
/********************************************************/
/* Genere la RadioBox donnant la liste des dimensions des feuilles
*/
{

W_PLOT * sheet;
int ii, select = NB_ITEMS-1;
wxString SizeList[NB_ITEMS];

	for( ii = 0; ii < NB_ITEMS; ii++ )
		{
		sheet = SheetList[ii];
		if( m_Parent->m_CurrentScreen->m_CurrentSheet == sheet ) select = ii;
		SizeList[ii] = wxString(_("Size ")) + sheet->m_Name;
		}

	PageSizeBox = new wxRadioBox(this, ID_LIST_PAGE_SIZES,
						_("Page Size:"),
						boxpos,wxSize(-1,-1),
						NB_ITEMS,SizeList,1,wxRA_SPECIFY_COLS);

	// Affichage de la selection courante
	PageSizeBox->SetSelection(select);
}



