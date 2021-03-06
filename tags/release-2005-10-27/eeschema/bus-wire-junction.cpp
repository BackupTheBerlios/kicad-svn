/*********************************************************/
/* Modules de creations de Traits, Wires, Bus, Junctions */
/*********************************************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"

#include "protos.h"


/* Routines Locales */
static void Polyline_in_Ghost(WinEDA_DrawPanel * panel, wxDC * DC, bool erase);
static void Segment_in_Ghost(WinEDA_DrawPanel * panel, wxDC * DC, bool erase);
static void ExitTrace(WinEDA_DrawFrame * frame, wxDC * DC);
static bool IsTerminalPoint(SCH_SCREEN * screen, const wxPoint & pos, int layer );
static bool IsJunctionNeeded (WinEDA_SchematicFrame * frame, wxPoint & pos );


wxPoint s_ConnexionStartPoint;

/*************************************************************/
void WinEDA_SchematicFrame::BeginSegment(wxDC * DC, int type)
/*************************************************************/
/* Create a new segment ( WIRE, BUS ).
*/
{
EDA_DrawLineStruct * oldsegment, * newsegment;
wxPoint pos = GetScreen()->m_Curseur;
	
	if ( GetScreen()->m_CurrentItem &&
		 (GetScreen()->m_CurrentItem->m_Flags == 0) )
		GetScreen()->m_CurrentItem = NULL;

	if ( GetScreen()->m_CurrentItem )
		{
		switch (GetScreen()->m_CurrentItem->m_StructType )
			{
			case DRAW_SEGMENT_STRUCT_TYPE:
			case DRAW_POLYLINE_STRUCT_TYPE:
				break;

			default:
				return;
			}
		}

	oldsegment = newsegment =
			(EDA_DrawLineStruct *) GetScreen()->m_CurrentItem;

	if (!newsegment)  /* first point : Create first wire ou bus */
	{
		s_ConnexionStartPoint = pos;
		SchematicCleanUp(NULL);
		switch(type)
		{
			default:
				newsegment = new EDA_DrawLineStruct(pos, LAYER_NOTES);
				break;
			case LAYER_WIRE:
				newsegment = new EDA_DrawLineStruct(pos, LAYER_WIRE);
				/* A junction will be created later, when w'll know the
				segment end, and if the junction is really needed */
				break;
			case LAYER_BUS:
				newsegment = new EDA_DrawLineStruct(pos, LAYER_BUS);
				break;
		}

		newsegment->m_Flags = IS_NEW;
		GetScreen()->m_CurrentItem = newsegment;
		GetScreen()->ManageCurseur = Segment_in_Ghost;
		GetScreen()->ForceCloseManageCurseur = ExitTrace;
		g_ItemToRepeat = NULL;
	}

	else	/* Trace en cours: Placement d'un point supplementaire */
	{
		if( (oldsegment->m_Start.x == oldsegment->m_End.x) &&
			(oldsegment->m_Start.y == oldsegment->m_End.y) )	/* Structure inutile */
			return;

		GetScreen()->ManageCurseur(DrawPanel, DC, FALSE);


		/* Creation du segment suivant ou fin de trac� si point sur pin, jonction ...*/
		if ( IsTerminalPoint(GetScreen(), oldsegment->m_End, oldsegment->m_Layer) )
		{
			EndSegment(DC); return;
		}

		/* Placement en liste generale */
		oldsegment->Pnext = GetScreen()->EEDrawList;
		g_ItemToRepeat = GetScreen()->EEDrawList = oldsegment;
		GetScreen()->Trace_Curseur(DrawPanel, DC);	// Erase schematic cursor
		RedrawOneStruct(DrawPanel,DC, oldsegment, GR_DEFAULT_DRAWMODE);
		GetScreen()->Trace_Curseur(DrawPanel, DC);	// Display schematic cursor

		/* Creation du segment suivant */
		newsegment = oldsegment->GenCopy();
		newsegment->m_Start = oldsegment->m_End;
		newsegment->m_End = pos;
		oldsegment->m_Flags = 0;
		newsegment->m_Flags = IS_NEW;
		GetScreen()->m_CurrentItem = newsegment;
		GetScreen()->ManageCurseur(DrawPanel, DC, FALSE);
		if ( (oldsegment->m_Start.x == s_ConnexionStartPoint.x)
			&& (oldsegment->m_Start.y == s_ConnexionStartPoint.y))
		{	/* This is the first segment: Now we know the start segment position.
			Create a junction if needed. Note: a junction can be needed
			later, if the new segment is merged (after a cleanup) with an older one 
			(tested when the connection will be finished)*/
			if ( IsJunctionNeeded(this, s_ConnexionStartPoint) )
				CreateNewJunctionStruct(DC, s_ConnexionStartPoint);
		}
	}
}


