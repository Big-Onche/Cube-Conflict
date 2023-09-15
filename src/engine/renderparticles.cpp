// renderparticles.cpp

#include "gfx.h"

Shader *particleshader = NULL, *particlenotextureshader = NULL, *particlesoftshader = NULL, *particletextshader = NULL;

VARP(particlelayers, 0, 1, 1);
FVARP(particlebright, 0, 1.75, 100);
VARP(particlesize, 20, 100, 500);

VARP(softparticles, 0, 1, 1);
VARP(softparticleblend, 1, 8, 64);

VARR(partcloudcolour, 0, 0x888888, 0xFFFFFF);

VARFP(particleslod, 0, 2, 3, initparticles());

// Check canemitparticles() to limit the rate that paricles can be emitted for models/sparklies
// Automatically stops particles being emitted when paused or in reflective drawing
VARP(emitmillis, 1, 17, 1000);
static int lastemitframe = 0, emitoffset = 0;
static bool canemit = false, regenemitters = false, canstep = false;

static bool canemitparticles()
{
    return canemit || emitoffset;
}

VARP(showparticles, 0, 1, 1);
VAR(cullparticles, 0, 1, 1);
VAR(replayparticles, 0, 1, 1);
VARN(seedparticles, seedmillis, 0, 3000, 10000);
VAR(dbgpcull, 0, 0, 1);
VAR(dbgpseed, 0, 0, 1);

struct particleemitter
{
    extentity *ent;
    vec bbmin, bbmax;
    vec center;
    float radius;
    ivec cullmin, cullmax;
    int maxfade, lastemit, lastcull;

    particleemitter(extentity *ent)
        : ent(ent), bbmin(ent->o), bbmax(ent->o), maxfade(-1), lastemit(0), lastcull(0)
    {}

    void finalize()
    {
        center = vec(bbmin).add(bbmax).mul(0.5f);
        radius = bbmin.dist(bbmax)/2;
        cullmin = ivec::floor(bbmin);
        cullmax = ivec::ceil(bbmax);
        if(dbgpseed) conoutf(CON_DEBUG, "radius: %f, maxfade: %d", radius, maxfade);
    }

    void extendbb(const vec &o, float size = 0)
    {
        bbmin.x = min(bbmin.x, o.x - size);
        bbmin.y = min(bbmin.y, o.y - size);
        bbmin.z = min(bbmin.z, o.z - size);
        bbmax.x = max(bbmax.x, o.x + size);
        bbmax.y = max(bbmax.y, o.y + size);
        bbmax.z = max(bbmax.z, o.z + size);
    }

    void extendbb(float z, float size = 0)
    {
        bbmin.z = min(bbmin.z, z - size);
        bbmax.z = max(bbmax.z, z + size);
    }
};

static vector<particleemitter> emitters;
static particleemitter *seedemitter = NULL;

void clearparticleemitters()
{
    emitters.setsize(0);
    regenemitters = true;
}

void addparticleemitters()
{
    emitters.setsize(0);
    const vector<extentity *> &ents = entities::getents();
    loopv(ents)
    {
        extentity &e = *ents[i];
        if(e.type != ET_PARTICLES) continue;
        emitters.add(particleemitter(&e));
    }
    regenemitters = false;
}

enum
{
    PT_PART = 0,
    PT_TAPE,
    PT_TRAIL,
    PT_TEXT,
    PT_TEXTUP,
    PT_METER,
    PT_METERVS,
    PT_FIREBALL,
    PT_LIGHTNING,
    PT_FLARE,

    PT_MOD          = 1<<8,
    PT_RND4         = 1<<9,
    PT_LERP         = 1<<10, // use very sparingly - order of blending issues
    PT_TRACK        = 1<<11,
    PT_BRIGHT       = 1<<12,
    PT_SOFT         = 1<<13,
    PT_HFLIP        = 1<<14,
    PT_VFLIP        = 1<<15,
    PT_ROT          = 1<<16,
    PT_CULL         = 1<<17,
    PT_FEW          = 1<<18,
    PT_ICON         = 1<<19,
    PT_NOTEX        = 1<<20,
    PT_SHADER       = 1<<21,
    PT_NOLAYER      = 1<<22,
    PT_COLLIDE      = 1<<23,
    PT_OVERBRIGHT   = 1<<24,
    PT_NOMAXDIST    = 1<<25,
    PT_FLIP         = PT_HFLIP | PT_VFLIP | PT_ROT
};

const char *partnames[] = { "part", "tape", "trail", "text", "textup", "meter", "metervs", "fireball", "lightning", "flare" };

struct particle
{
    vec o, d;
    int gravity, fade, millis, sizemod;
    bool hud;
    bvec color;
    uchar flags;
    float size;
    union
    {
        const char *text;
        float val;
        physent *owner;
        struct
        {
            uchar color2[3];
            uchar progress;
        };
    };
};

struct partvert
{
    vec pos;
    bvec4 color;
    vec2 tc;
};

#define COLLIDERADIUS 8.0f
#define COLLIDEERROR 1.0f

struct partrenderer
{
    Texture *tex;
    const char *texname;
    int texclamp;
    uint type;
    int stain;
    string info;

    partrenderer(const char *texname, int texclamp, int type, int stain = -1)
        : tex(NULL), texname(texname), texclamp(texclamp), type(type), stain(stain)
    {
    }
    partrenderer(int type, int stain = -1)
        : tex(NULL), texname(NULL), texclamp(0), type(type), stain(stain)
    {
    }
    virtual ~partrenderer()
    {
    }

    virtual void init(int n) { }
    virtual void reset() = 0;
    virtual void resettracked(physent *owner) { }
    virtual particle *addpart(const vec &o, const vec &d, int fade, int color, float size, int gravity = 0, int sizemod = 0, bool hud = false) = 0;
    virtual void update() { }
    virtual void render() = 0;
    virtual bool haswork() = 0;
    virtual int count() = 0; //for debug
    virtual void cleanup() {}

    virtual void seedemitter(particleemitter &pe, const vec &o, const vec &d, int fade, float size, int gravity)
    {
    }

    virtual void preload()
    {
        if(texname && !tex) tex = textureload(texname, texclamp);
    }

    //blend = 0 => remove it
    void calc(particle *p, int &blend, int &ts, vec &o, vec &d, bool step = true)
    {
        o = p->o;
        d = p->d;
        if(type&PT_TRACK && p->owner) game::particletrack(p->owner, o, d);
        if(p->fade <= 5)
        {
            ts = 1;
            blend = 255;
        }
        else
        {
            if(p->sizemod && !game::ispaused() && p->sizemod > -25 && p->sizemod < 25)
            {
                float mod = p->sizemod;
                p->size += mod/gfx::nbfps;
            }

            ts = lastmillis-p->millis;
            blend = max(255 - (ts<<8)/p->fade, 0);
            if(p->gravity)
            {
                if(ts > p->fade) ts = p->fade;
                float t = ts;
                o.add(vec(d).mul(t/5000.0f));
                o.z -= t*t/(2.0f * 5000.0f * p->gravity);
            }
            if(type&PT_COLLIDE && o.z < p->val && step)
            {
                if(stain >= 0)
                {
                    if((stain==STAIN_RAIN || stain==STAIN_SNOW) && p->o.dist(camera1->o) > 320) return;
                    vec surface;
                    float floorz = rayfloor(vec(o.x, o.y, p->val), surface, RAY_CLIPMAT|RAY_LIQUIDMAT|RAY_POLY, COLLIDERADIUS);
                    float collidez = floorz<0 ? o.z-COLLIDERADIUS : p->val - floorz;
                    if(o.z >= collidez+COLLIDEERROR)
                        p->val = collidez+COLLIDEERROR;
                    else
                    {
                        switch(stain)
                        {
                            case STAIN_RAIN:
                                addstain(stain, vec(o.x, o.y, collidez), vec(p->o).sub(o).normalize(), p->size/7.5f, 0xFFFFFF, type&PT_RND4 ? (p->flags>>5)&3 : 0);
                                regularsplash(PART_WATER, 0x18181A, 50, 3, 120, vec(o.x, o.y, collidez), 0.08f, 500, 0, 3, true);
                                break;
                            case STAIN_SNOW:
                                addstain(stain, vec(o.x, o.y, collidez), vec(p->o).sub(o).normalize(), p->size, p->color, type&PT_RND4 ? (p->flags>>5)&3 : 0);
                                break;
                            case STAIN_BURN:
                                addstain(stain, vec(o.x, o.y, collidez), vec(p->o).sub(o).normalize(), p->size*1.5f, 0x222222, type&PT_RND4 ? (p->flags>>5)&3 : 0);
                                addstain(STAIN_BULLET_GLOW, vec(o.x, o.y, collidez), vec(p->o).sub(o).normalize(), p->size, 0xFF6622, type&PT_RND4 ? (p->flags>>5)&3 : 0);
                                break;
                        }
                        blend = 0;
                    }
                }
                else blend = 0;
            }
        }
    }

