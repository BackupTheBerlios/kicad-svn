		/***********************************************/
		/* Routine de Generation du fichier de percage */
		/***********************************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "plot_common.h"
#include "trigo.h"
#include "pcbnew.h"
#include "pcbplot.h"
#include "autorout.h"

#include "protos.h"


/*
	Generation du fichier de percage en format EXCELLON
	Variantes supportees:
		- Decimal : coord flottantes en pouces
		- Metric :  coord entieres en 1/10000 mm
	format "Trailling Zero" ( TZ )

	On peut aussi generer le plan de percage en format HPGL ou PS
*/

/* Routines importees */

class FORET
{
public:
	int nb;
	int diametre;
};


/* Routines Locales */
static void Init_Drill(void);
static void Fin_Drill(void);
static void PlotDrillSymbol(int x0,int y0,int diametre,int num_forme,int format);


/* Variables locales : */
static int Nb_Forets;		/* Nombre de forets a utiliser */
static float conv_unit;		/* coeff de conversion des unites drill / pcb */
static bool Unit_Drill_is_Inch = TRUE;
static wxPoint File_Drill_Offset;	/* Offset des coord de percage pour le fichier généré */

/***************************************************************************/
/* Frame de gestion des options pour la generation des fichiers de percage */
/***************************************************************************/
enum id_drill {
	ID_CREATE_DRILL_FILES = 1370,
	ID_CLOSE_DRILL,
	ID_SEL_DRILL_UNITS,
	ID_SEL_DRILL_SHEET
};

class WinEDA_DrillFrame: public wxDialog
{

	WinEDA_PcbFrame * m_Parent;
	wxRadioBox * m_Choice_Drill_Plan;
	wxRadioBox * m_Choice_Unit;
	wxRadioBox * m_Choice_Drill_Offset;
	WinEDA_EnterText * m_EnterFileNameDrill;
	WinEDA_ValueCtrl * m_ViaDrillCtrl;
	WinEDA_ValueCtrl * m_PenSpeed;
	WinEDA_ValueCtrl * m_PenNum;


	// Constructor and destructor
public:
	WinEDA_DrillFrame(WinEDA_PcbFrame *parent, const wxPoint& pos);
	~WinEDA_DrillFrame(void) {};

private:
	void OnQuit(wxCommandEvent& event);
	void SetParams(void);
	void GenDrillFile(wxCommandEvent& event);
	void GenDrillPlan(int format);
	int Plot_Drill_PcbMap( FORET * buffer, int format);
	int Gen_Liste_Forets( FORET * buffer,bool print_header);
	int Gen_Liste_Trous_EXCELLON( FORET * buffer);

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WinEDA_DrillFrame, wxDialog)
	EVT_BUTTON(ID_CLOSE_DRILL, WinEDA_DrillFrame::OnQuit)
	EVT_BUTTON(ID_CREATE_DRILL_FILES, WinEDA_DrillFrame::GenDrillFile)
END_EVENT_TABLE()

#define H_SIZE 550
#define V_SIZE 250
/*************************************************************************/
WinEDA_DrillFrame::WinEDA_DrillFrame(WinEDA_PcbFrame *parent,
					const wxPoint& framepos):
		wxDialog(parent, -1, _("Drill tools"), framepos, wxSize(H_SIZE, V_SIZE),
				 DIALOG_STYLE)
