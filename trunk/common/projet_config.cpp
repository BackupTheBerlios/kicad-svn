		/**************************************************/
		/* projet_config : routines de trace du cartouche */
		/**************************************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"

#define CONFIG_VERSION 1

#define FORCE_LOCAL_CONFIG TRUE


/*********************************************************************/
static bool ReCreatePrjConfig(const wxString & local_config_filename,
			const wxString & GroupName, bool ForceUseLocalConfig)
/*********************************************************************/
/* Cree ou recree la configuration locale de kicad (filename.pro)
	initialise:
		g_Prj_Config
		g_Prj_Config_LocalFilename
		g_Prj_Default_Config_FullFilename
	return:
		TRUE di config locale
		FALSE si default config
*/
{
	// free old config
	if ( g_Prj_Config ) delete g_Prj_Config;
	g_Prj_Config  = NULL;
	
	// Init local Config filename
	if ( local_config_filename == "" ) g_Prj_Config_LocalFilename =  "kicad";
	else g_Prj_Config_LocalFilename = local_config_filename;

	ChangeFileNameExt( g_Prj_Config_LocalFilename, g_Prj_Config_Filename_ext );

	// Init local config filename
	if ( ForceUseLocalConfig || wxFileExists(g_Prj_Config_LocalFilename) )
	{
		g_Prj_Default_Config_FullFilename = "";
		g_Prj_Config = new wxFileConfig("", "",
			g_Prj_Config_LocalFilename,	"",
			wxCONFIG_USE_RELATIVE_PATH);
		g_Prj_Config->DontCreateOnDemand();

		if ( ForceUseLocalConfig ) return TRUE;
		
		// Test de la bonne version du fichier (ou groupe) de configuration
	int version = -1, def_version = 0;
		g_Prj_Config->SetPath(GroupName);
		version = g_Prj_Config->Read( "version", def_version);
		g_Prj_Config->SetPath("/");
		if ( version > 0 ) return TRUE;
		else delete g_Prj_Config;	// Version incorrecte
	}


	// Fichier local non trouve ou invalide
	g_Prj_Config_LocalFilename = "";
	g_Prj_Default_Config_FullFilename =
				ReturnKicadDatasPath() +
				"template/kicad" +
				g_Prj_Config_Filename_ext;
		
	// Recreate new config
	g_Prj_Config = new wxFileConfig("", "",
			"",	g_Prj_Default_Config_FullFilename,
			wxCONFIG_USE_RELATIVE_PATH);
	g_Prj_Config->DontCreateOnDemand();

	return FALSE;
}


/***************************************************************************************/
void WinEDA_App::WriteProjectConfig(const wxString & local_config_filename,
	const wxString & GroupName, PARAM_CFG_BASE ** List)
