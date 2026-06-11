// hitlog.cpp: hitlog / damage-log state, helpers, and UI commands.

#include "game.h"

static const int BURST_WINDOW = 500;

DamageLog::DamageLog() :
    weapon(-1),
    damage(0),
    hits(0),
    millis(0),
    distance(0),
    isActor(false),
    isAfterburn(false),
    friendlyActor(false),
    friendlyVictim(false),
    killHit(false)
{
    actorName[0] = victimName[0] = 0;
}

void DamageLog::clear()
{
    actorName[0] = victimName[0] = 0;
    hits = 0;
    weapon = -1;
    damage = 0;
    millis = 0;
    distance = 0;
    isActor = false;
    isAfterburn = false;
    friendlyActor = false;
    friendlyVictim = false;
    killHit = false;
}

void DamageLog::set(const char *actorName, const char *victimName, int weapon, int damage, bool isActor, bool isAfterburn, float distance, int millis, bool friendlyActor, bool friendlyVictim)
{
    copystring(this->actorName, actorName ? actorName : "");
    copystring(this->victimName, victimName ? victimName : "");
    hits = 1;
    this->weapon = weapon;
    this->damage = damage;
    this->millis = millis;
    this->distance = distance;
    this->isActor = isActor;
    this->isAfterburn = isAfterburn;
    this->friendlyActor = friendlyActor;
    this->friendlyVictim = friendlyVictim;
    killHit = false;
}

bool DamageLog::matchesBurst(const char *actorName, const char *victimName, int atk, bool isActor, bool isAfterburn) const
{
    return weapon == atk &&
           this->isActor == isActor &&
           this->isAfterburn == isAfterburn &&
           !strcmp(this->actorName, actorName ? actorName : "") &&
           !strcmp(this->victimName, victimName ? victimName : "");
}

void DamageLog::mergeBurst(const char *actorName, const char *victimName, int damage, bool isAfterburn, float distance, int millis, bool friendlyActor, bool friendlyVictim)
{
    copystring(this->actorName, actorName ? actorName : "");
    copystring(this->victimName, victimName ? victimName : "");
    hits++;
    this->damage += damage;
    this->millis = millis;
    this->distance = distance;
    this->isAfterburn = isAfterburn;
    this->friendlyActor = friendlyActor;
    this->friendlyVictim = friendlyVictim;
}

void gameent::clearDamageLog()
{
    damageHistory.clear();
}

void gameent::clearKillHitLog()
{
    loopi(damageHistory.count) damageHistory.entries[(damageHistory.offset + i) % DAMAGE_LOG_LENGTH].killHit = false;
}

void gameent::markKillHit(const char *actorName, const char *victimName, int weapon, bool isActor)
{
    clearKillHitLog();

    for(int index = damageHistory.count - 1; index >= 0; --index)
    {
        int slot = (damageHistory.offset + index) % DAMAGE_LOG_LENGTH;
        DamageLog &entry = damageHistory.entries[slot];
        if(entry.weapon != weapon ||
            entry.isActor != isActor ||
            strcmp(entry.actorName, actorName ? actorName : "") ||
            strcmp(entry.victimName, victimName ? victimName : "")) continue;

        entry.killHit = true;
        break;
    }
}

void gameent::logLastHit(const char *actorName, const char *victimName, int weapon, int damage, bool isActor, bool isAfterburn, float distance, bool friendlyActor, bool friendlyVictim)
{
    if(damage <= 0) return;

    const int millis = lastmillis;
    const int burstWindow = isAfterburn ? BURST_WINDOW * 2 : BURST_WINDOW;

    if(burstWindow > 0)
    {
        for(int index = damageHistory.count - 1; index >= 0; --index)
        {
            int slot = (damageHistory.offset + index) % DAMAGE_LOG_LENGTH;
            DamageLog &entry = damageHistory.entries[slot];
            if(!entry.matchesBurst(actorName, victimName, weapon, isActor, isAfterburn)) continue;
            if(millis - entry.millis > burstWindow) break;

            entry.mergeBurst(actorName, victimName, damage, isAfterburn, distance, millis, friendlyActor, friendlyVictim);
            return;
        }
    }

    int slot = (damageHistory.offset + damageHistory.count) % DAMAGE_LOG_LENGTH;
    if(damageHistory.count >= DAMAGE_LOG_LENGTH)
    {
        slot = damageHistory.offset;
        damageHistory.offset = (damageHistory.offset + 1) % DAMAGE_LOG_LENGTH;
    }
    else damageHistory.count++;

    damageHistory.entries[slot].set(actorName, victimName, weapon, damage, isActor, isAfterburn, distance, millis, friendlyActor, friendlyVictim);
}

