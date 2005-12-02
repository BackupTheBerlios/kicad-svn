	/*************************************************************/
	/* Traitement des netlistes VIEWLOGIC , Format NETLISt + PKG */
	/*************************************************************/

/* Traite la netliste VIEWLOGIC au format .net, avec fichiers .pkg
format .pkg
ex:
valeur;package;liste des composants
74LS00;14PDIP;U8,U9
74LS04;14PDIP;U2
CAP_NPOL;;CP1,CP2,CP3,CP4,CP5,CP6,CP7
RVERT;R_250MW;R11,R12,R13,R14,R15,R16,R17,R21,R22,R23,R24,R25,R26,R27,R31,R32,
 R33,R34,R35,R36,R37
..

format .net
netname;cmp^pin,...
ex:
$1I1\$1N2;U2^8,U3^10
$1I1\$1N6;U3^9,U6^9
$1N30;J2_8^2,Q1^2,Q2^2,Q3^2,Q4^2,Q5^2,Q6^2,Q7^2,R21^2,R22^2,R23^2,R24^2,R25^2,
 R26^2,R27^2,RP1^1,RP2^1,RP3^1,RP4^1,RP5^1,RP6^1,RP7^1
*/

#include "fctsys.h"

#include "wxstruct.h"
#include "common.h"
#include "cvpcb.h"
#include "protos.h"

/* routines locales : */
static void GenPin( STORECMP *BaseCmp, char *CmpName, char *PinNum, char *PinNet);
static int GenListeComposants(FILE * PkgFile);

/* Variables Locales */

	/******************************/
	/* int ReadViewlogicNet(void) */
	/******************************/

int WinEDA_CvpcbFrame::ReadViewlogicNetList(void)
{
int ii, LineNum;
char Line[1024], *Text;
wxString PkgFileName;
char PinName[256], NetName[256], RefName[256], *Data;
STORECMP * Cmp;
wxString msg;

	modified = 0;
	Rjustify = 1;

	/* Raz buffer et variable de gestion */
	if( BaseListeCmp ) FreeMemoryComponants();

	Cmp = NULL; LineNum = 0;
	memset(Line, 0, sizeof(Line) );

	/* Tst de la presence du fichier principal Netliste, et son format:
		Si format = PCBNEW, appel de rdorcad
	*/
	source = fopen(FFileName.GetData(),"rt");
	if (source == 0)
		 {
		 msg = _("File not found ") + FFileName;
		 DisplayError(this, msg); return(-1);
		 }

	if ( fgets(Line, sizeof(Line)-1, source) == 0 )
		{	/* fichier vide */
		fclose(source);	return(-1);
		}

	fclose(source);
	Text = StrPurge(Line);

	ii = strnicmp(Line,"( { ",4) ;	 /* net type PCB2 */
	if( ii != 0 ) ii =  strnicmp(Line,"# EESchema",4) ;	  /* net type EESchema */
	if( ii == 0 )
		{
		ii = rdorcad(); return(ii);
		}


	/* Traitement reel de la netliste Viewlogic ( .net et .pkg ) */
	SetStatusText( _("Format Netlist: ViewLogic net&pkg"), 0);

	/* Calcul du nom (full file name) du fichier .pkg */
	PkgFileName = FFileName;
	ChangeFileNameExt(PkgFileName, PkgInExtBuffer);

	/* Ouverture du fichier .pkg */
	source = fopen(PkgFileName.GetData(),"rt");
	if (source == 0)
	{
		msg = _("File not found ") + PkgFileName;
		DisplayError(this, msg);
		return(-1);
	}

	nbcomp = GenListeComposants(source);
	fclose(source);

	/* reclassement alpab‚tique : */
	BaseListeCmp = TriListeComposantss( BaseListeCmp, nbcomp);

	/* Ouverture du fichier netliste */
	source = fopen(FFileName.GetData(),"rt");
	if (source == 0)
	{
		msg = _("File not found ") + FFileName;
		DisplayError(this, msg);
		return(-1);
	}

	/* Lecture de la liste ( fichier netliste ) */
	LineNum = 0;
	for ( ;; )
		{
		LineNum++;
		if ( fgets(Line, sizeof(Line)-1, source) == 0 )  break ;
		Text = StrPurge(Line);
		if (*Text < ' ' ) continue; /* Ligne vide */

		/* Lecture du NetName */
		for( ii = 0; ii < 80; ii++, Text++)
			{
			if( *Text == 0 ) break;
			if( *Text == ';' ) break;
			NetName[ii] = *Text;
			}
		NetName[ii] = 0; if ( *Text == ';' ) Text++;
		if( NetName[0] == 0 )
			{
			sprintf(cbuf,"Err. Pin Name ligne %s",Line);
			DisplayError(this, cbuf, 20);
			}

		/* Lecture des attributions de pins */
		while( *Text != 0 )
			{
			Data = RefName; RefName[0] = PinName[0] = 0;
			for( ii = 0; ii < 1000; ii++, Text++ )
				{
				if( *Text == 0 ) break;
				if( *Text == ',' ) break;
				if( *Text == '^' )
					{
					*Data = 0; Data = PinName; continue;
					}
				*Data = *Text; Data++;
				}
			*Data = 0;
			if( (PinName[0] == 0 ) || (PinName[0] == 0 ) )
				{
				sprintf(cbuf,"Err. Pin Name ligne %s",Line);
				DisplayError(this, cbuf, 20); break;
				}
			GenPin( BaseListeCmp, RefName, PinName, NetName);
			if ( *Text == ',' ) /* Autre element a traiter, ou nouvelle */
								/* ligne a lire ( continuation de ligne ) */
				{
				Text++; Text = StrPurge(Text);
				if( *Text == 0 )	/* Nouvelle ligne a lire */
					{
					LineNum++;
					if ( fgets(Line, sizeof(Line)-1, source) == 0 )  break ;
					Text = StrPurge(Line);
					}
				}
			}
		}

	fclose(source);

	return(0);
}



