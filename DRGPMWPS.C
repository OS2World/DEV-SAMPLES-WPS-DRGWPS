/*********************************************************************
 *                                                                   *
 * MODULE NAME :  drgpmwps.c             AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  08-01-93                                           *
 *                                                                   *
 * HOW TO RUN THIS PROGRAM:                                          *
 *                                                                   *
 *  Just enter DRGPMWPS on the command line.                         *
 *                                                                   *
 * MODULE DESCRIPTION:                                               *
 *                                                                   *
 *  Root module for DRGPMWPS.EXE, a program that demonstrates using  *
 *  Drag/Drop on OS/2 2.x between a normal PM program and a WPS      *
 *  object. This program creates a container window that is used to  *
 *  drag from and drop on.                                           *
 *                                                                   *
 *  The drag window starts out with some icons in it that can be     *
 *  dragged. If you drag from this window, the icon can be dropped   *
 *  on a WPS object. If it is dropped, the WPS object will retain    *
 *  that icon. Be careful - it will really stick, so if you do this  *
 *  you've got to be prepared to change it back.                     *
 *                                                                   *
 *  You can also drop WPS objects on the window. If you do, a        *
 *  messagebox is displayed that gives you a minimal amount of       *
 *  information about the object, just to let you know that the      *
 *  interaction between the PM window and the WPS object was         *
 *  successful.                                                      *
 *                                                                   *
 *  The WPS object class is registered in this dll and an instance   *
 *  is created. That instance is also destroyed and the class        *
 *  deregistered when the PM window is destroyed, so all cleanup     *
 *  is done automatically. This program first tries to load the      *
 *  WPS object's dll from the directory where this program is        *
 *  loaded from. If it doesn't find it there, LIBPATH is used.       *
 *                                                                   *
 *  The WPS object is created visible so you can see that it's there.*
 *  If this were a real application, you would use the               *
 *  NOTVISIBLE=YES string on the WinCreateObject because the user    *
 *  has no idea about the 'agent' object and wouldn't want that icon *
 *  on their desktop.                                                *
 *                                                                   *
 *  The icons in the container are found in the current directory.   *
 *  That means that you can run this program from a directory with   *
 *  a lot of icons and really have some fun. It is actually a        *
 *  convenient way to assign icons to program objects. With a little *
 *  work this program can be a nice little desktop icon manager.     *
 *                                                                   *
 * OTHER MODULES:                                                    *
 *                                                                   *
 *  drag.c     - contains all drag/drop processing code.             *
 *  heap.c     - contains the code to create/maintain a shared memory*
 *               heap.                                               *
 *  commdata.c - contains the code to create/maintain a shared memory*
 *               segment with information common to this program and *
 *               the WPS object class.                               *
 *  drgagent.c - the module that contains the code for the WPS       *
 *               object.                                             *
 *  commwin.c  - contains the code for the WPS object's object       *
 *               window.                                             *
 *                                                                   *
 * NOTES:                                                            *
 *                                                                   *
 *  This sample was coded fairly quickly so there are bound to be    *
 *  some anomalies.                                                  *
 *                                                                   *
 *  I hope this code proves useful for other PM programmers. The     *
 *  more of us the better!                                           *
 *                                                                   *
 * HISTORY:                                                          *
 *                                                                   *
 *  08-01-93 - Program coding started.                               *
 *                                                                   *
 *  Rick Fishman                                                     *
 *  Code Blazers, Inc.                                               *
 *  4113 Apricot                                                     *
 *  Irvine, CA. 92720                                                *
 *  CIS ID: 72251,750                                                *
 *                                                                   *
 *********************************************************************/

#pragma strings(readonly)   // used for debug version of memory mgmt routines

/*********************************************************************/
/*------- Include relevant sections of the OS/2 header files --------*/
/*********************************************************************/

#define  INCL_DOSMODULEMGR
#define  INCL_DOSPROCESS
#define  INCL_SHLERRORS
#define  INCL_WINERRORS
#define  INCL_WINDIALOGS
#define  INCL_WINFRAMEMGR
#define  INCL_WINPOINTERS
#define  INCL_WINSHELLDATA
#define  INCL_WINSTDCNR
#define  INCL_WINSTDDRAG
#define  INCL_WINSYS
#define  INCL_WINTIMER
#define  INCL_WINWINDOWMGR

