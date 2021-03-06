/***************************************************/
/* export_gencad.cpp - export en formay GenCAD 1.4 */
/***************************************************/

#include "fctsys.h"

#include "common.h"
#include "pcbnew.h"
#include "trigo.h"

#include "bitmaps.h"
#include "protos.h"
#include "id.h"

bool CreateHeaderInfoData(FILE * file, WinEDA_PcbFrame * frame);
static int * CreateTracksInfoData(FILE * file, BOARD * pcb);
static void CreateBoardSection(FILE * file, BOARD * pcb);
static void CreateComponentsSection(FILE * file, BOARD * pcb);
static void CreateDevicesSection(FILE * file, BOARD * pcb);
static void CreateRoutesSection(FILE * file, BOARD * pcb);
static void CreateSignalsSection(FILE * file, BOARD * pcb);
static void CreateShapesSection(FILE * file, BOARD * pcb);
static void CreatePadsShapesSection(FILE * file, BOARD * pcb);
static void ModuleWriteShape( FILE * File, MODULE * module );

wxString GenCAD_Layer_Name[32]	// layer name pour extensions fichiers de trace
	= {
	"BOTTOM", "INNER1", "INNER2", "INNER3",
	"INNER4", "INNER5", "INNER6", "INNER7",
	"INNER8", "INNER9", "INNER10", "INNER11",
	"INNER12", "INNER13", "INNER14", "TOP",
	"adhecu", "adhecmp", "SOLDERPASTE_BOTTOM", "SOLDERPASTE_TOP",
	"SILKSCREEN_BOTTOM", "SILKSCREEN_TOP", "SOLDERMASK_BOTTOM", "SOLDERMASK_TOP",
	"drawings", "comments", "eco1", "eco2",
	"edges", "--", "--", "--",
	};

int offsetX, offsetY;
D_PAD * PadList;

/* routines de conversion des coord ( sous GenCAD axe Y vers le haut) */
static int mapXto(int x)
{
	return x - offsetX;
}
static int mapYto(int y)
{
	return offsetY - y;
}


/***********************************************************/
void WinEDA_PcbFrame::ExportToGenCAD(wxCommandEvent& event)
/***********************************************************/
/*
Exporte le board au format GenCAD 1.4
*/
{
wxString FullFileName = GetScreen()->m_FileName;
wxString msg, std_ext, mask;
FILE * file;
	
	std_ext = ".gcd";
	mask = "*" + std_ext;
	ChangeFileNameExt(FullFileName,std_ext);
	FullFileName = EDA_FileSelector(_("GenCAD file:"),
					"",						/* Chemin par defaut */
					FullFileName,	 		/* nom fichier par defaut */
					std_ext,					/* extension par defaut */
					mask,				/* Masque d'affichage */
					this,
					wxOPEN,
					FALSE
					);
	if ( FullFileName == "" ) return;
		
	if ( (file = fopen(FullFileName.GetData(), "wt" )) == NULL )
	{
		msg = _("Unable to create ") + FullFileName;
		DisplayError(this, msg); return;
	}

	/* Mise a jour des infos PCB: */
	m_Pcb->ComputeBoundaryBox();

	offsetX = m_Auxiliary_Axe_Position.x;
	offsetY = m_Auxiliary_Axe_Position.y;
wxClientDC dc(DrawPanel);
	DrawPanel->PrepareGraphicContext(&dc);
	Compile_Ratsnest( &dc, TRUE );
	
	/* Mise des modules vus en miroir en position "normale"
		(necessaire pour decrire les formes sous GenCAD,
		qui sont decrites en vue normale, orientation 0)) */
MODULE * module;
	for(module = m_Pcb->m_Modules; module != NULL; module = module->Next())
	{
		module->flag = 0;
		if ( module->m_Layer == CUIVRE_N )
		{
			Change_Side_Module(module, NULL);
			module->flag = 1;
		}
	}
	// Creation de l'entete:
	CreateHeaderInfoData(file, this);
	CreateBoardSection(file, m_Pcb);

	/* Creation liste des TRACKS
	(section $TRACK) id liste des outils de tracage de pistes */
	CreateTracksInfoData(file, m_Pcb);

	/* Creation de la liste des formes utilisees
	(formes des composants principalement */
	CreatePadsShapesSection(file, m_Pcb);	// doit etre appele avant CreateShapesSection()
	CreateShapesSection( file, m_Pcb);
	
	/* Creation de la liste des equipotentielles: */
	CreateSignalsSection(file, m_Pcb);
	
	CreateDevicesSection(file, m_Pcb);
	CreateComponentsSection(file, m_Pcb);
	CreateRoutesSection(file, m_Pcb);

	fclose(file);

	/* Remise en place des modules vus en miroir */
	for(module = m_Pcb->m_Modules; module != NULL; module = module->Next())
	{
		if ( module->flag )
		{
			Change_Side_Module(module, NULL);
			module->flag = 0;
		}
	}
}


