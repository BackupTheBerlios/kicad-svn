/* Macros utiles */

#ifndef MACROS_H
#define MACROS_H

#ifndef min
#define min(x, y)		((x) > (y) ? (y) : (x))
#endif
#ifndef max
#define max(x, y)		((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x, y)		((x) > (y) ? (y) : (x))
#endif
#ifndef MAX
#define MAX(x, y)		((x) > (y) ? (x) : (y))
#endif

#ifndef ABS
#define ABS(y)			((y) >= 0 ? (y) : (-(y)))
#endif

#define DEG2RAD(Deg)	((Deg) * M_PI / 180.0)
#define RAD2DEG(Rad)	((Rad) * 180.0 / M_PI)

/* Normalize angle to be in the -360.0 .. 360.0 range or 0 .. 360.0: */
#define	NORMALIZE_ANGLE(Angle)	{ while (Angle < 0) Angle += 3600; \
				  while (Angle > 3600) Angle -= 3600; }

/* Normalize angle to be in the 0.0 .. 360.0 range: */
#define	NORMALIZE_ANGLE_POS(Angle)	{ while (Angle < 0) Angle += 3600; while (Angle >= 3600) Angle -= 3600; }
#define	NEGATE_AND_NORMALIZE_ANGLE_POS(Angle) \
	{ Angle = - Angle; while (Angle < 0) Angle += 3600; while (Angle >= 3600) Angle -= 3600; }


/* Macro de selection du mode XOR convenable pour les traces en mode XOR,
	selon la couleur du fond */
#define XOR_MODE XorMode

/* exchange 2 donnees */
#define EXCHG(a,b) { typeof(a) __temp__ = (a); (a) = (b); (b) = __temp__; }

/* macro for insert menuitems with a icon: */
#define ADD_MENUITEM(menu, id, text, icon) {\
		wxMenuItem * l_item;\
		l_item = new wxMenuItem(menu, id, text);\
		l_item->SetBitmap(icon); menu->Append(l_item);}

#ifdef __WINDOWS__
#define ADD_MENUITEM_WITH_SUBMENU(menu, submenu, id, text, icon) {\
		wxMenuItem * l_item;\
		l_item = new wxMenuItem(menu, id, text);\
		l_item->SetSubMenu(submenu);\
		l_item->SetBitmap(icon);\
		l_item->SetFont(* g_ItalicFont);\
		menu->Append(l_item);}

#define ADD_MENUITEM_WITH_HELP_AND_SUBMENU(menu, submenu, id, text, help, icon) {\
		wxMenuItem * l_item;\
		l_item = new wxMenuItem(menu, id, text, help);\
		l_item->SetSubMenu(submenu);\
		l_item->SetBitmap(icon);\
		l_item->SetFont(*g_ItalicFont);\
		menu->Append(l_item);}
#else
#define ADD_MENUITEM_WITH_SUBMENU(menu, submenu, id, text, icon) {\
		wxMenuItem * l_item;\
		l_item = new wxMenuItem(menu, id, text);\
		l_item->SetSubMenu(submenu);\
		l_item->SetBitmap(icon);\
		menu->Append(l_item);}

#define ADD_MENUITEM_WITH_HELP_AND_SUBMENU(menu, submenu, id, text, help, icon) {\
		wxMenuItem * l_item;\
		l_item = new wxMenuItem(menu, id, text, help);\
		l_item->SetSubMenu(submenu);\
		l_item->SetBitmap(icon);\
		menu->Append(l_item);}
#endif

#endif /* ifdef MACRO_H */

