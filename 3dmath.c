#include <stdio.h>
#include <string.h>
#include <math.h>
#include "settings.h"
#include "player.h"
#include "3dmath.h"

Matrix identity_matrix = {
    .m={
        {1.0f,0.0f,0.0f,0.0f},
        {0.0f,1.0f,0.0f,0.0f},
        {0.0f,0.0f,1.0f,0.0f},
        {0.0f,0.0f,0.0f,1.0f}
    }
};

static void conjugate_q(Quaternion q, Quaternion* ret)
{
    ret->x = -q.x;
    ret->y = -q.y;
    ret->z = -q.z;
    ret->w = q.w;
}

static void multiply_q_v3f(Quaternion q, Vector3f v, Quaternion* ret)
{
    ret->w = - (q.x * v.x) - (q.y * v.y) - (q.z * v.z);
    ret->x = + (q.w * v.x) + (q.y * v.z) - (q.z * v.y);
    ret->y = + (q.w * v.y) + (q.z * v.x) - (q.x * v.z);
    ret->z = + (q.w * v.z) + (q.x * v.y) - (q.y * v.x);
}

static void multiply_q(Quaternion a,Quaternion b, Quaternion* ret)
{
    ret->w = (a.w * b.w) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z);
    ret->x = (a.x * b.w) + (a.w * b.x) + (a.y * b.z) - (a.z * b.y);
    ret->y = (a.y * b.w) + (a.w * b.y) + (a.z * b.x) - (a.x * b.z);
    ret->z = (a.z * b.w) + (a.w * b.z) + (a.x * b.y) - (a.y * b.x);
}

float barry_centric(Vector3f p1, Vector3f p2, Vector3f p3, Vector2f pos)
{
    float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
    float l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
    float l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
    float l3 = 1.0f - l1 - l2;

    return l1 * p1.y + l2 * p2.y + l3 * p3.y;
}

float magn(Vector3f v)
{
    return sqrt(v.x * v.x + v.y*v.y + v.z*v.z);
}

void copy_vector(Vector3f* dest, Vector3f src)
{
    dest->x = src.x;
    dest->y = src.y;
    dest->z = src.z;
}

void normalize(Vector3f* v)
{
    float len = magn(*v);

    if(len == 0)
    {
        v->x = 0.0f;
        v->y = 0.0f;
        v->z = 0.0f;
        return;
    }

    v->x /= len;
    v->y /= len;
    v->z /= len;
}

void cross(Vector3f a, Vector3f b, Vector3f* res)
{
    res->x = a.y * b.z - a.z * b.y;
    res->y = a.z * b.x - a.x * b.z;
    res->z = a.x * b.y - a.y * b.x;
}

void rotate(Vector3f* v, const Vector3f axis, float angle)
{
    const float sin_half_angle = sinf(RAD(angle/2.0f));
    const float cos_half_angle = cosf(RAD(angle/2.0f));

    const float rx = axis.x * sin_half_angle;
    const float ry = axis.y * sin_half_angle;
    const float rz = axis.z * sin_half_angle;
    const float rw = cos_half_angle;

    Quaternion rotation = {rx, ry, rz, rw};

    Quaternion conj;
    conjugate_q(rotation, &conj);

    Quaternion w;

    multiply_q_v3f(rotation, *v, &w);
    multiply_q(w,conj, &w);

    v->x = w.x;
    v->y = w.y;
    v->z = w.z;
}

