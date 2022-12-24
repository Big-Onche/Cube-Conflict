// rpg for cube conflict

#include "game.h"
#include "customisation.h"

namespace game
{
    //////////////////////////////////// Tutorial ////////////////////////////////////////////////////////////////////////

    VAR(examresult, 0, 0, 4); //calc exam result from tutorial's map
    ICOMMAND(calcexamresult, "iiii", (int *a, int *b, int *c, int *d),
        int max_value = max(max(*a, *b), max(*c, *d));
        max_value == *b ? examresult = 2 : max_value == *d ? examresult = 4 : max_value == *a ? examresult = 1 : examresult = 3;
    );

    ICOMMAND(getfreecust, "", (), //free customs from treasure
        if(m_tutorial) cust[TOM_BASIQUE1] = cust[CAPE_JVC] = cust[SMI_NOEL] = rnd(99)+1;
    );
}
