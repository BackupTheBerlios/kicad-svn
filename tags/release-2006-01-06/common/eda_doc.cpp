	/***************/
	/* eda_doc.cpp */
	/***************/

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWindows headers
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/mimetype.h>
#include <wx/tokenzr.h>

#include "fctsys.h"
#include "wxstruct.h"
#include "common.h"

//  Mime type extensions
static wxMimeTypesManager * mimeDatabase;
static const wxFileTypeInfo EDAfallbacks[] =
	{
	wxFileTypeInfo(wxT("text/pdf"),
					wxT("xpdf %s"),
					wxT("xpdf -p %s"),
                    wxT("pdf document (from Kicad)"),
					wxT("pdf"), wxT("PDF"), NULL),

	wxFileTypeInfo(wxT("text/html"),
                   wxT("wxhtml %s"),
					wxT("wxhtml %s"),
                    wxT("html document (from Kicad)"),
                    wxT("htm"), wxT("html"), NULL),

	wxFileTypeInfo(wxT("application/sch"),
                    wxT("eeschema %s"),
					wxT("eeschema -p %s"),
                    wxT("sch document (from Kicad)"),
                    wxT("sch"), wxT("SCH"), NULL),
	// must terminate the table with this!
	wxFileTypeInfo()
};


/********************************************************************/
bool GetAssociatedDocument(wxFrame * frame, const wxString & LibPath,
							const wxString & DocName)
/*********************************************************************/
/* appelle le viewer associ� au fichier de doc du composant (mime type)
	LibPath est le prefixe du chemin des docs (/usr/local/kicad/library par exe.)
	DocName est le nom du fichier. Les wildcarts sont autorisees
	la recherche se fait sur LibPath/doc/DocName
	On peut sp�cifier plusieus noms s�par�s ar des ; dans DocName
*/
{
wxString docpath, fullfilename;
wxString Line;
bool success = FALSE;

	/* Calcul du nom complet du fichier de documentation */
	if ( wxIsAbsolutePath(DocName) ) fullfilename = DocName;
	else
		{
		docpath = LibPath + wxT("doc/");
		fullfilename = docpath + DocName;
		}

#ifdef __WINDOWS__
	fullfilename.Replace(UNIX_STRING_DIR_SEP, WIN_STRING_DIR_SEP);
#else
	fullfilename.Replace(WIN_STRING_DIR_SEP, UNIX_STRING_DIR_SEP);
#endif

	if ( wxIsWild(fullfilename) )
		{
		fullfilename =
			EDA_FileSelector(_("Doc Files"),	/* Titre de la fenetre */
					wxPathOnly(fullfilename),		/* Chemin par defaut */
					fullfilename,				/* nom fichier par defaut */
					wxEmptyString,				/* extension par defaut */
					wxEmptyString,				/* Masque d'affichage */
					frame,					/* parent frame */
					wxOPEN,					/* wxSAVE, wxOPEN ..*/
					TRUE,	/* true = ne change pas le repertoire courant */
					wxPoint(-1,-1)
					);
		if ( fullfilename.IsEmpty() ) return FALSE;
		}

	if ( ! wxFileExists(fullfilename) )
		{
		Line = _("Doc File ") + fullfilename + _(" not found");
		DisplayError(frame, Line);
		return FALSE;
		}

wxFileType * filetype;
wxFileName CurrentFileName(fullfilename);
wxString ext, command, type;
	ext = CurrentFileName.GetExt();
	filetype = wxTheMimeTypesManager->GetFileTypeFromExtension(ext);

	if ( ! filetype )	// 2ieme tentative
	{
        mimeDatabase = new wxMimeTypesManager;
        mimeDatabase->AddFallbacks(EDAfallbacks);
		filetype = mimeDatabase->GetFileTypeFromExtension(ext);
		delete mimeDatabase;
		mimeDatabase = NULL;
	}

	if ( filetype )
	{
		wxFileType::MessageParameters params(fullfilename, type);
		success = filetype->GetOpenCommand( &command, params);
		delete filetype;
		if ( success ) wxExecute(command);
	}

	if ( ! success)
	{
#ifdef __LINUX__
		if ( ext == wxT("pdf") )
		{
			success = TRUE; command.Empty();
			if ( wxFileExists( wxT("/usr/bin/xpdf")) )
				command = wxT("xpdf ") + fullfilename;
			else if ( wxFileExists( wxT("/usr/bin/konqueror") ))
				command = wxT("konqueror ") + fullfilename;
			else if ( wxFileExists( wxT("/usr/bin/gpdf") ))
				command = wxT("gpdf ") + fullfilename;
			if ( command.IsEmpty() ) // not started
			{
				DisplayError(frame,
					_(" Cannot find the PDF viewer (xpdf, gpdf or konqueror) in /usr/bin/") );
				success = FALSE;
			}
			else wxExecute(command);
		}
		else
#endif
		{
			Line.Printf( _("Unknown MIME type for Doc File [%s] (%s)"),
				fullfilename.GetData(), ext.GetData());
			DisplayError(frame, Line);
		}
	}

	return success;
}

/******************************************************************/
int KeyWordOk(const wxString & KeyList, const wxString & Database )
/******************************************************************/
/* Recherche si dans le texte Database on retrouve tous les mots
	cles donnes dans KeyList ( KeyList = suite de mots cles
	separes par des espaces
	Retourne:
		0 si aucun mot cle trouv�
		1 si mot cle trouv�
*/
{
wxString KeysCopy, DataList;
	
	if( KeyList.IsEmpty() ) return(0);

	KeysCopy = KeyList; KeysCopy.MakeUpper();
	DataList = Database; DataList.MakeUpper();

wxStringTokenizer Token(KeysCopy, wxT(" \n\r"));
	while ( Token.HasMoreTokens() ) 
	{
		wxString Key = Token.GetNextToken(); 
		// Search Key in Datalist:
		wxStringTokenizer Data(DataList, wxT(" \n\r"));
		while ( Data.HasMoreTokens() ) 
		{
			wxString word = Data.GetNextToken(); 
			if ( word == Key ) return 1;	// Key found !
		}
	}

	// keyword not found
	return(0);
}


