#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "common.h"
#include "player.h"
#include "projectile.h"
#include "gfx.h"
#include "settings.h"
#include "3dmath.h"
#include "log.h"
#include "window.h"

static GLFWwindow* window;

int view_width = 0;
int view_height = 0;

static void window_size_callback(GLFWwindow* window, int window_width, int window_height);
static void window_maximize_callback(GLFWwindow* window, int maximized);
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
static void key_callback(GLFWwindow* window, int key, int scan_code, int action, int mods);
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

bool window_init()
{
    LOGI("Initializing GLFW.");

    if(!glfwInit())
    {
        LOGE("Failed to init GLFW!");
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 1); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Want to use OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 

    view_width = STARTING_VIEW_WIDTH;
    view_height = STARTING_VIEW_HEIGHT;

    window = glfwCreateWindow(view_width,view_height,"Dungeoner",NULL,NULL);

    if(window == NULL)
    {
        LOGE("Failed to create GLFW Window!");
        glfwTerminate();
        return false;
    }

    glfwSetWindowAspectRatio(window,ASPECT_NUM,ASPECT_DEM);
    glfwSetWindowSizeCallback(window,window_size_callback);
    glfwSetWindowMaximizeCallback(window, window_maximize_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSwapInterval(1);

    LOGI("Initializing GLEW.");

    // GLEW
    glewExperimental = 1;
    if(glewInit() != GLEW_OK)
    {
        LOGE("Failed to initialize GLEW");
        return false;
    }

    return true;
}

void window_deinit()
{
    glfwTerminate();
}

void window_poll_events()
{
    glfwPollEvents();
}

bool window_should_close()
{
    return (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS || glfwWindowShouldClose(window) != 0);
}

void window_swap_buffers()
{
    glfwSwapBuffers(window);
}

static void window_size_callback(GLFWwindow* window, int window_width, int window_height)
{
    LOGI("Window: W %d, H %d",window_width,window_height);

    view_width  = window_width; //ASPECT_RATIO * window_height;
    view_height = window_height;

    int start_x = (window_width + view_width) / 2.0f - view_width;
    int start_y = (window_height + view_height) / 2.0f - view_height;

    glViewport(start_x,start_y,view_width,view_height);
    update_projection_transform();
}

static void window_maximize_callback(GLFWwindow* window, int maximized)
{
    if (maximized)
    {
        // The window was maximized
        LOGI("Maximized.");
    }
    else
    {
        // The window was restored
        LOGI("Restored.");
    }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    int mode = glfwGetInputMode(window,GLFW_CURSOR);
    if(mode == GLFW_CURSOR_DISABLED)
        player_update_camera_angle(xpos, ypos);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if(action == GLFW_PRESS)
        {
            int mode = glfwGetInputMode(window,GLFW_CURSOR);
            if(mode == GLFW_CURSOR_NORMAL)
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            player->input.primary_action = true;
        }
        else if(action == GLFW_RELEASE)
        {
            player->input.primary_action = false;
        }
    }
    else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        if(action == GLFW_PRESS)
        {
            player->input.secondary_action = true;
        }
        else if(action == GLFW_RELEASE)
        {
            player->input.secondary_action = false;
        }
    }
}

static void key_callback(GLFWwindow* window, int key, int scan_code, int action, int mods)
{
    if(action == GLFW_PRESS)
    {
        switch(key)
        {
            case GLFW_KEY_1:
                player->equipped_projectile = PROJECTILE_FIREBALL;
                break;
            case GLFW_KEY_2:
                player->equipped_projectile = PROJECTILE_PORTAL;
                break;
            case GLFW_KEY_3:
                player->equipped_projectile = PROJECTILE_ICE;
                break;
            case GLFW_KEY_W:
                player->input.forward = true;
                break;
            case GLFW_KEY_S:
                player->input.back = true;
                break;
            case GLFW_KEY_A:
                player->input.left = true;
                break;
            case GLFW_KEY_D:
                player->input.right = true;
                break;
            case GLFW_KEY_E:
                player->input.use = true;
                break;
            case GLFW_KEY_SPACE:
                player->input.jump = true;
                break;
            case GLFW_KEY_LEFT_SHIFT:
                player->input.run = true;
                break;
            case GLFW_KEY_F:
                show_fog = !show_fog;
                break;
            case GLFW_KEY_F2:
                show_collision = !show_collision;
                break;
            case GLFW_KEY_LEFT_CONTROL:
                player->input.crouched = true;
                player->phys.height /= 2.0;
                break;
            case GLFW_KEY_P:

                player->camera.offset_transition = 0.0;

                if(player->camera.mode == CAMERA_MODE_FIRST_PERSON)
                {
                    player->camera.mode = CAMERA_MODE_THIRD_PERSON;
                }
                else if(player->camera.mode == CAMERA_MODE_THIRD_PERSON)
                {
                    player->camera.mode = CAMERA_MODE_FIRST_PERSON;
                    player->camera.offset.x = 0.0f;
                    player->camera.offset.y = 0.0f;
                    player->camera.offset.z = 0.0f;
                }
                LOGI("Camera mode: %d",player->camera.mode);
                break;
            case GLFW_KEY_TAB:
                show_wireframe = !show_wireframe;
                LOGI("Wireframe: %d",show_wireframe);
                break;
            case GLFW_KEY_M:
                player->spectator = !player->spectator;
                if(!player->spectator)
                    player_snap_camera();
                LOGI("Camera mode: %d",player->camera.mode);
                break;
            case GLFW_KEY_ESCAPE:
            {
                int mode = glfwGetInputMode(window,GLFW_CURSOR);
                if(mode == GLFW_CURSOR_DISABLED)
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                else
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }   break;
        }
    }
    else if(action == GLFW_RELEASE)
    {
        switch(key)
        {
            case GLFW_KEY_W:
                player->input.forward = false;
                break;
            case GLFW_KEY_S:
                player->input.back = false;
                break;
            case GLFW_KEY_A:
                player->input.left = false;
                break;
            case GLFW_KEY_D:
                player->input.right = false;
                break;
            case GLFW_KEY_LEFT_SHIFT:
                player->input.run = false;
                break;
            case GLFW_KEY_LEFT_CONTROL:
                player->input.crouched = false;
                player->phys.height *= 2.0;
                break;
            case GLFW_KEY_SPACE:
                player->input.jump = false;
                break;
        }
    }
}
