	/**************/
	/* listlib.cpp */
	/**************/

/*
cherche toutes les ref <chemin lib>*.??? si nom fichier pr‚sent,
ou examine <chemin lib>[MODULE.LIB]
*/

#include "fctsys.h"

#include "wxstruct.h"

#include "common.h"
#include "cvpcb.h"

#include "protos.h"

FILE *name_libmodules ;	 /* pour lecture librairie */

/* routines locales : */
static void ReadDocLib(const wxString & ModLibName );
static int LibCompare(void * mod1, void * mod2);
static STOREMOD * TriListeModules(STOREMOD * BaseListe, int nbitems);
/**/

/*********************/
int listlib(void)
/*********************/
/* Routine lisant la liste des librairies, et generant la liste chainee
 des modules disponibles
*/
{
char buffer[1024];
wxString FullLibName;
int errorlevel = 0, end;
int flag_librairie;
STOREMOD * ItemLib;
unsigned ii;
	
	if( BaseListePkg )	/* Liste Deja existante, a supprimer */
		{
		FreeMemoryModules(); BaseListePkg = NULL;
		}

	if ( g_LibName_List.GetCount() == 0 ) return -4;

	/* init recherche */
	SetRealLibraryPath("modules");
	nblib = 0;

	/* Lecture des Librairies */
	for( ii= 0 ; ii < g_LibName_List.GetCount(); ii++)
	{
		/* Calcul du nom complet de la librairie */
		FullLibName = MakeFileName(g_RealLibDirBuffer, g_LibName_List[ii], LibExtBuffer);
		/* acces a une librairie */
		if ((name_libmodules = fopen(FullLibName.GetData(),"rt"))  == NULL )
		{
			sprintf(cbuf,_("Library file <%s> not found"),FullLibName.GetData());
			DisplayError(NULL, cbuf, 20);
			continue;
		}

		/* Controle du type de la librairie : */
		flag_librairie = 0;
		fgets(buffer,32,name_libmodules) ;
		if( strncmp(buffer,ENTETE_LIBRAIRIE,L_ENTETE_LIB) != 0 )
			{
			sprintf(cbuf,_("Library file <%s> is not a module library"),
					FullLibName.GetData());
			DisplayError(NULL, cbuf, 20);
			fclose(name_libmodules); continue;
			}

		/* Lecture du nombre de composants */
		fseek(name_libmodules,0,0) ;

		 /* lecture nom des composants : */
		end = 0;
		while( !end && fgets(buffer,255,name_libmodules) != NULL )
			{
			if(strnicmp(buffer,"$INDEX",6) == 0 )
				{
				while( fgets(buffer,255,name_libmodules) != NULL )
					{
					if(strnicmp(buffer,"$EndINDEX",6) == 0 )
						{ end = 1; break; }
				
					ItemLib = (STOREMOD * ) MyZMalloc( sizeof(STOREMOD) );
					ItemLib->Pnext = BaseListePkg;
					BaseListePkg = ItemLib;
					ItemLib->Type = STRUCT_MODULE ;

					strncpy(ItemLib->Module,StrPurge(buffer),
											sizeof(ItemLib->Module)-1);
					strncpy(ItemLib->LibName,FullLibName.GetData(),
											sizeof(ItemLib->LibName)-1);

					nblib++;
					}
				if( !end ) errorlevel = -3;
				}
			}
		fclose(name_libmodules);
		ReadDocLib(FullLibName );
	}

	/* classement alphabetique: */
	if( BaseListePkg )
		BaseListePkg = TriListeModules(BaseListePkg, nblib);

	return(errorlevel) ;
}

/************************************************/
static int LibCompare(void * mod1, void * mod2)
/************************************************/
/*
routine compare() pour qsort() en classement alphabétique des modules
*/
{
int ii;
STOREMOD *pt1 , *pt2;

	pt1 = * ((STOREMOD**)mod1);
	pt2 = * ((STOREMOD**)mod2);

	ii = StrNumICmp( pt1->Module, pt2->Module );
	return(ii);
}



/********************************************************************/
static STOREMOD * TriListeModules(STOREMOD * BaseListe, int nbitems)
/********************************************************************/
/* Tri la liste des Modules par ordre alphabetique et met a jour
le nouveau chainage avant/arriere
	retourne un pointeur sur le 1er element de la liste
*/
{
STOREMOD ** bufferptr, * Item;
int ii, nb;

	if (nbitems <= 0 ) return(NULL);
	if ( BaseListe == NULL ) return(NULL);

	if (nbitems == 1 ) return(BaseListe);	// Tri inutile et impossible

	bufferptr = (STOREMOD**)MyZMalloc( (nbitems+3) * sizeof(STOREMOD*) );

	for( ii = 1, nb = 0, Item = BaseListe; Item != NULL; Item = Item->Pnext, ii++)
		{
		nb++;
		bufferptr[ii] = Item;
		}

	/* ici bufferptr[0] = NULL et bufferptr[nbitem+1] = NULL et ces 2 valeurs
	representent le chainage arriere du 1er element ( = NULL),
	et le chainage avant du dernier element ( = NULL ) */

	qsort(bufferptr+1,nb,sizeof(STOREMOD*),
							(int(*)(const void*,const void*))LibCompare) ;

	/* Mise a jour du chainage */
	for( ii = 1; ii <= nb; ii++ )
		{
		Item = bufferptr[ii];
		Item->Num = ii;
		Item->Pnext = bufferptr[ii+1];
		Item->Pback = bufferptr[ii-1];
		}

	Item = bufferptr[1];
	MyFree(bufferptr);

	return(Item);
}



/***************************************************/
static void ReadDocLib(const wxString & ModLibName )
/***************************************************/
/* Routine de lecture du fichier Doc associe a la librairie ModLibName.
	Cree en memoire la chaine liste des docs pointee par MList
	ModLibName = full file Name de la librairie Modules
*/
{
STOREMOD * NewMod;
char Line[1024], *Name;
wxString docfilename;
FILE * LibDoc;

	docfilename = ModLibName;
	ChangeFileNameExt(docfilename, EXT_DOC);

	if( (LibDoc = fopen(docfilename.GetData(),"rt")) == NULL ) return;

	GetLine(LibDoc, Line, NULL, sizeof(Line) -1);
	if(strnicmp( Line,ENTETE_LIBDOC, L_ENTETE_LIB) != 0) return;

	/* Lecture de la librairie */
	while( GetLine(LibDoc,Line, NULL, sizeof(Line) -1) )
		{
		NewMod = NULL;
		if( Line[0] != '$' ) continue;
		if( Line[1] == 'E' ) break;;
		if( Line[1] == 'M' )	/* Debut decription 1 module */
			{
			while( GetLine(LibDoc,Line, NULL, sizeof(Line) -1) )
				{
				if( Line[0] ==  '$' )	/* $EndMODULE */
						break;
				switch( Line[0] )
					{
					case 'L':	/* LibName */
						Name = StrPurge(Line+3);
						NewMod = BaseListePkg;
						while ( NewMod )
							{
							if( strcmp(Name, NewMod->Module ) == 0 ) break;
							NewMod = NewMod->Pnext;
							}
						break;

					case 'K':	/* KeyWords */
						if( NewMod && (! NewMod->KeyWord) )
							NewMod->KeyWord = strdup(StrPurge(Line+3) );
						break;

					case 'C':	/* Doc */
						if( NewMod && (! NewMod->Doc ) )
							NewMod->Doc = strdup(StrPurge(Line+3) );
						break;
					}
				}
			} /* lecture 1 descr module */
		}	/* Fin lecture librairie */
	fclose(LibDoc);
}