#define  GLOBALS_DEFINED    // extern globals instantiated

/**********************************************************************/
/*----------------------------- INCLUDES -----------------------------*/
/**********************************************************************/

#include <os2.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drgpmwps.h"
#include "common.h"

/*********************************************************************/
/*------------------- APPLICATION DEFINITIONS -----------------------*/
/*********************************************************************/

#define PROGRAM_TITLE       "PM/WPS Drag/Drop Sample"

#define MESSAGE_SIZE        1024

#define FRAME_FLAGS         (FCF_TASKLIST   | FCF_TITLEBAR   | FCF_SYSMENU | \
                             FCF_MINMAX     | FCF_SIZEBORDER | FCF_ICON    | \
                             FCF_ACCELTABLE)

#define CONTAINER_STYLES    (CCS_SINGLESEL | CCS_MINIRECORDCORE | \
                             CCS_AUTOPOSITION)

#define TIMER_INTERVAL      1000
#define MAX_TIMER_ITERS     10

/**********************************************************************/
/*---------------------------- STRUCTURES ----------------------------*/
/**********************************************************************/


/**********************************************************************/
/*----------------------- FUNCTION PROTOTYPES ------------------------*/
/**********************************************************************/

int  main             ( void );
void GetCurrentPath   ( void );
HWND CreateWindow     ( HAB hab );
BOOL InsertRecords    ( HWND hwndCnr );
void SizeAndShowWindow( HWND hwndFrame );
void CreateDrgAgent   ( void );
void DestroyDrgAgent  ( void );

FNWP wpFrame;

/**********************************************************************/
/*------------------------ GLOBAL VARIABLES --------------------------*/
/**********************************************************************/

PFNWP pfnwpFrame;

char szCnrTitle[] = "Drag to a WPS object\n"
                    "or drop a WPS object here";

HAB  hab;

HOBJECT hObjAgent;

int cTimerIters;

/**********************************************************************/
/*------------------------------ MAIN --------------------------------*/
/*                                                                    */
/*  PROGRAM ENTRYPOINT                                                */
/*                                                                    */
/*  PARMS: nothing                                                    */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: return code                                              */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
int main( void )
{
    HMQ  hmq = NULLHANDLE;
    QMSG qmsg;
    HWND hwndFrame = NULLHANDLE;

    // This macro is defined for the debug version of the C Set/2 Memory
    // Management routines. Since the debug version writes to stderr, we
    // send all stderr output to a debuginfo file.

#ifdef __DEBUG_ALLOC__
    freopen( DEBUG_FILENAME, "w", stderr );
#endif

    hab = WinInitialize( 0 );

    if( hab )
        hmq = WinCreateMsgQueue( hab, 0 );
    else
    {
        DosBeep( 1000, 100 );
        fprintf( stderr, "WinInitialize failed!" );
    }

    if( hmq )
    {
        GetCurrentPath();

        CreateDrgAgent();

        hwndFrame = CreateWindow( hab );

        if( hwndFrame )
            while( WinGetMsg( hab, &qmsg, NULLHANDLE, 0, 0 ) )
                WinDispatchMsg( hab, &qmsg );
    }
    else if( hab )
        Msg( "WinCreateMsgQueue RC(%X)", HABERR( hab ) );

    if( hwndFrame )
        WinDestroyWindow( hwndFrame );

    DestroyDrgAgent();

    if( hmq )
        WinDestroyMsgQueue( hmq );

    if( hab )
        WinTerminate( hab );

#ifdef __DEBUG_ALLOC__
    _dump_allocated( -1 );
#endif

    return 0;
}

