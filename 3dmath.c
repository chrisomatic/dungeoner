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

float magn(Vector3f v)
{
    return sqrt(v.x * v.x + v.y*v.y + v.z*v.z);
}

float dist_squared(Vector3f* a, Vector3f* b)
{
    Vector3f s = {
        b->x - a->x,
        b->y - a->y,
        b->z - a->z
    };

    float d_sq = (s.x * s.x) + (s.y * s.y) + (s.z * s.z);
    return d_sq;
}

float dist(Vector3f* a, Vector3f* b)
{
    float d = sqrt(dist_squared(a,b));
    return d;
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
    res->x = (a.y * b.z) - (a.z * b.y);
    res->y = (a.z * b.x) - (a.x * b.z);
    res->z = (a.x * b.y) - (a.y * b.x);
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

void rotate_toward_point(Vector curr_normal, Vector* starting_point, Vector* target_point, Vector* ret_rotation)
{
    //Rotation axis = normalize(crossproduct(currentNormal, desiredNormal))
    //Rotation angle = acos(dotproduct(normalize(currentNormal), normalize(desiredNormal)).

    Vector desired_normal = {
        target_point->x - starting_point->x,
        target_point->y - starting_point->y,
        target_point->z - starting_point->z
    };

    normalize(&desired_normal);

    Vector dn_x = {0.0,desired_normal.y, desired_normal.z};
    Vector dn_y = {desired_normal.x,0.0, desired_normal.z};
    Vector dn_z = {desired_normal.x, desired_normal.y, 0.0};

    Vector cn_x = {0.0,curr_normal.y, curr_normal.z};
    Vector cn_y = {curr_normal.x,0.0, curr_normal.z};
    Vector cn_z = {curr_normal.x, curr_normal.y, 0.0};

    ret_rotation->x = DEG(get_angle_between_vectors_rad(&cn_x,&dn_x));
    ret_rotation->y = DEG(get_angle_between_vectors_rad(&cn_y,&dn_y));
    ret_rotation->z = DEG(get_angle_between_vectors_rad(&cn_z,&dn_z));

    printf("desired_normal: %f %f %f, rotate: %f %f %f\n",
        desired_normal.x,desired_normal.y, desired_normal.z,
        ret_rotation->x,ret_rotation->y, ret_rotation->z
    );
}

void rotate_vector(Vector* v, float angle_h, float angle_v, Vector* ret_h_axis)
{
    const Vector3f v_axis = {0.0, 1.0, 0.0};

    // Rotate the view vector by the horizontal angle around the vertical axis
    rotate(v, v_axis, angle_h);
    normalize(v);

    // Rotate the view vector by the vertical angle around the horizontal axis
    Vector3f h_axis = {0};

    cross(v_axis, *v, &h_axis);
    normalize(&h_axis);
    rotate(v, h_axis, angle_v);

    if(ret_h_axis)
        copy_vector(ret_h_axis,h_axis);
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

void mult_v3f_mat4(Vector3f* v, Matrix* m, Vector3f* result)
{
    // assuming w is 1.0 for Vector
    result->x = (m->m[0][0] * v->x + m->m[0][1] * v->y + m->m[0][2] * v->z + m->m[0][3]);
    result->y = (m->m[1][0] * v->x + m->m[1][1] * v->y + m->m[1][2] * v->z + m->m[1][3]);
    result->z = (m->m[2][0] * v->x + m->m[2][1] * v->y + m->m[2][2] * v->z + m->m[2][3]);
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
    /*
    	projectionMatrix = new Matrix4f();
		float aspectRatio = (float) Display.getWidth() / (float) Display.getHeight();
		float y_scale = (float) ((1f / Math.tan(Math.toRadians(FOV / 2f))));
		float x_scale = y_scale / aspectRatio;
		float frustum_length = FAR_PLANE - NEAR_PLANE;

		projectionMatrix.m00 = x_scale;
		projectionMatrix.m11 = y_scale;
		projectionMatrix.m22 = -((FAR_PLANE + NEAR_PLANE) / frustum_length);
		projectionMatrix.m23 = -1;
		projectionMatrix.m32 = -((2 * NEAR_PLANE * FAR_PLANE) / frustum_length);
		projectionMatrix.m33 = 0;
    }

    const float ar           = view_width/(float)view_height;
    const float y_scale      = 1.0f / tanf(RAD(FOV / 2.0f));
    const float x_scale      = y_scale / ar;
    const float frustum_len  = Z_FAR - Z_NEAR;

    memset(mat,0,sizeof(Matrix));

    mat->m[0][0] = x_scale;
    mat->m[1][1] = y_scale;
    mat->m[2][2] = -((Z_FAR + Z_NEAR) / frustum_len);
    mat->m[2][3] = -1.0;
    mat->m[3][2] = -((2 * Z_NEAR * Z_FAR) / frustum_len);
    mat->m[3][3] = 0.0;
    */

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

void get_camera_transform(Matrix* mat, Vector3f lookat, Vector3f up)
{
    Vector3f n,u,v;

    copy_vector(&n,lookat);
    copy_vector(&u,up);

    normalize(&n);
    cross(lookat,u,&u);
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

void get_model_transform(Vector* pos, Vector* rotation, Vector* scale, Matrix* model)
{
    Matrix scale_trans            = {0};
    Matrix rotation_trans         = {0};
    Matrix translate_trans        = {0};

    get_scale_transform(&scale_trans, scale);
    get_rotation_transform(&rotation_trans, rotation);
    get_translate_transform(&translate_trans, pos);

    memcpy(model,&identity_matrix,sizeof(Matrix));
    dot_product_mat(*model, translate_trans, model);
    dot_product_mat(*model, rotation_trans,  model);
    dot_product_mat(*model, scale_trans,     model);
}

void get_view_proj_transforms(Matrix* view, Matrix* proj)
{
    Matrix perspective_trans      = {0};
    Matrix camera_translate_trans = {0};
    Matrix camera_rotate_trans    = {0};

    Camera* cam = &player->camera;
    Vector3f camera_pos = {
        cam->phys.pos.x + cam->offset.x,
        cam->phys.pos.y + cam->offset.y,
        cam->phys.pos.z + cam->offset.z
    };

    get_perspective_transform(&perspective_trans);
    get_translate_transform(&camera_translate_trans, &camera_pos);
    get_camera_transform(&camera_rotate_trans, player->camera.lookat, player->camera.up);

    memcpy(view,&identity_matrix,sizeof(Matrix));
    dot_product_mat(*view, camera_rotate_trans,    view);
    dot_product_mat(*view, camera_translate_trans, view);

    memcpy(proj,&identity_matrix,sizeof(Matrix));
    dot_product_mat(*proj, perspective_trans, proj);
}

void get_transforms(Vector3f* pos, Vector3f* rotation, Vector3f* scale, Matrix* world, Matrix* view, Matrix* proj)
{
    get_model_transform(pos,rotation,scale,world);
    get_view_proj_transforms(view,proj);
}

void get_ortho_transform(Matrix* m, float left, float right, float bottom, float top)
{
    memset(m,0,sizeof(Matrix));

    m->m[0][0] = 2.0f/(right-left);
    m->m[1][1] = -2.0f/(top-bottom);
    m->m[2][2] = -1.0f;
    m->m[3][3] = 1.0f;
    m->m[0][3] = -(right+left) / (right - left);
    m->m[1][3] = (top+bottom) / (top-bottom);
}

float get_y_value_on_plane(float x, float z, Vector* a, Vector* b, Vector* c)
{
    if(a == NULL || b == NULL || c == NULL)
        return 0.0;

    // plane equ: rx+sy+tz=k

    Vector v1 = {a->x - b->x, a->y - b->y, a->z - b->z};
    Vector v2 = {a->x - c->x, a->y - c->y, a->z - c->z};

    Vector n; // r,s,t
    cross(v1,v2,&n);

    // compute k
    float k = dot(n,*a);

    float rx = n.x * x;
    float tz = n.z * z;
    float s  = n.y;

    if(s == 0.0) return 0.0;

    float y = (k - rx - tz) / s;

    return y;
}

// This function accepts a point and a plane definition, and will find the point on the plane projected from the original point
Vector3f get_projected_point_on_plane(Vector3f* point, Vector3f* plane_normal, Vector3f* point_on_plane)
{
    Vector3f v = {
        point->x - point_on_plane->x,
        point->y - point_on_plane->y,
        point->z - point_on_plane->z
    };

    float dist = dot(v,*plane_normal);

    Vector3f proj_p = {
        point->x - (dist*plane_normal->x),
        point->y - (dist*plane_normal->y),
        point->z - (dist*plane_normal->z),
    };

    return proj_p;

}

float get_angle_between_vectors_rad(Vector3f* a, Vector3f* b)
{
    float ma = magn(*a);
    float mb = magn(*b);

    if(ma == 0.0 || mb == 0.0)
        return 0.0;

    float d  = dot(*a,*b);
    
    float angle = acosf(d/(ma*mb));
    return angle;
}

Vector3f get_center_of_triangle(Vector3f* a, Vector3f* b, Vector3f* c)
{
    float x = (a->x + b->x + c->x)/3.0;
    float y = (a->y + b->y + c->y)/3.0;
    float z = (a->z + b->z + c->z)/3.0;

    Vector3f v = {x,y,z};
    return v;
}

void get_wvp(Matrix* world, Matrix* view, Matrix* proj, Matrix* wvp)
{
    memcpy(wvp,&identity_matrix,sizeof(Matrix));
    dot_product_mat(*wvp, *proj,  wvp);
    dot_product_mat(*wvp, *view,  wvp);
    dot_product_mat(*wvp, *world, wvp);
}

void get_wv(Matrix* world, Matrix* view, Matrix* wv)
{
    memcpy(wv,&identity_matrix,sizeof(Matrix));
    dot_product_mat(*wv, *view,  wv);
    dot_product_mat(*wv, *world, wv);
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
