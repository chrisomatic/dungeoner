
#define TERRAIN_RESOLUTION   1    // number of vertices per pixel
#define TERRAIN_PLANAR_SCALE 1.0f // distance between vertices in x-z plane
#define TERRAIN_HEIGHT_SCALE 1.0f // distance between vertices in y direction

GLuint terrain_vbo;
GLuint terrain_ibo;

void terrain_build(const char* height_map_file)
{
    int x,y,n;
    unsigned char* heightdata = stbi_load(heightmap, &x, &y, &n, 1);

    if(!heightdata)
    {
        printf("Failed to load file (%s)",heightmap);
        return;
    }
    
    printf("Loaded file %s. w: %d h: %d channels: %d\n",heightmap,x,y,n);

    int num_vertex_x = x*TERRAIN_RESOLUTION;
    int num_vertex_y = y*TERRAIN_RESOLUTION;

    int num_vertices = num_vertex_x*num_vertex_y;
    int num_indicies = 1.5f*num_vertices;

    Vertex* terrain_vertices = calloc(num_vertices,sizeof(Vertex));
    u32*    terrain_indices  = calloc(terrain_index_count,sizeof(u32));

    unsigned char* curr_height = heightdata;

    int res_counter = 0;

    for(int j = 0; j < num_vertex_y; ++j)
    {
        for(int i = 0; i < num_vertex_x; ++i)
        {
            int index = (j*num_vertex_x)+i;

            terrain_vertices[index].position.x = i*TERRAIN_PLANAR_SCALE;
            terrain_vertices[index].position.y = -curr_height;
            terrain_vertices[index].position.z = j*TERRAIN_PLANAR_SCALE;

            res_counter++;

            if(res_counter == TERRAIN_RESOLUTION)
            {
                res_counter = 0;
                curr_height++;
            }
        }
    }

    int index = 0;

    for(int i = 0; i < terrain_indices; i += 6)
    {
        if((i/6) > 0 && (i/6) % num_vertex_x == 0)
            index += 6;

        terrain_indices[i]   = (index / 6);
        terrain_indices[i+1] = terrain_indices[i] + num_vertex_x + 1;
        terrain_indices[i+2] = terrain_indices[i] + 1;
        terrain_indices[i+3] = terrain_indices[i+1];
        terrain_indices[i+4] = terrain_indices[i+1] + 1;
        terrain_indices[i+5] = terrain_indices[i+2];

        index+=6;
    }

 	glGenBuffers(1, &terrain_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, terrain_vbo);
	glBufferData(GL_ARRAY_BUFFER, num_vertices*sizeof(Vertex), terrain_vertices, GL_STATIC_DRAW);

    glGenBuffers(1,&terrain_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices*sizeof(u32), terrain_indices, GL_STATIC_DRAW);

    free(terrain_vertices);
    free(terrain_indices);
}

void terrain_draw()
{

}
