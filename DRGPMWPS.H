/*********************************************************************
 *                                                                   *
 * MODULE NAME :  drgpmwps.h             AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  08-01-93                                           *
 *                                                                   *
 * DESCRIPTION:                                                      *
 *                                                                   *
 *  Common definitions and function prototypes for DRGPMWPS.EXE      *
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

#if !defined(DRGPMWPS_H)
#define DRGPMWPS_H

/*********************************************************************/
/*------------------- APPLICATION DEFINITIONS -----------------------*/
/*********************************************************************/

#define DEBUG_FILENAME        "drgpmwps.dbg"

#define PROGRAM_ICON_FILENAME "drgpmwps.ico"

#define ID_RESOURCES          10

#define IDM_EXIT              20   // Accelerator key

#ifndef CRA_SOURCE            // As of 09/03/93, CRA_SOURCE not in toolkit hdrs
#  define CRA_SOURCE          0x00004000L
#endif

/*********************************************************************/
/*-------------------------- HELPER MACROS --------------------------*/
/*********************************************************************/

#define ANCHOR(hwnd)   (WinQueryAnchorBlock( hwnd ))
#define HWNDERR(hwnd)  (ERRORIDERROR(WinGetLastError( ANCHOR( hwnd ))))
#define HABERR(hab)    (ERRORIDERROR(WinGetLastError( hab )))
#define PARENT(hwnd)   (WinQueryWindow( hwnd, QW_PARENT ))

/**********************************************************************/
/*---------------------------- STRUCTURES ----------------------------*/
/**********************************************************************/

typedef struct _CNRREC                            // CONTAINER RECORD STRUCTURE
{
  MINIRECORDCORE mrc;
  char           szFileName[ CCHMAXPATH ];        // File that icon represents

} CNRREC, *PCNRREC;

#define EXTRA_BYTES (sizeof( CNRREC ) - sizeof( MINIRECORDCORE ))

/**********************************************************************/
/*----------------------- FUNCTION PROTOTYPES ------------------------*/
/**********************************************************************/

// In drag.c

void    dragInit ( HWND hwndFrame, PCNRDRAGINIT pcdi );
MRESULT dragOver ( HWND hwndFrame, PCNRDRAGINFO pcdi );
void    dragDrop ( HWND hwndFrame, PCNRDRAGINFO pcdi );

// In drgpmwps.c

void Msg( PSZ szFormat, ... );

/**********************************************************************/
/*------------------------ GLOBAL VARIABLES --------------------------*/
/**********************************************************************/

#ifdef GLOBALS_DEFINED
#   define DATADEF
#else
#   define DATADEF extern
#endif

DATADEF char szCurrentPath[ CCHMAXPATH ]; // Current path where pgm was loaded
DATADEF PCNRREC pCnrRecBeingDragged;      // Container Record being dragged

#endif  // DRGPMWPS_H
/***************************************************************************
 *                         E N D   O F   S O U R C E                       *
 ***************************************************************************/
