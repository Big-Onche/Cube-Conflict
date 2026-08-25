#include "engine.h"

static void grasssettingschanged();

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
FVAR(grassshadowdensity, 0.01f, 1, 1);

VAR(grassanimmillis, 1, 3000, 60000);
FVAR(grassanimscale, 0, 0.2f, 1);
FVAR(grasswindangle, 0, 0, 360);
FVAR(grasswindscale, 0, 0.015f, 1);
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
VAR(grassburncarrierinterval, 16, 100, 1000);
FVAR(grassburncarrierstep, 0, 4, 64);

enum { MAXGRASSDAMAGEPARAMS = 64, MAXGRASSBURNEVENTS = 4096 };

static uint grasshash(uint n);
static uint grassdamageseed = 0;

struct grassburnevent
{
    vec center;
    float radius, seed;
    int burnstart, recoverstart, particleend, lastemit;
};

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
    if(grassburnevents.length() >= MAXGRASSBURNEVENTS) grassburnevents.remove(0);
    grassburnevent &event = grassburnevents.add();
    event.center = center;
    event.radius = radius;
    event.seed = (grasshash(++grassdamageseed) & 0xFFFFFFu)/float(0x1000000u);
    event.burnstart = lastmillis;
    event.recoverstart = lastmillis + grassburnpropagatemillis + max(grassburnfadeinmillis, grassburnmillis) +
                         (lifetime >= 0 ? lifetime : grassburnholdmillis);
    int minparticlemillis = min(grassburnparticleminmillis, grassburnparticlemaxmillis),
        maxparticlemillis = max(grassburnparticleminmillis, grassburnparticlemaxmillis);
    event.particleend = lastmillis + minparticlemillis + int((maxparticlemillis - minparticlemillis)*event.seed + 0.5f);
    event.lastemit = lastmillis - grassburnparticlemillis;
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
static Shader *grassshader = NULL, *grassshadowshader = NULL;
static Texture *grassburntex = NULL;

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
    const float spacing = grassstep/sqrtf(grassdensity), patchsize = float(grasspatchsize);
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

            int start = instances.length(),
                mingridx = int(floorf(patchminx/spacing)) - 1, maxgridx = int(floorf(patchmaxx/spacing)),
                mingridy = int(floorf(patchminy/spacing)) - 1, maxgridy = int(floorf(patchmaxy/spacing));
            vec bbmin(1e16f, 1e16f, 1e16f), bbmax(-1e16f, -1e16f, -1e16f);

            for(int gy = mingridy; gy <= maxgridy && !full; ++gy) for(int gx = mingridx; gx <= maxgridx; ++gx)
            {
                uint cellseed = grasshashcombine(grasshashcombine(seed, uint(gx)), uint(gy));
                float x = (gx + grasshashunit(cellseed))*spacing,
                      y = (gy + grasshashunit(cellseed ^ 0xB5297A4Du))*spacing;
                if(x < patchminx || x >= patchmaxx || y < patchminy || y >= patchmaxy) continue;

                float edgedist;
                if(!insidegrasstri(g, x, y, edgedist)) continue;

                float z = g.surface.zintersect(vec(x, y, 0));
                grassinstance &inst = instances.add();
                float variation = grasshashunit(cellseed ^ 0x68E31DA4u);
                inst.originangle = vec4(x, y, z, grasshashunit(cellseed ^ 0x1B56C4E9u)*2*M_PI);
                inst.variation = vec4(0.8f + 0.4f*variation, grasshashunit(cellseed ^ 0xC2B2AE35u), edgedist, g.surface.z);
                bbmin.min(vec(x, y, z));
                bbmax.max(vec(x, y, z));
                if(instances.length() >= grassmaxinstances) full = true;
            }

            int count = instances.length() - start;
            if(!count) continue;
            grasspatch &patch = va->grasspatches.add();
            patch.center = vec(bbmin).add(bbmax).mul(0.5f);
            patch.radius = patch.center.dist(bbmax);
            patch.offset = start;
            patch.count = count;
            patch.texture = g.texture;
            patch.blend = g.blend;
            patch.slot = slot;
            patch.blendpos = ivec(g.center);
            int particlecandidates = 0;
            loopj(count)
            {
                const vec4 &candidate = instances[start + j].originangle;
                if(g.blend && blendcache && lookupblendmap(blendcache, vec(candidate))/255.0f <= grasstest) continue;
                int selected = particlecandidates < MAXGRASSPATCHPARTICLEPOSITIONS ? particlecandidates :
                               int(grasshashcombine(seed, uint(j))%uint(particlecandidates + 1));
                particlecandidates++;
                if(selected < MAXGRASSPATCHPARTICLEPOSITIONS)
                    patch.particlepositions[selected] = vec4(vec(candidate), instances[start + j].variation.y);
            }
            patch.numparticlepositions = min(particlecandidates, int(MAXGRASSPATCHPARTICLEPOSITIONS));
        }
        if(full) break;
    }

    freeblendmapcache(blendcache);
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
    grassshadowshader = generateshader("grassshadow", "grassshadowshader");
}

void cleargrassshaders()
{
    grassshader = grassshadowshader = NULL;
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
    bvec color(grasscolour);
    GLOBALPARAMF(grasscolourparams, color.x/255.0f, color.y/255.0f, color.z/255.0f, 1.0f);
    bvec burncolor(grassburncolour);
    GLOBALPARAMF(grassburncolourparams, burncolor.x/255.0f, burncolor.y/255.0f, burncolor.z/255.0f);
    bvec burnglowcolor(grassburnglowcolour);
    GLOBALPARAMF(grassburnglowcolourparams, burnglowcolor.x/255.0f, burnglowcolor.y/255.0f, burnglowcolor.z/255.0f);
    float burntime = lastmillis/1000.0f;
    GLOBALPARAMF(grassburnanimparams, fmodf(burntime*grassburnscrollx, 1.0f), fmodf(burntime*grassburnscrolly, 1.0f), grassburnglowscale);
    GLOBALPARAMF(grassburntimingparams, float(grassburnpropagatemillis), float(grassburnfadeinmillis), float(grassburnmillis));
    GLOBALPARAMF(grasstest, grasstest);
}

