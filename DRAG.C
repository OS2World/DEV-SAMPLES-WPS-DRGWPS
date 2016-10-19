/*********************************************************************
 *                                                                   *
 * MODULE NAME :  drag.c                 AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  08-01-93                                           *
 *                                                                   *
 * MODULE DESCRIPTION:                                               *
 *                                                                   *
 *  Part of the 'DRGPMWPS' drag/drop sample program.                 *
 *                                                                   *
 *  This module handles all the Drag/Drop processing for the         *
 *  DRGPMWPS.EXE sample program.                                     *
 *                                                                   *
 * NOTES:                                                            *
 *                                                                   *
 *                                                                   *
 * FUNCTIONS AVALABLE TO OTHER MODULES:                              *
 *                                                                   *
 *   dragInit                                                        *
 *   dragOver                                                        *
 *   dragDrop                                                        *
 *                                                                   *
 *                                                                   *
 * HISTORY:                                                          *
 *                                                                   *
 *  08-01-93 - Program coded.                                        *
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

#define  INCL_WINDIALOGS
#define  INCL_WINERRORS
#define  INCL_WINFRAMEMGR
#define  INCL_WININPUT
#define  INCL_WINPOINTERS
#define  INCL_WINSTDCNR
#define  INCL_WINSTDDRAG
#define  INCL_WINWINDOWMGR

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


/**********************************************************************/
/*---------------------------- STRUCTURES ----------------------------*/
/**********************************************************************/


/**********************************************************************/
/*----------------------- FUNCTION PROTOTYPES ------------------------*/
/**********************************************************************/

BOOL SetDragItem( HWND hwndFrame, PCNRREC pCnrRec, PDRAGINFO pDragInfo,
                  PDRAGIMAGE pDragImage );

/**********************************************************************/
/*------------------------ GLOBAL VARIABLES --------------------------*/
/**********************************************************************/


/**********************************************************************/
/*----------------------------- dragInit -----------------------------*/
/*                                                                    */
/*  PROCESS CN_INITDRAG NOTIFY MESSAGE.                               */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         pointer to the CNRDRAGINIT structure                       */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void dragInit( HWND hwndFrame, PCNRDRAGINIT pcdi )
{
    PCNRREC    pCnrRec   = (PCNRREC) pcdi->pRecord;
    DRAGIMAGE  DragImage;
    PDRAGINFO  pDragInfo = NULL;

    if( pCnrRec )
    {
        pDragInfo = DrgAllocDraginfo( 1 );

        if( pDragInfo )
        {
            // It is necessary to set this to DO_DEFAULT so that the WPS
            // methods wpDraggedOverObject and wpDroppedOnObject work.

            pDragInfo->usOperation = DO_DEFAULT;

            memset( &DragImage, 0, sizeof DragImage );

            if( SetDragItem( hwndFrame, pCnrRec, pDragInfo, &DragImage ) )
            {
                // Give the WPS object's object window our window handle so if
                // something is dropped on a WPS object it can let us know
                // about it.

                if( pCommData && pCommData->hwndComm )
                    WinSendMsg( pCommData->hwndComm, UM_STARTING_DRAG,
                                MPFROMHWND( hwndFrame ), NULL );


                // Turn source emphasis on for this container record.
                if( !WinSendDlgItemMsg( hwndFrame, FID_CLIENT,
                                        CM_SETRECORDEMPHASIS,
                                        MPFROMP( pCnrRec ),
                                        MPFROM2SHORT( TRUE, CRA_SOURCE ) ) )
                    Msg( "CM_SETRECORDEMPHASIS failed! RC = %X",
                         HWNDERR( hwndFrame ) );

                if( !DrgDrag( hwndFrame, pDragInfo, &DragImage, 1, VK_ENDDRAG,
                              NULL ) )
                {
                    DrgDeleteDraginfoStrHandles( pDragInfo );
                    DrgFreeDraginfo( pDragInfo );
                }

                // Remove the container record's source emphasis
                WinSendDlgItemMsg( hwndFrame, FID_CLIENT, CM_SETRECORDEMPHASIS,
                                   MPFROMP( pCnrRec ),
                                   MPFROM2SHORT( FALSE, CRA_SOURCE ) );
            }
        }
        else
            Msg( "DrgAllocDraginfo failed. RC: %X", HWNDERR( hwndFrame ) );
    }
}

