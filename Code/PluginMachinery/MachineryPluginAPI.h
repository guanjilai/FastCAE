#ifndef _MACHINERYPLUGINAPI_H__
#define _MACHINERYPLUGINAPI_H__

#include <QtCore/QtGlobal>


#if defined(MACHINERYPLUGIN_API)
#define MACHINERYPLUGINAPI Q_DECL_EXPORT
#else
#define  MACHINERYPLUGINAPI Q_DECL_IMPORT
#endif

#endif
