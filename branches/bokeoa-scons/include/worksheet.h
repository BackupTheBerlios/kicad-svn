	/***************************************************/
	/* WORKSHEET.H: constantes pour trace du cartouche */
	/***************************************************/

	/****************************/
	/* Description du cartouche */
	/****************************/
#define GRID_REF_W 70	/* hauteur de la bande de reference grille */
#define SIZETEXT 90		/* Dimension des textes du cartouche */
#define SIZETEXT_REF 50	/* Dimension des lettres du marquage des reperes */
#define PAS_REF 2000	/* pas des marquages de reference des reperes */

/* Les coord ci dessous sont relatives au coin bas - droit de la feuille, et
seront soustraires de cette origine
*/
#define BLOCK_OX	6000
#define BLOCK_LICENCE_X BLOCK_OX - SIZETEXT
#define BLOCK_LICENCE_Y SIZETEXT
#define BLOCK_REV_X 1200
#define BLOCK_REV_Y (SIZETEXT*3)
#define BLOCK_DATE_X BLOCK_OX - (SIZETEXT*15)
#define BLOCK_DATE_Y (SIZETEXT*3)
#define BLOCK_ID_SHEET_X 1200
#define BLOCK_ID_SHEET_Y SIZETEXT
#define BLOCK_SIZE_SHEET_X BLOCK_OX - SIZETEXT
#define BLOCK_SIZE_SHEET_Y (SIZETEXT*3)
#define BLOCK_TITLE_X BLOCK_OX - SIZETEXT
#define BLOCK_TITLE_Y (SIZETEXT*5)
#define BLOCK_COMMENT_X BLOCK_OX - SIZETEXT
#define BLOCK_COMPANY_Y (SIZETEXT*7)
#define BLOCK_COMMENT1_Y (SIZETEXT*9)
#define BLOCK_COMMENT2_Y (SIZETEXT*11)
#define BLOCK_COMMENT3_Y (SIZETEXT*13)
#define BLOCK_COMMENT4_Y (SIZETEXT*15)

struct WorkSheet
{
public:
	int type;			/* nombre permettant de reconnaitre la description */
	WorkSheet * Pnext;
	int posx, posy;		/* position de l'element ou point de depart du segment */
	int endx, endy;		/* extremite d'un element type segment ou cadre */
	wxChar * Legende;	/* Pour Textes: texte a afficher avant le texte lui meme */
	wxChar * Text;		/* Pour Textes:pointeur sur le texte a afficher */
};

/* Type des descriptions WorkSheet */
enum TypeWorkSheet
	{
	WS_DATE,
	WS_REV,
	WS_LICENCE,
	WS_SIZESHEET,
	WS_IDENTSHEET,
	WS_TITLE,
	WS_NAMECOMP,
	WS_COMMENT1,
	WS_COMMENT2,
	WS_COMMENT3,
	WS_COMMENT4,
	WS_SEGMENT,
	WS_CADRE
	};

extern WorkSheet WS_Date;
extern WorkSheet WS_Revision;
extern WorkSheet WS_Licence;
extern WorkSheet WS_SizeSheet;
extern WorkSheet WS_IdentSheet;
extern WorkSheet WS_Title;
extern WorkSheet WS_Company;
extern WorkSheet WS_Comment1;
extern WorkSheet WS_Comment2;
extern WorkSheet WS_Comment3;
extern WorkSheet WS_Comment4;
extern WorkSheet WS_Segm1;
extern WorkSheet WS_Segm2;
extern WorkSheet WS_Segm3;
extern WorkSheet WS_Segm4;
extern WorkSheet WS_Segm5;
extern WorkSheet WS_Segm6;
extern WorkSheet WS_Segm7;

#ifdef EDA_BASE

WorkSheet WS_Date =
	{
	WS_DATE,
	&WS_Licence,
	BLOCK_DATE_X, BLOCK_DATE_Y,
	0,0,
	wxT("Date: "), NULL
	};

WorkSheet WS_Licence =
	{
	WS_LICENCE,
	&WS_Revision,
	BLOCK_LICENCE_X, BLOCK_LICENCE_Y,
	0,0,
	NULL, NULL
	};

