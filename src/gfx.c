#include "u8g2.h"
#include "include/gfx.h"
#include <math.h>

static gfx_scene_t current_scene = SCENE_WAVEFORM;

void gfx_set_scene(gfx_scene_t scene)
{
    current_scene = scene;
}

void gfx_update(u8g2_t *u8g2, float t)
{
    u8g2_ClearBuffer(u8g2);

    switch (current_scene)
    {
        case SCENE_WAVEFORM:
            draw_waveform(u8g2, t);
            break;

        case SCENE_CUBE:
            draw_cube(u8g2, t);
            break;

        case SCENE_RORSCHACH:
            draw_rorschach(u8g2, t);
            break;
            
    }

    u8g2_SendBuffer(u8g2);
}


void draw_waveform(u8g2_t *u8g2, float t)
{
    int prev_y = 32;

    for (int x = 0; x < 128; x++)
    {
        float phase = (float)x / 128.0f;

        // sawtooth wave (wraps every period)
        float v = fmodf(phase + t * 0.5f, 1.0f);

        int y = 63 - (int)(v * 63.0f);

        if (x > 0)
        {
            u8g2_DrawLine(u8g2, x - 1, prev_y, x, y);
        }

        prev_y = y;
    }
}


void project(float x, float y, float z, float t, int *sx, int *sy)
{
    // rotate Y
    float cx = x * cosf(t) - z * sinf(t);
    float cz = x * sinf(t) + z * cosf(t);

    // rotate X
    float cy = y * cosf(t * 0.7f) - cz * sinf(t * 0.7f);
    cz = y * sinf(t * 0.7f) + cz * cosf(t * 0.7f);

    float scale = 40.0f / (cz + 3.0f);

    *sx = (int)(64 + cx * scale);
    *sy = (int)(32 + cy * scale);
}


void draw_cube(u8g2_t *u8g2, float t)
{
    float v[8][3] = {
        {-1,-1,-1},{ 1,-1,-1},{ 1, 1,-1},{-1, 1,-1},
        {-1,-1, 1},{ 1,-1, 1},{ 1, 1, 1},{-1, 1, 1}
    };

    int p[8][2];

    for (int i = 0; i < 8; i++)
    {
        project(v[i][0], v[i][1], v[i][2], t, &p[i][0], &p[i][1]);
    }

    int edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };

    for (int i = 0; i < 12; i++)
    {
        u8g2_DrawLine(u8g2,
            p[edges[i][0]][0], p[edges[i][0]][1],
            p[edges[i][1]][0], p[edges[i][1]][1]);
    }
}


/* ----------------------------
   Hash noise
---------------------------- */
static uint32_t hash2(int x, int y)
{
    uint32_t ux = (uint32_t)x;
    uint32_t uy = (uint32_t)y;

    uint32_t h = ux * 374761393u ^ uy * 668265263u;
    h ^= (h >> 13);
    h *= 1274126177u;
    h ^= (h >> 16);

    return h;
}

static float noise(int x, int y)
{
    return hash2(x, y) / 4294967295.0f;
}

/* ----------------------------
   Centered coordinate mapping
---------------------------- */
static void map_coords(float x, float y, float *ox, float *oy)
{
    float nx = (x - 32.0f) / 32.0f;
    float ny = (y - 32.0f) / 32.0f;

    nx = nx * (1.0f - 0.5f * nx * nx);
    ny = ny * (1.0f - 0.5f * ny * ny);

    *ox = nx;
    *oy = ny;
}

/* ----------------------------
   Julia fractal (iterative)
---------------------------- */
static float julia(float zx, float zy, float cx, float cy)
{
    int i;
    const int max_iter = 14;

    for (i = 0; i < max_iter; i++)
    {
        float zx2 = zx*zx - zy*zy + cx;
        float zy2 = 2.0f*zx*zy + cy;

        zx = zx2;
        zy = zy2;

        if (zx*zx + zy*zy > 4.0f)
            break;
    }

    return (float)i / (float)max_iter;
}

/* ----------------------------
   Fractal field (multi-scale + dual Julia)
---------------------------- */
static float fractal_field(float x, float y, float t)
{
    float zx1, zy1;
    float zx2, zy2;

    map_coords(x, y, &zx1, &zy1);

    zx2 = zx1 * 0.5f;
    zy2 = zy1 * 0.5f;

    int mode = ((int)(t * 0.2f)) % 3;

    float c1x, c1y, c2x, c2y;

    if (mode == 0)
    {
        c1x = -0.4f; c1y = 0.1f;
        c2x =  0.285f; c2y = 0.01f;
    }
    else if (mode == 1)
    {
        c1x = -0.8f; c1y = 0.156f;
        c2x = -0.12f; c2y = 0.74f;
    }
    else
    {
        c1x = 0.285f; c1y = 0.01f;
        c2x = -0.4f;  c2y = 0.6f;
    }

    float f1 = julia(zx1 * 1.5f, zy1 * 1.5f, c1x, c1y);
    float f2 = julia(zx2 * 1.5f, zy2 * 1.5f, c2x, c2y);

    return 0.6f * f1 + 0.4f * f2;
}

/* ----------------------------
   Metaball field
---------------------------- */
static float metaball(float x, float y,
                       float cx, float cy,
                       float r)
{
    float dx = x - cx;
    float dy = y - cy;

    float d2 = dx*dx + dy*dy;

    return r*r / (d2 + 0.001f);
}

/* ----------------------------
   Hybrid ink field
---------------------------- */
static float ink_field(float x, float y, float t)
{
    float f = fractal_field(x, y, t);

    float v = 0.0f;

    const int N = 9;

    for (int i = 0; i < N; i++)
    {
        float a = i * 1.618f;

        /* full-screen scaling (fixes clustering) */
        float scale = 6.0f + 18.0f * f;

        float jx = sinf(f * 6.28318f + t);
        float jy = cosf(f * 6.28318f + t * 0.7f);

        float cx = 32.0f + cosf(a + jx) * scale;
        float cy = 32.0f + sinf(a + jy) * scale;

        /* symmetry jitter */
        float jitter = 0.03f * noise((int)x, (int)y);
        cx += jitter;
        cy -= jitter;

        /* smaller, denser ink blobs */
        float r = 1.8f + 2.0f * f;

        v += metaball(x, y, cx, cy, r);
    }

    /* fractal modulation */
    v *= (0.7f + 1.1f * f);

    /* contrast shaping */
    v = v - 1.1f;
    v = v * 2.6f;

    return v;
}

/* ----------------------------
   Renderer (Rorschach symmetry)
---------------------------- */
void draw_rorschach(u8g2_t *u8g2, float t)
{
    float threshold = 0.15f;

    for (int y = 0; y < 64; y++)
    {
        for (int x = 0; x < 64; x++)
        {
            float v = ink_field((float)x, (float)y, t);

            if (v > threshold)
            {
                u8g2_DrawPixel(u8g2, x + 16, y);
                u8g2_DrawPixel(u8g2, 127 - x - 16, y);
            }
        }
    }
}