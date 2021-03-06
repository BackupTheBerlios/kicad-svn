	/************/
	/* rdorcad()*/
	/************/

/* convertit la netliste ORCADPCB en netliste ORCADPCB (fichier temporaire)
assure la r�affectation des alimentations selon le format :
( XXXXXX VALEUR|(pin1,pin2,...=newalim) ID VALEUR
*/

#include "fctsys.h"

#include "wxstruct.h"
#include "common.h"
#include "cvpcb.h"

#include "protos.h"

#define SEPARATEUR '|'  /* caractere separateur dans netliste */

/* routines locales : */

int pin_orcad(STORECMP * CurrentCmp);


	/****************************************/
	/* int WinEDA_CvpcbFrame::rdorcad(void) */
	/****************************************/

int WinEDA_CvpcbFrame::rdorcad(void)
{
int i , j , k ,l ;
char * LibName;
char Line[1024];
int FlagEESchema = 0;
char label[80] ;		/* buffer des references composants */
char ref_schema[80] ;	/* buffer de la ref schematique */
char val[80] ;		 /* buffer des valeurs/ref.lib */
char postval[80] ;	/* buffer de la valeur de fin de ligne (vraie valeur) */
char *ptchar ;		/* pointeur de service */
STORECMP * Cmp;

	modified = 0;
	Rjustify = 0;

	/* Raz buffer et variable de gestion */
	if( BaseListeCmp ) FreeMemoryComponants();

	/* Ouverture du fichier source */
	source = fopen(FFileName.GetData(),"rt");
	if (source == 0)
		 {
		 sprintf(cbuf,_("File <%s> not found"),FFileName.GetData());
		 DisplayError(this, cbuf); return(-1);
		 }

	/* Lecture entete qui doit etre "( { OrCAD PCB"*/
	/* ou "# EESchema Netliste"*/
	fgets(Line,255,source) ;
	i =  strnicmp(Line,"( { ",4) ;	/* net type PCB2 */
	if( i != 0 )
		{
		i =  strnicmp(Line,"# EESchema",7) ;	/* net type EESchema */
		if( i == 0 ) FlagEESchema = 1;
		}

	if ( i != 0 )
		{
		sprintf(cbuf,"Unknown file format [%s]",Line) ;
		wxMessageBox(cbuf,"");
		fclose(source);
		return(-3) ;
		}

	SetStatusText( _("Netlist Format: EESchema"), 0);


	/* Lecture de la liste */
	for (;;)
		{
		/* recherche du debut de la description d'un composant */

		if( fgets(Line,80,source)  == 0 ) break;

		/* suppression des blancs en d�but de ligne */
		i = 0 ; while (Line[i] == ' ') i++ ;

		/* elimination des lignes vides : */
		if (Line[i] < ' ') continue ;

		if (strnicmp(&Line[i],"( ",2) != 0) continue ;

		/****************************/
		/* debut description trouv� */
		/****************************/
		/* memo ident schema */
		while ( Line[i] != ' ') i++ ;
		while ( Line[i] == ' ') i++ ; /* i pointe 1er caractere de l'ident schema */

		j = 0 ; while ( Line[i] != ' ') ref_schema[j++] = Line[i++] ;
		ref_schema[j] = 0 ;

		/* recherche val/ref.lib */
		while ( Line[i] == ' ') i++ ; /* i pointe la valeur du composant */
		LibName = Line + i;

		memset(label, 0, sizeof(label));
		memset(val, 0, sizeof(val) ) ;
		memset(postval, 0, sizeof(postval) ) ;
		memset(alim, 0, sizeof(alim) ) ;

		/* lecture valeur du composant */

		/* recherche fin de valeur (' ') */
		ptchar = strstr(&Line[i]," ") ;
		if (ptchar == 0)
			{
			sprintf(cbuf, _("Netlist error: %s"),Line) ;
			DisplayError(NULL, cbuf);
			k = 0 ;
			}
		else k = ptchar - Line ;

		for (j = 0 ; i < k ;  i++)
			 {
			 if ( Line[i] == SEPARATEUR ) break ;
			 if ( j < 8 ) val[j++] = Line[i] ;
			 }

		if ( (Line[++i] == '(') && (Line[k-1] == ')' ) )
			{
			i++ ; l = 0 ; while ( k-1 > i ) alim[l++] = Line[i++] ;
			}

		else	i = k ;

		/* recherche reference du composant */
		while(Line[i] != ' ') i++ ; /* elimination fin valeur */
		while(Line[i] == ' ') i++ ; /* recherche debut reference */

		/* debut reference trouv� */
		for ( k = 0 ; k < 8 ; i++ , k++ )
			{
			if ( Line[i] <= ' ' ) break ;
			label[k] = Line[i] ;
			}

		/* recherche vraie valeur du composant */
		while(Line[i] != ' ') i++ ; /* elimination fin reference */
		while(Line[i] == ' ') i++ ; /* recherche debut vraie valeur */

		/* debut vraie valeur trouv�e */
		for ( k = 0 ; k < 16 ; i++ , k++ )
			{
			if ( Line[i] <= ' ' ) break ;
			postval[k] = Line[i] ;
			}


		/* classement du composant ,suivi de sa valeur */
		Cmp = (STORECMP*) MyZMalloc( sizeof(STORECMP) );
		Cmp->Type = STRUCT_COMPONANT;
		Cmp->Pnext = BaseListeCmp;
		BaseListeCmp = Cmp;
		strncpy( Cmp->Reference, label ,sizeof(Cmp->Reference) -1 );
		strncpy(Cmp->Valeur, postval, sizeof(Cmp->Valeur) -1 ) ;

		if( FlagEESchema )	/* Copie du nom module: */
			{
			if( strnicmp(LibName, "$noname", 7 ) != 0 )
				{
				i = 0;
				while( *LibName > ' ' )
					{
					Cmp->Module[i] = *LibName;
					i++; LibName++;
					if( i >= ((int)sizeof(Cmp->Module) -1) ) break;
					}
				}
			}
		/* classement du TimeStamp */
		memcpy(	Cmp->TimeStamp, ref_schema, 8 );

		pin_orcad( Cmp) ;

		nbcomp++ ;
		}
	fclose(source);

	/* reclassement alpab�tique : */
	BaseListeCmp = TriListeComposantss( BaseListeCmp, nbcomp);

	return(0);
}

		/***********************************/
		/* pin() : analyse liste des pins */
		/***********************************/

