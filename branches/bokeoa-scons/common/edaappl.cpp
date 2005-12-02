	/****************/
	/* edaappl.ccpp */
	/****************/
/*
	 ROLE: methodes relative a la classe winEDA_App, communes
	 aux environements window et linux
*/

#include "fctsys.h"
#include <wx/image.h>
#include "wx/html/htmlwin.h"
#include "wx/fs_zip.h"

#define EDA_BASE

#include "wxstruct.h"
#include "macros.h"

#include "gr_basic.h"
#include "common.h"
#include "worksheet.h"
#include "id.h"

#include "bitmaps.h"
#include "Language.xpm"

#ifdef __WINDOWS__
/* Icons for language choice (only for Windows)*/
#include "Lang_Def.xpm"
#include "Lang_En.xpm"
#include "Lang_Es.xpm"
#include "Lang_Fr.xpm"
#include "Lang_Pt.xpm"
#endif


#define FONT_DEFAULT_SIZE 9

#ifdef __WINDOWS__
#define SETBITMAPS(icon) item->SetBitmaps(apply_xpm, (icon))
#else
#define SETBITMAPS(icon)
#endif

	/*****************************/
	/* Constructeur de WinEDA_App */
	/*****************************/

WinEDA_App::WinEDA_App(void)
{
	m_Checker = NULL;
	m_MainFrame = NULL;
	m_PcbFrame = NULL;
	m_ModuleEditFrame = NULL;	// Edition des modules
	SchematicFrame = NULL;		// Edition des Schemas
	LibeditFrame = NULL;		// Edition des composants
	ViewlibFrame = NULL;		// Visualisation des composants
	m_CvpcbFrame = NULL;
	m_GerberFrame = NULL;		// ecran de visualisation GERBER

	m_LastProjectMaxCount = 10;
	m_HtmlCtrl = NULL;
	m_EDA_CommonConfig = NULL;
	m_EDA_Config = NULL;
	m_Env_Defined = FALSE;
	m_LanguageId = wxLANGUAGE_DEFAULT;
	m_Language_Menu = NULL;

	/* Init de variables globales d'interet general: */
	g_FloatSeparator = '.';		// Nombres flottants = 0.1 par exemple
}

	/*****************************/
	/* Destructeur de WinEDA_App */
	/*****************************/

WinEDA_App::~WinEDA_App(void)
{
	SaveSettings();

	/* delete data non directement geree par wxAppl */
	delete g_Prj_Config;
	delete m_EDA_Config;
	delete m_EDA_CommonConfig;
	delete g_StdFont;
	delete g_DialogFont;
	delete g_ItalicFont;
	delete g_FixedFont;
	delete DrawPen;
	delete DrawBrush;
	if ( m_Checker ) delete m_Checker;
}

/**************************************************/
void WinEDA_App::InitEDA_Appl(const wxString & name)
/***************************************************/
{
wxString ident;
wxString EnvLang;

	ident = name + "-" + wxGetUserId();
    m_Checker = new wxSingleInstanceChecker(ident);

#if 0
	/*Test for an UTF8 (unicode) environment */
	wxGetEnv("LANG", &EnvLang);
	/* If LANG is xx_XX.UTF8, export LANG=xx_XX */
	wxString lang = EnvLang.AfterFirst('.');
	if ( lang != "" )
	{
		printf("locale is %s\n", lang.GetData());
		lang = EnvLang.BeforeFirst('.');
		setlocale( LC_ALL, lang.GetData());
		printf("Set locale %s to non UTF8 %s\n", EnvLang.GetData(), lang.GetData());
	}
#endif

	/* Init environnement
	(KICAD definit le chemin de kicad ex: set KICAD=d:\kicad) */
	m_Env_Defined = wxGetEnv("KICAD", &m_KicadEnv);
	if ( m_Env_Defined )	// m_KicadEnv doit finir par "/" ou "\"
	{
		m_KicadEnv.Replace("\\", "/");
		if ( m_KicadEnv.Last() != '/' ) m_KicadEnv += "/";
	}

	/* Prepare On Line Help */
	m_HelpFileName = name + ".html";

	// Init parametres pour configuration
	SetVendorName("kicad");
	SetAppName(name);
	m_EDA_Config = new wxConfig(name);
	m_EDA_CommonConfig = new wxConfig("kicad_common");

	/* Creation des outils de trace */
	DrawPen = new wxPen("GREEN", 1, wxSOLID);
	DrawBrush = new wxBrush("BLACK", wxTRANSPARENT);

	/* Creation des fontes utiles */
	g_StdFontPointSize = FONT_DEFAULT_SIZE;
	g_SmallFontPointSize = (FONT_DEFAULT_SIZE * 70) / 100;
	g_DialogFontPointSize = FONT_DEFAULT_SIZE;
	g_FixedFontPointSize = FONT_DEFAULT_SIZE;
	g_StdFont = new wxFont(g_StdFontPointSize, wxROMAN, wxNORMAL,  wxNORMAL);
	g_SmallFont = new wxFont(g_SmallFontPointSize, wxROMAN, wxNORMAL,  wxNORMAL);
	g_DialogFont = new wxFont(g_DialogFontPointSize, wxROMAN, wxNORMAL,  wxNORMAL);
	g_ItalicFont = new wxFont(g_DialogFontPointSize, wxROMAN, wxFONTSTYLE_ITALIC,  wxNORMAL);
	g_FixedFont = new wxFont(g_FixedFontPointSize, wxMODERN, wxNORMAL,  wxNORMAL);

	/* installation des gestionnaires de visu d'images (pour help) */
	wxImage::AddHandler(new wxPNGHandler);
	wxImage::AddHandler(new wxGIFHandler);
	wxImage::AddHandler(new wxJPEGHandler);
    wxFileSystem::AddHandler(new wxZipFSHandler);

	// Analyse command line & init binary path
	SetBinDir();

	// Internationalisation: chargement du Dictionnaire de kicad
	m_EDA_CommonConfig->Read("Language", &m_LanguageId, wxLANGUAGE_DEFAULT);

bool succes = SetLanguage(TRUE);
	if ( ! succes )
		{
		}

	if ( atof("0,1") ) g_FloatSeparator = ','; // Nombres flottants = 0,1
	else  g_FloatSeparator = '.';

}


