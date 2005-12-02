/********************/
/* Fichier colors.h */
/********************/

#ifndef _COLORS_H
#define _COLORS_H

#ifndef eda_global
#define eda_global extern
#endif

/* Definitions des Numeros des Couleurs ( palette de 32) */
#define NBCOLOR 32
#define MASKCOLOR 31	// masque pour bits significatifs

/* bit indicateur d'affichage (vu / non vu) des items : (defini dans les valeurs des couleurs*/
#define ITEM_NOT_SHOW 0x40000
/* Definition du bit de surbrillance */
#define HIGHT_LIGHT_FLAG 0x80000

enum EDA_Colors
{
 BLACK = 0,
 BLUE,
 GREEN,
 CYAN,
 RED,
 MAGENTA,
 BROWN,
 LIGHTGRAY,
 DARKGRAY,
 LIGHTBLUE,
 LIGHTGREEN,
 LIGHTCYAN,
 LIGHTRED,
 LIGHTMAGENTA,
 YELLOW,
 WHITE,
 DARKDARKGRAY,
 DARKBLUE,
 DARKGREEN,
 DARKCYAN,
 DARKRED,
 DARKMAGENTA,
 DARKBROWN,
 LIGHTYELLOW,
LASTCOLOR
};

class StructColors
{
public:
	unsigned char m_Blue, m_Green, m_Red, m_Numcolor;
	char* m_Name;
	int m_LightColor;
};

extern StructColors ColorRefs[NBCOLOR];
#ifdef MAIN
StructColors ColorRefs[NBCOLOR]	=
{
	 { 0,  0,	0,  BLACK, "BLACK", DARKDARKGRAY},
	 { 192,  0,  0, BLUE, "BLUE", LIGHTBLUE},
	 { 0, 160,  0,  GREEN, "GREEN", LIGHTGREEN },
	 { 160, 160, 0,  CYAN, "CYAN", LIGHTCYAN },
	 { 0,  0, 160,  RED, "RED", LIGHTRED },
	 { 160,  0, 160,  MAGENTA, "MAGENTA", LIGHTMAGENTA },
	 { 0, 128, 128,  BROWN, "BROWN", YELLOW },
	 { 192, 192, 192,  LIGHTGRAY, "GRAY", WHITE },
	 { 128,  128,  128,  DARKGRAY, "DARKGRAY", LIGHTGRAY },
	 { 255,	  0, 0,  LIGHTBLUE, "LIGHTBLUE",  LIGHTBLUE },
	 { 0, 255, 0, LIGHTGREEN, "LIGHTGREEN", LIGHTGREEN },
	 { 255, 255, 0, LIGHTCYAN, "LIGHTCYAN", LIGHTCYAN },
	 { 0,  0, 255, LIGHTRED, "LIGHTRED", LIGHTRED },
	 { 255,  0, 255, LIGHTMAGENTA, "LIGHTMAGENTA", LIGHTMAGENTA },
	 { 0, 255, 255, YELLOW, "YELLOW", YELLOW },
	 { 255, 255, 255, WHITE, "WHITE", WHITE },
	 {  64,  64, 64,  DARKDARKGRAY, "DARKDARKGRAY",  DARKGRAY },
	 {  64,	  0,  0,  DARKBLUE, "DARKBLUE", BLUE },
	 {	  0,  64,  0,  DARKGREEN, "DARKGREEN",  GREEN },
	 {  64,  64,  0,  DARKCYAN, "DARKCYAN",  CYAN },
	 {	  0,   0, 80,  DARKRED, "DARKRED",  RED },
	 {  64,   0, 64,  DARKMAGENTA, "DARKMAGENTA" },
	 {	  0,  64, 64,  DARKBROWN, "DARKBROWN",  BROWN },
 	 {	128, 255, 255,   LIGHTYELLOW, "LIGHTYELLOW",   LIGHTYELLOW }
   };
#endif /* ifdef MAIN */


eda_global wxPen * DrawPen;
eda_global wxBrush * DrawBrush;

#endif /* ifndef _COLORS_H */