    void debuginfo()
    {
        formatstring(info, "%d\t%s(", count(), partnames[type&0xFF]);
        if(type&PT_LERP) concatstring(info, "l,");
        if(type&PT_MOD) concatstring(info, "m,");
        if(type&PT_RND4) concatstring(info, "r,");
        if(type&PT_TRACK) concatstring(info, "t,");
        if(type&PT_FLIP) concatstring(info, "f,");
        if(type&PT_COLLIDE) concatstring(info, "c,");
        int len = strlen(info);
        info[len-1] = info[len-1] == ',' ? ')' : '\0';
        if(texname)
        {
            const char *title = strrchr(texname, '/');
            if(title) concformatstring(info, ": %s", title+1);
        }
    }
};

struct listparticle : particle
{
    listparticle *next;
};

VARP(outlinemeters, 0, 0, 1);

struct listrenderer : partrenderer
{
    static listparticle *parempty;
    listparticle *list;

    listrenderer(const char *texname, int texclamp, int type, int stain = -1)
        : partrenderer(texname, texclamp, type, stain), list(NULL)
    {
    }
    listrenderer(int type, int stain = -1)
        : partrenderer(type, stain), list(NULL)
    {
    }

    virtual ~listrenderer()
    {
    }

    virtual void killpart(listparticle *p)
    {
    }

    void reset()
    {
        if(!list) return;
        listparticle *p = list;
        for(;;)
        {
            killpart(p);
            if(p->next) p = p->next;
            else break;
        }
        p->next = parempty;
        parempty = list;
        list = NULL;
    }

    void resettracked(physent *owner)
    {
        if(!(type&PT_TRACK)) return;
        for(listparticle **prev = &list, *cur = list; cur; cur = *prev)
        {
            if(!owner || cur->owner==owner)
            {
                *prev = cur->next;
                cur->next = parempty;
                parempty = cur;
            }
            else prev = &cur->next;
        }
    }

    particle *addpart(const vec &o, const vec &d, int fade, int color, float size, int gravity, int sizemod, bool hud)
    {
        if(!parempty)
        {
            listparticle *ps = new listparticle[256];
            loopi(255) ps[i].next = &ps[i+1];
            ps[255].next = parempty;
            parempty = ps;
        }
        listparticle *p = parempty;
        parempty = p->next;
        p->next = list;
        list = p;
        p->o = o;
        p->d = d;
        p->gravity = gravity;
        p->fade = fade;
        p->millis = lastmillis + emitoffset;
        p->color = bvec::hexcolor(color);
        p->size = size;
        p->owner = NULL;
        p->flags = 0;
        p->sizemod = sizemod;
        p->hud = hud;
        return p;
    }

    int count()
    {
        int num = 0;
        listparticle *lp;
        for(lp = list; lp; lp = lp->next) num++;
        return num;
    }

    bool haswork()
    {
        return (list != NULL);
    }

    virtual void startrender() = 0;
    virtual void endrender() = 0;
    virtual void renderpart(listparticle *p, const vec &o, const vec &d, int blend, int ts) = 0;

    bool renderpart(listparticle *p)
    {
        vec o, d;
        int blend, ts;
        calc(p, blend, ts, o, d, canstep);
        if(blend <= 0) return false;
        renderpart(p, o, d, blend, ts);
        return p->fade > 5;
    }

    void render()
    {
        startrender();
        if(tex) glBindTexture(GL_TEXTURE_2D, tex->id);
        if(canstep) for(listparticle **prev = &list, *p = list; p; p = *prev)
        {
            if(renderpart(p)) prev = &p->next;
            else
            { // remove
                *prev = p->next;
                p->next = parempty;
                killpart(p);
                parempty = p;
            }
        }
        else for(listparticle *p = list; p; p = p->next) renderpart(p);
        endrender();
    }
};

listparticle *listrenderer::parempty = NULL;

struct meterrenderer : listrenderer
{
    meterrenderer(int type)
        : listrenderer(type|PT_NOTEX|PT_LERP|PT_NOLAYER)
    {}

    void startrender()
    {
         glDisable(GL_BLEND);
         gle::defvertex();
    }

    void endrender()
    {
         glEnable(GL_BLEND);
    }

    void renderpart(listparticle *p, const vec &o, const vec &d, int blend, int ts)
    {
        int basetype = type&0xFF;
        float scale = FONTH*p->size/80.0f, right = 8, left = p->progress/100.0f*right;
        matrix4x3 m(camright, vec(camup).neg(), vec(camdir).neg(), o);
        m.scale(scale);
        m.translate(-right/2.0f, 0, 0);

        if(outlinemeters)
        {
            gle::colorf(0, 0.8f, 0);
            gle::begin(GL_TRIANGLE_STRIP);
            loopk(4)
            {
                const vec2 &sc = sincos360[k*(180/(4-1))];
                float c = (0.5f + 0.1f)*sc.y, s = 0.5f - (0.5f + 0.1f)*sc.x;
                gle::attrib(m.transform(vec2(-c, s)));
                gle::attrib(m.transform(vec2(right + c, s)));
            }
            gle::end();
        }

        if(basetype==PT_METERVS) gle::colorub(p->color2[0], p->color2[1], p->color2[2]);
        else gle::colorf(0, 0, 0);
        gle::begin(GL_TRIANGLE_STRIP);
        loopk(4)
        {
            const vec2 &sc = sincos360[k*(180/(4-1))];
            float c = 0.5f*sc.y, s = 0.5f - 0.5f*sc.x;
            gle::attrib(m.transform(vec2(left + c, s)));
            gle::attrib(m.transform(vec2(right + c, s)));
        }
        gle::end();

        if(outlinemeters)
        {
            gle::colorf(0, 0.8f, 0);
            gle::begin(GL_TRIANGLE_FAN);
            loopk(4)
            {
                const vec2 &sc = sincos360[k*(180/(4-1))];
                float c = (0.5f + 0.1f)*sc.y, s = 0.5f - (0.5f + 0.1f)*sc.x;
                gle::attrib(m.transform(vec2(left + c, s)));
            }
            gle::end();
        }

        gle::color(p->color);
        gle::begin(GL_TRIANGLE_STRIP);
        loopk(4)
        {
            const vec2 &sc = sincos360[k*(180/(4-1))];

            float c = 0.5f*sc.y, s = 0.5f - 0.5f*sc.x;
            gle::attrib(m.transform(vec2(-c, s)));
            gle::attrib(m.transform(vec2(left + c, s)));
        }
        gle::end();
    }
};
static meterrenderer meters(PT_METER), metervs(PT_METERVS);

struct textrenderer : listrenderer
{
    textrenderer(int type = 0)
        : listrenderer(type|PT_TEXT|PT_LERP|PT_SHADER|PT_NOLAYER)
    {}

    void startrender()
    {
        textshader = particletextshader;

        pushfont();
        setfont("default_outline");
    }

    void endrender()
    {
        textshader = NULL;

        popfont();
    }

    void killpart(listparticle *p)
    {
        if(p->text && p->flags&1) delete[] p->text;
    }

    void renderpart(listparticle *p, const vec &o, const vec &d, int blend, int ts)
    {
        float scale = p->size/80.0f, xoff = -text_width(p->text)/2, yoff = 0;
        if((type&0xFF)==PT_TEXTUP) { xoff += detrnd((size_t)p, 100)-50; yoff -= detrnd((size_t)p, 101); }

        matrix4x3 m(camright, vec(camup).neg(), vec(camdir).neg(), o);
        m.scale(scale);
        m.translate(xoff, yoff, 50);

        textmatrix = &m;
        draw_text(p->text, 0, 0, p->color.r, p->color.g, p->color.b, blend);
        textmatrix = NULL;
    }
};
static textrenderer texts;

template<int T>
static inline void modifyblend(const vec &o, int &blend)
{
    blend = min(blend<<2, 255);
}

template<>
inline void modifyblend<PT_TAPE>(const vec &o, int &blend)
{
}

template<int T>
static inline void genpos(const vec &o, const vec &d, float size, int grav, int ts, partvert *vs)
{
    vec udir = vec(camup).sub(camright).mul(size);
    vec vdir = vec(camup).add(camright).mul(size);
    vs[0].pos = vec(o.x + udir.x, o.y + udir.y, o.z + udir.z);
    vs[1].pos = vec(o.x + vdir.x, o.y + vdir.y, o.z + vdir.z);
    vs[2].pos = vec(o.x - udir.x, o.y - udir.y, o.z - udir.z);
    vs[3].pos = vec(o.x - vdir.x, o.y - vdir.y, o.z - vdir.z);
}

template<>
inline void genpos<PT_TAPE>(const vec &o, const vec &d, float size, int ts, int grav, partvert *vs)
{
    vec dir1 = vec(d).sub(o), dir2 = vec(d).sub(camera1->o), c;
    c.cross(dir2, dir1).normalize().mul(size);
    vs[0].pos = vec(d.x-c.x, d.y-c.y, d.z-c.z);
    vs[1].pos = vec(o.x-c.x, o.y-c.y, o.z-c.z);
    vs[2].pos = vec(o.x+c.x, o.y+c.y, o.z+c.z);
    vs[3].pos = vec(d.x+c.x, d.y+c.y, d.z+c.z);
}