WorkSheet WS_Revision =
	{
	WS_REV,
	&WS_SizeSheet,
	BLOCK_REV_X, BLOCK_REV_Y,
	0,0,
	wxT("Rev: "), NULL
	};

WorkSheet WS_SizeSheet =
	{
	WS_SIZESHEET,
	&WS_IdentSheet,
	BLOCK_SIZE_SHEET_X, BLOCK_SIZE_SHEET_Y,
	0,0,
	wxT("Size: "),NULL
	};

WorkSheet WS_IdentSheet =
	{
	WS_IDENTSHEET,
	&WS_Title,
	BLOCK_ID_SHEET_X, BLOCK_ID_SHEET_Y,
	0,0,
	wxT("Sheet: "),NULL
	};

WorkSheet WS_Title =
	{
	WS_TITLE,
	&WS_Company,
	BLOCK_TITLE_X, BLOCK_TITLE_Y,
	0,0,
	wxT("Title: "),NULL
	};

WorkSheet WS_Company =
	{
	WS_NAMECOMP,
	&WS_Comment1,
	BLOCK_COMMENT_X, BLOCK_COMPANY_Y,
	0,0,
	NULL,NULL,
	};

WorkSheet WS_Comment1 =
	{
	WS_COMMENT1,
	&WS_Comment2,
	BLOCK_COMMENT_X, BLOCK_COMMENT1_Y,
	0,0,
	NULL,NULL
	};

WorkSheet WS_Comment2 =
	{
	WS_COMMENT2,
	&WS_Comment3,
	BLOCK_COMMENT_X, BLOCK_COMMENT2_Y,
	0,0,
	NULL,NULL
	};

WorkSheet WS_Comment3 =
	{
	WS_COMMENT3,
	&WS_Comment4,
	BLOCK_COMMENT_X, BLOCK_COMMENT3_Y,
	0,0,
	NULL,NULL
	};

WorkSheet WS_Comment4 =
	{
	WS_COMMENT4,
	&WS_Segm1,
	BLOCK_COMMENT_X, BLOCK_COMMENT4_Y,
	0,0,
	NULL,NULL
	};

WorkSheet WS_Segm1 =   /* segment vertical gauche */
	{
	WS_SEGMENT,
	&WS_Segm2,
	BLOCK_OX, SIZETEXT * 16,
	BLOCK_OX, 0,
	NULL,NULL
	};

WorkSheet WS_Segm2 =	/* segment horizontal superieur */
	{
	WS_SEGMENT,
	&WS_Segm3,
	BLOCK_OX, SIZETEXT * 16,
	0, SIZETEXT * 16,
	NULL,NULL
	};

WorkSheet WS_Segm3 =		/* segment horizontal au dessus de NAME COMPANY */
	{
	WS_SEGMENT,
	&WS_Segm4,
	BLOCK_OX, SIZETEXT * 6,
	0, SIZETEXT * 6,
	NULL,NULL
	};

WorkSheet WS_Segm4 =		/* segment vertical a gauche de SHEET et REV */
	{
	WS_SEGMENT,
	&WS_Segm5,
	BLOCK_REV_X + SIZETEXT, SIZETEXT * 4,
	BLOCK_REV_X + SIZETEXT, 0,
	NULL,NULL
	};

WorkSheet WS_Segm5 =		/* 1er segment horizontal */
	{
	WS_SEGMENT,
	&WS_Segm6,
	BLOCK_OX, SIZETEXT * 2,
	0, SIZETEXT * 2,
	NULL,NULL
	};

WorkSheet WS_Segm6 =		/* 2eme segment horizontal */
	{
	WS_SEGMENT,
	&WS_Segm7,
	BLOCK_OX, SIZETEXT * 4,
	0, SIZETEXT * 4,
	NULL,NULL
	};

WorkSheet WS_Segm7 =		/* segment vertical apres SIZE */
	{
	WS_SEGMENT,
	NULL,
	BLOCK_OX - (SIZETEXT * 11), SIZETEXT * 4,
	BLOCK_OX - (SIZETEXT * 11), SIZETEXT * 2,
	NULL,NULL
	};

#endif
