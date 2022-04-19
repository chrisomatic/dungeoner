#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <GL/glew.h>

#include "common.h"
#include "3dmath.h"
#include "light.h"
#include "shader.h"
#include "gfx.h"
#include "log.h"

GLuint program_basic;
GLuint program_sky;
GLuint program_terrain;
GLuint program_text;
GLuint program_debug;
GLuint program_particle;
GLuint program_water;
GLuint program_gui;

static void shader_add(GLuint program, GLenum shader_type, const char* shader_file_path);

void shader_load_all()
{
    shader_build_program(&program_basic,"shaders/basic.vert.glsl","shaders/basic.frag.glsl");
    shader_build_program(&program_sky,"shaders/skybox.vert.glsl","shaders/skybox.frag.glsl");
    shader_build_program(&program_terrain,"shaders/basic.vert.glsl","shaders/terrain.frag.glsl");
    shader_build_program(&program_text,"shaders/text.vert.glsl","shaders/text.frag.glsl");
    shader_build_program(&program_debug,"shaders/debug.vert.glsl","shaders/debug.frag.glsl");
    shader_build_program(&program_particle,"shaders/particle.vert.glsl","shaders/particle.frag.glsl");
    shader_build_program(&program_water,"shaders/water.vert.glsl","shaders/water.frag.glsl");
    shader_build_program(&program_gui,"shaders/gui.vert.glsl","shaders/gui.frag.glsl");
}

void shader_deinit()
{
    glDeleteProgram(program_basic);
    glDeleteProgram(program_sky);
    glDeleteProgram(program_terrain);
    glDeleteProgram(program_text);
    glDeleteProgram(program_debug);
    glDeleteProgram(program_particle);
    glDeleteProgram(program_water);
    glDeleteProgram(program_gui);
}

void shader_build_program(GLuint* p, const char* vert_shader_path, const char* frag_shader_path)
{
	*p = glCreateProgram();

    shader_add(*p, GL_VERTEX_SHADER,  vert_shader_path);
    shader_add(*p, GL_FRAGMENT_SHADER,frag_shader_path);

	glLinkProgram(*p);

	GLint success;
    glGetProgramiv(*p, GL_LINK_STATUS, &success);
	if (!success)
    {
        GLchar info[1000+1] = {0};
		glGetProgramInfoLog(*p, 1000, NULL, info);
		LOGE("Error linking shader program: '%s'", info);
        exit(1);
	}

    glValidateProgram(*p);
    glGetProgramiv(*p, GL_VALIDATE_STATUS, &success);
    if (!success) {
        GLchar info[1000+1] = {0};
        glGetProgramInfoLog(*p, 1000, NULL, info);
        LOGE("Invalid shader program: '%s'", info);
        exit(1);
    }

}

void shader_set_variables(GLuint program, Vector* pos, Vector* rot, Vector* sca, Vector4f* clip_plane)
{
    Matrix world, view, proj, wv, wvp;
    get_transforms(pos, rot, sca, &world, &view, &proj);

    get_wvp(&world, &view, &proj, &wvp);
    get_wv(&world, &view, &wv);

    if(program == program_basic)
    {
        shader_set_int(program,"sampler",0);
        shader_set_int(program,"wireframe",show_wireframe);
        shader_set_mat4(program,"wv",&wv);
        shader_set_mat4(program,"wvp",&wvp);
        shader_set_mat4(program,"world",&world);
        shader_set_vec3(program,"dl.color",sunlight.base.color.x, sunlight.base.color.y, sunlight.base.color.z);
        shader_set_vec3(program,"dl.direction",sunlight.direction.x, sunlight.direction.y, sunlight.direction.z);
        shader_set_vec3(program,"sky_color",SKY_COLOR_R, SKY_COLOR_G, SKY_COLOR_B);
        if(clip_plane)
            shader_set_vec4(program,"clip_plane",clip_plane->x, clip_plane->y, clip_plane->z, clip_plane->w);

        shader_set_float(program,"dl.ambient_intensity",sunlight.base.ambient_intensity);
        shader_set_float(program,"dl.diffuse_intensity",sunlight.base.diffuse_intensity);

        if(show_fog)
        {
            shader_set_float(program,"fog_density",fog_density);
            shader_set_float(program,"fog_gradient",fog_gradient);
        }
        else
        {
            shader_set_float(program,"fog_density",0.0);
            shader_set_float(program,"fog_gradient",1.0);
        }

    }
}

static int read_file(const char* filepath, char* ret_buf, uint32_t max_buffer_size)
{
    FILE* fp = fopen(filepath,"r");
    
    if(!fp)
        return -1;

    int c;
    int i = 0;

    for(;;)
    {
        c = fgetc(fp);
        if(c == EOF)
            break;

        ret_buf[i++] = c;

        if(i >= max_buffer_size)
            break;
    }
    return i;
}

static void shader_add(GLuint program, GLenum shader_type, const char* shader_file_path)
{
    // create
	GLuint shader_id = glCreateShader(shader_type);
    if (!shader_id)
    {
        LOGE("Error creating shader type %d", shader_type);
        return;
    }

    char* buf = calloc(MAX_SHADER_LEN+1,sizeof(char));
    if(!buf)
    {
        LOGE("Failed to malloc shader buffer");
        return;
    }

    // load
    int len = read_file(shader_file_path,buf, MAX_SHADER_LEN);
    if(len == 0)
    {
        LOGI("Read zero bytes from shader file.");
        free(buf);
        return;
    }

	// compile
	LOGI("Compiling shader: %s (size: %d bytes)", shader_file_path, len);

	glShaderSource(shader_id, 1, (const char**)&buf, NULL);
	glCompileShader(shader_id);

	// validate
	GLint success;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
	if (!success)
    {
        GLchar info[1000+1] = {0};
		glGetShaderInfoLog(shader_id, 1000, NULL, info);
		LOGE("Error compiling shader type %d: '%s'", shader_type, info);
        free(buf);
        return;
	}

	glAttachShader(program, shader_id);

    free(buf);
}

void shader_set_int(GLuint program, const char* name, int i)
{
    glUniform1i(glGetUniformLocation(program, name), i);
}

void shader_set_float(GLuint program, const char* name, float f)
{
    glUniform1f(glGetUniformLocation(program, name), f);
}

void shader_set_vec3(GLuint program, const char* name, float x, float y, float z)
{
    glUniform3f(glGetUniformLocation(program, name), x,y,z);
}

void shader_set_vec4(GLuint program, const char* name, float x, float y, float z, float w)
{
    glUniform4f(glGetUniformLocation(program, name), x,y,z,w);
}
void shader_set_mat4(GLuint program, const char* name, Matrix* mat)
{
    glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_TRUE, &mat->m[0][0]);
}
