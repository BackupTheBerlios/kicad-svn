	/********************************************************/
	/* Traitement des netlistes VIEWLOGIC , Format WIRELIST */
	/********************************************************/

/* Traite la netliste VIEWLOGIC au format WIRELIST
*/

#include "fctsys.h"

#include "common.h"
#include "cvpcb.h"

#include "protos.h"

/* routines locales : */
static int ReadVLDescrCmp( char * Line, STORECMP * Cmp, int Type);
static int ReadReelNumPin( char * Line, STORECMP * Cmp, int Type);
static int RegroupeUnitsComposant( STORECMP * BaseCmp );
static STORECMP *TraitePseudoCmp(char * Text, STORECMP *PseudoCmp, int Header);
static void MergePseudoCmp(STORECMP * BaseCmp, STORECMP * BasePseudoCmp);

/* Variables Locales */
STORECMP * BasePseudoCmp;	/* Stockage des descriptions generales */

/* Identificateurs de debut de ligne */
#define API ( (('A'&255) << 16) | (('P'&255) << 8 ) | ('I'&255) )
#define AP_ ( (('A'&255) << 8) | ('P'&255) )
#define AS_ ( (('A'&255) << 8) | ('S'&255) )
#define A__ ('A' & 255)
#define W__ ('W' & 255)
#define M__ ('M' & 255)
#define I__ ('I' & 255)


/************************************************/
int WinEDA_CvpcbFrame::ReadViewlogicWirList(void)
/************************************************/
{
int ii, Type = 0, error, Header;
char RefDes[40], val[40], LocalRef[40], Generic[40] ;
char Line[1024], *Text;
STOREPIN * Pin;
STORECMP * Cmp, *NextCmp;
STORECMP * PseudoCmp = NULL;
wxString msg;
	
	modified = 0;
	Rjustify = 1;

	/* Raz buffer et variable de gestion */
	if( BaseListeCmp ) FreeMemoryComponants();

	Cmp = NULL;

	/* Ouverture du fichier source */
	msg = _("Netlist file ") + FFileName;
	SetStatusText(msg,0);

	source = fopen(FFileName.GetData(),"rt");
	if (source == 0)
		{
		msg = _("File not found ") + FFileName;
		DisplayError(this, msg); return(-1);
		}

	/* Lecture entete qui doit etre "| Wirelist ..*/
	fgets(Line,1024,source) ;
	ii = strncmp(Line,"| Wirelist",3) ;	 /* net type Viewlogic */

	if ( ii != 0 )
		{
		sprintf(cbuf,"format inconnu <%s>",Line) ;
		DisplayError(this, cbuf);
		fclose(source); return(-3) ;
		}

	SetStatusText("Format Netliste ViewLogic wirelist", 0);

	/* Lecture de la liste */
	for ( ;; )
		{
		if ( fgets(Line,1024,source) == 0 )  break ;
		Text = StrPurge(Line);

		Header = *Text & 255;	/* Header est la copie du 1er mot de la ligne */
		for( ii = 1; ii < 3 ; ii++ )
			{
			if( Text[ii] <= ' ' ) break;
			Header <<= 8; Header |= Text[ii] & 255;
			}

		switch( Header )
			{
			case AP_ :
			case AS_ :
				PseudoCmp = TraitePseudoCmp(Text, PseudoCmp, Header);
				Type = 0;
				break;

			case API :
				if( (nbcomp <= 0 ) || (Cmp == NULL) )
					{
					DisplayError(NULL, "Description API inattendue", 20);
					break;
					}
				error = ReadReelNumPin( Text, Cmp, Type );
				if( error < 0 )
					{
					sprintf(Line,"Erreur %d ligne API", -error );
					DisplayError(NULL, Line,10 );
					}
				break;

			case I__ :	/* Lecture descr 1 composant */
				if( nbcomp <= 0 )
					{
					DisplayError(NULL, "Description Composant inattendue", 20);
					break;
					}
				*RefDes = 0; *val = 0;
				ReadVLDescrCmp( Text, Cmp, Type);
				Type = 1;	/* pour lecture num pins */
				break;

			case W__ :
			case M__ :
				Cmp = (STORECMP*) MyZMalloc( sizeof(STORECMP) );
				Cmp->Type = STRUCT_COMPONANT;
				Cmp->Pnext = BaseListeCmp;
				BaseListeCmp = Cmp;
				nbcomp++ ;
				Type = Header;
				Text = strtok(NULL, " \t\n\r");
				if( Text == NULL ) break;
				strncpy(Generic, Text, 40);
				Text = strtok(NULL, " \n\r");
				if( Text == NULL ) break;
				strncpy(LocalRef, Text, 40);
				break;

			default:
				Type = 0;
				break;
			}
		}

	fclose(source);

	/* reclassement alpab‚tique : */
	BaseListeCmp = TriListeComposantss( BaseListeCmp, nbcomp);
	nbcomp -= RegroupeUnitsComposant( BaseListeCmp );

	/* Addition des renseignements issus des pseudo composants */
	MergePseudoCmp( BaseListeCmp, BasePseudoCmp);

	/* Renumerotation des composants */
	Cmp = BaseListeCmp;
	for( ii = 1; Cmp != NULL; ii++, Cmp = Cmp->Pnext)
		{
		Cmp->Num = ii;
		}

	/* Liberation memoire */
	Cmp = BasePseudoCmp;
	for( ; Cmp != NULL; Cmp = NextCmp )
		{
		STOREPIN * NextPin;
		for( Pin = Cmp->Pins; Pin != NULL; Pin = NextPin )
			{
			NextPin = Pin->Pnext;
			if( Pin->PinNet ) MyFree(Pin->PinNet);
			MyFree(Pin);
			}
		NextCmp = Cmp->Pnext; MyFree(Cmp);
		}
	BasePseudoCmp = NULL;

	return(0);
}


