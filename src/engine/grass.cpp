#include "engine.h"

static void grasssettingschanged();
static void cleargrassburnfield();

VARP(grass, 0, 1, 1);
VARP(grassdist, 0, 768, 10000);
FVARP(grasstaper, 0, 0.3f, 1);
FVARFP(grassstep, 0.5f, 3.5, 8, grasssettingschanged());
FVARFP(grassdensity, 0.05f, 0.4, 4, grasssettingschanged());
VARFP(grasspatchsize, 4, 32, 256, grasssettingschanged());
VARFP(grassmaxinstances, 1024, 1<<20, 1<<22, grasssettingschanged());
VAR(grassheight, 1, 7, 64);
FVARP(grassmargin, 0, 0.25f, 32);
FVAR(grassmarginfade, 0, 0, 1);
VARR(grassscale, 1, 2, 64);
CVAR0R(grasscolour, 0xFFFFFF);
FVAR(grasstest, 0, 0.7f, 1);

VARP(grasslod1, 8, 192, 10000);
VAR(grasslodtransition, 0, 64, 256);
FVAR(grassloddensity1, 0.05f, 1, 1);

VARP(grassshadowcascades, 0, 2, 2);
FVARF(grassshadowdensitynear, 0.01f, 0.65f, 1, grasssettingschanged());
FVARF(grassshadowdensity, 0.01f, 0.25f, 1, grasssettingschanged());
VAR(grassshadowlodnear, 0, 1, 1);
VAR(grassshadowlodfar, 0, 1, 1);
VAR(grassshadowwindcascades, 0, 1, 2);
VAR(grassshadowdraws, 1, 0, 0);
VAR(grassshadowinstances0, 1, 0, 0);
VAR(grassshadowinstances1, 1, 0, 0);
VAR(grassstats, 0, 0, 1);
VAR(grassvisiblepatches, 1, 0, 0);
VAR(grassdrawcalls, 1, 0, 0);
VAR(grassinstancesrendered, 1, 0, 0);
VAR(grassmergedtris, 1, 0, 0);

VAR(grassanimmillis, 1, 3000, 60000);
FVAR(grassanimscale, 0, 0.2f, 1);
FVAR(grasswindangle, 0, 0, 360);
FVAR(grasswindscale, 0, 0.015f, 1);

VARP(grassimpulses, 0, 1, 1);
VARP(grassimpulsemaxevents, 1, 64, 256);
VARP(grassimpulsemaxpatch, 1, 4, 8);
FVARP(grassimpulsenear, 0, 256, 10000);
FVARP(grassimpulsedist, 0, 512, 10000);
FVAR(grassimpulsestrength, 0, 3, 5);
FVAR(grassimpulsewobble, 0, 0.5f, 1);
FVAR(grassimpulsewobblespeed, 0, 32, 64);
FVAR(grassimpulseminstrength, 0, 0.01f, 10);
VAR(grassimpulsebulletmergemillis, 0, 250, 500);
FVAR(grassimpulsebulletmergedist, 0, 16, 64);
VAR(grassimpulsesactive, 1, 0, 0);
VAR(grassimpulsecandidatechecks, 1, 0, 0);
VAR(grassimpulserelevantchecks, 1, 0, 0);
VAR(grassimpulsepatchesaffected, 1, 0, 0);

VAR(grassburnholdmillis, 0, 25000, 300000);
VAR(grassburnfademillis, 100, 15000, 300000);
VAR(grassburnpropagatemillis, 1, 500, 10000);
VAR(grassburnfadeinmillis, 1, 150, 5000);
VAR(grassburnmillis, 1, 500, 5000);
CVAR0R(grassburncolour, 0x4A3838);
CVAR0R(grassburnglowcolour, 0xFF2B00);
FVAR(grassburnpadding, -100, 10, 100);
FVAR(grassburnvariation, 0, 0.4f, 1);
FVAR(grassburnscrollx, -2, 0.03f, 2);
FVAR(grassburnscrolly, -2, 0.75f, 2);
FVAR(grassburnglowscale, 0, 6, 10);
VAR(grassburnparticles, 0, 1, 1);
VAR(grassburnparticlemillis, 16, 500, 1000);
VAR(grassburnparticlemax, 1, 32, 64);
VAR(grassburnparticleminmillis, 0, 3000, 10000);
VAR(grassburnparticlemaxmillis, 0, 6000, 10000);
FVAR(grassburnsparkspread, 0, 16, 512);
FVAR(grassburnflamethrowerradius, 0, 12, 256);
VAR(grassburncarrierinterval, 16, 500, 1000);
FVAR(grassburncarrierstep, 0, 64, 64);
VARFP(grassburnfieldtexelsize, 2, 8, 64, cleargrassburnfield());
VARFP(grassburnfieldmaxsize, 128, 2048, 4096, cleargrassburnfield());
VAR(grassburnfielddirtytiles, 1, 0, 0);
VAR(grassburnfieldtexelsupdated, 1, 0, 0);
VAR(grassburneventsactive, 1, 0, 0);
VAR(grassburncandidatechecks, 1, 0, 0);
VAR(grassburnrelevantchecks, 1, 0, 0);
VAR(grassburnpatchesaffected, 1, 0, 0);

enum { MAXGRASSIMPULSESPERPATCH = 8, GRASSIMPULSECELLSIZE = 64, MAXGRASSBURNEVENTS = 4096, GRASSBURNFIELDTILESIZE = 32,
       GRASSBURNEVENTCELLSIZE = 64 };

static uint grasshash(uint n);
static uint grassdamageseed = 0;

struct grassimpulse
{
    vec position, direction;
    float radius, strength, propagationspeed, falloff, radial;
    int starttime, lifetime, type;
    uint queryversion;
};

static vector<grassimpulse> grassimpulselist;
static hashtable<ullong, vector<int> > grassimpulsegrid(1<<10);
static uint grassimpulsequeryversion = 0;

static float grassimpulseremainingstrength(const grassimpulse &impulse)
{
    float age = clamp((lastmillis - impulse.starttime)/float(max(impulse.lifetime, 1)), 0.0f, 1.0f);
    return impulse.strength*(1.0f - age);
}

static void prunegrassimpulses()
{
    loopvrev(grassimpulselist) if(lastmillis - grassimpulselist[i].starttime >= grassimpulselist[i].lifetime)
        grassimpulselist.removeunordered(i);

    while(grassimpulselist.length() > grassimpulsemaxevents)
    {
        int weakest = 0;
        loopv(grassimpulselist)
            if(grassimpulseremainingstrength(grassimpulselist[i]) < grassimpulseremainingstrength(grassimpulselist[weakest])) weakest = i;
        grassimpulselist.removeunordered(weakest);
    }
}

void addgrassimpulse(const vec &position, const vec &direction, float radius, float strength, int lifetime, int type, float propagationspeed,
                     float falloff, float radial)
{
    if(!grassimpulses || position.isneg() || radius <= 0 || strength <= 0 || lifetime <= 0 ||
       (type != GRASS_IMPULSE_BULLET && type != GRASS_IMPULSE_EXPLOSION)) return;

    prunegrassimpulses();
    vec normalizeddir = vec(direction).safenormalize();
    if(type == GRASS_IMPULSE_BULLET && grassimpulsebulletmergemillis > 0 && grassimpulsebulletmergedist > 0)
    {
        float mergedist = max(grassimpulsebulletmergedist, radius*0.5f), mergeddist2 = mergedist*mergedist;
        loopv(grassimpulselist)
        {
            grassimpulse &impulse = grassimpulselist[i];
            if(impulse.type != type || lastmillis - impulse.starttime > grassimpulsebulletmergemillis ||
               impulse.position.squaredist(position) > mergeddist2) continue;
            impulse.position.add(position).mul(0.5f);
            impulse.direction.add(normalizeddir).safenormalize();
            impulse.radius = max(impulse.radius, radius);
            impulse.strength = max(impulse.strength, strength);
            impulse.lifetime = max(impulse.lifetime, lifetime);
            impulse.starttime = lastmillis;
            impulse.falloff = max(impulse.falloff, falloff);
            impulse.radial = max(impulse.radial, radial);
            return;
        }
    }

    int target = grassimpulselist.length();
    if(target >= grassimpulsemaxevents)
    {
        int weakest = 0;
        loopv(grassimpulselist)
            if(grassimpulseremainingstrength(grassimpulselist[i]) < grassimpulseremainingstrength(grassimpulselist[weakest])) weakest = i;
        if(strength <= grassimpulseremainingstrength(grassimpulselist[weakest])) return;
        target = weakest;
    }
    else grassimpulselist.add();

    grassimpulse &impulse = grassimpulselist[target];
    impulse.position = position;
    impulse.direction = normalizeddir;
    impulse.radius = radius;
    impulse.strength = strength;
    impulse.propagationspeed = max(propagationspeed, 0.0f);
    impulse.falloff = clamp(falloff, 0.0f, 1.0f);
    impulse.radial = max(radial, 0.0f);
    impulse.starttime = lastmillis;
    impulse.lifetime = lifetime;
    impulse.type = type;
    impulse.queryversion = 0;
}

