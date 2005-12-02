	/*******************************************************/
	/** cfg.h : configuration: definition des structures  **/
	/*******************************************************/

#ifndef eda_global
#define eda_global extern
#endif

#define INSETUP TRUE

#define GROUP "/cvpcb"
#define GROUPCOMMON "/common"
#define GROUPLIB "/pcbnew/libraries"
#define GROUPEQU "/cvpcb/libraries"

/* Liste des parametres */

static PARAM_CFG_BASE CommandCfg
(
	GROUPLIB,			/* identification de groupe */
	PARAM_COMMAND_ERASE	/* type */
);

static PARAM_CFG_LIBNAME_LIST LibNameBufCfg
(
	"LibName",			/* identification */
	&g_LibName_List,	/* Adresse du parametre */
	GROUPLIB
);

static PARAM_CFG_LIBNAME_LIST EquivNameBufCfg
(
	"EquName",		/* identification */
	&g_ListName_Equ,	/* Adresse du parametre */
	GROUPEQU
);


static PARAM_CFG_INT NetInTypeCfg	// format Netliste ( 0= auto 1=ORCADPCB2 3,4=VIEWLOGIC)
(
	"NetITyp",		/* identification */
	&g_NetType,		/* Adresse du parametre */
	0, 4,			/* Valeurs extremes */
	1				/* Valeur par defaut */
);

static PARAM_CFG_WXSTRING NetInExtBufCfg
(
	"NetIExt",		  /* identification */
	&NetInExtBuffer
);

static PARAM_CFG_WXSTRING PkgInExtBufCfg
(
	"PkgIExt",		  /* identification */
	&PkgInExtBuffer /* Adresse du parametre */
);

static PARAM_CFG_WXSTRING NetDirBufCfg
(
	"NetDir",			/* identification */
	&g_UserNetDirBuffer,	/* Adresse du parametre */
	GROUPCOMMON
);

static PARAM_CFG_WXSTRING UserLibDirBufCfg
(
	"LibDir",		  /* identification */
	&g_UserLibDirBuffer, /* Adresse du parametre */
	GROUPLIB
);

static PARAM_CFG_INT NetTypeCfg
(
	"NetType",		/* identification */
	&g_NetType,		/* Adresse du parametre */
	0,				/* Valeur par defaut */
	0, 1			/* Valeurs extremes */
);

static PARAM_CFG_BOOL DisplayPadFillCfg
(
	INSETUP,
	"DiPadFi",		/* identification */
	&DisplayOpt.DisplayPadFill,		/* Adresse du parametre */
	TRUE				/* Valeur par defaut */
);

static PARAM_CFG_BOOL DisplayPadNumCfg
(
	INSETUP,
	"DiPadNu",		/* identification */
	&DisplayOpt.DisplayPadNum,		/* Adresse du parametre */
	TRUE				/* Valeur par defaut */
);

static PARAM_CFG_BOOL DisplayPadNoConnCfg
(
	INSETUP,
	"DiPadNC",		/* identification */
	&DisplayOpt.DisplayPadNoConn,		/* Adresse du parametre */
	FALSE				/* Valeur par defaut */
);

static PARAM_CFG_BOOL DisplayPadIsolCfg
(
	INSETUP,
	"DiPadMg",		/* identification */
	&DisplayOpt.DisplayPadIsol,		/* Adresse du parametre */
	FALSE				/* Valeur par defaut */
);

static PARAM_CFG_INT DisplayModEdgeCfg
(
	INSETUP,
	"DiModEd",		/* identification */
	&DisplayOpt.DisplayModEdge,		/* Adresse du parametre */
	1,				/* Valeur par defaut */
	0, 1			/* Valeurs extremes */
);

static PARAM_CFG_INT DisplayModTextCfg
(
	INSETUP,
	"DiModTx",		/* identification */
	&DisplayOpt.DisplayModText,		/* Adresse du parametre */
	1,				/* Valeur par defaut */
	0, 1			/* Valeurs extremes */
);

static PARAM_CFG_BOOL DisplayPcbTrackFillCfg
(
	INSETUP,
	"DiPcbTF",		/* identification */
	&DisplayOpt.DisplayPcbTrackFill,	  /* Adresse du parametre */
	TRUE				/* Valeur par defaut */
);

static PARAM_CFG_BASE * ParamCfgList[] =
{
	& CommandCfg,
	& NetInTypeCfg,
	& NetInExtBufCfg,
	& PkgInExtBufCfg,
	& NetDirBufCfg,
	& UserLibDirBufCfg,
	& LibNameBufCfg,
	& EquivNameBufCfg,
	& NetTypeCfg,
	& DisplayPadFillCfg,
	& DisplayPadNumCfg,
	& DisplayPadNoConnCfg,
	& DisplayPadIsolCfg,
	& DisplayModEdgeCfg,
	& DisplayModTextCfg,
	& DisplayPcbTrackFillCfg,
	NULL
};