/*****************************************/
void WinEDA_App::InitOnLineHelp(void)
/*****************************************/
/* Init On Line Help
*/
{
wxString fullfilename = FindKicadHelpPath();

	fullfilename += "kicad.hhp";
	if ( wxFileExists(fullfilename) )
		{
		m_HtmlCtrl = new wxHtmlHelpController(wxHF_TOOLBAR |
				wxHF_CONTENTS | wxHF_PRINT | wxHF_OPEN_FILES
				/*| wxHF_SEARCH */);
		m_HtmlCtrl->UseConfig(m_EDA_Config);
		m_HtmlCtrl->SetTitleFormat("Kicad Help");
		m_HtmlCtrl->AddBook(fullfilename);
		}
}


/*******************************/
bool WinEDA_App::SetBinDir(void)
/*******************************/
/*
 Analyse la ligne de commande pour retrouver le chemin de l'executable
 Sauve en WinEDA_App::m_BinDir le repertoire de l'executable
*/

{
	/* Calcul du chemin ou se trouve l'executable */

#ifdef __UNIX__
	/* Sous LINUX ptarg[0] ne donne pas le chemin complet de l'executable,
	il faut le retrouver par la commande "which <filename> si aucun
	chemin n'est donne */
FILE * ftmp;
int ii;
char Line[1024];
char FileName[1024];
#define TMP_FILE "/tmp/kicad.tmp"

	Line[0] = 0;
	if( strchr(argv[0],'/') == NULL ) /* pas de chemin */
		{
		sprintf(FileName,"which %s > %s", argv[0], TMP_FILE);
		Line[0] = 0;
		ii = system(FileName);
		if( (ftmp = fopen(TMP_FILE, "rt")) != NULL )
			{
			fgets(Line,1000,ftmp);
			fclose(ftmp);
			remove(TMP_FILE);
			}
		}
	else strcpy(Line, argv[0]);
	m_BinDir = Line;
#else
	m_BinDir = argv[0];
#endif

	m_BinDir.Replace("\\", "/");
	while ( m_BinDir.Last() != '/' ) m_BinDir.RemoveLast();

	return TRUE;
}


