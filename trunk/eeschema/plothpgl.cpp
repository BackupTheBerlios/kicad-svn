		/*******************************/
		/**** Routine de trace HPGL ****/
		/*******************************/
#ifdef __GNUG__
#pragma implementation
#endif

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"
#include "worksheet.h"
#include "trigo.h"
#include "plot_common.h"

#include "protos.h"

#include "wx/spinctrl.h"
/* coeff de conversion dim en 1 mil -> dim en unite HPGL: */
#define SCALE_HPGL 1.02041

extern void Move_Plume( wxPoint pos, int plume );
extern void Plume( int plume );

/* Variables locales : */
FILE * PlotOutput;		/* exportee dans printps.cc */
static double Scale_X = 1;
static double Scale_Y = 1;
int HPGL_SizeSelect;

enum PageFormatReq {
	PAGE_DEFAULT = 0,
	PAGE_SIZE_A4,
	PAGE_SIZE_A3,
	PAGE_SIZE_A2,
	PAGE_SIZE_A1,
	PAGE_SIZE_A0,
	PAGE_SIZE_A,
	PAGE_SIZE_B,
	PAGE_SIZE_C,
	PAGE_SIZE_D,
	PAGE_SIZE_E
};

static W_PLOT * Plot_sheet_list[] =
{
	NULL,
	&g_Sheet_A4,
	&g_Sheet_A3,
	&g_Sheet_A2,
	&g_Sheet_A1,
	&g_Sheet_A0,
	&g_Sheet_A,
	&g_Sheet_B,
	&g_Sheet_C,
	&g_Sheet_D,
	&g_Sheet_E,
	&g_Sheet_GERBER,
	&g_Sheet_user
};

/* Routines Locales */


	/**************************************************/
	/* Description des menus de traitement de plot HPGL */
	/**************************************************/

/* Classe de la frame de gestion de l'impression */
class WinEDA_PlotHPGLFrame: public wxDialog
{
public:

	WinEDA_DrawFrame * m_Parent;

	wxRadioBox * m_SizeOption;
	wxSpinCtrl * m_ButtPenNum;
	wxString m_Buff_PenNum;
	wxSpinCtrl * m_ButtPenWidth;
	wxString m_Buff_Width;
	wxSpinCtrl * m_ButtPenSpeed;
	wxString m_Buff_Speed;
	WinEDA_PositionCtrl * m_PlotOrgPosition;
	wxTextCtrl * m_MsgBox;
	wxPoint m_WinOrgPos;

	// Constructor and destructor
	WinEDA_PlotHPGLFrame(WinEDA_DrawFrame *parent, wxPoint& pos);
	~WinEDA_PlotHPGLFrame(void) {};

	void OnQuit(wxCommandEvent& event);
	void HPGL_Plot(wxCommandEvent& event);
	void SetSizeOption(wxCommandEvent& event);
	void AcceptPlotOffset(wxCommandEvent& event);
	void SetPenWidth(wxSpinEvent& event);
	void SetPenSpeed(wxSpinEvent& event);
	void SetPenNum(wxSpinEvent& event);
	void Plot_1_Page_HPGL(const wxString &  FullFileName,BASE_SCREEN * screen);
	void Plot_Schematic_HPGL(int Select_PlotAll, int HPGL_SheetSize);
	void ReturnSheetDims( BASE_SCREEN * screen, wxSize & SheetSize, wxPoint & SheetOffset);

	DECLARE_EVENT_TABLE()

};


/**************************************************************/
void WinEDA_SchematicFrame::ToPlot_HPGL(wxCommandEvent& event)
/**************************************************************/
{
wxPoint pos;

	pos = GetPosition();
	pos.x += 10; pos.y+= 20;
	WinEDA_PlotHPGLFrame * HPGL_frame = new WinEDA_PlotHPGLFrame(this, pos);
	HPGL_frame->ShowModal(); HPGL_frame->Destroy();
}



/*************************************************/
/* Classe de la frame de gestion de Plot format HPGL */
/*************************************************/
enum print_id {
	ID_PLOT_HPGL_PAGE_SELECT = 1300,
	ID_PLOT_HPGL_CURRENT_EXECUTE,
	ID_PLOT_HPGL_ALL_EXECUTE,
	ID_PLOT_HPGL_CANCEL,
	ID_PEN_NUMBER,
	ID_PEN_WIDTH,
	ID_PEN_SPEED,
	ID_PLOT_ACCEPT_OFFSET
};

