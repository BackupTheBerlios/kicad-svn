	/***********************************************************/
	/* Edition de Module: Frame d'edition des caracteristiques */
	/*				 et proprietes generales du module		   */
	/***********************************************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "pcbnew.h"
#include "autorout.h"
#include "trigo.h"
#include "bitmaps.h"
#include "3d_struct.h"
#include "3d_viewer.h"

#include "protos.h"

/* Variables locales: */
bool GoToEditor = FALSE;

enum id_Module_properties
{
	ID_ACCEPT_MODULE_PROPERTIES = 1900,
	ID_CLOSE_MODULE_PROPERTIES,
	ID_GOTO_MODULE_EDITOR,
	ID_MODULE_PROPERTIES_EXCHANGE,
	ID_MODULE_EDIT_ADD_TEXT,
	ID_MODULE_EDIT_EDIT_TEXT,
	ID_MODULE_EDIT_DELETE_TEXT,
	ID_MODULE_LISTBOX_SELECT,
	ID_LISTBOX_ORIENT_SELECT,
	ID_BROWSE_3D_LIB,
	ID_ADD_3D_SHAPE,
	ID_REMOVE_3D_SHAPE,
	ID_NOTEBOOK
};

class Panel3D_Ctrl;

	/**************************************/
	/* class WinEDA_ModulePropertiesFrame */
	/**************************************/

class WinEDA_ModulePropertiesFrame: public wxDialog
{
private:

