	/**************************************************/
	/* Module de tst "ERC" ( Electrical Rules Check ) */
	/**************************************************/

#include "fctsys.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"
#include "netlist.h"

#include "protos.h"


#include "../bitmaps/ercgreen.xpm"
#include "../bitmaps/ercwarn.xpm"
#include "../bitmaps/ercerr.xpm"

/* On teste
	1 - conflits entre pins connectees ( ex: 2 sorties connectees )
	2 - les imperatifs minimaux ( 1 entree doit etre connectee a une sortie )
*/


/* fonctions exportees */

/* fonctions importees */

/* fonctions locales */
static bool WriteDiagnosticERC(const wxString & FullFileName);
static void Diagnose(WinEDA_DrawPanel * panel, wxDC * DC,
				ObjetNetListStruct * NetItemRef,
				ObjetNetListStruct * NetItemTst, int MinConnexion, int Diag);
static void TestOthersItems(WinEDA_DrawPanel * panel,
								wxDC * DC, ObjetNetListStruct *  NetItemRef,
								ObjetNetListStruct *  NetStart,
								int * NetNbItems, int * MinConnexion );
static void TestLabel(WinEDA_DrawPanel * panel, wxDC * DC,
							ObjetNetListStruct * NetItemRef,
							ObjetNetListStruct * StartNet);

/* Variable locales */
static int WriteFichierERC = FALSE;

/* Tableau des types de conflit :
	PIN_INPUT, PIN_OUTPUT, PIN_BIDI, PIN_TRISTATE, PIN_PASSIVE,
	PIN_UNSPECIFIED, PIN_POWER_IN, PIN_POWER_OUT, PIN_OPENCOLLECTOR,
	PIN_OPENEMITTER, PIN_NC
*/
#define OK 0
#define WAR 1	// utilisé aussi dans eeredraw
#define ERR 2
#define UNC 3

static wxChar * CommentERC_H[] =
{
	wxT("Input Pin...."),
	wxT("Output Pin..."),
	wxT("BiDi Pin....."),
	wxT("3 State Pin.."),
	wxT("Passive Pin.."),
	wxT("Unspec Pin..."),
	wxT("Power IN Pin."),
	wxT("PowerOUT Pin."),
	wxT("Open Coll...."),
	wxT("Open Emit...."),
	wxT("No Conn......"),
	NULL
};
static wxChar * CommentERC_V[] =
{
	wxT("Input Pin"),
	wxT("Output Pin"),
	wxT("BiDi Pin"),
	wxT("3 State Pin"),
	wxT("Passive Pin"),
	wxT("Unspec Pin"),
	wxT("Power IN Pin"),
	wxT("PowerOUT Pin"),
	wxT("Open Coll"),
	wxT("Open Emit"),
	wxT("No Conn"),
	NULL
};


/* Look up table which gives the diag for a pair of connected pins
	Can be modified by ERC options.
	at start up: must be loaded by DefaultDiagErc
*/
static int DiagErc[PIN_NMAX][PIN_NMAX];
bool DiagErcTableInit;	// go to TRUE after DiagErc init

/* Default Look up table which gives the diag for a pair of connected pins
	Same as DiagErc, but cannot be modified
	Used to init or reset DiagErc
*/
static int DefaultDiagErc[PIN_NMAX][PIN_NMAX] =
{ /*       I,   O,   Bi,  3S, Pas, UnS,PwrI,PwrO,  OC,  OE,  NC */
/* I */ { OK,   OK,  OK,  OK,  OK, WAR,  OK,  OK,  OK,  OK, WAR },
/* O */ { OK,  ERR,  OK, WAR,  OK, WAR,  OK, ERR, ERR, ERR, WAR },
/* Bi*/ { OK,   OK,  OK,  OK,  OK, WAR,  OK, WAR, WAR, WAR, WAR },
/* 3S*/ { OK,  WAR,  OK,  OK,  OK, WAR, WAR, ERR, WAR, WAR, WAR },
/*Pas*/ { OK,   OK,  OK,  OK,  OK, WAR,  OK,  OK,  OK,  OK, WAR },
/*UnS */{ WAR, WAR, WAR, WAR, WAR, WAR, WAR, WAR, WAR, WAR, WAR },
/*PwrI*/{ OK,   OK,  OK, WAR,  OK, WAR,  OK,  OK,  OK,  OK, ERR },
/*PwrO*/{ OK,  ERR, WAR, ERR,  OK, WAR,  OK, ERR, ERR, ERR, WAR },
/* OC */{ OK,  ERR, WAR, WAR,  OK, WAR,  OK, ERR,  OK,  OK, WAR },
/* OE */{ OK,  ERR, WAR, WAR,  OK, WAR,  OK, ERR,  OK,  OK, WAR },
/* NC */{ WAR, WAR, WAR, WAR, WAR, WAR, WAR, WAR, WAR, WAR, WAR }
} ;


