		/**********************************************************/
		/* Routines d'affichage de parametres et caracteristiques */
		/**********************************************************/

#include "fctsys.h"
#include "gr_basic.h"
#include "common.h"
#include "pcbnew.h"
#include "autorout.h"

#include "3d_struct.h"

#include "protos.h"

/* Routines locales */

/*****************************************************************************/
void Affiche_Infos_PCB_Texte(WinEDA_BasePcbFrame * frame, TEXTE_PCB* pt_texte)
/*****************************************************************************/
/* Affiche en bas d'ecran les caract du texte sur PCB
	Entree :
		pointeur de la description du texte
*/
{
	frame->MsgPanel->EraseMsgBox();
	if ( pt_texte == NULL ) return;

	if( pt_texte->m_StructType == TYPECOTATION )
		Affiche_1_Parametre(frame, 1,_("COTATION"),pt_texte->m_Text, DARKGREEN);

	else
		Affiche_1_Parametre(frame, 1,_("PCB Text"),pt_texte->m_Text, DARKGREEN);

	Affiche_1_Parametre(frame, 28,_("Layer"),
					ReturnPcbLayerName(pt_texte->m_Layer),
					g_DesignSettings.m_LayerColor[pt_texte->m_Layer]  & MASKCOLOR);

	Affiche_1_Parametre(frame, 36,_("Mirror"),"",GREEN) ;
	if( (pt_texte->m_Miroir & 1) )
			Affiche_1_Parametre(frame, -1,"",_("No"), DARKGREEN) ;
	else	Affiche_1_Parametre(frame, -1,"",_("Yes"), DARKGREEN) ;


	sprintf(cbuf,"%.1f",(float)pt_texte->m_Orient/10 );
	Affiche_1_Parametre(frame, 43,_("Orient"),cbuf, DARKGREEN) ;

	valeur_param(pt_texte->m_Width,cbuf) ;
	Affiche_1_Parametre(frame, 50,_("Width"),cbuf,MAGENTA) ;

	valeur_param(pt_texte->m_Size.x,cbuf) ;
	Affiche_1_Parametre(frame, 60,_("H Size"),cbuf,RED) ;

	valeur_param(pt_texte->m_Size.y,cbuf);
	Affiche_1_Parametre(frame, 70,_("V Size"),cbuf,RED) ;
}

/************************************************************************/
void Affiche_Infos_E_Texte(WinEDA_BasePcbFrame * frame, MODULE* Module,
			TEXTE_MODULE* pt_texte)
/************************************************************************/
/* Affiche en bas d'ecran les caract du texte sur empreinte
	Entree :
		pointeur de description sur le module
		pointeur de la description du texte
*/
{
wxString text_type_msg[3] = {_("Ref."),_("Value"),_("Text")};
char Line[1024];
int ii;

	frame->MsgPanel->EraseMsgBox();

	strcpy(Line,Module->m_Reference->GetText());
	Line[8] = 0;
	Affiche_1_Parametre(frame, 1,"Module", Line,  DARKCYAN) ;

	strncpy(Line,pt_texte->GetText(),8) ; Line[8] = 0 ;
	Affiche_1_Parametre(frame, 10,_("Text"),Line,YELLOW) ;

	ii = pt_texte->m_Type; if (ii > 2) ii = 2;
	Affiche_1_Parametre(frame, 20,_("Type"),text_type_msg[ii], DARKGREEN) ;

	Affiche_1_Parametre(frame, 25,_("Display"),"", DARKGREEN);
	if(pt_texte->m_NoShow)
		 Affiche_1_Parametre(frame, -1,"",_("No"), DARKGREEN);
	else Affiche_1_Parametre(frame, -1,"",_("Yes"), DARKGREEN);

	ii = pt_texte->m_Layer;
	if ( ii <= 28)
		Affiche_1_Parametre(frame, 28,_("Layer"),ReturnPcbLayerName(ii), DARKGREEN);
	else
		{
		sprintf(cbuf,"%d", ii);
		Affiche_1_Parametre(frame, 28,_("Layer"),cbuf, DARKGREEN) ;
		}

	strcpy(cbuf," Yes");
	if( (pt_texte->m_Miroir & 1) ) strcpy(cbuf," No");
	Affiche_1_Parametre(frame, 36,_("Mirror"),cbuf, DARKGREEN) ;

	sprintf(cbuf,"%.1f",(float)pt_texte->m_Orient / 10 );
	Affiche_1_Parametre(frame, 42,_("Orient"),cbuf, DARKGREEN) ;

	valeur_param(pt_texte->m_Width,cbuf) ;
	Affiche_1_Parametre(frame, 48,_("Width"),cbuf, DARKGREEN) ;

	valeur_param(pt_texte->m_Size.x,cbuf) ;
	Affiche_1_Parametre(frame, 56,_("H Size"),cbuf,RED) ;

	valeur_param(pt_texte->m_Size.y,cbuf);
	Affiche_1_Parametre(frame, 64,_("V Size"),cbuf,RED) ;

}



