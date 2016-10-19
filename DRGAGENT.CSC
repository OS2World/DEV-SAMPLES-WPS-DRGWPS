include <wpabs.sc>

class: DrgAgent,
       file stem = DrgAgent,
       external prefix = Agent_,
       class prefix = AgentCls_,
       major version = 1,
       minor version = 1,
       local;

parent class: WPAbstract;

passthru: C.ih;

   #define INCL_DOSPROCESS
   #define INCL_WINMENUS
   #define INCL_WINWORKPLACE
   #include <os2.h>
   #include <stdio.h>
   #include <stdlib.h>
   #include <wpfolder.h>
   #include "drgagent.rh"

endpassthru;   /* .ih */

data:

   HWND hwndComm;

methods:

override wpDraggedOverObject, public, external;
override wpDroppedOnObject, public, external;
override wpclsQueryIconData, public, class, external;
override wpclsQueryStyle, public, class, external;
override wpclsQueryTitle, public, class, external;
override wpclsInitData, public, class, external;
override wpclsUnInitData, public, class, external;

