
/*
 * This file was generated by the SOM Compiler.
 * FileName: drgagent.h.
 * Generated using:
 *     SOM Precompiler spc: 1.22
 *     SOM Emitter emith: 1.11
 */

#ifndef DrgAgent_h
#define DrgAgent_h


#include <som.h>

/*
 * Define the class name as an object type
 */
#ifndef DrgAgent
#define DrgAgent SOMAny
#endif
#include "wpabs.h"

#define DrgAgent_MajorVersion 1
#define DrgAgent_MinorVersion 1

/*
 * External name shortening if required
 */
#ifdef SOM_USE_SHORT_EXTERNAL_NAMES
#define DrgAgentNewClass c
#pragma linkage(c, system)
#define DrgAgentClassData d
#define DrgAgentCClassData x
#endif /* SOM_USE_SHORT_EXTERNAL_NAMES */
/*--------------Migration------------*/
#define DrgAgent_classObj DrgAgentClassData.classObject
#define _DrgAgent DrgAgent_classObj

/*
 * New and Renew macros for DrgAgent
 */
#define _DrgAgentNew() (_somNew(DrgAgentClassData.classObject))
#define DrgAgentNew() \
	( DrgAgentClassData.classObject \
		? _DrgAgentNew() \
		: ( DrgAgentNewClass(DrgAgent_MajorVersion, DrgAgent_MinorVersion), _DrgAgentNew() ) )
#define DrgAgentRenew(buf) \
	( _somRenew(DrgAgentClassData.classObject, buf) )

/*
 * Override method: wpDraggedOverObject
 */
#define DrgAgent_wpDraggedOverObject(somSelf,DraggedOverObject) \
	WPObject_wpDraggedOverObject(somSelf,DraggedOverObject)

/*
 * Override method: wpDroppedOnObject
 */
#define DrgAgent_wpDroppedOnObject(somSelf,DroppedOnObject) \
	WPObject_wpDroppedOnObject(somSelf,DroppedOnObject)

/*
 * Declare the class creation procedure
 */
SOMEXTERN SOMAny * SOMLINK DrgAgentNewClass(integer4 scemajorVersion,
		integer4 sceminorVersion);
#pragma linkage(DrgAgentNewClass, system)

/*
 * Declare the class data structure
 */
SOMEXTERN struct DrgAgentClassDataStructure {
	SOMAny *classObject;
} DrgAgentClassData;

/*
 * Declare the C specific class data structure
 */
SOMEXTERN struct DrgAgentCClassDataStructure {
	somMethodTab *parentMtab;
	somDToken instanceDataToken;
} DrgAgentCClassData;

#endif       /* DrgAgent_h */

/*
 * This file was generated by the SOM Compiler.
 * FileName: drgagent.h.
 * Generated using:
 *     SOM Precompiler spc: 1.22
 *     SOM Emitter emith: 1.11
 */

#ifndef DrgAgent_mh
#define DrgAgent_mh


/*
 * Define the class name as an object type
 */
#ifndef M_DrgAgent
#define M_DrgAgent SOMAny
#endif

#define M_DrgAgent_MajorVersion 1
#define M_DrgAgent_MinorVersion 1

/*
 * External name shortening if required
 */
#ifdef SOM_USE_SHORT_EXTERNAL_NAMES
#define M_DrgAgentNewClass c
#pragma linkage(c, system)
#define M_DrgAgentClassData d
#define M_DrgAgentCClassData x
#endif /* SOM_USE_SHORT_EXTERNAL_NAMES */
/*--------------Migration------------*/
#define M_DrgAgent_classObj M_DrgAgentClassData.classObject
#define _M_DrgAgent M_DrgAgent_classObj

/*
 * New and Renew macros for M_DrgAgent
 */
#define _M_DrgAgentNew() (_somNew(M_DrgAgentClassData.classObject))
#define M_DrgAgentNew() \
	( M_DrgAgentClassData.classObject \
		? _M_DrgAgentNew() \
		: ( M_DrgAgentNewClass(M_DrgAgent_MajorVersion, M_DrgAgent_MinorVersion), _M_DrgAgentNew() ) )
#define M_DrgAgentRenew(buf) \
	( _somRenew(M_DrgAgentClassData.classObject, buf) )

/*
 * Override method: wpclsQueryIconData
 */
#define M_DrgAgent_wpclsQueryIconData(somSelf,pIconInfo) \
	M_WPObject_wpclsQueryIconData(somSelf,pIconInfo)

/*
 * Override method: wpclsQueryStyle
 */
#define M_DrgAgent_wpclsQueryStyle(somSelf) \
	M_WPObject_wpclsQueryStyle(somSelf)

/*
 * Override method: wpclsQueryTitle
 */
#define M_DrgAgent_wpclsQueryTitle(somSelf) \
	M_WPObject_wpclsQueryTitle(somSelf)

/*
 * Override method: wpclsInitData
 */
#define M_DrgAgent_wpclsInitData(somSelf) \
	M_WPObject_wpclsInitData(somSelf)

/*
 * Override method: wpclsUnInitData
 */
#define M_DrgAgent_wpclsUnInitData(somSelf) \
	M_WPObject_wpclsUnInitData(somSelf)

/*
 * Declare the class creation procedure
 */
SOMEXTERN SOMAny * SOMLINK M_DrgAgentNewClass(integer4 scemajorVersion,
		integer4 sceminorVersion);
#pragma linkage(M_DrgAgentNewClass, system)

/*
 * Declare the class data structure
 */
SOMEXTERN struct M_DrgAgentClassDataStructure {
	SOMAny *classObject;
} M_DrgAgentClassData;

/*
 * Declare the C specific class data structure
 */
SOMEXTERN struct M_DrgAgentCClassDataStructure {
	somMethodTab *parentMtab;
	somDToken instanceDataToken;
} M_DrgAgentCClassData;

#endif       /* DrgAgent_h */