/**************************************************************************/
static int Pad_list_Sort_by_Shapes(const void * refptr, const void * objptr)
/**************************************************************************/
/*
Routine de tri de la liste des pads par type, puis pa taille
*/
{
const D_PAD * padref, *padcmp;
int diff;
	
	padref = *((D_PAD**) refptr);
	padcmp = *((D_PAD**) objptr);
	if( (diff = padref->m_PadShape - padcmp->m_PadShape) ) return diff;
	if( (diff = padref->m_Size.x - padcmp->m_Size.x) ) return diff;
	if( (diff = padref->m_Size.y - padcmp->m_Size.y) ) return diff;
	if( (diff = padref->m_Offset.x - padcmp->m_Offset.x) ) return diff;
	if( (diff = padref->m_Offset.y - padcmp->m_Offset.y) ) return diff;
	if( (diff = padref->m_DeltaSize.x - padcmp->m_DeltaSize.x) ) return diff;
	if( (diff = padref->m_DeltaSize.y - padcmp->m_DeltaSize.y) ) return diff;

	return 0;
}

/*****************************************************/
void CreatePadsShapesSection(FILE * file, BOARD * pcb)
/*****************************************************/
/* Cree la liste des formes des pads ( 1 forme par pad )
initialise le membre .m_logical_connexion de la struct pad, la valeur 1 ..n
pour les formes de pad PAD1 a PADn
*/
{
D_PAD * pad, **padlist, **pad_list_base;
char * pad_type;	
int memsize, ii, dx, dy;
D_PAD * old_pad = NULL;
int pad_name_number;
	
	fputs("$PADS\n", file);

	// Generation de la liste des pads tries par forme et dimensions:
	memsize = (pcb->m_NbPads + 1) * sizeof(D_PAD*);
	pad_list_base = (D_PAD**) MyZMalloc(memsize);
	memcpy(pad_list_base, pcb->m_Pads, memsize);
	qsort(pad_list_base, pcb->m_NbPads, sizeof(D_PAD*), Pad_list_Sort_by_Shapes);
	
	pad_name_number = 0;
	for( padlist = pad_list_base, ii = 0; ii < pcb->m_NbPads; padlist++, ii++)
	{
		pad = *padlist;
		pad->m_logical_connexion = pad_name_number;

		if ( old_pad &&
			(old_pad->m_PadShape == pad->m_PadShape) &&
			(old_pad->m_Size.x == pad->m_Size.x) &&
			(old_pad->m_Size.y == pad->m_Size.y) &&
			(old_pad->m_Offset.x == pad->m_Offset.x) &&
			(old_pad->m_Offset.y == pad->m_Offset.y) &&
			(old_pad->m_DeltaSize.x == pad->m_DeltaSize.x) &&
			(old_pad->m_DeltaSize.y == pad->m_DeltaSize.y)
			)
			continue;	// Forme deja generee

		old_pad = pad;

		pad_name_number ++;
		pad->m_logical_connexion = pad_name_number;

		fprintf(file, "PAD PAD%d", pad->m_logical_connexion);

		dx = pad->m_Size.x / 2; dy = pad->m_Size.y / 2;

		switch( pad->m_PadShape )
		{
			default:
			case CIRCLE:
				pad_type = "ROUND";
				fprintf(file, " %s %d\n", pad_type, pad->m_Drill);
				fprintf(file, "CIRCLE %d %d %d\n",
					pad->m_Offset.x, - pad->m_Offset.y, pad->m_Size.x/2);
				break;
			
			case RECT:
				pad_type = "RECTANGULAR";
				fprintf(file, " %s %d\n", pad_type, pad->m_Drill);
				fprintf(file, "RECTANGLE %d %d %d %d\n",
					-dx + pad->m_Offset.x, - dy - pad->m_Offset.y,
					dx + pad->m_Offset.x, - pad->m_Offset.y + dy );
				break;
			
			case OVALE: /* description du contour par 2 linges et 2 arcs */
			{
				pad_type = "FINGER";
				fprintf(file, " %s %d\n", pad_type, pad->m_Drill);
				int dr = dx - dy;
				if ( dr >= 0 )	// ovale horizontal
				{
				int rayon = dy;
				fprintf(file, "LINE %d %d %d %d\n",
					-dr + pad->m_Offset.x, - pad->m_Offset.y - rayon,
					dr + pad->m_Offset.x, - pad->m_Offset.y - rayon );
				fprintf(file, "ARC %d %d %d %d %d %d\n",
					dr + pad->m_Offset.x, - pad->m_Offset.y - rayon,
					dr + pad->m_Offset.x, - pad->m_Offset.y + rayon,
					dr + pad->m_Offset.x, - pad->m_Offset.y);

				fprintf(file, "LINE %d %d %d %d\n",
					dr + pad->m_Offset.x, - pad->m_Offset.y + rayon,
					-dr + pad->m_Offset.x, - pad->m_Offset.y + rayon );
				fprintf(file, "ARC %d %d %d %d %d %d\n",
					-dr + pad->m_Offset.x, - pad->m_Offset.y + rayon,
					-dr + pad->m_Offset.x, - pad->m_Offset.y - rayon,
					-dr + pad->m_Offset.x, - pad->m_Offset.y);
				}
				else	// ovale vertical
				{
				dr = -dr;
				int rayon = dx;
				fprintf(file, "LINE %d %d %d %d\n",
					-rayon + pad->m_Offset.x, - pad->m_Offset.y - dr,
					-rayon + pad->m_Offset.x, - pad->m_Offset.y + dr );
				fprintf(file, "ARC %d %d %d %d %d %d\n",
					-rayon + pad->m_Offset.x, - pad->m_Offset.y + dr,
					rayon + pad->m_Offset.x, - pad->m_Offset.y + dr,
					pad->m_Offset.x, - pad->m_Offset.y + dr);

				fprintf(file, "LINE %d %d %d %d\n",
					rayon + pad->m_Offset.x, - pad->m_Offset.y + dr,
					rayon + pad->m_Offset.x, - pad->m_Offset.y - dr );
				fprintf(file, "ARC %d %d %d %d %d %d\n",
					rayon + pad->m_Offset.x, - pad->m_Offset.y - dr,
					- rayon + pad->m_Offset.x, - pad->m_Offset.y - dr,
					pad->m_Offset.x, - pad->m_Offset.y - dr);
				}
				break;
			}
			
			case TRAPEZE:
				pad_type = "POLYGON";
				break;
		}
	}
	
	fputs("$ENDPADS\n\n", file);
	MyFree(pad_list_base);
}