float dot(Vector3f a, Vector3f b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

void subtract(Vector3f* a, Vector3f b)
{
    a->x = a->x - b.x;
    a->y = a->y - b.y;
    a->z = a->z - b.z;
}

void add(Vector3f* a, Vector3f b)
{
    a->x = a->x + b.x;
    a->y = a->y + b.y;
    a->z = a->z + b.z;
}

void mult(Vector3f* a, float c)
{
    a->x *= c;
    a->y *= c;
    a->z *= c;
}

void normal(Vector3f a, Vector3f b, Vector3f c, Vector3f* norm)
{
    subtract(&b,a);
    subtract(&c,a);

    cross(b, c, norm);
    normalize(norm);
}

void calc_vertex_normals(const uint32_t* indices, uint32_t index_count, Vertex* vertices, uint32_t vertex_count)
{
    for (int i = 0; i < index_count; i += 3)
    {
        uint32_t i0 = indices[i + 0];
        uint32_t i1 = indices[i + 1];
        uint32_t i2 = indices[i + 2];

        Vector3f p0 = vertices[i0].position;
        Vector3f p1 = vertices[i1].position;
        Vector3f p2 = vertices[i2].position;

        Vector3f n;

        normal(p0,p1,p2,&n);

        add(&vertices[i0].normal, n);
        add(&vertices[i1].normal, n);
        add(&vertices[i2].normal, n);
    }

    for (int i = 0 ; i < vertex_count ; ++i)
    {
        normalize(&vertices[i].normal);
    }
}

void dot_product_mat(Matrix a, Matrix b, Matrix* result)
{
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            result->m[i][j] =
                a.m[i][0] * b.m[0][j] + 
                a.m[i][1] * b.m[1][j] + 
                a.m[i][2] * b.m[2][j] + 
                a.m[i][3] * b.m[3][j];
        }
    }
}
static void get_scale_transform(Matrix* mat, Vector3f* scale)
{
    memset(mat,0,sizeof(Matrix));

    mat->m[0][0] = scale->x;
    mat->m[1][1] = scale->y;
    mat->m[2][2] = scale->z;
    mat->m[3][3] = 1.0f;
}

static void get_rotation_transform(Matrix* mat, Vector3f* rotation)
{
    Matrix rx = {0};
    Matrix ry = {0};
    Matrix rz = {0};

    const float x = RAD(rotation->x);
    const float y = RAD(rotation->y);
    const float z = RAD(rotation->z);

    rx.m[0][0] = 1.0f;
    rx.m[1][1] = cosf(x);
    rx.m[1][2] = -sinf(x);
    rx.m[2][1] = sinf(x);
    rx.m[2][2] = cosf(x);
    rx.m[3][3] = 1.0f;

    ry.m[0][0] = cosf(y);
    ry.m[0][2] = -sinf(y); 
    ry.m[1][1] = 1.0f;  
    ry.m[2][0] = sinf(y);
    ry.m[2][2] = cosf(y);
    ry.m[3][3] = 1.0f;

    rz.m[0][0] = cosf(z);
    rz.m[0][1] = -sinf(z);
    rz.m[1][0] = sinf(z);
    rz.m[1][1] = cosf(z);
    rz.m[2][2] = 1.0f;
    rz.m[3][3] = 1.0f;

    memset(mat,0,sizeof(Matrix));

    dot_product_mat(identity_matrix,rz,mat);
    dot_product_mat(*mat,ry,mat);
    dot_product_mat(*mat,rx,mat);
}

static void get_translate_transform(Matrix* mat, Vector3f* position)
{
    memset(mat,0,sizeof(Matrix));

    mat->m[0][0] = 1.0f;
    mat->m[0][3] = position->x;
    mat->m[1][1] = 1.0f;
    mat->m[1][3] = position->y;
    mat->m[2][2] = 1.0f;
    mat->m[2][3] = position->z;
    mat->m[3][3] = 1.0f;
}

static void get_perspective_transform(Matrix* mat)
{
    const float ar           = view_width/(float)view_height;
    const float z_near       = Z_NEAR;
    const float z_far        = Z_FAR;
    const float z_range      = z_near - z_far;
    const float tan_half_fov = tanf(RAD(FOV / 2.0f));

    memset(mat,0,sizeof(Matrix));

    mat->m[0][0] = 1.0f / (tan_half_fov * ar);
    mat->m[1][1] = 1.0f / tan_half_fov;
    mat->m[2][2] = (-z_near - z_far) / z_range;
    mat->m[2][3] = 2.0f * z_far * z_near / z_range;
    mat->m[3][2] = 1.0f;
}

void get_camera_transform(Matrix* mat, Vector3f target, Vector3f up)
{
    Vector3f n,u,v;

    copy_vector(&n,target);
    copy_vector(&u,up);

    normalize(&n);
    cross(target,u,&u);
    cross(n,u,&v);

    memset(mat,0,sizeof(Matrix));

    mat->m[0][0] = u.x;
    mat->m[0][1] = u.y;
    mat->m[0][2] = u.z;
    mat->m[1][0] = v.x;
    mat->m[1][1] = v.y;
    mat->m[1][2] = v.z;
    mat->m[2][0] = n.x;
    mat->m[2][1] = n.y;
    mat->m[2][2] = n.z;
    mat->m[3][3] = 1.0f;
}

