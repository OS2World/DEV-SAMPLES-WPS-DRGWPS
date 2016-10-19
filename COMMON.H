/*********************************************************************
 *                                                                   *
 * MODULE NAME :  common.h               AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  08-01-93                                           *
 *                                                                   *
 * DESCRIPTION:                                                      *
 *                                                                   *
 *  Common definitions and function prototypes used between the PM   *
 *  window and the WPS object in the DRGPMWPS.EXE sample program.    *
 *                                                                   *
 * HISTORY:                                                          *
 *                                                                   *
 *  08-01-93 - Coding started.                                       *
 *                                                                   *
 *  Rick Fishman                                                     *
 *  Code Blazers, Inc.                                               *
 *  4113 Apricot                                                     *
 *  Irvine, CA. 92720                                                *
 *  CIS ID: 72251,750                                                *
 *                                                                   *
 *********************************************************************/

#if !defined(COMMON_H)
#define COMMON_H

/*********************************************************************/
/*------------------- APPLICATION DEFINITIONS -----------------------*/
/*********************************************************************/

#define AGENT_OBJECT_ID     "<DrgAgentObject>"
#define AGENT_CLASS_NAME    "DrgAgent"
#define AGENT_DLL_NAME      "DrgAgent"

#define UM_DRAGINFO_REQUEST       WM_USER
#define UM_FORMAT_DRAGITEM        WM_USER + 1
#define UM_SET_HOBJECT            WM_USER + 2
#define UM_DROP                   WM_USER + 3
#define UM_STARTING_DRAG          WM_USER + 4
#define UM_DRAGINFO               WM_USER + 5
#define UM_DROP_ENDCONVERSATION   WM_USER + 6

/**********************************************************************/
/*---------------------------- STRUCTURES ----------------------------*/
/**********************************************************************/

// Data shared by all processes

typedef struct _COMMDATA
{
    HWND hwndComm;
} COMMDATA, *PCOMMDATA;

typedef struct _OBJECTDATA
{
    BOOL fPgmObject;
    char szClassName[ 100 ];
    char szTitle[ 100 ];
    char szExeName[ CCHMAXPATH ];
    char szParms[ CCHMAXPATH ];
    char szStartupDir[ CCHMAXPATH ];
} OBJECTDATA, *POBJECTDATA;

/**********************************************************************/
/*----------------------- FUNCTION PROTOTYPES ------------------------*/
/**********************************************************************/

// In commwin.c

void CommwinCreate( void *somClass );
void CommwinDestroy( void );
void Msg( PSZ szFormat, ... );

// In commdata.c

void CommdataCreate( void );
void CommdataDestroy( void );

// In heap.c

void HeapCreate( void );
void *HeapAlloc( ULONG cb );
void HeapFree( void *pBlock );
void HeapDestroy( void );

/**********************************************************************/
/*------------------------ GLOBAL VARIABLES --------------------------*/
/**********************************************************************/

#ifdef GLOBALS_DEFINED
#   define DATADEF
#else
#   define DATADEF extern
#endif

DATADEF PCOMMDATA pCommData;

#endif // COMMON_H
/***************************************************************************
 *                         E N D   O F   S O U R C E                       *
 ***************************************************************************/
