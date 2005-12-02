	/************************************************************/
	/** eeconfig.h : configuration: definition des structures  **/
	/************************************************************/

#ifndef eda_global
#define eda_global extern
#endif

#define GROUP "/eeschema"
#define GROUPCOMMON "/common"
#define GROUPLIB "libraries"

#include "netlist.h"	/* Definitions generales liees au calcul de netliste */

/* variables importees */
extern int PenMinWidth;

/* Liste des parametres */

#define INSETUP TRUE

//extern PARAM_CFG_INT UserLibDirBufCfg;


static PARAM_CFG_WXSTRING UserLibDirBufCfg
	(
	"LibDir",		  		/* identification */
	&g_UserLibDirBuffer 	/* Adresse du parametre */
	);


static PARAM_CFG_LIBNAME_LIST LibNameBufCfg
(
	"LibName",			/* identification */
	&g_LibName_List,	/* Adresse du parametre */
	GROUPLIB			/* Groupe */
);

static PARAM_CFG_INT NetFormatCfg
(
	"NetFmt",			/* identification */
	&g_NetFormat,			/* Adresse du parametre */
	NET_TYPE_PCBNEW,	/* Valeur par defaut */
	NET_TYPE_NOT_INIT, NET_TYPE_MAX-1	/* Valeurs extremes */
);

static PARAM_CFG_INT UnitCfg
(
	INSETUP,
	"Unite",			/* identification */
	&UnitMetric,	 	/* Adresse du parametre */
	0,					/* Valeur par defaut */
	0, 1				/* Valeurs extremes */
);

static PARAM_CFG_INT CursorShapeCfg
(
	INSETUP,
	"CuShape",			/* identification */
	&g_CursorShape,	/* Adresse du parametre */
	0,					/* Valeur par defaut */
	0, 1				/* Valeurs extremes */
);

