#include <GL/glew.h>
#include "log.h"

#define STB_IMAGE_IMPLEMENTATION
#include "util/stb_image.h"

GLuint load_texture(const char* texture_path)
{
    int x,y,n;
    unsigned char* data;  
    data = stbi_load(texture_path, &x, &y, &n, 0);

    if(!data)
    {
        LOGE("Failed to load file (%s)",texture_path);
        return 0;
    }
    
    LOGI("Loaded file %s. w: %d h: %d",texture_path,x,y);

    GLenum format;
    GLuint texture;

    if(n == 3) format = GL_RGB;
    else       format = GL_RGBA;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, format, x, y, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return texture;
}
