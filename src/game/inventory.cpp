#include "game.h"

namespace game
{
    const int MAXWEAPONS = 8;
    bool validInventoryWeapon(int i) { return i >= 0 && i < MAXWEAPONS; }
    int playerWeapons[MAXWEAPONS];

    void checkInventoryGuns()
    {
        int weaponAmount = 0;
        loopi(MAXWEAPONS) playerWeapons[i] = -1;

        loopi(NUMMAINGUNS)
        {
            if(player1->hasammo(i))
            {
                if(playerWeapons[weaponAmount] < 0) playerWeapons[weaponAmount] = i;
                weaponAmount++;
                if(!validInventoryWeapon(weaponAmount)) return;
            }
        }
    }

    ICOMMAND(maxweapons, "", (), intret(MAXWEAPONS));
    ICOMMAND(selectgun, "i", (int *i), gunselect(playerWeapons[*i], player1));

    ICOMMAND(getweaponcurammo, "i", (int *i),
        if(validInventoryWeapon(*i))
        {
            if(!validgun(playerWeapons[*i])) intret(0);
            else intret(player1->ammo[playerWeapons[*i]]);
        }
        else result(tempformatstring("Invalid gun (%d)", *i));
    );

    ICOMMAND(getweaponmaxammo, "i", (int *i),
        if(validInventoryWeapon(*i))
        {
            if(!validgun(playerWeapons[*i])) intret(0);
            else
            {
                int maxAmmo = itemstats[playerWeapons[*i]].max;
                intret(player1->character == C_AMERICAN ? maxAmmo * 1.5f : maxAmmo);
            }
        }
        else result(tempformatstring("Invalid gun (%d)", *i));
    );

    ICOMMAND(currentgun, "i", (int *i),
        if(validInventoryWeapon(*i)) intret(playerWeapons[*i] == player1->gunselect);
        else result(tempformatstring("Invalid gun (%d)", *i));
    );

    ICOMMAND(getweaponmodel, "i", (int *i),
        if(validInventoryWeapon(*i) && validgun(playerWeapons[*i])) result(getWeaponDir(playerWeapons[*i]));
        else result(tempformatstring("Invalid gun (%d)", *i));
    );

    ICOMMAND(getweaponname, "i", (int *i),
        if(validInventoryWeapon(*i) && validgun(playerWeapons[*i])) result(readstr(guns[playerWeapons[*i]].ident));
        else result(tempformatstring("Invalid gun (%d)", *i));
    );

    ICOMMAND(hassuperweapon, "", (), intret(hasSuperWeapon(player1)));

    ICOMMAND(getmeleeweaponmodel, "", (), findSpecialWeapon(player1, GUN_M_BUSTER, NUMMELEEWEAPONS, [](int gunId) { result(getWeaponDir(gunId)); }); );
    ICOMMAND(getmeleeweaponname, "", (), findSpecialWeapon(player1, GUN_M_BUSTER, NUMMELEEWEAPONS, [](int gunId) { result(readstr(guns[gunId].ident)); }); );
    ICOMMAND(selectmeleeweapon, "", (),
        if(player1->character==C_NINJA)
        {
            gunselect(GUN_NINJA, player1);
            return;
        }
        findSpecialWeapon(player1, GUN_M_BUSTER, NUMMELEEWEAPONS, [](int gunId) { gunselect(gunId, player1); });
    );

    ICOMMAND(getsuperweaponmodel, "", (), findSpecialWeapon(player1, GUN_S_NUKE, NUMSUPERWEAPONS, [](int gunId) { result(getWeaponDir(gunId)); }); );
    ICOMMAND(getsuperweaponname, "", (), findSpecialWeapon(player1, GUN_S_NUKE, NUMSUPERWEAPONS, [](int gunId) { result(readstr(guns[gunId].ident)); }); );
    ICOMMAND(selectsuperweapon, "", (), findSpecialWeapon(player1, GUN_S_NUKE, NUMSUPERWEAPONS, [](int gunId) { gunselect(gunId, player1); }); );
}
