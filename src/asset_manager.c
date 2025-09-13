#include "asset_manager.h"

AssetManager* asset_manager_create(SDL_Renderer* renderer) {
    AssetManager* manager = (AssetManager*)malloc(sizeof(AssetManager));
    if (!manager) {
        SDL_Log("Failed to allocate memory for AssetManager");
        return NULL;
    }
    
    manager->renderer = renderer;
    manager->texture_count = 0;
    
    for (int i = 0; i < MAX_TEXTURES; i++) {
        manager->textures[i].texture = NULL;
        manager->textures[i].loaded = false;
        manager->textures[i].width = 0;
        manager->textures[i].height = 0;
        manager->textures[i].filename[0] = '\0';
    }
    
    return manager;
}

void asset_manager_destroy(AssetManager* manager) {
    if (!manager) return;
    
    for (int i = 0; i < manager->texture_count; i++) {
        if (manager->textures[i].texture) {
            SDL_DestroyTexture(manager->textures[i].texture);
        }
    }
    
    free(manager);
}

SDL_Texture* asset_manager_load_texture(AssetManager* manager, const char* filename) {
    if (!manager || !filename) return NULL;
    
    SDL_Texture* existing = asset_manager_get_texture(manager, filename);
    if (existing) {
        return existing;
    }
    
    if (manager->texture_count >= MAX_TEXTURES) {
        SDL_Log("Asset manager texture limit reached");
        return NULL;
    }
    
    SDL_Surface* surface = NULL;
    
#ifdef HAVE_SDL_IMAGE
    surface = IMG_Load(filename);
#else
    surface = SDL_LoadBMP(filename);
#endif
    
    if (!surface) {
        SDL_Log("Failed to load texture '%s': %s", filename, SDL_GetError());
        return NULL;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(manager->renderer, surface);
    if (!texture) {
        SDL_Log("Failed to create texture from surface '%s': %s", filename, SDL_GetError());
        SDL_DestroySurface(surface);
        return NULL;
    }
    
    TextureAsset* asset = &manager->textures[manager->texture_count];
    strncpy(asset->filename, filename, MAX_FILENAME_LENGTH - 1);
    asset->filename[MAX_FILENAME_LENGTH - 1] = '\0';
    asset->texture = texture;
    asset->width = surface->w;
    asset->height = surface->h;
    asset->loaded = true;
    
    manager->texture_count++;
    
    SDL_DestroySurface(surface);
    
    SDL_Log("Loaded texture: %s (%dx%d)", filename, asset->width, asset->height);
    return texture;
}

SDL_Texture* asset_manager_get_texture(AssetManager* manager, const char* filename) {
    if (!manager || !filename) return NULL;
    
    for (int i = 0; i < manager->texture_count; i++) {
        if (manager->textures[i].loaded && strcmp(manager->textures[i].filename, filename) == 0) {
            return manager->textures[i].texture;
        }
    }
    
    return NULL;
}

void asset_manager_get_texture_size(AssetManager* manager, const char* filename, int* width, int* height) {
    if (!manager || !filename || !width || !height) return;
    
    for (int i = 0; i < manager->texture_count; i++) {
        if (manager->textures[i].loaded && strcmp(manager->textures[i].filename, filename) == 0) {
            *width = manager->textures[i].width;
            *height = manager->textures[i].height;
            return;
        }
    }
    
    *width = 0;
    *height = 0;
}