/*************************************************************/
/*	 Routine de fin de trace d'une struct segment (Wire, Bus */
/*************************************************************/
void WinEDA_SchematicFrame::EndSegment(wxDC *DC)
{
EDA_DrawLineStruct * segment = (EDA_DrawLineStruct *)GetScreen()->m_CurrentItem;
wxPoint end_point = segment->m_End;
	
	if ( GetScreen()->ManageCurseur == NULL ) return;
	if ( segment == NULL ) return;
	if ( (segment->m_Flags & IS_NEW) == 0) return;

	if( (segment->m_Start.x == segment->m_End.x) &&
		(segment->m_Start.y == segment->m_End.y) )	/* Structure inutile */
	{
		EraseStruct(segment, (SCH_SCREEN*)GetScreen());
		segment = NULL;
	}

	else
	{

		/* Put it in Drawlist */
		GetScreen()->ManageCurseur(DrawPanel, DC, FALSE);
		segment->Pnext = GetScreen()->EEDrawList;
		g_ItemToRepeat = GetScreen()->EEDrawList = segment;
		segment->m_Flags = 0;
	}

	/* Fin de trace */
	GetScreen()->ManageCurseur = NULL;
	GetScreen()->ForceCloseManageCurseur = NULL;
	GetScreen()->m_CurrentItem = NULL;

	SchematicCleanUp(NULL);

	// Automatic place of a junction on the end point, if needed
	DrawJunctionStruct * junction1 = NULL, * junction2 = NULL;
	if ( IsJunctionNeeded(this, end_point) )
	{
		junction1 = CreateNewJunctionStruct(DC, end_point);
	}
	/* Automatic place of a junction on the start point if necessary because the 
	Cleanup can suppress intermediate points by merging wire segments*/
	if ( IsJunctionNeeded(this, s_ConnexionStartPoint) )
	{
		GetScreen()->m_Curseur = s_ConnexionStartPoint;
		junction2 = CreateNewJunctionStruct(DC, s_ConnexionStartPoint);
	}
	TestDanglingEnds(GetScreen()->EEDrawList, DC);
	
	/* Redraw junctions which can be changed by TestDanglingEnds() */
	if ( junction1 ) RedrawOneStruct(DrawPanel,DC, junction1, GR_DEFAULT_DRAWMODE);
	if ( junction2 ) RedrawOneStruct(DrawPanel,DC, junction2, GR_DEFAULT_DRAWMODE);
	
	SetFlagModify(GetScreen());
	if( segment )
	{
		GetScreen()->Trace_Curseur(DrawPanel, DC);	// Erase schematic cursor
		RedrawOneStruct(DrawPanel,DC, segment, GR_DEFAULT_DRAWMODE);
		GetScreen()->Trace_Curseur(DrawPanel, DC);	// Display schematic cursor
	}
}