/***************************************************************************************/
/* enregistrement de la config "projet"*/
{
const PARAM_CFG_BASE * pt_cfg;
wxString msg;
	
	ReCreatePrjConfig(local_config_filename, GroupName,
			FORCE_LOCAL_CONFIG);
	/* Write date ( surtout pour eviter bug de wxFileConfig
	qui se trompe de rubrique si declaration [xx] en premiere ligne
	(en fait si groupe vide) */
char date[1024];
    DateAndTime(date);
	g_Prj_Config->SetPath("/");
	g_Prj_Config->Write("update", date);
	msg = GetAppName();
	g_Prj_Config->Write("last_client", msg);

	/* ecriture de la configuration */
	g_Prj_Config->DeleteGroup(GroupName);	// Erase all datas
	g_Prj_Config->Flush();

	g_Prj_Config->SetPath(GroupName);
	g_Prj_Config->Write("version", CONFIG_VERSION);
	g_Prj_Config->SetPath("/");

	for( ; *List != NULL ; List++)
		{
		pt_cfg = *List;
		if ( pt_cfg->m_Group ) g_Prj_Config->SetPath(pt_cfg->m_Group);
		else g_Prj_Config->SetPath(GroupName);

		switch( pt_cfg->m_Type )
			{
			case PARAM_INT:
				#undef PTCFG
				#define PTCFG ((PARAM_CFG_INT *)pt_cfg) 
				if (PTCFG->m_Pt_param == NULL) break;
				if ( pt_cfg->m_Setup)
					m_EDA_Config->Write(pt_cfg->m_Ident, *PTCFG->m_Pt_param);
				else
					g_Prj_Config->Write(pt_cfg->m_Ident, *PTCFG->m_Pt_param);
				break;

			case PARAM_SETCOLOR:
				#undef PTCFG
				#define PTCFG ((PARAM_CFG_SETCOLOR *)pt_cfg) 
				if (PTCFG->m_Pt_param == NULL) break;
				if ( pt_cfg->m_Setup)
					m_EDA_Config->Write(pt_cfg->m_Ident, *PTCFG->m_Pt_param);
				else
					g_Prj_Config->Write(pt_cfg->m_Ident, *PTCFG->m_Pt_param);
				break;

			case PARAM_DOUBLE:
				#undef PTCFG
				#define PTCFG ((PARAM_CFG_DOUBLE *)pt_cfg) 
				if (PTCFG->m_Pt_param == NULL) break;
				if ( pt_cfg->m_Setup)
					m_EDA_Config->Write(pt_cfg->m_Ident, *PTCFG->m_Pt_param);
				else
					g_Prj_Config->Write(pt_cfg->m_Ident, *PTCFG->m_Pt_param);
				break;

			case PARAM_BOOL:
				#undef PTCFG
				#define PTCFG ((PARAM_CFG_BOOL *)pt_cfg) 
				if (PTCFG->m_Pt_param == NULL) break;
				if ( pt_cfg->m_Setup)
					m_EDA_Config->Write(pt_cfg->m_Ident, (int)*PTCFG->m_Pt_param);
				else
					g_Prj_Config->Write(pt_cfg->m_Ident, (int)*PTCFG->m_Pt_param);
				break;

			case PARAM_WXSTRING:
				#undef PTCFG
				#define PTCFG ((PARAM_CFG_WXSTRING *)pt_cfg) 
				if (PTCFG->m_Pt_param == NULL) break;
				if ( pt_cfg->m_Setup)
					m_EDA_Config->Write(pt_cfg->m_Ident, *PTCFG->m_Pt_param);
				else
					g_Prj_Config->Write(pt_cfg->m_Ident, *PTCFG->m_Pt_param);
				break;

			case PARAM_LIBNAME_LIST:
			{
				#undef PTCFG
				#define PTCFG ((PARAM_CFG_LIBNAME_LIST *)pt_cfg) 
				if (PTCFG->m_Pt_param == NULL) break;
				wxArrayString * libname_list = PTCFG->m_Pt_param;
				if ( libname_list == NULL ) break;
				unsigned indexlib = 0;
				wxString cle_config;
				for ( ; indexlib < libname_list->GetCount(); indexlib++ )
				{
					cle_config = pt_cfg->m_Ident;
					// We use indexlib+1 because first lib name is LibName1
					cle_config << (indexlib+1);
					g_Prj_Config->Write(cle_config, libname_list->Item(indexlib));
				}
				break;
			}
			
			case PARAM_COMMAND_ERASE:	// Erase all datas
				if ( pt_cfg->m_Ident )
				{
					m_EDA_Config->DeleteGroup(pt_cfg->m_Ident);
					g_Prj_Config->DeleteGroup(pt_cfg->m_Ident);
				}
				break;
		}
	}
	
	g_Prj_Config->SetPath("/");
	delete g_Prj_Config;
	g_Prj_Config = NULL; 
}


/***************************************************************************************/
bool WinEDA_App::ReadProjectConfig(const wxString & local_config_filename,
		const wxString & GroupName, PARAM_CFG_BASE ** List,
		bool Load_Only_if_New)