/* Construction de la table des evenements pour PlotHPGLFrame*/
BEGIN_EVENT_TABLE(WinEDA_PlotHPGLFrame, wxDialog)
	EVT_BUTTON(ID_PLOT_HPGL_CURRENT_EXECUTE, WinEDA_PlotHPGLFrame::HPGL_Plot)
	EVT_BUTTON(ID_PLOT_HPGL_ALL_EXECUTE, WinEDA_PlotHPGLFrame::HPGL_Plot)
	EVT_BUTTON(ID_PLOT_HPGL_CANCEL, WinEDA_PlotHPGLFrame::OnQuit)
	EVT_BUTTON(ID_PLOT_ACCEPT_OFFSET, WinEDA_PlotHPGLFrame::AcceptPlotOffset)
	EVT_RADIOBOX(ID_PLOT_HPGL_PAGE_SELECT, WinEDA_PlotHPGLFrame::SetSizeOption)
	EVT_SPINCTRL(ID_PEN_WIDTH, WinEDA_PlotHPGLFrame::SetPenWidth)
	EVT_SPINCTRL(ID_PEN_SPEED, WinEDA_PlotHPGLFrame::SetPenSpeed)
	EVT_SPINCTRL(ID_PEN_NUMBER, WinEDA_PlotHPGLFrame::SetPenNum)
END_EVENT_TABLE()

#define SIZE_X 340
/***************************************************************************************/
WinEDA_PlotHPGLFrame::WinEDA_PlotHPGLFrame(WinEDA_DrawFrame *parent, wxPoint& framepos):
		wxDialog(parent, -1, _("EESchema Plot HPGL"), framepos, wxSize(SIZE_X, -1),
				 DIALOG_STYLE)
/***************************************************************************************/
{
wxPoint pos;
int low_y = 0;

	m_Parent = parent;
	SetFont(*g_DialogFont);

	pos.x = 20; pos.y = 10;

wxString size_list[] =
    { _("Sheet Size"),
	_("Page Size A4"), _("Page Size A3"),
    _("Page Size A2"), _("Page Size A1"),_("Page Size A0"),
	_("Page Size A"), _("Page Size B"),
    _("Page Size C"), _("Page Size D"), _("Page Size E") };
	m_SizeOption = new wxRadioBox(this, ID_PLOT_HPGL_PAGE_SELECT,
						_("Plot page size:"),
						pos,wxSize(-1,-1),
						11,size_list,1,wxRA_SPECIFY_COLS);
	m_SizeOption->SetSelection(HPGL_SizeSelect);

	int xx, yy; m_SizeOption->GetSize(&xx, &yy);
	pos.y += 10 + yy; m_WinOrgPos = pos;
	m_PlotOrgPosition = new WinEDA_PositionCtrl(this, _("Plot Offset"),
						HPGL_SizeSelect ? Plot_sheet_list[HPGL_SizeSelect]->m_Offset:
						wxPoint(0,0),
						UnitMetric, m_WinOrgPos);
	if (HPGL_SizeSelect == PAGE_DEFAULT) m_PlotOrgPosition->Enable(FALSE);
	low_y = pos.y + m_PlotOrgPosition->GetDimension().y;


	pos.x = 180; pos.y += 30;
	wxButton * Button = new wxButton(this,	ID_PLOT_ACCEPT_OFFSET,
						_("&Accept Offset"), pos);
	Button->SetForegroundColour(wxColour(100,0,0) );


	pos.y = 10;
	new wxStaticText(this,-1, _("Pen Width ( mils )"), pos);
	pos.y += 15;
	m_Buff_Width << g_HPGL_Pen_Descr.m_Pen_Diam;
	m_ButtPenWidth = new wxSpinCtrl(this, ID_PEN_WIDTH, m_Buff_Width, pos);
	m_ButtPenWidth->SetRange(1,100);

	pos.y += 30;
	new wxStaticText(this,-1, _("Pen Speed ( cm/s )"), pos);
	pos.y += 15;
	m_Buff_Speed << g_HPGL_Pen_Descr.m_Pen_Speed;
	m_ButtPenSpeed = new wxSpinCtrl(this, ID_PEN_SPEED, m_Buff_Speed, pos);
	m_ButtPenSpeed->SetRange(1,40);

	pos.y += 30;
	new wxStaticText(this,-1, _("Pen Number"), pos);
	pos.y += 15;
	m_Buff_PenNum << g_HPGL_Pen_Descr.m_Pen_Num;
	m_ButtPenNum = new wxSpinCtrl(this, ID_PEN_SPEED, m_Buff_PenNum, pos);
	m_ButtPenNum->SetRange(1,8);

	pos.y += 30;
	Button = new wxButton(this,	ID_PLOT_HPGL_CURRENT_EXECUTE,
						_("&Plot CURRENT"), pos);
	Button->SetForegroundColour(wxColour(0,100,0) );

	pos.y += Button->GetDefaultSize().y + 5;
	Button = new wxButton(this,	ID_PLOT_HPGL_ALL_EXECUTE,
						_("Plot A&LL"), pos);
	Button->SetForegroundColour(wxColour(0,0,150) );

	pos.y += Button->GetDefaultSize().y + 5;
	Button = new wxButton(this,	ID_PLOT_HPGL_CANCEL,
						_("&Cancel"), pos);
	Button->SetForegroundColour(wxColour(100,0,0) );

	pos.x = 5; pos.y = low_y+5;
	m_MsgBox = new wxTextCtrl(this, -1, "", pos, wxSize(SIZE_X-10, 110),
			wxTE_MULTILINE | wxTE_READONLY);

	low_y += m_MsgBox->GetSize().y;

	SetClientSize(wxSize(SIZE_X,low_y + 10));
}


