#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "util/stb_truetype.h"

#include "common.h"
#include "util.h"
#include "3dmath.h"
#include "shader.h"
#include "settings.h"
#include "gfx.h"
#include "log.h"
#include "text.h"

unsigned char ttf_buffer[1<<20];
unsigned char temp_bitmap[512*512];

stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs

static GLuint t_font;

static GLuint uni_location_text;
static GLuint uni_location_text_color;
static GLuint uni_location_proj;

void text_init()
{
    fread(ttf_buffer, 1, 1<<20, fopen("fonts/Roboto-Regular.ttf", "rb"));
    int result = stbtt_BakeFontBitmap(ttf_buffer,0, 24.0, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!

    LOGI("Loaded %d rows of font bitmap.\n", result);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &t_font);
    glBindTexture(GL_TEXTURE_2D, t_font);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512,512, 0, GL_RED , GL_UNSIGNED_BYTE, temp_bitmap);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    uni_location_proj       = glGetUniformLocation(program_text, "projection");
    uni_location_text       = glGetUniformLocation(program_text, "text");
    uni_location_text_color = glGetUniformLocation(program_text, "text_color");

    glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void text_print_all()
{
    Vector2f pos = {0.5,0.5};
    Vector2f sca = {0.025,0.025};

    //gfx_enable_blending();
    gfx_draw_quad2d(t_font, NULL, &pos, &sca);
    //gfx_disable_blending();

}

void text_print(float x, float y, char *text, Vector3f color)
{
            
}