/*********************************************************************/
void Affiche_Infos_Piste(WinEDA_BasePcbFrame * frame, TRACK * pt_piste)
/********************************************************************/
/* Affiche les caract principales d'un segment de piste en bas d'ecran
*/
{
wxString msg;
int text_pos;
	
	frame->MsgPanel->EraseMsgBox();

	switch(pt_piste->m_StructType)
		{
		case TYPEVIA:
			msg = g_ViaType_Name[pt_piste->m_Shape & 255];
			break;

		case TYPETRACK:
			msg = _("Track") ;
			break;

		case TYPEZONE:
			msg = _("Zone"); break;

		default:
			msg = "????"; break;
		}
	text_pos = 1;
	Affiche_1_Parametre(frame, text_pos,_("Type"),msg, DARKCYAN);

	/* Affiche NetName pour les segments de piste type cuivre */
	text_pos += 15;
	if( ( pt_piste->m_StructType == TYPETRACK ) ||
		( pt_piste->m_StructType == TYPEZONE ) ||
		( pt_piste->m_StructType == TYPEVIA ) )
		{
		EQUIPOT * equipot = GetEquipot( frame->m_Pcb, pt_piste->m_NetCode);
		if ( equipot )
			{
			msg = equipot->m_Netname;
			}
		else msg = "<noname>";
		Affiche_1_Parametre(frame, text_pos,_("NetName"),msg,RED) ;

		/* Affiche net code :*/
		sprintf(cbuf,"%d .%d",pt_piste->m_NetCode, pt_piste->m_Sous_Netcode);
		text_pos += 18;
		Affiche_1_Parametre(frame, text_pos,_("NetCode"),cbuf,RED) ;
		}

	else
		{
		Affiche_1_Parametre(frame, text_pos, _("Segment"),"",RED) ;
		if(pt_piste->m_Shape == S_CIRCLE)
			Affiche_1_Parametre(frame, -1,"",_("Circle"),RED) ;
		else Affiche_1_Parametre(frame, -1,"",_("Standard"),RED) ;
		}


	/* Affiche les flags Status piste */
	strcpy(cbuf,". . ");
	if(pt_piste->GetState(SEGM_FIXE) ) cbuf[0] = 'F';
	if(pt_piste->GetState(SEGM_AR) ) cbuf[2] = 'A';
	text_pos = 42;
	Affiche_1_Parametre(frame, text_pos,_("Stat"),cbuf,MAGENTA);

	/* Affiche Layer(s) */
	if ( pt_piste->m_StructType == TYPEVIA)
		{
		SEGVIA * Via = (SEGVIA *) pt_piste;
		int top_layer, bottom_layer;
		Via->ReturnLayerPair(&top_layer, &bottom_layer);
		msg = ReturnPcbLayerName(top_layer, TRUE) + "/" + ReturnPcbLayerName(bottom_layer, TRUE);
		}
	else msg = ReturnPcbLayerName(pt_piste->m_Layer);

	text_pos += 5;
	Affiche_1_Parametre(frame, text_pos, _("Layer"), msg, BROWN) ;

	/* Affiche Epaisseur */
	valeur_param((unsigned)(pt_piste->m_Width), cbuf) ;
	text_pos += 11;
	if (pt_piste->m_StructType == TYPEVIA )	// Display Diam and Drill values
	{
		Affiche_1_Parametre(frame, text_pos,_("Diam"),cbuf, DARKCYAN);

		int drill_value = (pt_piste->m_Drill >= 0 ) ?
			pt_piste->m_Drill : g_DesignSettings.m_ViaDrill;
		valeur_param((unsigned)drill_value, cbuf);
		text_pos += 8;
		wxString title =  _("Drill");
		if ( g_DesignSettings.m_ViaDrill >= 0 ) title += "*";
		Affiche_1_Parametre(frame, text_pos,_("Drill"),cbuf, RED);
	}
	else Affiche_1_Parametre(frame, text_pos,_("Width"),cbuf, DARKCYAN) ;
}