/***************************************************************************************/
/* Lecture de la config "projet"
	*** si Load_Only_if_New == TRUE, elle n'est lue que si elle
	*** est differente de la config actuelle (dates differentes)

	return:
		TRUE si lue.
	Met a jour en plus:
		EDA_Appl->m_CurrentOptionFileDateAndTime
		EDA_Appl->m_CurrentOptionFile
*/
{
const PARAM_CFG_BASE * pt_cfg;
wxString timestamp;
	
	if ( List == NULL )return FALSE;

	ReCreatePrjConfig(local_config_filename, GroupName, FALSE);

	g_Prj_Config->SetPath("/");
	timestamp = g_Prj_Config->Read("update");
	if ( Load_Only_if_New && (timestamp != "" ) &&
		 (timestamp == EDA_Appl->m_CurrentOptionFileDateAndTime) )
		return FALSE;
	
	EDA_Appl->m_CurrentOptionFileDateAndTime = timestamp;
	
	if ( g_Prj_Default_Config_FullFilename != "")
		EDA_Appl->m_CurrentOptionFile = g_Prj_Default_Config_FullFilename;
	else
	{
		if ( wxPathOnly(g_Prj_Config_LocalFilename) == "" )
			EDA_Appl->m_CurrentOptionFile =
				wxGetCwd() + STRING_DIR_SEP + g_Prj_Config_LocalFilename;
		else	
			EDA_Appl->m_CurrentOptionFile = g_Prj_Config_LocalFilename;
	}

	for( ; *List != NULL ; List++)
		{
		pt_cfg = *List;
		if ( pt_cfg->m_Group ) g_Prj_Config->SetPath(pt_cfg->m_Group);
		else g_Prj_Config->SetPath(GroupName);
		switch( pt_cfg->m_Type )
		{
			case PARAM_INT:
			{
				#undef PTCFG
				#define PTCFG ((PARAM_CFG_INT *)pt_cfg) 
				int itmp;
				if ( pt_cfg->m_Setup)
					itmp = m_EDA_Config->Read(pt_cfg->m_Ident, PTCFG->m_Default );
				else
					itmp = g_Prj_Config->Read(pt_cfg->m_Ident, PTCFG->m_Default );
				if( (itmp < PTCFG->m_Min) || (itmp > PTCFG->m_Max) )
					itmp = PTCFG->m_Default;
				*PTCFG->m_Pt_param = itmp;
				break;
			}
				
			case PARAM_SETCOLOR:
			{
				#undef PTCFG
				#define PTCFG ((PARAM_CFG_SETCOLOR *)pt_cfg) 
				int itmp;
				if ( pt_cfg->m_Setup)
					itmp = m_EDA_Config->Read(pt_cfg->m_Ident, PTCFG->m_Default );
				else
					itmp = g_Prj_Config->Read(pt_cfg->m_Ident, PTCFG->m_Default );
				if( (itmp < 0) || (itmp > MAX_COLOR) )
					itmp = PTCFG->m_Default;
				*PTCFG->m_Pt_param = itmp;
				break;
			}
				
			case PARAM_DOUBLE:
			{
				#undef PTCFG
				#define PTCFG ((PARAM_CFG_DOUBLE *)pt_cfg) 
				double ftmp; wxString msg;
				if ( pt_cfg->m_Setup)
					msg = m_EDA_Config->Read(pt_cfg->m_Ident, "" );
				else
					msg = g_Prj_Config->Read(pt_cfg->m_Ident, "" );
				
				if ( msg == "" ) ftmp = PTCFG->m_Default;
				else
				{
					ftmp = (double) atof(msg.GetData());
					if( (ftmp < PTCFG->m_Min) || (ftmp > PTCFG->m_Max) )
						ftmp = PTCFG->m_Default;
				}
				*PTCFG->m_Pt_param = ftmp;
				break;
			}
	
			case PARAM_BOOL:
			{
				#undef PTCFG
				#define PTCFG ((PARAM_CFG_BOOL *)pt_cfg) 
				int itmp;
				if ( pt_cfg->m_Setup)
					itmp = m_EDA_Config->Read(pt_cfg->m_Ident, PTCFG->m_Default );
				else
					itmp = g_Prj_Config->Read(pt_cfg->m_Ident, PTCFG->m_Default );
				*PTCFG->m_Pt_param = itmp ? TRUE : FALSE;
				break;
			}
	
			case PARAM_WXSTRING:
			{
				#undef PTCFG
				#define PTCFG ((PARAM_CFG_WXSTRING *)pt_cfg) 
				if (PTCFG->m_Pt_param == NULL) break;
				if ( pt_cfg->m_Setup)
					*PTCFG->m_Pt_param = m_EDA_Config->Read(pt_cfg->m_Ident );
				else
					*PTCFG->m_Pt_param = g_Prj_Config->Read(pt_cfg->m_Ident );
				break;
			}

			case PARAM_LIBNAME_LIST:
			{
				#undef PTCFG
				#define PTCFG ((PARAM_CFG_LIBNAME_LIST *)pt_cfg) 
				int indexlib = 1;	// We start indexlib to 1 because first lib name is LibName1
				wxString libname, id_lib;
				wxArrayString * libname_list = PTCFG->m_Pt_param;
				while ( 1 )
				{
					id_lib = pt_cfg->m_Ident; id_lib << indexlib; indexlib++;
					libname = g_Prj_Config->Read(id_lib, "");
					if( libname == "" ) break;
					libname_list->Add(libname);
				}
				break;
			}
	
			case PARAM_COMMAND_ERASE:
				break;
		}
	}
	
	delete g_Prj_Config;
	g_Prj_Config = NULL;
	
	return TRUE;
}