/****************************************************************/
static int ReadVLDescrCmp( char * Line, STORECMP * Cmp, int Type)
/****************************************************************/
/* Lecture de la description d'un composant
	(ligne commencant par I ...)
*/
{
char * Text, *Ident;
int nbpins = 0, ii;
char numpin[9];
STOREPIN * Pin = NULL;
STOREPIN ** LastPin = & Cmp->Pins;

	Text = strtok(Line, " \n\t\r`");

	Text = strtok(NULL, " \n\t\r`");	/* Text pointe 1er mot utile */

	Ident = strtok(NULL, " \n\t\r`");	/* Ident pointe identificateur */
	strncpy(Cmp->Repere, Ident, sizeof(Cmp->Repere) -1) ;

	while ( Text )
		{
		Text = strtok(NULL, " \t\n\r`");
		if( Text == NULL ) break;

		if( strncmp(Text, "VALUE=" ,6) == 0 )
			{
			strncpy(Cmp->Valeur, Text+6,
							sizeof(Cmp->Valeur) -1);
			continue;
			}

		if( strncmp(Text, "REFDES=",7 ) == 0 )
			{
			strncpy( Cmp->Reference, Text+7 ,
						sizeof(Cmp->Reference) -1);
			ii = strlen(Cmp->Reference) -1 ;
			if( !isdigit(Cmp->Reference[ii] ) )
				Cmp->Reference[ii] = 0;
			continue;
			}

		/* Lecture d'un net pin */
		nbpins++;
		Pin = (STOREPIN *)MyZMalloc( sizeof(STOREPIN) );
		Pin->Type = STRUCT_PIN;
		*LastPin = Pin; LastPin = &Pin->Pnext;
		sprintf(numpin,"%d", nbpins);
		Pin->index = nbpins;
		strncpy( Pin->PinNum, numpin, sizeof(Pin->PinNum) -1 );
		Pin->PinNet = strdup(Text);
		}

	if( Cmp->Valeur[0] == 0)
		{
		strncpy(Cmp->Valeur, Ident, 16);
		}


	/* Mise en place du TimeStamp init a 0 */
	memset(Cmp->TimeStamp, '0', 8 );

	/* Analyse du type */
	switch( Type )
		{
		case M__: return(0);
		case W__: return(0);
		default: break;
		}
	return(-1);
}

	/*********************************************************************/
	/* int ReadReelNumPin( char * Line, STORECMP * Cmp, int Type) */
	/*********************************************************************/

/* Lit les lignes commencant par "API" pour recuperer le vrai numero de pin
*/
static int ReadReelNumPin( char * Line, STORECMP * Cmp, int Type)
{
char * Text, *Ident;
int numpin;
STOREPIN * Pin;


	if( Type != 1 ) return (-1);

	Text = strtok(Line, " \n\t\r");

	Ident = strtok(NULL, " \n\t\r");	/* Ident pointe identificateur */

	Text = strtok(NULL, " \n\t\r");		/* Text pointe type composant */

	Text = strtok(NULL, " \n\t\r");		/* pointe numero d'ordre */
	if( Text == NULL ) return(-2);;

	numpin = atoi(Text);

	Text = strtok(NULL, " \n\t\r");		/* pointe Reel NumPin */
	if( Text == NULL ) return(-3);

	if( strncmp(Text, "#=" ,2) ) return (-4);

	for(Pin = Cmp->Pins ; Pin != NULL; Pin = Pin->Pnext )
		{
		if( Pin->Type != STRUCT_PIN ) return(-5);
		if( Pin->index != numpin ) continue;
		strncpy( Pin->PinNum, Text+2, sizeof(Pin->PinNum) -1 );
		return(0);
		}
	return(-6 );
}


	/***********************************************/
	/* int RegroupeComposant( STORECMP * BaseCmp ) */
	/***********************************************/