template<>
inline void genpos<PT_TRAIL>(const vec &o, const vec &d, float size, int ts, int grav, partvert *vs)
{
    vec e = d;
    if(grav) e.z -= float(ts)/grav;
    e.div(-75.0f).add(o);
    genpos<PT_TAPE>(o, e, size, ts, grav, vs);
}

template<int T>
static inline void genrotpos(const vec &o, const vec &d, float size, int grav, int ts, partvert *vs, int rot)
{
    genpos<T>(o, d, size, grav, ts, vs);
}

#define ROTCOEFFS(n) { \
    vec2(-1,  1).rotate_around_z(n*2*M_PI/32.0f), \
    vec2( 1,  1).rotate_around_z(n*2*M_PI/32.0f), \
    vec2( 1, -1).rotate_around_z(n*2*M_PI/32.0f), \
    vec2(-1, -1).rotate_around_z(n*2*M_PI/32.0f) \
}
static const vec2 rotcoeffs[32][4] =
{
    ROTCOEFFS(0),  ROTCOEFFS(1),  ROTCOEFFS(2),  ROTCOEFFS(3),  ROTCOEFFS(4),  ROTCOEFFS(5),  ROTCOEFFS(6),  ROTCOEFFS(7),
    ROTCOEFFS(8),  ROTCOEFFS(9),  ROTCOEFFS(10), ROTCOEFFS(11), ROTCOEFFS(12), ROTCOEFFS(13), ROTCOEFFS(14), ROTCOEFFS(15),
    ROTCOEFFS(16), ROTCOEFFS(17), ROTCOEFFS(18), ROTCOEFFS(19), ROTCOEFFS(20), ROTCOEFFS(21), ROTCOEFFS(22), ROTCOEFFS(7),
    ROTCOEFFS(24), ROTCOEFFS(25), ROTCOEFFS(26), ROTCOEFFS(27), ROTCOEFFS(28), ROTCOEFFS(29), ROTCOEFFS(30), ROTCOEFFS(31),
};

template<>
inline void genrotpos<PT_PART>(const vec &o, const vec &d, float size, int grav, int ts, partvert *vs, int rot)
{
    const vec2 *coeffs = rotcoeffs[rot];
    vs[0].pos = vec(o).madd(camright, coeffs[0].x*size).madd(camup, coeffs[0].y*size);
    vs[1].pos = vec(o).madd(camright, coeffs[1].x*size).madd(camup, coeffs[1].y*size);
    vs[2].pos = vec(o).madd(camright, coeffs[2].x*size).madd(camup, coeffs[2].y*size);
    vs[3].pos = vec(o).madd(camright, coeffs[3].x*size).madd(camup, coeffs[3].y*size);
}

template<int T>
static inline void seedpos(particleemitter &pe, const vec &o, const vec &d, int fade, float size, int grav)
{
    if(grav)
    {
        float t = fade;
        vec end = vec(o).madd(d, t/5000.0f);
        end.z -= t*t/(2.0f * 5000.0f * grav);
        pe.extendbb(end, size);

        float tpeak = d.z*grav;
        if(tpeak > 0 && tpeak < fade) pe.extendbb(o.z + 1.5f*d.z*tpeak/5000.0f, size);
    }
}

template<>
inline void seedpos<PT_TAPE>(particleemitter &pe, const vec &o, const vec &d, int fade, float size, int grav)
{
    pe.extendbb(d, size);
}

template<>
inline void seedpos<PT_TRAIL>(particleemitter &pe, const vec &o, const vec &d, int fade, float size, int grav)
{
    vec e = d;
    if(grav) e.z -= float(fade)/grav;
    e.div(-75.0f).add(o);
    pe.extendbb(e, size);
}

template<int T>
struct varenderer : partrenderer
{
    partvert *verts;
    particle *parts;
    int maxparts, numparts, lastupdate, rndmask;
    GLuint vbo;

    varenderer(const char *texname, int type, int stain = -1)
        : partrenderer(texname, 3, type, stain),
          verts(NULL), parts(NULL), maxparts(0), numparts(0), lastupdate(-1), rndmask(0), vbo(0)
    {
        if(type & PT_HFLIP) rndmask |= 0x01;
        if(type & PT_VFLIP) rndmask |= 0x02;
        if(type & PT_ROT) rndmask |= 0x1F<<2;
        if(type & PT_RND4) rndmask |= 0x03<<5;
    }

    void cleanup()
    {
        if(vbo) { glDeleteBuffers_(1, &vbo); vbo = 0; }
    }

    void init(int n)
    {
        DELETEA(parts);
        DELETEA(verts);
        parts = new particle[n];
        verts = new partvert[n*4];
        maxparts = n;
        numparts = 0;
        lastupdate = -1;
    }

    void reset()
    {
        numparts = 0;
        lastupdate = -1;
    }

    void resettracked(physent *owner)
    {
        if(!(type&PT_TRACK)) return;
        loopi(numparts)
        {
            particle *p = parts+i;
            if(!owner || (p->owner == owner)) p->fade = -1;
        }
        lastupdate = -1;
    }

    int count()
    {
        return numparts;
    }

    bool haswork()
    {
        return (numparts > 0);
    }

    particle *addpart(const vec &o, const vec &d, int fade, int color, float size, int gravity, int sizemod, bool hud)
    {
        particle *p = parts + (numparts < maxparts ? numparts++ : rnd(maxparts)); //next free slot, or kill a random kitten
        p->o = o;
        p->d = d;
        p->gravity = gravity;
        p->fade = fade;
        p->millis = lastmillis + emitoffset;
        p->color = bvec::hexcolor(color);
        p->size = size;
        p->owner = NULL;
        p->flags = 0x80 | (rndmask ? rnd(0x80) & rndmask : 0);
        p->sizemod = sizemod;
        p->hud = hud;
        lastupdate = -1;
        return p;
    }

    void seedemitter(particleemitter &pe, const vec &o, const vec &d, int fade, float size, int gravity)
    {
        pe.maxfade = max(pe.maxfade, fade);
        size *= SQRT2;
        pe.extendbb(o, size);

        seedpos<T>(pe, o, d, fade, size, gravity);
        if(!gravity) return;

        vec end(o);
        float t = fade;
        end.add(vec(d).mul(t/5000.0f));
        end.z -= t*t/(2.0f * 5000.0f * gravity);
        pe.extendbb(end, size);

        float tpeak = d.z*gravity;
        if(tpeak > 0 && tpeak < fade) pe.extendbb(o.z + 1.5f*d.z*tpeak/5000.0f, size);
    }

    void genverts(particle *p, partvert *vs, bool regen)
    {
        vec o, d;
        int blend, ts;

        calc(p, blend, ts, o, d);
        if(blend <= 1 || p->fade <= 5) p->fade = -1; //mark to remove on next pass (i.e. after render)

        modifyblend<T>(o, blend);

        if(regen)
        {
            p->flags &= ~0x80;

            #define SETTEXCOORDS(u1c, u2c, v1c, v2c, body) \
            { \
                float u1 = u1c, u2 = u2c, v1 = v1c, v2 = v2c; \
                body; \
                vs[0].tc = vec2(u1, v1); \
                vs[1].tc = vec2(u2, v1); \
                vs[2].tc = vec2(u2, v2); \
                vs[3].tc = vec2(u1, v2); \
            }
            if(type&PT_RND4)
            {
                float tx = 0.5f*((p->flags>>5)&1), ty = 0.5f*((p->flags>>6)&1);
                SETTEXCOORDS(tx, tx + 0.5f, ty, ty + 0.5f,
                {
                    if(p->flags&0x01) swap(u1, u2);
                    if(p->flags&0x02) swap(v1, v2);
                });
            }
            else if(type&PT_ICON)
            {
                float tx = 0.25f*(p->flags&3), ty = 0.25f*((p->flags>>2)&3);
                SETTEXCOORDS(tx, tx + 0.25f, ty, ty + 0.25f, {});
            }
            else SETTEXCOORDS(0, 1, 0, 1, {});

            #define SETCOLOR(r, g, b, a) \
            do { \
                bvec4 col(r, g, b, a); \
                loopi(4) vs[i].color = col; \
            } while(0)
            #define SETMODCOLOR SETCOLOR((p->color.r*blend)>>8, (p->color.g*blend)>>8, (p->color.b*blend)>>8, 255)
            if(type&PT_MOD) SETMODCOLOR;
            else SETCOLOR(p->color.r, p->color.g, p->color.b, blend);
        }
        else if(type&PT_MOD) SETMODCOLOR;
        else loopi(4) vs[i].color.a = blend;

        if(type&PT_ROT) genrotpos<T>(o, d, p->size, ts, p->gravity, vs, (p->flags>>2)&0x1F);
        else genpos<T>(o, d, p->size, ts, p->gravity, vs);
    }

    void genverts()
    {
        loopi(numparts)
        {
            particle *p = &parts[i];
            partvert *vs = &verts[i*4];
            if(p->fade < 0)
            {
                do
                {
                    --numparts;
                    if(numparts <= i) return;
                }
                while(parts[numparts].fade < 0);
                *p = parts[numparts];
                genverts(p, vs, true);
            }
            else genverts(p, vs, (p->flags&0x80)!=0);
        }
    }

