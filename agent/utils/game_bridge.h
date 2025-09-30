/**
 * Game Bridge Header for RL Training
 */

#ifndef GAME_BRIDGE_H
#define GAME_BRIDGE_H

#include <stdbool.h>

/**
 * Initialize the game bridge server on specified port
 */
bool bridge_init(int port);

/**
 * Wait for and accept a client connection (blocking)
 */
bool bridge_accept_connection(void);

/**
 * Receive action from the RL agent
 * Returns: -1 = reset, -2 = error/disconnect, 0-4 = discrete actions
 */
int bridge_receive_action(void);

/**
 * Send game state as JSON to the RL agent
 */
bool bridge_send_state(const char* json_state);

/**
 * Build JSON state string from game data
 * asteroid_data format: [x, y, vx, vy, radius] per asteroid
 */
char* bridge_build_state_json(
    float starship_x, float starship_y, float starship_vx, float starship_vy,
    int num_asteroids, float* asteroid_data,
    float reward, bool game_over
);

/**
 * Check if client is connected
 */
bool bridge_is_connected(void);

/**
 * Cleanup resources
 */
void bridge_cleanup(void);

#endif // GAME_BRIDGE_H