/* Minimal connection table */
#define DRV 3		/* Net driven by a signal (a pin output for instance) */
#define NET_NC 2	/* Net "connected" to a "NoConnect symbol" */
#define NOD 1		/* Net not driven ( Such as 2 or more connected inputs )*/
#define NOC 0		/* Pin isolee, non connectee */

/* Look up table which gives the minimal drive for a pair of connected pins on a net
	Initial state of a net is NOC (No Connection)
	Can be updated to NET_NC, or NOD (Not Driven) or DRV (DRIven)

	Can be updated to NET_NC only if the previous state is NOC

	Nets are OK when their final state is NET_NC or DRV
	Nets with the state NOD have no source signal
*/
static int MinimalReq[PIN_NMAX][PIN_NMAX] =
		{ /* In, Out,  Bi,  3S, Pas, UnS,PwrI,PwrO,  OC,  OE,  NC */
/* In*/ {	NOD, DRV, DRV, DRV, DRV, DRV, NOD, DRV, DRV, DRV, NOC },
/*Out*/ {	DRV, DRV, DRV, DRV, DRV, DRV, DRV, DRV, DRV, DRV, NOC },
/* Bi*/ {	DRV, DRV, DRV, DRV, DRV, DRV, NOD, DRV, DRV, DRV, NOC },
/* 3S*/ {	DRV, DRV, DRV, DRV, DRV, DRV, NOD, DRV, DRV, DRV, NOC },
/*Pas*/ {	DRV, DRV, DRV, DRV, DRV, DRV, NOD, DRV, DRV, DRV, NOC },
/*UnS*/ {	DRV, DRV, DRV, DRV, DRV, DRV, NOD, DRV, DRV, DRV, NOC },
/*PwrI*/{	NOD, DRV, NOD, NOD, NOD, NOD, NOD, DRV, NOD, NOD, NOC },
/*PwrO*/{	DRV, DRV, DRV, DRV, DRV, DRV, DRV, DRV, DRV, DRV, NOC },
/* OC*/ {	DRV, DRV, DRV, DRV, DRV, DRV, NOD, DRV, DRV, DRV, NOC },
/* OE*/ {	DRV, DRV, DRV, DRV, DRV, DRV, NOD, DRV, DRV, DRV, NOC },
/* NC*/ {	NOC, NOC, NOC, NOC, NOC, NOC, NOC, NOC, NOC, NOC, NOC }
} ;



enum id_erc {
	ID_ERC_CMP = 1400,
	ID_ERASE_DRC_MARKERS,
	ID_CLOSE_ERC,
	ID_GENERE_RAPPORT,
	ID_RAPPORT_BROWSE,
	ID_RESET_MATRIX,
	ID_ERC_NOTEBOOK,
	ID_MATRIX_0
};

/* Dialog frame for E.R.C control and status */
class WinEDA_ErcFrame: public wxDialog
{
public:
	WinEDA_DrawFrame * m_Parent;
	wxNotebook* NoteBook;
	wxPanel * PanelERC;
	wxPanel * PanelMatrice;
	wxCheckBox * WriteResult;
	wxStaticText * ErcTotalErrors;
	wxStaticText * ErcErrors;
	wxStaticText * WarnErcErrors;

	// Constructor and destructor
	WinEDA_ErcFrame(WinEDA_DrawFrame * parent, wxPoint& pos);
	~WinEDA_ErcFrame(void) {};

	void DelERCMarkers(wxCommandEvent& event);
	void TestErc(wxCommandEvent& event);
	void SelLocal(wxCommandEvent& event);
	void SelNewCmp(wxCommandEvent& event);
	void ResetDefaultERCDiag(wxCommandEvent& event);
	void ChangeErrorLevel(wxCommandEvent& event);
	void CloseFrame(wxCommandEvent& event);
	void ReBuildMatrixPanel(int active);

