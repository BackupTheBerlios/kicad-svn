	/**********************/
	/* CVPCB: autosel.cc  */
	/**********************/

/* Routines de selection automatique des modules */

#include "fctsys.h"

#include "common.h"
#include "cvpcb.h"

#include "protos.h"

#define QUOTE '\''
#define STRUCT_AUTOMODULE 1

typedef struct AutoModule
	{
	int Type;
	AutoModule * Pnext;
	char Name[17];
	char LibName[17];
	char Library[17];
	} AUTOMODULE;


/* routines locales : */
static int auto_select(WinEDA_CvpcbFrame * frame, STORECMP * Cmp, AUTOMODULE * BaseListeMod);
static void auto_associe(WinEDA_CvpcbFrame * frame);


	/*************************************************************/
void WinEDA_CvpcbFrame::AssocieModule(wxCommandEvent& event)
/*************************************************************/
/* Fonction liee au boutton "Auto"
	Lance l'association automatique modules/composants
*/
{
	auto_associe(this);
}

/**************************************************/
static void auto_associe(WinEDA_CvpcbFrame * frame)
/**************************************************/
{
unsigned ii, j, k;
wxString EquivFileName;
char Line[1024];
FILE *fichierstf ;	/* sert en lecture des differents fichiers *.STF */
AUTOMODULE * ItemModule,* NextMod;
AUTOMODULE * BaseListeMod = NULL;
STORECMP * Componant;

int nb_correspondances = 0;

	if( nbcomp <= 0 ) return;

	/* recherche des equivalences a travers les fichiers possibles */
	for( ii= 0 ; ii < g_ListName_Equ.GetCount(); ii++)
		{
		/* Calcul du nom complet avec son chemin */
		EquivFileName = MakeFileName(g_RealLibDirBuffer,g_ListName_Equ[ii],g_EquivExtBuffer);

		if ( (fichierstf = fopen(EquivFileName.GetData(),"rt"))  == 0)
			{
			sprintf(Line,_("Library: <%s> not found"),EquivFileName.GetData());
			DisplayError(frame, Line,10);
			continue ;
			}

		/* lecture fichier n */
		while ( fgets(Line,79,fichierstf) != 0 )
			{
			/* elimination des lignes vides */
			for (j = 0 ; j < 40 ; j++ )
				{ 
				if (Line[j] == 0 ) goto fin_de_while ;
				if (Line[j] == QUOTE ) break ;
				}

			ItemModule = (AUTOMODULE * ) MyZMalloc(sizeof(AUTOMODULE) );
			ItemModule->Pnext = BaseListeMod;
			BaseListeMod = ItemModule;
			
			/* stockage du composant ( 'namecmp'  'namelib')
			name sur 16 caract et namelib sur 16 octets */
			for ( j++ , k = 0 ; j < 40 ; j++, k++)
				{
				if ( Line[j] == QUOTE) break ;
				if (k < 16) ItemModule->Name[k] = Line[j];
				}
			j++ ;
			for ( ; j < 80 ; ) if (Line[j++] == QUOTE) break ;
			for ( k = 0 ; k < 16 ; k++ , j++)
				{
				if (Line[j] == QUOTE) break ;
				ItemModule->LibName[k] = Line[j];
				}
			nb_correspondances++ ;
			fin_de_while:;
			}
		fclose(fichierstf) ;

		/* Affichage Statistiques */
		sprintf(Line,_("%d equivalences"),nb_correspondances);
		frame->SetStatusText(Line, 0);
		}

	Componant = BaseListeCmp;
	for ( ii = 0; Componant != NULL; Componant = Componant->Pnext, ii++ )
		{
		frame->m_ListCmp->SetSelection(ii,TRUE);
		if( Componant->Module[0] <= ' ' )
			auto_select(frame, Componant, BaseListeMod);
		}

	/* Liberation memoire */
	for( ItemModule = BaseListeMod; ItemModule != NULL;
			 ItemModule = NextMod)
		{
		NextMod = ItemModule->Pnext; MyFree(ItemModule);
		}
	BaseListeMod = NULL;
}


/****************************************************************/
static int auto_select(WinEDA_CvpcbFrame * frame, STORECMP * Cmp,
			AUTOMODULE * BaseListeMod)
/****************************************************************/

/* associe automatiquement composant et Module
	Retourne;
		0 si OK
		1 si module specifie non trouve en liste librairie
		2 si pas de module specifie dans la liste des equivalences
*/
{
AUTOMODULE * ItemModule;
STOREMOD * Module;
char Line[1024];

	/* examen de la liste des correspondances */
	ItemModule = BaseListeMod;
	for ( ; ItemModule != NULL; ItemModule = ItemModule->Pnext )
		{
		if (stricmp (ItemModule->Name,Cmp->Valeur) != 0) continue;

		/* Correspondance trouvee, recherche nom module dans la liste des
		modules disponibles en librairie */
		Module= BaseListePkg;
		for ( ;Module != NULL; Module = Module->Pnext )
			{

			if(strnicmp(ItemModule->LibName,Module->Module,16) == 0 )
				{ /* empreinte trouv‚e */
				strcpy(CurrentPkg,Module->Module);
				frame->SetNewPkg();
				return(0);
				}
			}
		sprintf(Line,
				  _("Component %s: Footprint %s not found in libraries"),
						Cmp->Valeur, ItemModule->LibName);
		DisplayError(frame, Line, 10);
		return( 2 );
		}
	return(1);
}

