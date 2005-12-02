	/**************/
	/* savecmp()  */
	/**************/

/* sauvegarde la liste des associations composants/empreintes */

#include "fctsys.h"

#include "wxstruct.h"
#include "common.h"
#include "cvpcb.h"
#include "protos.h"

/* Chaines de caractere d'identification */
char EnteteCmpMod[] = {"Cmp-Mod V01"};

/*********************/
int savecmp(void)
/*********************/
/* Routine de sauvegarde du fichier des modules
	Retourne 1 si OK
			0 si ecriture non faite
*/
{
STORECMP * Cmp;
wxString FichCmp;
char Line[1024];

	/* calcul du nom du fichier */
	FichCmp = FFileName;
	ChangeFileNameExt(FichCmp, g_ExtCmpBuffer);

	dest = fopen(FichCmp.GetData(),"wt") ;
	if( dest == NULL ) return(0);	/* Erreur ecriture */

	fprintf(dest,"%s", EnteteCmpMod);
	fprintf(dest," Genere par %12.12s", Main_Title.GetData());
	fprintf(dest," le %s\n", DateAndTime(Line));

	for ( Cmp = BaseListeCmp ; Cmp != NULL ; Cmp = Cmp->Pnext )
		{
		fprintf(dest,"\nBeginCmp\n");
		fprintf(dest,"TimeStamp = %s;\n",Cmp->TimeStamp);
		fprintf(dest,"Reference = %s;\n",Cmp->Reference);
		fprintf(dest,"ValeurCmp = %s;\n",Cmp->Valeur);
		fprintf(dest,"IdModule  = %s;\n",Cmp->Module);
		fprintf(dest,"EndCmp\n");
		}
	fprintf(dest,"\nEndListe\n") ;
	fclose(dest) ;

	return(1) ;
}

	/**************/
	/* loadcmp()  */
	/**************/

/* recupere la liste des associations composants/empreintes */


