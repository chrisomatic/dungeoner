#pragma once

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

typedef Vector3f Vector;

typedef struct {float m[4][4];} Matrix;

typedef struct
{
    float x,y,z,w;
} Quaternion;

typedef struct
{
    Vector3f position;
    Vector2f tex_coord;
} Vertex;

extern Matrix identity_matrix;

float magn(Vector v);
float dot(Vector a, Vector b);

void cross(Vector a, Vector b, Vector* res);
void copy_vector(Vector* dest, Vector src);
void rotate(Vector* v, const Vector axis, float angle);
void normal(Vector3f a, Vector3f b, Vector3f c, Vector3f* norm);
void normalize(Vector* v);

void subtract(Vector3f* a, Vector3f b);
void add(Vector3f* a, Vector3f b);
void dot_product_mat(Matrix a, Matrix b, Matrix* result);

Matrix* get_wvp_transform(Vector3f* pos, Vector3f* rotation, Vector3f* scale);
void print_matrix(Matrix* mat);