/**************************************************/
void CreateShapesSection(FILE * file, BOARD * pcb)
/**************************************************/
/* Creation de la liste des formes des composants.
Comme la forme de base (module de librairie peut etre modifiee,
une forme est creee par composant
La forme est donnee normalisee, c'est a dire orientation 0, position 0 non miroir
Il y aura donc des formes indentiques redondantes

	Syntaxe:
$SHAPES
SHAPE <shape_name>
shape_descr (line, arc ..)
PIN <pin_name> <pad_name> <x_y_ref> <layer> <rot> <mirror>

SHAPE <shape_name>
..
$ENDSHAPES
*/
{
MODULE * module;
D_PAD * pad;
char * layer;
int orient;
char pinname[128];
char * mirror = "0";
	
	fputs("$SHAPES\n", file);

	for(module = pcb->m_Modules; module != NULL; module = (MODULE *) module->Pnext)
	{
		ModuleWriteShape( file, module );
		for( pad = module->m_Pads; pad != NULL; pad = (D_PAD*) pad->Pnext)
		{
		layer = "ALL";
		if ( (pad->m_Masque_Layer & ALL_CU_LAYERS) == CUIVRE_LAYER )
		{
			if ( module->m_Layer == CMP_N ) layer = "BOTTOM";
			else layer = "TOP";
		}
		else if ( (pad->m_Masque_Layer & ALL_CU_LAYERS) == CMP_LAYER )
		{
			if ( module->m_Layer == CMP_N ) layer = "TOP";
			else layer = "BOTTOM";
		}

		pad->ReturnStringPadName(pinname);
		if( strlen(pinname) == 0 ) strcpy(pinname, "noname");
			
		orient = pad->m_Orient - module->m_Orient;
		NORMALIZE_ANGLE_POS(orient);
		fprintf(file, "PIN %s PAD%d %d %d %s %d %s",
			pinname, pad->m_logical_connexion,
			pad->m_Pos0.x, - pad->m_Pos0.y,
			layer, orient/10, mirror );
		if ( orient % 10 ) fprintf(file, " .%d", orient % 10 );
		fprintf(file, "\n");
		}
	}
	
	fputs("$ENDSHAPES\n\n", file);
}