	WinEDA_BasePcbFrame * m_Parent;
	wxDC * m_DC;
	MODULE * m_CurrentModule;
	wxNotebook* m_NoteBook;
	wxPanel * m_PanelProperties;
	Panel3D_Ctrl * m_Panel3D;
	WinEDAChoiceBox * m_TextListBox;
	wxRadioBox * m_LayerCtrl;
	wxRadioBox * m_OrientCtrl;
	WinEDA_ValueCtrl * m_OrientValue;
	wxRadioBox * m_AttributsCtrl;
	wxRadioBox * m_AutoPlaceCtrl;
	wxSlider * m_CostRot90Ctrl, * m_CostRot180Ctrl;
	wxButton * m_DeleteFieddButton;
	WinEDA_EnterText *m_Doc, *m_Keyword;


public:
	// Constructor and destructor
	WinEDA_ModulePropertiesFrame(WinEDA_BasePcbFrame *parent,
							MODULE * Module, wxDC * DC, const wxPoint & pos);
	~WinEDA_ModulePropertiesFrame(void)
		{
		}

private:
	void OnQuit(wxCommandEvent& event);
	void ModulePropertiesAccept(wxCommandEvent& event);
	void CreateTextModule(wxCommandEvent& event);
	void EditOrDelTextModule(wxCommandEvent& event);
	void SelectTextListBox(wxCommandEvent& event);
	void ReCreateFieldListBox(void);
	void CreatePanelProperties(bool FullOptions);
	void ModuleOrientEvent(wxCommandEvent& event);
	void ExchangeModule(wxCommandEvent& event);
	void GotoModuleEditor(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(WinEDA_ModulePropertiesFrame, wxDialog)
	EVT_BUTTON(ID_ACCEPT_MODULE_PROPERTIES,
			WinEDA_ModulePropertiesFrame::ModulePropertiesAccept)
	EVT_BUTTON(ID_CLOSE_MODULE_PROPERTIES, WinEDA_ModulePropertiesFrame::OnQuit)
	EVT_BUTTON(ID_MODULE_EDIT_ADD_TEXT, WinEDA_ModulePropertiesFrame::CreateTextModule)
	EVT_BUTTON(ID_MODULE_EDIT_EDIT_TEXT, WinEDA_ModulePropertiesFrame::EditOrDelTextModule)
	EVT_BUTTON(ID_MODULE_EDIT_DELETE_TEXT, WinEDA_ModulePropertiesFrame::EditOrDelTextModule)
	EVT_BUTTON(ID_MODULE_PROPERTIES_EXCHANGE, WinEDA_ModulePropertiesFrame::ExchangeModule)
	EVT_KICAD_CHOICEBOX(ID_MODULE_LISTBOX_SELECT, WinEDA_ModulePropertiesFrame::SelectTextListBox)
	EVT_RADIOBOX(ID_LISTBOX_ORIENT_SELECT, WinEDA_ModulePropertiesFrame::ModuleOrientEvent)
	EVT_BUTTON(ID_GOTO_MODULE_EDITOR, WinEDA_ModulePropertiesFrame::GotoModuleEditor)

END_EVENT_TABLE()

/*********************************/
class Panel3D_Ctrl: public wxPanel
/*********************************/
/* panel d'entree des caract 3D */
{
public:
	Panel3D_Ctrl * m_Pnext, * m_Pback;	// Chainage
	wxNotebook * m_Parent;
	WinEDA_ModulePropertiesFrame * m_ParentFrame;
	WinEDA_EnterText *m_3D_ShapeName;
	WinEDA_VertexCtrl *m_3D_Scale, *m_3D_Offset, *m_3D_Rotation;
public:
	Panel3D_Ctrl(WinEDA_ModulePropertiesFrame * parentframe,
		wxNotebook * parent, int id, Struct3D_Master * struct3D);
	~Panel3D_Ctrl(void);
	void Browse3DLib(wxCommandEvent& event);
	void AddOrRemove3DShape(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};
BEGIN_EVENT_TABLE(Panel3D_Ctrl, wxPanel)
	EVT_BUTTON(ID_BROWSE_3D_LIB, Panel3D_Ctrl::Browse3DLib)
	EVT_BUTTON(ID_ADD_3D_SHAPE, Panel3D_Ctrl::AddOrRemove3DShape)
	EVT_BUTTON(ID_REMOVE_3D_SHAPE, Panel3D_Ctrl::AddOrRemove3DShape)
END_EVENT_TABLE()


/*******************************************************************/
void WinEDA_BasePcbFrame::InstallModuleOptionsFrame(MODULE * Module,
					wxDC * DC, const wxPoint & pos)
/*******************************************************************/
/* Fonction relai d'installation de la frame d'édition des proprietes
du module*/
{
	WinEDA_ModulePropertiesFrame * frame = new WinEDA_ModulePropertiesFrame(this,
					 Module, DC, pos);
	frame->ShowModal(); frame->Destroy();

	if ( GoToEditor && GetScreen()->m_CurrentItem )
	{
		if (m_Parent->m_ModuleEditFrame == NULL )
		{
			m_Parent->m_ModuleEditFrame = new WinEDA_ModuleEditFrame(this,
						m_Parent,_("Module Editor"),
						wxPoint(-1, -1), wxSize(600,400) );
		}

		m_Parent->m_ModuleEditFrame->Load_Module_Module_From_BOARD(
			(MODULE*)GetScreen()->m_CurrentItem);
		GetScreen()->m_CurrentItem = NULL;

		GoToEditor = FALSE;
		m_Parent->m_ModuleEditFrame->Show(TRUE);
		m_Parent->m_ModuleEditFrame->Iconize(FALSE);
	}
}


#define H_SIZE 430
#define V_SIZE 530
/**************************************************************************************/
WinEDA_ModulePropertiesFrame::WinEDA_ModulePropertiesFrame(WinEDA_BasePcbFrame *parent,
				MODULE * Module,wxDC * DC,
				const wxPoint & framepos):
		wxDialog(parent, -1, _("Module properties"), framepos,
					wxSize(H_SIZE, V_SIZE), DIALOG_STYLE)
/**************************************************************************************/
{
wxPoint pos;
wxString number;
wxButton * Button;
bool FullOptions = FALSE;
wxSize usize;
wxColour colour = GetBackgroundColour();

	SetIcon( wxICON(icon_modedit) );		// Give an icon

	m_Parent = parent;
	SetFont(*g_DialogFont);
	m_DC = DC;
	Centre();
	usize = GetClientSize();
	if ( m_Parent->m_Ident == PCB_FRAME ) FullOptions = TRUE;

	m_LayerCtrl = NULL;
	m_OrientCtrl = NULL;
	m_OrientValue = NULL;
	m_Doc = m_Keyword = NULL;

	m_CurrentModule = Module;
	m_DeleteFieddButton = NULL;

	if ( m_CurrentModule )
		{
		}

	m_NoteBook = new wxNotebook(this, ID_NOTEBOOK,
   		wxDefaultPosition,wxSize(usize.x,usize.y - 34) );
	m_NoteBook->SetFont(*g_DialogFont);

	wxLayoutConstraints* c = new wxLayoutConstraints;
	c->left.SameAs(this, wxLeft, 4);
	c->right.SameAs(this, wxRight, 4);
	c->top.SameAs(this, wxTop, 4);
    c->bottom.SameAs(this, wxBottom, 34);

	m_NoteBook->SetConstraints(c);

	// Add panels
	m_PanelProperties = new wxPanel(m_NoteBook, -1);
	m_PanelProperties->SetFont(*g_DialogFont);
	m_NoteBook->AddPage(m_PanelProperties, _("Properties"), TRUE);

	m_Panel3D = new Panel3D_Ctrl(this, m_NoteBook, -1,
				m_CurrentModule->m_3D_Drawings);
	m_NoteBook->AddPage(m_Panel3D, _("3D settings"), FALSE);

	/* creation des autres formes 3D */
	Panel3D_Ctrl * panel3D = m_Panel3D, * nextpanel3D;
	Struct3D_Master *draw3D = m_CurrentModule->m_3D_Drawings;
	draw3D = (Struct3D_Master *) draw3D->Pnext;
	for ( ; draw3D != NULL; draw3D = (Struct3D_Master *) draw3D->Pnext)
	{
		nextpanel3D = new Panel3D_Ctrl(this, m_NoteBook, -1, draw3D);
		m_NoteBook->AddPage(nextpanel3D, _("3D settings"), FALSE);
		panel3D->m_Pnext = nextpanel3D;
		nextpanel3D->m_Pback = panel3D;
		panel3D = nextpanel3D;
	}


	/* Creation des boutons de commande */
	pos.x = 150; pos.y = usize.y - 32;
	Button = new wxButton(this, ID_ACCEPT_MODULE_PROPERTIES,
						_("Ok"), pos);
	Button->SetForegroundColour(*wxRED);

	pos.x += Button->GetSize().x + 20;
	Button = new wxButton(this, ID_CLOSE_MODULE_PROPERTIES,
						_("Cancel"), pos);
	Button->SetForegroundColour(*wxBLUE);

	CreatePanelProperties(FullOptions);
}


/***********************************************************/
void Panel3D_Ctrl::AddOrRemove3DShape(wxCommandEvent& event)
/***********************************************************/
{
	if( event.GetId() == ID_ADD_3D_SHAPE)
	{
	Panel3D_Ctrl * panel3D = new Panel3D_Ctrl(m_ParentFrame, m_Parent,
						-1, NULL);
	m_Parent->InsertPage(m_Parent->GetSelection()+1,
						panel3D, _("3D settings"), TRUE);
	panel3D->m_Pback = this;
	panel3D->m_Pnext = m_Pnext;
	if ( m_Pnext ) m_Pnext->m_Pback = panel3D;
	m_Pnext = panel3D;
	}

	if( event.GetId() == ID_REMOVE_3D_SHAPE)
	{
		if ( m_Pback )
		{
			m_Pback->m_Pnext = m_Pnext;
			if ( m_Pnext ) m_Pnext->m_Pback = m_Pback;
			m_Parent->DeletePage(m_Parent->GetSelection());
			m_ParentFrame->Refresh(TRUE);
		}
	}
}

/***************************************************************************/
void WinEDA_ModulePropertiesFrame::CreatePanelProperties(bool FullOptions)
/***************************************************************************/
/* creation du panel d'edition des proprietes du module
*/
{
int xx, yy;
wxPoint pos;
wxButton * Button;

	pos.x = 200; pos.y = 10;

	if ( FullOptions )	// Edition du module su le C.I.
		{
		Button = new wxButton(m_PanelProperties, ID_MODULE_PROPERTIES_EXCHANGE,
						_("Change module(s)"), pos);
		Button->SetForegroundColour(wxColor(80,40,0));
		pos.y += Button->GetSize().y + 5;
		Button = new wxButton(m_PanelProperties, ID_GOTO_MODULE_EDITOR,
						_("Goto Module Editor"), pos);
		Button->SetForegroundColour(wxColor(0,128,80) );
		}
	else		// Edition du module en librairie
		{
		pos.y += 12;
		m_Doc = new WinEDA_EnterText(m_PanelProperties, _("Doc"),
						m_CurrentModule->m_Doc,
						pos, wxSize(200,-1) );
		pos.y += m_Doc->GetDimension().y + 17;
		m_Keyword = new WinEDA_EnterText(m_PanelProperties, _("Keywords"),
						m_CurrentModule->m_KeyWord,
						pos, wxSize(200,-1) );
		}

	pos.x = 5; pos.y = 5;
	yy = pos.y;

	wxStaticBox * box = new wxStaticBox(m_PanelProperties, -1, _("Fields:"), pos, wxSize(160, 140) );
	pos.x += 10; pos.y += 17;
	m_TextListBox = new WinEDAChoiceBox(m_PanelProperties, ID_MODULE_LISTBOX_SELECT,
			pos, wxSize(-1,-1));

	ReCreateFieldListBox();

	m_TextListBox->SetSelection(0);

	pos.y += m_TextListBox->GetSize().y + 5;
	Button = new wxButton(m_PanelProperties, ID_MODULE_EDIT_ADD_TEXT,
						_("Add Field"), pos);
	Button->SetForegroundColour(*wxBLACK);

	pos.y += Button->GetSize().y + 3;
	Button = new wxButton(m_PanelProperties, ID_MODULE_EDIT_EDIT_TEXT,
						_("Edit Field"), pos);
	Button->SetForegroundColour(*wxBLACK);

	pos.y += Button->GetSize().y + 3;
	m_DeleteFieddButton = Button = new wxButton(m_PanelProperties, ID_MODULE_EDIT_DELETE_TEXT,
						_("Delete Field"), pos);
	m_DeleteFieddButton->SetForegroundColour(*wxBLACK);
	m_DeleteFieddButton->Enable(FALSE);	// Enable pour fields autres que ref et valeur

	pos.y += Button->GetSize().y + 3;
	box->SetSize( wxSize(160, pos.y - yy) );

	pos.y = yy + 10 + box->GetSize().y;

	if ( FullOptions )
		{
		wxString layer_list[2] = { _("Componant"), _("Copper") };
		m_LayerCtrl = new wxRadioBox( m_PanelProperties, -1, _("Layer"), pos,
				wxSize(-1,-1), 2, layer_list, 1);
		m_LayerCtrl->SetSelection( (m_CurrentModule->m_Layer == CUIVRE_N) ? 1 : 0);

		m_LayerCtrl->GetSize(&xx, &yy);
		pos.y += 10 + yy;
		bool select = FALSE;
		wxString orient_list[5] = {
		            _("Normal"), wxT("+ 90.0"), wxT("- 90.0"), wxT("180.0"), _("User") };
		m_OrientCtrl = new wxRadioBox( m_PanelProperties, ID_LISTBOX_ORIENT_SELECT, _("Orient"),
					pos, wxSize(-1,-1), 5, orient_list, 1);

		switch (m_CurrentModule->m_Orient)
			{
			case 0:
			m_OrientCtrl->SetSelection( 0 );
			break;

			case 900:
			case -2700:
				m_OrientCtrl->SetSelection( 1 );
				break;

			case -900:
			case 2700:
				m_OrientCtrl->SetSelection( 2 );
				break;

			case -1800:
			case 1800:
				m_OrientCtrl->SetSelection( 3 );
				break;

			default:
				m_OrientCtrl->SetSelection( 4 );
				select = TRUE;
				break;
			}

		m_OrientCtrl->GetSize(&xx, &yy);
		pos.y += 10 + yy;
		m_OrientValue = new WinEDA_ValueCtrl(m_PanelProperties, _("Orient (0.1 deg)"),
				m_CurrentModule->m_Orient, 2, pos, 1);
		m_OrientValue->Enable(select);
		}

	pos.x = 200; pos.y = 100;
wxString attribut_list[3] = { _("Normal"), _("Normal+Insert"), _("Virtual") };
	m_AttributsCtrl = new wxRadioBox( m_PanelProperties, -1, _("Attributs"), pos,
				wxSize(-1,-1), 3, attribut_list, 1);

	switch (m_CurrentModule->m_Attributs & 255)
		{
		case 0:
			m_AttributsCtrl->SetSelection( 0 );
			break;

		case MOD_CMS:
			m_AttributsCtrl->SetSelection( 1 );
			break;

		case MOD_VIRTUAL:
			m_AttributsCtrl->SetSelection( 2 );
			break;

		default:
			m_AttributsCtrl->SetSelection( 0 );
			break;
		}


	m_AttributsCtrl->GetSize(&xx, &yy);
	pos.y += 15 + yy;
wxString autoplace_list[2] = { _("Free"), _("Locked")};
	m_AutoPlaceCtrl = new wxRadioBox( m_PanelProperties, -1, _("Auto Place"), pos,
				wxSize(-1,-1), 2, autoplace_list, 1);
	m_AutoPlaceCtrl->SetSelection(
		(m_CurrentModule->m_ModuleStatus & MODULE_is_LOCKED) ? 1 : 0);

	m_AutoPlaceCtrl->GetSize(&xx, &yy);
	pos.y += 12 + yy;
	new wxStaticText(m_PanelProperties, -1, _("Rot 90"), pos);
	pos.y += 13;
	m_CostRot90Ctrl = new wxSlider(m_PanelProperties, -1,
				m_CurrentModule->m_CntRot90, 0, 10, pos, wxSize(100, -1),
				wxSL_HORIZONTAL + wxSL_AUTOTICKS + wxSL_LABELS) ;

	m_CostRot90Ctrl->GetSize(&xx, &yy);
	pos.y += 12 + yy;
	new wxStaticText(m_PanelProperties, -1, _("Rot 180"), pos);
	pos.y += 13;
	m_CostRot180Ctrl= new wxSlider(m_PanelProperties, -1,
				m_CurrentModule->m_CntRot180, 0, 10, pos, wxSize(100, -1),
				wxSL_HORIZONTAL + wxSL_AUTOTICKS + wxSL_LABELS) ;
}

/**************************************************************/
Panel3D_Ctrl::Panel3D_Ctrl(WinEDA_ModulePropertiesFrame * parentframe,
		wxNotebook * parent,
		int id, Struct3D_Master * struct3D):
		wxPanel(parent, id)
/**************************************************************/
{
wxPoint pos;
wxButton * button;
S3D_Vertex dummy_vertex;

	m_Pnext = m_Pback = NULL;

	m_Parent = parent;
	m_ParentFrame = parentframe;
	SetFont(*g_DialogFont);
	pos.x = 10; pos.y = 20;
	m_3D_ShapeName = new WinEDA_EnterText(this, _("3D Shape Name"),
						struct3D ? struct3D->m_Shape3DName : wxT(""),
						pos, wxSize(300,-1) );

	pos.x += 310;
	button = new wxButton(this, ID_BROWSE_3D_LIB,
						_("Browse"), pos);
	button->SetForegroundColour(*wxBLUE);

	pos.x -= 100; pos.y += 50;
	button = new wxButton(this, ID_ADD_3D_SHAPE,
						_("Add 3D Shape"), pos);
	button->SetForegroundColour(*wxRED);

	pos.y += 30;
	if ( (struct3D == NULL) || (struct3D->Pback != NULL) )
	{
		button = new wxButton(this, ID_REMOVE_3D_SHAPE,
						_("Remove 3D Shape"), pos);
		button->SetForegroundColour(*wxRED);
	}

	pos.x = 10; pos.y += m_3D_ShapeName->GetDimension().y + 20 - 80;
	m_3D_Scale = new WinEDA_VertexCtrl(this, _("Shape Scale:"),
						2, pos, 1);
	if ( struct3D ) m_3D_Scale->SetValue(struct3D->m_MatScale);

	pos.y += m_3D_Scale->GetDimension().y + 10;
	m_3D_Offset = new WinEDA_VertexCtrl(this, _("Shape Offset:"),
						2, pos, 1);
	if ( struct3D ) m_3D_Offset->SetValue(struct3D->m_MatPosition);
	else m_3D_Offset->SetValue(dummy_vertex);

	pos.y += m_3D_Offset->GetDimension().y + 10;
	m_3D_Rotation = new WinEDA_VertexCtrl(this, _("Shape Rotation:"),
						2, pos, 1);
	if ( struct3D ) m_3D_Rotation->SetValue(struct3D->m_MatRotation);
	else m_3D_Rotation->SetValue(dummy_vertex);

	if ( struct3D == NULL )
	{
		dummy_vertex.x = dummy_vertex.y = dummy_vertex.z = 1.0;
		m_3D_Scale->SetValue(dummy_vertex);
	}

}

/********************************/
Panel3D_Ctrl::~Panel3D_Ctrl(void)
/********************************/
{
	delete m_3D_ShapeName;
	delete m_3D_Scale;
	delete m_3D_Offset;
	delete m_3D_Rotation;
}


/***************************************************/
void Panel3D_Ctrl::Browse3DLib(wxCommandEvent& event)
/***************************************************/
{
wxString fullfilename, shortfilename;
wxString fullpath = g_RealLibDirBuffer;
wxString mask = wxT("*");

	fullpath += LIB3D_PATH;
	mask += g_Shapes3DExtBuffer;
#ifdef __WINDOWS__
	fullpath.Replace( wxT("/"), wxT("\\") );
#endif
	fullfilename = EDA_FileSelector( _("3D Shape:"),
					fullpath,			/* Chemin par defaut */
					wxEmptyString,					/* nom fichier par defaut */
					g_Shapes3DExtBuffer,	/* extension par defaut */
					mask,				/* Masque d'affichage */
					this,
					wxOPEN,
					TRUE
					);

	if ( fullfilename == wxEmptyString ) return;

	shortfilename = MakeReducedFileName( fullfilename,
						fullpath, wxEmptyString);
	m_3D_ShapeName->SetValue(shortfilename);
}


/**********************************************************************/
void  WinEDA_ModulePropertiesFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
/**********************************************************************/
{
    Close(true);    // true is to force the frame to close
}


/******************************************************************************/
void WinEDA_ModulePropertiesFrame::ModulePropertiesAccept(wxCommandEvent& event)
/******************************************************************************/
{
bool change_layer = FALSE;

	if ( m_DC )
		m_CurrentModule->Draw(m_Parent->DrawPanel, m_DC, wxPoint(0,0), GR_XOR);

	if ( m_OrientValue )
		{
		if( m_CurrentModule->m_Orient != m_OrientValue->GetValue() )
			m_Parent->Rotate_Module(m_DC, m_CurrentModule,
				 m_OrientValue->GetValue(), FALSE);
		}

	if ( m_LayerCtrl )
		{
		if ( m_LayerCtrl->GetSelection() == 0 )		// layer req = COMPONENT
			{
			if( m_CurrentModule->m_Layer == CUIVRE_N) change_layer = TRUE;
			}
		else if( m_CurrentModule->m_Layer == CMP_N ) change_layer = TRUE;
		}

	if ( change_layer )
	{
		m_Parent->Change_Side_Module(m_CurrentModule, m_DC);
	}

	if (m_AutoPlaceCtrl->GetSelection() == 1)
		m_CurrentModule->m_ModuleStatus |= MODULE_is_LOCKED;
	else m_CurrentModule->m_ModuleStatus &= ~MODULE_is_LOCKED;

	switch (m_AttributsCtrl->GetSelection())
		{
		case 0:
			m_CurrentModule->m_Attributs = 0;
			break;

		case 1:
			m_CurrentModule->m_Attributs = MOD_CMS;
			break;

		case 2:
			m_CurrentModule->m_Attributs = MOD_VIRTUAL;
			break;
		}

	m_CurrentModule->m_CntRot90 = m_CostRot90Ctrl->GetValue();
	m_CurrentModule->m_CntRot180 = m_CostRot180Ctrl->GetValue();
	if ( m_Doc ) m_CurrentModule->m_Doc = m_Doc->GetData();
	if ( m_Keyword ) m_CurrentModule->m_KeyWord = m_Keyword->GetData();

	/* Mise a jour des parametres 3D */
	Panel3D_Ctrl * panel3D =  m_Panel3D;
	Struct3D_Master *draw3D = m_CurrentModule->m_3D_Drawings,
		* nextdraw3D;
	for ( ; panel3D != NULL; panel3D = panel3D->m_Pnext )
	{
		draw3D->m_Shape3DName = panel3D->m_3D_ShapeName->GetData();
		draw3D->m_MatScale = panel3D->m_3D_Scale->GetValue();
		draw3D->m_MatRotation = panel3D->m_3D_Rotation->GetValue();
		draw3D->m_MatPosition = panel3D->m_3D_Offset->GetValue();
		if ( (draw3D->m_Shape3DName.IsEmpty() ) &&
			 (draw3D != m_CurrentModule->m_3D_Drawings) )
			continue;
		if ( (draw3D->Pnext == NULL) && panel3D->m_Pnext )
		{
			nextdraw3D = new Struct3D_Master(draw3D);
			nextdraw3D->Pback = draw3D;
			draw3D->Pnext = nextdraw3D;
		}
		draw3D = (Struct3D_Master *) draw3D->Pnext;
	}

	for ( ; draw3D != NULL; draw3D = nextdraw3D )
	{
		nextdraw3D = (Struct3D_Master *)draw3D->Pnext;
		(draw3D->Pback)->Pnext = NULL;
		delete draw3D;
	}


	m_CurrentModule->Set_Rectangle_Encadrement();

	m_Parent->GetScreen()->SetModify();

	Close(TRUE);

	if ( m_DC )
		m_CurrentModule->Draw(m_Parent->DrawPanel, m_DC, wxPoint(0,0), GR_OR);
}


/************************************************************************/
void WinEDA_ModulePropertiesFrame::GotoModuleEditor(wxCommandEvent& event)
/************************************************************************/
{
	GoToEditor = TRUE;
	if ( m_CurrentModule->m_TimeStamp == 0 )	// Module Editor needs a non null timestamp
	{
		m_CurrentModule->m_TimeStamp = GetTimeStamp();
		m_Parent->GetScreen()->SetModify();
	}

	Close(TRUE);
}


/**********************************************************************/
void WinEDA_ModulePropertiesFrame::ExchangeModule(wxCommandEvent& event)
/**********************************************************************/
{
	m_Parent->InstallExchangeModuleFrame( m_CurrentModule,
					m_DC, wxPoint(-1,-1) );
	// Attention: si il y a eu echange, m_CurrentModule a été delete!
	m_Parent->GetScreen()->m_CurrentItem = NULL;
	Close(TRUE);
}


/*************************************************************************/
void WinEDA_ModulePropertiesFrame::ModuleOrientEvent(wxCommandEvent& event)
/*************************************************************************/
{
	switch ( m_OrientCtrl->GetSelection() )
		{
		case 0:
			m_OrientValue->Enable(FALSE);
			m_OrientValue->SetValue( 0 );
			break;

		case 1:
			m_OrientValue->Enable(FALSE);
			m_OrientValue->SetValue( 900 );
			break;

		case 2:
			m_OrientValue->Enable(FALSE);
			m_OrientValue->SetValue( 2700 );
			break;

		case 3:
			m_OrientValue->Enable(FALSE);
			m_OrientValue->SetValue( 1800 );
			break;

		default:
			m_OrientValue->Enable(FALSE);
			m_OrientValue->Enable(TRUE);
			break;
		}
}


/*************************************************************************/
void WinEDA_ModulePropertiesFrame::SelectTextListBox(wxCommandEvent& event)
/*************************************************************************/
{
	if ( m_DeleteFieddButton == NULL ) return;
	if ( m_TextListBox->GetChoice() > 1 )	// Texte autre que ref ou valeur selectionne
		{
		m_DeleteFieddButton->Enable(TRUE);
		}
	else m_DeleteFieddButton->Enable(FALSE);
}


/***********************************************************/
void WinEDA_ModulePropertiesFrame::ReCreateFieldListBox(void)
/***********************************************************/
{
	m_TextListBox->Clear();

	m_TextListBox->Append(m_CurrentModule->m_Reference->m_Text);
	m_TextListBox->Append(m_CurrentModule->m_Value->m_Text);

	EDA_BaseStruct * item = m_CurrentModule->m_Drawings;
	while ( item )
		{
		if( item->m_StructType == TYPETEXTEMODULE )
			m_TextListBox->Append( ((TEXTE_MODULE*)item)->m_Text );
		item = item->Pnext;
		}
}


/************************************************************************/
void WinEDA_ModulePropertiesFrame::CreateTextModule(wxCommandEvent& event)
/************************************************************************/
/* Cree un nouveau texte sur le module actif
	Le texte sera mis en fonction Move
*/
{
TEXTE_MODULE * Text;

	/* Creation de la place en memoire : */
	Text = m_Parent->CreateTextModule(m_CurrentModule, m_DC);

	ReCreateFieldListBox();
	m_TextListBox->SetSelection(2);
}


/****************************************************************************/
void WinEDA_ModulePropertiesFrame::EditOrDelTextModule(wxCommandEvent& event)
/****************************************************************************/
{
int ii = m_TextListBox->GetChoice();
TEXTE_MODULE * Text = NULL;

	if ( ii < 0 ) return;

	if ( ii == 0 ) Text = m_CurrentModule->m_Reference;
	else if ( ii == 1 ) Text = m_CurrentModule->m_Value;
	else
		{
		EDA_BaseStruct * item = m_CurrentModule->m_Drawings;
		int jj = 2;
		while ( item )
			{
			if( item->m_StructType == TYPETEXTEMODULE )
				{
				if ( jj == ii )	// Texte trouvé
					{
					Text = (TEXTE_MODULE*)item;
					break;
					}
				}
			item = item->Pnext; jj++;
			}
		}

	if ( Text )
	{
		if ( event.GetId() == ID_MODULE_EDIT_DELETE_TEXT )
		{
			wxString Line;
			Line.Printf( _("Delete [%s]"), Text->m_Text.GetData() );
			if ( ii < 2 ) { wxBell(); return; }		// Ref ou Value non effacables
			if ( !IsOK(this, Line ) ) return;
			m_Parent->DeleteTextModule(Text, m_DC);
			ReCreateFieldListBox();
			m_TextListBox->SetSelection(0);
		}
		else	// Edition du champ
			{
			m_Parent->InstallTextModOptionsFrame(Text, m_DC, wxPoint(-1,-1) );
			ReCreateFieldListBox();
			m_TextListBox->SetSelection(ii);
			}
		}
}


/*******************************************************************/
void WinEDA_ModuleEditFrame::Place_Ancre(MODULE* pt_mod , wxDC * DC)
/*******************************************************************/
/*
 Repositionne l'ancre sous le curseur souris
	Le module doit etre d'abort selectionne
*/
{
int deltaX, deltaY;
EDA_BaseStruct * PtStruct;
D_PAD * pt_pad;

	if(pt_mod == NULL) 	return ;

	pt_mod->DrawAncre(DrawPanel, DC, wxPoint(0,0), DIM_ANCRE_MODULE, GR_XOR);

	deltaX = pt_mod->m_Pos.x - GetScreen()->m_Curseur.x;
	deltaY = pt_mod->m_Pos.y - GetScreen()->m_Curseur.y;

	pt_mod->m_Pos = GetScreen()->m_Curseur;

	/* Mise a jour des coord relatives des elements:
	les coordonnees relatives sont relatives a l'ancre, pour orient 0.
	il faut donc recalculer deltaX et deltaY en orientation 0 */
	RotatePoint(&deltaX, &deltaY, - pt_mod->m_Orient);

	/* Mise a jour des coord relatives des pads */
	pt_pad = (D_PAD*)pt_mod->m_Pads;
	for( ; pt_pad != NULL; pt_pad = (D_PAD*) pt_pad->Pnext)
		{
		pt_pad->m_Pos0.x += deltaX; pt_pad->m_Pos0.y += deltaY;
		}

	/* Mise a jour des coord relatives contours .. */
	PtStruct = pt_mod->m_Drawings;
	for( ; PtStruct != NULL; PtStruct = PtStruct->Pnext)
		{
		switch( PtStruct->m_StructType)
			{
			case TYPEEDGEMODULE:
				#undef STRUCT
				#define STRUCT ((EDGE_MODULE*) PtStruct)
				STRUCT->m_Start0.x += deltaX; STRUCT->m_Start0.y += deltaY;
				STRUCT->m_End0.x += deltaX; STRUCT->m_End0.y += deltaY;
				break;

			case TYPETEXTEMODULE:
				#undef STRUCT
				#define STRUCT ((TEXTE_MODULE*) PtStruct)
				STRUCT->m_Pos0.x += deltaX; STRUCT->m_Pos0.y += deltaY;
				break;

			default:
				break;
			}
		}
	pt_mod->Set_Rectangle_Encadrement();
	pt_mod->DrawAncre(DrawPanel, DC, wxPoint(0,0), DIM_ANCRE_MODULE, GR_OR);
}

/**********************************************************************/
void WinEDA_ModuleEditFrame::RemoveStruct(EDA_BaseStruct * Item, wxDC * DC)
/**********************************************************************/
{
	if ( Item == NULL ) return;

	switch( Item->m_StructType )
		{
		case TYPEPAD:
			DeletePad( (D_PAD*) Item, DC);
			break;

		case TYPETEXTEMODULE:
			{
			TEXTE_MODULE * text = (TEXTE_MODULE *) Item;
			if ( text->m_Type == TEXT_is_REFERENCE )
				{
				DisplayError(this, _("Text is REFERENCE!") );
				break;
				}
			if ( text->m_Type == TEXT_is_VALUE )
				{
				DisplayError(this, _("Text is VALUE!") );
				break;
				}
			DeleteTextModule(text, DC);
			}
			break;

		case TYPEEDGEMODULE:
			Delete_Edge_Module((EDGE_MODULE *) Item, DC);
			break;

		case TYPEMODULE:
			break;

		default:
			{
			wxString Line;
			Line.Printf( wxT(" Remove: StructType %d Inattendu"),
										Item->m_StructType);
			DisplayError(this, Line);
			}
			break;
		}
}

