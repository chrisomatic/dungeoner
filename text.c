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
#include "text.h"

unsigned char ttf_buffer[1<<20];
unsigned char temp_bitmap[512*512];

stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs

static GLuint ftex;

static GLuint uni_location_text;
static GLuint uni_location_text_color;
static GLuint uni_location_proj;

static GLuint text_vao;
static GLuint text_vbo;
static GLuint text_ibo;

void text_init()
{
    fread(ttf_buffer, 1, 1<<20, fopen("fonts/Roboto-Regular.ttf", "rb"));
    int result = stbtt_BakeFontBitmap(ttf_buffer,0, 32.0, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!

    printf("Loaded %d rows of font bitmap.\n", result);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &ftex);
    glBindTexture(GL_TEXTURE_2D, ftex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512,512, 0, GL_RED , GL_UNSIGNED_BYTE, temp_bitmap);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenVertexArrays(1, &text_vao);
    glBindVertexArray(text_vao);

 	glGenBuffers(1, &text_vbo);
    glGenBuffers(1, &text_ibo);

    uni_location_proj = glGetUniformLocation(program_text, "projection");
    uni_location_text = glGetUniformLocation(program_text, "text");
    uni_location_text_color = glGetUniformLocation(program_text, "text_color");

    glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

typedef struct
{
    Vector2f position;
    Vector2f tex_coord;
} CharacterPoint;

typedef struct
{
    CharacterPoint points[4];
} Glyph;

void text_print(float x, float y, char *text, Vector3f color)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);
    glBindVertexArray(text_vao);

    Matrix proj = {0};
    get_ortho_transform(&proj,0.0f, view_width, 0.0f, view_height);

    glUseProgram(program_text);

    shader_set_int(program_text,"text",0);
    glUniformMatrix4fv(uni_location_proj, 1, GL_TRUE, &proj.m[0][0]);
    glUniform1i(uni_location_text, 0);
    glUniform3f(uni_location_text_color, color.x,color.y,color.z);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ftex);

    //glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
    //glBufferData(GL_ARRAY_BUFFER, 4*sizeof(CharacterPoint), NULL, GL_DYNAMIC_DRAW);

    int num_chars = strlen(text);

    Glyph* glyphs = calloc(num_chars,sizeof(Glyph));
    uint32_t*  indices = calloc(num_chars*6,sizeof(uint32_t));

    int i = 0;
    int j = 0;

    while (*text)
    {
        if (*text >= 32 && *text < 128)
        {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, 512,512, *text-32, &x,&y,&q,1);

            glyphs[i].points[0].position.x = q.x0; glyphs[i].points[0].position.y = q.y0;
            glyphs[i].points[1].position.x = q.x1; glyphs[i].points[1].position.y = q.y0;
            glyphs[i].points[2].position.x = q.x1; glyphs[i].points[2].position.y = q.y1;
            glyphs[i].points[3].position.x = q.x0; glyphs[i].points[3].position.y = q.y1;

            /*
            glyphs[i].points[0].position.x = ((q.x0*2.0)/view_width - 1.0f); glyphs[i].points[0].position.y = (q.y0*2.0)/view_height - 1.0f;
            glyphs[i].points[1].position.x = ((q.x1*2.0)/view_width - 1.0f); glyphs[i].points[1].position.y = (q.y0*2.0)/view_height - 1.0f;
            glyphs[i].points[2].position.x = ((q.x1*2.0)/view_width - 1.0f); glyphs[i].points[2].position.y = (q.y1*2.0)/view_height - 1.0f;
            glyphs[i].points[3].position.x = ((q.x0*2.0)/view_width - 1.0f); glyphs[i].points[3].position.y = (q.y1*2.0)/view_height - 1.0f;
            */

            glyphs[i].points[0].tex_coord.x = q.s0; glyphs[i].points[0].tex_coord.y = q.t0;
            glyphs[i].points[1].tex_coord.x = q.s1; glyphs[i].points[1].tex_coord.y = q.t0;
            glyphs[i].points[2].tex_coord.x = q.s1; glyphs[i].points[2].tex_coord.y = q.t1;
            glyphs[i].points[3].tex_coord.x = q.s0; glyphs[i].points[3].tex_coord.y = q.t1;

            int index = i*6;

            indices[index] = j;
            indices[index+1] = j+1;
            indices[index+2] = j+2;
            indices[index+3] = j;
            indices[index+4] = j+2;
            indices[index+5] = j+3;

#if 0 
            printf("C %c P %f %f %f %f %f %f %f %f : %f %f %f %f %f %f %f %f\n",
                *text,
                glyphs[i].points[0].position.x, glyphs[i].points[0].position.y,
                glyphs[i].points[1].position.x, glyphs[i].points[1].position.y,
                glyphs[i].points[2].position.x, glyphs[i].points[2].position.y,
                glyphs[i].points[3].position.x, glyphs[i].points[3].position.y,
                glyphs[i].points[0].tex_coord.x, glyphs[i].points[0].tex_coord.y,
                glyphs[i].points[1].tex_coord.x, glyphs[i].points[1].tex_coord.y,
                glyphs[i].points[2].tex_coord.x, glyphs[i].points[2].tex_coord.y,
                glyphs[i].points[3].tex_coord.x, glyphs[i].points[3].tex_coord.y
            );
#endif
        }
        ++text;
        ++i;
        j += 4;
    }

    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
    glBufferData(GL_ARRAY_BUFFER, num_chars*sizeof(Glyph), glyphs, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(CharacterPoint), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CharacterPoint), (const GLvoid*)8);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_chars*6*sizeof(uint32_t), indices, GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, num_chars*6, GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    free(glyphs);
    free(indices);
            
    glUseProgram(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDisable(GL_BLEND);
}