/******************************************************/
void CreateComponentsSection(FILE * file, BOARD * pcb)
/******************************************************/
/* Creation de la section $COMPONENTS (Placement des composants
	Composants cote CUIVRE:
	Les formes sont donnees avec l'option "FLIP", c.a.d.:
	- ils sont decrits en vue normale (comme s'ils etaient sur cote COMPOSANT)
	- leur orientation est donn�e comme s'ils etaient cote composant.
*/
{
MODULE * module = pcb->m_Modules;
TEXTE_MODULE * PtTexte;
char * mirror;
char * flip;
int ii;
	
	fputs("$COMPONENTS\n", file);

	for(; module != NULL; module = (MODULE *) module->Pnext)
	{
	int orient = module->m_Orient;
		if (module->flag)
		{
			mirror = "MIRRORX";		// Miroir selon axe X
			flip = "FLIP";			// Description normale ( formes a afficher en miroir X)
			NEGATE_AND_NORMALIZE_ANGLE_POS(orient);
		}
		else
		{
			mirror ="0";
			flip = "0";
		}
			
		fprintf(file, "COMPONENT %s\n", module->m_Reference->GetText());
		fprintf(file, "DEVICE %s\n", module->m_Reference->GetText());
		fprintf(file, "PLACE %d %d\n", mapXto(module->m_Pos.x), mapYto(module->m_Pos.y));
		fprintf(file, "LAYER %s\n", (module->flag)? "BOTTOM" : "TOP");

		fprintf(file, "ROTATION %d", orient/10 );
		if (orient%10 ) fprintf(file, ".%d", orient%10 );
		fputs("\n",file);

		fprintf(file, "SHAPE %s %s %s\n",
			module->m_Reference->GetText(), mirror, flip);
		
		/* Generation des elements textes (ref et valeur seulement) */
		PtTexte = module->m_Reference;
		for ( ii = 0; ii < 2; ii++ )
		{
			int orient = PtTexte->m_Orient;
			const char * layer = GenCAD_Layer_Name[SILKSCREEN_N_CMP].GetData();
			fprintf(file, "TEXT %d %d %d %d.%d %s %s \"%s\"",
				PtTexte->m_Pos0.x, - PtTexte->m_Pos0.y,
				PtTexte->m_Size.x,
				orient /10, orient %10,
				mirror, 
				layer,
				PtTexte->GetText()
			);
		
				
			fprintf(file, " 0 0 %d %d\n",
				PtTexte->m_Size.x * strlen(PtTexte->GetText()),
				PtTexte->m_Size.y );
				
			PtTexte = module->m_Value;
		}
		
		// commentaire:
		fprintf(file, "SHEET Part %s %s\n", module->m_Reference->GetText(),
					 module->m_Value->GetText());
		
	}
	
	fputs("$ENDCOMPONENTS\n\n", file);
}

