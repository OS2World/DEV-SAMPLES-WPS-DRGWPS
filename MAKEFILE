###########################################################################
#                                                                         #
# MAKE FILE FOR DRGPMWPS.EXE                                              #
#                                                                         #
# NOTES:                                                                  #
#                                                                         #
#  To enable the C Set/2 memory management debugging code, uncomment the  #
#  DEBUGALLOC macro. The debugging info will be sent to DRGPMWPS.DBG.     #
#                                                                         #
#  This makefile creates DRGPMWPS.EXE and DRGAGENT.DLL                    #
#                                                                         #
# HISTORY:                                                                #
#                                                                         #
#  08-01-93 - started coding.                                             #
#                                                                         #
#  Rick Fishman                                                           #
#  Code Blazers, Inc.                                                     #
#  4113 Apricot                                                           #
#  Irvine, CA. 92720                                                      #
#  CIS ID: 72251,750                                                      #
#                                                                         #
###########################################################################

#DEBUGALLOC=-D__DEBUG_ALLOC__

# /Kc not included because the SOM headers cause lots of messages
# saying includes defined more than once

EXECFLAGS = /Q+ /Ss /Kbcepr /Wuse-par- /Ge+ /Gd- /Gm+ /Ti /C
DLLCFLAGS = /Q+ /Ss /Kbepr /Wuse-par- /Ge- /Gd- /Gm+ /Ti /C
LFLAGS = /noe /noi /map /nol /exepack /packcode /packdata /DE /align:16
LIBS   = som.lib os2386.lib

.SUFFIXES: .csc .c .obj .dll .sc .h .ih .ph .psc .rc .res

BASE=drgpmwps
BASEOBJS =$(BASE).obj drag.obj commdata.obj heap.obj
BASELOBJS=$(BASEOBJS:.obj=)

WPSDLL=drgagent
WPSOBJS =$(WPSDLL).obj commwin.obj commdata.obj heap.obj
WPSLOBJS=$(WPSOBJS:.obj=)

all: $(WPSDLL).dll $(BASE).exe

.csc.c:
    sc $<

.c.obj:
    icc $(DLLCFLAGS) $<

$(WPSDLL).dll: $(WPSOBJS) $(WPSDLL).res
    link386 $(LFLAGS) $(WPSLOBJS) printf,$@,,$(LIBS),$*
    rc $*.res $@

$(WPSDLL).res: $(WPSDLL).rc $(WPSDLL).rh
    rc -r $*

$(BASE).exe: $(BASEOBJS) $(BASE).res $(BASE).def
    link386 $(LFLAGS) $(BASELOBJS),,, os2386, $*
    rc $*.res $@

$(BASE).res: $(BASE).rc
    rc -r $*

$(WPSDLL).c: $(WPSDLL).csc

$(BASE).obj:   $(BASE).c $(BASE).h common.h makefile
    icc $(EXECFLAGS) $*.c

$(WPSDLL).obj: $(WPSDLL).c $(WPSDLL).rh common.h makefile
drag.obj:      drag.c $(BASE).h common.h makefile
heap.obj:      heap.c common.h makefile
commwin.obj:   commwin.c common.h makefile
commdata.obj:  commdata.c common.h makefile

