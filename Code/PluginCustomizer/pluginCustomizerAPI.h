#ifndef _CUSTOMIZERPLUGINAPI_H__
#define _CUSTOMIZERPLUGINAPI_H__

#include <QtCore/QtGlobal>


#if defined(CUSTOMIZERPLUGIN_API)
#define CUSTOMIZERPLUGINAPI Q_DECL_EXPORT
#else
#define CUSTOMIZERPLUGINAPI Q_DECL_IMPORT
#endif

#endif
