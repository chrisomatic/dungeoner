#pragma once
#include <stdint.h>

#define PI        3.14159265358f
#define PI_OVER_2 1.57079632679f

#define RAD(x) (((x) * PI) / 180.0f)
#define DEG(x) (((x) * 180.0f) / PI)
#define ABS(x) ((x) < 0 ? -1*(x) : (x))

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

#define SQ(x) ((x)*(x))

typedef struct
{
    float x,y;
} Vector2f;

typedef struct
{
    float x,y,z;
} Vector3f;

typedef struct
{
    float x,y,z,w;
} Vector4f;

typedef Vector3f Vector;

typedef struct {float m[4][4];} Matrix;

typedef struct
{
    float x,y,z,w;
} Quaternion;

typedef struct
{
    Vector position;
    Vector2f tex_coord;
    Vector normal;
} Vertex;

extern Matrix identity_matrix;

float magn(Vector v);
float dot(Vector a, Vector b);

void cross(Vector a, Vector b, Vector* res);
void copy_vector(Vector* dest, Vector src);
void rotate(Vector* v, const Vector axis, float angle);
void normal(Vector a, Vector b, Vector c, Vector* norm);
void normalize(Vector* v);

void calc_vertex_normals(const uint32_t* indices, uint32_t index_count, Vertex* vertices, uint32_t vertex_count);

void subtract(Vector* a, Vector b);
void add(Vector* a, Vector b);
void mult(Vector* a, float c);
void dot_product_mat(Matrix a, Matrix b, Matrix* result);
void mult_v3f_mat4(Vector3f* v, Matrix* m, Vector3f* result);

void get_transforms(Vector* pos, Vector* rotation, Vector* scale, Matrix* world, Matrix* view, Matrix* proj);
void get_model_transform(Vector* pos, Vector* rotation, Vector* scale, Matrix* model);
void get_view_proj_transforms(Matrix* view, Matrix* proj);
void get_ortho_transform(Matrix* m, float left, float right, float bottom, float top);
void get_wvp(Matrix* world, Matrix* view, Matrix* proj, Matrix* wvp);
void get_wv(Matrix* world, Matrix* view, Matrix* wv);
void print_matrix(Matrix* mat);
Vector get_projected_point_on_plane(Vector* point, Vector* plane_normal, Vector* point_on_plane);
Vector get_center_of_triangle(Vector* a, Vector* b, Vector* c);
float get_angle_between_vectors_rad(Vector* a, Vector* b);
float get_y_value_on_plane(float x, float z, Vector* a, Vector* b, Vector* c);

void rotate_vector(Vector* v, float angle_h, float angle_v, Vector* ret_h_axis);
void rotate_toward_point(Vector curr_normal, Vector* starting_point, Vector* point, Vector* ret_rotation);
