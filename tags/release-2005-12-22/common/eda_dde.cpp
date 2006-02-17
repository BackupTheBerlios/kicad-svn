
	//////////////////////
	// Name: eda_dde.cc	//
	//////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "fctsys.h"
#include "eda_dde.h"
#include "wxstruct.h"
#include "id.h"

#include "common.h"

#define ID_CONN "CAO_COM"

const wxChar HOSTNAME[80] = wxT("localhost");

/* variables locales */
#define IPC_BUF_SIZE 4000
char client_ipc_buffer[IPC_BUF_SIZE];
char server_ipc_buffer[IPC_BUF_SIZE];

wxServer * server;

void (* RemoteFct)(char * cmd);

char buffcar[1024];

void SetupServerFunction(void (* remotefct)(char * remotecmd) )
{
	RemoteFct = remotefct;
}


	/*****************************/
	/* Routines liees au SERVEUR */
	/*****************************/
/* Fonction d'initialisation d'un serveur socket
*/
WinEDA_Server * CreateServer(wxWindow * window, int service)
{
wxIPV4address addr;

	// Create a new server
	addr.Service(service);

	server = new wxServer(addr);
	if(server)
	  {
	    server->SetNotify(wxSOCKET_CONNECTION_FLAG);
	    server->SetEventHandler(*window, ID_EDA_SOCKET_EVENT_SERV);
	    server->Notify(TRUE);
	  }

	return server;
}


/********************************************************/
void WinEDA_DrawFrame::OnSockRequest(wxSocketEvent& evt)
/********************************************************/
/* Fonction appelee a chaque demande d'un client
*/
{
size_t len;
wxSocketBase *sock = evt.GetSocket();

  switch (evt.GetSocketEvent())
     {
     case wxSOCKET_INPUT:
       sock->Read(server_ipc_buffer,1);
       len = sock->Read(server_ipc_buffer+1,IPC_BUF_SIZE-2).LastCount();
       server_ipc_buffer[len+1] = 0;
       if(RemoteFct ) RemoteFct(server_ipc_buffer);
       break;

     case wxSOCKET_LOST:
       return;
       break;

     default:
       wxPrintf( wxT("WinEDA_DrawFrame::OnSockRequest() error: Invalid event !"));
       break;
     }
}

/**************************************************************/
void WinEDA_DrawFrame::OnSockRequestServer(wxSocketEvent& evt)
/**************************************************************/
/* fonction appelée lors d'une demande de connexion d'un client
*/
{
wxSocketBase *sock2;
wxSocketServer *server = (wxSocketServer *) evt.GetSocket();

    sock2 = server->Accept();
    if (sock2 == NULL) return;

    sock2->Notify(TRUE);
	sock2->SetEventHandler(*this, ID_EDA_SOCKET_EVENT);
    sock2->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
}


	/****************************/
	/* Routines liees au CLIENT */
	/*****************************/

/********************************************/
bool SendCommand( int service, char * cmdline)
/********************************************/
/* Fonction utilisee par un client pour envoyer une information a un serveur.
	- Etablit une connection Socket Client
    - envoie le contenu du buffer cmdline
    - ferme la connexion

    service contient le numéro de service en ascii.
*/
{
static wxSocketClient * sock_client;
bool success = FALSE;
wxIPV4address addr;

	// Create a connexion
	if (!sock_client)
		{
		addr.Hostname(HOSTNAME);
		addr.Service(service);

		sock_client = new wxSocketClient();
		sock_client->SetTimeout(2);	// Time out in Seconds
		sock_client->Connect(addr, FALSE);
		sock_client->WaitOnConnect(0, 100);

		if (!sock_client->Ok())
			{
			sock_client->Destroy();
			sock_client = NULL;
			}

		if ( sock_client && !sock_client->IsConnected())
			{
			sock_client->Destroy();
			sock_client = NULL;
			}
		}

	if (sock_client)
		{
		success = TRUE;
		sock_client->SetFlags(wxSOCKET_NOWAIT /*wxSOCKET_WAITALL*/);
		sock_client->Write(cmdline, strlen(cmdline));
		}

	return success;
}

