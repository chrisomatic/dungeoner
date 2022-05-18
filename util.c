#include <GL/glew.h>
#include "log.h"

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "util/stb_image.h"

unsigned char* util_load_image(const char* image_path, int *x, int *y, int *n, int d)
{
    return stbi_load(image_path, x, y, n, d);
}

void util_unload_image(unsigned char* image_data)
{
    return stbi_image_free(image_data);
}

GLuint load_texture(const char* texture_path)
{
    int x,y,n;
    unsigned char* data;  

    //stbi_set_flip_vertically_on_load(1);

    data = stbi_load(texture_path, &x, &y, &n, 0);

    if(!data)
    {
        LOGE("Failed to load file (%s)",texture_path);
        return 0;
    }
    
    LOGI("Loaded file %s. w: %d h: %d n: %d",texture_path,x,y,n);

    GLenum format;
    GLuint texture;

    if(n == 3) format = GL_RGB;
    else       format = GL_RGBA;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0);
    //@TODO: Check to make sure this extension is available
    float amount = 4.0; //MIN(4.0f,glGetFloat(EXTTextureFilterAn));
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, amount);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, format, x, y, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    return texture;
}

GLuint load_texture_cube(char* cube_file_paths[], int num_file_paths)
{
    int x,y,n;
    unsigned char* data;  

    GLuint texture;
    GLenum format;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    printf("Generating cubemap texture.\n");
    
    stbi_set_flip_vertically_on_load(1);

    for(unsigned int i = 0; i < num_file_paths; i++)
    {
        data = stbi_load(cube_file_paths[i], &x, &y, &n, 0);

        if(n == 3) format = GL_RGB;
        else       format = GL_RGBA;

        if(!data)
        {
            printf("Failed to load file (%s)",cube_file_paths[i]);
            return 0;
        }

        printf("Loaded file %s. w: %d h: %d\n",cube_file_paths[i],x,y);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, x, y, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return texture;
}
