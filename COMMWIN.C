/*********************************************************************
 *                                                                   *
 * MODULE NAME :  commwin.c              AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  08-01-93                                           *
 *                                                                   *
 * DESCRIPTION:                                                      *
 *                                                                   *
 *   This module contains the code for the object window used by the *
 *   agent object to communicate with the PM window.                 *
 *                                                                   *
 * CALLABLE FUNCTIONS:                                               *
 *                                                                   *
 *   CommwinCreate                                                   *
 *   CommwinDestroy                                                  *
 *                                                                   *
 * HISTORY:                                                          *
 *                                                                   *
 *  08-01-93 - Started coding.                                       *
 *                                                                   *
 *  Rick Fishman                                                     *
 *  Code Blazers, Inc.                                               *
 *  4113 Apricot                                                     *
 *  Irvine, CA. 92720                                                *
 *  CIS ID: 72251,750                                                *
 *                                                                   *
 *********************************************************************/

#pragma strings(readonly)

/*********************************************************************/
/*------- Include relevant sections of the OS/2 header files --------*/
/*********************************************************************/

#define  INCL_DOSPROCESS
#define  INCL_WINERRORS
#define  INCL_WINFRAMEMGR
#define  INCL_WINPROGRAMLIST
#define  INCL_WINWINDOWMGR
#define  INCL_WINWORKPLACE

/**********************************************************************/
/*----------------------------- INCLUDES -----------------------------*/
/**********************************************************************/

#include <os2.h>
#include <process.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma info( none )
#include "drgagent.ih"
#pragma info( restore )

#include "wppgm.h"
#include "common.h"

/*********************************************************************/
/*------------------- APPLICATION DEFINITIONS -----------------------*/
/*********************************************************************/

#define THREAD_STACKSIZE    58000       // Stacksize for secondary thread

#define INSTDATA( hwnd )    ((PINSTANCE) WinQueryWindowPtr( hwnd, 0 ))

#define ANCHOR( hwnd )      (WinQueryAnchorBlock( hwnd ))
#define HWNDERR( hwnd )     (ERRORIDERROR( WinGetLastError( ANCHOR(hwnd) ) ))
#define HABERR( hab )       (ERRORIDERROR( WinGetLastError( hab ) ))

/**********************************************************************/
/*---------------------------- STRUCTURES ----------------------------*/
/**********************************************************************/

typedef struct _INSTANCE
{
    ULONG cb;
    M_DrgAgent *somClass;
    DrgAgent   *somObject;
    HWND       hwndDrag;

} INSTANCE, *PINSTANCE;

/**********************************************************************/
/*----------------------- FUNCTION PROTOTYPES ------------------------*/
/**********************************************************************/

void DragInfoRequest( ULONG ulItemID, HWND hwndRequestor );
MRESULT FormatDragItem( HWND hwndObject, PDRAGITEM pDragItem );
void SetSomObject( HWND hwndObject, HOBJECT hObj );
MRESULT BeenDroppedOn( HWND hwndObject, SOMAny *somObject );
extern VOID _Optlink CommThread( void * );

FNWP wpComm;

/**********************************************************************/
/*------------------------ GLOBAL VARIABLES --------------------------*/
/**********************************************************************/

HWND hwndComm;

/**********************************************************************/
/*-------------------------- CommwinCreate ---------------------------*/
/*                                                                    */
/*  CREATE THE OBJECT WINDOW THAT WILL COMMUNICATE WITH PM WINDOWS.   */
/*                                                                    */
/*  PARMS: pointer to class object                                    */
/*                                                                    */
/*  NOTES: The object window is put on a separate thread because the  */
/*         thread that we are now on is transitory and could be       */
/*         destroyed at anytime without us knowing about it.          */
/*                                                                    */
/*         Remember - we're in WPS-land now!                          */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void CommwinCreate( void *somClass )
{
    if( -1 == _beginthread( CommThread, NULL, THREAD_STACKSIZE, somClass ) )
    {
        DosBeep( 1000, 1000 );
        printf( "_beginthread failed in CommwinCreate\n" );
    }
}

const char pszWindowClass[] = "CommWindow";

extern VOID _Optlink CommThread( void *somClassIn )
{
    PINSTANCE pi;
    HAB       hab = WinInitialize( 0 );
    HMQ       hmq = WinCreateMsgQueue( hab, 0 );
    QMSG      qmsg;

    pi = (PINSTANCE) malloc( sizeof( INSTANCE ) );

    if( pi )
    {
        pi->cb = sizeof( INSTANCE );
        pi->somClass = somClassIn;
        pi->somObject = NULL;

        WinRegisterClass( hab, pszWindowClass, wpComm, 0, sizeof( PVOID ) );

        hwndComm = WinCreateWindow( HWND_OBJECT, pszWindowClass, NULL, 0, 0, 0,
                                    0, 0, NULLHANDLE, HWND_TOP, 1, pi, NULL );
        if( hwndComm )
        {
            while( WinGetMsg( hab, &qmsg, NULLHANDLE, 0, 0 ) )
                WinDispatchMsg( hab, &qmsg );

            WinDestroyWindow( hwndComm );
        }
        else
            Msg( "WinCreateWindow failed with rc(%X)", HABERR( hab ) );
    }
    else
        Msg( "Out of memory in CommThread!" );

    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );

    _endthread();
}


