/***********************************************************************/
/* Methodes de base de gestion des classes des elements de schematique */
/***********************************************************************/

#include "fctsys.h"
#include "gr_basic.h"
#include "macros.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"
#include "id.h"

#include "protos.h"



	/***************************/
	/* class DrawPartStruct	*/
	/* class EDA_SchComponentStruct */
	/***************************/

/***********************************************************************************/
DrawPartStruct::DrawPartStruct( DrawStructureType struct_type, const wxPoint & pos):
				EDA_BaseStruct(struct_type)
/***********************************************************************************/
{
	m_Pos = pos;
	m_TimeStamp = 0;
}

/************************************/
DrawPartStruct::~DrawPartStruct(void)
/************************************/
{
}

/********************************************************/
wxString DrawPartStruct::ReturnFieldName(int FieldNumber)
/********************************************************/
/* Return the Field name from its number (REFERENCE, VALUE ..)
	FieldNameList cannot be static, because we want the text translation
	for I18n
*/
{
wxString FieldNameList[] = {
	_("Ref"),		/* Reference of part, i.e. "IC21" */
	_("Value"),		/* Value of part, i.e. "3.3K" */
	_("Pcb"),		/* Footprint, i.e. "16DIP300" */
	_("Sheet"),		/* for components which are a schematic file, schematic file name, i.e. "cnt16.sch" */
	_("Fld1"),		/* User fields (1 to 8) */
	_("Fld2"),
	_("Fld3"),
	_("Fld4"),
	_("Fld5"),
	_("Fld6"),
	_("Fld7"),
	_("Fld8") };
	
	return FieldNameList[FieldNumber];
}


/*******************************************************************/
EDA_SchComponentStruct::EDA_SchComponentStruct(const wxPoint & pos):
					DrawPartStruct(DRAW_LIB_ITEM_STRUCT_TYPE, pos)
/*******************************************************************/
{
int ii;
	m_Multi = 0;	/* In multi unit chip - which unit to draw. */
	m_FlagControlMulti = 0;
	m_Convert = 0;	/* Gestion des mutiples representations (conversion De Morgan) */
	/* The rotation/mirror transformation matrix. pos normal*/
	m_Transform[0][0] = 1;
	m_Transform[0][1] = 0;
	m_Transform[1][0] = 0;
	m_Transform[1][1] = -1;

	/* initialisation des Fields */
	for(ii = 0; ii < NUMBER_OF_FIELDS; ii++)
		{
		m_Field[ii].m_Pos = m_Pos;
		m_Field[ii].m_Layer = LAYER_FIELDS;
		m_Field[ii].m_FieldId = REFERENCE + ii;
		m_Field[ii].m_Parent = this;
		}

	m_Field[VALUE].m_Layer = LAYER_VALUEPART;
	m_Field[REFERENCE].m_Layer = LAYER_REFERENCEPART;

	m_PinIsDangling = NULL;

}


/**********************************************************************/
EDA_Rect EDA_SchComponentStruct::GetBoundaryBox( void )
/**********************************************************************/
{
EDA_LibComponentStruct * Entry = FindLibPart(m_ChipName.GetData(), wxEmptyString, FIND_ROOT);
EDA_Rect BoundaryBox;
int x0, xm, y0, ym;
	/* Get the basic Boundary box */
	if ( Entry )
	{
		BoundaryBox = Entry->GetBoundaryBox( m_Multi, m_Convert);
		x0 = BoundaryBox.GetX(); xm = BoundaryBox.GetRight();
		// We must reverse Y values, because matrix orientation
		// suppose Y axis normal for the library items coordinates,
		// m_Transform reverse Y values, but BoundaryBox ais already reversed!
		y0 = - BoundaryBox.GetY();
		ym = - BoundaryBox.GetBottom();
	}
	else	/* if lib Entry not found, give a reasonable size */
	{
		x0 = y0 = -50;
		xm = ym = 50;
	}

	/* Compute the real Boundary box (rotated, mirrored ...)*/
	int x1 = m_Transform[0][0] * x0 + m_Transform[0][1] * y0;

	int y1 = m_Transform[1][0] * x0 + m_Transform[1][1] * y0;

	int x2 = m_Transform[0][0] * xm + m_Transform[0][1] * ym;

	int y2 = m_Transform[1][0] * xm + m_Transform[1][1] * ym;

	// H and W must be > 0 for wxRect:
	if ( x2 < x1 ) EXCHG( x2, x1 );
	if ( y2 < y1 ) EXCHG( y2, y1 );
	BoundaryBox.SetX(x1); BoundaryBox.SetY(y1);
	BoundaryBox.SetWidth(x2-x1);
	BoundaryBox.SetHeight(y2-y1);

	BoundaryBox.Offset(m_Pos);
	return BoundaryBox;
}

