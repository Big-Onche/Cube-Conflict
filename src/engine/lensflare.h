static const struct flaretype
{
    int type;             /* flaretex index, 0..5, -1 for 6+random shine */
    float loc;            /* postion on axis */
    float scale;          /* texture scaling */
    uchar alpha;          /* color alpha */
} flaretypes[] =
{
    {2,  1.30f, 0.04f, 153}, //flares
    {0,  1.00f, 0.10f, 102},
    {1,  0.50f, 0.20f, 77},
    {2,  0.20f, 0.05f, 77},
    {1,  0.00f, 0.10f, 150},
    {5, -0.25f, 0.07f, 75},
    {5, -0.40f, 0.02f, 100},
    {1, -0.50f, 0.03f, 105},
    {2, -0.60f, 0.04f, 102},
    {5, -1.00f, 0.03f, 51},
    {-1, 1.00f, 0.25f, 255}, //shine - red, green, blue
    {-2, 1.00f, 0.25f, 255},
    {-3, 1.00f, 0.25f, 255}
};

struct flare
{
    vec o, center;
    float size;
    bvec color;
    bool sparkle;
};

struct flarerenderer : partrenderer
{
    int maxflares, numflares;
    unsigned int shinetime;
    flare *flares;

    flarerenderer(const char *texname, int maxflares, int flags = 0)
        : partrenderer(texname, 3, PT_FLARE|PT_NOLAYER|flags), maxflares(maxflares), numflares(0), shinetime(0)
    {
        flares = new flare[maxflares];
    }
    ~flarerenderer()
    {
        delete[] flares;
    }

    void reset()
    {
        numflares = 0;
    }

    float flarefade = 0.f;

    void newflare(vec &o,  const vec &center, uchar r, uchar g, uchar b, float mod, float size, bool sun, bool sparkle)
    {
        if(numflares >= maxflares) return;
        //occlusion check (neccessary as depth testing is turned off)
        vec dir = vec(camera1->o).sub(o);
        float dist = dir.magnitude();
        dir.mul(1/dist);
        if(raycube(o, dir, dist, RAY_CLIPMAT|RAY_POLY) < dist) //simple fade in/out when flare position (=sun) is occluded
        {
            flarefade-=flarefade>0 ? 0.1f : 0;
            if(flarefade<=0) return;
        }
        else if (flarefade<1.f) flarefade+=0.1f;

        flare &f = flares[numflares++];
        f.o = o;
        f.center = center;
        f.size = size;
        f.color = bvec(uchar(r*mod*flarefade), uchar(g*mod*flarefade), uchar(b*mod*flarefade));
        f.sparkle = sparkle;
    }

    void addflare(vec &o, uchar r, uchar g, uchar b, int flaresize, int viewdist, bool sun, bool sparkle)
    {
        //frustrum + fog check
        if(isvisiblesphere(0.0f, o) > (sun?VFC_FOGGED:VFC_FULL_VISIBLE)) return;
        //find closest point between camera line of sight and flare pos
        vec flaredir = vec(o).sub(camera1->o);
        vec center = vec(camdir).mul(flaredir.dot(camdir)).add(camera1->o);
        float mod, size;

        mod = ((viewdist*100)-vec(o).sub(center).squaredlen())/(viewdist*100);
        if(mod < 0.0f) return;
        size = sun ? flaredir.magnitude() * flaresize / 100.0f : flaresize / 5.0f; //fixed size for sun

        newflare(o, center, r, g, b, mod, size, sun, sparkle);
    }

    void update()
    {
        numflares = 0; //regenerate flarelist each frame
        //shinetime = 0;
    }

    int count()
    {
        return numflares;
    }

    bool haswork()
    {
        return (numflares != 0);
    }

    void render()
    {
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, tex->id);
        gle::defattrib(gle::ATTRIB_VERTEX, 3, GL_FLOAT);
        gle::defattrib(gle::ATTRIB_TEXCOORD0, 2, GL_FLOAT);
        gle::defattrib(gle::ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE);
        gle::begin(GL_QUADS);

        loopi(numflares)
        {
            const flare &f = flares[i];
            vec axis = vec(f.o).sub(f.center);
            bvec4 color(f.color, 255);
            loopj(f.sparkle?12:9)
            {
                const flaretype &ft = flaretypes[j];
                vec o = vec(axis).mul(ft.loc).add(f.center);
                float sz = ft.scale * f.size;
                int tex = ft.type;
                if(ft.type < 0) //sparkles - always done last
                {
                    //shinetime = (shinetime + 1) % 10; //removed old ugly shine effect
                    tex = 6;
                    color.r = 0;
                    color.g = 0;
                    color.b = 0;
                    color[-ft.type-1] = f.color[-ft.type-1]; //only want a single channel
                }
                color.a = ft.alpha;
                const float tsz = 0.25; //flares are aranged in 4x4 grid
                float tx = tsz*(tex&0x03), ty = tsz*((tex>>2)&0x03);
                gle::attribf(o.x+(-camright.x+camup.x)*sz, o.y+(-camright.y+camup.y)*sz, o.z+(-camright.z+camup.z)*sz);
                    gle::attribf(tx,     ty+tsz);
                    gle::attrib(color);
                gle::attribf(o.x+( camright.x+camup.x)*sz, o.y+( camright.y+camup.y)*sz, o.z+( camright.z+camup.z)*sz);
                    gle::attribf(tx+tsz, ty+tsz);
                    gle::attrib(color);
                gle::attribf(o.x+( camright.x-camup.x)*sz, o.y+( camright.y-camup.y)*sz, o.z+( camright.z-camup.z)*sz);
                    gle::attribf(tx+tsz, ty);
                    gle::attrib(color);
                gle::attribf(o.x+(-camright.x-camup.x)*sz, o.y+(-camright.y-camup.y)*sz, o.z+(-camright.z-camup.z)*sz);
                    gle::attribf(tx,     ty);
                    gle::attrib(color);
            }
        }
        gle::end();
        glEnable(GL_DEPTH_TEST);
    }

    //square per round hole - use addflare(..) instead
    particle *addpart(const vec &o, const vec &d, int fade, int color, float size, int gravity = 0, int sizemod = 0) { return NULL; }
};
static flarerenderer flares("<grey>media/particles/misc/lensflares.png", 64);

