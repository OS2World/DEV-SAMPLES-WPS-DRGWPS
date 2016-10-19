/*********************************************************************
 *                                                                   *
 * MODULE NAME :  commdata.c             AUTHOR:  Rick Fishman       *
 * DATE WRITTEN:  08-01-93                                           *
 *                                                                   *
 * DESCRIPTION:                                                      *
 *                                                                   *
 *   This module is part of the DRGPMWPS sample program. It is used  *
 *   by both DRGPMWPS.EXE and DRGAGENT.DLL. It allocates and controls*
 *   a shared memory segment that is common to both modules.         *
 *                                                                   *
 * CALLABLE FUNCTIONS:                                               *
 *                                                                   *
 *   CommdataCreate                                                  *
 *   CommdataDestroy                                                 *
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

#define GLOBALS_DEFINED

#include <os2.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

/*********************************************************************/
/*------------------- APPLICATION DEFINITIONS -----------------------*/
/*********************************************************************/

#define MEM_NAME  "\\SHAREMEM\\CBLAZERS\\DRGPMWPS\\COMMDATA.MEM"

/**********************************************************************/
/*---------------------------- STRUCTURES ----------------------------*/
/**********************************************************************/

/**********************************************************************/
/*----------------------- FUNCTION PROTOTYPES ------------------------*/
/**********************************************************************/


/**********************************************************************/
/*------------------------ GLOBAL VARIABLES --------------------------*/
/**********************************************************************/


/**********************************************************************/
/*------------------------- CommdataCreate ---------------------------*/
/*                                                                    */
/*  CREATE THE SHARED MEMORY USED TO HOLD COMMON DATA.                */
/*                                                                    */
/*  PARMS: nothing                                                    */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void CommdataCreate()
{
    APIRET rc;

    rc = DosGetNamedSharedMem( (PPVOID) &pCommData, MEM_NAME,
                               PAG_READ | PAG_WRITE );
    if( rc )
    {
        if( rc == ERROR_FILE_NOT_FOUND )
        {
            rc = DosAllocSharedMem( (PPVOID) &pCommData, MEM_NAME,
                                    sizeof( COMMDATA ),
                                    PAG_COMMIT | PAG_READ | PAG_WRITE );
            if( rc )
                Msg( "CommdataCreate DosAllocSharedMem failed! rc:%u", rc );
        }
        else
            Msg( "CommdataCreate DosGetNamedSharedMem failed! rc:%u", rc );
    }
}

/**********************************************************************/
/*------------------------- CommdataDestroy --------------------------*/
/*                                                                    */
/*  DESTROY THE SHARED COMMON DATA.                                   */
/*                                                                    */
/*  PARMS: nothing                                                    */
/*                                                                    */
/*  NOTES:                                                            */
/*                                                                    */
/*  RETURNS: nothing                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/
void CommdataDestroy()
{
    if( pCommData )
        DosFreeMem( pCommData );
}

/*************************************************************************
 *                     E N D     O F     S O U R C E                     *
 *************************************************************************/