const DamageLog *gameent::getLastDamage(int index) const
{
    if(index < 0 || index >= damageHistory.count) return NULL;
    return &damageHistory.entries[(damageHistory.offset + damageHistory.count - 1 - index + DAMAGE_LOG_LENGTH) % DAMAGE_LOG_LENGTH];
}

int gameent::getKillIndex() const
{
    loopi(damageHistory.count)
    {
        const DamageLog *entry = getLastDamage(i);
        if(entry && entry->killHit) return i;
    }
    return -1;
}

namespace game
{
    static inline bool teamDamage(gameent *d)
    {
        return d && player1 && (d == player1 || isteam(player1->team, d->team));
    }

    static inline const DamageLog *getDamageLog(int index)
    {
        if(index < 0 || index >= DAMAGE_LOG_LENGTH) return NULL;

        gameent *hp = hudplayer();
        return hp ? hp->getLastDamage(index) : NULL;
    }

    static inline const char *getHitlogNameValue(const DamageLog *entry)
    {
        if(!entry) return "";

        const char *name = entry->isActor ? entry->victimName : entry->actorName;
        if(!name[0]) return "";

        const bool isFriendly = entry->isActor ? entry->friendlyVictim : entry->friendlyActor;
        return tempformatstring("%s%s\fr", isFriendly ? "\fd" : "\fc", name);
    }

    void updateHitlogUi()
    {
        gameent *hp = hudplayer();
        if(hp && hp->state == CS_DEAD) UI::showui("hitlog");
        else UI::hideui("hitlog");
    }

    void updateHitlogKill(gameent *victim, gameent *actor, int atk)
    {
        if(!victim) return;
        if(!actor)
        {
            victim->clearKillHitLog();
            return;
        }

        defformatstring(hitlogVictimName, "%s", colorname(victim));
        defformatstring(hitlogActorName, "%s", colorname(actor));
        victim->markKillHit(hitlogActorName, hitlogVictimName, atk, victim == actor);
    }

    void recordHitlogDamage(gameent *actor, gameent *victim, int weapon, int damage, bool afterburnHit, float distance)
    {
        if(!actor || !victim || damage <= 0) return;

        defformatstring(actorName, "%s", colorname(actor));
        defformatstring(victimName, "%s", colorname(victim));
        bool friendlyActor = teamDamage(actor);
        bool friendlyVictim = teamDamage(victim);
        actor->logLastHit(actorName, victimName, weapon, damage, true, afterburnHit, distance, friendlyActor, friendlyVictim);
        if(actor != victim) victim->logLastHit(actorName, victimName, weapon, damage, false, afterburnHit, distance, friendlyActor, friendlyVictim);
    }

    ICOMMAND(gethitloglength, "", (),
        gameent *hp = hudplayer();
        intret(hp ? hp->damageHistory.count : 0);
    );

    ICOMMAND(gethitlogkillindex, "", (),
        gameent *hp = hudplayer();
        intret(hp ? hp->getKillIndex() : -1);
    );

    ICOMMAND(gethitlogdamage, "i", (int *index),
        const DamageLog *entry = getDamageLog(*index);
        intret(entry ? entry->damage : 0);
    );

    ICOMMAND(gethitloghits, "i", (int *index),
        const DamageLog *entry = getDamageLog(*index);
        intret(entry ? entry->hits : 0);
    );

    ICOMMAND(gethitlogweapon, "i", (int *index),
        const DamageLog *entry = getDamageLog(*index);
        intret(entry ? entry->weapon : -1);
    );

    ICOMMAND(gethitlogname, "i", (int *index),
        const DamageLog *entry = getDamageLog(*index);
        result(getHitlogNameValue(entry));
    );

    ICOMMAND(gethitlogisactor, "i", (int *index),
        const DamageLog *entry = getDamageLog(*index);
        intret(entry && entry->isActor ? 1 : 0);
    );

    /*
    Dead hitlog UI commands retained as comments for now.
    ICOMMAND(getlastdamageisafterburn, "i", (int *index),
        const DamageLog *entry = getDamageLog(*index);
        intret(entry && entry->isAfterburn ? 1 : 0);
    );
    */

    ICOMMAND(gethitlogdistance, "i", (int *index),
        const DamageLog *entry = getDamageLog(*index);
        floatret(entry ? round(distToMeter(entry->distance) * 10) / 10 : 0);
    );

}
