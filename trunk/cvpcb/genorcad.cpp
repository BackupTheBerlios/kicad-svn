	/***************/
	/* genorcad()  */
	/***************/

/*
ComplŠte la netliste (*.NET) en y placant les ref *.lib FORMAT ORCADPCB
La valeur (Part Value) est tronquee a 16 lettres
*/

#include "fctsys.h"

#include "common.h"
#include "cvpcb.h"

#include "protos.h"

#define MAX_LEN_NETNAME 16

/* Routines locales */
static void TriPinsModule( STORECMP * CurrentCmp );
static int PinCompare(const void *cmp1, const void *cmp2);
static void ChangePinNet( char * PinNet );

/* Variables Locales */
int NetNumCode;			/* Nombre utilise pour cree des NetNames lors de
						reaffectation de NetNames */

int genorcad(void)
{
char Line[1024];
STOREPIN * Pin;
STORECMP * CurrentCmp;

	NetNumCode = 1; DateAndTime(Line);
	fprintf(dest,"( { Netlist by %s, date = %s }\n",
		Main_Title.GetData(), Line ) ;

	  /***********************/
	  /* Lecture de la liste */
	  /***********************/

	CurrentCmp = BaseListeCmp;
	for( ; CurrentCmp != NULL; CurrentCmp = CurrentCmp->Pnext)
		{
		fprintf(dest," ( %s ", CurrentCmp->TimeStamp);

		if( CurrentCmp->Module[0] > ' ' )
			fprintf(dest,CurrentCmp->Module);

		else fprintf(dest,"$noname$") ;

		fprintf(dest," %s ",CurrentCmp->Reference) ;

		/* placement de la valeur */
		fprintf(dest,"%s\n",CurrentCmp->Valeur) ;

		/* Tri des pins */
		TriPinsModule( CurrentCmp );

		/* Placement de la liste des pins */
		Pin = CurrentCmp->Pins;
		for( ; Pin != NULL; Pin = Pin->Pnext )
			{
			if( strlen(Pin->PinNet) > MAX_LEN_NETNAME)
				ChangePinNet( Pin->PinNet );
				
			if( Pin->PinNet[0] > ' ' )
				fprintf(dest,"  ( %s %s )\n", Pin->PinNum, Pin->PinNet);
			else
				fprintf(dest,"  ( %s ? )\n", Pin->PinNum);
			}
		fprintf(dest," )\n");
		}
	fprintf(dest,")\n*\n");
	fclose(dest);
	return(0);
}



	/***********************************************/
	/* void TriPinsModule( STORECMP * CurrentCmp ) */
	/***********************************************/

/* Tri et controle des pins du module CurrenCmp
*/
static void TriPinsModule( STORECMP * CurrentCmp )
{
STOREPIN * Pin, * NextPin, ** BasePin;
int nbpins = 0, ii;
char Line[256];

	Pin = CurrentCmp->Pins;
	if( Pin == NULL ) return;

	/* comptage des pins */
	for( ; Pin != NULL ; Pin = Pin->Pnext ) nbpins++;

	/* Tri des pins: etablissement de la liste des pointeurs */
	BasePin = (STOREPIN ** )MyZMalloc( nbpins * sizeof(STOREPIN*) );

	Pin = CurrentCmp->Pins;
	for( ii = 0 ; ii < nbpins ; ii++, Pin = Pin->Pnext )
		{
		BasePin[ii] = Pin;
		}
	/* Tri des Pins */
	qsort( BasePin, nbpins, sizeof( STORECMP*), PinCompare) ;

	/* Remise a jour des pointeurs chaines */
	for( ii = 0 ; ii < nbpins-1 ; ii++ )
		{
		BasePin[ii]->Pnext = BasePin[ii+1];
		}
	BasePin[ii]->Pnext = NULL;
	CurrentCmp->Pins = BasePin[0];

	MyFree(BasePin);

	/* Elimination des redondances */
	Pin = CurrentCmp->Pins;
	while( Pin != NULL)
		{
		NextPin = Pin->Pnext;
		if ( NextPin == NULL ) break;
		if( strcmp( Pin->PinNum, NextPin->PinNum ) )
			{
			Pin = Pin->Pnext;  continue;
			}
		/* 2 pins successives ont le meme numero */
		if( strcmp( Pin->PinNet, NextPin->PinNet ) )
			{
			sprintf(Line,"%s %s pin %s : Differents Nets",
					CurrentCmp->Reference,CurrentCmp->Valeur,
					Pin->PinNum);
			DisplayError(NULL, Line, 60 );
			}
		Pin->Pnext = NextPin->Pnext;
		if( NextPin->PinNet ) MyFree( NextPin->PinNet);
		MyFree( NextPin );
		}
}


	/*****************************************/
	/* int CmpCompare(void *cmp1,void *cmp2) */
	/*****************************************/

/*
routine PinCompare() pour qsort() pour classement alphab‚tique
	pour tri de la liste des Pins
*/


static int PinCompare(const void *cmp1,const void *cmp2)
{
STOREPIN **pt1 , **pt2 ;
int ii;

	pt1 = (STOREPIN**)cmp1;
	pt2 = (STOREPIN**)cmp2;

	ii = StrLenNumICmp( (*pt1)->PinNum, (*pt2)->PinNum, 4);
	return(ii);
}


	/**************************************/
	/* void ChangePinNet( char * PinNet ) */
	/**************************************/

/* Change le NetName PinNet par un nom compose des 8 derniers codes de PinNet
	suivi de _Xnnnnn ou nnnnn est un nom de 0 a 99999
*/
static void ChangePinNet( char * PinNet )
{
STOREPIN * Pin;
STORECMP * CurrentCmp;
int ii;
char *OldName;
char NewName[256];

	OldName = strdup(PinNet);
	ii = strlen(PinNet);
	if( Rjustify )  /* On conserve les 8 dernieres lettres du nom */
		sprintf( NewName,"%s_X%5.5d", OldName + ii - 8, NetNumCode);

	else			 /* On conserve les 8 premieres lettres du nom */
		{
		sprintf( NewName,"%.8s_X%5.5d", OldName, NetNumCode);
		}
	NetNumCode ++;

	CurrentCmp = BaseListeCmp;
	for( ; CurrentCmp != NULL; CurrentCmp = CurrentCmp->Pnext)
		{
		Pin = CurrentCmp->Pins;
		for( ; Pin != NULL; Pin = Pin->Pnext )
			{
			if( Pin->PinNet == NULL ) continue;
			if( strcmp( Pin->PinNet, OldName ) != 0 ) continue;
			strcpy( Pin->PinNet, NewName );
			}
		}

	MyFree(OldName);
}