/************************************************/
void EDA_SchComponentStruct::ClearAnnotation(void)
/************************************************/
/* Suppress annotation ( i.i IC23 changed to IC? and part reset to 1)
*/
{
	while ( isdigit(m_Field[REFERENCE].m_Text.Last() ) )
		m_Field[REFERENCE].m_Text.RemoveLast();
	if ( m_Field[REFERENCE].m_Text.Last() != '?' )
		m_Field[REFERENCE].m_Text.Append('?');

EDA_LibComponentStruct *Entry;
	Entry = FindLibPart(m_ChipName.GetData(),wxEmptyString,FIND_ROOT);

	if ( !Entry || ! Entry->m_UnitSelectionLocked )
		m_Multi = 1;
}

/**************************************************************/
EDA_SchComponentStruct * EDA_SchComponentStruct::GenCopy(void)
/**************************************************************/
{
EDA_SchComponentStruct * new_item = new EDA_SchComponentStruct( m_Pos );
int ii;

	new_item->m_Multi = m_Multi;
	new_item->m_ChipName = m_ChipName;
	new_item->m_FlagControlMulti = m_FlagControlMulti;
	new_item->m_Convert = m_Convert;
	new_item->m_Transform[0][0] = m_Transform[0][0];
	new_item->m_Transform[0][1] = m_Transform[0][1];
	new_item->m_Transform[1][0] = m_Transform[1][0];
	new_item->m_Transform[1][1] = m_Transform[1][1];
	new_item->m_TimeStamp = m_TimeStamp;


	/* initialisation des Fields */
	for(ii = 0; ii < NUMBER_OF_FIELDS; ii++)
	{
		m_Field[ii].PartTextCopy(& new_item->m_Field[ii]);
	}

	return new_item;
}


/***********************************************************/
void EDA_SchComponentStruct::SetRotationMiroir( int type_rotate )
/***********************************************************/
{
int TempMat[2][2], TempR;
bool Transform = FALSE;

	switch (type_rotate)
		{
		case CMP_ORIENT_0:
		case CMP_NORMAL:		/* Position Initiale */
			m_Transform[0][0] = 1;
			m_Transform[1][1] = -1;
			m_Transform[1][0] = m_Transform[0][1] = 0;
			break;

		case CMP_ROTATE_CLOCKWISE:		  /* Rotate + */
			TempMat[0][0] = TempMat[1][1] = 0;
			TempMat[0][1] = 1;
			TempMat[1][0] = -1;
			Transform = TRUE;
			break;

		case CMP_ROTATE_COUNTERCLOCKWISE:		  /* Rotate - */
			TempMat[0][0] = TempMat[1][1] = 0;
			TempMat[0][1] = -1;
			TempMat[1][0] = 1;
			Transform = TRUE;
			break;

		case CMP_MIROIR_Y:		/* MirrorY */
			TempMat[0][0] = -1;
			TempMat[1][1] = 1;
			TempMat[0][1] = TempMat[1][0] = 0;
			Transform = TRUE;
			break;

		case CMP_MIROIR_X:		  /* MirrorX */
			TempMat[0][0] = 1;
			TempMat[1][1] = -1;
			TempMat[0][1] = TempMat[1][0] = 0;
			Transform = TRUE;
			break;

		case CMP_ORIENT_90:
			SetRotationMiroir( CMP_ORIENT_0 );
			SetRotationMiroir( CMP_ROTATE_COUNTERCLOCKWISE );
			break;

		case CMP_ORIENT_180:
			SetRotationMiroir( CMP_ORIENT_0 );
			SetRotationMiroir( CMP_ROTATE_COUNTERCLOCKWISE );
 			SetRotationMiroir( CMP_ROTATE_COUNTERCLOCKWISE );
			break;

		case CMP_ORIENT_270:
			SetRotationMiroir( CMP_ORIENT_0 );
			SetRotationMiroir( CMP_ROTATE_CLOCKWISE );
			break;

		case (CMP_ORIENT_0+CMP_MIROIR_X):
			SetRotationMiroir( CMP_ORIENT_0 );
			SetRotationMiroir( CMP_MIROIR_X );
			break;

		case (CMP_ORIENT_0+CMP_MIROIR_Y):
			SetRotationMiroir( CMP_ORIENT_0 );
			SetRotationMiroir( CMP_MIROIR_Y );
			break;

		case (CMP_ORIENT_90+CMP_MIROIR_X):
			SetRotationMiroir( CMP_ORIENT_90 );
			SetRotationMiroir( CMP_MIROIR_X );
			break;

		case (CMP_ORIENT_90+CMP_MIROIR_Y):
			SetRotationMiroir( CMP_ORIENT_90 );
			SetRotationMiroir( CMP_MIROIR_Y );
			break;

		case (CMP_ORIENT_180+CMP_MIROIR_X):
			SetRotationMiroir( CMP_ORIENT_180 );
			SetRotationMiroir( CMP_MIROIR_X );
			break;

		case (CMP_ORIENT_180+CMP_MIROIR_Y):
			SetRotationMiroir( CMP_ORIENT_180 );
			SetRotationMiroir( CMP_MIROIR_Y );
			break;

		case (CMP_ORIENT_270+CMP_MIROIR_X):
			SetRotationMiroir( CMP_ORIENT_270 );
			SetRotationMiroir( CMP_MIROIR_X );
			break;

		case (CMP_ORIENT_270+CMP_MIROIR_Y):
			SetRotationMiroir( CMP_ORIENT_270 );
			SetRotationMiroir( CMP_MIROIR_Y );
			break;

		default:
			Transform = FALSE;
			DisplayError(NULL, wxT("SetRotateMiroir() error: ill value") );
			break;
		}

	if ( Transform )
		{
		TempR = m_Transform[0][0] * TempMat[0][0] +
							m_Transform[0][1] * TempMat[1][0];
		m_Transform[0][1] = m_Transform[0][0] * TempMat[0][1] +
								m_Transform[0][1] * TempMat[1][1];
		m_Transform[0][0] = TempR;
		TempR = m_Transform[1][0] * TempMat[0][0] +
						m_Transform[1][1] * TempMat[1][0];
		m_Transform[1][1] = m_Transform[1][0] * TempMat[0][1] +
								m_Transform[1][1] * TempMat[1][1];
		m_Transform[1][0] = TempR;
		}
}