void cleargrassimpulses()
{
    grassimpulselist.setsize(0);
    grassimpulsegrid.clear();
    grassimpulsequeryversion = 0;
}

struct grassburnevent
{
    vec center;
    float radius, seed, fieldradius;
    int burnstart, recoverstart, particleend, lastemit, lastfieldupdate;
    uint queryversion;
};

static void dirtygrassburnfield(const grassburnevent &event, float radius = -1);

struct grassburncarrier
{
    size_t owner;
    vec center;
    int lastspawn;
};

static vector<grassburnevent> grassburnevents;
static vector<grassburncarrier> grassburncarriers;

void addgrassburnevent(const vec &center, float radius, int lifetime)
{
    if(center.isneg() || radius <= 0) return;
    if(grassburnevents.length() >= MAXGRASSBURNEVENTS)
    {
        dirtygrassburnfield(grassburnevents[0]);
        grassburnevents.remove(0);
    }
    grassburnevent &event = grassburnevents.add();
    event.center = center;
    event.radius = radius;
    event.seed = (grasshash(++grassdamageseed) & 0xFFFFFFu)/float(0x1000000u);
    event.fieldradius = -1;
    event.burnstart = lastmillis;
    event.recoverstart = lastmillis + grassburnpropagatemillis + max(grassburnfadeinmillis, grassburnmillis) +
                         (lifetime >= 0 ? lifetime : grassburnholdmillis);
    int minparticlemillis = min(grassburnparticleminmillis, grassburnparticlemaxmillis),
        maxparticlemillis = max(grassburnparticleminmillis, grassburnparticlemaxmillis);
    event.particleend = lastmillis + minparticlemillis + int((maxparticlemillis - minparticlemillis)*event.seed + 0.5f);
    event.lastemit = lastmillis - grassburnparticlemillis;
    event.lastfieldupdate = -1;
    event.queryversion = 0;
}

void carrygrassburnevent(size_t owner, const vec &center, float radius)
{
    if(center.isneg() || radius <= 0) return;
    loopv(grassburncarriers) if(grassburncarriers[i].owner == owner)
    {
        grassburncarrier &carrier = grassburncarriers[i];
        float moveddist = carrier.center.squaredist(center);
        if(moveddist <= 1e-4f || (lastmillis - carrier.lastspawn < grassburncarrierinterval && moveddist < grassburncarrierstep*grassburncarrierstep)) return;
        carrier.center = center;
        carrier.lastspawn = lastmillis;
        addgrassburnevent(center, radius);
        return;
    }

    grassburncarrier &carrier = grassburncarriers.add();
    carrier.owner = owner;
    carrier.center = center;
    carrier.lastspawn = lastmillis;
    addgrassburnevent(center, radius);
}

void removegrassburnevent(size_t owner)
{
    loopvrev(grassburncarriers) if(grassburncarriers[i].owner == owner) grassburncarriers.removeunordered(i);
}

void cleargrassburnevents()
{
    grassburnevents.setsize(0);
    grassburncarriers.setsize(0);
    grassdamageseed = 0;
    cleargrassburnfield();
}

struct grassinstance
{
    vec4 originangle, variation;
};

struct grassmeshvert
{
    vec pos;
    vec2 tc;
};

struct grassmesh
{
    int offset, count, verts;
};

static GLuint grassmeshvbo = 0, grassmeshebo = 0;
static grassmesh grassmeshes[2];
static Shader *grassshader = NULL, *grassimpulseshader = NULL, *grassburnshader = NULL, *grassburnimpulseshader = NULL,
              *grassshadowshader = NULL;
static Texture *grassburntex = NULL;
static GLuint grassburnfieldtex = 0;
static int grassburnfielddim = 0, grassburnfieldworldsize = 0, grassburnfieldtiles = 0;
static float grassburnfieldcellsize = 0;
static vector<uchar> grassburnfielddirty;
static hashtable<ullong, vector<int> > grassburneventgrid(1<<12);
static uint grassburneventqueryversion = 0;
static float grassburnfieldlastpadding = 0, grassburnfieldlastvariation = 0;
static int grassburnfieldlastpropagation = -1, grassburnfieldlastfadein = -1, grassburnfieldlastburn = -1,
           grassburnfieldlastrecovery = -1;

static uint grasshash(uint n)
{
    n ^= n >> 16;
    n *= 0x7FEB352Du;
    n ^= n >> 15;
    n *= 0x846CA68Bu;
    return n ^ (n >> 16);
}

static uint grasshashcombine(uint seed, uint value)
{
    return grasshash(seed ^ (value + 0x9E3779B9u + (seed << 6) + (seed >> 2)));
}

static inline ullong grassimpulsecellkey(int x, int y)
{
    return (ullong(uint(x)) << 32) | uint(y);
}

static bool grassimpulseisrenderable(const grassimpulse &impulse)
{
    float strength = grassimpulseremainingstrength(impulse);
    if(strength <= grassimpulseminstrength || grassimpulsedist <= 0) return false;
    float dist = max(camera1->o.dist(impulse.position) - impulse.radius, 0.0f),
          limit = impulse.type == GRASS_IMPULSE_BULLET ? min(grassimpulsenear, grassimpulsedist) : grassimpulsedist;
    return dist <= limit && isvisiblesphere(impulse.radius + grassheight, impulse.position) != VFC_NOT_VISIBLE;
}

static void buildgrassimpulsegrid()
{
    prunegrassimpulses();
    enumerate(grassimpulsegrid, vector<int>, bucket, bucket.setsize(0));
    grassimpulsegrid.recycle();
    grassimpulsequeryversion = 0;
    if(!grassimpulses || grassimpulselist.empty()) return;

    int cells = max((worldsize + GRASSIMPULSECELLSIZE - 1)/GRASSIMPULSECELLSIZE, 1);
    loopv(grassimpulselist)
    {
        grassimpulse &impulse = grassimpulselist[i];
        if(!grassimpulseisrenderable(impulse)) continue;
        int x1 = clamp(int(floorf((impulse.position.x - impulse.radius)/GRASSIMPULSECELLSIZE)), 0, cells - 1),
            x2 = clamp(int(floorf((impulse.position.x + impulse.radius)/GRASSIMPULSECELLSIZE)), 0, cells - 1),
            y1 = clamp(int(floorf((impulse.position.y - impulse.radius)/GRASSIMPULSECELLSIZE)), 0, cells - 1),
            y2 = clamp(int(floorf((impulse.position.y + impulse.radius)/GRASSIMPULSECELLSIZE)), 0, cells - 1);
        for(int y = y1; y <= y2; ++y) for(int x = x1; x <= x2; ++x) grassimpulsegrid[grassimpulsecellkey(x, y)].add(i);
    }
}

static void collectgrassimpulsecandidates(float x1, float y1, float x2, float y2, vector<int> &candidates)
{
    candidates.setsize(0);
    if(!grassimpulsegrid.numelems) return;
    int cells = max((worldsize + GRASSIMPULSECELLSIZE - 1)/GRASSIMPULSECELLSIZE, 1),
        cx1 = clamp(int(floorf(x1/GRASSIMPULSECELLSIZE)), 0, cells - 1),
        cx2 = clamp(int(floorf(x2/GRASSIMPULSECELLSIZE)), 0, cells - 1),
        cy1 = clamp(int(floorf(y1/GRASSIMPULSECELLSIZE)), 0, cells - 1),
        cy2 = clamp(int(floorf(y2/GRASSIMPULSECELLSIZE)), 0, cells - 1);
    uint queryversion = ++grassimpulsequeryversion;
    if(!queryversion)
    {
        loopv(grassimpulselist) grassimpulselist[i].queryversion = 0;
        queryversion = ++grassimpulsequeryversion;
    }
    for(int y = cy1; y <= cy2; ++y) for(int x = cx1; x <= cx2; ++x)
    {
        vector<int> *bucket = grassimpulsegrid.access(grassimpulsecellkey(x, y));
        if(!bucket) continue;
        loopv(*bucket)
        {
            grassimpulse &impulse = grassimpulselist[(*bucket)[i]];
            if(impulse.queryversion == queryversion) continue;
            impulse.queryversion = queryversion;
            candidates.add((*bucket)[i]);
        }
    }
}

