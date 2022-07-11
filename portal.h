#pragma once

typedef struct
{
    Vector3f pos;
} PortalDoor;

typedef struct
{
    PortalDoor a;
    PortalDoor b;
} Portal;

void portal_init();
void portal_update();
void portal_draw();
