/*********************************************************************
 *                                                                   *
 * MODULE NAME :  heap.c                 AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  08-01-93                                           *
 *                                                                   *
 * DESCRIPTION:                                                      *
 *                                                                   *
 *   This module is part of the DRGPMWPS sample program. It is used  *
 *   by both DRGPMWPS.EXE and DRGAGENT.DLL. It allocates and controls*
 *   a shared memory heap that is common to both modules.            *
 *                                                                   *
 * CALLABLE FUNCTIONS:                                               *
 *                                                                   *
 *   HeapCreate                                                      *
 *   HeapAlloc                                                       *
 *   HeapFree                                                        *
 *   HeapDestroy                                                     *
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

#define  INCL_DOSERRORS
#define  INCL_DOSPROCESS
#define  INCL_DOSMEMMGR

/**********************************************************************/
/*----------------------------- INCLUDES -----------------------------*/
/**********************************************************************/

#include <os2.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

/*********************************************************************/
/*------------------- APPLICATION DEFINITIONS -----------------------*/
/*********************************************************************/

#define MEM_NAME  "\\SHAREMEM\\CBLAZERS\\DRGPMWPS\\HEAP.MEM"

#define HEAP_SIZE 0xC000

/**********************************************************************/
/*---------------------------- STRUCTURES ----------------------------*/
/**********************************************************************/


/**********************************************************************/
/*----------------------- FUNCTION PROTOTYPES ------------------------*/
/**********************************************************************/


/**********************************************************************/
/*------------------------ GLOBAL VARIABLES --------------------------*/
/**********************************************************************/

PBYTE pHeap;

/**********************************************************************/
/*--------------------------- HeapCreate -----------------------------*/
/*                                                                    */
/*  CREATE THE SHARED HEAP USED BETWEEN THE PM AND WPS WINDOWS.       */
/*                                                                    */
/*  PARMS: nothing                                                    */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void HeapCreate()
{
    APIRET rc;
    BOOL   fAlloced = TRUE;

    rc = DosGetNamedSharedMem( (PPVOID) &pHeap, MEM_NAME,
                               PAG_READ | PAG_WRITE );
    if( rc )
    {
        if( rc == ERROR_FILE_NOT_FOUND )
        {
            rc = DosAllocSharedMem( (PPVOID) &pHeap, MEM_NAME, HEAP_SIZE,
                                    PAG_READ | PAG_WRITE );
            if( rc )
                Msg( "HeapCreate DosAllocSharedMem failed! rc:%u", rc );
        }
        else
            Msg( "HeapCreate DosGetNamedSharedMem failed! rc:%u", rc );
    }
    else
        fAlloced = FALSE;

    if( !rc )
    {
        ULONG fl = DOSSUB_SPARSE_OBJ | DOSSUB_SERIALIZE;

        if( fAlloced )
            fl |= DOSSUB_INIT;

        rc = DosSubSetMem( pHeap, fl, HEAP_SIZE );
        if( rc )
            Msg( "DosSubSetMem failed! rc:%u", rc );
    }
}

/**********************************************************************/
/*---------------------------- HeapAlloc -----------------------------*/
/*                                                                    */
/*  ALLOCATE A BLOCK FROM THE SHARED HEAP.                            */
/*                                                                    */
/*  PARMS: size of block to allocate                                  */
/*                                                                    */
/*  NOTES: Since DosSubFreeMem needs the size of the block to be      */
/*         freed, we add that info to the beginning of the allocated  */
/*         block and return a pointer past that memory, effectively   */
/*         hiding that data.                                          */
/*                                                                    */
/*  RETURNS: pointer to block or NULL if unsuccessful                 */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void *HeapAlloc( ULONG cb )
{
    APIRET rc = ERROR_NOACCESS;
    ULONG  cbExtra = sizeof cb;
    ULONG  cbBlock = cb + cbExtra;
    PVOID  pBlock = NULL;

    if( pHeap )
    {
        rc = DosSubAllocMem( pHeap, &pBlock, cbBlock );
        if( !rc )
        {
            (void) memset( pBlock, 0, (size_t) cbBlock );

            // Add info used by Free

            *((PULONG) pBlock) = cb;
        }
        else
            Msg( "HeapAlloc DosSubAllocMem rc(%u)", rc );
    }
    else
        Msg( "HeapAlloc NO HEAP!" );

    return( rc ? NULLHANDLE : ((PBYTE)pBlock + cbExtra) );
}

/**********************************************************************/
/*----------------------------- HeapFree -----------------------------*/
/*                                                                    */
/*  FREE A BLOCK FROM THE SHARED HEAP.                                */
/*                                                                    */
/*  PARMS: pointer to block to be freed                               */
/*                                                                    */
/*  NOTES: See the notes under HeapAlloc for an explanation about the */
/*         'hidden' memory.                                           */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void HeapFree( void *pBlock )
{
    ULONG  cbRegion = 1, flAlloc, cbBlock;
    ULONG  cbExtra = sizeof cbBlock;
    APIRET rc;

    pBlock = (PBYTE) pBlock - cbExtra;

    // Make sure it is valid before we start accessing it. We don't want no
    // traps!

    rc = DosQueryMem( pBlock, &cbRegion, &flAlloc );
    if( !rc )
    {
        cbBlock = *((PULONG) pBlock);
        cbBlock += cbExtra;

        rc = DosSubFreeMem( pHeap, pBlock, cbBlock );
        if( rc )
            Msg( "HeapFree DosSubFreeMem rc(%u)", rc );
    }
    else
        Msg( "HeapFree DosQueryMem rc(%u)", rc );
}

/**********************************************************************/
/*--------------------------- HeapDestroy ----------------------------*/
/*                                                                    */
/*  DESTROY THE SHARED HEAP.                                          */
/*                                                                    */
/*  PARMS: nothing                                                    */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void HeapDestroy()
{
    if( pHeap )
    {
        DosSubUnsetMem( pHeap );
        DosFreeMem( pHeap );
    }
}

/*************************************************************************
 *                     E N D     O F     S O U R C E                     *
 *************************************************************************/