/******************************************************************************/
static void GenPin( STORECMP *BaseCmp, char * CmpName,
			char *PinNum, char *PinNet)
/******************************************************************************/

/* Routine de creation d'une pin pour le composant de ref CmpName.
	la pin a pour "numero" PinNum, et pour Net PinNet
*/
{
STORECMP *Cmp;
STOREPIN * Pin;

	/* Recherche du composant */
	Cmp = BaseCmp;
	for( ; Cmp != NULL; Cmp = Cmp->Pnext )
		{
		if( stricmp(Cmp->Reference, CmpName) == 0 ) break;
		}

	if( Cmp == NULL )
		{
		sprintf( cbuf, _("Component [%s] not found in .pkg file"), CmpName);
		DisplayError(NULL, cbuf, 1);
		return;
		}

	/* Creation de la Pin */
	Pin = (STOREPIN *)MyZMalloc( sizeof(STOREPIN) );
	Pin->Type = STRUCT_PIN;
	Pin->Pnext = Cmp->Pins; Cmp->Pins = Pin;
	strncpy( Pin->PinNum, PinNum, sizeof(Pin->PinNum) -1 );
	Pin->PinNet = strdup(PinNet);
}


	/******************************************/
	/* int GenListeComposants(FILE * PkgFile) */
	/******************************************/

/* Cree la liste des composants cites dans le fichier .pkg
	Retourne le nombre de composants
*/

static int GenListeComposants(FILE * PkgFile)
{
int ii, LineNum, NbComp;
char Line[1024], *Text;
char Valeur[256], Package[256], Name[256];
STORECMP * Cmp;

	LineNum = 0; NbComp = 0;

	/* Lecture de la liste ( fichier .pkg ) */
	for ( ;; )
		{
		LineNum++;
		if ( fgets(Line, sizeof(Line)-1, PkgFile) == 0 )  break ;
		Text = StrPurge(Line);
		if (*Text < ' ' ) continue; /* Ligne vide */

		/* Lecture de la Valeur */
		for( ii = 0; ii < 80; ii++, Text++)
			{
			if( *Text == 0 ) break;
			if( *Text == ';' ) break;
			Valeur[ii] = *Text;
			}
		Valeur[ii] = 0; if ( *Text == ';' ) Text++;

		/* Lecture du type du boitier */
		for( ii = 0; ii < 80; ii++, Text++)
			{
			if( *Text == 0 ) break;
			if( *Text == ';' ) break;
			Package[ii] = *Text;
			}
		Package[ii] = 0; if ( *Text == ';' ) Text++;

		/* Lecture des composants */
		while( *Text )
			{
			/* Lecture du nom du composant */
			for( ii = 0; ii < 80; ii++, Text++)
				{
				if( *Text <= ' ' ) break;
				if( *Text == ',' ) break;
				Name[ii] = *Text;
				}
			Name[ii] = 0;
			Cmp = (STORECMP*) MyZMalloc( sizeof(STORECMP) );
			Cmp->Type = STRUCT_COMPONANT;
			Cmp->Pnext = BaseListeCmp;
			BaseListeCmp = Cmp;
			NbComp++ ;
			strncpy( Cmp->Reference, StrPurge(Name), sizeof(Cmp->Reference) -1);
			strncpy(Cmp->Valeur, StrPurge(Valeur), sizeof(Cmp->Valeur) -1);
			strncpy(Cmp->Module, StrPurge(Package), sizeof(Cmp->Valeur) -1);
			memset(Cmp->TimeStamp,'0', 8);

			if ( *Text == ',' ) /* Autre element a traiter, ou nouvelle */
								/* ligne a lire ( continuation de ligne ) */
				{
				Text++; Text = StrPurge(Text);
				if( *Text == 0 )	/* Nouvelle ligne a lire */
					{
					LineNum++;
					if ( fgets(Line, sizeof(Line)-1, PkgFile) == 0 )  break ;
					Text = StrPurge(Line);
					}
				}
			}
		}
	return(NbComp);
}