/**********************************************************************/
/*------------------------- CommwinDestroy ---------------------------*/
/*                                                                    */
/*  DESTROY THE OBJECT WINDOW THAT COMMUNICATES WITH PM WINDOWS.      */
/*                                                                    */
/*  PARMS: nothing                                                    */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void CommwinDestroy()
{
    WinPostMsg( hwndComm, WM_QUIT, NULL, NULL );
}


/**********************************************************************/
/*----------------------------- wpComm -------------------------------*/
/*                                                                    */
/*  COMM WINDOW PROCEDURE                                             */
/*                                                                    */
/*  PARMS: standard winproc parms                                     */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: standard winproc return value                            */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
MRESULT EXPENTRY wpComm( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    switch( msg )
    {
        case WM_CREATE:
        {
            PINSTANCE pi = (PINSTANCE) mp1;

            WinSetWindowPtr( hwnd, 0, pi );

            // Create the shared-memory heap used to allocate chunks of memory
            // that can be used to pass information between the WPS object and
            // the PM window.

            HeapCreate();

            // Create the shared memory that will contain information shared by
            // both the PM window and the WPS object.

            CommdataCreate();

            // Set our window handle into the shared memory so the PM window
            // can use it.

            if( pCommData )
                pCommData->hwndComm = hwnd;
            break;
        }

        case WM_DESTROY:
        {
            PINSTANCE pi = INSTDATA( hwnd );

            if( pi )
            {
                free( pi );
                WinSetWindowPtr( hwnd, 0, NULL );
            }

            // Destroy the shared memory and shared heap.

            CommdataDestroy();
            HeapDestroy();
            break;
        }

        case UM_FORMAT_DRAGITEM:

            // This is sent by the PM window to allow us to format a DRAGITEM
            // structure. It has preallocated the DRAGITEM struct in the
            // shared heap.

            return FormatDragItem( hwnd, (PDRAGITEM) mp1 );

        case UM_SET_HOBJECT:

            // This is sent by the PM window so that we know the HOBJECT that
            // we are communicating with.

            SetSomObject( hwnd, (HOBJECT) mp1 );
            return 0;

        case UM_DRAGINFO_REQUEST:

            // This is sent to us by the PM window when a WPS object is dropped
            // on it and it needs more info about that object.

            DragInfoRequest( (ULONG) mp1, (HWND) mp2 );
            return 0;

        case UM_STARTING_DRAG:

            // The PM window is giving us its window handle so we can use it
            // if its icon is dropped on a WPS object and we need to inform it.

            INSTDATA( hwnd )->hwndDrag = (HWND) mp1;
            return 0;

        case UM_DROP:

            // This is sent to us from the WPS object when it gets a
            // wpDroppedOnObject method call.

            return BeenDroppedOn( hwnd, (SOMAny *) mp1 );

        case UM_DROP_ENDCONVERSATION:
        {
            ULONG  ulItemID = LONGFROMMP( mp1 );
            SOMAny *somObject = OBJECT_FROM_PREC( ulItemID );

            // This is sent to us from the PM window when it is done with a
            // drop.

            _wpEndConversation( somObject, ulItemID, DMFL_TARGETSUCCESSFUL );
            return 0;
        }
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

/**********************************************************************/
/*------------------------- DragInfoRequest --------------------------*/
/*                                                                    */
/*  A WINDOW HAS REQUESTED DRAG INFORMATION. SUPPLY IT.               */
/*                                                                    */
/*  PARMS: DRAGITEM.ulItemID,                                         */
/*         window handle of the window that requested the info        */
/*                                                                    */
/*  NOTES: Populate a shared memory block with information about the  */
/*         DRAGITEM whose ulItemID was passed to us. We allocate the  */
/*         block of memory from the shared heap. The PM window will   */
/*         free that block when it is done with it.                   */
/*                                                                    */
/*         We only get a lot of information if this is a WPProgram    */
/*         object. Obviously you can get more information on other    */
/*         objects. This is just used to show how to do it.           */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void DragInfoRequest( ULONG ulItemID, HWND hwndRequestor )
{
    SOMAny *somObject = OBJECT_FROM_PREC( ulItemID );

    if( somObject )
    {
        POBJECTDATA pObjData = HeapAlloc( sizeof( OBJECTDATA ) );

        if( pObjData )
        {
            memset( pObjData, 0, sizeof( OBJECTDATA ) );
            strcpy( pObjData->szClassName,
                    _somGetName( _somGetClass( somObject ) ) );
            strcpy( pObjData->szTitle, _wpQueryTitle( somObject ) );
            if( _somRespondsTo( somObject,
                                SOM_IdFromString( "wpQueryProgDetails" ) ) )
            {
                PPROGDETAILS ppd = NULL;
                ULONG        cbProgDetails = 0;

                _wpQueryProgDetails( somObject, ppd, &cbProgDetails );
                ppd = (PPROGDETAILS) SOMMalloc( cbProgDetails );
                if( cbProgDetails && ppd &&
                    _wpQueryProgDetails( somObject, ppd, &cbProgDetails ) )
                {
                    pObjData->fPgmObject = TRUE;
                    if( ppd->pszExecutable )
                        strcpy( pObjData->szExeName,    ppd->pszExecutable );
                    if( ppd->pszParameters )
                        strcpy( pObjData->szParms,      ppd->pszParameters );
                    if( ppd->pszStartupDir )
                        strcpy( pObjData->szStartupDir, ppd->pszStartupDir );
                }
                else
                    pObjData->fPgmObject = FALSE;

                if( ppd )
                    SOMFree( ppd );
            }
            else
                pObjData->fPgmObject = FALSE;
            WinPostMsg( hwndRequestor, UM_DRAGINFO, MPFROMP( pObjData ), NULL );
        }
        else
            Msg( "HeapAlloc failed in DragInfoRequest!\n" );
    }
    else
        Msg( "No somObject supplied!" );
}

/**********************************************************************/
/*------------------------- FormatDragItem ---------------------------*/
/*                                                                    */
/*  A WINDOW HAS REQUESTED THAT WE FORMAT A DRAG ITEM. BASICALLY IT   */
/*  WANTS THE ulItemID FILLED IN.                                     */
/*                                                                    */
/*  PARMS: object window handle,                                      */
/*         pointer to a pre-allocated DRAGITEM structure              */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: an MRESULT (this was a WinSent message                   */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
MRESULT FormatDragItem( HWND hwndObject, PDRAGITEM pDragItem )
{
    PINSTANCE pi = INSTDATA( hwndObject );
    BOOL      fSuccess = TRUE;

    if( pi->somObject )
    {
        if( !_wpFormatDragItem( pi->somObject, pDragItem ) )
        {
            fSuccess = FALSE;
            Msg( "wpFormatDragitem failed!" );
        }
    }
    else
    {
        fSuccess = FALSE;
        Msg( "FormatDragItem somObject not set at this point!" );
    }

    return (MRESULT) fSuccess;
}

/**********************************************************************/
/*-------------------------- SetSomObject ----------------------------*/
/*                                                                    */
/*  SET THE WINDOW WORD SOMOBJECT POINTER FROM AN HOBJECT SENT TO US  */
/*  VIA A PM MESSAGE FROM A PM WINDOW.                                */
/*                                                                    */
/*  PARMS: object window handle,                                      */
/*         HOBJECT of an object created by the PM window.             */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void SetSomObject( HWND hwndObject, HOBJECT hObj )
{
    PINSTANCE    pi = INSTDATA( hwndObject );
    DrgAgentData *somThis;

    pi->somObject = _wpclsQueryObject( pi->somClass, hObj );

    somThis = DrgAgentGetData( pi->somObject );

    _hwndComm = hwndObject;
}

/**********************************************************************/
/*-------------------------- BeenDroppedOn ---------------------------*/
/*                                                                    */
/*  WE'VE BEEN DROPPED ON ANOTHER OBJECT. PROCESS IT.                 */
/*                                                                    */
/*  PARMS: object window handle,                                      */
/*         the somObject pointer that we were dropped on              */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: return message to WPS object                             */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
MRESULT BeenDroppedOn( HWND hwndObject, SOMAny *somObject )
{
    PINSTANCE pi = INSTDATA( hwndObject );
    HOBJECT   hObj = _wpQueryHandle( somObject );

    // We get this from the WPS object and we need to route it to the PM
    // window after translating the somObject pointer to an HOBJECT. PM
    // windows can deal with HOBJECT's but not with somObjects.

    WinSendMsg( pi->hwndDrag, UM_DROP, MPFROMLONG( hObj ), NULL );

    return (MRESULT) TRUE;
}

/**********************************************************************/
/*------------------------------- Msg --------------------------------*/
/*                                                                    */
/*  DISPLAY A MESSAGE TO THE USER IN A PM MESSAGE BOX.                */
/*                                                                    */
/*  PARMS: standard printf parameters                                 */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/

#define MESSAGE_SIZE    1024

void Msg( PSZ szFormat,... )
{
    PSZ     szMsg;
    va_list argptr;

    if( (szMsg = (PSZ) malloc( MESSAGE_SIZE )) == NULL )
    {
        DosBeep( 1000, 1000 );
        return;
    }

    va_start( argptr, szFormat );

    vsprintf( szMsg, szFormat, argptr );

    va_end( argptr );

    szMsg[ MESSAGE_SIZE - 1 ] = 0;

    WinAlarm( HWND_DESKTOP, WA_WARNING );

    WinMessageBox(  HWND_DESKTOP, HWND_DESKTOP, szMsg,
                    "DrgPmWps DragAgent object",
                    1, MB_OK | MB_MOVEABLE );

    free( szMsg );
}

/*************************************************************************
 *                     E N D     O F     S O U R C E                     *
 *************************************************************************/
