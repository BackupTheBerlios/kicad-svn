	/**********************************************************/
	/** pcbcfg.h : configuration: definition des structures  **/
	/**********************************************************/

#define GROUP "/pcbnew"
#define GROUPLIB "/pcbnew/libraries"
#define GROUPCOMMON "/common"

#define INSETUP TRUE


static int Pcbdiv_grille;	/* memorisation temporaire */

/* Liste des parametres */

static  PARAM_CFG_WXSTRING UserLibDirBufCfg
(
	"LibDir",		  		/* identification */
	&g_UserLibDirBuffer, 	/* Adresse du parametre */
	GROUPLIB
);

static  PARAM_CFG_LIBNAME_LIST LibNameBufCfg
(
	"LibName",			/* identification */
	&g_LibName_List,		/* Adresse du parametre */
	GROUPLIB
);

static  PARAM_CFG_INT PadDrillCfg
(
	"PadDril",		/* identification */
	&g_Pad_Master.m_Drill,		/* Adresse du parametre */
	320,			/* Valeur par defaut */
	0, 0x7FFF		/* Valeurs extremes */
);

static  PARAM_CFG_INT PadDimHCfg	//Pad Diameter / H Size
(
	"PadDimH",		/* identification */
	&g_Pad_Master.m_Size.x,/* Adresse du parametre */
	550,			/* Valeur par defaut */
	0, 0x7FFF		/* Valeurs extremes */
);

static  PARAM_CFG_INT PadDimVCfg
(
	"PadDimV",			/* identification */
	&g_Pad_Master.m_Size.y,	/* Adresse du parametre */
	550,				/* Valeur par defaut */
	0, 0x7FFF			/* Valeurs extremes */
);


static  PARAM_CFG_INT PadFormeCfg
(
	"PadForm",				/* identification */
	&g_Pad_Master.m_PadShape,	/* Adresse du parametre */
	CIRCLE,					/* Valeur par defaut */
	0, 0x7F					/* Valeurs extremes */
);

static  PARAM_CFG_INT PadMasqueLayerCfg
(
	"PadMask",					/* identification */
	&g_Pad_Master.m_Masque_Layer,	/* Adresse du parametre */
	0x0000FFFF					/* Valeur par defaut */
);

static  PARAM_CFG_INT ViaDiametreCfg
(
	"ViaDiam",			/* identification */
	&g_DesignSettings.m_CurrentViaSize,		/* Adresse du parametre */
	450,				/* Valeur par defaut */
	0, 0xFFFF			/* Valeurs extremes */
);

static  PARAM_CFG_INT ViaDrillCfg
(
	"ViaDril",			/* identification */
	&g_DesignSettings.m_ViaDrill,			/* Adresse du parametre */
	250,				/* Valeur par defaut */
	0, 0xFFFF			/* Valeurs extremes */
);
static  PARAM_CFG_INT ViaShowHoleCfg
(
	INSETUP,
	"ViaSHole",						/* identification */
	&DisplayOpt.m_DisplayViaMode,	/* Adresse du parametre */
	VIA_SPECIAL_HOLE_SHOW,			/* Valeur par defaut */
	VIA_HOLE_NOT_SHOW,				/* Valeurs extremes */
	OPT_VIA_HOLE_END-1				/* Valeurs extremes */
);

static  PARAM_CFG_INT TrackClearenceCfg
(
	"Isol",				/* identification */
	&g_DesignSettings.m_TrackClearence,			/* Adresse du parametre */
	120,				/* Valeur par defaut */
	0, 0xFFFF			/* Valeurs extremes */
);

static  PARAM_CFG_INT LayerCountCfg	// Mask Working Layers
(
	"Countlayer",			/* identification */
	&g_DesignSettings.m_CopperLayerCount,			/* Adresse du parametre */
	2,						/* Valeur par defaut */
	1, NB_COPPER_LAYERS		/* Valeurs extremes */
);

static  PARAM_CFG_INT TrackWidthCfg
(
	"Lpiste",			/* identification */
	&g_DesignSettings.m_CurrentTrackWidth,		/* Adresse du parametre */
	170,				/* Valeur par defaut */
	2, 0xFFFF			/* Valeurs extremes */
);

