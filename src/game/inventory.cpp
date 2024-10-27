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

    ICOMMAND(getweaponcurammo, "ii", (int *i, bool *superWeapon),
        if(*superWeapon) findSpecialWeapon(player1, GUN_S_NUKE, NUMSUPERWEAPONS, [](int gunId) { intret(player1->ammo[gunId]); });
        else if(validInventoryWeapon(*i))
        {
            if(!validgun(playerWeapons[*i])) intret(0);
            else intret(player1->ammo[playerWeapons[*i]]);
        }
        else result(tempformatstring("Invalid gun (%d)", *i));
    );

    ICOMMAND(getweaponmaxammo, "ii", (int *i, bool *superWeapon),
        if(*superWeapon) findSpecialWeapon(player1, GUN_S_NUKE, NUMSUPERWEAPONS, [](int gunId) { intret(itemstats[gunId].max); });
        else if(validInventoryWeapon(*i))
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

    ICOMMAND(getweaponid, "i", (int *i),
        if(validInventoryWeapon(*i) && validgun(playerWeapons[*i])) intret(playerWeapons[*i]);
        else result(tempformatstring("Invalid gun (%d)", *i));
    );

    ICOMMAND(getweaponname, "i", (int *i),
        if(validInventoryWeapon(*i) && validgun(playerWeapons[*i])) result(readstr(guns[playerWeapons[*i]].ident));
        else result(tempformatstring("Invalid gun (%d)", *i));
    );

    ICOMMAND(meleeweaponselected, "", (), intret((player1->gunselect >= GUN_M_BUSTER && player1->gunselect <= GUN_M_FLAIL) || player1->gunselect == GUN_NINJA));
    ICOMMAND(getmeleeweaponid, "", (), findSpecialWeapon(player1, GUN_M_BUSTER, NUMMELEEWEAPONS, [](int gunId) { intret(gunId); }); );
    ICOMMAND(getmeleeweaponname, "", (), findSpecialWeapon(player1, GUN_M_BUSTER, NUMMELEEWEAPONS, [](int gunId) { result(readstr(guns[gunId].ident)); }); );
    ICOMMAND(selectmeleeweapon, "", (),
        if(player1->character==C_NINJA)
        {
            gunselect(GUN_NINJA, player1);
            return;
        }
        findSpecialWeapon(player1, GUN_M_BUSTER, NUMMELEEWEAPONS, [](int gunId) { gunselect(gunId, player1); });
    );

    ICOMMAND(hassuperweapon, "", (), intret(hasSuperWeapon(player1)));
    ICOMMAND(superweaponselected, "", (), intret(player1->gunselect >= GUN_S_NUKE && player1->gunselect <= GUN_S_CAMPER));
    ICOMMAND(getsuperweaponid, "", (), findSpecialWeapon(player1, GUN_S_NUKE, NUMSUPERWEAPONS, [](int gunId) { intret(gunId); }); );
    ICOMMAND(getsuperweaponname, "", (), findSpecialWeapon(player1, GUN_S_NUKE, NUMSUPERWEAPONS, [](int gunId) { result(readstr(guns[gunId].ident)); }); );
    ICOMMAND(selectsuperweapon, "", (), findSpecialWeapon(player1, GUN_S_NUKE, NUMSUPERWEAPONS, [](int gunId) { gunselect(gunId, player1); }); );

    int classWeapon(bool maxAmmo = false)
    {
        switch(player1->character)
        {
            case C_KAMIKAZE:
                if(maxAmmo) return 1;
                else return GUN_KAMIKAZE;
                break;
            default:
                return 0;
        }
    }

    ICOMMAND(getclassweaponcurammo, "", (), intret(player1->ammo[classWeapon()]));
    ICOMMAND(getclassweaponmaxammo, "", (), intret(classWeapon(true)));
    ICOMMAND(classweaponselected, "", (), intret(player1->gunselect == classWeapon()));
    ICOMMAND(getclasswweaponid, "", (), intret(classWeapon()));
    ICOMMAND(getclasswweaponname, "", (), result(readstr(guns[classWeapon()].ident)));
    ICOMMAND(selectclasswweapon, "", (), gunselect(classWeapon(), player1));

    ICOMMAND(infiniteammo, "", (), intret(m_muninfinie));
}