/*************************************************************************/
{
wxPoint pos;
int w, h;
wxSize size , client_size = GetClientSize();

	m_Parent = parent;
	SetFont(*g_DialogFont);

	SetReturnCode(1);

	pos.x = 10, pos.y = 5;
	if ( (framepos.x == -1) && (framepos.x == -1) ) Centre();

wxString choice_unit_msg[] =
	{_("millimeters"), _("inches") };
	m_Choice_Unit = new wxRadioBox(this, ID_SEL_DRILL_UNITS,
						_("drill units:"),
						pos,wxSize(-1,-1),
						2,choice_unit_msg,1,wxRA_SPECIFY_COLS);
	if ( Unit_Drill_is_Inch )m_Choice_Unit->SetSelection(1);

	m_Choice_Unit->GetSize(&w, &h);
	size.x = pos.x + w;
	pos.y += h + 10;
wxString choice_drill_offset_msg[] =
	{_("Absolute"), _("Auxiliary Axe")};
	m_Choice_Drill_Offset = new wxRadioBox(this, ID_SEL_DRILL_SHEET,
						_("Drill Origine:"),
						pos,wxSize(-1,-1),
						2,choice_drill_offset_msg,1,wxRA_SPECIFY_COLS);
	m_Choice_Drill_Offset->GetSize(&w, &h);
	size.y = pos.y + h;
	size.x = MAX(size.x, pos.x + w);
	pos.x = size.x + 20;
	pos.y = 5;
wxString choice_drill_plan_msg[] =
	{_("none"), _("drill sheet (HPGL)"), _("drill sheet (Postscript)")};
	m_Choice_Drill_Plan = new wxRadioBox(this, ID_SEL_DRILL_SHEET,
						_("Drill Sheet:"),
						pos,wxSize(-1,-1),
						3,choice_drill_plan_msg,1,wxRA_SPECIFY_COLS);

	/* Creation des boutons de commande */
	m_Choice_Drill_Plan->GetSize(&w, &h);
	pos.x += w + 20; pos.y += 5;
	wxButton * Button = new wxButton(this, ID_CREATE_DRILL_FILES,
						_("&Execute"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.y += Button->GetDefaultSize().y + 5;
	Button = new wxButton(this, ID_CLOSE_DRILL,
						_("&Close"), pos);
	Button->SetForegroundColour(*wxBLUE);

	pos.x = 5; pos.y = size.y + 20;
	m_ViaDrillCtrl = new WinEDA_ValueCtrl(this, _("Via Drill"),
			g_DesignSettings.m_ViaDrill, UnitMetric, pos, m_Parent->m_InternalUnits);

	pos.x += m_ViaDrillCtrl->GetDimension().x + 60;

	wxString value;
	m_PenNum = new WinEDA_ValueCtrl(this, _("Pen Number"),
			g_HPGL_Pen_Num, 2, pos, 1);

	pos.x += m_PenNum->GetDimension().x + 30;
	value.Printf("%d", g_HPGL_Pen_Speed);
	m_PenSpeed = new WinEDA_ValueCtrl(this, _("Speed(cm/s)"),
			g_HPGL_Pen_Speed, CENTIMETRE, pos, 1);

	pos.y += m_PenSpeed->GetDimension().y;
	SetClientSize(wxSize(client_size.x, pos.y + 10) );
}


/**************************************/
void WinEDA_DrillFrame::SetParams(void)
/**************************************/
{
	Unit_Drill_is_Inch = (m_Choice_Unit->GetSelection() == 0) ? FALSE : TRUE;
	g_DesignSettings.m_ViaDrill = m_ViaDrillCtrl->GetValue();
	Unit_Drill_is_Inch = m_Choice_Unit->GetSelection();
	g_HPGL_Pen_Speed = m_PenSpeed->GetValue();
	g_HPGL_Pen_Num = m_PenNum->GetValue();
	if ( m_Choice_Drill_Offset->GetSelection() == 0)
		File_Drill_Offset = wxPoint(0,0);
	else
		File_Drill_Offset = m_Parent->m_Auxiliary_Axe_Position;
}


/*****************************************************************/
void WinEDA_PcbFrame::InstallDrillFrame(wxCommandEvent& event)
/*****************************************************************/
{
WinEDA_DrillFrame * frame = new WinEDA_DrillFrame(this, wxPoint(-1,-1));

	frame->ShowModal(); frame->Destroy();
}


/****************************************************************/
void  WinEDA_DrillFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
/****************************************************************/
{
    Close(true);    // true is to force the frame to close
}


/*************************************************************/
void WinEDA_DrillFrame::GenDrillFile(wxCommandEvent& event)
/*************************************************************/
{
int ii;
wxString FullFileName;
char line[1024];

	m_Parent->MsgPanel->EraseMsgBox();

	/* Calcul des echelles de conversion */
	conv_unit = 0.0001; /* unites = INCHES */
	if( ! Unit_Drill_is_Inch) conv_unit = 0.000254; /* unites = mm */

	/* Init nom fichier */
	FullFileName = m_Parent->m_CurrentScreen->m_FileName;
	ChangeFileNameExt(FullFileName,".drl") ;

	FullFileName = EDA_FileSelector(_("Drill file"),
					"",					/* Chemin par defaut */
					FullFileName,		/* nom fichier par defaut */
					".cnf",				/* extension par defaut */
					"*.drl",				/* Masque d'affichage */
					this,
					wxSAVE,
					TRUE
					);
	if ( FullFileName != "" )
	{
		dest = fopen(FullFileName.GetData(),"w");
		if (dest == 0)
		{
			sprintf(line,_("Unable to create file <%s>"),FullFileName.GetData());
			DisplayError(this, line);
			EndModal(0);
			return ;
		}

		/* Init : */
		Affiche_1_Parametre(m_Parent, 0,_("File"),FullFileName,BLUE) ;

		Init_Drill();

		ii = Gen_Liste_Forets( (FORET *) adr_lowmem,TRUE);
		sprintf(line,"%d",ii);
		Affiche_1_Parametre(m_Parent, 30,_("Tools"),line,BROWN);

		ii = Gen_Liste_Trous_EXCELLON( (FORET *) adr_lowmem);
		sprintf(line,"%d",ii);
		Affiche_1_Parametre(m_Parent, 45,_("Drill"),line,GREEN);

		Fin_Drill();
	}

	switch ( m_Choice_Drill_Plan->GetSelection() )
	{
		case 0: break;
		case 1:
			GenDrillPlan(PLOT_FORMAT_HPGL);
			break;
		case 2:
			GenDrillPlan(PLOT_FORMAT_POST);
			break;
	}

	EndModal(0);
}


/************************************************************/
int WinEDA_DrillFrame::Gen_Liste_Trous_EXCELLON( FORET * buffer)
/************************************************************/
/* Create the drill file in EXECELLON format
	Return hole count
	buffer: Drill tools list
*/
{
FORET * foret;
TRACK * pt_piste;
D_PAD * pt_pad;
MODULE * Module;
int ii, diam, nb_trous;
int x0, y0;
char line[1024];

	/* Generation des trous des pastilles : */
	nb_trous = 0;

	/* Examen de la liste des forets  */
	for(ii = 0, foret = buffer; ii < Nb_Forets; ii++, foret++)
	{
		sprintf(line,"T%d\n",ii+1); fputs(line,dest);

		/* Read the via list */
		{
			pt_piste = m_Parent->m_Pcb->m_Track;
			for( ; pt_piste != NULL; pt_piste = (TRACK*) pt_piste->Pnext)
			{
				if(pt_piste->m_StructType != TYPEVIA) continue;
				if ( pt_piste->m_Drill == 0 ) continue; 
				int via_drill = ( pt_piste->m_Drill < 0 ) ? g_DesignSettings.m_ViaDrill : pt_piste->m_Drill;
				if ( foret->diametre != via_drill )
					continue;

				x0 = pt_piste->m_Start.x - File_Drill_Offset.x;
				y0 = pt_piste->m_Start.y - File_Drill_Offset.y;

				if(Unit_Drill_is_Inch)
				{
					sprintf(line,"X%.3fY%.3f\n",
						float(x0) * conv_unit, float(y0) * conv_unit);
				}

				else
				{
					sprintf(line,"X%dY%d\n",
						(int)(float(x0) * conv_unit * 10000.0),
						(int)(float(y0) * conv_unit * 10000.0) );
				}

				// si les flottants sont ecrits avec , au lieu de .
				// conversion , -> . necessaire !
				to_point( line );
				fputs(line,dest);
				nb_trous++;
			}
		}
		/* Examen de la liste des pads : */
		Module = m_Parent->m_Pcb->m_Modules;
		for( ; Module != NULL; Module = (MODULE*)Module->Pnext )
		{
			/* Examen  des pastilles */
			pt_pad = (D_PAD*) Module->m_Pads;
			for ( ; pt_pad != NULL; pt_pad = (D_PAD*)pt_pad->Pnext )
			{
				diam = pt_pad->m_Drill;
				if(diam != foret->diametre) continue;

				/* calcul de la position des trous: */
				x0 = pt_pad->m_Pos.x - File_Drill_Offset.x;
				y0 = pt_pad->m_Pos.y - File_Drill_Offset.y;

				if(Unit_Drill_is_Inch)
					sprintf(line,"X%.3fY%.3f\n",
						float(x0) * conv_unit, float(y0) * conv_unit);

				else
						sprintf(line,"X%dY%d\n",
							(int)(float(x0) * conv_unit * 10000.0),
							(int)(float(y0) * conv_unit * 10000.0) );

				// Si les flottants sont ecrits avec , au lieu de .
				// conversion , -> . necessaire !
				to_point( line );
				fputs(line,dest);
				nb_trous++;
			} /* Fin examen 1 module */
		} /* Fin 1 passe de foret */
	} /* fin analyse des trous de modules */

	return(nb_trous);
}

/***************************************************/
FORET * GetOrAddForet( FORET * buffer, int diameter)
/***************************************************/
/* Search the drill tool for the diameter "diameter"
	Create a new one if not found
*/
{
int ii;
FORET * foret;
	
	if(diameter == 0) return NULL;

	/* Search for an existing tool: */
	for(ii = 0, foret = buffer ; ii < Nb_Forets; ii++, foret++)
	{
		if(foret->diametre == diameter) /* found */
			return foret;
	}
	
	/* No tool found, we must create a new one */
	Nb_Forets ++;
	foret->nb = 0;
	foret->diametre = diameter;
	return foret;
}

/* fonction de service pour le tri par diametre des forets */
int tri_par_diametre(void * foret1, void * foret2)
{
	return( ((FORET*)foret1)->diametre - ((FORET*)foret2)->diametre);
}

/*********************************************************************/
int WinEDA_DrillFrame::Gen_Liste_Forets( FORET * buffer, bool print_header)
/**********************************************************************/
/* Etablit la liste des forets de percage, dans l'ordre croissant des
	diametres
		Retourne:
		Nombre de Forets

		Mise a jour: Nb_Forets = nombre de forets differents
		Genere une liste de Nb_Forets structures FORET a partir de buffer
*/
{
FORET * foret;
D_PAD * pt_pad;
MODULE * Module;
int ii, diam;
char line[1024];

	Nb_Forets = 0; foret = buffer;

	/* Create the via tools */
	TRACK * pt_piste = m_Parent->m_Pcb->m_Track;
	for( ; pt_piste != NULL; pt_piste = (TRACK*) pt_piste->Pnext )
	{
		if(pt_piste->m_StructType != TYPEVIA) continue;
		if ( pt_piste->m_Drill == 0 ) continue;
		int via_drill = g_DesignSettings.m_ViaDrill;
		if ( pt_piste->m_Drill > 0 )	// Drill value is not the default value
			via_drill = pt_piste->m_Drill;
		foret = GetOrAddForet( buffer, via_drill);
		if ( foret ) foret->nb++;
	}

	/* Create the pad tools : */
	Module = m_Parent->m_Pcb->m_Modules;
	for( ; Module != NULL; Module = (MODULE*)Module->Pnext )
	{
		/* Examen  des pastilles */
		pt_pad = (D_PAD*) Module->m_Pads;
		for ( ; pt_pad != NULL; pt_pad = (D_PAD*)pt_pad->Pnext )
		{
			diam = pt_pad->m_Drill;
			if(diam == 0) continue;

			foret = GetOrAddForet( buffer, diam);
			if ( foret ) foret->nb++;
		} /* Fin examen 1 module */
	}

	/* tri des forets par ordre de diametre croissant */
	qsort(buffer,Nb_Forets,sizeof(FORET),
				(int(*)(const void *, const void * ))tri_par_diametre);

	/* Generation de la section liste des outils */
	if (print_header)
	{
		for(ii = 0, foret = (FORET*) buffer ; ii < Nb_Forets; ii++, foret++)
		{
			sprintf(line,"T%dC%.3f\n", ii+1,
						float(foret->diametre) * conv_unit);
			to_point( line );
			fputs(line,dest);
		}
		fputs("%\nM47\nG05\nM72\n",dest);
	}

	return(Nb_Forets);
}


/*************************/
void Init_Drill(void)
/*************************/
/* Genere l'entete du fichier DRILL */
{
char Line[256];

	DateAndTime(Line);
	fprintf(dest,"M47 DRILL file {%s} date %s\n",Main_Title.GetData(),Line);

	fputs("M48\nR,T\nVER,1\nFMAT,2\n",dest);

	if(Unit_Drill_is_Inch) fputs("INCH,TZ\n",dest);	// Si unites en INCHES
	else fputs("METRIC,TZ\n",dest);	// Si unites en mm

	fputs("TCST,OFF\nICI,OFF\nATC,ON\n",dest);
}

/************************/
void Fin_Drill(void)
/************************/
{
	fputs("T0\nM30\n",dest) ; fclose(dest) ;
}


/***********************************************/
void WinEDA_DrillFrame::GenDrillPlan(int format)
/***********************************************/
/* Genere le plan de percage (Drill map) format HPGL ou POSTSCRIPT
*/
{
int ii, x, y;
int plotX, plotY, TextWidth, TextLen;
FORET * foret;
int intervalle = 0, CharSize = 0;
EDA_BaseStruct * PtStruct;
int old_g_PlotOrient = g_PlotOrient;
wxString FullFileName, Mask("*"), Ext;
char line[1024];
int dX, dY;
wxPoint BoardCentre;
int marge = g_PlotMarge * U_PCB;
wxSize SheetSize;
float fTextScale = 1.0;
double scale_x = 1.0, scale_y = 1.0;
W_PLOT * SheetPS = NULL;

	/* Init extension */
	switch( format )
		{
		case PLOT_FORMAT_HPGL:
			Ext = "-drl.plt";
			break;

		case PLOT_FORMAT_POST:
			Ext = "-drl.ps";
			break;

		default:
			DisplayError (this, "WinEDA_DrillFrame::GenDrillPlan() error");
			return;
		}

	/* Init nom fichier */
	FullFileName = m_Parent->m_CurrentScreen->m_FileName;
	ChangeFileNameExt(FullFileName,Ext) ;
	Mask += Ext;

	FullFileName = EDA_FileSelector(_("Drill Map file"),
					"",					/* Chemin par defaut */
					FullFileName,		/* nom fichier par defaut */
					Ext,				/* extension par defaut */
					Mask,				/* Masque d'affichage */
					this,
					wxSAVE,
					TRUE
					);
	if ( FullFileName == "") return;

	g_PlotOrient = 0;
	/* calcul des dimensions et centre du PCB */
	m_Parent->m_Pcb->ComputeBoundaryBox();

	dX = m_Parent->m_Pcb->m_BoundaryBox.GetWidth();
	dY = m_Parent->m_Pcb->m_BoundaryBox.GetHeight();
	BoardCentre = m_Parent->m_Pcb->m_BoundaryBox.Centre();

	// Calcul de l'echelle du dessin du PCB,
	// Echelle 1 en HPGL, dessin sur feuille A4 en PS, + texte description des symboles
	switch( format )
	{
		case PLOT_FORMAT_HPGL: /* Calcul des echelles de conversion format HPGL */
			Scale_X = Scale_Y = 1.0;
			scale_x = Scale_X * SCALE_HPGL;
			scale_y = Scale_Y * SCALE_HPGL;
			fTextScale = SCALE_HPGL;
			SheetSize = m_Parent->m_CurrentScreen->m_CurrentSheet->m_Size;
			SheetSize.x *= U_PCB;
			SheetSize.y *= U_PCB;
			g_PlotOffset.x = 0;
			g_PlotOffset.y = (int)(SheetSize.y * scale_y);
			break;

		case PLOT_FORMAT_POST:
		{
			// calcul en unites internes des dimensions de la feuille ( connues en 1/1000 pouce )
			SheetPS = &g_Sheet_A4;
			SheetSize.x = SheetPS->m_Size.x * U_PCB;
			SheetSize.y = SheetPS->m_Size.y * U_PCB;
			float Xscale = (float) (SheetSize.x - (marge*2)) / dX;
			float Yscale = (float) (SheetSize.y * 0.6 - (marge*2)) / dY;

			scale_x = scale_y = min(Xscale, Yscale);

			g_PlotOffset.x = - (SheetSize.x/2) +
					(int)(BoardCentre.x * scale_x) + g_PlotMarge*U_PCB;
			g_PlotOffset.y = SheetSize.y/2 +
					(int)(BoardCentre.y * scale_y) + g_PlotMarge*U_PCB;
			g_PlotOffset.y += SheetSize.y / 8 ;	/* decalage pour legende */
			break;
		}
	}

	dest = fopen(FullFileName.GetData(),"wt");
	if (dest == 0)
	{
		sprintf(line,_("Unable to create file <%s>"),FullFileName.GetData()) ;
		DisplayError(this, line); return ;
	}

	/* Init : */
	m_Parent->MsgPanel->EraseMsgBox();
	Affiche_1_Parametre(m_Parent, 0,_("File"), FullFileName, BLUE);

	/* Calcul de la liste des diametres de percage (liste des forets) */
	ii = Gen_Liste_Forets( (FORET *) adr_lowmem, FALSE);
	sprintf(line,"%d",ii);
	Affiche_1_Parametre(m_Parent, 48, _("Tools"), line, BROWN);


	switch( format )
	{
		case PLOT_FORMAT_HPGL:
			InitPlotParametresHPGL(g_PlotOffset, scale_x, scale_y);
			PrintHeaderHPGL(dest, g_HPGL_Pen_Speed,g_HPGL_Pen_Num);
			break;

		case PLOT_FORMAT_POST:
			{
			int BBox[4];
			BBox[0] = BBox[1] = g_PlotMarge;
			BBox[2] = SheetPS->m_Size.x - g_PlotMarge;
			BBox[3] = SheetPS->m_Size.y - g_PlotMarge;
			InitPlotParametresPS(g_PlotOffset, SheetPS,
				(double) 1.0 / PCB_INTERNAL_UNIT, (double) 1.0 / PCB_INTERNAL_UNIT);
			SetDefaultLineWidthPS(10);	// Set line with to 10/1000 inch
			PrintHeaderPS(dest, "PCBNEW-PS", FullFileName, BBox);
			InitPlotParametresPS(g_PlotOffset, SheetPS, scale_x, scale_y);
			}
			break;

		default:
			break;
	}

	/* Trace du contour ( couche EDGE ) */
	PtStruct = m_Parent->m_Pcb->m_Drawings;
	for( ; PtStruct != NULL; PtStruct = PtStruct->Pnext )
		{
		switch( PtStruct->m_StructType )
			{
			case TYPEDRAWSEGMENT:
				PlotDrawSegment( (DRAWSEGMENT*) PtStruct, format, EDGE_LAYER );
				break;

			case TYPETEXTE:
				PlotTextePcb((TEXTE_PCB*) PtStruct, format, EDGE_LAYER);
				break;

			case TYPECOTATION:
				PlotCotation((COTATION*) PtStruct, format, EDGE_LAYER);
				break;

			case TYPEMIRE:
				PlotMirePcb((MIREPCB*) PtStruct, format, EDGE_LAYER);
				break;

			default:
				DisplayError(this,"WinEDA_DrillFrame::GenDrillPlan() : Unexpected Draw Type");
				break;
			}
		}

	TextWidth = (int)(50 * scale_x);	// Set Drill Symbols width

	if( format == PLOT_FORMAT_POST )
	{
		sprintf( line, "%d setlinewidth\n", TextWidth );
		fputs(line,dest);
	}

	ii = Plot_Drill_PcbMap( (FORET *) adr_lowmem, format);
	sprintf(line,"%d",ii);
	Affiche_1_Parametre(m_Parent, 64, _("Drill"),line,GREEN);

	/* Impression de la liste des symboles utilises */
	CharSize = 600;  /* hauteur des textes en 1/10000 mils */
	float CharScale = 1.0;
	TextWidth = (int)(50 * CharScale);	// Set text width
	intervalle = (int)(CharSize * CharScale) + TextWidth;

	switch( format)
	{
		case PLOT_FORMAT_HPGL:
		{
			/* generation des dim: commande SI x,y; x et y = dim en cm */
			char csize[256];
			sprintf(csize, "%2.3f", (float)CharSize * CharScale * 0.000254);
			to_point(csize);
			sprintf(line,"SI %s, %s;\n", csize, csize);
			break;
		}

		case PLOT_FORMAT_POST:
			/* Reglage de l'epaisseur de traits des textes */
			sprintf( line,"%d setlinewidth\n", TextWidth );
			break;

		default: *line = 0;
			break;
	}
	fputs(line,dest);

	switch( format )
	{
		default:
		case PLOT_FORMAT_POST:
			g_PlotOffset.x = 0;
			g_PlotOffset.y = 0;
			InitPlotParametresPS(g_PlotOffset, SheetPS, scale_x, scale_x);
			break;

		case PLOT_FORMAT_HPGL:
			InitPlotParametresHPGL(g_PlotOffset, scale_x, scale_x);
			break;
	}

	/* Trace des informations */

	/* Trace de "Infos" */
	plotX = marge + 1000;
	plotY = CharSize + 1000;
	x = plotX ; y = plotY;
	x = +g_PlotOffset.x + (int)(x * fTextScale);
	y = g_PlotOffset.y - (int)(y * fTextScale);
	switch( format )
	{
		case PLOT_FORMAT_HPGL:
			sprintf(line,"PU %d, %d; LBInfos\03;\n",
						x + (int)(intervalle * CharScale * fTextScale),
						y - (int)(CharSize / 2 * CharScale * fTextScale) );
			fputs(line,dest);
			break;

		case PLOT_FORMAT_POST:
			strcpy(line,"Infos");
			TextLen = strlen(line);
			Plot_1_texte(format, line, TextLen, 0, TextWidth,
					x, y,
					(int) (CharSize * CharScale), (int) (CharSize * CharScale),
					FALSE);
			break;
	}

	plotY += (int)( intervalle * 1.2) + 200;

	for(ii = 0, foret = (FORET*)adr_lowmem ; ii < Nb_Forets; ii++, foret++)
	{
		int plot_diam;
		if ( foret->nb == 0 ) continue;

		plot_diam = (int)(foret->diametre * scale_x);
		x = plotX; y = plotY;
		x = -g_PlotOffset.x + (int)(x * fTextScale);
		y = g_PlotOffset.y - (int)(y * fTextScale);
		PlotDrillSymbol(x,y,plot_diam,ii, format);

		intervalle = (int)(CharSize * CharScale) + TextWidth;
		intervalle = (int)( intervalle * 1.2);

		if ( intervalle < (plot_diam + 200 + TextWidth) ) intervalle = plot_diam + 200 + TextWidth;

		int rayon = plot_diam/2;
		x = plotX + rayon + (int)(CharSize * CharScale); y = plotY;
		x = -g_PlotOffset.x + (int)(x * fTextScale); y = g_PlotOffset.y - (int)(y * fTextScale);

		/* Trace de la legende associee */
		switch( format )
		{
			case PLOT_FORMAT_HPGL:
				sprintf(line,"PU %d, %d; LB%2.2fmm / %2.3f\" (%d trous)\03;\n",
						x + (int)(intervalle * CharScale * fTextScale),
						y - (int)(CharSize / 2 * CharScale * fTextScale),
						float(foret->diametre) * 0.00254,
						float(foret->diametre) * 0.0001,
						foret->nb );
				fputs(line,dest);
				break;

			case PLOT_FORMAT_POST:
				sprintf(line,"%2.2fmm / %2.3f\" (%d trous)",
						float(foret->diametre) * 0.00254,
						float(foret->diametre) * 0.0001,
						foret->nb );
				TextLen = strlen(line);
				Plot_1_texte(format, line, TextLen, 0, TextWidth,
						x, y,
						(int) (CharSize * CharScale),
						(int) (CharSize * CharScale),
						FALSE);
				break;
		}

		plotY += intervalle;
	}

	switch( format )
	{
		case PLOT_FORMAT_HPGL:
			CloseFileHPGL(dest);
			break;

		case PLOT_FORMAT_POST:
			CloseFilePS(dest);
			break;
	}
	g_PlotOrient = old_g_PlotOrient;
}


/********************************************************************/
int WinEDA_DrillFrame::Plot_Drill_PcbMap( FORET * buffer, int format)
/*********************************************************************/
/* Trace la liste des trous a percer en format HPGL ou POSTSCRIPT
		Retourne:
			Nombre de trous
		Utilise:
			liste des forets pointee par buffer
*/
{
FORET * foret;
TRACK * pt_piste;
D_PAD * pt_pad;
MODULE * Module;
int ii , diam, nb_trous;
wxPoint pos;
wxSize size;

	/* Generation des trous des pastilles : */
	nb_trous = 0;

	/* Examen de la liste des vias  */
	for(ii = 0, foret = (FORET*)buffer ; ii < Nb_Forets; ii++, foret++)
	{
		/* Generation de la liste des vias */
		{
			pt_piste = m_Parent->m_Pcb->m_Track;
			for( ; pt_piste != NULL; pt_piste = (TRACK*) pt_piste->Pnext)
			{
				if(pt_piste->m_StructType != TYPEVIA) continue;
				if(pt_piste->m_Drill == 0) continue;
				
				int via_drill = g_DesignSettings.m_ViaDrill;
				if (pt_piste->m_Drill >= 0) via_drill = pt_piste->m_Drill;
				pos = pt_piste->m_Start;
				PlotDrillSymbol(pos.x,pos.y, via_drill,ii, format);

				nb_trous++;
			}
		}
		/* Examen de la liste des pads : */
		Module = m_Parent->m_Pcb->m_Modules;
		for( ; Module != NULL; Module = (MODULE*)Module->Pnext )
		{
			/* Examen  des pastilles */
			pt_pad = (D_PAD*) Module->m_Pads;
			for ( ; pt_pad != NULL; pt_pad = (D_PAD*)pt_pad->Pnext )
			{
				diam = pt_pad->m_Drill;
				if(diam != foret->diametre) continue;

				/* calcul de la position des trous: */
				pos = pt_pad->m_Pos;
				PlotDrillSymbol(pos.x,pos.y, diam,ii, format);

				nb_trous++;
			} /* Fin examen 1 module */
		} /* Fin 1 passe de foret */
	} /* fin analyse des trous de modules */

	return(nb_trous);
}


/*************************************************************************/
void PlotDrillSymbol(int x0,int y0,int diametre,int num_forme, int format)
/*************************************************************************/
/* Trace un motif de numero de forme num_forme, aux coord x0, y0.
	x0, y0 = coordonnees tables
	diametre = diametre (coord table) du trou
	num_forme = index ( permet de gererer des formes caract )
*/
{
int rayon = diametre / 2;
void (*FctPlume)(wxPoint pos, int state);

	FctPlume = Move_Plume_HPGL;
	if(format == PLOT_FORMAT_POST) FctPlume = LineTo_PS;

	switch( num_forme)
		{
		case 0: /* vias : forme en X */
			FctPlume( wxPoint(x0 - rayon, y0 - rayon), 'U');
			FctPlume( wxPoint(x0 + rayon, y0 + rayon), 'D');
			FctPlume( wxPoint(x0 + rayon, y0 - rayon), 'U');
			FctPlume( wxPoint(x0 - rayon, y0 + rayon), 'D');
			break;

		case 1: /* Cercle */
			if( format == PLOT_FORMAT_HPGL )
				trace_1_pastille_RONDE_HPGL(wxPoint(x0, y0), diametre, FILAIRE);
			if( format == PLOT_FORMAT_POST )
				trace_1_pastille_RONDE_POST(wxPoint(x0, y0), diametre, FILAIRE);
			break;

		case 2: /* forme en + */
			FctPlume( wxPoint(x0 , y0 - rayon),'U');
			FctPlume( wxPoint(x0 , y0 + rayon),'D');
			FctPlume( wxPoint(x0 + rayon, y0), 'U');
			FctPlume( wxPoint(x0 - rayon, y0), 'D');
			break;

		case 3: /* forme en X cercle */
			FctPlume( wxPoint(x0 - rayon, y0 - rayon),'U');
			FctPlume( wxPoint(x0 + rayon, y0 + rayon),'D');
			FctPlume( wxPoint(x0 + rayon, y0 - rayon),'U');
			FctPlume( wxPoint(x0 - rayon, y0 + rayon),'D');
			if( format == PLOT_FORMAT_HPGL )
				trace_1_pastille_RONDE_HPGL(wxPoint(x0, y0), diametre, FILAIRE);
			if( format == PLOT_FORMAT_POST )
				trace_1_pastille_RONDE_POST(wxPoint(x0, y0), diametre, FILAIRE);
			break;

		case 4: /* forme en cercle barre de - */
			FctPlume( wxPoint(x0 - rayon, y0), 'U');
			FctPlume( wxPoint(x0 + rayon, y0), 'D');
			if( format == PLOT_FORMAT_HPGL )
				trace_1_pastille_RONDE_HPGL(wxPoint(x0, y0), diametre, FILAIRE);
			if( format == PLOT_FORMAT_POST )
				trace_1_pastille_RONDE_POST(wxPoint(x0, y0), diametre, FILAIRE);
			break;

		case 5: /* forme en cercle barre de | */
			FctPlume( wxPoint(x0, y0 - rayon),'U');
			FctPlume( wxPoint(x0, y0 + rayon),'D');
			if( format == PLOT_FORMAT_HPGL )
				trace_1_pastille_RONDE_HPGL(wxPoint(x0, y0), diametre, FILAIRE);
			if( format == PLOT_FORMAT_POST )
				trace_1_pastille_RONDE_POST(wxPoint(x0, y0), diametre, FILAIRE);
			break;

		case 6: /* forme en carre */
			if( format == PLOT_FORMAT_HPGL )
				trace_1_pad_TRAPEZE_HPGL(wxPoint(x0,y0),wxSize(rayon, rayon),wxSize(0,0), 0, FILAIRE) ;
			if( format == PLOT_FORMAT_POST )
				trace_1_pad_TRAPEZE_POST(wxPoint(x0,y0),wxSize(rayon, rayon),wxSize(0,0), 0, FILAIRE) ;
			break;

		case 7: /* forme en losange */
			if( format == PLOT_FORMAT_HPGL )
				trace_1_pad_TRAPEZE_HPGL(wxPoint(x0,y0),wxSize(rayon, rayon),wxSize(0,0), 450, FILAIRE) ;
			if( format == PLOT_FORMAT_POST )
				trace_1_pad_TRAPEZE_POST(wxPoint(x0,y0),wxSize(rayon, rayon),wxSize(0,0), 450, FILAIRE) ;
			break;

		case 8: /* forme en carre barre par un X*/
			FctPlume( wxPoint(x0 - rayon, y0 - rayon),'U');
			FctPlume( wxPoint(x0 + rayon, y0 + rayon),'D');
			FctPlume( wxPoint(x0 + rayon, y0 - rayon),'U');
			FctPlume( wxPoint(x0 - rayon, y0 + rayon),'D');
			if( format == PLOT_FORMAT_HPGL )
				trace_1_pad_TRAPEZE_HPGL(wxPoint(x0,y0),wxSize(rayon, rayon),wxSize(0,0), 0, FILAIRE) ;
			if( format == PLOT_FORMAT_POST )
				trace_1_pad_TRAPEZE_POST(wxPoint(x0,y0),wxSize(rayon, rayon),wxSize(0,0), 0, FILAIRE) ;
			break;

		case 9: /* forme en losange barre par un +*/
			FctPlume( wxPoint(x0, y0 - rayon),'U');
			FctPlume( wxPoint(x0, y0 + rayon),'D');
			FctPlume( wxPoint(x0 + rayon, y0), 'U');
			FctPlume( wxPoint(x0 - rayon, y0), 'D');
			if( format == PLOT_FORMAT_HPGL )
				trace_1_pad_TRAPEZE_HPGL(wxPoint(x0,y0),wxSize(rayon, rayon),wxSize(0,0), 450, FILAIRE);
			if( format == PLOT_FORMAT_POST )
				trace_1_pad_TRAPEZE_POST(wxPoint(x0,y0),wxSize(rayon, rayon),wxSize(0,0), 450, FILAIRE);
			break;

		case 10: /* forme en carre barre par un '/' */
			FctPlume( wxPoint(x0 - rayon, y0 - rayon),'U');
			FctPlume( wxPoint(x0 + rayon, y0 + rayon),'D');
			if( format == PLOT_FORMAT_HPGL )
				trace_1_pad_TRAPEZE_HPGL(wxPoint(x0,y0),wxSize(rayon, rayon),wxSize(0,0), 0, FILAIRE) ;
			if( format == PLOT_FORMAT_POST )
				trace_1_pad_TRAPEZE_POST(wxPoint(x0,y0),wxSize(rayon, rayon),wxSize(0,0), 0, FILAIRE) ;
			break;

		case 11: /* forme en losange barre par un |*/
			FctPlume( wxPoint(x0, y0 - rayon), 'U');
			FctPlume( wxPoint(x0, y0 + rayon), 'D');
			if( format == PLOT_FORMAT_HPGL )
				trace_1_pad_TRAPEZE_HPGL(wxPoint(x0,y0),wxSize(rayon, rayon),wxSize(0,0), 450, FILAIRE) ;
			if( format == PLOT_FORMAT_POST )
				trace_1_pad_TRAPEZE_POST(wxPoint(x0,y0),wxSize(rayon, rayon),wxSize(0,0), 450, FILAIRE) ;
			break;

		case 12: /* forme en losange barre par un -*/
			FctPlume( wxPoint(x0 - rayon, y0), 'U');
			FctPlume( wxPoint(x0 + rayon, y0), 'D');
			if( format == PLOT_FORMAT_HPGL )
				trace_1_pad_TRAPEZE_HPGL(wxPoint(x0,y0),wxSize(rayon, rayon),wxSize(0,0), 450, FILAIRE) ;
			if( format == PLOT_FORMAT_POST )
				trace_1_pad_TRAPEZE_POST(wxPoint(x0,y0),wxSize(rayon, rayon),wxSize(0,0), 450, FILAIRE) ;
			break;

		default:
			DisplayError(NULL, " Trop de diametres differents ( max 13)", 10);
			if( format == PLOT_FORMAT_HPGL )
				trace_1_pastille_RONDE_HPGL(wxPoint(x0, y0), diametre, FILAIRE);
			if( format == PLOT_FORMAT_POST )
				trace_1_pastille_RONDE_POST(wxPoint(x0, y0), diametre, FILAIRE);
			break;
		}
	if( format == PLOT_FORMAT_HPGL) Plume_HPGL('U');
}


