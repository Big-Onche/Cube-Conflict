// client.cpp, mostly network related client game code

#include "engine.h"
#include "game.h"
#include "gfx.h"
#include "stats.h"

ENetHost *clienthost = NULL;
ENetPeer *curpeer = NULL, *connpeer = NULL;
int connmillis = 0, connattempts = 0, discmillis = 0;

bool multiplayer(bool msg)
{
    bool val = curpeer || hasnonlocalclients();
    if(val && msg) conoutf(CON_ERROR, "\f3%s", readstr("Console_NotAvailableInMultiplayer"));
    return val;
}

void setrate(int rate)
{
   if(!curpeer) return;
   enet_host_bandwidth_limit(clienthost, rate*1024, rate*1024);
}

VARF(rate, 0, 0, 1024, setrate(rate));

void throttle();

VARF(throttle_interval, 0, 5, 30, throttle());
VARF(throttle_accel,    0, 2, 32, throttle());
VARF(throttle_decel,    0, 2, 32, throttle());

void throttle()
{
    if(!curpeer) return;
    ASSERT(ENET_PEER_PACKET_THROTTLE_SCALE==32);
    enet_peer_throttle_configure(curpeer, throttle_interval*1000, throttle_accel, throttle_decel);
}

bool isconnected(bool attempt, bool local)
{
    return curpeer || (attempt && connpeer) || (local && haslocalclients());
}

ICOMMAND(isconnected, "bb", (int *attempt, int *local), intret(isconnected(*attempt > 0, *local != 0) ? 1 : 0));

const ENetAddress *connectedpeer()
{
    return curpeer ? &curpeer->address : NULL;
}

ICOMMAND(connectedip, "", (),
{
    const ENetAddress *address = connectedpeer();
    string hostname;
    result(address && enet_address_get_host_ip(address, hostname, sizeof(hostname)) >= 0 ? hostname : "");
});

ICOMMAND(connectedport, "", (),
{
    const ENetAddress *address = connectedpeer();
    intret(address ? address->port : -1);
});

void abortconnect()
{
    IS_ON_OFFICIAL_SERV = false;
    if(!connpeer) return;
    game::connectfail();
    if(connpeer->state!=ENET_PEER_STATE_DISCONNECTED) enet_peer_reset(connpeer);
    connpeer = NULL;
    if(curpeer) return;
    enet_host_destroy(clienthost);
    clienthost = NULL;
}

SVARP(connectname, "");
VARP(connectport, 0, 0, 0xFFFF);

void connectserv(const char *servername, int serverport, const char *serverpassword)
{
    if(connpeer)
    {
        conoutf(CON_INFO, readstr("Console_Connection_Abort"));
        IS_ON_OFFICIAL_SERV = false;
        abortconnect();
    }

    if(serverport <= 0) serverport = server::serverport();

    ENetAddress address;
    address.port = serverport;

    const char* domains[] = {"s1.cube-conflict.com", "s2.cube-conflict.com", "s3.cube-conflict.com"};

    if(servername)
    {
        if(strcmp(servername, connectname)) setsvar("connectname", servername);
        if(serverport != connectport) setvar("connectport", serverport);
        addserver(servername, serverport, serverpassword && serverpassword[0] ? serverpassword : NULL);
        conoutf(CON_INFO, "%s %s:%d", readstr("Console_Connection_Attempt"), servername, serverport);
        loopi(sizeof(domains)/sizeof(domains[0])) {if(!strcasecmp(servername, domains[i])) IS_ON_OFFICIAL_SERV = true; }
        if(IS_ON_OFFICIAL_SERV) conoutf(CON_INFO, "\fe%s", readstr("Console_Connection_OfficialServ"));

        if(!resolverwait(servername, &address))
        {
            conoutf(CON_ERROR, "\f3%s %s", readstr("Console_Connection_ServNotFound"), servername);
            IS_ON_OFFICIAL_SERV = false;
            return;
        }
    }
    else
    {
        setsvar("connectname", "");
        setvar("connectport", 0);
        conoutf(CON_INFO, readstr("Console_Connection_AttemptLan"));
        address.host = ENET_HOST_BROADCAST;
    }

    if(!clienthost)
    {
        clienthost = enet_host_create(NULL, 2, server::numchannels(), rate*1024, rate*1024);
        if(!clienthost)
        {
            conoutf(CON_ERROR, "\f3%s", readstr("Console_Connection_Impossible"));
            IS_ON_OFFICIAL_SERV = false;
            return;
        }
        clienthost->duplicatePeers = 0;
    }

    connpeer = enet_host_connect(clienthost, &address, server::numchannels(), 0);
    enet_host_flush(clienthost);
    connmillis = totalmillis;
    connattempts = 0;

    game::connectattempt(servername ? servername : "", serverpassword ? serverpassword : "", address);
}

void reconnect(const char *serverpassword)
{
    if(!connectname[0] || connectport <= 0)
    {
        conoutf(CON_ERROR, "\f3%s", readstr("Console_Connection_NoPrevious"));
        return;
    }

    connectserv(connectname, connectport, serverpassword);
}