static void setgrassdrawparams(Texture *tex, float density, float fade)
{
    float width = grassheight*tex->xs/float(max(grassscale*tex->ys, 1)),
          taperstart = grassdist*grasstaper;
    LOCALPARAMF(grassmeshparams, width, float(grassheight), 0.0f, 0.0f);
    LOCALPARAMF(grassdrawparams, density, fade, float(grassdist), taperstart);
}

static void updategrassburnevents()
{
    loopvrev(grassburnevents) if(lastmillis - grassburnevents[i].recoverstart >= grassburnfademillis) grassburnevents.removeunordered(i);
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

static void setgrassdamageparams(const grasspatch &patch)
{
    vec4 params[MAXGRASSDAMAGEPARAMS];
    vec4 states[MAXGRASSDAMAGEPARAMS];
    int count = 0;
    loopvrev(grassburnevents)
    {
        const grassburnevent &event = grassburnevents[i];
        float maxradius = max(event.radius*(1 + grassburnvariation) + grassburnpadding, 0.0f),
              reach = patch.radius + maxradius;
        if(maxradius <= 0) continue;
        if(patch.center.squaredist(event.center) > reach*reach) continue;

        float recovery = lastmillis <= event.recoverstart ? 0.0f :
                         min((lastmillis - event.recoverstart)/float(grassburnfademillis), 1.0f),
              burnage = max(float(lastmillis - event.burnstart), 0.0f);
        params[count] = vec4(event.center, event.radius);
        states[count++] = vec4(recovery, event.seed, burnage, 0.0f);
        if(count >= MAXGRASSDAMAGEPARAMS) break;
    }
    LOCALPARAMF(grassdamagecontrol, float(count), 0.0f, grassburnpadding, grassburnvariation);
    if(count)
    {
        LOCALPARAMV(grassdamageparams, params, count);
        LOCALPARAMV(grassdamagestates, states, count);
    }
}

static void drawgrasslod(const grasspatch &patch, Texture *tex, int lod, float density, float fade)
{
    if(density <= 0 || fade <= 0) return;
    setgrassdrawparams(tex, density, fade);
    const grassmesh &mesh = grassmeshes[lod];
    glDrawElementsInstanced_(GL_TRIANGLES, mesh.count, GL_UNSIGNED_SHORT, (const void *)(size_t(mesh.offset)*sizeof(ushort)), patch.count);
    xtravertsva += mesh.verts*patch.count;
    glde++;
}

static void drawgrasspatchlod(const grasspatch &patch, Texture *tex, float dist, float densityscale, bool shadow)
{
    if(!shadow) setgrassdamageparams(patch);
    float transition = min(float(grasslodtransition), float(grasslod1));
    if(transition > 0 && dist >= grasslod1 - transition && dist <= grasslod1 + transition)
    {
        float blend = clamp((dist - (grasslod1 - transition))/(2*transition), 0.0f, 1.0f);
        drawgrasslod(patch, tex, 0, densityscale, 1 - blend);
        drawgrasslod(patch, tex, 1, grassloddensity1*densityscale, blend);
        return;
    }

    int lod = dist < grasslod1 ? 0 : 1;
    drawgrasslod(patch, tex, lod, (lod ? grassloddensity1 : 1.0f)*densityscale, 1);
}

static void rendergrasspatches(vtxarray *vas, bool shadow, int cascade)
{
    Shader *shader = shadow ? grassshadowshader : grassshader;
    if(!shader) return;

    setgrassframeparams();
    updategrassburnevents();
    if(!shadow) emitgrassburnparticles(vas);
    initgrassmeshes();
    setupgrassattribs();
    glDisable(GL_CULL_FACE);
    glActiveTexture_(GL_TEXTURE0);
    if(!shadow)
    {
        if(!grassburntex) grassburntex = textureload("media/noise/burning_grass.jpg", 0, true, false);
        glActiveTexture_(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, grassburntex->id);
        glActiveTexture_(GL_TEXTURE0);
    }

    GLuint texid = 0;
    bool texbound = false;
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
            }
            else if(isvisiblesphere(radius, patch.center) >= VFC_FOGGED) continue;

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

            if(blend != patch.blend)
            {
                if(patch.blend)
                {
                    glActiveTexture_(GL_TEXTURE1);
                    bindblendtexture(patch.blendpos);
                    glActiveTexture_(GL_TEXTURE0);
                    shader->setvariant(0, 0);
                }
                else shader->set();
                blend = patch.blend;
            }

            bindgrassinstances(va, patch.offset);
            float densityscale = shadow && cascade > 0 ? grassshadowdensity : 1.0f;
            drawgrasspatchlod(patch, tex, dist, densityscale, shadow);
        }
    }

    cleanupgrassattribs();
    glEnable(GL_CULL_FACE);
}

void rendergrass()
{
    if(!grass || !grassdist || glversion < 400 || !glDrawElementsInstanced_ || !visibleva) return;
    rendergrasspatches(visibleva, false, 0);
}

void rendergrassshadow(int cascade)
{
    if(!grass || !grassdist || cascade < 0 || cascade >= grassshadowcascades || glversion < 400 || !glDrawElementsInstanced_ || !shadowva) return;
    rendergrasspatches(shadowva, true, cascade);
}

void cleanupgrass()
{
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