static Matrix world_trans = {0};
static Matrix wv_trans = {0};
static Matrix wvp_trans = {0};

Matrix* get_wv_transform(Vector3f* pos, Vector3f* rotation, Vector3f* scale)
{
    Matrix scale_trans            = {0};
    Matrix rotation_trans         = {0};
    Matrix translate_trans        = {0};
    Matrix camera_translate_trans = {0};
    Matrix camera_rotate_trans    = {0};

    Camera* cam = &player.camera;
    Vector3f camera_pos = {
        cam->phys.pos.x + cam->offset.x,
        cam->phys.pos.y + cam->offset.y,
        cam->phys.pos.z + cam->offset.z
    };

    get_scale_transform(&scale_trans, scale);
    get_rotation_transform(&rotation_trans, rotation);
    get_translate_transform(&translate_trans, pos);
    get_translate_transform(&camera_translate_trans, &camera_pos);
    get_camera_transform(&camera_rotate_trans, player.camera.target, player.camera.up);

    memcpy(&wv_trans,&identity_matrix,sizeof(Matrix));

    dot_product_mat(wv_trans, camera_rotate_trans,    &wv_trans);
    dot_product_mat(wv_trans, camera_translate_trans, &wv_trans);
    dot_product_mat(wv_trans, translate_trans,        &wv_trans);
    dot_product_mat(wv_trans, rotation_trans,         &wv_trans);
    dot_product_mat(wv_trans, scale_trans,            &wv_trans);

    return &wv_trans;
}

Matrix* get_wvp_transform(Vector3f* pos, Vector3f* rotation, Vector3f* scale)
{
    Matrix scale_trans            = {0};
    Matrix rotation_trans         = {0};
    Matrix translate_trans        = {0};
    Matrix perspective_trans      = {0};
    Matrix camera_translate_trans = {0};
    Matrix camera_rotate_trans    = {0};

    Camera* cam = &player.camera;
    Vector3f camera_pos = {
        cam->phys.pos.x + cam->offset.x,
        cam->phys.pos.y + cam->offset.y,
        cam->phys.pos.z + cam->offset.z
    };

    get_scale_transform(&scale_trans, scale);
    get_rotation_transform(&rotation_trans, rotation);
    get_translate_transform(&translate_trans, pos);
    get_perspective_transform(&perspective_trans);
    get_translate_transform(&camera_translate_trans, &camera_pos);
    get_camera_transform(&camera_rotate_trans, player.camera.target, player.camera.up);

    memcpy(&wvp_trans,&identity_matrix,sizeof(Matrix));

    dot_product_mat(wvp_trans, perspective_trans,      &wvp_trans);
    dot_product_mat(wvp_trans, camera_rotate_trans,    &wvp_trans);
    dot_product_mat(wvp_trans, camera_translate_trans, &wvp_trans);
    dot_product_mat(wvp_trans, translate_trans,        &wvp_trans);
    dot_product_mat(wvp_trans, rotation_trans,         &wvp_trans);
    dot_product_mat(wvp_trans, scale_trans,            &wvp_trans);

    return &wvp_trans;
}

Matrix* get_world_transform(Vector3f* pos, Vector3f* rotation, Vector3f* scale)
{
    Matrix scale_trans            = {0};
    Matrix rotation_trans         = {0};
    Matrix translate_trans        = {0};

    get_scale_transform(&scale_trans, scale);
    get_rotation_transform(&rotation_trans, rotation);
    get_translate_transform(&translate_trans, pos);

    memcpy(&world_trans,&identity_matrix,sizeof(Matrix));

    dot_product_mat(world_trans, translate_trans, &world_trans);
    dot_product_mat(world_trans, rotation_trans,  &world_trans);
    dot_product_mat(world_trans, scale_trans,     &world_trans);

    return &world_trans;
}

void print_matrix(Matrix* mat)
{
    printf("Matrix:\n");
    for(int i = 0; i < 4; ++i)
    {
        printf("[ %f %f %f %f]"
                ,mat->m[i][0]
                ,mat->m[i][1]
                ,mat->m[i][2]
                ,mat->m[i][3]
              );
        printf("\n");
    }
            
}
