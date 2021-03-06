	/****************************************************/
	/*					AUTOROUT.H						*/
	/* d�clarations communes relative au routage, DRC	*/
	/* et placement automatique des composants			*/
	/****************************************************/

#ifndef AUTOROUT_H
#define AUTOROUT_H


#define TOP 0
#define BOTTOM  1
#define EMPTY	0
#define ILLEGAL -1


	/*****************************************************/
	/* Structures de representation des pads et chevelus */
	/*	pour etablissement du chevelu general complet	 */
	/*****************************************************/



	/***********************************************/
	/* description d'un segment de chevelu general */
	/***********************************************/
	/****************************/
	/* bits d'etat du chevelu : */
	/****************************/
#define CH_VISIBLE 1		/* affichage permanent demande */
#define CH_UNROUTABLE 2		/* non route par l'autorouteur */
#define CH_ROUTE_REQ 4		/* doit etre route par l'autorouteur */
#define CH_ACTIF 8			/* chevelu non encore rout� */
#define LOCAL_CHEVELU 0x8000	/* indique un chevelu reliant 2 pins d'un meme
				module pour le calcul des chevelus relatifs a 1 seul module */

class CHEVELU
{
public:
	int m_NetCode;		// numero de code du net ( = 0.. n , avec 0 si non connecte)
	int status;			// etat: voir defines pr�c�dents (CH_ ...)
	D_PAD * pad_start;  // pointeur sur le pad de depart
	D_PAD * pad_end ;	// pointeur sur le pad de fin
	int dist;			//  longeur du chevelu
};

	/****************************************************************/
	/* description d'un point de piste pour le suivi des connexions */
	/****************************************************************/
#define START_SUR_PAD 0x10
#define END_SUR_PAD 0x20
#define START_SUR_TRACK 0x40
#define END_SUR_TRACK 0x80

#define START_EQU_VIA 0x10
#define END_EQU_VIA 0x20


	/* bits d'etat du PCB */

#define LISTE_PAD_OK 1  /* Bit indicant que la liste des pads est calculee */
#define LISTE_CHEVELU_OK 2 /* Bit indicant que la liste des CHEVELUS est calculee */
#define CHEVELU_LOCAL_OK 4 /* Bit indicant que le CHEVELU local d'un MODULE est calculee */
#define CONNEXION_OK 8  /* Bit indicant que la liste des connexions existe */
#define NET_CODES_OK 16	/* Bit indicant que les netcodes sont OK ( pas de modif
						de noms de net */

#define OK_DRC 0
#define BAD_DRC 1


/* Commandes d'autoplacement / autorouage possibles */
enum CommandOpt
{
	PLACE_ALL,
	PLACE_OUT_OF_BOARD,
	PLACE_INCREMENTAL,
	PLACE_1_MODULE,

	ROUTE_ALL,
	ROUTE_NET,
	ROUTE_MODULE,
	ROUTE_PAD
	};

/* parametre Pas de routage pour routage automatique et generation des zones */

#if defined MAIN
int pas_route = 250;
#else
extern int pas_route;
#endif

/* Variables et structures d'autoroutage */

eda_global int E_scale;			/* facteur d'echelle des tables de distance */

#define ONE_SIDE 0
#define TWO_SIDES 1
eda_global int Nb_Sides;		/* Nombre de couches pour autoroutage (0 ou 1) */

/* Bits Flags de gestion de remplissage du BOARD */
#define FORCE_PADS 1		/* pour forcage placement pads quel que soit le netcode */

/* board dimensions */
extern int Nrows, Ncols;
#if defined MAIN
int Nrows = ILLEGAL;
int Ncols = ILLEGAL;
#endif

eda_global int Ntotal;

/* search statistics */
eda_global int OpenNodes; /* total number of nodes opened */
eda_global int ClosNodes; /* total number of nodes closed */
eda_global int MoveNodes; /* total number of nodes moved */
eda_global int MaxNodes; /* maximum number of nodes opened at one time */

/* Structures utiles a la generation du board en Bit Map */


typedef char BoardCell;
typedef int DistCell;

class BOARDHEAD	 /* header of blocks of BoardCell */
{
public:
	BoardCell * m_BoardSide[2]; /* ptr to block of memory: 2-sided board */
	DistCell * m_DistSide[2];	/* ptr to block of memory: path distance to cells */
	char * m_DirSide[2];		/* header of blocks of chars:pointers back to source */
	bool m_InitBoardDone;
	int m_Layers;
	int m_Nrows, m_Ncols;
	int m_MemSize;

public:
	BOARDHEAD::BOARDHEAD(void);
	BOARDHEAD::~BOARDHEAD(void);
	int InitBoard(void);
	void UnInitBoard(void);

};

eda_global BOARDHEAD Board ;		/* 2-sided board */


/* Constantes utilisees pour le trace des cellules sur le BOARD */
#define WRITE_CELL	0
#define WRITE_OR_CELL 1
#define WRITE_XOR_CELL 2
#define WRITE_AND_CELL 3
#define WRITE_ADD_CELL 4


#include "ar_protos.h"


#endif	/* AUTOROUT_H */

