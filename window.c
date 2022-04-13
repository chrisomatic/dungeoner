#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "player.h"
#include "gfx.h"
#include "settings.h"
#include "window.h"
#include "log.h"

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

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
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

    glfwSwapInterval(0);

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
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        int mode = glfwGetInputMode(window,GLFW_CURSOR);
        if(mode == GLFW_CURSOR_NORMAL)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        player.primary_action = true;
    }
    else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        player.secondary_action = true;
    }
}

static void key_callback(GLFWwindow* window, int key, int scan_code, int action, int mods)
{
    if(action == GLFW_PRESS)
    {
        switch(key)
        {
            case GLFW_KEY_W:
                player.forward = true;
                break;
            case GLFW_KEY_S:
                player.back = true;
                break;
            case GLFW_KEY_A:
                player.left = true;
                break;
            case GLFW_KEY_D:
                player.right = true;
                break;
            case GLFW_KEY_SPACE:
                player.jump = true;
                break;
            case GLFW_KEY_LEFT_SHIFT:
                player.run = true;
                break;
            case GLFW_KEY_F:
                show_fog = !show_fog;
                break;
            case GLFW_KEY_P:
                if(player.camera.mode == CAMERA_MODE_FIRST_PERSON)
                {
                    player.camera.mode = CAMERA_MODE_THIRD_PERSON;
                }
                else if(player.camera.mode == CAMERA_MODE_THIRD_PERSON)
                {
                    player.camera.mode = CAMERA_MODE_FIRST_PERSON;
                    player.camera.offset.x = 0.0f;
                    player.camera.offset.y = 0.0f;
                    player.camera.offset.z = 0.0f;
                }
                LOGI("Camera mode: %d",player.camera.mode);
                break;
            case GLFW_KEY_TAB:
                show_wireframe = !show_wireframe;
                LOGI("Wireframe: %d",show_wireframe);
                break;
            case GLFW_KEY_M:
                player.spectator = !player.spectator;
                if(!player.spectator)
                    player_snap_camera();
                LOGI("Camera mode: %d",player.camera.mode);
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
                player.forward = false;
                break;
            case GLFW_KEY_S:
                player.back = false;
                break;
            case GLFW_KEY_A:
                player.left = false;
                break;
            case GLFW_KEY_D:
                player.right = false;
                break;
            case GLFW_KEY_SPACE:
                player.jump = false;
                break;
            case GLFW_KEY_LEFT_SHIFT:
                player.run = false;
                break;
        }
    }
}
