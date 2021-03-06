/////////////////////////////////////////////////////////////////////////////
// Name:        3d_viewer.h
/////////////////////////////////////////////////////////////////////////////
#if !wxUSE_GLCANVAS
#error Please set wxUSE_GLCANVAS to 1 in setup.h.
#endif

#include "wx/glcanvas.h"

#ifdef __WXMAC__
#  ifdef __DARWIN__
#    include <OpenGL/gl.h>
#    include <OpenGL/glu.h>
#  else
#    include <gl.h>
#    include <glu.h>
#  endif
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#endif

#ifdef VIEWER_MAIN
#define global_3d
#else
#define global_3d extern
#endif

#include "pcbstruct.h"

#define LIB3D_PATH wxT("packages3d/")

class Pcb3D_GLCanvas;
class WinEDA3D_DrawFrame;
class Info_3D_Visu;
class S3D_Vertex;
class SEGVIA;


#define m_ROTX m_Rot[0]
#define m_ROTY m_Rot[1]
#define m_ROTZ m_Rot[2]

/* information needed to display 3D board */
class Info_3D_Visu
{
public:
    float m_Beginx, m_Beginy;	/* position of mouse */
    float m_Quat[4];			/* orientation of object */
    float m_Rot[4];				/* man rotation of object */
    float m_Zoom;				/* field of view in degrees */
	wxPoint m_BoardPos;
	wxSize m_BoardSize;
	int m_Layers;
	EDA_BoardDesignSettings * m_BoardSettings;	// Link to current board design settings
	float m_Epoxy_Width;		/* Epoxy tickness (normalized) */

	float m_BoardScale;			/* Normalisation scale for coordinates:
								when scaled tey are between -1.0 and +1.0 */
	float m_LayerZcoord[32];
public:
	Info_3D_Visu(void);
	~Info_3D_Visu(void);
};


class Pcb3D_GLCanvas: public wxGLCanvas
{
public:
	WinEDA3D_DrawFrame * m_Parent;

private:
	bool   m_init;
	GLuint m_gllist;

public:
	Pcb3D_GLCanvas(WinEDA3D_DrawFrame *parent, const wxWindowID id = -1,
			int* gl_attrib = NULL);
	~Pcb3D_GLCanvas(void);

	void ClearLists(void);

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
	void OnChar(wxKeyEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
	void OnRightClick(wxMouseEvent& event);
	void OnPopUpMenu(wxCommandEvent & event);
	void TakeScreenshot(wxCommandEvent & event);
	void SetView3D(int keycode);
	void DisplayStatus(void);
	void Redraw(void);
	GLuint DisplayCubeforTest(void);

	void OnEnterWindow( wxMouseEvent& event );

	void Render( void );
	GLuint CreateDrawGL_List(void);
	void InitGL(void);
	void SetLights(void);
	void Draw3D_Track(TRACK * track);
	void Draw3D_Via(SEGVIA * via);
	void Draw3D_DrawSegment(DRAWSEGMENT * segment);

DECLARE_EVENT_TABLE()
};


class WinEDA3D_DrawFrame: public wxFrame
{
public:
	WinEDA_BasePcbFrame * m_Parent;
	WinEDA_App * m_ParentAppl;
    Pcb3D_GLCanvas * m_Canvas;
	wxToolBar * m_HToolBar;
	wxToolBar * m_VToolBar;
	int m_InternalUnits;
	wxPoint m_FramePos;
	wxSize m_FrameSize;

private:
	wxString m_FrameName;		// name used for writting and reading setup
								// It is "Frame3D"
	

public:
	WinEDA3D_DrawFrame(WinEDA_BasePcbFrame * parent, WinEDA_App *app_parent,
	const wxString& title );

    void Exit3DFrame(wxCommandEvent& event);
	void OnCloseWindow(wxCloseEvent & Event);
	void ReCreateMenuBar(void);
	void ReCreateHToolbar(void);
	void ReCreateVToolbar(void);
	void SetToolbars(void);
	void GetSettings(void);
	void SaveSettings(void);

	void OnLeftClick(wxDC * DC, const wxPoint& MousePos);
	void OnRightClick(const wxPoint& MousePos, wxMenu * PopMenu);
	void OnKeyEvent(wxKeyEvent& event);
	int BestZoom(void);	// Retourne le meilleur zoom
	void RedrawActiveWindow(wxDC * DC, bool EraseBg);
	void Process_Special_Functions(wxCommandEvent& event);
	void Process_Zoom(wxCommandEvent& event);

	void NewDisplay(void);

DECLARE_EVENT_TABLE()
};

void SetGLColor(int color);
void Set_Object_Data(const S3D_Vertex * coord, int nbcoord );

global_3d  Info_3D_Visu g_Parm_3D_Visu;
global_3d double Draw3d_dx, Draw3d_dy;
global_3d double ZBottom, ZTop;
global_3d double DataScale3D;		// coeff de conversion unites utilsateut -> unites 3D
global_3d int gl_attrib[]
#ifdef VIEWER_MAIN
= { WX_GL_RGBA, WX_GL_MIN_RED, 8, WX_GL_MIN_GREEN, 8,
            WX_GL_MIN_BLUE, 8, WX_GL_DEPTH_SIZE, 16,
            WX_GL_DOUBLEBUFFER,
			GL_NONE }
#endif
;


