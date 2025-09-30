/**
 * Game Bridge for RL Training
 *
 * This file provides a socket-based interface for communicating between
 * the C game and Python RL training environment.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

typedef struct {
    SOCKET server_socket;
    SOCKET client_socket;
    int port;
    bool connected;
} GameBridge;

static GameBridge bridge = {0};

/**
 * Initialize the game bridge server
 */
bool bridge_init(int port) {
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return false;
    }
#endif

    bridge.port = port;
    bridge.server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (bridge.server_socket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create socket\n");
        return false;
    }

    // Allow port reuse
    int opt = 1;
    setsockopt(bridge.server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(bridge.server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed on port %d\n", port);
        closesocket(bridge.server_socket);
        return false;
    }

    if (listen(bridge.server_socket, 1) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed\n");
        closesocket(bridge.server_socket);
        return false;
    }

    printf("Game bridge listening on port %d\n", port);
    return true;
}

/**
 * Wait for a client connection (blocking)
 */
bool bridge_accept_connection(void) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    printf("Waiting for RL agent connection...\n");
    bridge.client_socket = accept(bridge.server_socket, (struct sockaddr*)&client_addr, &addr_len);

    if (bridge.client_socket == INVALID_SOCKET) {
        fprintf(stderr, "Accept failed\n");
        return false;
    }

    bridge.connected = true;
    printf("RL agent connected!\n");
    return true;
}

/**
 * Receive action from the RL agent
 * Returns -1 for reset, -2 for error/disconnect, 0-4 for actions
 */
int bridge_receive_action(void) {
    if (!bridge.connected) {
        return -2;
    }

    int action;
    int bytes_received = recv(bridge.client_socket, (char*)&action, sizeof(action), 0);

    if (bytes_received <= 0) {
        bridge.connected = false;
        return -2;
    }

    return action;
}

/**
 * Send game state to the RL agent
 * Format: [message_length][JSON_string]
 */
bool bridge_send_state(const char* json_state) {
    if (!bridge.connected) {
        return false;
    }

    int msg_length = strlen(json_state);

    // Send length first
    if (send(bridge.client_socket, (const char*)&msg_length, sizeof(msg_length), 0) == SOCKET_ERROR) {
        bridge.connected = false;
        return false;
    }

    // Send JSON state
    if (send(bridge.client_socket, json_state, msg_length, 0) == SOCKET_ERROR) {
        bridge.connected = false;
        return false;
    }

    return true;
}

/**
 * Build JSON state string from game data
 */
char* bridge_build_state_json(
    float starship_x, float starship_y, float starship_vx, float starship_vy,
    int num_asteroids, float* asteroid_data, // [x, y, vx, vy, radius] per asteroid
    float reward, bool game_over
) {
    static char buffer[8192];
    int offset = 0;

    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
        "{\"starship\":{\"x\":%.2f,\"y\":%.2f,\"vx\":%.2f,\"vy\":%.2f},"
        "\"asteroids\":[",
        starship_x, starship_y, starship_vx, starship_vy
    );

    for (int i = 0; i < num_asteroids && i < 10; i++) {
        int idx = i * 5;
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
            "%s{\"x\":%.2f,\"y\":%.2f,\"vx\":%.2f,\"vy\":%.2f,\"radius\":%.2f}",
            i > 0 ? "," : "",
            asteroid_data[idx], asteroid_data[idx+1], asteroid_data[idx+2],
            asteroid_data[idx+3], asteroid_data[idx+4]
        );
    }

    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
        "],\"reward\":%.2f,\"game_over\":%s}",
        reward, game_over ? "true" : "false"
    );

    return buffer;
}

/**
 * Check if connected
 */
bool bridge_is_connected(void) {
    return bridge.connected;
}

/**
 * Cleanup
 */
void bridge_cleanup(void) {
    if (bridge.client_socket != INVALID_SOCKET) {
        closesocket(bridge.client_socket);
    }
    if (bridge.server_socket != INVALID_SOCKET) {
        closesocket(bridge.server_socket);
    }

#ifdef _WIN32
    WSACleanup();
#endif

    bridge.connected = false;
}