static PARAM_CFG_INT ShowGridCfg
(
	INSETUP,
	"ShGrid",			/* identification */
	&g_ShowGrid,	 		/* Adresse du parametre */
	0, 1,				/* Valeurs extremes */
	1					/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR DrawBgColorCfg
(
	INSETUP,
	"BgColor",			/* identification */
	&DrawBgColor,	 	/* Adresse du parametre */
	WHITE				/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerWireCfg
(
	INSETUP,
	"ColWire",				/* identification */
	&g_LayerDescr.LayerColor[LAYER_WIRE],		/* Adresse du parametre */
	GREEN					/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerBusCfg
(
	INSETUP,
	"ColorBus",				/* identification */
	&g_LayerDescr.LayerColor[LAYER_BUS],		/* Adresse du parametre */
	BLUE					/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerJunctionCfg
(
	INSETUP,
	"ColorConn",				/* identification */
	&g_LayerDescr.LayerColor[LAYER_JUNCTION],		/* Adresse du parametre */
	GREEN					/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerLLabelCfg
(
	INSETUP,
	"ColorLlab",				/* identification */
	&g_LayerDescr.LayerColor[LAYER_LOCLABEL],		/* Adresse du parametre */
	BLACK					/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerGLabelCfg
(
	INSETUP,
	"ColorGlab",				/* identification */
	&g_LayerDescr.LayerColor[LAYER_GLOBLABEL],		/* Adresse du parametre */
	BROWN					/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerPinFunCfg
(
	INSETUP,
	"ColorPinF",				/* identification */
	&g_LayerDescr.LayerColor[LAYER_PINFUN],		/* Adresse du parametre */
	MAGENTA				/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerPinNumCfg
(
	INSETUP,
	"ColPinN",				/* identification */
	&g_LayerDescr.LayerColor[LAYER_PINNUM],		/* Adresse du parametre */
	RED					/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerPinNamCfg
(
	INSETUP,
	"ColorPNam",				/* identification */
	&g_LayerDescr.LayerColor[LAYER_PINNAM],		/* Adresse du parametre */
	CYAN					/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerFieldsCfg
(
	INSETUP,
	"ColorField",				/* identification */
	&g_LayerDescr.LayerColor[LAYER_FIELDS],		/* Adresse du parametre */
	MAGENTA				/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerReferenceCfg
(
	INSETUP,
	"ColorRef",			/* identification */
	&g_LayerDescr.LayerColor[LAYER_REFERENCEPART],	/* Adresse du parametre */
	CYAN					/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerValueCfg
(
	INSETUP,
	"ColorValue",			/* identification */
	&g_LayerDescr.LayerColor[LAYER_VALUEPART],	/* Adresse du parametre */
	CYAN					/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerNotesCfg
(
	INSETUP,
	"ColorNote",			/* identification */
	&g_LayerDescr.LayerColor[LAYER_NOTES],	/* Adresse du parametre */
	LIGHTBLUE					/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerBodyCfg
(
	INSETUP,
	"ColorBody",			/* identification */
	&g_LayerDescr.LayerColor[LAYER_DEVICE],	/* Adresse du parametre */
	RED					/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerBodyBackgroundCfg
(
	INSETUP,
	"ColorBodyBg",			/* identification */
	&g_LayerDescr.LayerColor[LAYER_DEVICE_BACKGROUND],	/* Adresse du parametre */
	LIGHTYELLOW					/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerNetNameCfg
(
	INSETUP,
	"ColorNetN",			/* identification */
	&g_LayerDescr.LayerColor[LAYER_NETNAM],	/* Adresse du parametre */
	DARKGRAY				/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerPinCfg
(
	INSETUP,
	"ColorPin",			/* identification */
	&g_LayerDescr.LayerColor[LAYER_PIN],	  /* Adresse du parametre */
	RED						/* Valeur par defaut */
);


static PARAM_CFG_SETCOLOR ColorLayerSheetCfg
(
	INSETUP,
	"ColorSheet",			/* identification */
	&g_LayerDescr.LayerColor[LAYER_SHEET],	/* Adresse du parametre */
	MAGENTA					/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerSheetFileNameCfg
(
	INSETUP,
	"ColorSheetFileName",			/* identification */
	&g_LayerDescr.LayerColor[LAYER_SHEETFILENAME],	/* Adresse du parametre */
	BROWN				/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerSheetNameCfg
(
	INSETUP,
	"ColorSheetName",			/* identification */
	&g_LayerDescr.LayerColor[LAYER_SHEETNAME],	/* Adresse du parametre */
	CYAN					/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerSheetLabelCfg
(
	INSETUP,
	"ColorSheetLab",				/* identification */
	&g_LayerDescr.LayerColor[LAYER_SHEETLABEL],	/* Adresse du parametre */
	BROWN					/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerNoConnectCfg
(
	INSETUP,
	"ColorNoCo",					/* identification */
	&g_LayerDescr.LayerColor[LAYER_NOCONNECT],	/* Adresse du parametre */
	BLUE						/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerErcWarnCfg
(
	INSETUP,
	"ColorErcW",					/* identification */
	&g_LayerDescr.LayerColor[LAYER_ERC_WARN],	/* Adresse du parametre */
	GREEN						/* Valeur par defaut */
);

static PARAM_CFG_SETCOLOR ColorLayerErcErrCfg
(
	INSETUP,
	"ColorErcE",					/* identification */
	&g_LayerDescr.LayerColor[LAYER_ERC_ERR],	/* Adresse du parametre */
	RED						/* Valeur par defaut */
);

static PARAM_CFG_INT PlotMarginCfg
(
	INSETUP,
	"Pltmarg",			/* identification */
	&g_PlotMargin,			/* Adresse du parametre */
	300,				/* Valeur par defaut */
	0,10000				/* Valeurs extremes */
);

static PARAM_CFG_INT HPGLSpeed
(
	"HPGLSpd",				/* identification */
	&g_HPGL_Pen_Descr.m_Pen_Speed,				/* Adresse du parametre */
	20,						/* Valeur par defaut */
	2,45					/* Valeurs extremes */
);

static PARAM_CFG_INT HPGLDiam
(
	"HPGLDm",				/* identification */
	&g_HPGL_Pen_Descr.m_Pen_Diam,				/* Adresse du parametre */
	15,						/* Valeur par defaut */
	1,150					/* Valeurs extremes */
);

static PARAM_CFG_INT HPGLPenNum
(
	"HPGLNum",				/* identification */
	&g_HPGL_Pen_Descr.m_Pen_Num,				/* Adresse du parametre */
	1,						/* Valeur par defaut */
	1,8						/* Valeurs extremes */
);

static PARAM_CFG_INT PlotSheetOffsetX_A4
(
	"offX_A4",					/* identification */
	&g_Sheet_A4.m_Offset.x		/* Adresse du parametre */
);

static PARAM_CFG_INT PlotSheetOffsetY_A4
(
	"offY_A4",				/* identification */
	&g_Sheet_A4.m_Offset.y	/* Adresse du parametre */
);

static PARAM_CFG_INT PlotSheetOffsetX_A3
(
	"offX_A3",				/* identification */
	&g_Sheet_A3.m_Offset.x	/* Adresse du parametre */
);

static PARAM_CFG_INT PlotSheetOffsetY_A3
(
	"offY_A3",				/* identification */
	&g_Sheet_A3.m_Offset.y	/* Adresse du parametre */
);

static PARAM_CFG_INT PlotSheetOffsetX_A2
(
	"offX_A2",				/* identification */
	&g_Sheet_A2.m_Offset.x	/* Adresse du parametre */
);

static PARAM_CFG_INT PlotSheetOffsetY_A2
(
	"offY_A2",				/* identification */
	&g_Sheet_A2.m_Offset.y	/* Adresse du parametre */
);


static PARAM_CFG_INT PlotSheetOffsetX_A1
(
	"offX_A1",				/* identification */
	&g_Sheet_A1.m_Offset.x	/* Adresse du parametre */
);

static PARAM_CFG_INT PlotSheetOffsetY_A1
(
	"offY_A1",				/* identification */
	&g_Sheet_A1.m_Offset.y	/* Adresse du parametre */
);


static PARAM_CFG_INT PlotSheetOffsetX_A0
(
	"offX_A0",				/* identification */
	&g_Sheet_A0.m_Offset.x	/* Adresse du parametre */
);

static PARAM_CFG_INT PlotSheetOffsetY_A0
(
	"offY_A0",				/* identification */
	&g_Sheet_A0.m_Offset.y	/* Adresse du parametre */
);


static PARAM_CFG_INT PlotSheetOffsetX_A 
(
	"offX_A",				/* identification */
	&g_Sheet_A.m_Offset.x		/* Adresse du parametre */
);

static PARAM_CFG_INT PlotSheetOffsetY_A 
(
	"offY_A",				/* identification */
	&g_Sheet_A.m_Offset.y		/* Adresse du parametre */
);


static PARAM_CFG_INT PlotSheetOffsetX_B 
(
	"offX_B",				/* identification */
	&g_Sheet_B.m_Offset.x		/* Adresse du parametre */
);

static PARAM_CFG_INT PlotSheetOffsetY_B 
(
	"offY_B",				/* identification */
	&g_Sheet_B.m_Offset.y		/* Adresse du parametre */
);


static PARAM_CFG_INT PlotSheetOffsetX_C 
(
	"offX_C",				/* identification */
	&g_Sheet_C.m_Offset.x		/* Adresse du parametre */
);

static PARAM_CFG_INT PlotSheetOffsetY_C 
(
	"offY_C",				/* identification */
	&g_Sheet_C.m_Offset.y		/* Adresse du parametre */
);


static PARAM_CFG_INT PlotSheetOffsetX_D 
(
	"offX_D",				/* identification */
	&g_Sheet_D.m_Offset.x		/* Adresse du parametre */
);

static PARAM_CFG_INT PlotSheetOffsetY_D 
(
	"offY_D",				/* identification */
	&g_Sheet_D.m_Offset.y		/* Adresse du parametre */
);


static PARAM_CFG_INT PlotSheetOffsetX_E 
(
	"offX_E",				/* identification */
	&g_Sheet_E.m_Offset.x		/* Adresse du parametre */
);

static PARAM_CFG_INT PlotSheetOffsetY_E 
(
	"offY_E",				/* identification */
	&g_Sheet_E.m_Offset.y			/* Adresse du parametre */
);

static PARAM_CFG_INT CfgRepeatDeltaX 
(
	"RptD_X",				/* identification */
	&g_RepeatStep.x,			/* Adresse du parametre */
	0,						/* Valeur par defaut */
	-1000,+1000 			/* Valeurs extremes */
);

static PARAM_CFG_INT CfgRepeatDeltaY 
(
	"RptD_Y",				/* identification */
	&g_RepeatStep.y,			/* Adresse du parametre */
	100,					/* Valeur par defaut */
	-1000,+1000				/* Valeurs extremes */
);

static PARAM_CFG_INT CfgRepeatDeltaLabel 
(
	"RptLab",				/* identification */
	&g_RepeatDeltaLabel,		/* Adresse du parametre */
	1,						/* Valeur par defaut */
	-10,+10					/* Valeurs extremes */
);

static PARAM_CFG_INT CfgPenMinWidth
(
	"PenMin",					/* identification */
	&PenMinWidth,				/* Adresse du parametre */
	30,							/* Valeur par defaut */
	5, 100						/* Valeurs extremes */
);

static PARAM_CFG_WXSTRING CfgSimulatorCommandLine
(
	"SimCmd",				/* identification */
	&g_SimulatorCommandLine	/* Adresse du parametre */
);

static PARAM_CFG_INT OptNetListUseNamesCfg
(
	"UseNetN",				/* identification */
	&g_OptNetListUseNames,	/* Adresse du parametre */
	0,						/* Valeur par defaut */
	0, 1					/* Valeurs extremes */
);

PARAM_CFG_BASE * ParamCfgList[] =
{	
	& UserLibDirBufCfg,
	& LibNameBufCfg,

	& NetFormatCfg,

	& UnitCfg,
	& CursorShapeCfg,
	& ShowGridCfg,
	& DrawBgColorCfg,
	& ColorLayerWireCfg,
	& ColorLayerBusCfg,
	& ColorLayerJunctionCfg,
	& ColorLayerLLabelCfg,
	& ColorLayerGLabelCfg,
	& ColorLayerPinFunCfg,
	& ColorLayerPinNumCfg,
	& ColorLayerPinNamCfg,
	& ColorLayerFieldsCfg,
	& ColorLayerReferenceCfg,
	& ColorLayerValueCfg,
	& ColorLayerNotesCfg,
	& ColorLayerBodyCfg,
	& ColorLayerBodyBackgroundCfg,
	& ColorLayerNetNameCfg,
	& ColorLayerPinCfg,
	& ColorLayerSheetCfg,
	& ColorLayerSheetFileNameCfg,
	& ColorLayerSheetNameCfg,
	& ColorLayerSheetLabelCfg,
	& ColorLayerNoConnectCfg,
	& ColorLayerErcWarnCfg,
	& ColorLayerErcErrCfg,

	& PlotMarginCfg,
	& HPGLSpeed,
	& HPGLDiam,
	& HPGLPenNum,
	& PlotSheetOffsetX_A4,
	& PlotSheetOffsetY_A4,
	& PlotSheetOffsetX_A3,
	& PlotSheetOffsetY_A3,
	& PlotSheetOffsetX_A2,
	& PlotSheetOffsetY_A2,
	& PlotSheetOffsetX_A1,
	& PlotSheetOffsetY_A1,
	& PlotSheetOffsetX_A0,
	& PlotSheetOffsetY_A0,
	& PlotSheetOffsetX_A,
	& PlotSheetOffsetY_A,
	& PlotSheetOffsetX_B,
	& PlotSheetOffsetY_B,
	& PlotSheetOffsetX_C,
	& PlotSheetOffsetY_C,
	& PlotSheetOffsetX_D,
	& PlotSheetOffsetY_D,
	& PlotSheetOffsetX_E,
	& PlotSheetOffsetY_E,
	& CfgRepeatDeltaX,
	& CfgRepeatDeltaY,
	& CfgRepeatDeltaLabel,
	& CfgPenMinWidth,
	& CfgSimulatorCommandLine,
	& OptNetListUseNamesCfg,
	NULL
};
