		/***********************************************/
		/* Routines d'effacement et copie de structures*/
		/***********************************************/

#include "fctsys.h"

#include "common.h"
#include "gerbview.h"

/* Variables locales */


/***************************************************/
void DeleteStructure( EDA_BaseStruct * PtStruct )
/***************************************************/
/* Supprime de la liste chainee la stucture pointee par GenericStructure
	et libere la memoire correspondante
*/
{
EDA_BaseStruct *PtNext, *PtBack;
int IsDeleted;
char Line[256];

	if( PtStruct == NULL) return ;

	IsDeleted = PtStruct->GetState(DELETED);

	PtNext = PtStruct->Pnext;
	PtBack = PtStruct->Pback;

	switch( PtStruct->m_StructType )
		{
		case TYPE_NOT_INIT:
			DisplayError(NULL, "DeleteStruct: Type NOT_INIT");
			break;


		case TYPEDRAWSEGMENT:
			#undef Struct
			#define Struct ((DRAWSEGMENT*)PtStruct)
			Struct->UnLink();
			delete Struct;
			break;

		 case TYPETEXTE:
			#undef Struct
			#define Struct ((TEXTE_PCB*)PtStruct)
			Struct->UnLink();
			delete Struct;
			break;


		case TYPEVIA:
		case TYPETRACK:
		case TYPEZONE:
			#undef Struct
			#define Struct ((TRACK*)PtStruct)
			Struct->UnLink();
			delete Struct;
			break;

		 case TYPEMARQUEUR:
			#undef Struct
			#define Struct ((MARQUEUR*)PtStruct)
			Struct->UnLink();
			delete Struct;
			break;

		case TYPEPCB:
		default:
			sprintf(Line," DeleteStructure: Type %d Inattendu",
										PtStruct->m_StructType);
			DisplayError(NULL, Line);
			break;
		}
}

/*************************************************/
void DeleteStructList( EDA_BaseStruct * PtStruct )
/*************************************************/
/* Supprime la liste chainee pointee par PtStruct
	et libere la memoire correspondante
*/
{
EDA_BaseStruct *PtNext;
	while (PtStruct)
		{
		PtNext = PtStruct->Pnext;
		delete PtStruct;
		PtStruct = PtNext;
		}
}