    void genvbo()
    {
        if(lastmillis == lastupdate && vbo) return;
        lastupdate = lastmillis;

        genverts();

        if(!vbo) glGenBuffers_(1, &vbo);
        gle::bindvbo(vbo);
        glBufferData_(GL_ARRAY_BUFFER, maxparts*4*sizeof(partvert), NULL, GL_STREAM_DRAW);
        glBufferSubData_(GL_ARRAY_BUFFER, 0, numparts*4*sizeof(partvert), verts);
        gle::clearvbo();
    }

    void render()
    {
        genvbo();

        glBindTexture(GL_TEXTURE_2D, tex->id);

        gle::bindvbo(vbo);
        const partvert *ptr = 0;
        gle::vertexpointer(sizeof(partvert), ptr->pos.v);
        gle::texcoord0pointer(sizeof(partvert), ptr->tc.v);
        gle::colorpointer(sizeof(partvert), ptr->color.v);
        gle::enablevertex();
        gle::enabletexcoord0();
        gle::enablecolor();
        gle::enablequads();

        gle::drawquads(0, numparts);

        gle::disablequads();
        gle::disablevertex();
        gle::disabletexcoord0();
        gle::disablecolor();
        gle::clearvbo();
    }
};
typedef varenderer<PT_PART> quadrenderer;
typedef varenderer<PT_TAPE> taperenderer;
typedef varenderer<PT_TRAIL> trailrenderer;

#include "explosion.h"
#include "lensflare.h"
#include "lightning.h"

struct softquadrenderer : quadrenderer
{
    softquadrenderer(const char *texname, int type, int stain = -1)
        : quadrenderer(texname, type|PT_SOFT, stain)
    {
    }
};

static partrenderer *parts[] =
{
    new quadrenderer("media/particles/game/basic.png", PT_PART|PT_FLIP|PT_BRIGHT),                                           // PART_BASIC
    // guns muzzle flashes
    new quadrenderer("media/particles/flashes/little.png", PT_PART|PT_FEW|PT_FLIP|PT_BRIGHT|PT_TRACK),                       // PART_MF_LITTLE
    new quadrenderer("media/particles/flashes/big.png", PT_PART|PT_FEW|PT_FLIP|PT_BRIGHT|PT_TRACK),                          // PART_MF_BIG
    new quadrenderer("media/particles/flashes/electric.png", PT_PART|PT_FEW|PT_FLIP|PT_BRIGHT|PT_TRACK),                     // PART_MF_ELEC
    new quadrenderer("media/particles/flashes/plasma.png", PT_PART|PT_FEW|PT_FLIP|PT_BRIGHT|PT_TRACK),                       // PART_MF_PLASMA
    new quadrenderer("media/particles/flashes/rocket.png", PT_PART|PT_FEW|PT_FLIP|PT_BRIGHT|PT_TRACK),                       // PART_MF_ROCKET
    new quadrenderer("media/particles/flashes/shotgun.png", PT_PART|PT_FEW|PT_BRIGHT|PT_TRACK),                              // PART_MF_SHOTGUN
    new quadrenderer("media/particles/flashes/sniper.png", PT_PART|PT_FEW|PT_BRIGHT|PT_TRACK),                               // PART_MF_SNIPER
    // bullets flares
    new taperenderer("media/particles/trails/bullet_side.png", PT_TAPE|PT_FEW|PT_OVERBRIGHT),                                // PART_F_BULLET
    new taperenderer("media/particles/trails/shotgun_side.png", PT_TAPE|PT_OVERBRIGHT),                                      // PART_F_SHOTGUN
    new taperenderer("media/particles/trails/plasma_side.png", PT_TAPE|PT_FEW|PT_OVERBRIGHT),                                // PART_F_PLASMA
    new taperenderer("media/particles/trails/smoke_side.png", PT_TAPE|PT_FLIP|PT_FEW),                                       // PART_F_SMOKE
    new quadrenderer("media/particles/trails/spock_front.png", PT_PART|PT_FEW|PT_HFLIP|PT_BRIGHT),                           // PART_SPOCK_FRONT
    new quadrenderer("media/particles/trails/plasma_front.png", PT_PART|PT_FLIP|PT_FEW|PT_OVERBRIGHT),                       // PART_PLASMA_FRONT
    // flames and smokes
    new quadrenderer("media/particles/fire/smoke.png", PT_PART|PT_FLIP|PT_BRIGHT|PT_LERP|PT_RND4),                           // PART_SMOKE
    new quadrenderer("media/particles/fire/flames.png", PT_PART|PT_HFLIP|PT_RND4|PT_OVERBRIGHT),                             // PART_FLAME
    new quadrenderer("media/particles/fire/fire_ball.png", PT_PART|PT_FLIP|PT_BRIGHT|PT_RND4),                               // PART_FIRE_BALL
    new quadrenderer("media/particles/fire/firespark.png", PT_PART|PT_FLIP|PT_RND4|PT_OVERBRIGHT|PT_COLLIDE, STAIN_BURN),    // PART_FIRESPARK
    // water
    new quadrenderer("media/particles/water/water.png", PT_PART|PT_FLIP|PT_RND4|PT_BRIGHT),                                  // PART_WATER
    new quadrenderer("media/particles/water/bubbles.png", PT_PART|PT_FLIP|PT_BRIGHT|PT_RND4|PT_COLLIDE),                     // PART_BUBBLE
    new quadrenderer("<grey>media/particles/water/steam.png", PT_PART|PT_FLIP|PT_RND4),                                      // PART_STEAM
    // weather
    new quadrenderer("media/particles/weather/snow.png", PT_PART|PT_FLIP|PT_RND4|PT_COLLIDE, STAIN_SNOW),                    // PART_SNOW
    new trailrenderer("media/particles/weather/rain.png", PT_PART|PT_COLLIDE, STAIN_RAIN),                                   // PART_RAIN
    new trailrenderer("media/particles/weather/cloud_1.png", PT_TRAIL|PT_NOMAXDIST),                                         // PART_CLOUD1
    new trailrenderer("media/particles/weather/cloud_2.png", PT_TRAIL|PT_NOMAXDIST),                                         // PART_CLOUD2
    new trailrenderer("media/particles/weather/cloud_3.png", PT_TRAIL|PT_NOMAXDIST),                                         // PART_CLOUD3
    new trailrenderer("media/particles/weather/cloud_4.png", PT_TRAIL|PT_NOMAXDIST),                                         // PART_CLOUD4
    new trailrenderer("media/particles/weather/rainbow.png", PT_TRAIL),                                                      // PART_RAINBOW
    &lightnings,                                                                                                             // PART_LIGHTNING
    // game specific
    new quadrenderer("media/particles/game/target.png", PT_PART|PT_LERP|PT_BRIGHT),                                          // PART_TARGET
    new quadrenderer("media/particles/game/zero.png", PT_PART|PT_BRIGHT),                                                    // PART_ZERO
    new quadrenderer("media/particles/game/one.png", PT_PART|PT_BRIGHT),                                                     // PART_ONE
    new quadrenderer("media/particles/game/blip.png", PT_PART|PT_LERP),                                                      // PART_BLIP
    new quadrenderer("media/particles/game/health.png", PT_PART|PT_BRIGHT),                                                  // PART_HEALTH
    new quadrenderer("media/particles/game/mana.png", PT_PART|PT_BRIGHT),                                                    // PART_MANA
    &radar,                                                                                                                  // PART_RADAR
    &meters,                                                                                                                 // PART_METER
    &metervs,                                                                                                                // PART_METER_VS
    // explosions
    &ondedechoc,                                                                                                             // PART_SHOCKWAVE
    &plasmabursts,                                                                                                           // PART_PLASMABURST
    &plasmagrenade,                                                                                                          // PART_PLASMAGRENADE
    &fireballs,                                                                                                              // PART_EXPLOSION
    // misc
    new quadrenderer("<grey>media/particles/misc/blood.png", PT_PART|PT_FLIP|PT_MOD|PT_RND4|PT_COLLIDE, STAIN_BLOOD),        // PART_BLOOD (note: rgb is inverted)
    new quadrenderer("media/particles/misc/spark.png", PT_PART|PT_FLIP|PT_BRIGHT),                                           // PART_SPARK
    &texts,                                                                                                                  // PART_TEXT
    &flares                                                                                                                  // PART_LENS_FLARE - must be done last
};

VARFP(maxparticles, 10, 8000, 20000, initparticles());
VARFP(fewparticles, 10, 500, 10000, initparticles());

void initparticles()
{
    if(initing) return;
    if(!particleshader) particleshader = lookupshaderbyname("particle");
    if(!particlenotextureshader) particlenotextureshader = lookupshaderbyname("particlenotexture");
    if(!particlesoftshader) particlesoftshader = lookupshaderbyname("particlesoft");
    if(!particletextshader) particletextshader = lookupshaderbyname("particletext");
    loopi(sizeof(parts)/sizeof(parts[0])) parts[i]->init(parts[i]->type&PT_FEW ? min(fewparticles, maxparticles) : maxparticles);
    loopi(sizeof(parts)/sizeof(parts[0])) parts[i]->preload();
}

void clearparticles()
{
    loopi(sizeof(parts)/sizeof(parts[0])) parts[i]->reset();
    clearparticleemitters();
}