/**************************************************************/
/* Constructeurs des descripteurs de structs de configuration */
/**************************************************************/

PARAM_CFG_BASE::PARAM_CFG_BASE(const char * ident, const paramcfg_id type,
			const char * group)
{
	m_Ident = ident;
	m_Type = type;
	m_Group = group;
	m_Setup = FALSE;
}


PARAM_CFG_INT::PARAM_CFG_INT(const char * ident, int * ptparam,
				int default_val, int min, int max,
				const char * group)
			: PARAM_CFG_BASE(ident, PARAM_INT, group)
{
	m_Pt_param = ptparam;
	m_Default = default_val;
	m_Min = min;
	m_Max = max;
}

PARAM_CFG_INT::PARAM_CFG_INT(bool Insetup, const char * ident, int * ptparam,
				int default_val, int min, int max,
				const char * group)
			: PARAM_CFG_BASE(ident, PARAM_INT, group)
{
	m_Pt_param = ptparam;
	m_Default = default_val;
	m_Min = min;
	m_Max = max;
	m_Setup = Insetup;
}

PARAM_CFG_SETCOLOR::PARAM_CFG_SETCOLOR(const char * ident, int * ptparam,
				int default_val, const char * group)
			: PARAM_CFG_BASE(ident, PARAM_SETCOLOR, group)
{
	m_Pt_param = ptparam;
	m_Default = default_val;
}

PARAM_CFG_SETCOLOR::PARAM_CFG_SETCOLOR(bool Insetup, const char * ident, int * ptparam,
				int default_val, const char * group)
			: PARAM_CFG_BASE(ident, PARAM_SETCOLOR, group)
{
	m_Pt_param = ptparam;
	m_Default = default_val;
	m_Setup = Insetup;
}

PARAM_CFG_DOUBLE::PARAM_CFG_DOUBLE(const char * ident, double * ptparam,
				double default_val, double min , double max,
				const char * group)
			: PARAM_CFG_BASE(ident, PARAM_DOUBLE, group)
{
	m_Pt_param = ptparam;
	m_Default = default_val;
	m_Min = min;
	m_Max = max;
}
PARAM_CFG_DOUBLE::PARAM_CFG_DOUBLE(bool Insetup, const char * ident, double * ptparam,
				double default_val, double min , double max,
				const char * group)
			: PARAM_CFG_BASE(ident, PARAM_DOUBLE, group)
{
	m_Pt_param = ptparam;
	m_Default = default_val;
	m_Min = min;
	m_Max = max;
	m_Setup = Insetup;
}

PARAM_CFG_BOOL::PARAM_CFG_BOOL(const char * ident, bool * ptparam,
				int default_val, const char * group)
			: PARAM_CFG_BASE(ident, PARAM_BOOL, group)
{
	m_Pt_param = ptparam;
	m_Default = default_val ? TRUE : FALSE;
}

PARAM_CFG_BOOL::PARAM_CFG_BOOL(bool Insetup, const char * ident, bool * ptparam,
				int default_val, const char * group)
			: PARAM_CFG_BASE(ident, PARAM_BOOL, group)
{
	m_Pt_param = ptparam;
	m_Default = default_val ? TRUE : FALSE;
	m_Setup = Insetup;
}

PARAM_CFG_WXSTRING::PARAM_CFG_WXSTRING(const char * ident,
					wxString * ptparam, const char * group)
			: PARAM_CFG_BASE(ident, PARAM_WXSTRING, group)
{
	m_Pt_param = ptparam;
}

PARAM_CFG_WXSTRING::PARAM_CFG_WXSTRING(bool Insetup, const char * ident,
					wxString * ptparam, const char * group)
			: PARAM_CFG_BASE(ident, PARAM_WXSTRING, group)
{
	m_Pt_param = ptparam;
	m_Setup = Insetup;
}

PARAM_CFG_LIBNAME_LIST::PARAM_CFG_LIBNAME_LIST(const char * ident,
					wxArrayString * ptparam, const char * group)
			: PARAM_CFG_BASE(ident, PARAM_LIBNAME_LIST, group)
{
	m_Pt_param = ptparam;
}