static float grassburneventfieldradius(const grassburnevent &event)
{
    return max(event.radius*(1 + grassburnvariation) + grassburnpadding, 0.0f);
}

static void cleargrassburnfield()
{
    if(grassburnfieldtex)
    {
        glDeleteTextures(1, &grassburnfieldtex);
        grassburnfieldtex = 0;
    }
    grassburnfielddim = grassburnfieldworldsize = grassburnfieldtiles = 0;
    grassburnfieldcellsize = 0;
    grassburnfielddirty.setsize(0);
    grassburneventgrid.clear();
    grassburneventqueryversion = 0;
    grassburnfieldlastpropagation = grassburnfieldlastfadein = grassburnfieldlastburn = grassburnfieldlastrecovery = -1;
    loopv(grassburnevents)
    {
        grassburnevents[i].fieldradius = -1;
        grassburnevents[i].lastfieldupdate = -1;
    }
}

static bool setupgrassburnfield()
{
    int limit = max(1, min(grassburnfieldmaxsize, hwtexsize)),
        wanted = max(1, (worldsize + grassburnfieldtexelsize - 1)/grassburnfieldtexelsize),
        dimension = min(wanted, limit);
    if(grassburnfieldtex && grassburnfielddim == dimension && grassburnfieldworldsize == worldsize) return true;

    cleargrassburnfield();
    grassburnfielddim = dimension;
    grassburnfieldworldsize = worldsize;
    grassburnfieldcellsize = worldsize/float(grassburnfielddim);
    grassburnfieldtiles = (grassburnfielddim + GRASSBURNFIELDTILESIZE - 1)/GRASSBURNFIELDTILESIZE;
    int dirtytiles = grassburnfieldtiles*grassburnfieldtiles;
    uchar *dirty = grassburnfielddirty.pad(dirtytiles);
    memset(dirty, 0, dirtytiles*sizeof(uchar));

    size_t bytes = size_t(grassburnfielddim)*grassburnfielddim*2;
    uchar *empty = new uchar[bytes];
    memset(empty, 0, bytes);
    glGenTextures(1, &grassburnfieldtex);
    glActiveTexture_(GL_TEXTURE3);
    createtexture(grassburnfieldtex, grassburnfielddim, grassburnfielddim, empty, 3, 1, GL_RG8, GL_TEXTURE_2D, 0, 0, 0, false, GL_RG);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glActiveTexture_(GL_TEXTURE0);
    delete[] empty;
    return true;
}

static void dirtygrassburnfield(const grassburnevent &event, float radius)
{
    if(!grassburnfieldtex || !grassburnfielddim || grassburnfielddirty.empty()) return;
    if(radius < 0) radius = event.fieldradius >= 0 ? event.fieldradius : grassburneventfieldradius(event);
    float reach = radius + 2*grassburnfieldcellsize;
    int x1 = clamp(int(floorf((event.center.x - reach)/grassburnfieldcellsize)), 0, grassburnfielddim),
        x2 = clamp(int(ceilf((event.center.x + reach)/grassburnfieldcellsize)), 0, grassburnfielddim),
        y1 = clamp(int(floorf((event.center.y - reach)/grassburnfieldcellsize)), 0, grassburnfielddim),
        y2 = clamp(int(ceilf((event.center.y + reach)/grassburnfieldcellsize)), 0, grassburnfielddim);
    if(x1 >= x2 || y1 >= y2) return;
    int tx1 = x1/GRASSBURNFIELDTILESIZE, tx2 = (x2 - 1)/GRASSBURNFIELDTILESIZE,
        ty1 = y1/GRASSBURNFIELDTILESIZE, ty2 = (y2 - 1)/GRASSBURNFIELDTILESIZE;
    for(int ty = ty1; ty <= ty2; ++ty) for(int tx = tx1; tx <= tx2; ++tx)
        grassburnfielddirty[ty*grassburnfieldtiles + tx] = 1;
}

static inline ullong grassburneventcellkey(int x, int y)
{
    return (ullong(uint(x)) << 32) | uint(y);
}

static void buildgrassburneventgrid()
{
    enumerate(grassburneventgrid, vector<int>, bucket, bucket.setsize(0));
    grassburneventgrid.recycle();
    grassburneventqueryversion = 0;

    int cells = max((worldsize + GRASSBURNEVENTCELLSIZE - 1)/GRASSBURNEVENTCELLSIZE, 1);
    loopv(grassburnevents)
    {
        grassburnevent &event = grassburnevents[i];
        event.queryversion = 0;
        float reach = grassburneventfieldradius(event) + 2*grassburnfieldcellsize;
        int x1 = clamp(int(floorf((event.center.x - reach)/GRASSBURNEVENTCELLSIZE)), 0, cells - 1),
            x2 = clamp(int(floorf((event.center.x + reach)/GRASSBURNEVENTCELLSIZE)), 0, cells - 1),
            y1 = clamp(int(floorf((event.center.y - reach)/GRASSBURNEVENTCELLSIZE)), 0, cells - 1),
            y2 = clamp(int(floorf((event.center.y + reach)/GRASSBURNEVENTCELLSIZE)), 0, cells - 1);
        for(int y = y1; y <= y2; ++y) for(int x = x1; x <= x2; ++x)
            grassburneventgrid[grassburneventcellkey(x, y)].add(i);
    }
}

static void collectgrassburneventcandidates(float x1, float y1, float x2, float y2, vector<int> &candidates)
{
    candidates.setsize(0);
    if(!grassburneventgrid.numelems) return;

    int cells = max((worldsize + GRASSBURNEVENTCELLSIZE - 1)/GRASSBURNEVENTCELLSIZE, 1),
        cx1 = clamp(int(floorf(x1/GRASSBURNEVENTCELLSIZE)), 0, cells - 1),
        cx2 = clamp(int(floorf(x2/GRASSBURNEVENTCELLSIZE)), 0, cells - 1),
        cy1 = clamp(int(floorf(y1/GRASSBURNEVENTCELLSIZE)), 0, cells - 1),
        cy2 = clamp(int(floorf(y2/GRASSBURNEVENTCELLSIZE)), 0, cells - 1);
    uint queryversion = ++grassburneventqueryversion;
    for(int y = cy1; y <= cy2; ++y) for(int x = cx1; x <= cx2; ++x)
    {
        vector<int> *bucket = grassburneventgrid.access(grassburneventcellkey(x, y));
        if(!bucket) continue;
        loopv(*bucket)
        {
            int eventindex = (*bucket)[i];
            grassburnevent &event = grassburnevents[eventindex];
            if(event.queryversion == queryversion) continue;
            event.queryversion = queryversion;
            candidates.add(eventindex);
        }
    }
}

static float grassburnfieldjitter(int x, int y, const grassburnevent &event)
{
    uint seed = uint(event.seed*0x1000000u), hash = grasshashcombine(grasshashcombine(seed, uint(x)), uint(y));
    return (hash & 0xFFFFFFu)/float(0x7FFFFFu) - 1.0f;
}