static  PARAM_CFG_INT RouteLayTopCfg	// First current working layer
(
	"RouteTo",			 /* identification */
	&Route_Layer_TOP,	 /* Adresse du parametre */
	15,					/* Valeur par defaut */
	0, 15				/* Valeurs extremes */
);

static  PARAM_CFG_INT RouteLayBotCfg	// second current working layer
(
	"RouteBo",		 	 /* identification */
	&Route_Layer_BOTTOM, /* Adresse du parametre */
	0,					 /* Valeur par defaut */
	0, 15				 /* Valeurs extremes */
);

static  PARAM_CFG_INT TypeViaCfg
(
	"TypeVia",			/* identification */
	&g_DesignSettings.m_CurrentViaType,			/* Adresse du parametre */
	VIA_NORMALE,		/* Valeur par defaut */
	0, 3				/* Valeurs extremes */
);

static  PARAM_CFG_BOOL Segm45Cfg	// Segm Pistes a 0, 45, 90 degres uniquement
(
	"Segm45",		/* identification */
	&Track_45_Only,	/* Adresse du parametre */
	TRUE				/* Valeur par defaut */
);

static  PARAM_CFG_BOOL Raccord45Cfg	// Generation automatique des Raccords a 45 degres
(
	"Racc45",		  /* identification */
	&g_Raccord_45_Auto,  /* Adresse du parametre */
	TRUE				/* Valeur par defaut */
);

static  PARAM_CFG_BOOL UnitCfg	// Units: 0 inch, 1 mm
(
	"Unite",			/* identification */
	&UnitMetric,		/* Adresse du parametre */
	FALSE				/* Valeur par defaut */
);

static  PARAM_CFG_BOOL SegmFillCfg
(
	"SegFill",				/* identification */
	&DisplayOpt.DisplayPcbTrackFill,			/* Adresse du parametre */
	TRUE					/* Valeur par defaut */
);

static  PARAM_CFG_BOOL NewTrackAfficheGardeCfg
(
	"NewAffG",				/* identification */
	&g_ShowIsolDuringCreateTrack,  /* Adresse du parametre */
	TRUE					/* Valeur par defaut */
);

static  PARAM_CFG_BOOL TrackAfficheGardeCfg
(
	"SegAffG",				/* identification */
	&DisplayOpt.DisplayTrackIsol,  /* Adresse du parametre */
	FALSE					/* Valeur par defaut */
);

static  PARAM_CFG_BOOL PadFillCfg
(
	"PadFill",				/* identification */
	&DisplayOpt.DisplayPadFill,	/* Adresse du parametre */
	TRUE					/* Valeur par defaut */
);

static  PARAM_CFG_BOOL PadAfficheGardeCfg
(
	"PadAffG",					/* identification */
	&DisplayOpt.DisplayPadIsol, /* Adresse du parametre */
	TRUE						/* Valeur par defaut */
);

static  PARAM_CFG_BOOL PadShowNumCfg
(
	"PadSNum",					/* identification */
	&DisplayOpt.DisplayPadNum,	/* Adresse du parametre */
	TRUE						/* Valeur par defaut */
);

static  PARAM_CFG_INT AfficheContourModuleCfg	// Module Edges: fill/line/sketch
(
	"ModAffC",				/* identification */
	&DisplayOpt.DisplayModEdge,  /* Adresse du parametre */
	FILAIRE,				/* Valeur par defaut */
	0, 2					/* Valeurs extremes */
);

static  PARAM_CFG_INT AfficheTexteModuleCfg	// Module Texts: fill/line/sketch
(
	"ModAffT",				/* identification */
	&DisplayOpt.DisplayModText,  /* Adresse du parametre */
	FILAIRE,				/* Valeur par defaut */
	0, 2					/* Valeurs extremes */
);

static  PARAM_CFG_INT AfficheTextePcbCfg	// PCB Texts: fill/line/sketch
(
	"PcbAffT",			/* identification */
	&DisplayOpt.DisplayDrawItems, /* Adresse du parametre */
	FILAIRE,			/* Valeur par defaut */
	0, 2				/* Valeurs extremes */
);

static  PARAM_CFG_BOOL SegmPcb45Cfg	// Force 45 degrees for segments
(
	"SgPcb45",			 /* identification */
	&Segments_45_Only,  /* Adresse du parametre */
	TRUE				/* Valeur par defaut */
);