/* Regroupe les pins des differentes Unites d'un meme composant
	et modifie le chainage
*/
static int RegroupeUnitsComposant( STORECMP * BaseCmp )
{
STORECMP * Cmp = BaseCmp;
STORECMP * NextCmp = Cmp->Pnext;
STOREPIN * Pin;
int Deleted = 0;


	if( NextCmp == NULL ) return(0);

	for( ; NextCmp != NULL; Cmp = NextCmp, NextCmp = NextCmp->Pnext )
		{
		if( strcmp(Cmp->Reference,NextCmp->Reference) ) continue;
		/* 2 composants identiques : Pins a regrouper */
		Deleted++;
		Pin = Cmp->Pins;
		if( Pin == NULL ) Cmp->Pins = NextCmp->Pins;
		else
			{
			while(Pin->Pnext) Pin = Pin->Pnext;
			Pin->Pnext = NextCmp->Pins;
			}
		NextCmp->Pins = NULL;
		Cmp->Pnext = NextCmp->Pnext;
		(Cmp->Pnext)->Pback = Cmp;
		MyFree( NextCmp);
		NextCmp = Cmp;
		}
	return(Deleted);
}



/****************************************************************************/
/* STORECMP * TraitePseudoCmp(char * Text, STORECMP *PseudoCmp, int Header) */
/****************************************************************************/

static STORECMP *TraitePseudoCmp(char * Line, STORECMP *PseudoCmp, int Header)
{
STORECMP * Cmp = PseudoCmp;
STOREPIN * Pin;
char Name[256], *Text;

	Text = strtok(Line," \t\n\r");
	Text = strtok(NULL," \t\n\r");  /* Pointe Name */
	strcpy(Name, Text);
	Text = strtok(NULL," \t\n\r");  /* Pointe partie utile */

	if( Cmp == NULL )
		{
		Cmp = BasePseudoCmp = (STORECMP*) MyZMalloc( sizeof(STORECMP) );
		Cmp->Type = STRUCT_COMPONANT;
		strncpy( Cmp->Repere, Name, sizeof(Cmp->Repere) - 1);
		strncpy( Cmp->Valeur, Name, sizeof(Cmp->Valeur) - 1);
		}

	else if( strcmp(Name, Cmp->Valeur) )	/* Nouveau pseudo composant */
		{
		Cmp = (STORECMP*) MyZMalloc( sizeof(STORECMP) );
		PseudoCmp->Pnext = Cmp;
		Cmp->Type = STRUCT_COMPONANT;
		strncpy( Cmp->Valeur, Name, sizeof(Cmp->Valeur) - 1);
		strncpy( Cmp->Repere, Name, sizeof(Cmp->Repere) - 1);
		}

	switch ( Header )
		{
		case AS_ :
			if( strnicmp(Text,"PKG_TYPE=",9) == 0 )
				{
				strncpy(Cmp->Module,Text+9,sizeof(Cmp->Module) -1 );
				break;
				}
			if( strnicmp(Text,"PARTS =",7) == 0 )
				{
				Cmp->Multi = atoi(Text+7);
				break;
				}
			if( strnicmp(Text,"REFDES=",7) == 0 )
				{
				strncpy(Cmp->Reference,Text+7,sizeof(Cmp->Reference) -1 );
				break;
				}
			if( strnicmp(Text,"SIGNAL=",7) == 0 )
				{
				Text = strtok(Text," ;=\t\n\r");
				Text = strtok(NULL," ;=\t\n\r");  /* Pointe partie utile */
				Pin = (STOREPIN*) MyZMalloc( sizeof(STOREPIN) );
				Pin->Type = STRUCT_PIN;
				Pin->Pnext = Cmp->Pins;
				Cmp->Pins = Pin;
				Pin->PinNet = strdup(Text);
				Text = strtok(NULL," ;=\t\n\r");  /* Pointe partie utile */
				strncpy(Pin->PinNum,Text,sizeof(Pin->PinNum) -1 );
				break;
				}
			break;

		case AP_ :
			break;

		}	

	return(Cmp);
}


	/*********************************************************************/
	/* void MergePseudoCmp(STORECMP * BaseCmp, STORECMP * BasePseudoCmp) */
	/*********************************************************************/

/* Additionne aux composants standards les renseignements contenus
dans les descriptions generales
*/
static void MergePseudoCmp(STORECMP * BaseCmp, STORECMP * BasePseudoCmp)
{
STORECMP * Cmp, * PseudoCmp;
STOREPIN * Pin, * PseudoPin;

	Cmp = BaseCmp;
	for( ; Cmp != NULL; Cmp = Cmp->Pnext)
		{
		PseudoCmp = BasePseudoCmp;
		for( ; PseudoCmp != NULL; PseudoCmp = PseudoCmp->Pnext)
			{
			if( strcmp(Cmp->Repere, PseudoCmp->Repere) ) continue;
			/* Description trouvee, transfert des infos */
			Cmp->Multi = PseudoCmp->Multi;
			PseudoPin = PseudoCmp->Pins;
			for ( ; PseudoPin != NULL; PseudoPin = PseudoPin->Pnext)
				{
				Pin = (STOREPIN*) MyMalloc(sizeof(STOREPIN) );
				*Pin = *PseudoPin;
				if (PseudoPin->PinNet)
					 Pin->PinNet = strdup(PseudoPin->PinNet);
				Pin->Pnext = Cmp->Pins; Cmp->Pins = Pin;
				}
			break;
			}
		}
}