/**********************************************************************/
/*------------------------- GetCurrentPath ---------------------------*/
/*                                                                    */
/*  STORE THE CURRENT DRIVE/DIRECTORY.                                */
/*                                                                    */
/*  PARMS: nothing                                                    */
/*                                                                    */
/*  NOTES: This stores the current drive:\directory\  that is used    */
/*         to create temporary files in.                              */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void GetCurrentPath()
{
    PBYTE  pbCurrent = szCurrentPath;
    INT    cbBuf = sizeof szCurrentPath, cbUsed;
    ULONG  ulDrive, ulCurrDriveNo, ulDriveMap, cbPath;
    APIRET rc;

    // Fill in the drive letter, colon, and backslash

    rc = DosQueryCurrentDisk( &ulCurrDriveNo, &ulDriveMap );

    if( !rc )                                // Use 'current' drive
    {
        *(pbCurrent++) = (BYTE) (ulCurrDriveNo + ('A' - 1));
        *(pbCurrent++) = ':';
        *(pbCurrent++) = '\\';
    }
    else
    {                                        // API failed - use drive C:
        strcpy( pbCurrent, "C:\\" );
        pbCurrent += 3;                      // Incr our place in the buffer
    }

    cbUsed = pbCurrent - szCurrentPath;      // How many bytes left?

    // Fill in the current directory

    ulDrive = *szCurrentPath - 'A' + 1;      // Get drive number from letter
    cbPath = cbBuf - cbUsed;                 // How many bytes left?

    rc = DosQueryCurrentDir( ulDrive, pbCurrent, &cbPath );
                                             // Get 'current' directory
    if( szCurrentPath[ strlen( szCurrentPath ) - 1 ] != '\\' )
        strcat( szCurrentPath, "\\" );       // Add trailing backslash
}

/**********************************************************************/
/*-------------------------- CreateWindow ----------------------------*/
/*                                                                    */
/*  CREATE THE APPLICATION WINDOW                                     */
/*                                                                    */
/*  PARMS: anchor block handle                                        */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: frame window handle                                      */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
HWND CreateWindow( HAB hab )
{
    FRAMECDATA fcdata;
    HWND       hwndFrame = NULLHANDLE, hwndCnr = NULLHANDLE;

    memset( &fcdata, 0, sizeof fcdata );
    fcdata.cb            = sizeof( FRAMECDATA );
    fcdata.idResources   = ID_RESOURCES;
    fcdata.flCreateFlags = FRAME_FLAGS;

    // Create the container as the client window of the frame. There is no
    // need for a normal client window. Because of this we must subclass the
    // frame window so we can catch the messages that the container sends to
    // its owner.

    hwndFrame = WinCreateWindow( HWND_DESKTOP, WC_FRAME, NULL,
                                 FS_NOBYTEALIGN | WS_CLIPCHILDREN,
                                 0, 0, 0, 0, NULLHANDLE, HWND_TOP,
                                 1, &fcdata, NULL );
    if( hwndFrame )
    {
        pfnwpFrame = WinSubclassWindow( hwndFrame, wpFrame );

        if( pfnwpFrame )
        {
            hwndCnr = WinCreateWindow( hwndFrame, WC_CONTAINER, NULL,
                                       WS_VISIBLE | CONTAINER_STYLES,
                                       0, 0, 0, 0, hwndFrame, HWND_TOP,
                                       FID_CLIENT, NULL, NULL );
            if( hwndCnr )
                WinSetWindowText( hwndFrame, PROGRAM_TITLE );
            else
            {
                WinDestroyWindow( hwndFrame );
                hwndFrame = NULLHANDLE;
                Msg( "WinCreateWindow(hwndCnr) RC(%X)", HABERR( hab ) );
            }
        }
        else
        {
            WinDestroyWindow( hwndFrame );
            hwndFrame = NULLHANDLE;
            Msg( "WinSubclassWindow RC(%X)", HABERR( hab ) );
        }
    }
    else
        Msg( "WinCreateWindow(hwndFrame) RC(%X)", HABERR( hab ) );

    if( hwndFrame )
    {
        // Create the shared memory heap

        HeapCreate();

        // Create the shared memory segment that contains information shared
        // by the PM window and the WPS object.

        CommdataCreate();

        // Start a time that is used to indicate if there is a problem with the
        // creation of the WPS object's object window. In order to communicate
        // with the WPS object, the WPS object must have created an object
        // window and placed its handle in the shared segment. If we time out
        // this program will be ended after an error message is displayed.

        if( hObjAgent )
            WinStartTimer( hab, hwndFrame, 1, TIMER_INTERVAL );
    }

    if( hwndFrame )
    {
        CNRINFO cnri;

        // Set container into Icon view and give it a read-only title

        cnri.cb           = sizeof( CNRINFO );
        cnri.pszCnrTitle  = szCnrTitle;
        cnri.flWindowAttr = CV_ICON | CA_CONTAINERTITLE | CA_TITLESEPARATOR |
                            CA_TITLEREADONLY;

        if( !WinSendMsg( hwndCnr, CM_SETCNRINFO, MPFROMP( &cnri ),
                         MPFROMLONG( CMA_FLWINDOWATTR | CMA_CNRTITLE ) ) )
        {
            WinDestroyWindow( hwndFrame );
            hwndFrame = NULLHANDLE;
            Msg( "CM_SETCNRINFO RC(%X)", HABERR( hab ) );
        }
    }

    if( hwndFrame )
    {
        SizeAndShowWindow( hwndFrame );
        if( !InsertRecords( hwndCnr ) )
        {
            WinDestroyWindow( hwndFrame );
            hwndFrame = NULLHANDLE;
        }
    }

    return hwndFrame;
}