/*********************************************/
int EDA_SchComponentStruct::GetRotationMiroir(void)
/*********************************************/
{
int type_rotate = CMP_ORIENT_0;
int TempMat[2][2], MatNormal[2][2];
int ii;
bool found = FALSE;

	memcpy(TempMat, m_Transform, sizeof(TempMat));
	SetRotationMiroir(CMP_ORIENT_0);
	memcpy(MatNormal, m_Transform, sizeof(MatNormal));

	for ( ii = 0; ii < 4; ii++ )
		{
		if ( memcmp(TempMat, m_Transform, sizeof(MatNormal)) == 0)
			{
			found = TRUE; break;
			}
		SetRotationMiroir(CMP_ROTATE_COUNTERCLOCKWISE);
		}

	if ( ! found )
		{
		type_rotate = CMP_MIROIR_X + CMP_ORIENT_0;
		SetRotationMiroir(CMP_NORMAL);
		SetRotationMiroir(CMP_MIROIR_X);
		for ( ii = 0; ii < 4; ii++ )
			{
			if ( memcmp(TempMat, m_Transform, sizeof(MatNormal)) == 0)
				{
				found = TRUE; break;
				}
			SetRotationMiroir(CMP_ROTATE_COUNTERCLOCKWISE);
			}
		}

	if ( ! found )
		{
		type_rotate = CMP_MIROIR_Y + CMP_ORIENT_0;
		SetRotationMiroir(CMP_NORMAL);
		SetRotationMiroir(CMP_MIROIR_Y);
		for ( ii = 0; ii < 4; ii++ )
			{
			if ( memcmp(TempMat, m_Transform, sizeof(MatNormal)) == 0)
				{
				found = TRUE; break;
				}
			SetRotationMiroir(CMP_ROTATE_COUNTERCLOCKWISE);
			}
		}

	memcpy(m_Transform, TempMat, sizeof(m_Transform));

	if ( found )
		{
		return (type_rotate + ii);
		}
	else
		{
		wxBell(); return CMP_NORMAL;
		}
}

/***************************************************************/
wxPoint EDA_SchComponentStruct::GetScreenCoord(const wxPoint & coord)
/***************************************************************/
/* Renvoie la coordonn�e du point coord, en fonction de l'orientation
du composant (rotation, miroir).
	Les coord sont toujours relatives � l'ancre (coord 0,0) du composant
*/
{
wxPoint screenpos;
	screenpos.x = m_Transform[0][0] * coord.x + m_Transform[0][1] * coord.y;
	screenpos.y = m_Transform[1][0] * coord.x + m_Transform[1][1] * coord.y;
	return screenpos;
}