	DECLARE_EVENT_TABLE()
};
BEGIN_EVENT_TABLE(WinEDA_ErcFrame, wxDialog)
	EVT_BUTTON(ID_CLOSE_ERC, WinEDA_ErcFrame::CloseFrame)
	EVT_BUTTON(ID_ERC_CMP, WinEDA_ErcFrame::TestErc)
	EVT_BUTTON(ID_ERASE_DRC_MARKERS, WinEDA_ErcFrame::DelERCMarkers)
	EVT_BUTTON(ID_RESET_MATRIX, WinEDA_ErcFrame::ResetDefaultERCDiag)
	EVT_COMMAND_RANGE(ID_MATRIX_0,
					ID_MATRIX_0 + (PIN_NMAX * PIN_NMAX) - 1,
					wxEVT_COMMAND_BUTTON_CLICKED,
					WinEDA_ErcFrame::ChangeErrorLevel)
END_EVENT_TABLE()


/*************************************************************/
void InstallErcFrame(WinEDA_DrawFrame *parent, wxPoint & pos)
/*************************************************************/
/* Install function  for the ERC dialog frame
*/
{
	WinEDA_ErcFrame * frame = new WinEDA_ErcFrame(parent, pos);
	frame->ShowModal(); frame->Destroy();
}


#define H_SIZE 430
#define V_SIZE 335
/*******************************************************************************/
WinEDA_ErcFrame::WinEDA_ErcFrame(WinEDA_DrawFrame *parent, wxPoint& framepos):
		wxDialog(parent, -1, _("EESchema Erc"), framepos, wxSize(H_SIZE, V_SIZE),
				 DIALOG_STYLE)