/**********************************************************************/
/*-------------------------- InsertRecords ---------------------------*/
/*                                                                    */
/*  INSERT RECORDS INTO A CONTAINER                                   */
/*                                                                    */
/*  PARMS: container window handle                                    */
/*                                                                    */
/*  NOTES: We insert a record for every .ico file we find in the      */
/*         current directory. If we find no .ico files we put up an   */
/*         error message.                                             */
/*                                                                    */
/*  RETURNS: TRUE if successful, FALSE if not                         */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
BOOL InsertRecords( HWND hwndCnr )
{
    BOOL         fSuccess = TRUE;
    RECORDINSERT ri;
    PCNRREC      pCnrRec;
    FILEFINDBUF3 ffb;
    HDIR         hdir = HDIR_SYSTEM;
    ULONG        cFiles = 1, cIconsFound = 0;
    APIRET       rc;

    memset( &ri, 0, sizeof( RECORDINSERT ) );
    ri.cb                 = sizeof( RECORDINSERT );
    ri.pRecordOrder       = (PRECORDCORE) CMA_END;
    ri.pRecordParent      = (PRECORDCORE) NULL;
    ri.zOrder             = (USHORT) CMA_TOP;
    ri.cRecordsInsert     = 1;
    ri.fInvalidateRecord  = TRUE;

    rc = DosFindFirst( "*.ico", &hdir, FILE_NORMAL,
                       &ffb, sizeof ffb, &cFiles, FIL_STANDARD );
    while( !rc )
    {
        cIconsFound++;

        pCnrRec = WinSendMsg( hwndCnr, CM_ALLOCRECORD, MPFROMLONG(EXTRA_BYTES),
                              MPFROMLONG( 1 ) );

        if( pCnrRec )
        {
            strcpy( pCnrRec->szFileName, ffb.achName );

            pCnrRec->mrc.pszIcon  = (PSZ) &pCnrRec->szFileName;
            pCnrRec->mrc.hptrIcon = WinLoadFileIcon( ffb.achName, FALSE );
            if( !pCnrRec->mrc.hptrIcon )
                pCnrRec->mrc.hptrIcon =
                    WinQuerySysPointer( HWND_DESKTOP, SPTR_QUESICON, FALSE );

            if( !WinSendMsg( hwndCnr, CM_INSERTRECORD, MPFROMP( pCnrRec ),
                             MPFROMP( &ri ) ) )
            {
                fSuccess = FALSE;
                Msg( "InsertRecords CM_INSERTRECORD RC(%X)", HWNDERR(hwndCnr) );
            }
        }

        rc = DosFindNext( hdir, &ffb, sizeof ffb, &cFiles );
    }

    if( !cIconsFound )
    {
        Msg( "No icons found in this directory. Please copy at least one "
             "icon into this directory or start this program from a "
             "directory with at least one icon in it" );

        fSuccess = FALSE;
    }

    return fSuccess;
}

/**********************************************************************/
/*------------------------ SizeAndShowWindow -------------------------*/
/*                                                                    */
/*  SIZE AND SHOW THE FRAME WINDOW.                                   */
/*                                                                    */
/*  PARMS: frame window handle                                        */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void SizeAndShowWindow( HWND hwndFrame )
{
    LONG cxDesktop = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
    LONG cyDesktop = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );

    // Place the window in the center of the screen and make it the size
    // of half the width and half the height.

    WinSetWindowPos( hwndFrame, HWND_TOP,
                     cxDesktop / 4, cyDesktop / 4,
                     cxDesktop / 2, cyDesktop / 2,
                     SWP_MOVE | SWP_SIZE | SWP_SHOW | SWP_ACTIVATE |
                     SWP_ZORDER );

    // The container was set up as the client window of the frame. We
    // need to set focus to it - otherwise it will not accept keystrokes
    // right away.

    WinSetFocus( HWND_DESKTOP, WinWindowFromID( hwndFrame, FID_CLIENT ) );
}