/***************************************************************************/
PartTextStruct::PartTextStruct(const wxPoint & pos, const wxString & text):
			EDA_BaseStruct(DRAW_PART_TEXT_STRUCT_TYPE),
			EDA_TextStruct(text)
/***************************************************************************/
{
	m_Pos = pos;
	m_FieldId = 0;
}

/************************************/
PartTextStruct::~PartTextStruct(void)
/************************************/
{
}

/***********************************************************/
void PartTextStruct::PartTextCopy(PartTextStruct * target)
/***********************************************************/
{
	target->m_Text = m_Text;
	target->m_Layer = m_Layer;
	target->m_Pos = m_Pos;
	target->m_Size = m_Size;
	target->m_Attributs = m_Attributs;
	target->m_FieldId = m_FieldId;
	target->m_Orient = m_Orient;
	target->m_HJustify = m_HJustify;
	target->m_VJustify = m_VJustify;
	target->m_Flags = m_Flags;
}


/*********************************/
bool PartTextStruct::IsVoid(void)
/*********************************/
/* return True if The field is void, i.e.:
	contains wxEmptyString or "~"
*/
{
	if ( m_Text.IsEmpty() || m_Text == wxT("~") ) return TRUE;
	return FALSE;
}


/********************************************/
EDA_Rect PartTextStruct::GetBoundaryBox(void)
/********************************************/
/* return
	EDA_Rect contains the real (user coordinates) boundary box for a text field,
according to the component position, rotation, mirror ...

*/
{
EDA_Rect BoundaryBox;
int hjustify, vjustify;
int textlen;
int orient;
int dx, dy, x1, y1, x2, y2;

EDA_SchComponentStruct * DrawLibItem = (EDA_SchComponentStruct *) m_Parent;

	textlen = GetLength();
	orient = m_Orient;
	wxPoint pos = DrawLibItem->m_Pos;
	x1 = m_Pos.x - pos.x;
	y1 = m_Pos.y - pos.y;

	dx = m_Size.x * textlen;
	if ( m_FieldId == REFERENCE )	// Real Text can be U1 or U1A
	{
		EDA_LibComponentStruct *Entry =
			FindLibPart(DrawLibItem->m_ChipName.GetData(),wxEmptyString,FIND_ROOT);
		if ( Entry && (Entry->m_UnitCount > 1) )
			dx = m_Size.x * (textlen + 1 );	// because U1 is U1A or U1B ...
	}

	// Real X Size is 10/9 char size because space between 2 chars is 1/10 X Size
	dx = (dx * 10) / 9;

	dy = m_Size.y;
	hjustify = m_HJustify;
	vjustify = m_VJustify;

	x2 = pos.x  + (DrawLibItem->m_Transform[0][0] * x1)
		+ (DrawLibItem->m_Transform[0][1] * y1);
	y2 = pos.y + (DrawLibItem->m_Transform[1][0] * x1)
		+ (DrawLibItem->m_Transform[1][1] * y1);

	/* If the component orientation is +/- 90 deg, the text orienation must be changed */
	if(DrawLibItem->m_Transform[0][1])
	{
		if ( orient == TEXT_ORIENT_HORIZ)
			orient = TEXT_ORIENT_VERT;
		else orient = TEXT_ORIENT_HORIZ;
		/* is it mirrored (for text justify)*/
		EXCHG(hjustify, vjustify);
		if (DrawLibItem->m_Transform[1][0] < 0 ) vjustify = - vjustify;
		if (DrawLibItem->m_Transform[0][1] > 0 ) hjustify = - hjustify;
	}
	else	/* component horizontal: is it mirrored (for text justify)*/
	{
		if (DrawLibItem->m_Transform[0][0] < 0 )
			hjustify = - hjustify;
		if (DrawLibItem->m_Transform[1][1] > 0 )
			vjustify = - vjustify;
	}
	
	if ( orient == TEXT_ORIENT_VERT ) EXCHG(dx, dy);

	if ( hjustify == GR_TEXT_HJUSTIFY_CENTER ) x1 = x2 - (dx/2);
	else if ( hjustify == GR_TEXT_HJUSTIFY_RIGHT ) x1 = x2 - dx;
	else x1 = x2;
	if ( vjustify == GR_TEXT_VJUSTIFY_CENTER ) y1 = y2 - (dy/2);
	else if ( vjustify == GR_TEXT_VJUSTIFY_BOTTOM ) y1 = y2 - dy;
	else y1 = y2;

	BoundaryBox.SetX(x1);
	BoundaryBox.SetY(y1);
	BoundaryBox.SetWidth(x2-x1);
	BoundaryBox.SetHeight(y2-y1);

	return BoundaryBox;
}