/******************************************************************/
void  WinEDA_PlotHPGLFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
/******************************************************************/
{
    // true is to force the frame to close
    Close(true);
}


/************************************************************/
void WinEDA_PlotHPGLFrame::SetPenWidth(wxSpinEvent & event)
/************************************************************/
{
	g_HPGL_Pen_Descr.m_Pen_Diam = m_ButtPenWidth->GetValue();
	if ( g_HPGL_Pen_Descr.m_Pen_Diam > 100 ) g_HPGL_Pen_Descr.m_Pen_Diam = 100;
	if ( g_HPGL_Pen_Descr.m_Pen_Diam < 1 ) g_HPGL_Pen_Descr.m_Pen_Diam = 1;

	m_Buff_Width.Printf("%d", g_HPGL_Pen_Descr.m_Pen_Diam);
	m_ButtPenWidth->SetValue(m_Buff_Width);
}


/*********************************************************/
void WinEDA_PlotHPGLFrame::SetPenSpeed(wxSpinEvent& event)
/*********************************************************/
{
	g_HPGL_Pen_Descr.m_Pen_Speed = m_ButtPenSpeed->GetValue();
	if ( g_HPGL_Pen_Descr.m_Pen_Speed > 40 ) g_HPGL_Pen_Descr.m_Pen_Speed = 40;
	if ( g_HPGL_Pen_Descr.m_Pen_Speed < 1 ) g_HPGL_Pen_Descr.m_Pen_Speed = 1;

	m_Buff_Speed.Printf("%d", g_HPGL_Pen_Descr.m_Pen_Speed);
	m_ButtPenSpeed->SetValue(m_Buff_Speed);
}

/*******************************************************/
void WinEDA_PlotHPGLFrame::SetPenNum(wxSpinEvent& event)
/*******************************************************/
{
	g_HPGL_Pen_Descr.m_Pen_Num = m_ButtPenNum->GetValue();
	if ( g_HPGL_Pen_Descr.m_Pen_Num > 8 ) g_HPGL_Pen_Descr.m_Pen_Num = 8;
	if ( g_HPGL_Pen_Descr.m_Pen_Num < 1 ) g_HPGL_Pen_Descr.m_Pen_Num = 1;

	m_Buff_PenNum.Printf("%d", g_HPGL_Pen_Descr.m_Pen_Num);
	m_ButtPenNum->SetValue(m_Buff_PenNum);
}

/*****************************************************************/
void WinEDA_PlotHPGLFrame::AcceptPlotOffset(wxCommandEvent& event)
/*****************************************************************/
{
int ii = m_SizeOption->GetSelection();

	if ( ii <= 0 ) HPGL_SizeSelect = 0;
	else HPGL_SizeSelect  = ii;

	if ( HPGL_SizeSelect )
		Plot_sheet_list[HPGL_SizeSelect]->m_Offset = m_PlotOrgPosition->GetCoord();
}

/**************************/
/* Fonctions d'impression */
/**************************/

void WinEDA_PlotHPGLFrame::SetSizeOption(wxCommandEvent& event)
{
int ii = m_SizeOption->GetSelection();

	if ( ii <= 0 ) HPGL_SizeSelect = 0;
	else HPGL_SizeSelect  = ii;

	delete m_PlotOrgPosition;
	m_PlotOrgPosition = new WinEDA_PositionCtrl(this, _("Plot Offset"),
						HPGL_SizeSelect ? Plot_sheet_list[HPGL_SizeSelect]->m_Offset:
						wxPoint(0,0),
						UnitMetric, m_WinOrgPos);
	if (HPGL_SizeSelect == PAGE_DEFAULT) m_PlotOrgPosition->Enable(FALSE);
	else m_PlotOrgPosition->Enable(TRUE);
}