/*********************************/
void WinEDA_App::GetSettings(void)
/*********************************/
/* Lit les infos utiles sauvees lors de la derniere utilisation du logiciel
*/
{
wxString Line, Ident;
unsigned ii;

	m_HelpSize.x = 500; m_HelpSize.y = 400;

	if ( m_EDA_CommonConfig )
	{
		m_LanguageId = m_EDA_CommonConfig->Read("Language", wxLANGUAGE_DEFAULT);
		g_EditorName = m_EDA_CommonConfig->Read("Editor");
	}

	if ( ! m_EDA_Config ) return;


	for ( ii = 0; ii < 10; ii++ )
	{
	Ident = "LastProject"; if ( ii ) Ident << ii;
	if( m_EDA_Config->Read(Ident, &Line) )
		m_LastProject.Add(Line);
	}

	g_StdFontPointSize = m_EDA_Config->Read("SdtFontSize", FONT_DEFAULT_SIZE);
	g_SmallFontPointSize = m_EDA_Config->Read("SmallFontSize", (FONT_DEFAULT_SIZE * 70) / 100);
	g_DialogFontPointSize = m_EDA_Config->Read("DialogFontSize", FONT_DEFAULT_SIZE);
	g_FixedFontPointSize = m_EDA_Config->Read("DialogFontSize", FONT_DEFAULT_SIZE);

	Line = m_EDA_Config->Read("SdtFontType", "");
	if ( Line != "" ) g_StdFont->SetFaceName(Line);
	ii = m_EDA_Config->Read("SdtFontStyle", wxNORMAL);
	g_StdFont->SetStyle(ii);
	ii = m_EDA_Config->Read("SdtFontWeight", wxNORMAL);
	g_StdFont->SetWeight(ii);
	g_StdFont->SetPointSize(g_StdFontPointSize);

	g_SmallFont->SetPointSize(g_SmallFontPointSize);

	ii = m_EDA_Config->Read("DialogFontWeight", wxNORMAL);
	g_StdFont->SetWeight(ii);
	g_DialogFont->SetPointSize(g_DialogFontPointSize);

	g_FixedFont->SetPointSize(g_FixedFontPointSize);


	if( m_EDA_Config->Read("WorkingDir", &Line) )
		{
		if ( wxDirExists(Line) ) wxSetWorkingDirectory(Line);
		}
	m_EDA_Config->Read("BgColor", &DrawBgColor);
}


/**********************************/
void WinEDA_App::SaveSettings(void)
/**********************************/
/* Enregistre dans le fichier xxx.ini la position et dimension
de la fenetre du logiciel xxx (eeschema, cvpcb ou pcbnew)
*/
{
unsigned int ii;
char Line[80];


	if( m_EDA_Config == NULL ) return;

	m_EDA_Config->Write("SdtFontSize", g_StdFontPointSize);
	m_EDA_Config->Write("SdtFontType", g_StdFont->GetFaceName());
	m_EDA_Config->Write("SdtFontStyle", g_StdFont->GetStyle());
	m_EDA_Config->Write("SdtFontWeight", g_StdFont->GetWeight());

	m_EDA_Config->Write("SmallFontSize", g_SmallFontPointSize);

	m_EDA_Config->Write("DialogFontSize", g_DialogFontPointSize);
	m_EDA_Config->Write("DialogFontWeight", g_DialogFont->GetWeight());

	m_EDA_Config->Write("FixedFontSize", g_FixedFontPointSize);

	m_EDA_Config->Write("WorkingDir", wxGetCwd());

	for( ii = 0; ii < 10; ii++ )
		{
		if ( ii ) sprintf(Line,"LastProject%d", ii);
		else strcpy(Line,"LastProject");
		if ( ii < m_LastProject.GetCount() )
			m_EDA_Config->Write(Line, m_LastProject[ii]);
		else
			m_EDA_Config->Write(Line, "");
		}
}

/*********************************************/
bool WinEDA_App::SetLanguage(bool first_time)
/*********************************************/
/* Set the dictionary file name for internationalization
	the files are in kicad/internat/xx or kicad/internat/xx_XX
	and are named kicad.mo
*/
{
wxString DictionaryName("kicad");	// dictionary file name without extend (full name is kicad.mo)
wxString BaseDictionaryPath("internat");	// Real path is kicad/internat/xx_XX or kicad/internat/xx

	m_Locale.Init(m_LanguageId);
	if ( first_time )
	{
	wxString dic_path = ReturnKicadDatasPath() + BaseDictionaryPath;
		m_Locale.AddCatalogLookupPathPrefix(dic_path);
	}
	else
	{
		if ( m_EDA_CommonConfig )
			m_EDA_CommonConfig->Write("Language", m_LanguageId);
	}
	if ( ! m_Locale.IsLoaded(DictionaryName) ) m_Locale.AddCatalog(DictionaryName);
	SetLanguageList(NULL);


	if ( atof("0,1") ) g_FloatSeparator = ','; // Nombres flottants = 0,1
	else  g_FloatSeparator = '.';

	return m_Locale.IsOk();
}


/**************************************************/
void WinEDA_App::SetLanguageIdentifier(int menu_id)
/**************************************************/
/* return in m_LanguageId the language id (wxWidgets language identifier)
	from menu id (internal menu identifier)
*/
{
	switch (menu_id)
	{
		case ID_LANGUAGE_ITALIAN:
			m_LanguageId = wxLANGUAGE_ITALIAN;
			break;

		case ID_LANGUAGE_PORTUGUESE:
			m_LanguageId = wxLANGUAGE_PORTUGUESE;
			break;

		case ID_LANGUAGE_RUSSIAN:
			m_LanguageId = wxLANGUAGE_RUSSIAN;
			break;

		case ID_LANGUAGE_DUTCH:
			m_LanguageId = wxLANGUAGE_DUTCH;
			break;

		case ID_LANGUAGE_SPANISH:
			m_LanguageId = wxLANGUAGE_SPANISH;
			break;

		case ID_LANGUAGE_ENGLISH:
			m_LanguageId = wxLANGUAGE_ENGLISH;
			break;

		case ID_LANGUAGE_FRENCH:
			m_LanguageId = wxLANGUAGE_FRENCH;
			break;

		default:
			m_LanguageId = wxLANGUAGE_DEFAULT;
			break;

	}
}