/************************************************************/
void Affiche_Infos_DrawSegment(WinEDA_BasePcbFrame * frame,
		DRAWSEGMENT * DrawSegment)
/************************************************************/
/* Affiche les caract principales d'un segment type drawind PCB en bas d'ecran */
{
int itype;

	frame->MsgPanel->EraseMsgBox();

	itype = DrawSegment->m_Type & 0x0F;

	strcpy(cbuf,"DRAWING");
	if( DrawSegment->m_StructType == TYPECOTATION ) strcpy(cbuf,"COTATION");

	Affiche_1_Parametre(frame, 1,_("Type"),cbuf, DARKCYAN) ;

	Affiche_1_Parametre(frame, 16,_("Shape"),"",RED) ;
	if(DrawSegment->m_Shape == S_CIRCLE)
		Affiche_1_Parametre(frame, -1,"",_("Circle"),RED) ;
	else if (DrawSegment->m_Shape == S_ARC)
		{
		Affiche_1_Parametre(frame, -1,"","  arc  ",RED) ;
		sprintf(cbuf,"%d",DrawSegment->m_Angle);
		Affiche_1_Parametre(frame, 32," l.arc ",cbuf,RED) ;
		}
	else Affiche_1_Parametre(frame, -1,"","segment",RED) ;

	Affiche_1_Parametre(frame, 48,_("Layer"),ReturnPcbLayerName(DrawSegment->m_Layer),BROWN) ;

	/* Affiche Epaisseur */
	valeur_param((unsigned)(DrawSegment->m_Width), cbuf) ;
	Affiche_1_Parametre(frame, 60,_("Width"),cbuf, DARKCYAN) ;
}


/*****************************************************************************/
void Affiche_Infos_Segment_Module(WinEDA_BasePcbFrame * frame,
		MODULE* Module,EDGE_MODULE* pt_edge)
/*****************************************************************************/
/* Affiche en bas d'ecran les caract du Segment contour d'un module
	Les parametres du module doivent etre a jour*/
{
char bufcar[256];

	if(Module == NULL) return;

	frame->MsgPanel->EraseMsgBox();

	Affiche_1_Parametre(frame, 1,_("Seg"),"", DARKCYAN) ;
	Affiche_1_Parametre(frame, 5 ,_("Module"),Module->m_Reference->GetText() , DARKCYAN) ;

	Affiche_1_Parametre(frame, 14,_("Value"),Module->m_Value->GetText(),BLUE) ;

	sprintf(bufcar, "%8.8lX", Module->m_TimeStamp) ;
	Affiche_1_Parametre(frame, 24,_("TimeStamp"),bufcar,BROWN) ;

	Affiche_1_Parametre(frame, 34,_("Mod Layer"),ReturnPcbLayerName(Module->m_Layer),RED);

	Affiche_1_Parametre(frame, 44,_("Seg Layer"),
							ReturnPcbLayerName(Module->m_Layer),RED) ;

	valeur_param(pt_edge->m_Width,bufcar);
	Affiche_1_Parametre(frame, 54,_("Width"), bufcar,BLUE);
}


