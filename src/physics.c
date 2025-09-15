#include "physics.h"

void physics_update_entity(Entity* entity, float delta_time) {
    if (!entity || !entity->active) return;

    entity->position.x += entity->velocity.x * delta_time;
    entity->position.y += entity->velocity.y * delta_time;
}

void physics_apply_boundary_constraints(Entity* entity, float min_x, float max_x, float min_y, float max_y) {
    if (!entity || !entity->active) return;

    if (entity->position.x < min_x) {
        entity->position.x = min_x;
    }
    if (entity->position.x > max_x - entity->width) {
        entity->position.x = max_x - entity->width;
    }
    if (entity->position.y < min_y) {
        entity->position.y = min_y;
    }
    if (entity->position.y > max_y - entity->height) {
        entity->position.y = max_y - entity->height;
    }
}

bool physics_check_collision(const Rectangle* a, const Rectangle* b) {
    if (!a || !b) return false;

    return (a->x < b->x + b->w &&
            a->x + a->w > b->x &&
            a->y < b->y + b->h &&
            a->y + a->h > b->y);
}

Rectangle physics_entity_to_rectangle(const Entity* entity) {
    Rectangle rect = {0};
    if (entity) {
        rect.x = entity->position.x;
        rect.y = entity->position.y;
        rect.w = entity->width;
        rect.h = entity->height;
    }
    return rect;
}

bool physics_entities_collide(const Entity* a, const Entity* b) {
    if (!a || !b || !a->active || !b->active) return false;

    Rectangle rect_a = physics_entity_to_rectangle(a);
    Rectangle rect_b = physics_entity_to_rectangle(b);

    return physics_check_collision(&rect_a, &rect_b);
}

void physics_clamp_entity_position(Entity* entity, float screen_width, float screen_height) {
    if (!entity || !entity->active) return;

    physics_apply_boundary_constraints(entity, 0.0f, screen_width, 0.0f, screen_height);
}