/***************************************************/
void CreateSignalsSection(FILE * file, BOARD * pcb)
/***************************************************/
/* Creation de la liste des equipotentielles:
	$SIGNALS
		SIGNAL <equipot name>
		NODE <component name> <pin name>
		...
		NODE <component name> <pin name>
	$ENDSIGNALS
*/
{
wxString msg;
EQUIPOT * equipot;
D_PAD * pad;
MODULE * module;
int NbNoConn = 1;
	
	fputs("$SIGNALS\n", file);
	
	for( equipot = pcb->m_Equipots; equipot != NULL; equipot = (EQUIPOT *) equipot->Pnext )
	{
		if ( equipot->m_Netname == "" ) 	// dummy equipot (non connexion)
		{
			equipot->m_Netname << "NoConnection" << NbNoConn++;
		}
		
		if ( equipot->m_NetCode <= 0 )		// dummy equipot (non connexion)
			continue;
		
		msg = "\nSIGNAL " + equipot->m_Netname;
		fputs(msg.GetData(), file); fputs("\n", file);
		
		for ( module = pcb->m_Modules; module != NULL; module = (MODULE *) module->Pnext )
		{
			for( pad = module->m_Pads; pad != NULL; pad = (D_PAD *) pad->Pnext )
			{
				if ( pad->m_NetCode != equipot->m_NetCode ) continue;
				msg.Printf("NODE %s %.4s",
					module->m_Reference->GetText(), pad->m_Padname);
				fputs(msg.GetData(), file); fputs("\n", file);
			}
		}
	}
	
	fputs("$ENDSIGNALS\n\n", file);

}

/*************************************************************/
bool CreateHeaderInfoData(FILE * file, WinEDA_PcbFrame * frame)
/*************************************************************/
/* Creation de la section $HEADER ... $ENDHEADER
*/
{
wxString msg;
PCB_SCREEN * screen = frame->GetScreen();
	
	fputs("$HEADER\n", file);
	fputs("GENCAD 1.4\n", file);
	msg = "USER " + Main_Title; fputs(msg.GetData(), file); fputs("\n", file);
	msg = "DRAWING " + screen->m_FileName;
	fputs(msg.GetData(), file); fputs("\n", file);
	msg = "REVISION " + screen->m_Revision + " " + screen->m_Date;
	fputs(msg.GetData(), file); fputs("\n", file);
	msg.Printf("UNITS USER %d", PCB_INTERNAL_UNIT);
	fputs(msg.GetData(), file); fputs("\n", file);
	msg.Printf("ORIGIN %d %d", mapXto(frame->m_Auxiliary_Axe_Position.x),
				mapYto(frame->m_Auxiliary_Axe_Position.y)) ;
	fputs(msg.GetData(), file); fputs("\n", file);
	fputs("INTERTRACK 0\n", file);
	fputs("$ENDHEADER\n\n", file);
	
	return TRUE;
}


