#include <string.h>

#include "common.h"
#include "3dmath.h"
#include "camera.h"

static void update_view_matrix(Camera* camera)
{
    Matrix camera_translate_trans = {0};
    Matrix camera_rotate_trans    = {0};

    Vector3f camera_pos = {
        camera->phys.pos.x + camera->offset.x,
        camera->phys.pos.y + camera->offset.y,
        camera->phys.pos.z + camera->offset.z
    };

    get_translate_transform(&camera_translate_trans, &camera_pos);
    get_camera_transform(&camera_rotate_trans, camera->lookat, camera->up);

    Matrix* view = &camera->view_matrix;

    memcpy(view,&identity_matrix,sizeof(Matrix));
    dot_product_mat(*view, camera_rotate_trans,    view);
    dot_product_mat(*view, camera_translate_trans, view);
}

void camera_update_rotation(Camera* camera)
{
    // const Vector3f v_axis = {0.0, 1.0, 0.0};

    // Rotate the view vector by the horizontal angle around the vertical axis
    Vector3f view = {0.0, 0.0, 1.0};

    Vector3f h_axis = {0};

    rotate_vector(&view, camera->angle_h, camera->angle_v, &h_axis);
    copy_vector(&camera->lookat,view);
    normalize(&camera->lookat);

    //printf("lookat: %f %f %f\n", camera->lookat.x, camera->lookat.y, camera->lookat.z);

    cross(camera->lookat,h_axis, &camera->up);
    normalize(&camera->up);

    update_view_matrix(camera);
    //printf("Up: %f %f %f\n", camera->up.x, camera->up.y, camera->up.z);
}
