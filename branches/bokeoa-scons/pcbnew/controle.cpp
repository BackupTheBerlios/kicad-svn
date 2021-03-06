		/********************************************************/
		/* Routines generales de gestion des commandes usuelles */
		/********************************************************/

	/* fichier controle.cpp */
/*
 Routines d'affichage grille, Boite de coordonnees, Curseurs, marqueurs ...
*/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "pcbnew.h"

#include "id.h"
#include "protos.h"

/* Routines Locales : */

/* Variables Locales */

/**********************************/
void RemoteCommand(char * cmdline)
/**********************************/
{
char Line[1024];
wxString msg;
char *idcmd, * text;
WinEDA_PcbFrame * frame = EDA_Appl->m_PcbFrame;

	strncpy(Line, cmdline, sizeof(Line) -1 );
	frame->Affiche_Message( CONV_FROM_UTF8(Line));

	idcmd = strtok(Line," \n\r");
	text = strtok(NULL," \n\r");
	if( strcmp(idcmd,"$PART:") == 0)
	{
		MODULE * Module;
		msg = CONV_FROM_UTF8(text);
		Module = ReturnModule(frame->m_Pcb, msg);
		msg.Printf(_("Locate module %s %s"),msg.GetData(), Module ? wxT("Ok") : wxT("not found"));
		frame->SetStatusText(msg);
		if ( Module )
		{
	wxClientDC dc(frame->DrawPanel);
			frame->DrawPanel->PrepareGraphicContext(&dc);
			frame->GetScreen()->Trace_Curseur(frame->DrawPanel, &dc);
			frame->GetScreen()->m_Curseur = Module->m_Pos;
			frame->GetScreen()->Trace_Curseur(frame->DrawPanel, &dc);
		}
	}

	if( strcmp(idcmd,"$PIN:") == 0)
	{
		wxString PinName, ModName;
		MODULE * Module;
		D_PAD * Pad = NULL;
		int netcode = -1;
		PinName = CONV_FROM_UTF8(text);
		text = strtok(NULL," \n\r");
		if( strcmp(text, "$PART:") == 0 ) text = strtok(NULL,"\n\r");

wxClientDC dc(frame->DrawPanel);
	frame->DrawPanel->PrepareGraphicContext(&dc);

		ModName = CONV_FROM_UTF8(text);
		Module = ReturnModule(frame->m_Pcb, ModName);
		if( Module ) Pad = ReturnPad(Module, PinName);
		if( Pad ) netcode = Pad->m_NetCode;
		if ( netcode > 0 )
			{
			/* effacement surbrillance ancienne */
			if(g_HightLigt_Status) frame->Hight_Light(&dc);
			g_HightLigth_NetCode = netcode;
			frame->Hight_Light(&dc);
			frame->GetScreen()->Trace_Curseur(frame->DrawPanel, &dc);
			frame->GetScreen()->m_Curseur = Pad->m_Pos;
			frame->GetScreen()->Trace_Curseur(frame->DrawPanel, &dc);
			}

		if ( Module == NULL )
			msg.Printf( _("module %s not found"), text);
		else if ( Pad == NULL )
			msg.Printf( _("Pin %s (module %s) not found"), PinName.GetData(), ModName.GetData());
		else
			msg.Printf( _("Locate Pin %s (module %s)"), PinName.GetData(),ModName.GetData());
		frame->Affiche_Message(msg);
	}
}


/***********************************************************************/
EDA_BaseStruct * WinEDA_BasePcbFrame::PcbGeneralLocateAndDisplay(void)
/***********************************************************************/
{
	return Locate(CURSEUR_OFF_GRILLE);
}


