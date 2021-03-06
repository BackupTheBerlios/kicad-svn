	/***************/
	/* genstf()  */
	/***************/

/* genere le fichier STF type 'ref' 'nom_empreinte' pour DRAFT */

#include "fctsys.h"

#include "wxstruct.h"
#include "common.h"
#include "cvpcb.h"

#include "protos.h"

void WinEDA_CvpcbFrame::WriteStuffList(wxCommandEvent & event)
{
FILE * FileEquiv;
STORECMP * Cmp;
wxString Line, FullFileName, Mask;

	if( nbcomp <= 0 ) return;

	/* calcul du nom du fichier */
	Mask = "*" + ExtRetroBuffer;
	FullFileName = FFileName;
	ChangeFileNameExt(FullFileName, ExtRetroBuffer);

	FullFileName = EDA_FileSelector("Create Stuff File",
					wxGetCwd(),					/* Chemin par defaut */
					FullFileName,		/* nom fichier par defaut */
					ExtRetroBuffer,		/* extension par defaut */
					Mask,				/* Masque d'affichage */
					this,
					wxSAVE,
					TRUE
					);
	if ( FullFileName == "") return;

	FileEquiv = fopen(FullFileName.GetData(),"wt") ;
	if (FileEquiv == 0 )
		{
		Line = _("Unable to create ") + FullFileName;
		DisplayError(this, Line,30);
		return;
		}

	/* Generation de la liste */
	for (Cmp = BaseListeCmp ; Cmp != NULL ; Cmp = Cmp->Pnext )
		{
		/* g�n�ration du composant si son empreinte est d�finie */
		if ( Cmp->Module[0]  <= ' ') continue;
		fprintf(FileEquiv, "comp = \"%s\" module = \"%s\"\n",
				Cmp->Reference, Cmp->Module);
		}

	fclose(FileEquiv);
}