/**********************************************************************/
/*--------------------------- SetDragItem ----------------------------*/
/*                                                                    */
/*  SET THE DRAGITEM STRUCT INTO A DRAGINFO STRUCT.                   */
/*                                                                    */
/*  PARMS: frame window handlerecord,                                 */
/*         pointer to container record,                               */
/*         pointer to allocated DRAGINFO struct,                      */
/*         pointer to DRAGIMAGE struct                                */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: TRUE or FALSE if successful or not                       */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
BOOL SetDragItem( HWND hwndFrame, PCNRREC pCnrRec, PDRAGINFO pDragInfo,
                  PDRAGIMAGE pDragImage )
{
    BOOL      fSuccess = TRUE;
    PDRAGITEM pDragItem = HeapAlloc( sizeof( DRAGITEM ) );

    if( pDragItem )
    {
        (void) memset( pDragItem, 0, sizeof( DRAGITEM ) );

        // Let the WPS object format the DragItem. Only it knows the proper
        // format of the DRAGITEM so that we will be allowed to be dropped on
        // WPS objects.

        if( WinSendMsg( pCommData->hwndComm, UM_FORMAT_DRAGITEM,
                        MPFROMP( pDragItem ), NULL ) )
        {
            // The source name and target name are not used so get rid of them.

            DrgDeleteStrHandle( pDragItem->hstrSourceName );
            DrgDeleteStrHandle( pDragItem->hstrTargetName );
            pDragItem->hstrSourceName = NULLHANDLE;
            pDragItem->hstrTargetName = NULLHANDLE;

            // This is the window handle that DM_ messages will be sent to.

            pDragItem->hwndItem       = hwndFrame;

            DrgSetDragitem( pDragInfo, pDragItem, sizeof( DRAGITEM ), 0 );

            // Free the shared memory since it is no longer needed.

            HeapFree( pDragItem );

            pDragImage->cb       = sizeof( DRAGIMAGE );
            pDragImage->hImage   = pCnrRec->mrc.hptrIcon;
            pDragImage->fl       = DRG_ICON;

            // Save the container record that is being dragged so we have
            // access to it on the drop.

            pCnrRecBeingDragged  = pCnrRec;
        }
        else
        {
            fSuccess = FALSE;
            Msg( "UM_FORMAT_DRAGITEM failed!\n" );
        }
    }
    else
    {
        fSuccess = FALSE;
        Msg( "HeapAlloc failed in SetDragItem!\n" );
    }

    return fSuccess;
}

/**********************************************************************/
/*----------------------------- dragOver -----------------------------*/
/*                                                                    */
/*  PROCESS CN_DRAGOVER NOTIFY MESSAGE.                               */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         pointer to the CNRDRAGINFO structure                       */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: return value from CN_DRAGOVER processing                 */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
MRESULT dragOver( HWND hwndFrame, PCNRDRAGINFO pcdi )
{
    USHORT    usDrop = DOR_NEVERDROP, usDefOp = 0;
    PDRAGINFO pDragInfo = pcdi->pDragInfo;
    PDRAGITEM pDragItem;

    if( DrgAccessDraginfo( pDragInfo ) )
    {
        // Don't let us drop on ourself in this simple test program. Normally
        // this would be allowed so the user could move the icon in the
        // container. Not necessary here.

        if( pDragInfo->hwndSource != hwndFrame )
        {
            pDragItem = DrgQueryDragitemPtr( pDragInfo, 0 );

            // The only items we will let be dropped on us are WPS objects
            // which use the <DRM_OBJECT,DRF_OBJECT> RMF.

            if( DrgVerifyRMF( pDragItem, "DRM_OBJECT", "DRF_OBJECT" ) )
            {
                usDrop  = DOR_DROP;
                usDefOp = DO_COPY;
            }
        }

        DrgFreeDraginfo( pDragInfo );
    }

    return MRFROM2SHORT( usDrop, usDefOp );
}

/**********************************************************************/
/*---------------------------- dragDrop ------------------------------*/
/*                                                                    */
/*  PROCESS CN_DROP NOTIFY MESSAGE.                                   */
/*                                                                    */
/*  PARMS: frame window handle,                                       */
/*         pointer to the CNRDRAGINFO structure                       */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void dragDrop( HWND hwndFrame, PCNRDRAGINFO pcdi )
{
    PDRAGINFO pDragInfo = pcdi->pDragInfo;

    if( DrgAccessDraginfo( pDragInfo ) )
    {
        PDRAGITEM pDragItem;
        int       i;

        if( pCommData && pCommData->hwndComm )
            for( i = 0; i < pDragInfo->cditem; i++ )
            {
                pDragItem = DrgQueryDragitemPtr( pDragInfo, i );

                // A WPS object was dropped on us. Request information on this
                // object from our agent object. It will eventually post the
                // frame window a UM_DRAGINFO message with that information.

                WinSendMsg( pCommData->hwndComm, UM_DRAGINFO_REQUEST,
                            MPFROMLONG( pDragItem->ulItemID ),
                            MPFROMHWND( hwndFrame ) );

                // Let em know we're done so it can erase source emphasis

                WinSendMsg( pCommData->hwndComm, UM_DROP_ENDCONVERSATION,
                            MPFROMLONG( pDragItem->ulItemID ), NULL );
            }
        else
            Msg( "Dont have access to the Drag Agent's window handle!" );
    }

    DrgDeleteDraginfoStrHandles( pDragInfo );
    DrgFreeDraginfo( pDragInfo );
}

/*************************************************************************
 *                     E N D     O F     S O U R C E                     *
 *************************************************************************/