/*********************************************************/
wxMenu * WinEDA_App::SetLanguageList(wxMenu * MasterMenu)
/*********************************************************/
/* Create menu list for language choice.
*/
{
wxMenuItem * item;

	if ( m_Language_Menu == NULL )
	{
		m_Language_Menu = new wxMenu;
		item = new wxMenuItem(m_Language_Menu, ID_LANGUAGE_DEFAULT,
				_("Default"), "", wxITEM_CHECK );
		SETBITMAPS(lang_def_xpm);
		m_Language_Menu->Append(item);

		item = new wxMenuItem(m_Language_Menu, ID_LANGUAGE_ENGLISH,
			"English", "", wxITEM_CHECK);
		SETBITMAPS(lang_en_xpm);
		m_Language_Menu->Append(item);

		item = new wxMenuItem(m_Language_Menu, ID_LANGUAGE_FRENCH,
			_("French"), "", wxITEM_CHECK);
		SETBITMAPS(lang_fr_xpm);
		m_Language_Menu->Append(item);

		item = new wxMenuItem(m_Language_Menu, ID_LANGUAGE_SPANISH,
			_("Spanish"), "", wxITEM_CHECK);
		SETBITMAPS(lang_es_xpm);
		m_Language_Menu->Append(item);

		item = new wxMenuItem(m_Language_Menu, ID_LANGUAGE_PORTUGUESE,
			_("Portuguese"), "", wxITEM_CHECK);
		SETBITMAPS(lang_pt_xpm);
		m_Language_Menu->Append(item);


		item = new wxMenuItem(m_Language_Menu, ID_LANGUAGE_ITALIAN,
			_("Italian"), "", wxITEM_CHECK);
//		SETBITMAPS(apply_xpm, lang_it_xpm);	TODO
		m_Language_Menu->Append(item);

		item = new wxMenuItem(m_Language_Menu, ID_LANGUAGE_DUTCH,
			_("Dutch"), "", wxITEM_CHECK);
//		item->SETBITMAPS(apply_xpm, lang_de_xpm);	TODO
		m_Language_Menu->Append(item);

#if 0
		item = new wxMenuItem(m_Language_Menu, ID_LANGUAGE_RUSSIAN,
			_("Russian"), "", wxITEM_CHECK);
		SETBITMAPS(lang_pt_xpm);
		m_Language_Menu->Append(item);
#endif
	}

	m_Language_Menu->Check(ID_LANGUAGE_ITALIAN, FALSE);
	m_Language_Menu->Check(ID_LANGUAGE_PORTUGUESE, FALSE);
	m_Language_Menu->Check(ID_LANGUAGE_SPANISH, FALSE);
	m_Language_Menu->Check(ID_LANGUAGE_FRENCH, FALSE);
	m_Language_Menu->Check(ID_LANGUAGE_ENGLISH, FALSE);
	m_Language_Menu->Check(ID_LANGUAGE_DEFAULT, FALSE);

	switch ( m_LanguageId )
	{
		case wxLANGUAGE_RUSSIAN:
			m_Language_Menu->Check(ID_LANGUAGE_RUSSIAN, TRUE);
			break;
		case wxLANGUAGE_DUTCH:
			m_Language_Menu->Check(ID_LANGUAGE_DUTCH, TRUE);
			break;
		case wxLANGUAGE_FRENCH:
			m_Language_Menu->Check(ID_LANGUAGE_FRENCH, TRUE);
			break;
		case wxLANGUAGE_ENGLISH:
			m_Language_Menu->Check(ID_LANGUAGE_ENGLISH, TRUE);
			break;
		case wxLANGUAGE_SPANISH:
			m_Language_Menu->Check(ID_LANGUAGE_SPANISH, TRUE);
			break;
		case wxLANGUAGE_PORTUGUESE:
			m_Language_Menu->Check(ID_LANGUAGE_PORTUGUESE, TRUE);
			break;

		case wxLANGUAGE_ITALIAN:
			m_Language_Menu->Check(ID_LANGUAGE_ITALIAN, TRUE);
			break;

		default:
			m_Language_Menu->Check(ID_LANGUAGE_DEFAULT, TRUE);
			break;
	}

	if ( MasterMenu )
	{
		ADD_MENUITEM_WITH_HELP_AND_SUBMENU(MasterMenu, m_Language_Menu,
			ID_LANGUAGE_CHOICE,  _("Language"),
			"For test only, use Default setup for normal use",
			language_xpm);
	}
	return m_Language_Menu;
}