/****************************************************************************/
static void Segment_in_Ghost(WinEDA_DrawPanel * panel, wxDC * DC, bool erase)
/****************************************************************************/
/*  Dessin du Segment Fantome lors des deplacements du curseur
*/
{
EDA_DrawLineStruct * segment =
		(EDA_DrawLineStruct *) panel->m_Parent->GetScreen()->m_CurrentItem;
wxPoint endpos;
int color;

	if ( segment == NULL ) return;

	color = ReturnLayerColor(segment->m_Layer) ^ HIGHT_LIGHT_FLAG;

	endpos = panel->m_Parent->GetScreen()->m_Curseur;

	if( g_HVLines )	/* Coerce the line to vertical or horizontal one: */
		{
		if (ABS(endpos.x - segment->m_Start.x) < ABS(endpos.y - segment->m_Start.y))
			endpos.x = segment->m_Start.x;
		else
			endpos.y = segment->m_Start.y;
		}

	if( erase )		// Redraw if segment lengtht != 0
	{
		if ( (segment->m_Start.x != segment->m_End.x) ||
			 (segment->m_Start.y != segment->m_End.y) )
		RedrawOneStruct(panel,DC, segment, XOR_MODE, color);
	}
	segment->m_End = endpos;
	// Redraw if segment lengtht != 0

	if ( (segment->m_Start.x != segment->m_End.x) ||
			(segment->m_Start.y != segment->m_End.y) )
	RedrawOneStruct(panel,DC, segment, XOR_MODE,color);
}

/*****************************************************************************/
static void Polyline_in_Ghost(WinEDA_DrawPanel * panel, wxDC * DC, bool erase)
/*****************************************************************************/
/*  Dessin du du Polyline Fantome lors des deplacements du curseur
*/
{
DrawPolylineStruct * NewPoly =
		(DrawPolylineStruct *)panel->m_Parent->GetScreen()->m_CurrentItem;
int color;
wxPoint endpos;

	endpos = panel->m_Parent->GetScreen()->m_Curseur;
	color = ReturnLayerColor(NewPoly->m_Layer);

	GRSetDrawMode(DC, XOR_MODE);

	if( g_HVLines )
		{
		/* Coerce the line to vertical or horizontal one: */
		if (ABS(endpos.x - NewPoly->m_Points[NewPoly->m_NumOfPoints * 2 - 2]) <
			 ABS(endpos.y - NewPoly->m_Points[NewPoly->m_NumOfPoints * 2 - 1]))
			endpos.x = NewPoly->m_Points[NewPoly->m_NumOfPoints * 2 - 2];
		else
			endpos.y = NewPoly->m_Points[NewPoly->m_NumOfPoints * 2 - 1];
		}

	NewPoly->m_NumOfPoints++;
	if( erase )
		RedrawOneStruct(panel,DC, NewPoly, XOR_MODE, color);

	NewPoly->m_Points[NewPoly->m_NumOfPoints * 2 - 2] = endpos.x;
	NewPoly->m_Points[NewPoly->m_NumOfPoints * 2 - 1] = endpos.y;
	RedrawOneStruct(panel,DC, NewPoly, XOR_MODE, color);
	NewPoly->m_NumOfPoints--;
}

/**********************************************************/
void WinEDA_SchematicFrame::DeleteCurrentSegment(wxDC * DC)
/**********************************************************/
/*
Routine effacant le dernier trait trace, ou l'element pointe par la souris
*/
{

	g_ItemToRepeat = NULL;

	if( (GetScreen()->m_CurrentItem == NULL) ||
		((GetScreen()->m_CurrentItem->m_Flags & IS_NEW) == 0) )
		{
		return;
		}

	/* Trace en cours: annulation */
	if (GetScreen()->m_CurrentItem->m_StructType == DRAW_POLYLINE_STRUCT_TYPE)
		{
		Polyline_in_Ghost(DrawPanel, DC, FALSE); /* Effacement du trace en cours */
		}

	else
		{
		Segment_in_Ghost(DrawPanel, DC, FALSE); /* Effacement du trace en cours */
		}

	EraseStruct(GetScreen()->m_CurrentItem, GetScreen());
	GetScreen()->ManageCurseur = NULL;
	GetScreen()->m_CurrentItem = NULL;
}


/***************************************************************************/
DrawJunctionStruct * WinEDA_SchematicFrame::CreateNewJunctionStruct(
		wxDC * DC, const wxPoint & pos)
