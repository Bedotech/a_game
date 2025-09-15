#ifndef PHYSICS_H
#define PHYSICS_H

#include "common.h"

void physics_update_entity(Entity* entity, float delta_time);

void physics_apply_boundary_constraints(Entity* entity, float min_x, float max_x, float min_y, float max_y);

bool physics_check_collision(const Rectangle* a, const Rectangle* b);

Rectangle physics_entity_to_rectangle(const Entity* entity);

bool physics_entities_collide(const Entity* a, const Entity* b);

void physics_clamp_entity_position(Entity* entity, float screen_width, float screen_height);

#endif // PHYSICS_H