/***********************************************************/
void WinEDA_PlotHPGLFrame::HPGL_Plot(wxCommandEvent& event)
/***********************************************************/
{
int Select_PlotAll = FALSE;

	if( event.GetId() == ID_PLOT_HPGL_ALL_EXECUTE ) Select_PlotAll = TRUE;

	if ( HPGL_SizeSelect )
		Plot_sheet_list[HPGL_SizeSelect]->m_Offset = m_PlotOrgPosition->GetCoord();

	Plot_Schematic_HPGL(Select_PlotAll, HPGL_SizeSelect);
}

/*******************************************************************/
void WinEDA_PlotHPGLFrame::ReturnSheetDims( BASE_SCREEN * screen,
							wxSize & SheetSize, wxPoint & SheetOffset)
/*******************************************************************/
/* Fonction calculant les dims et offsets de trace de la feuille selectionnee
	retourne:
*/
{
W_PLOT * PlotSheet;

	if( screen == NULL ) screen = ActiveScreen;

	PlotSheet = screen->m_CurrentSheet;

	SheetSize = PlotSheet->m_Size;
	SheetOffset = PlotSheet->m_Offset;
}

/***********************************************************************************/
void WinEDA_PlotHPGLFrame::Plot_Schematic_HPGL(int Select_PlotAll, int HPGL_SheetSize)
/***********************************************************************************/
{
wxString PlotFileName, ShortFileName;
BASE_SCREEN *screen;
W_PLOT * PlotSheet;
wxSize SheetSize;
wxPoint SheetOffset, PlotOffset;
int margin;

	SchematicCleanUp(NULL);
	g_PlotFormat = PLOT_FORMAT_HPGL;

	screen = ActiveScreen;
	if ( Select_PlotAll == TRUE )
	{
		screen = ScreenSch;
	}
	while( screen )
	{
		ReturnSheetDims(screen, SheetSize, SheetOffset);
		/* Calcul des echelles de conversion */
		g_PlotScaleX = Scale_X * SCALE_HPGL ;
		g_PlotScaleY = Scale_Y * SCALE_HPGL ;

		margin = 400;	// Margin in mils
		PlotSheet = screen->m_CurrentSheet;
		g_PlotScaleX = g_PlotScaleX * (SheetSize.x - 2 * margin)/ PlotSheet->m_Size.x;
		g_PlotScaleY = g_PlotScaleY * (SheetSize.y - 2 * margin) / PlotSheet->m_Size.y;

		/* calcul des offsets */
		PlotOffset.x = - (int)(SheetOffset.x * SCALE_HPGL);
		PlotOffset.y = (int)( (SheetOffset.y + SheetSize.y) * SCALE_HPGL);
		PlotOffset.x -= (int)(margin * SCALE_HPGL);
		PlotOffset.y += (int)(margin * SCALE_HPGL);

		wxSplitPath(screen->m_FileName.GetData(), (wxString*) NULL,
				&ShortFileName, (wxString*) NULL);
		wxString dirbuf = wxGetCwd() + STRING_DIR_SEP;
		if( ShortFileName != "" )
			PlotFileName = MakeFileName(dirbuf, ShortFileName, ".plt");
		else PlotFileName = MakeFileName(dirbuf, g_DefaultSchematicFileName, ".plt");

		InitPlotParametresHPGL(PlotOffset, g_PlotScaleX, g_PlotScaleY);
		Plot_1_Page_HPGL(PlotFileName,screen);
		screen = (BASE_SCREEN*)screen->Pnext;
		if ( Select_PlotAll == FALSE ) screen = NULL;
	}

	m_MsgBox->AppendText(_("** Plot End **\n"));
}


/**************************************************************************/
void WinEDA_PlotHPGLFrame::Plot_1_Page_HPGL(const wxString & FullFileName,
			BASE_SCREEN * screen)
/**************************************************************************/