/***************************************************************************/
/* Routine to create new connection struct.
*/
{
DrawJunctionStruct *NewConnect;

	NewConnect = new DrawJunctionStruct(pos);

	g_ItemToRepeat = NewConnect;

	GetScreen()->Trace_Curseur(DrawPanel, DC);	// Erase schematic cursor
	RedrawOneStruct(DrawPanel,DC, NewConnect, GR_DEFAULT_DRAWMODE);
	GetScreen()->Trace_Curseur(DrawPanel, DC);	// Display schematic cursor

	NewConnect->Pnext = GetScreen()->EEDrawList;
	GetScreen()->EEDrawList = NewConnect;
	SetFlagModify(GetScreen());
	return(NewConnect);
}

/*************************************************************************/
EDA_BaseStruct *WinEDA_SchematicFrame::CreateNewNoConnectStruct(wxDC * DC)
/*************************************************************************/
/*Routine to create new NoConnect struct. ( Symbole de Non Connexion)
*/
{
DrawNoConnectStruct *NewNoConnect;

	NewNoConnect = new DrawNoConnectStruct(GetScreen()->m_Curseur);
	g_ItemToRepeat = NewNoConnect;

	GetScreen()->Trace_Curseur(DrawPanel, DC);	// Erase schematic cursor
	RedrawOneStruct(DrawPanel,DC, NewNoConnect,  GR_DEFAULT_DRAWMODE);
	GetScreen()->Trace_Curseur(DrawPanel, DC);	// Display schematic cursor

	NewNoConnect->Pnext = GetScreen()->EEDrawList;
	GetScreen()->EEDrawList = NewNoConnect;
	SetFlagModify(GetScreen());
	return(NewNoConnect);
}


/**********************************************************/
static void ExitTrace(WinEDA_DrawFrame * frame, wxDC * DC)
/**********************************************************/
/* Routine de sortie des menus de trace */
{
BASE_SCREEN * Screen = frame->GetScreen();

	if( Screen->m_CurrentItem)  /* trace en cours */
		{
		Screen->ManageCurseur(frame->DrawPanel, DC, FALSE);
		Screen->ManageCurseur = NULL;
		Screen->ForceCloseManageCurseur = NULL;
		EraseStruct(Screen->m_CurrentItem,(SCH_SCREEN*) Screen);
		Screen->m_CurrentItem = NULL;
		return;
		}

	else g_ItemToRepeat = NULL;	// Fin de commande generale
}