/****************************************************************/
void WinEDA_BasePcbFrame::GeneralControle(wxDC *DC, wxPoint Mouse)
/*****************************************************************/
{
int ll;
wxSize delta;
int zoom = GetScreen()->GetZoom();
wxPoint curpos, oldpos;
int hotkey = 0;

	ActiveScreen = GetScreen();

	// Sauvegarde du board si necessaire:
int CurrentTime = time(NULL);
	if( !GetScreen()->IsModify() || GetScreen()->IsSave() )
	{		/* Sauvegarde inutile */
		g_SaveTime = CurrentTime;
	}

	if ( (CurrentTime - g_SaveTime) > g_TimeOut)
	{
		wxString tmpFileName = GetScreen()->m_FileName;
		wxString filename = g_SaveFileName + PcbExtBuffer;
		((WinEDA_PcbFrame*)this)->SavePcbFile(filename);
		GetScreen()->m_FileName = tmpFileName;
		SetTitle(GetScreen()->m_FileName);
	}

	curpos = DrawPanel->CursorRealPosition(Mouse);
	oldpos = GetScreen()->m_Curseur;

	delta.x = (int) round((double)GetScreen()->GetGrid().x / zoom);
	delta.y = (int) round((double)GetScreen()->GetGrid().y / zoom);
	if( delta.x <= 0 ) delta.x = 1;
	if( delta.y <= 0 ) delta.y = 1;

	switch(g_KeyPressed)
		{
		case WXK_NUMPAD_SUBTRACT :
		case WXK_SUBTRACT :
		case '-' :
			ll = GetScreen()->m_Active_Layer;
			if(ll > CMP_N) break;
			if(ll <= CUIVRE_N) break;
			if ( m_Pcb->m_BoardSettings->m_CopperLayerCount <= 1)	// Single layer
				ll = CUIVRE_N;
			if ( ll == CMP_N )
				ll = MAX(CUIVRE_N, m_Pcb->m_BoardSettings->m_CopperLayerCount-2);
			else if ( ll > CUIVRE_N) ll--;
			GetScreen()->m_Active_Layer = ll;
			if ( DisplayOpt.ContrastModeDisplay ) DrawPanel->Refresh(TRUE);
			break ;

		case WXK_NUMPAD_ADD :
		case WXK_ADD :
		case '+' :
			ll = GetScreen()->m_Active_Layer;
			if(ll >= CMP_N) break;
			ll++;
			if ( ll >= m_Pcb->m_BoardSettings->m_CopperLayerCount-1 )
				ll = CMP_N;
			if ( m_Pcb->m_BoardSettings->m_CopperLayerCount <= 1)	// Single layer
				ll = CUIVRE_N;
			GetScreen()->m_Active_Layer = ll;
			if ( DisplayOpt.ContrastModeDisplay ) DrawPanel->Refresh(TRUE);
			break ;

		case WXK_NUMPAD0 :
		case WXK_PRIOR :
			if ( GetScreen()->m_Active_Layer != CMP_N )
			{
				GetScreen()->m_Active_Layer = CMP_N;
				if ( DisplayOpt.ContrastModeDisplay )
					GetScreen()->SetRefreshReq();
			}
			break ;

		case WXK_NUMPAD9 :
		case WXK_NEXT :
			if ( GetScreen()->m_Active_Layer != CUIVRE_N )
			{
				GetScreen()->m_Active_Layer = CUIVRE_N;
				if ( DisplayOpt.ContrastModeDisplay )
					GetScreen()->SetRefreshReq();
			}
			break ;

		case 'F' | GR_KB_CTRL :
		case 'f' | GR_KB_CTRL:
			DisplayOpt.DisplayPcbTrackFill ^= 1; DisplayOpt.DisplayPcbTrackFill &= 1 ;
			GetScreen()->SetRefreshReq();
			break ;

		case ' ' : /* Mise a jour de l'origine des coord relatives */
			GetScreen()->m_O_Curseur = GetScreen()->m_Curseur;
			break ;


		case 'U' | GR_KB_CTRL :
		case 'u' | GR_KB_CTRL :
			g_UnitMetric = (g_UnitMetric == INCHES ) ? MILLIMETRE : INCHES;
			break ;

		case WXK_F1 :
			OnZoom(ID_ZOOM_PLUS_KEY);
			curpos = GetScreen()->m_Curseur;
			break;

		case WXK_F2 :
			OnZoom(ID_ZOOM_MOINS_KEY);
			curpos = GetScreen()->m_Curseur;
			break;

		case WXK_F3 :
			OnZoom(ID_ZOOM_REDRAW_KEY);
			break;

		case WXK_F4 :
			OnZoom(ID_ZOOM_CENTER_KEY);
			curpos = GetScreen()->m_Curseur;
			break;

		case WXK_F5 :	/* unused */
			break;

		case WXK_NUMPAD8  :	/* Deplacement curseur vers le haut */
		case WXK_UP	:
			Mouse.y -= delta.y;
			DrawPanel->MouseTo(Mouse);
			break ;

		case WXK_NUMPAD2:	/* Deplacement curseur vers le bas */
		case WXK_DOWN:
			Mouse.y += delta.y;
			DrawPanel->MouseTo(Mouse);
			break ;

		case WXK_NUMPAD4:	/* Deplacement curseur vers la gauche */
		case WXK_LEFT :
			Mouse.x -= delta.x;
			DrawPanel->MouseTo(Mouse);
			break ;

		case WXK_NUMPAD6:  /* Deplacement curseur vers la droite */
		case WXK_RIGHT:
			Mouse.x += delta.x;
			DrawPanel->MouseTo(Mouse);
			break ;

		default: hotkey = g_KeyPressed;
			break;
		}
	/* Recalcul de la position du curseur schema */
	GetScreen()->m_Curseur = curpos;
	/* Placement sur la grille generale */
	PutOnGrid( & GetScreen()->m_Curseur);

	if( GetScreen()->IsRefreshReq() )
		{
		RedrawActiveWindow(DC, TRUE);
		}

	if ( (oldpos.x != GetScreen()->m_Curseur.x) ||
		 (oldpos.y != GetScreen()->m_Curseur.y) )
		{
		curpos = GetScreen()->m_Curseur;
		GetScreen()->m_Curseur = oldpos;
		GetScreen()->Trace_Curseur(DrawPanel, DC);

		GetScreen()->m_Curseur = curpos;
		GetScreen()->Trace_Curseur(DrawPanel, DC);

		if(GetScreen()->ManageCurseur)
			{
			GetScreen()->ManageCurseur(DrawPanel, DC, TRUE);
			}
		}

	SetToolbars();
	Affiche_Status_Box();	 /* Affichage des coord curseur */

	if ( hotkey )
	{
		OnHotKey(DC, hotkey, NULL);
	}
}