void disconnect(bool async, bool cleanup, bool volontaire)
{
    if(curpeer)
    {
        if(!discmillis || volontaire)
        {
            enet_peer_disconnect(curpeer, volontaire ? DISC_NORMAL : DISC_NONE);
            enet_host_flush(clienthost);
            discmillis = totalmillis;
        }
        if(curpeer->state!=ENET_PEER_STATE_DISCONNECTED)
        {
            if(async) return;
            enet_peer_reset(curpeer);
        }
        curpeer = NULL;
        discmillis = 0;
        conoutf(CON_INFO, readstr("Console_Connection_Disconnected"));
        IS_ON_OFFICIAL_SERV = false;
        game::gamedisconnect(cleanup);
        gfx::resetpostfx();
        clearsleep();
        stopAllSounds();
        game::clearbouncers();
        game::clearprojectiles(); // clean up before showing menu
        loopi(NUMSONGS) stopMusic(i);
        playMusic(language == 2 ? S_MAINMENURU : S_MAINMENU);
        mainmenu = 1;
        if(stat[STAT_DAMMAGERECORD] < game::player1->totaldamage/10) updateStat(game::player1->totaldamage/10, STAT_DAMMAGERECORD, true);
        if(game::player1->totaldamage/10 > 10000) unlockAchievement(ACH_DESTRUCTEUR);
    }
    if(!connpeer && clienthost)
    {
        enet_host_destroy(clienthost);
        clienthost = NULL;
    }
}

void trydisconnect(bool local)
{
    stopAllSounds();
    game::clearbouncers();
    game::clearprojectiles();
    loopi(NUMSONGS) stopMusic(i);
    playMusic(language == 2 ? S_MAINMENURU : S_MAINMENU);
    if(connpeer)
    {
        conoutf(CON_INFO, readstr("Console_Connection_Abort"));
        abortconnect();
    }
    else if(curpeer)
    {
        conoutf(CON_INFO, readstr("Console_Connection_DiscAttempt"));
        disconnect(!discmillis, true, true);
    }
    else if(local && haslocalclients()) localdisconnect();
    else conoutf(CON_WARN, readstr("Console_Connection_NotConnected"));
}

ICOMMAND(connect, "sis", (char *name, int *port, char *pw), connectserv(name, *port, pw));
ICOMMAND(lanconnect, "is", (int *port, char *pw), connectserv(NULL, *port, pw));
COMMAND(reconnect, "s");
ICOMMAND(disconnect, "b", (int *local), trydisconnect(*local != 0));
ICOMMAND(localconnect, "", (), { if(!isconnected()) localconnect(); });
ICOMMAND(localdisconnect, "", (), { if(haslocalclients()) localdisconnect(); });

void sendclientpacket(ENetPacket *packet, int chan)
{
    if(curpeer) enet_peer_send(curpeer, chan, packet);
    else localclienttoserver(chan, packet);
}

void flushclient()
{
    if(clienthost) enet_host_flush(clienthost);
}

void neterr(const char *s, bool disc)
{
    conoutf(CON_ERROR, "\f3%s (%s).", readstr("Console_Connection_IllegalMsg"), s);
    if(disc) disconnect();
}

void localservertoclient(int chan, ENetPacket *packet)   // processes any updates from the server
{
    packetbuf p(packet);
    game::parsepacketclient(chan, p);
}

void clientkeepalive() { if(clienthost) enet_host_service(clienthost, NULL, 0); }

VAR(discReason, 0, 0, 3);

void gets2c()           // get updates from the server
{
    ENetEvent event;
    if(!clienthost) return;
    if(connpeer && totalmillis/3000 > connmillis/3000)
    {
        conoutf(CON_INFO, readstr("Console_Connection_AttemptWait"));
        connmillis = totalmillis;
        ++connattempts;
        if(connattempts > 3)
        {
            conoutf(CON_ERROR, "\f3%s", readstr("Console_Connection_Impossible"));
            abortconnect();
            return;
        }
    }
    while(clienthost && enet_host_service(clienthost, &event, 0)>0)
    switch(event.type)
    {
        case ENET_EVENT_TYPE_CONNECT:
            disconnect(false, false, true);
            localdisconnect(false);
            curpeer = connpeer;
            connpeer = NULL;
            conoutf(CON_INFO, readstr("Console_Connection_Success"));
            throttle();
            if(rate) setrate(rate);
            game::gameconnect(true);
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            if(discmillis) conoutf(CON_INFO, readstr("Console_Connection_Disconnecting"));
            else localservertoclient(event.channelID, event.packet);
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            if(event.data>=DISC_NUM) event.data = DISC_NONE;
            if(event.peer==connpeer)
            {
                conoutf(CON_ERROR, "\f3%s", readstr("Console_Connection_Impossible"));
                abortconnect();
            }
            else
            {
                if(!discmillis || event.data)
                {
                    const char *msg = readstr(disconnectreason(event.data));
                    if(msg) conoutf(CON_ERROR, "\f3%s (%s).", readstr("Console_Connection_NetError"), msg);
                    else conoutf(CON_ERROR, "\f3%s.", readstr("Console_Connection_NetError"));
                }
                disconnect();
            }
            switch(event.data)
            {
                case DISC_OVERFLOW:
                    discReason = 0;
                    break;
                case DISC_SERVERSTOP:
                    discReason = 1;
                    break;
                case DISC_MSGERR:
                case DISC_SERVMSGERR:
                    discReason = 2;
                    break;
                default:
                    discReason = 3;
            }
            UI::showui("disconnected");
            return;

        default:
            break;
    }
}