/***************************************************/
void WinEDA_SchematicFrame::RepeatDrawItem(wxDC *DC)
/***************************************************/
/* Routine de recopie du dernier element dessine
	Les elements duplicables sont
		fils, bus, traits, textes, labels
		Les labels termines par un nombre seront incrementes 
*/
{
char Line[256];
int ox = 0, oy = 0;

	if( g_ItemToRepeat == NULL ) return;

	switch( g_ItemToRepeat->m_StructType )
		{
		case DRAW_JUNCTION_STRUCT_TYPE:
			#undef STRUCT
			#define STRUCT ((DrawJunctionStruct*) g_ItemToRepeat)
			g_ItemToRepeat = STRUCT->GenCopy();
			STRUCT->m_Pos.x += g_RepeatStep.x; ox = STRUCT->m_Pos.x;
			STRUCT->m_Pos.y += g_RepeatStep.y; oy = STRUCT->m_Pos.y;
			break;

		case DRAW_NOCONNECT_STRUCT_TYPE:
			#undef STRUCT
			#define STRUCT ((DrawNoConnectStruct*) g_ItemToRepeat)
			g_ItemToRepeat = STRUCT->GenCopy();
			STRUCT->m_Pos.x += g_RepeatStep.x; ox = STRUCT->m_Pos.x;
			STRUCT->m_Pos.y += g_RepeatStep.y; oy = STRUCT->m_Pos.y;
			break;

		case DRAW_TEXT_STRUCT_TYPE:
			#undef STRUCT
			#define STRUCT ((DrawTextStruct*) g_ItemToRepeat)
			g_ItemToRepeat = STRUCT->GenCopy();
			STRUCT->m_Pos.x += g_RepeatStep.x; ox = STRUCT->m_Pos.x;
			STRUCT->m_Pos.y += g_RepeatStep.y; oy = STRUCT->m_Pos.y;
			/*** Increment du numero de label ***/
			strcpy(Line,STRUCT->m_Text.GetData());
			IncrementLabelMember(Line);
			STRUCT->m_Text = Line;
			break;

 
		case DRAW_LABEL_STRUCT_TYPE:
			#undef STRUCT
			#define STRUCT ((DrawLabelStruct*) g_ItemToRepeat)
			g_ItemToRepeat = STRUCT->GenCopy();
			STRUCT->m_Pos.x += g_RepeatStep.x; ox = STRUCT->m_Pos.x;
			STRUCT->m_Pos.y += g_RepeatStep.y; oy = STRUCT->m_Pos.y;
			/*** Increment du numero de label ***/
			strcpy(Line,STRUCT->m_Text.GetData());
			IncrementLabelMember(Line);
			STRUCT->m_Text = Line;
			break;


		case DRAW_GLOBAL_LABEL_STRUCT_TYPE:
			#undef STRUCT
			#define STRUCT ((DrawGlobalLabelStruct*) g_ItemToRepeat)
			g_ItemToRepeat = STRUCT->GenCopy();
			STRUCT->m_Pos.x += g_RepeatStep.x; ox = STRUCT->m_Pos.x;
			STRUCT->m_Pos.y += g_RepeatStep.y; oy = STRUCT->m_Pos.y;
			/*** Increment du numero de label ***/
			strcpy(Line,STRUCT->m_Text.GetData());
			IncrementLabelMember(Line);
			STRUCT->m_Text = Line;
			break;

		case DRAW_SEGMENT_STRUCT_TYPE:
			#undef STRUCT
			#define STRUCT ((EDA_DrawLineStruct*) g_ItemToRepeat)
			g_ItemToRepeat = STRUCT->GenCopy();
			STRUCT->m_Start.x += g_RepeatStep.x; ox = STRUCT->m_Start.x;
			STRUCT->m_Start.y += g_RepeatStep.y; oy = STRUCT->m_Start.y;
			STRUCT->m_End.x += g_RepeatStep.x;
			STRUCT->m_End.y += g_RepeatStep.y;
			break;

		case DRAW_BUSENTRY_STRUCT_TYPE:
			#undef STRUCT
			#define STRUCT ((DrawBusEntryStruct*) g_ItemToRepeat)
			g_ItemToRepeat = STRUCT->GenCopy();
			STRUCT->m_Pos.x += g_RepeatStep.x; ox = STRUCT->m_Pos.x;
			STRUCT->m_Pos.y += g_RepeatStep.y; oy = STRUCT->m_Pos.y;
			break;

		default:
			g_ItemToRepeat = NULL;
			DisplayError(this, "Repeat Type Error", 10);
			break;
		}

	if ( g_ItemToRepeat )
		{
		g_ItemToRepeat->Pnext = GetScreen()->EEDrawList;
		GetScreen()->EEDrawList = g_ItemToRepeat;
		TestDanglingEnds(GetScreen()->EEDrawList, NULL);
		RedrawOneStruct(DrawPanel,DC, g_ItemToRepeat, GR_DEFAULT_DRAWMODE);
//		GetScreen()->Curseur.x = ox; GetScreen()->Curseur.x = oy; 
//		GRMouseWarp(DrawPanel, DrawPanel->CursorScreenPosition() );
		}
}


/******************************************/
void IncrementLabelMember(char * Line)
/******************************************/
/* Routine incrementant les labels, c'est a dire pour les textes finissant
par un nombre, ajoutant <RepeatDeltaLabel> a ce nombre
*/
{
char * strnum;
int ii;

 	strnum = Line + strlen(Line) - 1;
	if( !isdigit(*strnum) ) return;

	while( (strnum >= Line) && isdigit(*strnum) ) strnum--;
	strnum++;	/* pointe le debut de la chaine des digits */
	ii = atoi(strnum) + g_RepeatDeltaLabel;
	sprintf(strnum, "%d", ii);
}