/*********************************************************/
void Affiche_Infos_Status_Pcb(WinEDA_BasePcbFrame * frame)
/*********************************************************/
/* Affiche l'etat du PCB : nb de pads, nets , connexions.. */
#define POS_AFF_NBPADS 1
#define POS_AFF_NBVIAS 8
#define POS_AFF_NBNODES 16
#define POS_AFF_NBLINKS 24
#define POS_AFF_NBNETS 32
#define POS_AFF_NBCONNECT 40
#define POS_AFF_NBNOCONNECT 48
{
int nb_vias = 0, ii;
EDA_BaseStruct * Struct;
char txt[128];

	frame->MsgPanel->EraseMsgBox();

	sprintf(txt,"%d",frame->m_Pcb->m_NbPads);
	Affiche_1_Parametre(frame, POS_AFF_NBPADS,_("Pads"),txt, DARKGREEN) ;

	for( ii= 0, Struct = frame->m_Pcb->m_Track; Struct != NULL; Struct = Struct->Pnext)
		{
		ii++;
		if(Struct->m_StructType == TYPEVIA) nb_vias++ ;
		}

	sprintf(txt,"%d",nb_vias);
	Affiche_1_Parametre(frame, POS_AFF_NBVIAS,_("Vias"),txt, DARKGREEN) ;

	sprintf(txt,"%d",frame->m_Pcb->GetNumNodes() );
	Affiche_1_Parametre(frame, POS_AFF_NBNODES,_("Nodes"),txt, DARKCYAN) ;

	sprintf(txt,"%d",frame->m_Pcb->m_NbLinks);
	Affiche_1_Parametre(frame, POS_AFF_NBLINKS,_("Links"),txt, DARKGREEN) ;

	sprintf(txt,"%d",frame->m_Pcb->m_NbNets);
	Affiche_1_Parametre(frame, POS_AFF_NBNETS,_("Nets"),txt,RED) ;

	sprintf(txt,"%d",frame->m_Pcb->m_NbLinks - frame->m_Pcb->GetNumNoconnect());
	Affiche_1_Parametre(frame, POS_AFF_NBCONNECT,_("Connect"),txt, DARKGREEN) ;

	sprintf(txt,"%d",frame->m_Pcb->GetNumNoconnect());
	Affiche_1_Parametre(frame, POS_AFF_NBNOCONNECT,_("NoConn"),txt,BLUE) ;
}

/*******************************************************************/
void Affiche_Infos_Equipot(int netcode, WinEDA_BasePcbFrame * frame)
/*******************************************************************/
/* Affiche les infos relatives a une equipot: nb de pads, nets , connexions.. */
{
int nb_vias = 0, ii;
EDA_BaseStruct * Struct;
char txt[128];
MODULE * module;
D_PAD * pad;
EQUIPOT * equipot;

	frame->MsgPanel->EraseMsgBox();

	equipot = GetEquipot(frame->m_Pcb, netcode);
	if ( equipot )
		Affiche_1_Parametre(frame, 1,_("Net Name"),equipot->m_Netname,RED) ;
	else
		Affiche_1_Parametre(frame, 1,_("No Net (not connected)"), "", RED) ;

	sprintf(txt,"%d", netcode);
	Affiche_1_Parametre(frame, 30,_("Net Code"),txt,RED) ;

	for ( ii = 0, module = frame->m_Pcb->m_Modules; module != 0;
				module = (MODULE*)module->Pnext)
		{
		for ( pad = module->m_Pads;pad != 0; pad = (D_PAD*) pad->Pnext)
			{
			if ( pad->m_NetCode == netcode ) ii++;
			}
		}

	sprintf(txt,"%d",ii);
	Affiche_1_Parametre(frame, 40,_("Pads"),txt, DARKGREEN) ;

	for( ii= 0, Struct = frame->m_Pcb->m_Track; Struct != NULL; Struct = Struct->Pnext)
		{
		ii++;
		if( Struct->m_StructType == TYPEVIA )
			if ( ((SEGVIA*)Struct)->m_NetCode == netcode ) nb_vias++ ;
		}

	sprintf(txt,"%d",nb_vias);
	Affiche_1_Parametre(frame, 50,_("Vias"),txt, BLUE) ;
}