static void evaluategrassburnfield(int x, int y, const vector<int> &eventindices, float &burndamage, float &burnglow)
{
    float worldx = (x + 0.5f)*grassburnfieldcellsize,
          worldy = (y + 0.5f)*grassburnfieldcellsize,
          feather = max(grassburnfieldcellsize*1.5f, 0.001f);
    burndamage = burnglow = 0;
    loopv(eventindices)
    {
        const grassburnevent &event = grassburnevents[eventindices[i]];
        float offsetx = worldx - event.center.x, offsety = worldy - event.center.y,
              angle = atan2f(offsety, offsetx),
              contour = 0.55f*sinf(angle*5.0f + event.seed*2.0f*M_PI) +
                        0.30f*sinf(angle*9.0f - event.seed*4.976f) + 0.15f*grassburnfieldjitter(x, y, event),
              radius = max(event.radius*(1.0f + grassburnvariation*contour) + grassburnpadding, 0.0f),
              distance = sqrtf(offsetx*offsetx + offsety*offsety),
              edge = clamp((radius - distance)/feather + 0.5f, 0.0f, 1.0f);
        if(radius <= 0 || edge <= 0) continue;

        float recovery = lastmillis <= event.recoverstart ? 0.0f :
                         min((lastmillis - event.recoverstart)/float(grassburnfademillis), 1.0f),
              ignitiondelay = min(distance/radius, 1.0f)*grassburnpropagatemillis,
              burnage = max(float(lastmillis - event.burnstart) - ignitiondelay, 0.0f),
              ignition = clamp(burnage/float(max(grassburnfadeinmillis, 1)), 0.0f, 1.0f),
              glowphase = clamp(burnage/float(max(grassburnmillis, 1)), 0.0f, 1.0f),
              strength = (1.0f - recovery)*ignition*edge;
        burndamage = max(burndamage, strength);
        burnglow = max(burnglow, strength*(1.0f - fabsf(2.0f*glowphase - 1.0f)));
    }
}

static bool grassburnfieldsettingschanged()
{
    return grassburnfieldlastpadding != grassburnpadding || grassburnfieldlastvariation != grassburnvariation ||
           grassburnfieldlastpropagation != grassburnpropagatemillis || grassburnfieldlastfadein != grassburnfadeinmillis ||
           grassburnfieldlastburn != grassburnmillis || grassburnfieldlastrecovery != grassburnfademillis;
}

