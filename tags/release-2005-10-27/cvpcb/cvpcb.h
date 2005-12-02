	/**********************************************/
	/* CVPCB : declaration des variables globales */
	/**********************************************/


#ifndef eda_global
#define eda_global extern
#endif

#include "wxstruct.h"
#include "pcbnew.h"
#include "cvstruct.h"

#include "gr_basic.h"
#include "colors.h"

// Define print format d to display a schematic component line
#define CMP_FORMAT "%3d %8.8s - %16.16s : %-.32s"

enum TypeOfStruct
	{
	STRUCT_NOT_INIT,
	STRUCT_COMPONANT,
	STRUCT_PIN,
	STRUCT_MODULE,
	STRUCT_PSEUDOMODULE
	};

typedef struct StorePin
	{
	int Type;				/* Type de la structure */
	StorePin * Pnext;		/* Chainage avant */
	int index;				/* variable utilisee selon types de netlistes */
	int PinType;			/* code type electrique ( Entree Sortie Passive..) */
	char *PinNet;			/* Pointeur sur le texte nom de net */
	char PinNum[9];
	char PinName[17];
	char Repere[17] ;		/* utilise selon formats de netliste */
	} STOREPIN;

typedef struct StoreCmp
	{
	int Type;				/* Type de la structure */
	StoreCmp * Pnext;		/* Chainage avant */
	StoreCmp * Pback;		/* Chainage arriere */
	int Num;				/* Numero d'ordre */
	int Multi;				/* Nombre d' unites par boitier */
	STOREPIN * Pins;		/* pointeur sur la liste des Pins */
	char Reference[17] ;	/* U3, R5  ... */
	char Valeur[33];		/* 7400, 47K ... */
	char TimeStamp[9];		/* Signature temporelle ("00000000" si absente) */
	char Module[33];		/* Nom du module (Package) corresp */
	char Repere[33];		/* utilise selon formats de netliste */
	} STORECMP;

typedef struct StoreMod
	{
	int Type;				/* Type de la structure */
	StoreMod * Pnext;		/* Chainage avant */
	StoreMod * Pback;		/* Chainage arriere */
	char Module[33] ;		/* Nom du module */
	char LibName[33] ;		/* Nom de la librairie contenant ce module */
	int Num ;				/* Numero d'ordre pour affichage sur la liste */
	char * Doc;				/* Doc associee */
	char * KeyWord;			/* Mots cles associes */
	} STOREMOD;


eda_global STOREMOD * BaseListePkg;
eda_global STORECMP * BaseListeCmp;

eda_global FILE *source ;
eda_global FILE *dest	;
eda_global FILE *libcmp ;
eda_global FILE *lib_module ;

/* nom des fichiers a traiter */
eda_global wxString FFileName;

/* Types de netliste: */
#define TYPE_NON_SPECIFIE 0
#define TYPE_ORCADPCB2 1
#define TYPE_PCAD 2
#define TYPE_VIEWLOGIC_WIR 3
#define TYPE_VIEWLOGIC_NET 4

/* Gestion des noms des librairies */
eda_global wxString g_EquivExtBuffer
#ifdef MAIN
(".equ")
#endif
;
eda_global wxString g_ExtCmpBuffer
#ifdef MAIN
 (".cmp")
#endif
;

eda_global wxString g_UserNetDirBuffer;	// Netlist path (void = current working directory)

eda_global wxArrayString g_ListName_Equ;	// list of .equ files to load

eda_global int output_type;		/* Voir ci dessus */
eda_global int Rjustify;		/* flag pout troncature des noms de Net:
							= 0: debut de chaine conservee (->ORCADPCB2)
							= 1: fin de chaine conservee (->VIEWLOGIC) */
eda_global int selection_type;	/* 0 pour sel par U??, 1 pour sel par ref ORCADPCB */

eda_global int modified;		/* Flag != 0 si modif attribution des modules */
eda_global int ListModIsModified;		/* Flag != 0 si modif liste des lib modules */

eda_global char alim[1024];

eda_global char CurrentPkg[256];	/* nom du module selectionne */

eda_global int nbcomp ;					/* nombre de composants trouves */
eda_global int nblib  ;					/* nombre d'empreintes trouv‚es */
eda_global int composants_non_affectes ;/* nbre de composants non affectes */

eda_global wxString NameBuffer;
eda_global wxString NetInNameBuffer;
eda_global wxString NetInExtBuffer;
eda_global wxString PkgInExtBuffer;
eda_global wxString NetDirBuffer;

eda_global wxString ExtRetroBuffer
#ifdef MAIN
 (".stf")
#endif
;


// Variables generales */

eda_global W_PLOT * SheetList[]
#ifdef MAIN
= {NULL}
#endif
;

// Unused, for pcbnew compatibility:
void Plume(int state);