/**************************************************************************/
static int Track_list_Sort_by_Netcode(const void * refptr, const void * objptr)
/**************************************************************************/
/*
Routine de tri de la liste des piste par netcode,
	puis par largeur puis par layer
*/
{
const TRACK * ref, *cmp;
int diff;
	
	ref = *((TRACK**) refptr);
	cmp = *((TRACK**) objptr);
	if( (diff = ref->m_NetCode - cmp->m_NetCode) ) return diff;
	if( (diff = ref->m_Width - cmp->m_Width) ) return diff;
	if( (diff = ref->m_Layer - cmp->m_Layer) ) return diff;

	return 0;
}
/*************************************************/
void CreateRoutesSection(FILE * file, BOARD * pcb)
/*************************************************/
/* Creation de la liste des pistes, vias et zones
section:
$ROUTE
...
$ENROUTE
Les segments de piste doivent etre regroupes par nets
*/
{
TRACK * track, ** tracklist;
int vianum = 1;
int old_netcode, old_width, old_layer;
int nbitems, ii;
	
	// Calcul du nombre de segments a ecrire
	nbitems = 0;
	for( track = pcb->m_Track; track != NULL; track = (TRACK*) track->Pnext ) nbitems++;
	for( track = pcb->m_Zone; track != NULL; track = (TRACK*) track->Pnext )
	{
		if ( track->m_StructType == TYPEZONE ) nbitems++;
	}
	tracklist = (TRACK **) MyMalloc( (nbitems+1) * sizeof(TRACK *) );
	
	nbitems = 0;
	for( track = pcb->m_Track; track != NULL; track = (TRACK*) track->Pnext )
		tracklist[nbitems++] = track;
	for( track = pcb->m_Zone; track != NULL; track = (TRACK*) track->Pnext )
	{
		if ( track->m_StructType == TYPEZONE ) tracklist[nbitems++] = track;
	}
	tracklist[nbitems] = NULL;

	qsort(tracklist, nbitems, sizeof(TRACK *), Track_list_Sort_by_Netcode);
	
	fputs("$ROUTES\n", file);

	old_netcode = -1; old_width = -1; old_layer = -1;
	for( ii = 0; ii < nbitems; ii++ )
	{
		track = tracklist[ii];
		if ( old_netcode != track->m_NetCode )
		{
			old_netcode = track->m_NetCode;
			EQUIPOT * equipot = GetEquipot( pcb, track->m_NetCode);
			fprintf(file,"\nROUTE %s\n",
				(equipot && (equipot->m_Netname != "") ) ? equipot->m_Netname.GetData() : "_noname_" );
		}

		if ( old_width != track->m_Width )
		{
			old_width = track->m_Width;
			fprintf(file,"TRACK TRACK%d\n", track->m_Width);
		}
		
		if ( (track->m_StructType == TYPETRACK) || (track->m_StructType == TYPEZONE) )
		{
			if ( old_layer != track->m_Layer )
			{
				old_layer = track->m_Layer;
				fprintf(file,"LAYER %s\n",
					GenCAD_Layer_Name[track->m_Layer & 0x1F].GetData());
			}
				
			fprintf(file, "LINE %d %d %d %d\n",
				mapXto(track->m_Start.x), mapYto(track->m_Start.y),
				mapXto(track->m_End.x), mapYto(track->m_End.y) );
		}
		if ( track->m_StructType == TYPEVIA )
		{
			fprintf(file, "VIA viapad%d %d %d ALL %d via%d\n",
				track->m_Width,
				mapXto(track->m_Start.x), mapYto(track->m_Start.y),
				g_DesignSettings.m_ViaDrill, vianum++ );
		}
	}

	fputs("$ENDROUTES\n\n", file);

	free(tracklist);
}


