/* Date for kicad build version */

#ifndef KICAD_BUILD_VERSION
#define KICAD_BUILD_VERSION

COMMON_GLOBL wxString g_BuildVersion
#ifdef EDA_BASE
	(wxT("(2006-04-24)"))
#endif
;

#endif	// KICAD_BUILD_VERSION