/*******************************************************************************/
{
wxPoint pos;

	m_Parent = parent;
	SetFont(*g_DialogFont);

	if ( (framepos.x == -1) && (framepos.x == -1) ) Centre();

	NoteBook = new wxNotebook(this, ID_ERC_NOTEBOOK,
   		wxDefaultPosition,wxSize(H_SIZE - 6, V_SIZE - 28) );
	NoteBook->SetFont(*g_DialogFont);
	PanelMatrice = NULL;

	wxLayoutConstraints* c = new wxLayoutConstraints;
	c->left.SameAs(this, wxLeft, 4);
	c->right.SameAs(this, wxRight, 4);
	c->top.SameAs(this, wxTop, 4);
    c->bottom.SameAs(this, wxBottom, 10);

	NoteBook->SetConstraints(c);

	// Add panels
	PanelERC = new wxPanel(NoteBook, -1);
	PanelERC->SetFont(*g_DialogFont);
	NoteBook->AddPage(PanelERC, _("Erc"), TRUE);

	// Init Panel ERC
	pos.x = 5; pos.y = 15;
	new wxStaticBox(PanelERC,-1, _("Erc Diags:"), pos, wxSize(210,95) );
	pos.x = 10; pos.y += 22;
	ErcTotalErrors = new wxStaticText(PanelERC, -1, _("-> Total Errors:  "), pos);
	ErcTotalErrors->SetFont(*g_FixedFont);
	ErcTotalErrors->SetLabel(ErcTotalErrors->GetLabel() << g_EESchemaVar.NbErrorErc);

	pos.x = 10; pos.y += 18;
	ErcErrors = new wxStaticText(PanelERC, -1, _("-> Last Errors:   "), pos);
	ErcErrors->SetFont(*g_FixedFont);
	ErcErrors->SetLabel(ErcErrors->GetLabel() << (g_EESchemaVar.NbErrorErc-g_EESchemaVar.NbWarningErc));
	ErcErrors->SetForegroundColour(*wxRED);

	pos.y += 15;
	WarnErcErrors = new wxStaticText(PanelERC, -1, _("-> Last Warnings: "), pos);
	WarnErcErrors->SetFont(*g_FixedFont);
	WarnErcErrors->SetLabel(WarnErcErrors->GetLabel() << g_EESchemaVar.NbWarningErc);
	WarnErcErrors->SetForegroundColour(*wxBLUE);

	pos.x = 5; pos.y = 165;
	new wxStaticBox(PanelERC,-1, _("Erc File Report:"), pos, wxSize(310,50) );

	pos.x = 15; pos.y += 20;
	WriteResult = new wxCheckBox(PanelERC, -1,
						_("Write erc report"),
						pos);
	WriteResult->SetValue(WriteFichierERC);

	/* Create command buttons */
	pos.x = 230; pos.y = 15;
	wxButton * Button = new wxButton(PanelERC, ID_ERC_CMP,
						_("&Test Erc"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.y += Button->GetSize().y + 10;
	Button = new wxButton(PanelERC,	ID_ERASE_DRC_MARKERS,
						_("&Del Markers"), pos);
	Button->SetForegroundColour(*wxBLACK);

	pos.y += Button->GetSize().y + 10;
	Button = new wxButton(PanelERC,	ID_CLOSE_ERC,
						_("&Exit"), pos);
	Button->SetForegroundColour(*wxBLUE);


	// Init Panel Matrix
	ReBuildMatrixPanel(FALSE);
}

/******************************************************/
void WinEDA_ErcFrame::CloseFrame(wxCommandEvent& event)
/******************************************************/
{
	EndModal(0);
}

/****************************************************/
void WinEDA_ErcFrame::ReBuildMatrixPanel(int active)
/****************************************************/
/* construit ou reconstruit le panel d'affichage de la matrice de
controle ERC
*/
{
int ii, jj;
wxPoint pos;

	if ( PanelMatrice != NULL ) NoteBook->DeletePage(1);
		
	if ( ! DiagErcTableInit )
	{
		memcpy(DiagErc, DefaultDiagErc, sizeof (DefaultDiagErc));
		DiagErcTableInit = TRUE;
	}

	PanelMatrice = new wxPanel(NoteBook, -1);
	PanelMatrice->SetFont(*g_FixedFont);

	NoteBook->AddPage(PanelMatrice, _("Options"), active);

	/* Creation des boutons de commande */
	pos.x = 5; pos.y = 5;
	new wxButton(PanelMatrice, ID_RESET_MATRIX,
						_("Reset"), pos);
	ii = jj = 0;
	pos.x = 100; pos.y = 50;
	#define BITMAP_SIZE 19
	for ( ii = 0; ii < PIN_NMAX; ii++ )
	{
		int x, y;
		y = pos.y + (ii * BITMAP_SIZE);
		new wxStaticText( PanelMatrice,-1,CommentERC_H[ii], wxPoint(5,y));
		for ( jj = 0; jj <= ii; jj++ )
		{
			int diag = DiagErc[ii][jj];
			x = pos.x + (jj * BITMAP_SIZE);
			if( ii == jj )
				new wxStaticText( PanelMatrice,-1,CommentERC_V[ii], wxPoint(x+4,y-15));
			switch ( diag )
			{
				case OK:
					new wxBitmapButton(PanelMatrice,
						ID_MATRIX_0 + ii + (jj * PIN_NMAX),
						wxBitmap(green_xpm),
						wxPoint(x,y) );
					break;

				case WAR:
					new wxBitmapButton(PanelMatrice,
						ID_MATRIX_0 + ii + (jj * PIN_NMAX),
						wxBitmap(warning_xpm),
						wxPoint(x,y) );
					break;

				case ERR:
					new wxBitmapButton(PanelMatrice,
						ID_MATRIX_0 + ii + (jj * PIN_NMAX),
						wxBitmap(error_xpm),
						wxPoint(x,y) );
					break;
			}
		}
	}
}


/**************************************************/
void WinEDA_ErcFrame::TestErc(wxCommandEvent& event)
/**************************************************/
{
ObjetNetListStruct * NetItemRef, * OldItem, * StartNet, * Lim;
int NetNbItems, MinConn;

	if ( ! DiagErcTableInit )
	{
		memcpy(DiagErc, DefaultDiagErc, sizeof (DefaultDiagErc));
		DiagErcTableInit = TRUE;
	}

	WriteFichierERC = WriteResult->GetValue();

	if( CheckAnnotate(m_Parent, 0) )
	{
		DisplayError(this, _("Annotation Required!") );
		return;
	}

	/* Effacement des anciens marqueurs DRC */
	DelERCMarkers(event);

wxClientDC dc(m_Parent->DrawPanel);

	m_Parent->DrawPanel->PrepareGraphicContext(&dc);

	g_EESchemaVar.NbErrorErc = 0;
	g_EESchemaVar.NbWarningErc = 0;

	SchematicCleanUp(&dc);

	BuildNetList(m_Parent, ScreenSch);

	/* Analyse de la table des connexions : */
	Lim = g_TabObjNet + g_NbrObjNet;

	/* Reset du flag m_FlagOfConnection, utilise par la suite */
	for (NetItemRef = g_TabObjNet; NetItemRef < Lim; NetItemRef ++ )
		NetItemRef->m_FlagOfConnection = (IsConnectType) 0;

	NetNbItems = 0; MinConn = NOC;
	StartNet = OldItem = NetItemRef = g_TabObjNet;
	for ( ; NetItemRef < Lim; NetItemRef ++ )
	{
		/* Tst changement de net */
		if( OldItem->m_NetCode != NetItemRef->m_NetCode)
		{
			MinConn = NOC; NetNbItems = 0; StartNet = NetItemRef;
		}

		switch ( NetItemRef->m_Type )
		{
			case NET_SEGMENT:
			case NET_BUS:
			case NET_JONCTION:
			case NET_LABEL:
			case NET_BUSLABELMEMBER:
			case NET_PINLABEL:
				break;

			case NET_GLOBLABEL:
			case NET_GLOBBUSLABELMEMBER:
			case NET_SHEETLABEL:
			case NET_SHEETBUSLABELMEMBER:
				TestLabel(m_Parent->DrawPanel, &dc, NetItemRef, StartNet);
				break;

			case NET_NOCONNECT:
				MinConn = NET_NC;
				if( NetNbItems != 0 ) 
					  Diagnose(m_Parent->DrawPanel, &dc, NetItemRef, NULL, MinConn, UNC);
				break;

			case NET_PIN:
				TestOthersItems(m_Parent->DrawPanel, &dc,
						NetItemRef, StartNet, &NetNbItems , &MinConn);
				break;
		} 
		OldItem = NetItemRef;
	}

	FreeTabNetList(g_TabObjNet, g_NbrObjNet );

	ErcTotalErrors->SetLabel(wxString(_("-> Total Errors: ")) << g_EESchemaVar.NbErrorErc);

	ErcErrors->SetLabel(wxString(_("-> Errors ERC:   ")) << (g_EESchemaVar.NbErrorErc-g_EESchemaVar.NbWarningErc) );

	WarnErcErrors->SetLabel(wxString(_("-> Warnings ERC: ")) << g_EESchemaVar.NbWarningErc);

	/* Generation ouverture fichier diag */
	if( WriteFichierERC == TRUE )
	{
		wxString ErcFullFileName;
		ErcFullFileName = ScreenSch->m_FileName;
		ChangeFileNameExt(ErcFullFileName, wxT(".erc"));
		ErcFullFileName = EDA_FileSelector(_("ERC file:"),
					wxEmptyString,					/* Chemin par defaut */
					ErcFullFileName,	/* nom fichier par defaut */
					wxT(".erc"),				/* extension par defaut */
					wxT("*.erc"),			/* Masque d'affichage */
					this,
					wxSAVE,
					TRUE
					);
		if ( ErcFullFileName.IsEmpty()) return;

		if ( WriteDiagnosticERC(ErcFullFileName) )
		{
			Close(TRUE);
			wxString editorname = GetEditorName();
			ExecuteFile(this, editorname, ErcFullFileName);
		}
	}
}


/***********************************************************/
void WinEDA_ErcFrame::DelERCMarkers(wxCommandEvent& event)
/***********************************************************/
/* Delete the old ERC markers, over the whole hierarchy
*/
{
EDA_BaseStruct * DrawStruct;
DrawMarkerStruct * Marker;
wxClientDC dc(m_Parent->DrawPanel);

	m_Parent->DrawPanel->PrepareGraphicContext(&dc);

	// Delete markers for the currens screen
	DrawStruct = m_Parent->GetScreen()->EEDrawList;
	for ( ; DrawStruct != NULL; DrawStruct = DrawStruct->Pnext )
		{
		if(DrawStruct->m_StructType != DRAW_MARKER_STRUCT_TYPE ) continue;
		/* Marqueur trouve */
		Marker = (DrawMarkerStruct * ) DrawStruct;
		if( Marker->m_Type == MARQ_ERC )
			RedrawOneStruct(m_Parent->DrawPanel, &dc, Marker, g_XorMode);
		}
	/* Suppression en memoire des marqueurs ERC */
	DeleteAllMarkers(MARQ_ERC);
}


/**************************************************************/
void WinEDA_ErcFrame::ResetDefaultERCDiag(wxCommandEvent& event)
/**************************************************************/
/* Remet aux valeurs par defaut la matrice de diagnostic
*/
{
	memcpy(DiagErc,DefaultDiagErc, sizeof(DiagErc) );
	ReBuildMatrixPanel(TRUE);
}

/************************************************************/
void WinEDA_ErcFrame::ChangeErrorLevel(wxCommandEvent& event)
/************************************************************/
/* Change the error level for the pressed button, on the matrix table
*/
{
int id, level, ii, x, y;
wxBitmapButton * Butt;
wxPoint pos;

	id = event.GetId();
	ii = id - ID_MATRIX_0;
	Butt = (wxBitmapButton*) event.GetEventObject();
	pos = Butt->GetPosition();

	x = ii /PIN_NMAX; y = ii % PIN_NMAX;

	level = DiagErc[y][x];
	switch (level )
		{
		case OK:
			level = WAR;
			delete Butt;
			new wxBitmapButton(PanelMatrice,
						id,
						wxBitmap(warning_xpm),
						pos );
			break;

		case WAR :
			level = ERR;
			delete Butt;
			new wxBitmapButton(PanelMatrice,
						id,
						wxBitmap(error_xpm),
						pos );
			break;

		case ERR:
			level = OK;
			delete Butt;
			new wxBitmapButton(PanelMatrice,
						id,
						wxBitmap(green_xpm),
						pos );
			break;

		}

	DiagErc[y][x] = DiagErc[x][y] = level;
}


/********************************************************/
static void Diagnose(WinEDA_DrawPanel * panel, wxDC * DC,
                    ObjetNetListStruct * NetItemRef,
					ObjetNetListStruct * NetItemTst,
                    int MinConn, int Diag)
/********************************************************/
/* Routine de creation du marqueur ERC correspondant au conflit electrique
	entre NetItemRef et NetItemTst
	si MinConn < 0: Traitement des erreurs sur labels
*/
{
DrawMarkerStruct * Marker = NULL;
wxString DiagLevel;
SCH_SCREEN * screen;
int ii, jj;

	if( Diag == OK ) return;

	/* Creation du nouveau marqueur type Erreur ERC */
	Marker = new DrawMarkerStruct(NetItemRef->m_Start, wxEmptyString);
	Marker->m_Type = MARQ_ERC;
	Marker->m_MarkFlags = WAR;
	screen = NetItemRef->m_Window;
	Marker->Pnext = screen->EEDrawList;
	screen->EEDrawList = Marker;
	g_EESchemaVar.NbErrorErc++;
	g_EESchemaVar.NbWarningErc++;

	if( MinConn < 0 )	// Traitement des erreurs sur labels
		{
		if( (NetItemRef->m_Type == NET_GLOBLABEL) ||
			(NetItemRef->m_Type == NET_GLOBBUSLABELMEMBER) )
			{
			Marker->m_Comment.Printf( _("Warning GLabel %s not connected to SheetLabel"),
					NetItemRef->m_Label);
			}
		else Marker->m_Comment.Printf( _("Warning SheetLabel %s not connected to GLabel"),
					NetItemRef->m_Label);

		if( screen == panel->GetScreen() ) RedrawOneStruct(panel, DC, Marker, GR_COPY);
		return;
		}

	ii = NetItemRef->m_ElectricalType;

	if( NetItemTst == NULL )
		{
		if( MinConn == NOC )	/* 1 seul element dans le net */
			{
			Marker->m_Comment.Printf( _("Warning Pin %s Unconnected"), MsgPinElectricType[ii]);
			if( screen == panel->GetScreen() )
				RedrawOneStruct(panel, DC, Marker, GR_COPY);
			return;
			}

		if( MinConn == NOD )	/* pas de pilotage du net */
			{
			Marker->m_Comment.Printf(
				_("Warning Pin %s not driven (Net %d)"),
				MsgPinElectricType[ii], NetItemRef->m_NetCode);
			if( screen == panel->GetScreen() )
				RedrawOneStruct(panel, DC, Marker, GR_COPY);
			return;
			}

		if( Diag == UNC )
			{
			Marker->m_Comment.Printf( 
                _("Warning More than 1 Pin connected to UnConnect symbol") );
			if( screen == panel->GetScreen() )
				RedrawOneStruct(panel, DC, Marker, GR_COPY);
			return;
			}

		}

	if( NetItemTst )		 /* Erreur entre 2 pins */
	{
		jj = NetItemTst->m_ElectricalType;
		DiagLevel = _("Warning");
		if(Diag == ERR)
		{
			DiagLevel = _("Error");
			Marker->m_MarkFlags = ERR;
			g_EESchemaVar.NbWarningErc--;
		}


		Marker->m_Comment.Printf( _("%s: Pin %s connected to Pin %s (net %d)"), DiagLevel.GetData(),
					 MsgPinElectricType[ii],
					 MsgPinElectricType[jj], NetItemRef->m_NetCode);

		if( screen == panel->GetScreen() )
			RedrawOneStruct(panel, DC, Marker, GR_COPY);
	}
}


/********************************************************************/
static void TestOthersItems(WinEDA_DrawPanel * panel, wxDC * DC,
								ObjetNetListStruct *  NetItemRef,
								ObjetNetListStruct *  netstart,
								int * NetNbItems, int * MinConnexion)
/********************************************************************/
/* Routine testant les conflits electriques entre
	NetItemRef
	et les autres items du meme net
*/
{
ObjetNetListStruct * NetItemTst, * Lim;
int ref_elect_type, jj, erc = OK, local_minconn;

	/* Analyse de la table des connexions : */
	Lim = g_TabObjNet + g_NbrObjNet; // pointe la fin de la liste
	ref_elect_type = NetItemRef->m_ElectricalType;

	NetItemTst = netstart;
	local_minconn = NOC;

	/* Examen de la liste des Pins connectees a NetItemRef */
	for ( ; ; NetItemTst ++ )
		{
		if ( NetItemRef == NetItemTst ) continue;

		/* Est - on toujours dans le meme net ? */
		if( (NetItemTst >= Lim) ||  	// fin de liste (donc fin de net)
			(NetItemRef->m_NetCode != NetItemTst->m_NetCode) )  // fin de net
			{	/* Fin de netcode trouve: Tst connexion minimum */
			if( (*MinConnexion < NET_NC ) &&
				 (local_minconn < NET_NC ) ) /* pin non connectée ou non pilotee */
				{
				Diagnose(panel, DC, NetItemRef, NULL, local_minconn, WAR);
				* MinConnexion = DRV;	// inhibition autres messages de ce type pour ce net
				}
			return;
			}

		switch ( NetItemTst->m_Type )
			{
			case NET_SEGMENT:
			case NET_BUS:
			case NET_JONCTION:
			case NET_LABEL:
			case NET_GLOBLABEL:
			case NET_BUSLABELMEMBER:
			case NET_GLOBBUSLABELMEMBER:
			case NET_SHEETBUSLABELMEMBER:
			case NET_SHEETLABEL:
			case NET_PINLABEL:
				break;

			case NET_NOCONNECT:
				local_minconn = MAX( NET_NC, local_minconn);
				break;

			case NET_PIN:
				jj = NetItemTst->m_ElectricalType;
				local_minconn = MAX( MinimalReq[ref_elect_type][jj], local_minconn );

				if ( NetItemTst <= NetItemRef ) break;
				*NetNbItems += 1;
				if( erc == OK )		// 1 marqueur par pin maxi
					{
					erc = DiagErc[ref_elect_type][jj];
					if (erc != OK )
						{
					   if( NetItemTst->m_FlagOfConnection == 0 )
							{
							Diagnose(panel, DC, NetItemRef, NetItemTst, 0, erc);
							NetItemTst->m_FlagOfConnection = (IsConnectType) 1;
							}
						}
					}
				break;
			}
		}
}


/********************************************************/
static bool WriteDiagnosticERC(const wxString & FullFileName)
/*********************************************************/
/* Genere le fichier des diagnostics
*/
{
SCH_SCREEN * Window;
EDA_BaseStruct * DrawStruct;
DrawMarkerStruct * Marker;
char Line[256];
static FILE * OutErc;
DrawSheetStruct * Sheet;
wxString msg;
	
	if( (OutErc = wxFopen( FullFileName, wxT("wt"))) == NULL ) return FALSE;

	DateAndTime(Line);
	msg = _("ERC control");
	fprintf( OutErc, "%s (%s)\n", CONV_TO_UTF8(msg), Line);

	for( Window = ScreenSch; Window != NULL; Window = (SCH_SCREEN*)Window->Pnext )
		{
		Sheet = (DrawSheetStruct *) Window->m_Parent;

		msg.Printf( _("\n***** Sheet %d (%s)\n"),
							Window->m_SheetNumber,
							Sheet ? Sheet->m_Field[VALUE].m_Text.GetData() : _("Root"));
		fprintf( OutErc, "%s", CONV_TO_UTF8(msg));

		DrawStruct = Window->EEDrawList;
		for ( ; DrawStruct != NULL; DrawStruct = DrawStruct->Pnext)
			{
			if(DrawStruct->m_StructType != DRAW_MARKER_STRUCT_TYPE )
				continue;
			/* Marqueur trouve */

			Marker = (DrawMarkerStruct * ) DrawStruct;
			if( Marker->m_Type != MARQ_ERC ) continue;
			/* Write diag marqueur */
			msg.Printf( _("ERC: %s (X= %2.3f inches, Y= %2.3f inches\n"),
								 Marker->GetComment().GetData(),
								 (float)Marker->m_Pos.x / 1000,
								 (float)Marker->m_Pos.y / 1000);
			fprintf( OutErc, "%s", CONV_TO_UTF8(msg));
			}
		}
	msg.Printf( _("\n >> Errors ERC: %d\n"), g_EESchemaVar.NbErrorErc);
	fprintf( OutErc, "%s", CONV_TO_UTF8(msg));
	fclose ( OutErc );

	return TRUE;
}


/***********************************************************************/
void TestLabel(WinEDA_DrawPanel * panel, wxDC * DC,
		ObjetNetListStruct * NetItemRef, ObjetNetListStruct * StartNet)
/***********************************************************************/
/* Routine controlant qu'un sheetLabel est bien connecte a un Glabel de la
sous-feuille correspondante
*/
{
ObjetNetListStruct * NetItemTst, * Lim;
int erc = 1;

	/* Analyse de la table des connexions : */
	Lim = g_TabObjNet + g_NbrObjNet;

	NetItemTst = StartNet;

	/* Examen de la liste des Labels connectees a NetItemRef */
	for ( ; ; NetItemTst ++ )
		{
		if( NetItemTst == NetItemRef ) continue;

		/* Est - on toujours dans le meme net ? */
		if( ( NetItemTst ==  Lim ) ||
			( NetItemRef->m_NetCode != NetItemTst->m_NetCode ) )
			{	/* Fin de netcode trouve */
			if( erc )
				{  /* GLabel ou SheetLabel orphelin */
				Diagnose(panel, DC, NetItemRef, NULL, -1, WAR);
				}
			return;
			}

		if( (NetItemRef->m_Type == NET_GLOBLABEL) ||
			(NetItemRef->m_Type == NET_GLOBBUSLABELMEMBER) )
			{
			switch ( NetItemTst->m_Type )
				{
				case NET_SEGMENT:
				case NET_BUS:
				case NET_JONCTION:
				case NET_LABEL:
				case NET_GLOBLABEL:
				case NET_BUSLABELMEMBER:
				case NET_GLOBBUSLABELMEMBER:
				case NET_PINLABEL:
				case NET_NOCONNECT:
				case NET_PIN:
					break;

				case NET_SHEETBUSLABELMEMBER:
				case NET_SHEETLABEL:
					/* Tst si le GLabel est bien dans la bonne sousfeuille */
					if( NetItemRef->m_SheetNumber == NetItemTst->m_NumInclude )
						{
						erc = 0;
						}
					break;
				} 
			} 

		else
			{
			switch ( NetItemTst->m_Type )
				{
				case NET_SEGMENT:
				case NET_BUS:
				case NET_JONCTION:
				case NET_LABEL:
				case NET_BUSLABELMEMBER:
				case NET_SHEETBUSLABELMEMBER:
				case NET_SHEETLABEL:
				case NET_PINLABEL:
				case NET_NOCONNECT:
				case NET_PIN:
					break;

				case NET_GLOBLABEL:
				case NET_GLOBBUSLABELMEMBER:
					/* Tst si le GLabel est bien dans la bonne sous-feuille */
					if( NetItemTst->m_SheetNumber == NetItemRef->m_NumInclude )
						{
						erc = 0;
						}
					break;
				} 
			}
		}
}