/***************************************************************************/
static bool IsTerminalPoint(SCH_SCREEN * screen, const wxPoint & pos, int layer)
/***************************************************************************/
/* Returne TRUE si pos est un point possible pour terminer automatiquement un
segment, c'est a dire pour
	- type WIRE, si il y a
		- une jonction
		- ou une pin
		- ou une extr�mit� unique de fil

	- type BUS, si il y a
		- ou une extr�mit� unique de BUS
*/
{
EDA_BaseStruct * item;
LibDrawPin * pin;
EDA_SchComponentStruct * LibItem = NULL;
DrawSheetLabelStruct * pinsheet;
wxPoint itempos;

	switch ( layer )
		{
		case LAYER_BUS:
			item = PickStruct(pos, screen->EEDrawList, BUSITEM);
			if ( item ) return TRUE;
			pinsheet = LocateAnyPinSheet(pos, screen->EEDrawList );
			if ( pinsheet && IsBusLabel(pinsheet->m_Text.GetData()) )
			{
				itempos = pinsheet->m_Pos;
				if ( (itempos.x == pos.x) && (itempos.y == pos.y) )	return TRUE;
			}
			break;

		case LAYER_NOTES:
			item = PickStruct(pos, screen->EEDrawList, DRAWITEM);
			if ( item )
				return TRUE;
			break;

		case LAYER_WIRE:
			item = PickStruct(pos, screen->EEDrawList, RACCORDITEM |JUNCTIONITEM);
			if ( item ) return TRUE;

			pin = LocateAnyPin( screen->EEDrawList, pos, &LibItem );
			if ( pin && LibItem )
				{
				// calcul de la position exacte du point de connexion de la pin,
				// selon orientation du composant:
				itempos = LibItem->GetScreenCoord(pin->m_Pos);
				itempos.x += LibItem->m_Pos.x;
				itempos.y += LibItem->m_Pos.y;
				if ( (itempos.x == pos.x) && (itempos.y == pos.y) )	return TRUE;
				}

			item = PickStruct(pos, screen->EEDrawList, WIREITEM);
			if ( item ) return TRUE;

			item = PickStruct(pos, screen->EEDrawList, LABELITEM);
			if ( item && (item->m_StructType != DRAW_TEXT_STRUCT_TYPE) &&
				( ((DrawGlobalLabelStruct*)item)->m_Pos.x == pos.x) &&
				( ((DrawGlobalLabelStruct*)item)->m_Pos.y == pos.y) )
					return TRUE;

			pinsheet = LocateAnyPinSheet( pos, screen->EEDrawList );
			if ( pinsheet && ! IsBusLabel(pinsheet->m_Text.GetData()) )
			{
				itempos = pinsheet->m_Pos;
				if ( (itempos.x == pos.x) && (itempos.y == pos.y) )	return TRUE;
			}

			break;

		default:
			break;
		}

	return FALSE;
}


/****************************************************************/
bool IsJunctionNeeded (WinEDA_SchematicFrame * frame, wxPoint & pos )
/****************************************************************/
/* Return True when a wire is located at pos "pos" if
	- there is no junction.
	- The wire has no ends at pos "pos",
		and therefore it is considered as no connected.
	- One (or more) wire has one end at pos "pos"
	or
	- a pin is on location pos
*/
{
	if ( PickStruct(pos,frame->GetScreen()->EEDrawList, JUNCTIONITEM ) ) return FALSE;
	
	if ( PickStruct(pos,frame->GetScreen()->EEDrawList, WIREITEM |EXCLUDE_WIRE_BUS_ENDPOINTS ) )
	{
		if ( PickStruct(pos,frame->GetScreen()->EEDrawList, WIREITEM |WIRE_BUS_ENDPOINTS_ONLY ) )
			return TRUE;
		if ( frame->LocatePinEnd(frame->GetScreen()->EEDrawList, pos) )
			return TRUE;
	}

	return FALSE;
}