/**********************************************************************/
/*----------------------------- wpFrame ------------------------------*/
/*                                                                    */
/*  SUBCLASSED FRAME WINDOW PROCEDURE.                                */
/*                                                                    */
/*  PARMS: normal winproc parms                                       */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: MRESULT value                                            */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
MRESULT EXPENTRY wpFrame( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    switch( msg )
    {
        case WM_TIMER:

            // If we time out, that means something went wrong on the WPS
            // object's end.

            if( pCommData && pCommData->hwndComm )
            {
                WinStopTimer( hab, hwnd, 1 );
                WinPostMsg( pCommData->hwndComm, UM_SET_HOBJECT,
                            MPFROMLONG( hObjAgent ), NULL );
            }
            else if( ++cTimerIters > MAX_TIMER_ITERS )
            {
                WinStopTimer( hab, hwnd, 1 );
                Msg( "Drag Agent not running! We can't do anything" );
            }

            return 0;

        case UM_DRAGINFO:
        {
            POBJECTDATA pObjData = (POBJECTDATA) mp1;

            // The WPS object has passed us information about an object that
            // was dropped on us. Display the information then free the shared
            // memory object.

            if( pObjData->fPgmObject )
                Msg( "*** DROPPED ITEM INFO ***\n"
                     "Object Name: %s\n"
                     "Object Class Name: %s\n"
                     "Exe Name: %s\n"
                     "Program Parameters: %s\n"
                     "Program's Startup Directory: %s\n",
                     pObjData->szTitle,
                     pObjData->szClassName,
                     pObjData->szExeName,
                     pObjData->szParms,
                     pObjData->szStartupDir );
            else
                Msg( "*** DROPPED ITEM INFO ***\n"
                     "Object Name: %s\n"
                     "Object Class Name: %s\n",
                     pObjData->szTitle,
                     pObjData->szClassName );
            HeapFree( pObjData );
            break;
        }

        case UM_DROP:
        {
            char szIconInfo[ CCHMAXPATH ];

            // The WPS object has passed us the HOBJECT of a WPS object that
            // one of our icons was dropped on. Set that object's icon to the
            // icon that was dropped on it.

            sprintf( szIconInfo, "%s%s%s", "ICONFILE=", szCurrentPath,
                     pCnrRecBeingDragged->szFileName );

            if( !WinSetObjectData( (HOBJECT) mp1, szIconInfo ) )
                Msg( "WinsSetObjectData failed!" );

            return 0;
        }

        case WM_DESTROY:

            // Free all container resources associated with any records that
            // were inserted (note that the container is actually the client
            // window.

            WinSendDlgItemMsg( hwnd, FID_CLIENT, CM_REMOVERECORD, NULL,
                               MPFROM2SHORT( 0, CMA_FREE ) );

            // Destroy the shared memory that holds the data common to both
            // the PM window and the WPS object.

            CommdataDestroy();

            // Free the shared memory heap.

            HeapDestroy();
            break;

        case WM_COMMAND:

            // Process the accelerator key

            if( SHORT1FROMMP( mp1 ) == IDM_EXIT )
            {
                WinPostMsg( hwnd, WM_SYSCOMMAND, MPFROMSHORT( SC_CLOSE ),
                            MPFROM2SHORT( CMDSRC_ACCELERATOR, FALSE ) );
                return 0;
            }

            break;

        case WM_CONTROL:
            if( SHORT1FROMMP( mp1 ) == FID_CLIENT )
                switch( SHORT2FROMMP( mp1 ) )
                {
                    case CN_INITDRAG:
                        dragInit( hwnd, (PCNRDRAGINIT) mp2 );
                        return 0;

                    case CN_DRAGOVER:
                        return dragOver( hwnd, (PCNRDRAGINFO) mp2 );

                    case CN_DROP:
                        dragDrop( hwnd, (PCNRDRAGINFO) mp2 );
                        return 0;
                }

            break;
    }

    return pfnwpFrame( hwnd, msg, mp1, mp2 );
}

