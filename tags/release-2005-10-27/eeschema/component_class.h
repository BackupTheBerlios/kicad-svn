	/*****************************************************/
	/* Definitions for the Component classes for EESchema */
	/*****************************************************/

#ifndef COMPONENT_CLASS
#define COMPONENT_CLASS

#ifndef eda_global
#define eda_global extern
#endif

#include "macros.h"
#include "base_struct.h"


/* Definition de la representation du composant */
#define NUMBER_OF_FIELDS 12	/* Nombre de champs de texte affectes au composant */
typedef enum
{
	REFERENCE = 0,			/* Champ Reference of part, i.e. "IC21" */
	VALUE,					/* Champ Value of part, i.e. "3.3K" */
	FOOTPRINT,				/* Champ Name Module PCB, i.e. "16DIP300" */
	SHEET_FILENAME,			/* Champ Name Schema component, i.e. "cnt16.sch" */
	FIELD1,
	FIELD2,
	FIELD3,
	FIELD4,
	FIELD5,
	FIELD6,
	FIELD7,
	FIELD8
} NumFieldType;

eda_global wxString g_FieldNameList[]
#ifdef MAIN
 = {
	"Ref",			/* Reference of part, i.e. "IC21" */
	"Name",			/* Value of part, i.e. "3.3K" */
	"Pcb",			/* Footprint, i.e. "16DIP300" */
	"Sheet",		/* for components which are a schematic file, schematic file name, i.e. "cnt16.sch" */
	"Fld1",			/* User fields */
	"Fld2",
	"Fld3",
	"Fld4",
	"Fld5",
	"Fld6",
	"Fld7",
	"Fld8"
}
#endif
;


/* Class to manage component fields.
	component fields are texts attached to the component (not the graphic texts)
	There are 2 major fields : Reference and Value
*/
class PartTextStruct:  public EDA_BaseStruct, public EDA_TextStruct
{
public:
	int m_FieldId;

public:
	PartTextStruct(const wxPoint & pos = wxPoint(0,0), const wxString & text = "");
	~PartTextStruct(void);
	void PartTextCopy(PartTextStruct * target);
	void Place(WinEDA_DrawFrame * frame, wxDC * DC);

	EDA_Rect GetBoundaryBox(void);
	bool IsVoid();
};

/* the class DrawPartStruct describes a basic virtual component
	Not used directly:
	used classes are EDA_SchComponentStruct (the "classic" schematic component
	and the Pseudo component DrawSheetStruct
*/
class DrawPartStruct: public EDA_BaseStruct
{
public:
	char * m_ChipName;			/* Key to look for in the library, i.e. "74LS00". */
	PartTextStruct m_Field[NUMBER_OF_FIELDS];
	wxPoint m_Pos;				/* Exact position of part. */

public:
	DrawPartStruct( DrawStructureType struct_type, const wxPoint & pos);
	~DrawPartStruct(void);
};


/* the class EDA_SchComponentStruct describes a real component */
class EDA_SchComponentStruct: public DrawPartStruct
{
public:
	int m_Multi;				/* In multi unit chip - which unit to draw. */
	int m_FlagControlMulti;
	int m_Convert;				/* Gestion des mutiples representations (ex: conversion De Morgan) */
	int m_Transform[2][2];		/* The rotation/mirror transformation matrix. */
	bool * m_PinIsDangling;		// liste des indicateurs de pin non connectee

public:
	EDA_SchComponentStruct(const wxPoint & pos = wxPoint(0,0));
	~EDA_SchComponentStruct(void){}
	EDA_SchComponentStruct * GenCopy(void);
	void SetRotationMiroir( int type );
	int GetRotationMiroir(void);
	wxPoint GetScreenCoord(const wxPoint & coord);
	void Display_Infos(WinEDA_DrawFrame * frame);
	void ClearAnnotation(void);
	EDA_Rect GetBoundaryBox( void );

};


#endif /* COMPONENT_CLASS */