void cleanupparticles()
{
    loopi(sizeof(parts)/sizeof(parts[0])) parts[i]->cleanup();
}

void removetrackedparticles(physent *owner)
{
    loopi(sizeof(parts)/sizeof(parts[0])) parts[i]->resettracked(owner);
}

VARN(debugparticles, dbgparts, 0, 0, 1);

void debugparticles()
{
    if(!dbgparts) return;
    int n = sizeof(parts)/sizeof(parts[0]);
    pushhudmatrix();
    hudmatrix.ortho(0, FONTH*n*2*vieww/float(viewh), FONTH*n*2, 0, -1, 1); // squeeze into top-left corner
    flushhudmatrix();
    loopi(n) draw_text(parts[i]->info, FONTH, (i+n/2)*FONTH);
    pophudmatrix();
}

void renderparticles(int layer)
{
    canstep = layer != PL_UNDER;

    //want to debug BEFORE the lastpass render (that would delete particles)
    if(dbgparts && (layer == PL_ALL || layer == PL_UNDER)) loopi(sizeof(parts)/sizeof(parts[0])) parts[i]->debuginfo();

    bool rendered = false;
    uint lastflags = PT_LERP|PT_SHADER,
         flagmask = PT_LERP|PT_MOD|PT_BRIGHT|PT_NOTEX|PT_SOFT|PT_SHADER,
         excludemask = layer == PL_ALL ? ~0 : (layer != PL_NOLAYER ? PT_NOLAYER : 0);

    loopi(sizeof(parts)/sizeof(parts[0]))
    {
        partrenderer *p = parts[i];
        if((p->type&PT_NOLAYER) == excludemask || !p->haswork()) continue;

        if(!rendered)
        {
            rendered = true;
            glDepthMask(GL_FALSE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glActiveTexture_(GL_TEXTURE2);
            if(msaalight) glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msdepthtex);
            else glBindTexture(GL_TEXTURE_RECTANGLE, gdepthtex);
            glActiveTexture_(GL_TEXTURE0);
        }

        uint flags = p->type & flagmask, changedbits = flags ^ lastflags;
        if(changedbits)
        {
            if(changedbits&PT_LERP) { if(flags&PT_LERP) resetfogcolor(); else zerofogcolor(); }
            if(changedbits&(PT_LERP|PT_MOD))
            {
                if(flags&PT_LERP) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                else if(flags&PT_MOD) glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
                else glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            }
            if(!(flags&PT_SHADER))
            {
                if(changedbits&(PT_LERP|PT_SOFT|PT_NOTEX|PT_SHADER))
                {
                    if(flags&PT_SOFT && softparticles)
                    {
                        particlesoftshader->set();
                        LOCALPARAMF(softparams, -1.0f/softparticleblend, 0, 0);
                    }
                    else if(flags&PT_NOTEX) particlenotextureshader->set();
                    else particleshader->set();
                }
                if(changedbits&(PT_MOD|PT_BRIGHT|PT_SOFT|PT_NOTEX|PT_SHADER))
                {
                    float colorscale = flags&PT_MOD ? 1 : ldrscale;
                    if(flags&PT_BRIGHT || flags&PT_OVERBRIGHT) colorscale *= particlebright*(flags&PT_OVERBRIGHT ? 1.5f : 1);
                    LOCALPARAMF(colorscale, colorscale, colorscale, colorscale, 1);
                }
            }
            lastflags = flags;
        }
        p->render();
    }

    if(rendered)
    {
        if(lastflags&(PT_LERP|PT_MOD)) glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        if(!(lastflags&PT_LERP)) resetfogcolor();
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
}

static int addedparticles = 0;

static inline particle *newparticle(const vec &o, const vec &d, int fade, int type, int color, float size, int gravity = 0, int sizemod = 0, bool hud = false)
{
    static particle dummy;
    if(seedemitter)
    {
        parts[type]->seedemitter(*seedemitter, o, d, fade, size, gravity);
        return &dummy;
    }
    if(fade + emitoffset < 0) return &dummy;
    addedparticles++;
    return parts[type]->addpart(o, d, fade, color, size, gravity, sizemod, hud);
}

VARP(maxparticledistance, 256, 1024, 4092);

static void splash(int type, int color, int radius, int num, int fade, const vec &p, float size, int gravity, int sizemod, bool upsplash = false)
{
    if(camera1->o.dist(p) > (parts[type]->type&PT_NOMAXDIST ? 9999 : maxparticledistance) && !seedemitter) return;
    float collidez = parts[type]->type&PT_COLLIDE ? p.z - raycube(p, vec(0, 0, -1), COLLIDERADIUS, RAY_CLIPMAT|RAY_POLY) + (parts[type]->stain >= 0 ? COLLIDEERROR : 0) : -1;
    int fmin = 1;
    int fmax = fade*3;
    loopi(num)
    {
        int x, y, z;
        do
        {
            x = rnd(radius*2)-radius;
            y = rnd(radius*2)-radius;
            z = rnd(radius*2)-radius;
        }
        while(x*x+y*y+z*z>radius*radius);
        vec tmp = vec(upsplash ? (float)x/3.5f : (float)x, upsplash ? (float)y/3.5f : (float)y, (float)z<0 && upsplash ? (float)z==(float)z/-1 : (float)z);
        int f = (num < 10) ? (fmin + rnd(fmax)) : (fmax - (i*(fmax-fmin))/(num-1)); //help deallocater by using fade distribution rather than random
        newparticle(p, tmp, f, type, color, size, upsplash ? gravity*3 : gravity, sizemod)->val = collidez;
    }
}

void regularsplash(int type, int color, int radius, int num, int fade, const vec &p, float size, int gravity, int delay, int sizemod, bool upsplash)
{
    if(minimized || (delay > 0 && rnd(delay) != 0)) return;
    splash(type, color, radius, num, fade, p, size, gravity, sizemod, upsplash);
}

void particle_flying_flare(const vec &o, const vec &d, int fade, int type, int color, float size, int gravity, int sizemod, bool randomcolor)
{
    newparticle(o, d, fade, type, randomcolor ? gfx::rndcolor[rnd(6)].color : color, size, gravity, sizemod);
}

bool canaddparticles()
{
    return !minimized;
}

void regular_particle_splash(int type, int num, int fade, const vec &p, int color, float size, int radius, int gravity, int delay)
{
    if(!canaddparticles()) return;
    regularsplash(type, color, radius, num, fade, p, size, gravity, delay);
}

void particle_splash(int type, int num, int fade, const vec &p, int color, float size, int radius, int gravity, int sizemod, bool randomcolor)
{
    if(!canaddparticles()) return;
    splash(type, randomcolor ? gfx::rndcolor[rnd(6)].color : color, radius, num, fade, p, size, gravity, sizemod);
}

VARP(maxtrail, 1, 2000, 10000);

void particle_trail(int type, int fade, const vec &s, const vec &e, int color, float size, int gravity)
{
    if(!canaddparticles()) return;
    vec v;
    float d = e.dist(s, v);
    int steps = clamp(int(d*2), 1, maxtrail);
    v.div(steps);
    vec p = s;
    loopi(steps)
    {
        p.add(v);
        vec tmp = vec(float(rnd(11)-5), float(rnd(11)-5), float(rnd(11)-5));
        newparticle(p, tmp, rnd(fade)+fade, type, color, size, gravity);
    }
}

vec computepartpos(const vec &s)
{
    vec pdir = s;
    vec cpos = camera1->o;
    pdir.sub(cpos);
    pdir.normalize();
    pdir.mul(2);
    return cpos.add(pdir);
}

VARP(particletext, 0, 1, 1);
VARP(maxparticletextdistance, 0, 4096, 10000);

void particle_text(const vec &s, const char *t, int type, int fade, int color, float size, int gravity)
{
    if(!canaddparticles()) return;
    if(!particletext || camera1->o.dist(s) > maxparticletextdistance) return;
    particle *p = newparticle(s, vec(0, 0, 1), fade, type, color, size, gravity);
    p->text = t;
}

void particle_textcopy(const vec &s, const char *t, int type, int fade, int color, float size, int gravity, bool hud)
{
    if(((!particletext || camera1->o.dist(s) > maxparticletextdistance) || !canaddparticles()) && !hud) return;
    particle *p = newparticle(hud ? computepartpos(s) : s, vec(0, 0, 1), fade, type, color, size, gravity, hud);
    p->text = newstring(t);
    p->flags = 1;
}

void particle_icon(const vec &s, int ix, int iy, int type, int fade, int color, float size, int gravity)
{
    if(!canaddparticles()) return;
    particle *p = newparticle(s, vec(0, 0, 1), fade, type, color, size, gravity);
    p->flags |= ix | (iy<<2);
}

void particle_hud(int type, const vec &pos, int color, float size)
{
    if(!canaddparticles()) return;
    newparticle(computepartpos(pos), vec(0, 0, 1), 1, type, color, size, 0);
}

void particle_meter(const vec &s, float val, int type, int fade, int color, int color2, float size, bool ui)
{
    if(!canaddparticles()) return;

    particle *p = newparticle(ui ? computepartpos(s) : s, vec(0, 0, 1), fade, type, color, size);
    p->color2[0] = color2>>16;
    p->color2[1] = (color2>>8)&0xFF;
    p->color2[2] = color2&0xFF;
    p->progress = clamp(int(val*100), 0, 100);
}

void particle_flare(const vec &p, const vec &dest, int fade, int type, int color, float size, physent *owner, bool randomcolor, int sizemod)
{
    if(!canaddparticles()) return;
    newparticle(p, dest, fade, type, randomcolor ? gfx::rndcolor[rnd(6)].color : color, size, 0, sizemod)->owner = owner;
}

void particle_fireball(const vec &dest, float maxsize, int type, int fade, int color, float size, bool randomcolor)
{
    if(!canaddparticles()) return;
    float growth = maxsize - size;
    if(fade < 0) fade = int(growth*20);
    newparticle(dest, vec(0, 0, 1), fade, type, randomcolor ? gfx::rndcolor[rnd(6)].color : color, size)->val = growth;
}

//dir = 0..6 where 0=up
static inline vec offsetvec(vec o, int dir, int dist)
{
    vec v = vec(o);
    v[(2+dir)%3] += (dir>2)?(-dist):dist;
    return v;
}

//converts a 16bit color to 24bit
static inline int colorfromattr(int attr)
{
    return (((attr&0xF)<<4) | ((attr&0xF0)<<8) | ((attr&0xF00)<<12)) + 0x0F0F0F;
}

/* Experiments in shapes...
 * dir: (where dir%3 is similar to offsetvec with 0=up)
 * 0..2 circle
 * 3.. 5 cylinder shell
 * 6..11 cone shell
 * 12..14 plane volume
 * 15..20 line volume, i.e. wall
 * 21 sphere
 * 24..26 flat plane
 * +32 to inverse direction
 */
void regularshape(int type, int radius, int color, int dir, int num, int fade, const vec &p, float size, int gravity, float vel, int windoffset, bool weather, int height, int sizemod)
{
    if(!canemitparticles()) return;

    int basetype = parts[type]->type&0xFF;
    bool flare = (basetype == PT_TAPE) || (basetype == PT_LIGHTNING),
         inv = (dir&0x20)!=0, taper = (dir&0x40)!=0 && !seedemitter;
    dir &= 0x1F;
    loopi(num)
    {
        vec to, from;
        if(dir < 12)
        {
            const vec2 &sc = sincos360[rnd(360)];
            to[dir%3] = sc.y*radius;
            to[(dir+1)%3] = sc.x*radius;
            to[(dir+2)%3] = 0.0;
            to.add(p);
            if(dir < 3) //circle
                from = p;
            else if(dir < 6) //cylinder
            {
                from = to;
                to[(dir+2)%3] += radius;
                from[(dir+2)%3] -= radius;
            }
            else //cone
            {
                from = p;
                to[(dir+2)%3] += (dir < 9)?radius:(-radius);
            }
        }
        else if(dir < 15) //plane
        {
            to[dir%3] = float(rnd(radius<<4)-(radius<<3))/8.0;
            to[(dir+1)%3] = float(rnd(radius<<4)-(radius<<3))/8.0;
            to[(dir+2)%3] = radius;
            to.add(p);
            from = to;
            from[(dir+2)%3] -= 2*radius;
        }
        else if(dir < 21) //line
        {
            if(dir < 18)
            {
                to[dir%3] = float(rnd(radius<<4)-(radius<<3))/8.0;
                to[(dir+1)%3] = 0.0;
            }
            else
            {
                to[dir%3] = 0.0;
                to[(dir+1)%3] = float(rnd(radius<<4)-(radius<<3))/8.0;
            }
            to[(dir+2)%3] = 0.0;
            to.add(p);
            from = to;
            to[(dir+2)%3] += radius;
        }
        else if(dir < 24) //sphere
        {
            to = vec(2*M_PI*float(rnd(1000))/1000.0, M_PI*float(rnd(1000)-500)/1000.0).mul(radius);
            to.add(p);
            from = p;
        }
        else if(dir < 27) // flat plane
        {
            to[dir%3] = float(rndscale(2*radius)-radius);
            to[(dir+1)%3] = float(rndscale(2*radius)-radius);
            to[(dir+2)%3] = 0.0;
            to.add(p);
            from = to;
        }
        else from = to = p;

        if(inv) swap(from, to);

        if(weather)
        {
            vec toz(to.x, to.y, camera1->o.z);
            if(camera1->o.dist(toz) > 192*(particleslod+1) && !seedemitter) continue;

            float z = camera1->o.z + height;
            vec spawnz(to.x, to.y, z);

            vec d(spawnz);
            d.sub(from);
            if(windoffset) d.add(vec(windoffset/2+rnd(windoffset), windoffset/2+rnd(windoffset), 0));
            d.normalize().mul(-vel); //velocity
            particle *np = newparticle(spawnz, d, fade, type, color, size, gravity, sizemod);
            np->val = spawnz.z;
        }
        else
        {
			if(taper)
			{
				float dist = clamp(from.dist2(camera1->o)/(parts[type]->type&PT_NOMAXDIST ? 9999 : maxparticledistance), 0.0f, 1.0f);
				if(dist > 0.2f)
				{
					dist = 1 - (dist - 0.2f)/0.8f;
					if(rnd(0x10000) > dist*dist*0xFFFF) continue;
				}
			}

			if(flare)
				newparticle(from, to, rnd(fade*3)+1, type, color, size, gravity);
			else
			{
				vec d = vec(to).sub(from).rescale(vel); //velocity
				particle *n = newparticle(from, d, rnd(fade*3)+1, type, color, size, gravity);
				if(parts[type]->type&PT_COLLIDE)
					n->val = from.z - raycube(from, vec(0, 0, -1), parts[type]->stain >= 0 ? COLLIDERADIUS : max(from.z, 0.0f), RAY_CLIPMAT) + (parts[type]->stain >= 0 ? COLLIDEERROR : 0);
			}
		}
    }
}

void particle_explodesplash(const vec &o, int fade, int type, int color, int size, int gravity, int num)
{
    regularshape(type, 16, color, 22, num, fade, o, size, gravity, num);
}

void regular_particle_flame(int type, const vec &p, float radius, float height, int color, int density, float scale, float speed, float fade, int gravity)
{
    if(!canaddparticles()) return;
    regularflame(type, p, radius, height, color, density, scale, speed, fade, gravity);
}

void regularflame(int type, const vec &p, float radius, float height, int color, int density, float scale, float speed, float fade, int gravity)
{
    if(!canemitparticles()) return;

    float size = scale * min(radius, height)*1.5f;
    vec v(0, 0, min(1.0f, height)*speed);
    loopi(density)
    {
        vec s = p;
        s.x += rndscale(radius*2.0f)-radius;
        s.y += rndscale(radius*2.0f)-radius;
        newparticle(s, v, rnd(max(int(fade*height), 1))+1, type, color, size, gravity, -3.f);
    }
}

void regularflare(const vec &p, int color, int flaresize, int viewdist)
{
    vec pos = p;
    flares.addflare(pos, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, flaresize, viewdist, false, true);
}

static void makeparticles(entity &e)
{
    if(!isconnected()) return;
    switch(e.attr1)
    {
        case 0: //fire and smoke -  <radius> <height> <rgb> - 0 values default to compat for old maps
        {
            float radius = e.attr2 ? float(e.attr2)/100.0f : 1.5f,
                  height = e.attr3 ? float(e.attr3)/100.0f : radius/3;
            int rndflamecolor, rndsmokecolor;
            switch(rnd(3))
            {
                case 1: rndflamecolor = 0x444422; rndsmokecolor = 0x222222; break;
                case 2: rndflamecolor = 0x363636; rndsmokecolor = 0x111111; break;
                default: rndflamecolor = 0x663515; rndsmokecolor = 0x303020; break;
            }
            regularflame(PART_FLAME, e.o, radius, height, e.attr4 ? colorfromattr(e.attr4) : rndflamecolor, 2, 2.0f+(rnd(2)));
            regularflame(PART_SMOKE, vec(e.o.x, e.o.y, e.o.z + 4.0f*min(radius, height)), radius, height, rndsmokecolor, 1, 4.0f+(rnd(6)), 100.0f, 2750.0f, -15);
            if(e.attr5==0 && randomevent(0.16f*gfx::nbfps)) regularsplash(PART_FIRESPARK, 0xFFFF55, 125, rnd(3)+1, 750+(rnd(750)), offsetvec(e.o, rnd(12)-rnd(24), rnd(12)-rnd(10)), 0.5f+(rnd(18)/12.f), -10);
            break;
        }
        case 1: //steam vent - <dir>
            regularsplash(PART_STEAM, colorfromattr(e.attr3), 50, 1, 200, offsetvec(e.o, e.attr2, rnd(10)), 2.4f, -20);
            break;
        case 2: //water fountain - <dir>
        {
            int color;
            if(e.attr3 > 0) color = colorfromattr(e.attr3);
            else
            {
                int mat = MAT_WATER + clamp(-e.attr3, 0, 3);
                color = getwaterfallcolour(mat).tohexcolor();
                if(!color) color = getwatercolour(mat).tohexcolor();
            }
            regularsplash(PART_WATER, color, 150, 4, 200, offsetvec(e.o, e.attr2, rnd(10)), 0.6f, 2);
            break;
        }
        case 3: //fire ball - <size> <rgb>
            newparticle(e.o, vec(0, 0, 1), 1, PART_EXPLOSION, colorfromattr(e.attr3), 4.0f)->val = 1+e.attr2;
            break;
        case 4:  //tape - <dir> <length> <rgb>
        case 7:  //lightning
        case 9:  //steam
        case 10: //water
            break;

        case 5: //meter, metervs - <percent> <rgb> <rgb2>
        case 6:
        {
            particle *p = newparticle(e.o, vec(0, 0, 1), 1, e.attr1==5 ? PART_METER : PART_METER_VS, colorfromattr(e.attr3), 2.0f);
            int color2 = colorfromattr(e.attr4);
            p->color2[0] = color2>>16;
            p->color2[1] = (color2>>8)&0xFF;
            p->color2[2] = color2&0xFF;
            p->progress = clamp(int(e.attr2), 0, 100);
            break;
        }
        case 11: // flame <radius> <height> <rgb> - radius=100, height=100 is the classic size
            regularflame(PART_FLAME, e.o, float(e.attr2)/100.0f, float(e.attr3)/100.0f, colorfromattr(e.attr4), 3, 2.0f);
            break;
        case 12: // smoke plume <radius> <height> <rgb>
            regularflame(PART_SMOKE, e.o, float(e.attr2)/100.0f, float(e.attr3)/100.0f, colorfromattr(e.attr4), 1, 4.0f, 100.0f, 2000.0f, -20);
            break;
        case 14: //Clouds/Nuages
            newparticle(e.o, offsetvec(e.o, e.attr4, 1000*3+(e.attr5*300)), 1, PART_CLOUD1+e.attr2, partcloudcolour, 100+(e.attr3*10));
            break;
        case 15: //Rainbow/Arc-en-ciel
            if(map_atmo==8) newparticle(e.o, offsetvec(e.o, e.attr4, 1000*3+(e.attr5*300)), 1, PART_RAINBOW, 0xAAAAAA, 100+(e.attr3*10));
            break;
        case 16:
            {
                if(randomevent(5*gfx::nbfps))
                {
                    vec pos = e.o;
                    pos.add(vec(-30+rnd(30), -30+rnd(30), -5));
                    playSound(S_LAVASPLASH, &e.o, 300, 100);
                    loopi(6)regularsplash(PART_FIRESPARK, 0xFFBB55, 800+rnd(600), 10, 300+(rnd(500)), pos, 3.f+(rnd(30)/6.f), 200, 0, -3.f, true);
                    loopi(4)regularsplash(PART_SMOKE, 0x333333, 400, 3, 1000+(rnd(1000)), pos, 8.f+(rnd(8)), -20, 0, 18.f, true);
                    loopi(2)particle_fireball(pos, 20, PART_EXPLOSION, 500, 0xFF9900, 2.5f, false);
                }
            }
            break;

        case 17: //rain
            {
                if(map_atmo==4 || map_atmo==8) regularshape(PART_RAIN, max(1+e.attr2, 1), 0x555566, 44, map_atmo==8 ? e.attr3/2 : e.attr3, 10000, e.o, 5+(rnd(3)), 200, -900, e.attr5, true, 200);
                if(randomevent(25*gfx::nbfps) && map_atmo == 4 && isconnected())
                {
                    vec possky = e.o; vec posground = e.o;
                    int posx = -1250+rnd(2500), posy = -1250+rnd(2500);
                    possky.add(vec(posx+(-200+(rnd(400))), posy+(-200+(rnd(400))), 800));
                    posground.add(vec(posx, posy, -50));
                    loopi(2)particle_flare(possky, posground, 750, PART_LIGHTNING, 0x8888FF, 15.f+rnd(10), NULL, gfx::champicolor());

                    playSound(S_ECLAIRPROCHE, &posground, 400, 100);

                    vec posA = possky;
                    vec posB = camera1->o;
                    vec flashloc = (posA.add((posB.mul(vec(3, 3, 3))))).div(vec(4, 4, 4));

                    if(camera1->o.dist(posground) >= 250) playSound(S_ECLAIRPROCHE, &flashloc, 1500, 300);
                    adddynlight(flashloc, 4000, vec(1.5f, 1.5f, 2.0f), 200, 40, L_NOSHADOW, 2000, vec(0.5f, 0.5f, 1.0f));
                }
            }
            break;

        case 18: //snow
            if(map_atmo==8 || map_atmo==9) regularshape(PART_SNOW, max(1+e.attr2, 1), colorfromattr(e.attr4), 44, map_atmo==9 ? e.attr3 : e.attr3/1.75f, 10000, e.o, 2+(rnd(3)), 200, -400, e.attr5, true, 200);
            break;

        case 19: //apocalypse
            {
                if(map_sel==5);
                else if(map_atmo!=5) return;

                regularshape(PART_SMOKE, max(1+e.attr2, 1), map_sel==5 ? 0x352412 : 0x184418, 44, 3, 10000, e.o, 0.5f, 200, -200, -2500, true, 500, 22.f);
                regularshape(PART_FIRESPARK, max(1+e.attr2, 1), colorfromattr(e.attr4), 44, e.attr3, 10000, e.o, 2+(rnd(3)), 200, -400, e.attr5, true, 300);

                if(randomevent(6*gfx::nbfps) && map_sel!=5 && isconnected())
                {
                    vec possky = e.o; vec posground = e.o;
                    int posx = -1250+rnd(2500), posy = -1250+rnd(2500);
                    possky.add(vec(posx+(-200+(rnd(400))), posy+(-200+(rnd(400))), 800));
                    posground.add(vec(posx, posy, -50));
                    particle_flare(possky, posground, 1000, PART_LIGHTNING, 0xFF6622, 15.f+rnd(10), NULL, gfx::champicolor());
                    playSound(S_ECLAIRPROCHE, &posground, 500, 50);

                    vec posA = possky;
                    vec posB = camera1->o;
                    vec flashloc = (posA.add((posB.mul(vec(3, 3, 3))))).div(vec(4, 4, 4));

                    if(camera1->o.dist(posground) >= 250) playSound(S_ECLAIRLOIN, &flashloc, 1000, 250);
                    if(camera1->o.dist(posground) < 1500) adddynlight(flashloc, 4000, vec(1.5f, 0.5f, 0.0f), 200, 40, L_NOSHADOW, 1000, vec(0.5f, 1.0f, 0.5f));
                }
            }
            break;

        case 32: //lens flares - plain/sparkle/sun/sparklesun <red> <green> <blue> <size>
        case 33:
        case 34:
        case 35:
            flares.addflare(e.o, e.attr2, e.attr3, e.attr4, e.attr5, e.attr5, (e.attr1&0x02)!=0, (e.attr1&0x01)!=0);
            break;

        case 51:    //mun dust
            loopi((particleslod*13)+3) regularshape(PART_SMOKE, max(1+e.attr2, 1), 0xBBBBBB, 44, 6, 4000, e.o, 0.5f, -50, 30, 0, true, -50, 18.f);
            break;
        case 52:    //volcano smoke
            loopi((particleslod*13)+3) regularshape(PART_SMOKE, max(1+e.attr2, 1), 0x181818, 44, 6, 4000, e.o, 0.5f, -50, 30, 500, true, -30, 18.f);
            break;

        default:
            if(!editmode)
            {
                defformatstring(ds, "particles %d?", e.attr1);
                particle_textcopy(e.o, ds, PART_TEXT, 1, 0x6496FF, 2.0f);
            }
            break;
    }
}

bool printparticles(extentity &e, char *buf, int len)
{
    switch(e.attr1)
    {
        case 0: case 4: case 7: case 8: case 9: case 10: case 11: case 12: case 13:
            nformatstring(buf, len, "%s %d %d %d 0x%.3hX %d", entities::entname(e.type), e.attr1, e.attr2, e.attr3, e.attr4, e.attr5);
            return true;
        case 3:
            nformatstring(buf, len, "%s %d %d 0x%.3hX %d %d", entities::entname(e.type), e.attr1, e.attr2, e.attr3, e.attr4, e.attr5);
            return true;
        case 5: case 6:
            nformatstring(buf, len, "%s %d %d 0x%.3hX 0x%.3hX %d", entities::entname(e.type), e.attr1, e.attr2, e.attr3, e.attr4, e.attr5);
            return true;
    }
    return false;
}

void seedparticles()
{
    renderprogress(0, "seeding particles");
    addparticleemitters();
    canemit = true;
    loopv(emitters)
    {
        particleemitter &pe = emitters[i];
        extentity &e = *pe.ent;
        seedemitter = &pe;
        for(int millis = 0; millis < seedmillis; millis += min(emitmillis, seedmillis/10))
            makeparticles(e);
        seedemitter = NULL;
        pe.lastemit = -seedmillis;
        pe.finalize();
    }
}

VARR(sunflarex, -1000, -400, 1000);
VARR(sunflarey, -1000, 360, 1000);
VARR(sunflarez, -1000, 135, 1000);
CVARR(sunflarecolour, 0x000000);

static const char * const enthudnames[] =
{
    "invalide?", "none?", "Lumière", "Light", "Modèle 3D", "3D model", "Point de réapparition","Respawn point",
    "Placage d'environnement", "Environment map", "Effet de particules", "Particles effect", "Son", "Sound", "Spot de lumière", "Spotlight", "Projection", "Decal",

    "Fusil électrique", "Electric Rifle", "Fusil plasma", "Plasma rifle", "SMAW", "SMAW", "Minigun", "Minigun", "Pistolet spock", "Spockgun",
    "M32", "M32", "Lance-flammes", "Flamethrower", "UZI", "UZI", "FAMAS", "FAMAS", "Mossberg 500", "Mossberg 500", "Hydra", "Hydra",
    "SV-98", "SV-98", "SKS", "SKS", "Arbalète", "Crossbow", "AK-47", "AK-47", "GAPB-1", "GAPB-1", "Feux d'artifice", "Fireworks", "Glock 45", "Glock 45",
    "Super-arme", "Superweapon", "invalide?", "none?", "invalide?", "none?", "invalide?", "none?",

    "Santé [25]", "Health [25]", "Boost de santé [50]", "Health boost [50]", "Stéroïdes [Dégâts]", "Steroïds [Damages]", "Champis [Cadence]", "Shrooms [Cadency]",
    "EPO [Vitesse]", "EPO [Speed]", "Joint [Résistance]", "Joint [Resistance]", "Bois [75]", "Wood [75]", "Fer [125]", "Iron [125]", "Or [200]", "Gold [200]",
    "Magnétique [150]", "Magnetic [150]", "Armure assistée [300]", "Power armor [300]", "Mana [25]", "Mana [25]",

    "Téléporation [Entrée]", "Teleport [In]", "Téléportation [Destination]", "Téléportation [Destination]", "Trampoline", "Jump pad", "Drapeau", "Flag",
    "Base", "Base", "PNJ [Solo]", "NPC [SP]", "Point de réapparition [Solo]", "Respawn point [SP]", "Zone de déclencheur [Solo]", "Trigger zone [SP]", "Caméra", "Camera",
};

void updateparticles()
{
    if(regenemitters) addparticleemitters();

    if(minimized) { canemit = false; return; }

    if(lastmillis - lastemitframe >= emitmillis)
    {
        canemit = true;
        lastemitframe = lastmillis - (lastmillis%emitmillis);
    }
    else canemit = false;

    loopi(sizeof(parts)/sizeof(parts[0]))
    {
        parts[i]->update();
    }

    if(!editmode || showparticles)
    {
        int emitted = 0, replayed = 0;
        addedparticles = 0;
        loopv(emitters)
        {
            particleemitter &pe = emitters[i];
            extentity &e = *pe.ent;
            if(e.o.dist(camera1->o) > (e.attr1==14 || e.attr1==15 ? 9999 : maxparticledistance)) { pe.lastemit = lastmillis; continue; }
            if(cullparticles && pe.maxfade >= 0)
            {
                if(isfoggedsphere(pe.radius, pe.center)) { pe.lastcull = lastmillis; continue; }
                if(pvsoccluded(pe.cullmin, pe.cullmax)) { pe.lastcull = lastmillis; continue; }
            }
            makeparticles(e);
            emitted++;
            if(replayparticles && pe.maxfade > 5 && pe.lastcull > pe.lastemit)
            {
                for(emitoffset = max(pe.lastemit + emitmillis - lastmillis, -pe.maxfade); emitoffset < 0; emitoffset += emitmillis)
                {
                    makeparticles(e);
                    replayed++;
                }
                emitoffset = 0;
            }
            pe.lastemit = lastmillis;
        }

        if(sunflarecolour != bvec(0, 0, 0)) //sunflare
        {
            float flaredist = 1250; // distance from the camera to the flare

            vec flaredir(
                flaredist * cos(sunlightpitch * M_PI / 180.0f) * cos(fmod(sunlightyaw + 90.0f, 360.0f) * M_PI / 180.0f),
                flaredist * cos(sunlightpitch * M_PI / 180.0f) * sin(fmod(sunlightyaw + 90.0f, 360.0f) * M_PI / 180.0f),
                flaredist * sin(sunlightpitch * M_PI / 180.0f)
            );

            vec flarepos = camera1->o;
            flarepos.add(flaredir);

            flares.addflare(flarepos, sunflarecolour.r, sunflarecolour.g, sunflarecolour.b, 125, 4000, true, true);
        }

        if(dbgpcull && (canemit || replayed) && addedparticles) conoutf(CON_DEBUG, "%d emitters, %d particles", emitted, addedparticles);
    }
    if(editmode) // show sparkly thingies for map entities in edit mode
    {
        const vector<extentity *> &ents = entities::getents();
        // note: order matters in this case as particles of the same type are drawn in the reverse order that they are added
        //loopv(entgroup)
        //{
        //    entity &e = *ents[entgroup[i]];
        //    particle_textcopy(e.o, entname(e), PART_TEXT, 1, 0xFF4B19, 2.5f);
        //}
        loopv(ents)
        {
            entity &e = *ents[i];
            if(e.type==ET_EMPTY) continue;

            vec dir = vec(camera1->o).sub(e.o);
            float dist = dir.magnitude();
            dir.mul(1/dist);
            if(raycube(e.o, dir, dist, RAY_CLIPMAT|RAY_POLY) < dist) continue;          //occlusion check

            vec entpos = e.o;
            vec campos = camera1->o;
            vec partpos = (entpos.add((campos.mul(vec(1, 1, 1))))).div(vec(2, 2, 2));   //bring ent info closer to camera

            int partcol = e.type == TRIGGER_ZONE || e.type == RESPAWNPOINT || e.type == MONSTER ? 0xCCCC00 : 0xBBBBBB;

            switch(e.type)
            {
                case MAPMODEL:
                {
                    defformatstring(txt, "%s (\fg%s\f7)", enthudnames[(e.type*2)+GAME_LANG], mapmodelname(e.attr1));
                    particle_textcopy(partpos.addz(1), txt, PART_TEXT, 1, 0xFFFFFF, 1.25f);
                    break;
                }
                case ET_LIGHT:
                {
                    unsigned int color = ((e.attr2 & 0xff) << 16) + ((e.attr3 & 0xff) << 8) + (e.attr4 & 0xff);
                    particle_textcopy(partpos.addz(1), enthudnames[(e.type*2)+GAME_LANG], PART_TEXT, 1, color, 1.25f);
                    break;
                }
                case ET_PLAYERSTART: case FLAG:
                {
                    defformatstring(txt, "%s%s", e.attr2==0 ? "\fe" : e.attr2==1? "\fd" : "\fc", enthudnames[(e.type*2)+GAME_LANG]);
                    particle_textcopy(partpos.addz(1), txt, PART_TEXT, 1, 0xFFFFFF, 1.25f);
                    partcol = e.attr2==0 ? 0x00FF00 : e.attr2==1? 0xFFFF00 : 0xFF0000;
                    break;
                }
                case BASE:
                {
                    defformatstring(alias, GAME_LANG ? "base_en_%d" : "base_fr_%d", e.attr2);
                    const char *name = getalias(alias);
                    defformatstring(basename, name);
                    defformatstring(txt, "%s - %s", enthudnames[(e.type*2)+GAME_LANG], basename);
                    particle_textcopy(partpos.addz(1), txt, PART_TEXT, 1, 0xFFFFFF, 1.25f);
                    partcol = 0x00FF00;
                    break;
                }
                case MAPSOUND:
                {
                    defformatstring(txt, "%s (\fg%s\f7)", enthudnames[(e.type*2)+GAME_LANG], getmapsoundname(e.attr1));
                    particle_textcopy(partpos.addz(1), txt, PART_TEXT, 1, 0xFFFFFF, 1.25f);
                    break;
                }
                default:
                {
                    string gameenttype = "";
                    if(e.type >= I_RAIL && e.type <= I_SUPERARME) formatstring(gameenttype, "%s", GAME_LANG ? "Weapon" : "Arme");
                    else if(e.type==I_SANTE || e.type==I_MANA) formatstring(gameenttype, "%s", GAME_LANG ? "Item" : "Objet");
                    else if(e.type >= I_WOODSHIELD && e.type <= I_POWERARMOR) formatstring(gameenttype, "%s", GAME_LANG ? "Shield" : "Bouclier");
                    else if(e.type >= I_BOOSTPV && e.type <= I_JOINT) formatstring(gameenttype, "%s", GAME_LANG ? "Boost" : "Boost");
                    defformatstring(txt, "%s%s%s%s", gameenttype, strcmp(gameenttype, "") ? " (\ff" : "", enthudnames[(e.type*2)+GAME_LANG], strcmp(gameenttype, "") ? "\f7)" : "");
                    particle_textcopy(partpos.addz(1), txt, PART_TEXT, 1, 0xFFFFFF, 1.25f);
                    if(strcmp(gameenttype, "")) partcol = 0x0000FF;
                }
            }
            newparticle(partpos.addz(-1), partpos.addz(-1), 1, PART_BASIC, partcol, 1.f);
        }
    }
}
