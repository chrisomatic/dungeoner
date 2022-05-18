#include "common.h"
#include "3dmath.h"

typedef struct
{
    Vector3f a;
    bool is_valid;
} Point;

typedef struct
{
    Point a,b,c,d;
    int num_valid;
} Simplex;

void add_to_simplex(Simplex* s, Vector3f* p)
{

    if(s->num_valid == 1)
    {
        copy_vector(&s->b, s->a);
    }
    else if(s->num_valid == 2)
    {

    }
        v = &s->c;
    else if(s->num_valid == 3)
        v = &s->d;

    v->x = p->x;
    v->y = p->y;
    v->x = p->z;

    s->num_valid++;
}

static int support_single(PointList* p, Vector3f* dir)
{
    float max_d = 0.0f;
    int max_index = 0;

    for(int i = 0; i < p->point_count; ++i)
    {
        float d = dot(*dir,p->points[i]);
        if(d >= max_d)
        {
            max_d = d;
            max_index = i;
        }
    }

    return max_index;
}

static Vector3f support(PointList* a, PointList* b, Vector3f* dir)
{
    int index_a = support_single(a,dir);
    int index_b = support_single(b,mult(dir,-1.0));

    Vector3f s = {
        a->points[index_a].x - b->points[index_b].x,
        a->points[index_a].y - b->points[index_b].y,
        a->points[index_a].z - b->points[index_b].z
    };

    return s;
}

static bool do_simplex(Simplex* s, Vector3f* dir)
{
    if(s->num_valid == 1)
        return false;

    if(s->num_valid == 2)
    {
        Vector3f ab = {s->b.x - s->a.x,s->b.y - s->a.y,s->b.z - s->a.z};

        if(dot(ab,mult(&a,-1.0)) > 0.0)
        {

        }
    }

}

bool gjk(PointList *a, PointList *b)
{
    Simplex simplex = {0};

    Vector3f S = {
        a->points[0].x - b->points[0].x,
        a->points[0].y - b->points[0].y,
        a->points[0].z - b->points[0].z
    };

    add_to_simplex(&simplex,&S);

    Vector3f dir = {-s.x, -s.y, -s.z};

    for(;;)
    {
        Vector3f A = support(a,b,dir);

        if(dot(A,dir) < 0.0)
            return false; // No Intersection!

        add_to_simplex(&simplex,&A);

        if(do_simplex(&simplex, &dir))
            return true; // Intersection!
    }
}