/***************************************************/
void CreateDevicesSection(FILE * file, BOARD * pcb)
/***************************************************/
/* Creation de la section de description des proprietes des composants
	( la forme des composants est dans la section shape )
*/
{
MODULE * module;
D_PAD * pad;
	
	fputs("$DEVICES\n", file);

	for(module = pcb->m_Modules; module != NULL; module = (MODULE *) module->Pnext)
	{
		fprintf(file, "DEVICE %s\n", module->m_Reference->GetText());
		fprintf(file, "PART %s\n", module->m_LibRef.GetData());
		fprintf(file, "TYPE %s\n", "UNKNOWN");
		for( pad = module->m_Pads; pad != NULL; pad = (D_PAD*) pad->Pnext)
		{
		fprintf(file, "PINDESCR %.4s", pad->m_Padname);
		if ( pad->m_Netname == "" ) fputs(" NoConn\n", file);
		else fprintf(file, " %.4s\n", pad->m_Padname);
		}
		fprintf(file, "ATTRIBUTE %s\n", module->m_Value->GetText());
	}
	
	fputs("$ENDDEVICES\n\n", file);
}


/*************************************************/
void CreateBoardSection(FILE * file, BOARD * pcb)
/*************************************************/
/* Creation de la section $BOARD.
On ne cree ici que le rectangle d'encadrement du Board
*/
{
	fputs("$BOARD\n", file);
	fprintf(file, "LINE %d %d %d %d\n",
				mapXto(pcb->m_BoundaryBox.m_Pos.x), mapYto(pcb->m_BoundaryBox.m_Pos.y),
				mapXto(pcb->m_BoundaryBox.GetRight()), mapYto(pcb->m_BoundaryBox.m_Pos.y) );
	fprintf(file, "LINE %d %d %d %d\n",
				mapXto(pcb->m_BoundaryBox.GetRight()), mapYto(pcb->m_BoundaryBox.m_Pos.y),
				mapXto(pcb->m_BoundaryBox.GetRight()), mapYto(pcb->m_BoundaryBox.GetBottom()) );
	fprintf(file, "LINE %d %d %d %d\n",
				mapXto(pcb->m_BoundaryBox.GetRight()), mapYto(pcb->m_BoundaryBox.GetBottom()),
				mapXto(pcb->m_BoundaryBox.m_Pos.x), mapYto(pcb->m_BoundaryBox.GetBottom()) );
	fprintf(file, "LINE %d %d %d %d\n",
				mapXto(pcb->m_BoundaryBox.m_Pos.x), mapYto(pcb->m_BoundaryBox.GetBottom()),
				mapXto(pcb->m_BoundaryBox.m_Pos.x), mapYto(pcb->m_BoundaryBox.m_Pos.y) );

	fputs("$ENDBOARD\n\n", file);
}


/****************************************************/
int * CreateTracksInfoData(FILE * file, BOARD * pcb)
/****************************************************/
/* Creation de la section "$TRACKS"
Cette section definit les largeurs de pistes utilsees
format:
$TRACK
TRACK <name> <width>
$ENDTRACK

on attribut ici comme nom l'epaisseur des traits precede de "TRACK": ex
pour une largeur de 120 : nom = "TRACK120".
*/
{
TRACK * track;
int * trackinfo, * ptinfo;
	
	/* recherche des epaisseurs utilisees pour les traces: */
	
	trackinfo = (int*) adr_lowmem;
	*trackinfo = -1;

	for( track = pcb->m_Track; track != NULL; track = (TRACK*) track->Pnext )
	{
		if ( *trackinfo != track->m_Width ) // recherche d'une epaisseur deja utilisee
		{
			ptinfo = (int*) adr_lowmem;
			while (*ptinfo >= 0 )
			{
				if ( *ptinfo != track->m_Width ) ptinfo ++;
				else break;
			}
			trackinfo = ptinfo;
			if ( *ptinfo < 0 )
			{
				*ptinfo = track->m_Width;
				ptinfo++; *ptinfo = -1;
			}
		}
	}
	for( track = pcb->m_Zone; track != NULL; track = (TRACK*) track->Pnext )
	{
		if ( *trackinfo != track->m_Width ) // recherche d'une epaisseur deja utilisee
		{
			ptinfo = (int*) adr_lowmem;
			while (*ptinfo >= 0 )
			{
				if ( *ptinfo != track->m_Width ) ptinfo ++;
				else break;
			}
			trackinfo = ptinfo;
			if ( *ptinfo < 0 )
			{
				*ptinfo = track->m_Width;
				ptinfo++; *ptinfo = -1;
			}
		}
	}

	// Write data
	fputs("$TRACKS\n", file);
	for( trackinfo = (int*) adr_lowmem; * trackinfo >= 0; trackinfo++ )
	{
		fprintf(file,"TRACK TRACK%d %d\n", * trackinfo, * trackinfo);
	}		
	fputs("$ENDTRACKS\n\n", file);
	
	return (int*) adr_lowmem;
}