/* Trace en format HPGL. d'une feuille de dessin
	1 unite HPGL = 0.98 mils ( 1 mil = 1.02041 unite HPGL ) .
*/
{
EDA_BaseStruct *DrawList;
EDA_SchComponentStruct *DrawLibItem;
int x1=0, y1=0, x2=0, y2=0, layer;
wxString msg;

	PlotOutput = fopen(FullFileName.GetData(),"wt");
	if (PlotOutput == 0)
		{
		msg = _("Unable to create ") + FullFileName;
		DisplayError(this, msg); return ;
		}

	msg = _("Plot  ") + FullFileName + "\n";
	m_MsgBox->AppendText(msg);

	/* Init : */
	PrintHeaderHPGL(PlotOutput, g_HPGL_Pen_Descr.m_Pen_Speed, g_HPGL_Pen_Descr.m_Pen_Num);

	PlotWorkSheet(PLOT_FORMAT_HPGL, screen, 0 );

	DrawList = screen->EEDrawList;
	while ( DrawList )	/* tracage */
		{
		Plume('U');
		layer = LAYER_NOTES;
		switch( DrawList->m_StructType )
			{
			case DRAW_BUSENTRY_STRUCT_TYPE :
				#undef STRUCT
				#define STRUCT ((DrawBusEntryStruct*)DrawList)
				x1 = STRUCT->m_Pos.x; y1 = STRUCT->m_Pos.y;
				x2 = STRUCT->m_End().x; y2 = STRUCT->m_End().y;
				layer = STRUCT->m_Layer;
			case DRAW_SEGMENT_STRUCT_TYPE :
				#undef STRUCT
				#define STRUCT ((EDA_DrawLineStruct*)DrawList)
				if ( DrawList->m_StructType == DRAW_SEGMENT_STRUCT_TYPE)
					{
					x1 = STRUCT->m_Start.x; y1 = STRUCT->m_Start.y;
					x2 = STRUCT->m_End.x; y2 = STRUCT->m_End.y;
					layer = STRUCT->m_Layer;
					}
				switch (layer)
					{
					case LAYER_NOTES: /* Trace en pointilles */
						Move_Plume( wxPoint(x1,y1),'U');
						fprintf(PlotOutput,"LT 2;\n");
						Move_Plume( wxPoint(x2,y2),'D');
						fprintf(PlotOutput,"LT;\n");
						break;

					case LAYER_BUS:	/* Trait large */
						{
						int deltaX = 0, deltaY = 0; double angle;
						if( (x2 - x1) == 0 ) deltaX = 8;
						else if( (y2 - y1) == 0 ) deltaY = 8;
						else
							{
							angle = atan2( (double)(x2-x1), (double)(y1-y2) );
							deltaX = (int)( 8 * sin(angle) );
							deltaY = (int)( 8 * cos(angle) );
							}
						Move_Plume( wxPoint(x1 + deltaX, y1 - deltaY), 'U');
						Move_Plume( wxPoint(x1 - deltaX,y1 + deltaY), 'D');
						Move_Plume( wxPoint(x2 - deltaX,y2 + deltaY), 'D');
						Move_Plume( wxPoint(x2 + deltaX,y2 - deltaY), 'D');
						Move_Plume( wxPoint(x1 + deltaX,y1 - deltaY), 'D');
						}
						break;

					default:
						Move_Plume( wxPoint(x1,y1), 'U');
						Move_Plume( wxPoint(x2,y2), 'D');
						break;
					}
				break;

			case DRAW_JUNCTION_STRUCT_TYPE :
				#undef STRUCT
				#define STRUCT ((DrawJunctionStruct*)DrawList)
				x1 = STRUCT->m_Pos.x; y1 = STRUCT->m_Pos.y;
				PlotCercle( wxPoint(x1,y1), DRAWJUNCTION_SIZE * 2);
				break;

			case DRAW_TEXT_STRUCT_TYPE :
			case DRAW_LABEL_STRUCT_TYPE :
			case DRAW_GLOBAL_LABEL_STRUCT_TYPE :
				PlotTextStruct(DrawList);
				break;

			case DRAW_LIB_ITEM_STRUCT_TYPE :
				DrawLibItem = (EDA_SchComponentStruct *) DrawList;
				PlotLibPart( DrawLibItem );
				break;

			case DRAW_PICK_ITEM_STRUCT_TYPE : break;
			case DRAW_POLYLINE_STRUCT_TYPE : break;
			case DRAW_SHEETLABEL_STRUCT_TYPE : break;
			case DRAW_MARKER_STRUCT_TYPE : break;

			case DRAW_SHEET_STRUCT_TYPE :
				#undef STRUCT
				#define STRUCT ((DrawSheetStruct*)DrawList)
				PlotSheetStruct(STRUCT);
				break;

			case DRAW_NOCONNECT_STRUCT_TYPE:
				#undef STRUCT
				#define STRUCT ((DrawNoConnectStruct*)DrawList)
				PlotNoConnectStruct(STRUCT);
				break;

			default :
				break;
			}

		Plume('U');
		DrawList = DrawList->Pnext;
		}

	/* fin */
	CloseFileHPGL(PlotOutput);
}