/**********************************************************************/
/*-------------------------- CreateDrgAgent --------------------------*/
/*                                                                    */
/*  CREATE THE WPS AGENT OBJECT                                       */
/*                                                                    */
/*  PARMS: nothing                                                    */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void CreateDrgAgent()
{
    char szObjectId[ 100 ];

    if( !WinQueryObject( AGENT_OBJECT_ID ) )
    {
        char    szFullDllPath[ CCHMAXPATH ];
        PCH     pchBackslash;
        PTIB    ptib;
        PPIB    ppib;

        // Load the WPS object's dll from the same directory that the .exe
        // was loaded from. This allows you to start drgpmwps from a directory
        // with a lot of icons and not worry about where the WPS object's dll
        // is.

        DosGetInfoBlocks( &ptib, &ppib );
        DosQueryModuleName( ppib->pib_hmte, sizeof szFullDllPath,szFullDllPath);
        pchBackslash = strrchr( szFullDllPath, '\\' );
        if( pchBackslash )
        {
            strcpy( pchBackslash + 1, AGENT_DLL_NAME );
            strcat( szFullDllPath, ".dll" );
        }
        else
            strcpy( szFullDllPath, AGENT_DLL_NAME );

        // If the WinRegisterObjectClass fails, chances are you moved the DLL
        // to the LIBPATH because you didn't trust my program <g>. In that
        // case we use the non-fully-qualified dllname and try again.

        if( !WinRegisterObjectClass( AGENT_CLASS_NAME, szFullDllPath ) )
            if( !WinRegisterObjectClass( AGENT_CLASS_NAME, AGENT_DLL_NAME ) )
                Msg( "\nWinRegisterObjectClasst rc(%X)", HABERR( hab ) );

        // Create the WPS object with the given object id. If this were a real
        // program you would want to also create it with NOTVISIBLE=YES so the
        // user doesn't know about the agent.

        sprintf( szObjectId, "OBJECTID=%s", AGENT_OBJECT_ID );

        hObjAgent = WinCreateObject( AGENT_CLASS_NAME, "ÿDrag Agent",
                                     szObjectId, "<WP_DESKTOP>",
                                     CO_FAILIFEXISTS );
        if( !hObjAgent )
            Msg( "\nWinCreateObject rc(%X)", HABERR( hab ) );
    }
}

/**********************************************************************/
/*------------------------- DestroyDrgAgent --------------------------*/
/*                                                                    */
/*  DESTROY THE WPS AGENT OBJECT AND DEREGISTER ITS CLASS             */
/*                                                                    */
/*  PARMS: nothing                                                    */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void DestroyDrgAgent()
{
    HOBJECT hObj = WinQueryObject( AGENT_OBJECT_ID );

    if( hObj )
        WinDestroyObject( hObj );
    else
        Msg( "Coudn't locate agent object to destroy it" );

    WinDeregisterObjectClass( AGENT_CLASS_NAME );
}

/**********************************************************************/
/*------------------------------- Msg --------------------------------*/
/*                                                                    */
/*  DISPLAY A MESSAGE TO THE USER.                                    */
/*                                                                    */
/*  PARMS: a message in printf format with its parms                  */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void Msg( PSZ szFormat,... )
{
    PSZ     szMsg;
    va_list argptr;

    szMsg = (PSZ) malloc( MESSAGE_SIZE );
    if( szMsg )
    {
        va_start( argptr, szFormat );
        vsprintf( szMsg, szFormat, argptr );
        va_end( argptr );

        szMsg[ MESSAGE_SIZE - 1 ] = 0;

        WinAlarm( HWND_DESKTOP, WA_WARNING );
        WinMessageBox(  HWND_DESKTOP, HWND_DESKTOP, szMsg,
                        "PM/WPS Drag/Drop Sample Program", 1,
                        MB_OK | MB_MOVEABLE );
        free( szMsg );
    }
    else
    {
        DosBeep( 1000, 1000 );
        return;
    }
}

/*************************************************************************
 *                     E N D     O F     S O U R C E                     *
 *************************************************************************/