int pin_orcad(STORECMP * Cmp)
{
int i , jj;
char numpin[9] , net[1024] ;
char Line[1024];
STOREPIN * Pin = NULL;
STOREPIN ** LastPin = & Cmp->Pins;

for ( ;; )
	{
	/* debut description trouv� */
	for ( ;; )
		{
		if ( fgets(Line,80,source) == 0 ) return(-1) ;

		/* suppression des blancs en d�but de ligne */
		i = 0 ; while (Line[i] == ' ') i++ ;
		while (Line[i] == '(') i++ ;
		while (Line[i] == ' ') i++ ;

		/* elimination des lignes vides : */
		if (Line[i] < ' ') continue ;

		/* fin de description ? */
		if (Line[i] == ')' ) return(0) ;

		memset( net, 0, sizeof(net) );
		memset( numpin, 0, sizeof(numpin) );

		/* lecture name pin , 4 lettres */
		for (jj = 0 ; jj < 4 ; jj++ , i++)
			{
			if ( Line[i] == ' ' ) break ;
			numpin[jj] = Line[i] ;
			}

		/* recherche affectation forc�e de net  */
		if ( reaffect(numpin,net) != 0)
			{
			Pin = (STOREPIN *)MyZMalloc( sizeof(STOREPIN) );
			Pin->Type = STRUCT_PIN;
			*LastPin = Pin; LastPin = &Pin->Pnext;
			strncpy( Pin->PinNum, numpin, sizeof(Pin->PinNum) -1 );
			Pin->PinNet = strdup(net);
			continue ;
			}

		/* recherche netname */
		while(Line[i] == ' ') i++ ; /* recherche debut reference */

		/* debut netname trouv� */
		for ( jj = 0 ; jj < (int)sizeof(net)-1 ; i++ , jj++ )
			{
			if ( Line[i] <= ' ' ) break ;
			net[jj] = Line[i] ;
			}

		Pin = (STOREPIN *)MyZMalloc( sizeof(STOREPIN) );
		Pin->Type = STRUCT_PIN;
		*LastPin = Pin; LastPin = &Pin->Pnext;
		strncpy( Pin->PinNum, numpin, sizeof(Pin->PinNum) -1 );
		Pin->PinNet = strdup(net);
		}
	}
}


	/***********************************************************/
	/* void TriListeComposants(STOREMOD * BaseListe, int nbitems) */
	/***********************************************************/

/* Tri la liste des composants par ordre alphabetique et me a jour
le nouveau chainage avant/arriere
	retourne un pointeur sur le 1er element de la liste
*/

STORECMP * TriListeComposantss(STORECMP * BaseListe, int nbitems)
{
STORECMP ** bufferptr, * Item;
int ii;

	if (nbitems <= 0 ) return(NULL);
	bufferptr = (STORECMP**)MyZMalloc( (nbitems+2) * sizeof(STORECMP*) );

	for( ii= 1, Item = BaseListe; Item != NULL; Item = Item->Pnext, ii++)
		{
		bufferptr[ii] = Item;
		}

	/* ici bufferptr[0] = NULL et bufferptr[nbitem+1] = NULL et ces 2 valeurs
	representent le chainage arriere du 1er element, et le chainage avant
	du dernier element */

	qsort(bufferptr+1,nbitems,sizeof(STORECMP*),
							(int(*)(const void*,const void*))CmpCompare) ;
	/* Mise a jour du chainage */
	for( ii = 1; ii <= nbitems; ii++ )
		{
		Item = bufferptr[ii];
		Item->Num = ii;
		Item->Pnext = bufferptr[ii+1];
		Item->Pback = bufferptr[ii-1];
		}
	return(bufferptr[1]);
}

	/***************************************************/
	/* int CmpCompare(void *mod1, void *mod2) */
	/***************************************************/

/*
routine compare() pour qsort() en classement alphab�tique des composants
*/

int CmpCompare(void * mod1, void * mod2)
{
int ii;
STORECMP *pt1 , *pt2 ;

	pt1 = * ((STORECMP**)mod1);
	pt2 = * ((STORECMP**)mod2);

	ii = StrNumICmp( pt1->Reference, pt2->Reference );
	return(ii);
}