static void updategrassburnfield()
{
    grassburnfielddirtytiles = grassburnfieldtexelsupdated = 0;
    if(!grassburnfieldtex && grassburnevents.empty())
    {
        grassburneventsactive = 0;
        return;
    }
    if(!setupgrassburnfield()) return;

    bool settingschanged = grassburnfieldsettingschanged();
    loopvrev(grassburnevents) if(lastmillis - grassburnevents[i].recoverstart >= grassburnfademillis)
    {
        dirtygrassburnfield(grassburnevents[i]);
        grassburnevents.removeunordered(i);
    }

    loopv(grassburnevents)
    {
        grassburnevent &event = grassburnevents[i];
        float radius = grassburneventfieldradius(event);
        if(settingschanged && event.fieldradius >= 0 && event.fieldradius != radius) dirtygrassburnfield(event, event.fieldradius);
        int propagationend = event.burnstart + grassburnpropagatemillis + max(grassburnfadeinmillis, grassburnmillis);
        bool changing = event.lastfieldupdate != lastmillis && (lastmillis <= propagationend || lastmillis >= event.recoverstart),
             crossedphase = event.lastfieldupdate < 0 ||
                            (event.lastfieldupdate < propagationend && lastmillis > propagationend) ||
                            (event.lastfieldupdate < event.recoverstart && lastmillis >= event.recoverstart);
        if(settingschanged || changing || crossedphase) dirtygrassburnfield(event, radius);
        event.fieldradius = radius;
        event.lastfieldupdate = lastmillis;
    }
    grassburneventsactive = grassburnevents.length();
    buildgrassburneventgrid();

    grassburnfieldlastpadding = grassburnpadding;
    grassburnfieldlastvariation = grassburnvariation;
    grassburnfieldlastpropagation = grassburnpropagatemillis;
    grassburnfieldlastfadein = grassburnfadeinmillis;
    grassburnfieldlastburn = grassburnmillis;
    grassburnfieldlastrecovery = grassburnfademillis;

    bool anydirty = false;
    loopv(grassburnfielddirty) if(grassburnfielddirty[i]) { anydirty = true; break; }
    if(!anydirty) return;

    uchar pixels[GRASSBURNFIELDTILESIZE*GRASSBURNFIELDTILESIZE*2];
    vector<int> eventindices;
    glActiveTexture_(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, grassburnfieldtex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for(int ty = 0; ty < grassburnfieldtiles; ++ty) for(int tx = 0; tx < grassburnfieldtiles; ++tx)
    {
        int tileindex = ty*grassburnfieldtiles + tx;
        if(!grassburnfielddirty[tileindex]) continue;
        int x1 = tx*GRASSBURNFIELDTILESIZE, y1 = ty*GRASSBURNFIELDTILESIZE,
            width = min(int(GRASSBURNFIELDTILESIZE), grassburnfielddim - x1),
            height = min(int(GRASSBURNFIELDTILESIZE), grassburnfielddim - y1);
        float worldx1 = x1*grassburnfieldcellsize, worldx2 = (x1 + width)*grassburnfieldcellsize,
              worldy1 = y1*grassburnfieldcellsize, worldy2 = (y1 + height)*grassburnfieldcellsize;
        collectgrassburneventcandidates(worldx1, worldy1, worldx2, worldy2, eventindices);
        for(int j = 0; j < eventindices.length();)
        {
            const grassburnevent &event = grassburnevents[eventindices[j]];
            float reach = grassburneventfieldradius(event) + 2*grassburnfieldcellsize;
            if(event.center.x + reach <= worldx1 || event.center.x - reach >= worldx2 ||
               event.center.y + reach <= worldy1 || event.center.y - reach >= worldy2) eventindices.removeunordered(j);
            else ++j;
        }

        for(int y = 0; y < height; ++y) for(int x = 0; x < width; ++x)
        {
            float damage, glow;
            evaluategrassburnfield(x1 + x, y1 + y, eventindices, damage, glow);
            int offset = (y*width + x)*2;
            pixels[offset] = uchar(clamp(damage, 0.0f, 1.0f)*255 + 0.5f);
            pixels[offset + 1] = uchar(clamp(glow, 0.0f, 1.0f)*255 + 0.5f);
        }
        glTexSubImage2D(GL_TEXTURE_2D, 0, x1, y1, width, height, GL_RG, GL_UNSIGNED_BYTE, pixels);
        grassburnfielddirty[tileindex] = 0;
        grassburnfielddirtytiles++;
        grassburnfieldtexelsupdated += width*height;
    }
    glActiveTexture_(GL_TEXTURE0);
}

struct grasspatchkey
{
    int x, y;
    ushort texture, blend;

    grasspatchkey() {}
    grasspatchkey(int x, int y, ushort texture, ushort blend) : x(x), y(y), texture(texture), blend(blend) {}
};

static inline bool htcmp(const grasspatchkey &x, const grasspatchkey &y)
{
    return x.x == y.x && x.y == y.y && x.texture == y.texture && x.blend == y.blend;
}

static inline uint hthash(const grasspatchkey &key)
{
    uint hash = grasshashcombine(uint(key.x), uint(key.y));
    hash = grasshashcombine(hash, uint(key.texture));
    return grasshashcombine(hash, uint(key.blend));
}

struct grasscellkey
{
    int patch, x, y;
    uint seed;

    grasscellkey() {}
    grasscellkey(int patch, int x, int y, uint seed) : patch(patch), x(x), y(y), seed(seed) {}
};

static inline bool htcmp(const grasscellkey &x, const grasscellkey &y)
{
    return x.patch == y.patch && x.x == y.x && x.y == y.y && x.seed == y.seed;
}

static inline uint hthash(const grasscellkey &key)
{
    uint hash = grasshashcombine(uint(key.patch), uint(key.x));
    hash = grasshashcombine(hash, uint(key.y));
    return grasshashcombine(hash, key.seed);
}

struct grassbuildpatch
{
    grasspatchkey key;
    Slot *slot;
    ivec blendpos;
    vector<grassinstance> instances;
    vec bbmin, bbmax;
    vec4 particlepositions[MAXGRASSPATCHPARTICLEPOSITIONS];
    int numparticlecandidates, sourcetris;

    grassbuildpatch(const grasspatchkey &key, Slot *slot, const ivec &blendpos) :
        key(key), slot(slot), blendpos(blendpos), bbmin(1e16f, 1e16f, 1e16f), bbmax(-1e16f, -1e16f, -1e16f), numparticlecandidates(0), sourcetris(0)
    {
    }
};

static float grasshashunit(uint n)
{
    return (grasshash(n) & 0xFFFFFFu) / float(0x1000000u);
}

static uint grasssurfaceseed(const vtxarray &va, const grasstri &g)
{
    uint seed = 0xA341316Cu;
    seed = grasshashcombine(seed, uint(va.o.x));
    seed = grasshashcombine(seed, uint(va.o.y));
    seed = grasshashcombine(seed, uint(va.o.z));
    seed = grasshashcombine(seed, uint(va.size));
    seed = grasshashcombine(seed, uint(g.texture));
    seed = grasshashcombine(seed, uint(int(g.surface.x*65536.0f)));
    seed = grasshashcombine(seed, uint(int(g.surface.y*65536.0f)));
    seed = grasshashcombine(seed, uint(int(g.surface.z*65536.0f)));
    return grasshashcombine(seed, uint(int(g.surface.offset*8.0f)));
}

static bool insidegrasstri(const grasstri &g, float x, float y, float &edgedist)
{
    bool positive = false, negative = false;
    edgedist = 1e16f;
    loopi(g.numv)
    {
        const vec &a = g.v[i], &b = g.v[(i + 1)%g.numv];
        float dx = b.x - a.x, dy = b.y - a.y,
              side = dx*(y - a.y) - dy*(x - a.x),
              edgelen = sqrtf(dx*dx + dy*dy);
        if(side > 1e-4f) positive = true;
        else if(side < -1e-4f) negative = true;
        if(positive && negative) return false;
        if(edgelen > 1e-6f) edgedist = min(edgedist, fabsf(side)/edgelen);
    }
    return true;
}

void destroygrass(vtxarray *va)
{
    if(va->grassbuf)
    {
        glDeleteBuffers_(1, &va->grassbuf);
        va->grassbuf = 0;
    }
    va->grasspatches.setsize(0);
}

void buildgrass(vtxarray *va)
{
    destroygrass(va);
    if(glversion < 400 || !glDrawElementsInstanced_ || !glVertexAttribDivisor_ || va->grasstris.empty()) return;

    vector<grassinstance> instances;
    vector<grassbuildpatch *> buildpatches;
    hashtable<grasspatchkey, int> patchindices;
    hashset<grasscellkey> generatedcells(1<<16);
    const float spacing = grassstep/sqrtf(grassdensity), patchsize = float(grasspatchsize);
    int numinstances = 0;
    bool full = false;
    BlendMapCache *blendcache = NULL;
    loopv(va->grasstris) if(va->grasstris[i].blend)
    {
        blendcache = newblendmapcache();
        setblendmaporigin(blendcache, va->o, va->size);
        break;
    }

    loopv(va->grasstris)
    {
        const grasstri &g = va->grasstris[i];
        Slot *slot = lookupvslot(g.texture, false).slot;
        float minx = g.v[0].x, maxx = minx, miny = g.v[0].y, maxy = miny;
        loopj(g.numv)
        {
            minx = min(minx, g.v[j].x);
            maxx = max(maxx, g.v[j].x);
            miny = min(miny, g.v[j].y);
            maxy = max(maxy, g.v[j].y);
        }

        int minpatchx = int(floorf(minx/patchsize)), maxpatchx = int(floorf(maxx/patchsize)),
            minpatchy = int(floorf(miny/patchsize)), maxpatchy = int(floorf(maxy/patchsize));
        uint seed = grasssurfaceseed(*va, g);

        for(int py = minpatchy; py <= maxpatchy && !full; ++py) for(int px = minpatchx; px <= maxpatchx && !full; ++px)
        {
            float patchminx = max(minx, px*patchsize), patchmaxx = min(maxx, (px + 1)*patchsize),
                  patchminy = max(miny, py*patchsize), patchmaxy = min(maxy, (py + 1)*patchsize);
            if(patchminx >= patchmaxx || patchminy >= patchmaxy) continue;

            grasspatchkey key(px, py, g.texture, g.blend);
            int *patchindex = patchindices.access(key);
            if(!patchindex)
            {
                int index = buildpatches.length();
                patchindices.access(key, index);
                buildpatches.add(new grassbuildpatch(key, slot, ivec(g.center)));
                patchindex = patchindices.access(key);
            }
            grassbuildpatch &patch = *buildpatches[*patchindex];
            int start = patch.instances.length(),
                mingridx = int(floorf(patchminx/spacing)) - 1, maxgridx = int(floorf(patchmaxx/spacing)),
                mingridy = int(floorf(patchminy/spacing)) - 1, maxgridy = int(floorf(patchmaxy/spacing));

            for(int gy = mingridy; gy <= maxgridy && !full; ++gy) for(int gx = mingridx; gx <= maxgridx; ++gx)
            {
                uint cellseed = grasshashcombine(grasshashcombine(seed, uint(gx)), uint(gy));
                float x = (gx + grasshashunit(cellseed))*spacing,
                      y = (gy + grasshashunit(cellseed ^ 0xB5297A4Du))*spacing;
                if(x < patchminx || x >= patchmaxx || y < patchminy || y >= patchmaxy) continue;

                float edgedist;
                if(!insidegrasstri(g, x, y, edgedist)) continue;

                grasscellkey cell(*patchindex, gx, gy, seed);
                if(generatedcells.access(cell)) continue;
                generatedcells.add(cell);

                float z = g.surface.zintersect(vec(x, y, 0));
                grassinstance &inst = patch.instances.add();
                float variation = grasshashunit(cellseed ^ 0x68E31DA4u);
                inst.originangle = vec4(x, y, z, grasshashunit(cellseed ^ 0x1B56C4E9u)*2*M_PI);
                inst.variation = vec4(0.8f + 0.4f*variation, grasshashunit(cellseed ^ 0xC2B2AE35u), edgedist, g.surface.z);
                patch.bbmin.min(vec(x, y, z));
                patch.bbmax.max(vec(x, y, z));
                numinstances++;
                if(numinstances >= grassmaxinstances) full = true;
            }

            int count = patch.instances.length() - start;
            if(!count) continue;
            patch.sourcetris++;
            loopj(count)
            {
                const grassinstance &inst = patch.instances[start + j];
                const vec4 &candidate = inst.originangle;
                if(g.blend && blendcache && lookupblendmap(blendcache, vec(candidate))/255.0f <= grasstest) continue;
                int selected = patch.numparticlecandidates < MAXGRASSPATCHPARTICLEPOSITIONS ? patch.numparticlecandidates :
                               int(grasshashcombine(seed, uint(j))%uint(patch.numparticlecandidates + 1));
                patch.numparticlecandidates++;
                if(selected < MAXGRASSPATCHPARTICLEPOSITIONS)
                    patch.particlepositions[selected] = vec4(vec(candidate), inst.variation.y);
            }
        }
        if(full) break;
    }

    freeblendmapcache(blendcache);
    loopv(buildpatches)
    {
        grassbuildpatch &build = *buildpatches[i];
        if(build.instances.empty()) continue;

        grasspatch &patch = va->grasspatches.add();
        patch.center = vec(build.bbmin).add(build.bbmax).mul(0.5f);
        patch.radius = patch.center.dist(build.bbmax);
        patch.offset = instances.length();
        patch.count = build.instances.length();
        patch.sourcetris = build.sourcetris;
        patch.texture = build.key.texture;
        patch.blend = build.key.blend;
        patch.slot = build.slot;
        patch.blendpos = build.blendpos;
        patch.numparticlepositions = min(build.numparticlecandidates, int(MAXGRASSPATCHPARTICLEPOSITIONS));
        loopj(patch.numparticlepositions) patch.particlepositions[j] = build.particlepositions[j];
        patch.shadowcount[0] = patch.shadowcount[1] = 0;
        loopvj(build.instances)
        {
            float instancehash = build.instances[j].variation.y;
            if(instancehash <= grassshadowdensitynear) patch.shadowcount[0]++;
            if(instancehash <= grassshadowdensity) patch.shadowcount[1]++;
        }
        instances.move(build.instances);
    }
    buildpatches.deletecontents();
    if(instances.empty()) return;
    glGenBuffers_(1, &va->grassbuf);
    gle::bindvbo(va->grassbuf);
    glBufferData_(GL_ARRAY_BUFFER, instances.length()*sizeof(grassinstance), instances.getbuf(), GL_STATIC_DRAW);
    gle::clearvbo();
}

static void grasssettingschanged()
{
    loopv(valist) if(!valist[i]->grasstris.empty()) buildgrass(valist[i]);
}

static void addgrassplane(vector<grassmeshvert> &verts, vector<ushort> &indices, float angle, int segments)
{
    int base = verts.length();
    float dx = cosf(angle), dy = sinf(angle);
    for(int row = 0; row <= segments; ++row)
    {
        float z = row/float(segments);
        grassmeshvert &left = verts.add(), &right = verts.add();
        left.pos = vec(-0.5f*dx, -0.5f*dy, z);
        left.tc = vec2(0, 1 - z);
        right.pos = vec(0.5f*dx, 0.5f*dy, z);
        right.tc = vec2(1, 1 - z);
    }
    loopi(segments)
    {
        int row = base + 2*i;
        indices.add(ushort(row));
        indices.add(ushort(row + 1));
        indices.add(ushort(row + 3));
        indices.add(ushort(row));
        indices.add(ushort(row + 3));
        indices.add(ushort(row + 2));
    }
}

static void addgrassmesh(vector<grassmeshvert> &verts, vector<ushort> &indices, int lod, int planes, int segments)
{
    grassmesh &mesh = grassmeshes[lod];
    mesh.offset = indices.length();
    int startverts = verts.length();
    loopi(planes) addgrassplane(verts, indices, M_PI*i/float(planes), segments);
    mesh.count = indices.length() - mesh.offset;
    mesh.verts = verts.length() - startverts;
}

static void initgrassmeshes()
{
    if(grassmeshvbo) return;
    vector<grassmeshvert> verts;
    vector<ushort> indices;
    addgrassmesh(verts, indices, 0, 3, 2);
    addgrassmesh(verts, indices, 1, 2, 1);

    glGenBuffers_(1, &grassmeshvbo);
    gle::bindvbo(grassmeshvbo);
    glBufferData_(GL_ARRAY_BUFFER, verts.length()*sizeof(grassmeshvert), verts.getbuf(), GL_STATIC_DRAW);
    glGenBuffers_(1, &grassmeshebo);
    gle::bindebo(grassmeshebo);
    glBufferData_(GL_ELEMENT_ARRAY_BUFFER, indices.length()*sizeof(ushort), indices.getbuf(), GL_STATIC_DRAW);
    gle::clearvbo();
    gle::clearebo();
}

Shader *loadgrassshader()
{
    return generateshader("grass", "grassshader");
}

void loadgrassshaders()
{
    if(glversion < 400) return;
    grassshader = loadgrassshader();
    grassimpulseshader = generateshader("grassimpulse", "grassimpulseshader");
    grassburnshader = generateshader("grassburn", "grassburnshader");
    grassburnimpulseshader = generateshader("grassburnimpulse", "grassburnimpulseshader");
    grassshadowshader = generateshader("grassshadow", "grassshadowshader");
}

void cleargrassshaders()
{
    grassshader = grassimpulseshader = grassburnshader = grassburnimpulseshader = grassshadowshader = NULL;
}

static void setupgrassattribs()
{
    gle::bindvbo(grassmeshvbo);
    const grassmeshvert *vert = 0;
    glVertexAttribPointer_(gle::ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, sizeof(grassmeshvert), vert->pos.v);
    glVertexAttribPointer_(gle::ATTRIB_TEXCOORD0, 2, GL_FLOAT, GL_FALSE, sizeof(grassmeshvert), vert->tc.v);
    glEnableVertexAttribArray_(gle::ATTRIB_VERTEX);
    glEnableVertexAttribArray_(gle::ATTRIB_TEXCOORD0);
    glVertexAttribDivisor_(gle::ATTRIB_VERTEX, 0);
    glVertexAttribDivisor_(gle::ATTRIB_TEXCOORD0, 0);
    gle::bindebo(grassmeshebo);
}

static void bindgrassinstances(vtxarray *va, int offset)
{
    gle::bindvbo(va->grassbuf);
    const grassinstance *inst = (const grassinstance *)(size_t(offset)*sizeof(grassinstance));
    glVertexAttribPointer_(gle::ATTRIB_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(grassinstance), inst->originangle.v);
    glVertexAttribPointer_(gle::ATTRIB_TEXCOORD1, 4, GL_FLOAT, GL_FALSE, sizeof(grassinstance), inst->variation.v);
    glEnableVertexAttribArray_(gle::ATTRIB_COLOR);
    glEnableVertexAttribArray_(gle::ATTRIB_TEXCOORD1);
    glVertexAttribDivisor_(gle::ATTRIB_COLOR, 1);
    glVertexAttribDivisor_(gle::ATTRIB_TEXCOORD1, 1);
}

static void cleanupgrassattribs()
{
    glVertexAttribDivisor_(gle::ATTRIB_COLOR, 0);
    glVertexAttribDivisor_(gle::ATTRIB_TEXCOORD1, 0);
    glDisableVertexAttribArray_(gle::ATTRIB_VERTEX);
    glDisableVertexAttribArray_(gle::ATTRIB_COLOR);
    glDisableVertexAttribArray_(gle::ATTRIB_TEXCOORD0);
    glDisableVertexAttribArray_(gle::ATTRIB_TEXCOORD1);
    gle::clearvbo();
    gle::clearebo();
}

static void setgrassframeparams()
{
    float angle = grasswindangle*RAD,
          windphase = fmodf(float(lastmillis), float(max(grassanimmillis, 1)))/float(max(grassanimmillis, 1));
    GLOBALPARAMF(grasswindparams, windphase, grassanimscale, cosf(angle), sinf(angle));
    GLOBALPARAMF(grasswindspatial, grasswindscale, grassmargin, grassmarginfade, 0.0f);
    GLOBALPARAMF(grassimpulsecontrol, grassimpulsestrength, grassimpulsewobble, grassimpulsewobblespeed, 0.0f);
    bvec color(grasscolour);
    GLOBALPARAMF(grasscolourparams, color.x/255.0f, color.y/255.0f, color.z/255.0f, 1.0f);
    GLOBALPARAMF(grasstest, grasstest);
}

static void setgrassburnframeparams()
{
    bvec burncolor(grassburncolour);
    GLOBALPARAMF(grassburncolourparams, burncolor.x/255.0f, burncolor.y/255.0f, burncolor.z/255.0f);
    bvec burnglowcolor(grassburnglowcolour);
    GLOBALPARAMF(grassburnglowcolourparams, burnglowcolor.x/255.0f, burnglowcolor.y/255.0f, burnglowcolor.z/255.0f);
    float burntime = lastmillis/1000.0f;
    GLOBALPARAMF(grassburnanimparams, fmodf(burntime*grassburnscrollx, 1.0f), fmodf(burntime*grassburnscrolly, 1.0f), grassburnglowscale);
    GLOBALPARAMF(grassburnfieldparams, 1.0f/float(max(worldsize, 1)), 1.0f/float(max(worldsize, 1)), 0.0f, 0.0f);
}

static void setgrassdrawparams(Texture *tex, float density, float fade, float windscale)
{
    float width = grassheight*tex->xs/float(max(grassscale*tex->ys, 1)),
          taperstart = grassdist*grasstaper;
    LOCALPARAMF(grassmeshparams, width, float(grassheight), windscale, 0.0f);
    LOCALPARAMF(grassdrawparams, density, fade, float(grassdist), taperstart);
}

static int grassburnparticleframe = -1, grassburnparticleemits = 0;

static float grassburnignition(const vec4 &candidate, const grassburnevent &event)
{
    vec offset = vec(candidate).sub(event.center);
    float angle = atan2f(offset.y, offset.x),
          jitter = sinf(candidate.w*91.713f + event.seed*37.119f)*43758.5453f;
    jitter = (jitter - floorf(jitter))*2.0f - 1.0f;
    float contour = 0.55f*sinf(angle*5.0f + event.seed*2.0f*M_PI) +
                    0.30f*sinf(angle*9.0f - event.seed*4.976f) + 0.15f*jitter,
          radius = max(event.radius*(1.0f + grassburnvariation*contour) + grassburnpadding, 0.0f),
          distance = offset.magnitude();
    if(radius <= 0 || distance > radius) return 0;
    float ignitiondelay = distance/radius*grassburnpropagatemillis;
    return clamp((lastmillis - event.burnstart - ignitiondelay)/float(grassburnfadeinmillis), 0.0f, 1.0f);
}

static bool findgrassburnparticleposition(vtxarray *vas, const grassburnevent &event, vec &result)
{
    int matches = 0;
    float maxradius = max(event.radius*(1 + grassburnvariation) + grassburnpadding, 0.0f);
    for(vtxarray *va = vas; va; va = va->next)
    {
        if(!va->grassbuf || va->grasspatches.empty() || va->occluded >= OCCLUDE_GEOM || va->distance > grassdist) continue;
        loopv(va->grasspatches)
        {
            const grasspatch &patch = va->grasspatches[i];
            float radius = patch.radius + grassheight,
                  dist = max(camera1->o.dist(patch.center) - patch.radius, 0.0f),
                  reach = patch.radius + maxradius;
            if(dist > grassdist || isfoggedsphere(radius, patch.center) || patch.center.squaredist(event.center) > reach*reach) continue;

            loopk(patch.numparticlepositions)
            {
                const vec4 &candidate = patch.particlepositions[k];
                if(grassburnignition(candidate, event) <= 0) continue;
                if(!rnd(++matches)) result = vec(candidate);
            }
        }
    }
    return matches > 0;
}

static void emitgrassburnparticles(vtxarray *vas)
{
    if(!grassburnparticles || !canemitparticles()) return;
    if(grassburnparticleframe != lastmillis)
    {
        grassburnparticleframe = lastmillis;
        grassburnparticleemits = 0;
    }

    loopv(grassburnevents)
    {
        if(grassburnparticleemits >= grassburnparticlemax) return;
        grassburnevent &event = grassburnevents[i];
        if(lastmillis < event.burnstart || lastmillis >= event.recoverstart || lastmillis >= event.particleend ||
           lastmillis - event.lastemit < grassburnparticlemillis) continue;

        float maxradius = max(event.radius*(1 + grassburnvariation) + grassburnpadding, 0.0f);
        if(maxradius <= 0) continue;

        event.lastemit = lastmillis;
        vec grassorigin;
        if(!findgrassburnparticleposition(vas, event, grassorigin)) continue;
        vec origin = grassorigin;
        origin.z += 1.25f;

        particle_splash(PART_SMOKE, 1, 1200, origin, 0x20202088, 1.0f, 1, -150, 15);
        particle_splash(PART_HAZE_SMALL, 1, 800, origin, 60, 8.0f, 1, 0, 1);
        float sparkspread = min(grassburnsparkspread, maxradius);
        loopk(2)
        {
            float angle = rndscale(2*M_PI), radius = sqrtf(rndscale(1))*sparkspread;
            vec sparkorigin = grassorigin;
            sparkorigin.x += cosf(angle)*radius;
            sparkorigin.y += sinf(angle)*radius;
            sparkorigin.z += 1.25f;
            particle_splash(PART_FIRESPARK, 1, 1000, sparkorigin, 0xFF7020, 0.55f, 1, -60, 0);
        }


        grassburnparticleemits++;
    }
}

static bool grasspatchusesburnfield(const grasspatch &patch)
{
    static vector<int> candidates;
    collectgrassburneventcandidates(patch.center.x - patch.radius, patch.center.y - patch.radius,
                                    patch.center.x + patch.radius, patch.center.y + patch.radius, candidates);
    loopv(candidates)
    {
        const grassburnevent &event = grassburnevents[candidates[i]];
        grassburncandidatechecks++;
        float maxradius = grassburneventfieldradius(event) + 2*grassburnfieldcellsize,
              reach = patch.radius + maxradius;
        if(maxradius <= 0) continue;
        if(patch.center.squaredist(event.center) <= reach*reach)
        {
            grassburnrelevantchecks++;
            return true;
        }
    }
    return false;
}

struct grasspatchimpulses
{
    vec4 positions[MAXGRASSIMPULSESPERPATCH], directions[MAXGRASSIMPULSESPERPATCH], params[MAXGRASSIMPULSESPERPATCH];
    int count;

    grasspatchimpulses() : count(0) {}
};

static void collectgrasspatchimpulses(const grasspatch &patch, grasspatchimpulses &selected)
{
    selected.count = 0;
    if(!grassimpulses || grassimpulsestrength <= 0 || grassimpulselist.empty() || !grassimpulsegrid.numelems || !grassimpulsemaxpatch) return;
    static vector<int> candidates;
    collectgrassimpulsecandidates(patch.center.x - patch.radius, patch.center.y - patch.radius,
                                  patch.center.x + patch.radius, patch.center.y + patch.radius, candidates);

    int indices[MAXGRASSIMPULSESPERPATCH], limit = min(grassimpulsemaxpatch, int(MAXGRASSIMPULSESPERPATCH));
    float scores[MAXGRASSIMPULSESPERPATCH];
    loopv(candidates)
    {
        const grassimpulse &impulse = grassimpulselist[candidates[i]];
        grassimpulsecandidatechecks++;
        float reach = patch.radius + grassheight + impulse.radius,
              centerdist = vec2(patch.center).dist(vec2(impulse.position));
        if(centerdist > reach) continue;

        float distance = max(centerdist - patch.radius, 0.0f), relevance = max(1.0f - distance/max(impulse.radius, 0.001f), 0.0f),
              age = max(lastmillis - impulse.starttime, 0)/1000.0f;
        if(impulse.type == GRASS_IMPULSE_EXPLOSION)
        {
            float wave = impulse.propagationspeed > 0 ? age*impulse.propagationspeed : impulse.radius*age*1000.0f/max(impulse.lifetime, 1),
                  frontwidth = max(impulse.radius*impulse.falloff, 2.0f);
            relevance = max(1.0f - fabsf(distance - wave)/(patch.radius + frontwidth), 0.0f);
        }
        float score = grassimpulseremainingstrength(impulse)*relevance;
        if(score <= grassimpulseminstrength) continue;
        grassimpulserelevantchecks++;

        int insert = selected.count;
        if(selected.count < limit) selected.count++;
        else
        {
            if(score <= scores[limit - 1]) continue;
            insert = limit - 1;
        }
        while(insert > 0 && score > scores[insert - 1])
        {
            if(insert < limit)
            {
                scores[insert] = scores[insert - 1];
                indices[insert] = indices[insert - 1];
            }
            --insert;
        }
        scores[insert] = score;
        indices[insert] = candidates[i];
    }

    loopi(selected.count)
    {
        const grassimpulse &impulse = grassimpulselist[indices[i]];
        float ageseconds = max(lastmillis - impulse.starttime, 0)/1000.0f,
              behavior = impulse.type == GRASS_IMPULSE_EXPLOSION ? max(impulse.propagationspeed, 0.001f) : impulse.radial;
        selected.positions[i] = vec4(impulse.position, impulse.radius);
        selected.directions[i] = vec4(impulse.direction, impulse.strength);
        selected.params[i] = vec4(float(impulse.type), ageseconds, behavior, impulse.lifetime/1000.0f);
    }
}

static void setgrassimpulseparams(const grasspatchimpulses &impulses)
{
    LOCALPARAMI(grassimpulsecount, impulses.count);
    LOCALPARAMV(grassimpulsepositions, impulses.positions, impulses.count);
    LOCALPARAMV(grassimpulsedirections, impulses.directions, impulses.count);
    LOCALPARAMV(grassimpulseparams, impulses.params, impulses.count);
}

struct grassrenderstats
{
    int patches, drawcalls, instances, sourcetris;

    grassrenderstats() : patches(0), drawcalls(0), instances(0), sourcetris(0) {}
};

static void drawgrasslod(const grasspatch &patch, Texture *tex, int lod, float density, float fade, float windscale, grassrenderstats *stats,
                         int statinstances = -1)
{
    if(density <= 0 || fade <= 0) return;
    setgrassdrawparams(tex, density, fade, windscale);
    const grassmesh &mesh = grassmeshes[lod];
    glDrawElementsInstanced_(GL_TRIANGLES, mesh.count, GL_UNSIGNED_SHORT, (const void *)(size_t(mesh.offset)*sizeof(ushort)), patch.count);
    xtravertsva += mesh.verts*patch.count;
    glde++;
    if(stats)
    {
        stats->drawcalls++;
        stats->instances += statinstances >= 0 ? statinstances : patch.count;
    }
}

static void drawgrasspatchlod(const grasspatch &patch, Texture *tex, float dist, bool shadow, int cascade, grassrenderstats *stats)
{
    if(shadow)
    {
        int index = clamp(cascade, 0, 1), lod = index ? grassshadowlodfar : grassshadowlodnear;
        float density = index ? grassshadowdensity : grassshadowdensitynear,
              windscale = cascade < grassshadowwindcascades ? 1.0f : 0.0f;
        if(!patch.shadowcount[index]) return;
        drawgrasslod(patch, tex, lod, density, 1, windscale, stats, patch.shadowcount[index]);
        return;
    }

    float transition = min(float(grasslodtransition), float(grasslod1));
    if(transition > 0 && dist >= grasslod1 - transition && dist <= grasslod1 + transition)
    {
        float blend = clamp((dist - (grasslod1 - transition))/(2*transition), 0.0f, 1.0f);
        drawgrasslod(patch, tex, 0, 1, 1 - blend, 1, stats);
        drawgrasslod(patch, tex, 1, grassloddensity1, blend, 1, stats);
        return;
    }

    int lod = dist < grasslod1 ? 0 : 1;
    drawgrasslod(patch, tex, lod, lod ? grassloddensity1 : 1.0f, 1, 1, stats);
}

static void rendergrasspatches(vtxarray *vas, bool shadow, int cascade)
{
    Shader *baseshader = shadow ? grassshadowshader : grassshader;
    if(!baseshader) return;
    grassrenderstats stats;

    setgrassframeparams();
    if(!shadow)
    {
        buildgrassimpulsegrid();
        updategrassburnfield();
        emitgrassburnparticles(vas);
    }
    initgrassmeshes();
    setupgrassattribs();
    glDisable(GL_CULL_FACE);
    glActiveTexture_(GL_TEXTURE0);

    GLuint texid = 0;
    Shader *boundshader = NULL;
    bool texbound = false, burnframeset = false, burntexbound = false;
    int blend = -1;
    for(vtxarray *va = vas; va; va = shadow ? va->rnext : va->next)
    {
        if(!va->grassbuf || va->grasspatches.empty()) continue;
        if(shadow)
        {
            if(!(va->shadowmask&(1<<cascade))) continue;
        }
        else if(va->occluded >= OCCLUDE_GEOM || va->distance > grassdist) continue;

        loopv(va->grasspatches)
        {
            const grasspatch &patch = va->grasspatches[i];
            float radius = patch.radius + grassheight,
                  dist = max(camera1->o.dist(patch.center) - patch.radius, 0.0f);
            if(dist > grassdist) continue;
            if(shadow)
            {
                if(!(calcspherecsmsplits(patch.center, radius)&(1<<cascade))) continue;
                if(!patch.shadowcount[cascade > 0 ? 1 : 0]) continue;
            }
            else if(isvisiblesphere(radius, patch.center) >= VFC_FOGGED) continue;

            grasspatchimpulses patchimpulses;
            if(!shadow) collectgrasspatchimpulses(patch, patchimpulses);
            bool interacting = !shadow && patchimpulses.count > 0 && grassimpulseshader,
                 burning = !shadow && grassburnshader && grassburnfieldtex && !grassburnevents.empty() && grasspatchusesburnfield(patch);
            // Effect selection only changes shader state; patches keep the same instance buffer and draw ranges.
            Shader *patchshader = burning ? (interacting && grassburnimpulseshader ? grassburnimpulseshader : grassburnshader) :
                                  interacting ? grassimpulseshader : baseshader;

            Slot &slot = *patch.slot;
            if(!slot.grasstex)
            {
                if(!slot.grass) continue;
                slot.grasstex = textureload(slot.grass, 2);
            }
            Texture *tex = slot.grasstex;
            if(!texbound || texid != tex->id)
            {
                glBindTexture(GL_TEXTURE_2D, tex->id);
                texid = tex->id;
                texbound = true;
            }

            if(burning && !burnframeset)
            {
                setgrassburnframeparams();
                burnframeset = true;
            }
            if(burning && !burntexbound)
            {
                if(!grassburntex) grassburntex = textureload("media/noise/burning_grass.jpg", 0, true, false);
                glActiveTexture_(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, grassburntex->id);
                glActiveTexture_(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, grassburnfieldtex);
                glActiveTexture_(GL_TEXTURE0);
                burntexbound = true;
            }

            if(boundshader != patchshader || blend != patch.blend)
            {
                if(patch.blend)
                {
                    glActiveTexture_(GL_TEXTURE1);
                    bindblendtexture(patch.blendpos);
                    glActiveTexture_(GL_TEXTURE0);
                    patchshader->setvariant(0, 0);
                }
                else patchshader->set();
                boundshader = patchshader;
                blend = patch.blend;
            }
            if(interacting)
            {
                setgrassimpulseparams(patchimpulses);
                grassimpulsepatchesaffected++;
            }

            if(!shadow)
            {
                if(burning) grassburnpatchesaffected++;
                stats.patches++;
                stats.sourcetris += patch.sourcetris;
            }
            bindgrassinstances(va, patch.offset);
            drawgrasspatchlod(patch, tex, dist, shadow, cascade, &stats);
        }
    }

    cleanupgrassattribs();
    glEnable(GL_CULL_FACE);
    if(!shadow)
    {
        grassvisiblepatches = stats.patches;
        grassdrawcalls = stats.drawcalls;
        grassinstancesrendered = stats.instances;
        grassmergedtris = stats.sourcetris;
    }
    else
    {
        grassshadowdraws += stats.drawcalls;
        if(cascade > 0) grassshadowinstances1 += stats.instances;
        else grassshadowinstances0 += stats.instances;
    }
}

static int lastgrassstatprint = 0;

void rendergrass()
{
    prunegrassimpulses();
    grassvisiblepatches = grassdrawcalls = grassinstancesrendered = grassmergedtris = 0;
    grassshadowdraws = grassshadowinstances0 = grassshadowinstances1 = 0;
    grassburnfielddirtytiles = grassburnfieldtexelsupdated = 0;
    grassburneventsactive = grassburnevents.length();
    grassburncandidatechecks = grassburnrelevantchecks = grassburnpatchesaffected = 0;
    grassimpulsesactive = grassimpulselist.length();
    grassimpulsecandidatechecks = grassimpulserelevantchecks = grassimpulsepatchesaffected = 0;
    if(!grass || !grassdist || glversion < 400 || !glDrawElementsInstanced_ || !visibleva) return;
    timer *grasscputimer = begintimer("grass", false), *grasstimer = begintimer("grass");
    rendergrasspatches(visibleva, false, 0);
    endtimer(grasstimer);
    endtimer(grasscputimer);
    if(grassstats && totalmillis - lastgrassstatprint >= 1000)
    {
        lastgrassstatprint = totalmillis - totalmillis%1000;
        conoutf(CON_INFO, "grass: %d visible patches, %d draw calls, %d instances, %d source triangle contributions, "
                         "%d impulses, %d impulse candidate checks, %d impulse relevant checks, %d impulse patches, "
                         "%d burn events, %d burn candidate checks, %d burn relevant checks, %d fire patches, "
                         "%d burn field dirty tiles, %d burn field texels updated",
                grassvisiblepatches, grassdrawcalls, grassinstancesrendered, grassmergedtris,
                grassimpulsesactive, grassimpulsecandidatechecks, grassimpulserelevantchecks, grassimpulsepatchesaffected,
                grassburneventsactive, grassburncandidatechecks, grassburnrelevantchecks, grassburnpatchesaffected,
                grassburnfielddirtytiles, grassburnfieldtexelsupdated);
    }
}

void rendergrassshadow(int cascade)
{
    if(!grass || !grassdist || cascade < 0 || cascade >= grassshadowcascades || glversion < 400 || !glDrawElementsInstanced_ || !shadowva) return;
    rendergrasspatches(shadowva, true, cascade);
}

void cleanupgrass()
{
    cleargrassimpulses();
    cleargrassburnevents();
    if(grassmeshvbo)
    {
        glDeleteBuffers_(1, &grassmeshvbo);
        grassmeshvbo = 0;
    }
    if(grassmeshebo)
    {
        glDeleteBuffers_(1, &grassmeshebo);
        grassmeshebo = 0;
    }
    cleargrassshaders();
}