static  PARAM_CFG_INT TextePcbDimVCfg
(
	"TxtPcbV",				/* identification */
	&g_DesignSettings.m_PcbTextSize.y,			/* Adresse du parametre */
	600,					/* Valeur par defaut */
	10, 2000				/* Valeurs extremes */
);

static  PARAM_CFG_INT TextePcbDimHCfg
(
	"TxtPcbH",				/* identification */
	&g_DesignSettings.m_PcbTextSize.x,			/* Adresse du parametre */
	600,					/* Valeur par defaut */
	10, 2000				/* Valeurs extremes */
);

static  PARAM_CFG_INT ScreenPcbGrilleXCfg
(
	"GridPcbX",				/* identification */
	&g_PcbDefaultGrid.x,				/* Adresse du parametre */
	500,					/* Valeur par defaut */
	1,10000					/* Valeurs extremes */
);

static  PARAM_CFG_INT ScreenPcbGrilleYCfg
(
	"GridPcbY",				/* identification */
	&g_PcbDefaultGrid.y,				/* Adresse du parametre */
	500,					/* Valeur par defaut */
	1,10000					/* Valeurs extremes */
);

static  PARAM_CFG_SETCOLOR ColorLayer0Cfg	// CU Layer Color
(
	INSETUP,
	"ColLay0",				/* identification */
	&g_DesignSettings.m_LayerColor[0],		/* Adresse du parametre */
	GREEN					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer1Cfg
(
	INSETUP,
	"ColLay1",				/* identification */
	&g_DesignSettings.m_LayerColor[1],		/* Adresse du parametre */
	BLUE					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer2Cfg
(
	INSETUP,
	"ColLay2",				/* identification */
	&g_DesignSettings.m_LayerColor[2],		/* Adresse du parametre */
	LIGHTGRAY				/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer3Cfg
(
	INSETUP,
	"ColLay3",				/* identification */
	&g_DesignSettings.m_LayerColor[3],		/* Adresse du parametre */
	5						/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer4Cfg
(
	INSETUP,
	"ColLay4",				/* identification */
	&g_DesignSettings.m_LayerColor[4],		/* Adresse du parametre */
	4						/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer5Cfg
(
	INSETUP,
	"ColLay5",				/* identification */
	&g_DesignSettings.m_LayerColor[5],		/* Adresse du parametre */
	5						/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer6Cfg
(
	INSETUP,
	"ColLay6",				/* identification */
	&g_DesignSettings.m_LayerColor[6],		/* Adresse du parametre */
	6						/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer7Cfg
(
	INSETUP,
	"ColLay7",				/* identification */
	&g_DesignSettings.m_LayerColor[7],		/* Adresse du parametre */
	5						/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer8Cfg
(
	INSETUP,
	"ColLay8",				/* identification */
	&g_DesignSettings.m_LayerColor[8],		/* Adresse du parametre */
	7						/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer9Cfg
(
	INSETUP,
	"ColLay9",			/* identification */
	&g_DesignSettings.m_LayerColor[9],	/* Adresse du parametre */
	1					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer10Cfg
(
	INSETUP,
	"ColLayA",			/* identification */
	&g_DesignSettings.m_LayerColor[10],	/* Adresse du parametre */
	2					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer11Cfg
(
	INSETUP,
	"ColLayB",			/* identification */
	&g_DesignSettings.m_LayerColor[11],	/* Adresse du parametre */
	3					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer12Cfg
(
	INSETUP,
	"ColLayC",			/* identification */
	&g_DesignSettings.m_LayerColor[12],	/* Adresse du parametre */
	12					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer13Cfg
(
	INSETUP,
	"ColLayD",			/* identification */
	&g_DesignSettings.m_LayerColor[13],	/* Adresse du parametre */
	13					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer14Cfg
(
	INSETUP,
	"ColLayE",			/* identification */
	&g_DesignSettings.m_LayerColor[14],	/* Adresse du parametre */
	14					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer15Cfg	// CMP Layer Color
(
	INSETUP,
	"ColLayF",			/* identification */
	&g_DesignSettings.m_LayerColor[15],	/* Adresse du parametre */
	RED				/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer16Cfg	// Adhesive CU Layer Color
(
	INSETUP,
	"ColLayG",			/* identification */
	&g_DesignSettings.m_LayerColor[16],	/* Adresse du parametre */
	1					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer17Cfg	// Adhesive CMP Layer Color
(
	INSETUP,
	"ColLayH",			/* identification */
	&g_DesignSettings.m_LayerColor[17],	/* Adresse du parametre */
	5					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer18Cfg	// Solder Mask CU Layer Color
(
	INSETUP,
	"ColLayI",			/* identification */
	&g_DesignSettings.m_LayerColor[18],	/* Adresse du parametre */
	11					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer19Cfg	// Solder Mask CMP Layer Color
(
	INSETUP,
	"ColLayJ",			/* identification */
	&g_DesignSettings.m_LayerColor[19],	/* Adresse du parametre */
	4					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer20Cfg	// Silk Screen CU Layer Color
(
	INSETUP,
	"ColLayK",			/* identification */
	&g_DesignSettings.m_LayerColor[20],	/* Adresse du parametre */
	5					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer21Cfg	// Silk Screen CMP Layer Color
(
	INSETUP,
	"ColLayL",			/* identification */
	&g_DesignSettings.m_LayerColor[21],	/* Adresse du parametre */
	3					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer22Cfg	// Mask CU Layer Color
(
	INSETUP,
	"ColLayM",			/* identification */
	&g_DesignSettings.m_LayerColor[22],	/* Adresse du parametre */
	6					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer23Cfg	// Mask CMP Layer Color
(
	INSETUP,
	"ColLayN",			/* identification */
	&g_DesignSettings.m_LayerColor[23],	/* Adresse du parametre */
	5					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer24Cfg	// DRAW Layer Color
(
	INSETUP,
	"ColLayO",				/* identification */
	&g_DesignSettings.m_LayerColor[24],		/* Adresse du parametre */
	LIGHTGRAY				/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer25Cfg	// Comment Layer Color
(
	INSETUP,
	"ColLayP",			/* identification */
	&g_DesignSettings.m_LayerColor[25],	/* Adresse du parametre */
	1					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer26Cfg	// ECO1 Layer Color
(
	INSETUP,
	"ColLayQ",			/* identification */
	&g_DesignSettings.m_LayerColor[26],	/* Adresse du parametre */
	2					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer27Cfg	//ECO2 Layer Color
(
	INSETUP,
	"ColLayR",				/* identification */
	&g_DesignSettings.m_LayerColor[27],		/* Adresse du parametre */
	14						/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer28Cfg	// EDGES Layer Color
(
	INSETUP,
	"ColLayS",				/* identification */
	&g_DesignSettings.m_LayerColor[28],		/* Adresse du parametre */
	YELLOW					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer29Cfg
(
	INSETUP,
	"ColLayT",			/* identification */
	&g_DesignSettings.m_LayerColor[29],	/* Adresse du parametre */
	13					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer30Cfg
(
	INSETUP,
	"ColLayU",			/* identification */
	&g_DesignSettings.m_LayerColor[30],	/* Adresse du parametre */
	14					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorLayer31Cfg
(
	INSETUP,
	"ColLayV",			/* identification */
	&g_DesignSettings.m_LayerColor[31],	/* Adresse du parametre */
	7					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorTxtModCmpCfg
(
	INSETUP,
	"CTxtMoC",			/* identification */
	&g_ModuleTextCMPColor, /* Adresse du parametre */
	LIGHTGRAY			/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorTxtModCuCfg
(
	INSETUP,
	"CTxtMoS",			/* identification */
	&g_ModuleTextCUColor, /* Adresse du parametre */
	1					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR VisibleTxtModCfg
(
	INSETUP,
	"CTxtVis",			/* identification */
	&g_ModuleTextNOVColor, /* Adresse du parametre */
	DARKGRAY					/* Valeur par defaut */
);

static  PARAM_CFG_INT TexteModDimVCfg
(
	"TxtModV",		/* identification */
	&ModuleTextSize.y,  /* Adresse du parametre */
	500,			  /* Valeur par defaut */
	1, 20000		  /* Valeurs extremes */
);

static  PARAM_CFG_INT TexteModDimHCfg
(
	"TxtModH",				/* identification */
	&ModuleTextSize.x,		/* Adresse du parametre */
	500,					/* Valeur par defaut */
	1, 20000				/* Valeurs extremes */
);

static  PARAM_CFG_INT TexteModWidthCfg
(
	"TxtModW",				/* identification */
	&ModuleTextWidth,		/* Adresse du parametre */
	100,					/* Valeur par defaut */
	1, 10000				/* Valeurs extremes */
);

static  PARAM_CFG_SETCOLOR ColorAncreModCfg
(
	INSETUP,
	"CAncreM",			/* identification */
	&g_AnchorColor, 	/* Adresse du parametre */
	BLUE				/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorPadCuCfg
(
	INSETUP,
	"CoPadCu",				/* identification */
	&g_PadCUColor,	/* Adresse du parametre */
	GREEN					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorPadCmpCfg
(
	INSETUP,
	"CoPadCm",				/* identification */
	&g_PadCMPColor, /* Adresse du parametre */
	RED					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorViaNormCfg
(
	INSETUP,
	"CoViaNo",				/* identification */
	&g_DesignSettings.m_ViaColor[VIA_NORMALE], /* Adresse du parametre */
	LIGHTGRAY				/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorViaborgneCfg
(
	INSETUP,
	"CoViaBo",				/* identification */
	&g_DesignSettings.m_ViaColor[VIA_BORGNE], /* Adresse du parametre */
	CYAN					/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorViaEnterreeCfg	// Buried Via Color
(
	INSETUP,
	"CoViaEn",					/* identification */
	&g_DesignSettings.m_ViaColor[VIA_ENTERREE],	/* Adresse du parametre */
	BROWN						/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorpcbGrilleCfg
(
	INSETUP,
	"CoPcbGr",				/* identification */
	&g_PcbGridColor,			/* Adresse du parametre */
	DARKGRAY				/* Valeur par defaut */
);

static  PARAM_CFG_SETCOLOR ColorCheveluCfg
(
	INSETUP,
	"CoRatsN",				/* identification */
	&g_DesignSettings.m_RatsnestColor,		/* Adresse du parametre */
	WHITE					/* Valeur par defaut */
);

static  PARAM_CFG_INT PlotMargeCfg
(
	"Pltmarg",			/* identification */
	&g_PlotMarge,			/* Adresse du parametre */
	300,					/* Valeur par defaut */
	0,10000				/* Valeurs extremes */
);

static  PARAM_CFG_INT HPGLpenNumCfg
(
	"HPGLnum",			/* identification */
	&g_HPGL_Pen_Num,			/* Adresse du parametre */
	1,					/* Valeur par defaut */
	1, 16				/* Valeurs extremes */
);

static  PARAM_CFG_INT HPGLdiamCfg	// HPGL pen size (mils)
(
	"HPGdiam",			/* identification */
	&g_HPGL_Pen_Diam,			/* Adresse du parametre */
	15,					/* Valeur par defaut */
	0,100			/* Valeurs extremes */
);

static  PARAM_CFG_INT HPGLspeedCfg	//HPGL pen speed (cm/s)
(
	"HPGLSpd",			/* identification */
	&g_HPGL_Pen_Speed,	/* Adresse du parametre */
	20,					/* Valeur par defaut */
	0,1000				/* Valeurs extremes */
);

static  PARAM_CFG_INT HPGLrecouvrementCfg
(
	"HPGLrec",			/* identification */
	&g_HPGL_Pen_Recouvrement,  /* Adresse du parametre */
	2,					/* Valeur par defaut */
	0,0x100			/* Valeurs extremes */
);

static  PARAM_CFG_BOOL HPGLcenterCfg	//HPGL Org Coord ( 0 normal, 1 Centre)
(
	"HPGLorg",				/* identification */
	&HPGL_Org_Centre,		/* Adresse du parametre */
	FALSE					/* Valeur par defaut */
);

static  PARAM_CFG_INT GERBERSpotMiniCfg	//Aperture Mini (mils)
(
	"GERBmin",			/* identification */
	&spot_mini,			/* Adresse du parametre */
	15,					/* Valeur par defaut */
	1,100				/* Valeurs extremes */
);

static  PARAM_CFG_INT VernisEpargneGardeCfg
(
	"VEgarde",			/* identification */
	&g_DesignSettings.m_MaskMargin,		/* Adresse du parametre */
	100,				/* Valeur par defaut */
	0,0xFFFF			/* Valeurs extremes */
);

static  PARAM_CFG_INT DrawSegmLargeurCfg
(
	"DrawLar",			/* identification */
	&g_DesignSettings.m_DrawSegmentWidth,		/* Adresse du parametre */
	120,				/* Valeur par defaut */
	0,0xFFFF			/* Valeurs extremes */
);

static  PARAM_CFG_INT EdgeSegmLargeurCfg
(
	"EdgeLar",			/* identification */
	&g_DesignSettings.m_EdgeSegmentWidth,		/* Adresse du parametre */
	120,				/* Valeur par defaut */
	0,0xFFFF			/* Valeurs extremes */
);

static  PARAM_CFG_INT TexteSegmLargeurCfg
(
	"TxtLar",			/* identification */
	&g_DesignSettings.m_PcbTextWidth,		/* Adresse du parametre */
	120,				/* Valeur par defaut */
	0,0xFFFF			/* Valeurs extremes */
);

static  PARAM_CFG_INT ModuleSegmWidthCfg
(
	"MSegLar",			/* identification */
	&ModuleSegmentWidth,		/* Adresse du parametre */
	120,				/* Valeur par defaut */
	0,0xFFFF			/* Valeurs extremes */
);

static  PARAM_CFG_INT FormatPlotCfg
(
	"ForPlot",			/* identification */
	&format_plot,		/* Adresse du parametre */
	1,					/* Valeur par defaut */
	0,3				/* Valeurs extremes */
);

static  PARAM_CFG_INT WTraitSerigraphiePlotCfg
(
	"WpenSer",			/* identification */
	&g_PlotLine_Width,		/* Adresse du parametre */
	10,				/* Valeur par defaut */
	1,10000			/* Valeurs extremes */
);

static  PARAM_CFG_DOUBLE UserGrilleXCfg
(
	"UserGrX",				/* identification */
	&g_UserGrid.x,			/* Adresse du parametre */
	0.01,						/* Valeur par defaut */
	0.0, 100.0					/* Valeurs extremes */
);

static  PARAM_CFG_DOUBLE UserGrilleYCfg
(
	"UserGrY",				/* identification */
	&g_UserGrid.y,			/* Adresse du parametre */
	0.01,						/* Valeur par defaut */
	0.0, 100.0					/* Valeurs extremes */
);

static  PARAM_CFG_INT UserGrilleUnitCfg
(
	"UserGrU",				/* identification */
	&g_UserGrid_Unit,			/* Adresse du parametre */
	1,						/* Valeur par defaut */
	0, 1					/* Valeurs extremes */
);

static  PARAM_CFG_INT DivGrillePcbCfg
(
	"DivGrPc",				/* identification */
	&Pcbdiv_grille,			/* Adresse du parametre */
	1,						/* Valeur par defaut */
	1,10					/* Valeurs extremes */
);

static  PARAM_CFG_INT TimeOutCfg	//Duree entre Sauvegardes auto en secondes
(
	"TimeOut",				/* identification */
	&g_TimeOut,				/* Adresse du parametre */
	600,					/* Valeur par defaut */
	0,60000					/* Valeurs extremes */
);

static  PARAM_CFG_BOOL DisplPolairCfg
(
	INSETUP,
	"DPolair",				/* identification */
	&DisplayOpt.DisplayPolarCood,	/* Adresse du parametre */
	FALSE						/* Valeur par defaut */
);

static  PARAM_CFG_INT CursorShapeCfg
(
	INSETUP,
	"CuShape",				/* identification */
	&g_CursorShape,	/* Adresse du parametre */
	0,						/* Valeur par defaut */
	0, 1					/* Valeurs extremes */
);

static  PARAM_CFG_INT PrmMaxLinksShowed
(
	"MaxLnkS",				/* identification */
	&g_MaxLinksShowed,		/* Adresse du parametre */
	3,						/* Valeur par defaut */
	0, 15					/* Valeurs extremes */
);

static  PARAM_CFG_BOOL ShowRatsnestCfg
(
	"ShowRat",				/* identification */
	&g_Show_Ratsnest,			/* Adresse du parametre */
	FALSE					/* Valeur par defaut */
);

static  PARAM_CFG_BOOL ShowModuleRatsnestCfg
(
	"ShowMRa",				/* identification */
	&g_Show_Module_Ratsnest,	/* Adresse du parametre */
	TRUE					/* Valeur par defaut */
);

PARAM_CFG_BASE * ParamCfgList[] =
{
	& UserLibDirBufCfg,
	& LibNameBufCfg,
	& PadDrillCfg,
	& PadDimHCfg,
	& PadDimVCfg,
	& PadFormeCfg,
	& PadMasqueLayerCfg,
	& ViaDiametreCfg,
	& ViaDrillCfg,
	& ViaShowHoleCfg,
	& TrackClearenceCfg,
	& LayerCountCfg,
	& TrackWidthCfg,
	& RouteLayTopCfg,
	& RouteLayBotCfg,
	& TypeViaCfg,
	& Segm45Cfg,
	& Raccord45Cfg,
	& UnitCfg,
	& SegmFillCfg,
	& TrackAfficheGardeCfg,
	& NewTrackAfficheGardeCfg,
	& PadFillCfg,
	& PadAfficheGardeCfg,
	& PadShowNumCfg,
	& AfficheContourModuleCfg,
	& AfficheTexteModuleCfg,
	& AfficheTextePcbCfg,
	& SegmPcb45Cfg,
	& TextePcbDimVCfg,
	& TextePcbDimHCfg,
	& ScreenPcbGrilleXCfg,
	& ScreenPcbGrilleYCfg,
	& ColorLayer0Cfg,
	& ColorLayer1Cfg,
	& ColorLayer2Cfg,
	& ColorLayer3Cfg,
	& ColorLayer4Cfg,
	& ColorLayer5Cfg,
	& ColorLayer6Cfg,
	& ColorLayer7Cfg,
	& ColorLayer8Cfg,
	& ColorLayer9Cfg,
	& ColorLayer10Cfg,
	& ColorLayer11Cfg,
	& ColorLayer12Cfg,
	& ColorLayer13Cfg,
	& ColorLayer14Cfg,
	& ColorLayer15Cfg,
	& ColorLayer16Cfg,
	& ColorLayer17Cfg,
	& ColorLayer18Cfg,
	& ColorLayer19Cfg,
	& ColorLayer20Cfg,
	& ColorLayer21Cfg,
	& ColorLayer22Cfg,
	& ColorLayer23Cfg,
	& ColorLayer24Cfg,
	& ColorLayer25Cfg,
	& ColorLayer26Cfg,
	& ColorLayer27Cfg,
	& ColorLayer28Cfg,
	& ColorLayer29Cfg,
	& ColorLayer30Cfg,
	& ColorLayer31Cfg,
	& ColorTxtModCmpCfg,
	& ColorTxtModCuCfg,
	& VisibleTxtModCfg,
	& TexteModDimVCfg,
	& TexteModDimHCfg,
	& TexteModWidthCfg,
	& ColorAncreModCfg,
	& ColorPadCuCfg,
	& ColorPadCmpCfg,
	& ColorViaNormCfg,
	& ColorViaborgneCfg,
	& ColorViaEnterreeCfg,
	& ColorpcbGrilleCfg,
	& ColorCheveluCfg,
	& HPGLpenNumCfg,
	& PlotMargeCfg,
	& HPGLdiamCfg,
	& HPGLspeedCfg,
	& HPGLrecouvrementCfg,
	& HPGLcenterCfg,
	& GERBERSpotMiniCfg,
	& VernisEpargneGardeCfg,
	& DrawSegmLargeurCfg,
	& EdgeSegmLargeurCfg,
	& TexteSegmLargeurCfg,
	& ModuleSegmWidthCfg,
	& FormatPlotCfg,
	& WTraitSerigraphiePlotCfg,
	& UserGrilleXCfg,
	& UserGrilleYCfg,
	& UserGrilleUnitCfg,
	& DivGrillePcbCfg,
	& TimeOutCfg,
	& DisplPolairCfg,
	& CursorShapeCfg,
	& PrmMaxLinksShowed,
	& ShowRatsnestCfg,
	& ShowModuleRatsnestCfg,

	NULL
};
