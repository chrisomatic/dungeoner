#pragma once

void portal_init();
void portal_update();
void portal_draw();
bool portal_handle_collision(Player* player, Vector3f prior_pos);
void portal_update_location(Vector3f* pos, float angle);
