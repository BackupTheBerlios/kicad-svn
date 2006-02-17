		/***************************************/
		/**** Eeschema: Routine de trace PS ****/
		/***************************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"
#include "worksheet.h"
#include "plot_common.h"
#include "protos.h"


/* coeff de conversion dim en 1 mil -> dim en unite PS: */
#define SCALE_PS 0.001

extern void Move_Plume( wxPoint pos, int plume );
extern void Plume( int plume );

enum PageFormatReq {
	PAGE_SIZE_AUTO,
	PAGE_SIZE_A4,
	PAGE_SIZE_A
};


/* Variables locales : */
static int s_DefaultPenWidth = 6;			/* default pen width for drawings in 1/1000 inch */
static int PS_SizeSelect = PAGE_SIZE_AUTO;
extern FILE * PlotOutput;
static bool Plot_Sheet_Ref = TRUE;

/* Routines Locales */

	/**************************************************/
	/* Description des menus de traitement de plot PS */
	/**************************************************/

/* Classe de la frame de gestion de l'impression */
class WinEDA_PlotPSFrame: public wxDialog
{
private:

	wxRadioBox * m_SizeOption;
	wxRadioBox * m_PlotPSColorOption;
	wxTextCtrl * m_MsgBox;
	wxCheckBox * m_Plot_Sheet_Ref;

public:
	// Constructor and destructor
	WinEDA_PlotPSFrame(wxWindow *parent, wxPoint& pos);
	~WinEDA_PlotPSFrame(void) {};

private:
	void PS_Plot(wxCommandEvent& event);
	void SetSizeOption(wxCommandEvent& event);
	void Genere_PS(int AllPages, int pagesize);
	void PlotOneSheetPS(const wxString & FileName,
		BASE_SCREEN * Window, Ki_PageDescr * sheet, int BBox[4], wxPoint plot_offset);
	void OnQuit(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};



/***********************************************************/
void WinEDA_SchematicFrame::ToPlot_PS(wxCommandEvent& event)
/***********************************************************/
/* fonction relai de creation de la frame de dialogue pour trace Postscript
*/
{
wxPoint pos;

	pos = GetPosition();
	pos.x += 10; pos.y += 20;
	WinEDA_PlotPSFrame * Ps_frame = new WinEDA_PlotPSFrame(this, pos);
	Ps_frame->ShowModal(); Ps_frame->Destroy();
}



/*************************************************/
/* Classe de la frame de gestion de Plot format PS */
/*************************************************/
enum print_id {
	ID_PLOT_PS_PAGE_SELECT = 1200,
	ID_PLOT_PS_COLOR_SELECT,
	ID_PLOT_PS_CURRENT_EXECUTE,
	ID_PLOT_PS_ALL_EXECUTE,
	ID_PLOT_PS_CANCEL,
	OPT_PLOT_SHEETREF
};

/* Construction de la table des evenements pour PlotPSFrame*/
BEGIN_EVENT_TABLE(WinEDA_PlotPSFrame, wxDialog)
	EVT_BUTTON(ID_PLOT_PS_CURRENT_EXECUTE, WinEDA_PlotPSFrame::PS_Plot)
	EVT_BUTTON(ID_PLOT_PS_ALL_EXECUTE, WinEDA_PlotPSFrame::PS_Plot)
	EVT_BUTTON(ID_PLOT_PS_CANCEL, WinEDA_PlotPSFrame::OnQuit)
	EVT_RADIOBOX(ID_PLOT_PS_PAGE_SELECT, WinEDA_PlotPSFrame::SetSizeOption)
END_EVENT_TABLE()


#define SIZE_X 420
/*******************************************************************************/
WinEDA_PlotPSFrame::WinEDA_PlotPSFrame(wxWindow *parent, wxPoint& framepos):
		wxDialog(parent, -1, _("EESchema Plot PS"), framepos, wxSize(SIZE_X, -1),
				 DIALOG_STYLE)
/*******************************************************************************/
{
wxPoint pos;
int Ymax, Y1max, w, h;

	SetFont(*g_DialogFont);

	pos.x = 20; pos.y = 10;

wxString size_list[] =
	{ _("Auto"), _("Page Size A4"), _("Page Size A") };

	m_SizeOption = new wxRadioBox(this, ID_PLOT_PS_PAGE_SELECT,
						_("Plot page size:"),
						pos,wxSize(-1,-1),
						3,size_list,1,wxRA_SPECIFY_COLS);
	m_SizeOption->SetSelection(PS_SizeSelect);
	m_SizeOption->GetSize(&w, &h);
	Y1max = pos.y + w + 5;
	pos.x += w + 15;

wxString color_opt[] =
	{ _("B/W"), _("Color")};
	m_PlotPSColorOption = new wxRadioBox(this, ID_PLOT_PS_COLOR_SELECT,
						_("Plot Color:"),
						pos,wxSize(-1,-1),
						2,color_opt,1,wxRA_SPECIFY_COLS);
	m_PlotPSColorOption->SetSelection(g_PlotPSColorOpt);
	m_PlotPSColorOption->GetSize(&w, &h);
	pos.x += w + 15;

	wxButton * Button = new wxButton(this,	ID_PLOT_PS_CURRENT_EXECUTE,
						_("&Plot CURRENT"), pos);
	Button->SetForegroundColour(wxColour(0,80,0) );

	pos.y += Button->GetSize().y + 5;
	Button = new wxButton(this,	ID_PLOT_PS_ALL_EXECUTE,
						_("Plot A&LL"), pos);
	Button->SetForegroundColour(wxColour(100,0,0) );

	pos.y += Button->GetSize().y + 5;
	Button = new wxButton(this,	ID_PLOT_PS_CANCEL,
						_("&Cancel"), pos);
	Button->SetForegroundColour(wxColour(0,0,150) );
	Ymax = pos.y + Button->GetSize().y + 5;

	pos.x = 20; pos.y = MAX(Ymax, Y1max);
	m_Plot_Sheet_Ref = new wxCheckBox(this, OPT_PLOT_SHEETREF,
			_("Print Sheet Ref"),
			pos);
	m_Plot_Sheet_Ref->SetValue(Plot_Sheet_Ref);

	pos.x = 6; pos.y += m_Plot_Sheet_Ref->GetSize().y + 8;
	new wxStaticText(this, -1, _("Messages :"), pos);
	pos.y += 15;
	m_MsgBox = new wxTextCtrl(this, -1, wxEmptyString, pos, wxSize(SIZE_X - 20, 200),
			wxTE_MULTILINE | wxTE_READONLY);

	pos.y += m_MsgBox->GetSize().y;

	SetClientSize(wxSize(SIZE_X,pos.y + 10));
}

/****************************************************************/
void  WinEDA_PlotPSFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
/****************************************************************/
{
	g_PlotPSColorOpt = m_PlotPSColorOption->GetSelection();
	Close(true);// true is to force the frame to close
}

/**************************/
/* Fonctions d'impression */
/**************************/

void WinEDA_PlotPSFrame::SetSizeOption(wxCommandEvent& event)
{
	PS_SizeSelect = m_SizeOption->GetSelection();
}



/*****************************************************/
void WinEDA_PlotPSFrame::PS_Plot(wxCommandEvent& event)
/*****************************************************/
{
int Select_PlotAll = FALSE;

	g_PlotPSColorOpt = m_PlotPSColorOption->GetSelection();

	if( event.GetId() == ID_PLOT_PS_ALL_EXECUTE ) Select_PlotAll = TRUE;

	Genere_PS(Select_PlotAll, PS_SizeSelect);
	m_MsgBox->AppendText( wxT("*****\n"));
}



/*************************************************************/
void WinEDA_PlotPSFrame::Genere_PS(int AllPages, int pagesize)
/*************************************************************/
{
wxString PlotFileName, ShortFileName;
BASE_SCREEN *screen;
Ki_PageDescr * PlotSheet, * RealSheet;
int BBox[4];
wxPoint plot_offset;

	SchematicCleanUp(NULL);
	g_PlotFormat = PLOT_FORMAT_POST;

	screen = ActiveScreen;
	if ( AllPages == TRUE )
	{
		screen = ScreenSch;
	}
	while( screen )
	{
		PlotSheet = screen->m_CurrentSheet;
		RealSheet = &g_Sheet_A4;
		if ( pagesize == PAGE_SIZE_AUTO ) RealSheet = PlotSheet;
		else if ( pagesize == PAGE_SIZE_A )	RealSheet = &g_Sheet_A;

		/* Calcul des limites de trace en 1/1000 pouce */
		BBox[0] = BBox[1] = g_PlotMargin;	// Plot margin in 1/1000 inch
		BBox[2] = RealSheet->m_Size.x - g_PlotMargin;
		BBox[3] = RealSheet->m_Size.y - g_PlotMargin;

		/* Calcul des echelles de conversion */
		g_PlotScaleX = SCALE_PS *
					(float) (BBox[2] - BBox[0]) /
					PlotSheet->m_Size.x;
		g_PlotScaleY = SCALE_PS *
					(float) (BBox[3] - BBox[1]) /
					PlotSheet->m_Size.y;

		plot_offset.x = 0;
		plot_offset.y = PlotSheet->m_Size.y;

		wxSplitPath(screen->m_FileName.GetData(), (wxString*) NULL,
				&ShortFileName, (wxString*) NULL);
		wxString dirbuf = wxGetCwd() + STRING_DIR_SEP;
		if( ! ShortFileName.IsEmpty() )
			PlotFileName = MakeFileName(dirbuf, ShortFileName, wxT(".ps"));
		else PlotFileName = MakeFileName(dirbuf, g_DefaultSchematicFileName, wxT(".ps"));

		PlotOneSheetPS(PlotFileName,screen, RealSheet, BBox, plot_offset);
		screen = (BASE_SCREEN*)screen->Pnext;
		if (AllPages == FALSE ) screen = NULL;
	}
}


/**********************************************************/
void WinEDA_PlotPSFrame::PlotOneSheetPS(const wxString & FileName,
			BASE_SCREEN * screen, Ki_PageDescr * sheet, int BBox[4], wxPoint plot_offset)
/**********************************************************/
/* Trace en format PS. d'une feuille de dessin
*/
{
wxString Line;
EDA_BaseStruct *DrawList;
EDA_SchComponentStruct *DrawLibItem;
int x1=0, y1=0, x2=0, y2=0, layer;

	PlotOutput = wxFopen(FileName, wxT("wt"));
	if (PlotOutput == NULL)
	{
		Line = wxT("\n** ");
		Line += _("Unable to create ") + FileName + wxT(" **\n\n");
		m_MsgBox->AppendText(Line);
		wxBell();
		return ;
	}

	Line.Printf(_("Plot: %s\n"), FileName.GetData()) ;
	m_MsgBox->AppendText(Line);

	InitPlotParametresPS(plot_offset, sheet, g_PlotScaleX, g_PlotScaleY);
	SetDefaultLineWidthPS( s_DefaultPenWidth);

	/* Init : */
	PrintHeaderPS(PlotOutput, wxT("EESchema-PS"), FileName, BBox);
	InitPlotParametresPS(plot_offset, sheet, 1.0, 1.0);

	if ( m_Plot_Sheet_Ref->GetValue() )
	{
		if ( (g_PlotFormat == PLOT_FORMAT_POST) && g_PlotPSColorOpt )
			SetColorMapPS ( BLACK );
		PlotWorkSheet(PLOT_FORMAT_POST, screen);
	}

	DrawList = screen->EEDrawList;
	while ( DrawList )	/* tracage */
	{
		Plume('U');
		layer = LAYER_NOTES;
		switch( DrawList->m_StructType )
		{
			case DRAW_BUSENTRY_STRUCT_TYPE :		/* Struct Raccord et Segment sont identiques */
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
				if ( g_PlotPSColorOpt )
					SetColorMapPS ( ReturnLayerColor(layer) );
				switch (layer)
					{
					case LAYER_NOTES: /* Trace en pointilles */
						fprintf(PlotOutput,"[50 50] 0 setdash\n");
						Move_Plume( wxPoint(x1,y1), 'U');
						Move_Plume( wxPoint(x2,y2), 'D');
						fprintf(PlotOutput,"[] 0 setdash\n");
						break;

					case LAYER_BUS:	/* Trait large */
						{
						fprintf(PlotOutput,"%d setlinewidth\n", s_DefaultPenWidth * 3);
						Move_Plume( wxPoint(x1,y1),'U');
						Move_Plume( wxPoint(x2,y2),'D');
						fprintf(PlotOutput,"%d setlinewidth\n", s_DefaultPenWidth);
						}
						break;

					default:
						Move_Plume( wxPoint(x1,y1),'U');
						Move_Plume( wxPoint(x2,y2),'D');
						break;
					}
				break;

			case DRAW_JUNCTION_STRUCT_TYPE :
				#undef STRUCT
				#define STRUCT ((DrawJunctionStruct*)DrawList)
				if ( g_PlotPSColorOpt )
					SetColorMapPS (ReturnLayerColor(STRUCT->m_Layer) );
				PlotCercle( STRUCT->m_Pos, DRAWJUNCTION_SIZE);
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
			case DRAW_SHEETLABEL_STRUCT_TYPE: break;
			case DRAW_MARKER_STRUCT_TYPE : break;

			case DRAW_SHEET_STRUCT_TYPE :
				#undef STRUCT
				#define STRUCT ((DrawSheetStruct*)DrawList)
				PlotSheetStruct(STRUCT);
				break;

			case DRAW_NOCONNECT_STRUCT_TYPE:
				#undef STRUCT
				#define STRUCT ((DrawNoConnectStruct*)DrawList)
				if ( g_PlotPSColorOpt )
					SetColorMapPS (ReturnLayerColor(LAYER_NOCONNECT) );
				PlotNoConnectStruct(STRUCT);
				break;

			default:
				break;
		}

		Plume('U');
		DrawList = DrawList->Pnext;
	}

	/* fin */
	CloseFilePS(PlotOutput);

	m_MsgBox->AppendText( wxT("Ok\n"));
}

