#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "common.h"

#define MAX_TEXTURES 64
#define MAX_FILENAME_LENGTH 256

typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    SDL_Texture* texture;
    int width;
    int height;
    bool loaded;
} TextureAsset;

typedef struct {
    TextureAsset textures[MAX_TEXTURES];
    int texture_count;
    SDL_Renderer* renderer;
} AssetManager;

AssetManager* asset_manager_create(SDL_Renderer* renderer);
void asset_manager_destroy(AssetManager* manager);
SDL_Texture* asset_manager_load_texture(AssetManager* manager, const char* filename);
SDL_Texture* asset_manager_load_texture_from_memory(AssetManager* manager, const char* name, const unsigned char* data, size_t size);
SDL_Texture* asset_manager_get_texture(AssetManager* manager, const char* filename);
void asset_manager_get_texture_size(AssetManager* manager, const char* filename, int* width, int* height);

#endif // ASSET_MANAGER_H