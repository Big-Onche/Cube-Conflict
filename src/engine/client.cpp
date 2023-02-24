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
    if(val && msg) conoutf(CON_ERROR, GAME_LANG ? "Operation not available in multiplayer." : "Opération non disponible en multijoueur.");
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
        conoutf(GAME_LANG ? "Aborting connection attempt." : "Abandon de la tentative de connexion.");
        IS_ON_OFFICIAL_SERV = false;
        abortconnect();
    }

    if(serverport <= 0) serverport = server::serverport();

    ENetAddress address;
    address.port = serverport;

    if(servername)
    {
        if(strcmp(servername, connectname)) setsvar("connectname", servername);
        if(serverport != connectport) setvar("connectport", serverport);
        addserver(servername, serverport, serverpassword && serverpassword[0] ? serverpassword : NULL);
        conoutf("%s %s:%d", GAME_LANG ? "Attempting to connect to" : "Connexion en cours à", servername, serverport);
        if(strcasecmp(servername, "serveur1.cube-conflict.com")==0 || strcasecmp(servername, "serveur2.cube-conflict.com")==0) IS_ON_OFFICIAL_SERV = true;
        if(IS_ON_OFFICIAL_SERV) conoutf(GAME_LANG ? "Official server: Stats and achievements are saved." : "Serveur officiel : Les statistiques et succès sont enregistrés.");

        if(!resolverwait(servername, &address))
        {
            conoutf(CON_ERROR, "\f3%s %s", GAME_LANG ? "Could not resolve server": "Impossible de trouver le serveur", servername);
            IS_ON_OFFICIAL_SERV = false;
            return;
        }
    }
    else
    {
        setsvar("connectname", "");
        setvar("connectport", 0);
        conoutf(GAME_LANG ? "Attempting to connect over LAN" : "Connexion en cours au réseau LAN");
        address.host = ENET_HOST_BROADCAST;
    }

    if(!clienthost)
    {
        clienthost = enet_host_create(NULL, 2, server::numchannels(), rate*1024, rate*1024);
        if(!clienthost)
        {
            conoutf(CON_ERROR, GAME_LANG ? "\f3Could not connect to server." : "\f3La connexion au serveur a échoué.");
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
        conoutf(CON_ERROR, GAME_LANG ? "No previous connection." : "Aucune connexion précédente.");
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
        conoutf(GAME_LANG ? "Disconnected" : "Déconnecté");
        IS_ON_OFFICIAL_SERV = false;
        game::gamedisconnect(cleanup);
        gfx::resetshroomsgfx();
        clearsleep();
        soundmenu_cleanup();
        mainmenu = 1;
        if(stat[STAT_DAMMAGERECORD] < game::player1->totaldamage/10) addstat(game::player1->totaldamage/10, STAT_DAMMAGERECORD, true);
        if(game::player1->totaldamage/10 > 10000) unlockachievement(ACH_DESTRUCTEUR);
    }
    if(!connpeer && clienthost)
    {
        enet_host_destroy(clienthost);
        clienthost = NULL;
    }
}

void trydisconnect(bool local)
{
    soundmenu_cleanup();

    if(connpeer)
    {
        conoutf(GAME_LANG ? "Aborting connection attempt" : "Annulation de la connexion");
        abortconnect();
    }
    else if(curpeer)
    {
        conoutf(GAME_LANG ? "Attempting to disconnect..." : "Déconnexion en cours...");
        disconnect(!discmillis, true, true);
    }
    else if(local && haslocalclients()) localdisconnect();
    else conoutf(CON_WARN, GAME_LANG ? "Not connected" : "Non connecté");
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
    conoutf(CON_ERROR, "\f3%s (%s)", GAME_LANG ? "Illegal network message" : "Message réseau erroné", s);
    if(disc) disconnect();
}

void localservertoclient(int chan, ENetPacket *packet)   // processes any updates from the server
{
    packetbuf p(packet);
    game::parsepacketclient(chan, p);
}

void clientkeepalive() { if(clienthost) enet_host_service(clienthost, NULL, 0); }

void gets2c()           // get updates from the server
{
    ENetEvent event;
    if(!clienthost) return;
    if(connpeer && totalmillis/3000 > connmillis/3000)
    {
        conoutf(GAME_LANG ? "Attempting to connect..." : "Connexion au serveur...");
        connmillis = totalmillis;
        ++connattempts;
        if(connattempts > 3)
        {
            conoutf(CON_ERROR, GAME_LANG ? "\f3Could not connect to server" : "\f3Connexion au serveur impossible");
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
            conoutf(GAME_LANG ? "Connexion successful." : "Connecté au serveur.");
            throttle();
            if(rate) setrate(rate);
            game::gameconnect(true);
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            if(discmillis) conoutf(GAME_LANG ? "Disconnecting..." : "Déconnexion...");
            else localservertoclient(event.channelID, event.packet);
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            if(event.data>=DISC_NUM) event.data = DISC_NONE;
            if(event.peer==connpeer)
            {
                conoutf(CON_ERROR, GAME_LANG ? "\f3Could not connect to server." : "\f3Impossible de se connecter au serveur.");
                abortconnect();
            }
            else
            {
                if(!discmillis || event.data)
                {
                    const char *msg = disconnectreason(event.data);
                    if(msg) conoutf(CON_ERROR, GAME_LANG ? "\f3Server network error, disconnecting (%s) ..." : "\f3Erreur de serveur, déconnexion (%s)", msg);
                    else conoutf(CON_ERROR, GAME_LANG ? "\f3Server network error, disconnecting." : "\f3Erreur de serveur, déconnexion.");
                }
                disconnect();
            }
            switch(event.data)
            {
                case DISC_OVERFLOW: UI::showui("overflowpopup"); break;
                case DISC_SERVERSTOP:  UI::showui("servstoppopup"); break;
                case DISC_MSGERR: case DISC_SERVMSGERR: UI::showui("msgerrpopup"); break;
            }
            return;

        default:
            break;
    }
}