int loadcmp(void)
{
int ii , kk ;
char ireforcad[80], valeur[80] , ilib[80] , itmp[80], namecmp[80];
char * pt_item = itmp, * pt_ligne = itmp, *pt_start;
int Version;
int state = 0;
STORECMP * Cmp;
char Line[1024];
wxString FichCmp;

	/* calcul du nom du fichier */
	FichCmp = FFileName;
	ChangeFileNameExt(FichCmp, g_ExtCmpBuffer);

	source = fopen(FichCmp.GetData(),"rt") ;
	if (source == NULL ) return(0) ;

	/* Identification du Type de fichier CmpMod */
	if ( fgets(Line,79,source) == 0 ) return(0);
	if( strnicmp(Line, EnteteCmpMod, 11 ) == 0 ) Version = 1;
	else
		{
		Version = 0;
		fseek(source,0,0);
		}

	/* lecture de la liste , Nouveau type */
	if( Version )
	  while( fgets(Line,79,source) != 0 )
		{
		if( strnicmp(Line, "EndListe", 8 ) == 0 ) break;

		/* Recherche du debut de description du composant */
		if( strnicmp(Line, "BeginCmp", 8 ) != 0 ) continue;
		memset(ireforcad, 0, sizeof(ireforcad));
		memset(valeur, 0, sizeof(valeur));
		memset(ilib, 0, sizeof(ilib));
		memset(itmp, 0, sizeof(itmp));
		memset(namecmp, 0, sizeof(namecmp));
		state = 1;

		while( state > 0 )
			{
			if( fgets(Line,79,source) == 0 )
				{
				state = -1; break;
				}

			if( strnicmp(Line, "EndCmp", 6 ) == 0 )
				{
				state = 0; break;
				}

			pt_item = NULL;

			/* Lecture Time Stamp */
			if( strnicmp(Line, "TimeStamp =", 11) == 0)
				{
				pt_ligne = Line + 11; pt_item = ireforcad;
				}

			/* lecture Reference (nom du composant) */
			if( strnicmp(Line, "Reference =", 11) == 0)
				{
				pt_ligne = Line + 11; pt_item = namecmp;
				}

			/* lecture valeur du composant */
			if( strnicmp(Line, "ValeurCmp =", 11) == 0)
				{
				pt_ligne = Line + 11; pt_item = valeur;
				}

			/* lecture nom de l'empreinte du composant */
			if( strnicmp(Line, "IdModule  =", 11) == 0)
				{
				pt_ligne = Line + 11; pt_item = ilib;
				}

			/* Copie de la ligne utile dans le buffer correspondant */
			if( pt_item == NULL) continue;	/* Ligne vide ou non comprise */
			while(*pt_ligne == ' ') pt_ligne++;
			pt_start = pt_item;
			while(*pt_ligne != ';')
				{
				if(*pt_ligne < ' ') break;
				*pt_item = *pt_ligne; pt_item++; pt_ligne++;
				}
			/* suppression des blancs inutiles en fin de ligne */
			while( (*pt_item <= ' ') && ( pt_item > pt_start) )
				*(pt_item--) = 0;
			} /* Fin lecture description de 1 composant */

		/* Recherche du composant correspondant en netliste et
			 mise a jour de ses parametres */
		if(state < 0 ) continue; /* Erreur */

		for ( Cmp = BaseListeCmp ; Cmp != NULL ; Cmp = Cmp->Pnext )
			{
			if (selection_type == 1 )
				{
				if(strnicmp(ireforcad,Cmp->TimeStamp,8) != 0 )
					continue ;
				}
			else
				if(stricmp(namecmp,Cmp->Reference) != 0 ) continue;

			/* recalcul de la valeur des ‚l‚ments,en ‚liminant les blancs*/
			kk = 0 ;
			while((itmp[kk] = Cmp->Valeur[kk]) > ' ' ) kk++;
			itmp[kk] = 0 ;
			if (stricmp(valeur,itmp) != 0 ) continue ;

			/* composant identifi‚ , copie du nom du module correspondant */
			strncpy(Cmp->Module, ilib, sizeof(Cmp->Module) - 1);
			}
		}


	/* lecture de la liste , ancien type */
	else while ( fgets(Line,79,source)  != 0)
		{
		/* lecture reference orcad (si elle existe) du composant */
		if ( Line[0] == '$' )
			{
			ii = 0 ; while( Line[ii] >= ' ')
				{
				ireforcad[ii] = Line[ii+1] ; ii++ ;
				}
			ireforcad[ii] = 0 ;
			fgets(Line,79,source) ;
			}

		/* lecture nom du composant */
		ii = 0 ; while( Line[ii] >= ' ') ii++ ; Line[ii] = 0 ;

		/* lecture valeur du composant(‚limination des blancs */
		if ( fgets(valeur,79,source) == 0) break ;
		ii = 0 ; while( valeur[ii] > ' ') ii++ ; valeur[ii] = 0 ;

		/* lecture nom de l'empreinte du composant */
		if ( fgets(ilib,79,source) == 0) break ;
		ii = 0 ; while( ilib[ii] >= ' ') ii++ ; ilib[ii] = 0 ;

		for ( Cmp = BaseListeCmp ; Cmp != NULL ; Cmp = Cmp->Pnext )
			{
			if (selection_type == 1 )
				{
				if(strnicmp(ireforcad,Cmp->TimeStamp,8) != 0 )
					continue ;
				}
			else
				if (stricmp(Line,Cmp->Reference) != 0 ) continue ;

			/* recalcul de la valeur des ‚l‚ments,en ‚liminant les blancs*/
			kk = 0 ;
			while((itmp[kk] = Cmp->Valeur[kk]) > ' ' ) kk++ ;
			itmp[kk] = 0 ;
			if (stricmp(valeur,itmp) != 0 ) continue ;

			/* composant identifi‚ , copie du nom du module correspondant */
			strncpy(Cmp->Module, ilib, sizeof(Cmp->Module) - 1);
			}
		}
	fclose(source) ;
	return(1) ;
}


