		/**********************************************************/
		/* Routines d'affichage de parametres et caracteristiques */
		/**********************************************************/

#include "fctsys.h"
#include "gr_basic.h"
#include "common.h"
#include "gerbview.h"
#include "pcbplot.h"

#include "protos.h"

/* Routines locales */

/****************************************************************************/
void Affiche_Infos_PCB_Texte(WinEDA_BasePcbFrame * frame, TEXTE_PCB* pt_texte)
/****************************************************************************/
/* Affiche en bas d'ecran les caract du texte sur PCB
	Entree :
		pointeur de la description du texte
*/
{
char Line[1024];

	frame->MsgPanel->EraseMsgBox();

	if( pt_texte->m_StructType == TYPECOTATION )
		Affiche_1_Parametre(frame, 1,_("COTATION"),pt_texte->m_Text, DARKGREEN);

	else
		Affiche_1_Parametre(frame, 1,_("PCB Text"),pt_texte->m_Text, DARKGREEN);

	sprintf(Line,"Layer %d",pt_texte->m_Layer + 1);
	Affiche_1_Parametre(frame, 28, _("Layer:"), Line, g_DesignSettings.m_LayerColor[pt_texte->m_Layer] );

	Affiche_1_Parametre(frame, 36,"miroir","",GREEN) ;
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



/*********************************************************************/
void Affiche_Infos_Piste(WinEDA_BasePcbFrame * frame, TRACK * pt_piste)
/*********************************************************************/
/* Affiche les caract principales d'un segment de piste en bas d'ecran */
{
int d_index, ii = -1;
D_CODE * pt_D_code;
int layer = frame->GetScreen()->m_Active_Layer;

	frame->MsgPanel->EraseMsgBox();

	d_index = pt_piste->m_NetCode;
	pt_D_code = ReturnToolDescr(layer, d_index, &ii);

	switch(pt_piste->m_StructType)
		{
		case TYPETRACK:
			if ( pt_piste->m_Shape < S_SPOT_CIRCLE ) strcpy(cbuf,"LINE");
			else  strcpy(cbuf,"FLASH");
			break;

		case TYPEZONE:
			strcpy(cbuf,"ZONE"); break;

		default:
			strcpy(cbuf,"????"); break;
		}
	Affiche_1_Parametre(frame, 1,"Type",cbuf, DARKCYAN);

	sprintf(cbuf,"%d", ii+1);
	Affiche_1_Parametre(frame, 10,"Tool",cbuf, RED);

	if ( pt_D_code )
		{
		sprintf(cbuf,"D%d", d_index);
		Affiche_1_Parametre(frame, 20,"D CODE",cbuf, BLUE);

		Affiche_1_Parametre(frame, 30,"D type",
					pt_D_code ? g_GERBER_Tool_Type[pt_D_code->m_Shape] : "????",
					BLUE);
		}

	sprintf(cbuf,"%d",pt_piste->m_Layer + 1);
	Affiche_1_Parametre(frame, 40,_("Layer"),cbuf,BROWN) ;

	/* Affiche Epaisseur */
	valeur_param((unsigned)(pt_piste->m_Width), cbuf) ;
	Affiche_1_Parametre(frame, 50,_("Width"),cbuf, DARKCYAN) ;
}

	/************************************************************/
	/* void Affiche_Infos_DrawSegment(DRAWSEGMENT* DrawSegment) */
	/************************************************************/

/* Affiche les caract principales d'un segment type drawind PCB en bas d'ecran */

void Affiche_Infos_DrawSegment(WinEDA_DrawFrame * frame, DRAWSEGMENT * DrawSegment)
{
int itype;

	frame->MsgPanel->EraseMsgBox();

	itype = DrawSegment->m_Type & 0x0F;

	Affiche_1_Parametre(frame, 1,"Type","FLASH", DARKCYAN) ;

	Affiche_1_Parametre(frame, 16,_("Shape"),"",RED) ;
	if(DrawSegment->m_Shape == S_CIRCLE)
		Affiche_1_Parametre(frame, -1,"",_("Circle"),RED) ;
	else if (DrawSegment->m_Shape == S_ARC)
		{
		Affiche_1_Parametre(frame, -1,"",_("  Arc  "),RED) ;
		sprintf(cbuf,"%d",DrawSegment->m_Angle);
		Affiche_1_Parametre(frame, 32," l.arc ",cbuf,RED) ;
		}
	else Affiche_1_Parametre(frame, -1,"","Segment",RED) ;


	sprintf(cbuf,"%d",DrawSegment->m_Layer);

	Affiche_1_Parametre(frame, 48,_("Layer"),cbuf,BROWN) ;

	/* Affiche Epaisseur */
	valeur_param((unsigned)(DrawSegment->m_Width), cbuf) ;
	Affiche_1_Parametre(frame, 60,_("Width"),cbuf, DARKCYAN) ;
}



	/*************************************************/
	/* void Affiche_Infos_Status_Pcb(COMMAND * frame ) */
	/*************************************************/

/* Affiche l'etat du PCB : nb de pads, nets , connexions.. */

#define POS_AFF_NBPADS 1
#define POS_AFF_NBVIAS 8
#define POS_AFF_NBNODES 16
#define POS_AFF_NBLINKS 24
#define POS_AFF_NBNETS 32
#define POS_AFF_NBCONNECT 40
#define POS_AFF_NBNOCONNECT 48

void Affiche_Infos_Status_Pcb(WinEDA_BasePcbFrame * frame)
{
int nb_vias = 0, ii;
EDA_BaseStruct * Struct;
char txt[128];

	frame->MsgPanel->EraseMsgBox();

	sprintf(txt,"%d",frame->m_Pcb->m_NbPads);
	Affiche_1_Parametre(frame, POS_AFF_NBPADS,"Pads",txt, DARKGREEN) ;

	for( ii= 0, Struct = frame->m_Pcb->m_Track; Struct != NULL; Struct = Struct->Pnext)
		{
		ii++;
		if(Struct->m_StructType == TYPEVIA) nb_vias++ ;
		}

	sprintf(txt,"%d",nb_vias);
	Affiche_1_Parametre(frame, POS_AFF_NBVIAS,"Vias",txt, DARKGREEN) ;

	sprintf(txt,"%d",frame->m_Pcb->GetNumNodes() );
	Affiche_1_Parametre(frame, POS_AFF_NBNODES,"Nodes",txt, DARKCYAN) ;

	sprintf(txt,"%d",frame->m_Pcb->m_NbLinks);
	Affiche_1_Parametre(frame, POS_AFF_NBLINKS,"Links",txt, DARKGREEN) ;

	sprintf(txt,"%d",frame->m_Pcb->m_NbNets);
	Affiche_1_Parametre(frame, POS_AFF_NBNETS,"Nets",txt,RED) ;

	sprintf(txt,"%d",frame->m_Pcb->m_NbLinks - frame->m_Pcb->GetNumNoconnect());
	Affiche_1_Parametre(frame, POS_AFF_NBCONNECT,"Connect",txt, DARKGREEN) ;

	sprintf(txt,"%d",frame->m_Pcb->GetNumNoconnect());
	Affiche_1_Parametre(frame, POS_AFF_NBNOCONNECT,"NoConn",txt,BLUE) ;
}