/***************************************************/
void ModuleWriteShape( FILE * file, MODULE * module )
/***************************************************/
/* Sauvegarde de la forme d'un MODULE (section SHAPE)
	La forme est donnee "normalisee" (Orient 0, vue normale ( non miroir)
	Syntaxe:
SHAPE <shape_name>
	shape_descr (line, arc ..):
LINE startX startY endX endY
ARC startX startY endX endY centreX scentreY
CIRCLE centreX scentreY radius
*/
{
EDGE_MODULE * PtEdge;
EDA_BaseStruct * PtStruct;
int Yaxis_sign = -1;	// Controle changement signe axe Y (selon module normal/miroir et conevtions d'axe)
	
	
	/* Generation du fichier module: */
	fprintf(file, "\nSHAPE %s\n", module->m_Reference->GetText());

	/* Attributs du module */
	if( module->m_Attributs != MOD_DEFAULT )
	{
		fprintf(file,"ATTRIBUTE");
		if( module->m_Attributs & MOD_CMS ) fprintf(file," SMD");
		if( module->m_Attributs & MOD_VIRTUAL ) fprintf(file," VIRTUAL");
		fprintf(file,"\n");
	}


	/* Generation des elements Drawing modules */
	PtStruct = module->m_Drawings;
	for( ; PtStruct != NULL; PtStruct = PtStruct->Pnext)
	{
		switch(PtStruct->m_StructType )
		{
			case TYPETEXTEMODULE:
				break;

			case TYPEEDGEMODULE:
				PtEdge = (EDGE_MODULE*) PtStruct;
				switch(PtEdge->m_Shape )
				{
					case S_SEGMENT:
						fprintf(file,"LINE %d %d %d %d\n",
									PtEdge->m_Start0.x, Yaxis_sign * PtEdge->m_Start0.y,
									PtEdge->m_End0.x, Yaxis_sign * PtEdge->m_End0.y);
						break;
					case S_CIRCLE:
					{
						int rayon = (int)hypot(
							(double)(PtEdge->m_End0.x - PtEdge->m_Start0.x),
							(double)(PtEdge->m_End0.y - PtEdge->m_Start0.y) );
						fprintf(file,"CIRCLE %d %d %d\n",
									PtEdge->m_Start0.x, Yaxis_sign * PtEdge->m_Start0.y,
									rayon);
						break;
					}

					case S_ARC:	/* print ARC x,y start x,y end x,y centre */
					{
						int arcendx, arcendy;
						arcendx = PtEdge->m_Start0.x;
						arcendy = PtEdge->m_Start0.y;
						RotatePoint(& arcendx, &arcendy, PtEdge->m_Angle);
						fprintf(file,"ARC %d %d %d %d %d %d\n",
									PtEdge->m_End0.x, Yaxis_sign * PtEdge->m_End0.y,
									arcendx, Yaxis_sign * arcendy,
									PtEdge->m_Start0.x, Yaxis_sign * PtEdge->m_Start0.y);
						break;
					}
					default:
						DisplayError(NULL, "Type Edge Module inconnu");
						break;
				}	/* Fin switch type edge */
				break;

			default:
				break;
		}	/* Fin switch gestion des Items draw */
	}
}

