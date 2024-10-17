/*******************************************************************************************
*
*   raylib gamejam template
*
*   Template originally created with raylib 4.5-dev, last time updated with raylib 5.0
*
*   Template licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "LizardFreeCamera.h"
#include "LizardBlockWorld.h"

#if defined(PLATFORM_WEB)
    #define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include <emscripten/emscripten.h>      // Emscripten library - LLVM to JavaScript compiler
#endif

#include <stdio.h>                          // Required for: printf()
#include <stdlib.h>                         // Required for: 
#include <string.h>                         // Required for: 


static const int screenWidth = 800;
static const int screenHeight = 450;
static RenderTexture2D target = { 0 };

Texture2D LOGO;
Texture2D BLOCKS;


Shader 

#pragma region MINECRAFT

bool ProceduralBlocks;
//Chunk definitions
#define CHUNK_SIZE 16  // Chunk size: 16x16x16 blocks
#define BLOCK_SIZE 1.0f // Each block is 1x1x1 units
#define WORLD_WIDTH 4  // 4x4 chunks in total
#define WORLD_DEPTH 4
// Texture atlas definitions
#define ATLAS_WIDTH 2 // Number of textures in a row
#define ATLAS_HEIGHT 2 // Number of textures in a column
#define BLOCK_TEXTURE_SIZE 0.5f // Size of each block texture (1 / 4 for 4x2 atlas)


// Block types
enum BlockType
{
    Air,
    Dirt,
    Grass,
    Stone,
    Sand,
    Water,
    BlockTypeCount // Use this to keep track of the number of block types
};


typedef struct
{
    int type;  // 0 = air, 1 = solid block
} Block;

typedef struct {
    Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; // 3D array of blocks in the chunk
    Vector3 position; // World position of this chunk
    BoundingBox boundingBox; // The chunk's bounding box
    Mesh mesh; // Mesh for the chunk
    Model model;
    Color chunkColor;
    bool meshNeedsUpdate; // Whether we need to rebuild the chunk's mesh
} Chunk;


Chunk worldChunks[WORLD_WIDTH][WORLD_DEPTH];

Texture2D blocksTexture;

// Scale factors
float scale = 0.1f; // Adjust scale for the noise
float heightScale = CHUNK_SIZE; // Maximum height for your terrain
float islandFalloffFactor = 0.0f; // Factor to control the falloff (experiment with this value)
// Define the center of the world
int worldCenterX = (WORLD_WIDTH * CHUNK_SIZE) / 2;
int worldCenterZ = (WORLD_DEPTH * CHUNK_SIZE) / 2;
// Radial falloff function
float GetRadialFalloff(int x, int z) {
    float distanceX = x - worldCenterX;
    float distanceZ = z - worldCenterZ;
    float distance = sqrtf(distanceX * distanceX + distanceZ * distanceZ);

    // Normalize the distance so that it goes from 1 in the center to 0 at the edges
    float maxDistance = sqrtf((WORLD_WIDTH * CHUNK_SIZE / 2) * (WORLD_WIDTH * CHUNK_SIZE / 2) +
        (WORLD_DEPTH * CHUNK_SIZE / 2) * (WORLD_DEPTH * CHUNK_SIZE / 2));
    float falloff = 1.0f - (distance / maxDistance);

    // Apply a falloff factor to control how steep the tapering is
    falloff = powf(falloff, islandFalloffFactor); // Adjust this factor to control sharpness

    return falloff;
}
// Get height using Perlin noise with island falloff
float GetHeight(int x, int z) {
    // Generate noise value; you can adjust the parameters for different results
    float noiseValue = stb_perlin_noise3((float)x * scale, (float)z * scale, 0.0f, 4, 0.5f, 0.1f);

    // Apply the radial falloff to the noise-based height
    float falloff = GetRadialFalloff(x, z);
    return noiseValue * falloff * heightScale; // Scale the height based on the falloff
}
// Function to check if a chunk is inside the camera's frustum
bool IsChunkVisible(Camera3D camera, BoundingBox box)
{
    return true;
}
// Helper function to check if a block at (x, y, z) is solid
bool IsBlockSolid(Chunk* chunk, int x, int y, int z) {
    // If the block is out of bounds, return false (air)
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) return false;
    return chunk->blocks[x][y][z].type != Air;
}
// Function to get texture coordinates
Vector2 GetTextureCoord(int blockType)
{
    int x = blockType % ATLAS_WIDTH;
    int y = blockType / ATLAS_WIDTH;
    return (Vector2) { x* BLOCK_TEXTURE_SIZE, y* BLOCK_TEXTURE_SIZE };
}
//Function to generate the blocks mesh.
Mesh GenMeshCustom(Vector3* vertices, Vector2* texcoords, unsigned int* indices, int vertexCount, int indexCount) {
    Mesh mesh = { 0 };

    // Allocate space for vertices, texcoords, and indices
    mesh.vertexCount = vertexCount;
    mesh.triangleCount = indexCount / 3;

    mesh.vertices = (float*)MemAlloc(vertexCount * 3 * sizeof(float));   // 3 components (x, y, z)
    mesh.texcoords = (float*)MemAlloc(vertexCount * 2 * sizeof(float));  // 2 components (u, v)
    mesh.indices = (unsigned short*)MemAlloc(indexCount * sizeof(unsigned short));

    // Copy vertex data
    for (int i = 0; i < vertexCount; i++) {
        mesh.vertices[i * 3 + 0] = vertices[i].x;
        mesh.vertices[i * 3 + 1] = vertices[i].y;
        mesh.vertices[i * 3 + 2] = vertices[i].z;
        mesh.texcoords[i * 2 + 0] = texcoords[i].x;
        mesh.texcoords[i * 2 + 1] = texcoords[i].y;
    }

    // Copy index data
    for (int i = 0; i < indexCount; i++) {
        mesh.indices[i] = indices[i];
    }

    // Upload mesh data to the GPU (for fast rendering)
    UploadMesh(&mesh, true);

    return mesh;
}
//Generate mesh chunk function
void GenerateChunkMesh(Chunk* chunk) {


    if (chunk->mesh.vertexCount > 0) {
        UnloadMesh(chunk->mesh);
    }



    // Max number of vertices and indices for all blocks in the chunk (6 faces per block)
    const int maxVertices = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6 * 4;
    const int maxIndices = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6 * 6;

    Vector3* vertices = (Vector3*)MemAlloc(maxVertices * sizeof(Vector3));
    Vector2* texcoords = (Vector2*)MemAlloc(maxVertices * sizeof(Vector2));
    unsigned int* indices = (unsigned int*)MemAlloc(maxIndices * sizeof(unsigned int));

    int vertexCount = 0;
    int indexCount = 0;

    // For each block in the chunk
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                Block currentBlock = chunk->blocks[x][y][z];

                if (currentBlock.type != Air) {  // Only process solid blocks

                    Vector3 blockPosition = (Vector3){ x * BLOCK_SIZE, y * BLOCK_SIZE, z * BLOCK_SIZE };

                    // Get texture coordinates for the current block type
                    Vector2 texCoord = GetTextureCoord(currentBlock.type);

                    // Left face (west) - negative X direction
                    if (!IsBlockSolid(chunk, x - 1, y, z)) {
                        vertices[vertexCount + 0] = (Vector3){ blockPosition.x, blockPosition.y, blockPosition.z };
                        vertices[vertexCount + 1] = (Vector3){ blockPosition.x, blockPosition.y + BLOCK_SIZE, blockPosition.z };
                        vertices[vertexCount + 2] = (Vector3){ blockPosition.x, blockPosition.y + BLOCK_SIZE, blockPosition.z + BLOCK_SIZE };
                        vertices[vertexCount + 3] = (Vector3){ blockPosition.x, blockPosition.y, blockPosition.z + BLOCK_SIZE };

                        texcoords[vertexCount + 0] = (Vector2){ texCoord.x, texCoord.y };
                        texcoords[vertexCount + 1] = (Vector2){ texCoord.x, texCoord.y + BLOCK_TEXTURE_SIZE };
                        texcoords[vertexCount + 2] = (Vector2){ texCoord.x + BLOCK_TEXTURE_SIZE, texCoord.y + BLOCK_TEXTURE_SIZE };
                        texcoords[vertexCount + 3] = (Vector2){ texCoord.x + BLOCK_TEXTURE_SIZE, texCoord.y };

                        indices[indexCount + 0] = vertexCount + 0;
                        indices[indexCount + 1] = vertexCount + 1;
                        indices[indexCount + 2] = vertexCount + 2;
                        indices[indexCount + 3] = vertexCount + 0;
                        indices[indexCount + 4] = vertexCount + 2;
                        indices[indexCount + 5] = vertexCount + 3;

                        vertexCount += 4;
                        indexCount += 6;
                    }

                    // Right face (east) - positive X direction
                    if (!IsBlockSolid(chunk, x + 1, y, z)) {
                        vertices[vertexCount + 0] = (Vector3){ blockPosition.x + BLOCK_SIZE, blockPosition.y, blockPosition.z };
                        vertices[vertexCount + 1] = (Vector3){ blockPosition.x + BLOCK_SIZE, blockPosition.y + BLOCK_SIZE, blockPosition.z };
                        vertices[vertexCount + 2] = (Vector3){ blockPosition.x + BLOCK_SIZE, blockPosition.y + BLOCK_SIZE, blockPosition.z + BLOCK_SIZE };
                        vertices[vertexCount + 3] = (Vector3){ blockPosition.x + BLOCK_SIZE, blockPosition.y, blockPosition.z + BLOCK_SIZE };

                        texcoords[vertexCount + 0] = (Vector2){ texCoord.x, texCoord.y };
                        texcoords[vertexCount + 1] = (Vector2){ texCoord.x, texCoord.y + BLOCK_TEXTURE_SIZE };
                        texcoords[vertexCount + 2] = (Vector2){ texCoord.x + BLOCK_TEXTURE_SIZE, texCoord.y + BLOCK_TEXTURE_SIZE };
                        texcoords[vertexCount + 3] = (Vector2){ texCoord.x + BLOCK_TEXTURE_SIZE, texCoord.y };

                        indices[indexCount + 0] = vertexCount + 0;
                        indices[indexCount + 1] = vertexCount + 2;
                        indices[indexCount + 2] = vertexCount + 1;
                        indices[indexCount + 3] = vertexCount + 0;
                        indices[indexCount + 4] = vertexCount + 3;
                        indices[indexCount + 5] = vertexCount + 2;

                        vertexCount += 4;
                        indexCount += 6;
                    }

                    // Bottom face - negative Y direction
                    if (!IsBlockSolid(chunk, x, y - 1, z)) {
                        vertices[vertexCount + 0] = (Vector3){ blockPosition.x, blockPosition.y, blockPosition.z };
                        vertices[vertexCount + 1] = (Vector3){ blockPosition.x + BLOCK_SIZE, blockPosition.y, blockPosition.z };
                        vertices[vertexCount + 2] = (Vector3){ blockPosition.x + BLOCK_SIZE, blockPosition.y, blockPosition.z + BLOCK_SIZE };
                        vertices[vertexCount + 3] = (Vector3){ blockPosition.x, blockPosition.y, blockPosition.z + BLOCK_SIZE };

                        texcoords[vertexCount + 0] = (Vector2){ texCoord.x, texCoord.y };
                        texcoords[vertexCount + 1] = (Vector2){ texCoord.x + BLOCK_TEXTURE_SIZE, texCoord.y };
                        texcoords[vertexCount + 2] = (Vector2){ texCoord.x + BLOCK_TEXTURE_SIZE, texCoord.y + BLOCK_TEXTURE_SIZE };
                        texcoords[vertexCount + 3] = (Vector2){ texCoord.x, texCoord.y + BLOCK_TEXTURE_SIZE };

                        indices[indexCount + 0] = vertexCount + 0;
                        indices[indexCount + 1] = vertexCount + 2;
                        indices[indexCount + 2] = vertexCount + 1;
                        indices[indexCount + 3] = vertexCount + 0;
                        indices[indexCount + 4] = vertexCount + 3;
                        indices[indexCount + 5] = vertexCount + 2;

                        vertexCount += 4;
                        indexCount += 6;
                    }

                    // Top face - positive Y direction
                    if (!IsBlockSolid(chunk, x, y + 1, z)) {
                        vertices[vertexCount + 0] = (Vector3){ blockPosition.x, blockPosition.y + BLOCK_SIZE, blockPosition.z };
                        vertices[vertexCount + 1] = (Vector3){ blockPosition.x + BLOCK_SIZE, blockPosition.y + BLOCK_SIZE, blockPosition.z };
                        vertices[vertexCount + 2] = (Vector3){ blockPosition.x + BLOCK_SIZE, blockPosition.y + BLOCK_SIZE, blockPosition.z + BLOCK_SIZE };
                        vertices[vertexCount + 3] = (Vector3){ blockPosition.x, blockPosition.y + BLOCK_SIZE, blockPosition.z + BLOCK_SIZE };

                        texcoords[vertexCount + 0] = (Vector2){ texCoord.x, texCoord.y };
                        texcoords[vertexCount + 1] = (Vector2){ texCoord.x, texCoord.y + BLOCK_TEXTURE_SIZE };
                        texcoords[vertexCount + 2] = (Vector2){ texCoord.x + BLOCK_TEXTURE_SIZE, texCoord.y + BLOCK_TEXTURE_SIZE };
                        texcoords[vertexCount + 3] = (Vector2){ texCoord.x + BLOCK_TEXTURE_SIZE, texCoord.y };

                        indices[indexCount + 0] = vertexCount + 0;
                        indices[indexCount + 1] = vertexCount + 2;
                        indices[indexCount + 2] = vertexCount + 1;
                        indices[indexCount + 3] = vertexCount + 0;
                        indices[indexCount + 4] = vertexCount + 3;
                        indices[indexCount + 5] = vertexCount + 2;

                        vertexCount += 4;
                        indexCount += 6;
                    }

                    // Front face (north) - negative Z direction
                    if (!IsBlockSolid(chunk, x, y, z - 1)) {
                        vertices[vertexCount + 0] = (Vector3){ blockPosition.x, blockPosition.y, blockPosition.z };
                        vertices[vertexCount + 1] = (Vector3){ blockPosition.x, blockPosition.y + BLOCK_SIZE, blockPosition.z };
                        vertices[vertexCount + 2] = (Vector3){ blockPosition.x + BLOCK_SIZE, blockPosition.y + BLOCK_SIZE, blockPosition.z };
                        vertices[vertexCount + 3] = (Vector3){ blockPosition.x + BLOCK_SIZE, blockPosition.y, blockPosition.z };

                        texcoords[vertexCount + 0] = (Vector2){ texCoord.x, texCoord.y };
                        texcoords[vertexCount + 1] = (Vector2){ texCoord.x, texCoord.y + BLOCK_TEXTURE_SIZE };
                        texcoords[vertexCount + 2] = (Vector2){ texCoord.x + BLOCK_TEXTURE_SIZE, texCoord.y + BLOCK_TEXTURE_SIZE };
                        texcoords[vertexCount + 3] = (Vector2){ texCoord.x + BLOCK_TEXTURE_SIZE, texCoord.y };

                        indices[indexCount + 0] = vertexCount + 0;
                        indices[indexCount + 1] = vertexCount + 1;
                        indices[indexCount + 2] = vertexCount + 2;
                        indices[indexCount + 3] = vertexCount + 0;
                        indices[indexCount + 4] = vertexCount + 2;
                        indices[indexCount + 5] = vertexCount + 3;

                        vertexCount += 4;
                        indexCount += 6;
                    }

                    // Back face (south) - positive Z direction
                    if (!IsBlockSolid(chunk, x, y, z + 1)) {
                        vertices[vertexCount + 0] = (Vector3){ blockPosition.x, blockPosition.y, blockPosition.z + BLOCK_SIZE };
                        vertices[vertexCount + 1] = (Vector3){ blockPosition.x, blockPosition.y + BLOCK_SIZE, blockPosition.z + BLOCK_SIZE };
                        vertices[vertexCount + 2] = (Vector3){ blockPosition.x + BLOCK_SIZE, blockPosition.y + BLOCK_SIZE, blockPosition.z + BLOCK_SIZE };
                        vertices[vertexCount + 3] = (Vector3){ blockPosition.x + BLOCK_SIZE, blockPosition.y, blockPosition.z + BLOCK_SIZE };

                        texcoords[vertexCount + 0] = (Vector2){ texCoord.x, texCoord.y };
                        texcoords[vertexCount + 1] = (Vector2){ texCoord.x, texCoord.y + BLOCK_TEXTURE_SIZE };
                        texcoords[vertexCount + 2] = (Vector2){ texCoord.x + BLOCK_TEXTURE_SIZE, texCoord.y + BLOCK_TEXTURE_SIZE };
                        texcoords[vertexCount + 3] = (Vector2){ texCoord.x + BLOCK_TEXTURE_SIZE, texCoord.y };

                        indices[indexCount + 0] = vertexCount + 0;
                        indices[indexCount + 1] = vertexCount + 2;
                        indices[indexCount + 2] = vertexCount + 1;
                        indices[indexCount + 3] = vertexCount + 0;
                        indices[indexCount + 4] = vertexCount + 3;
                        indices[indexCount + 5] = vertexCount + 2;

                        vertexCount += 4;
                        indexCount += 6;
                    }
                }
            }
        }
    }

    chunk->mesh = GenMeshCustom(vertices, texcoords, indices, vertexCount, indexCount);
    chunk->model = LoadModelFromMesh(chunk->mesh);
    // Free the temporary arrays
    MemFree(vertices);
    MemFree(texcoords);
    MemFree(indices);
    chunk->meshNeedsUpdate = false;
}

// Function to draw all chunk meshes
void DrawChunks(Camera3D camera)
{
    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int z = 0; z < WORLD_DEPTH; z++) {
            Chunk* chunk = &worldChunks[x][z];

            // Only update the mesh if it is marked for update
            if (chunk->meshNeedsUpdate) {
                GenerateChunkMesh(chunk);
            }

            // Only draw the chunk if it's visible in the camera's frustum
            if (IsChunkVisible(camera, chunk->boundingBox))
            {
                SetMaterialTexture(&chunk->model.materials[0], MATERIAL_MAP_DIFFUSE, BLOCKS);
                DrawModel(chunk->model, chunk->position, 1.0f, WHITE);
            }
        }
    }
}

// Function to initialize chunks with random block types
void InitChunks() {
    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int z = 0; z < WORLD_DEPTH; z++) {


            /* if (worldChunks[x][z].mesh.vertexCount > 0) {
                 UnloadMesh(worldChunks[x][z].mesh);
             }*/

            worldChunks[x][z].position = (Vector3){ x * CHUNK_SIZE * BLOCK_SIZE, 0, z * CHUNK_SIZE * BLOCK_SIZE };
            worldChunks[x][z].boundingBox =
                (BoundingBox)
            {
                (Vector3) {
                    worldChunks[x][z].position.x, 0, worldChunks[x][z].position.z
                },
                (Vector3) {
                    worldChunks[x][z].position.x + CHUNK_SIZE * BLOCK_SIZE, CHUNK_SIZE* BLOCK_SIZE, worldChunks[x][z].position.z + CHUNK_SIZE * BLOCK_SIZE
                }
            };

            // Generate blocks based on Perlin noise and island falloff
            for (int bx = 0; bx < CHUNK_SIZE; bx++) {
                for (int bz = 0; bz < CHUNK_SIZE; bz++) {
                    // Calculate world coordinates
                    int worldX = x * CHUNK_SIZE + bx;
                    int worldZ = z * CHUNK_SIZE + bz;

                    // Get height using Perlin noise and falloff
                    float height = GetHeight(worldX, worldZ);
                    int heightInt = (int)height;

                    for (int by = 0; by < CHUNK_SIZE; by++) {
                        // Set block types based on height
                        if (by < heightInt)
                        {
                            if (by == heightInt - 1) {
                                worldChunks[x][z].blocks[bx][by][bz].type = Grass; // Top layer
                            }
                            else if (by >= heightInt - 3) {
                                worldChunks[x][z].blocks[bx][by][bz].type = Dirt; // Below top layer
                            }
                            else {
                                worldChunks[x][z].blocks[bx][by][bz].type = Stone; // Lower layers
                            }
                        }
                        else {
                            worldChunks[x][z].blocks[bx][by][bz].type = Air; // Air above the terrain
                        }
                    }
                }
            }

            // Create a default mesh and mark it for update
            //worldChunks[x][z].mesh = GenMeshCube(1.0f, 1.0f, 1.0f);  // Placeholder mesh
            worldChunks[x][z].meshNeedsUpdate = true;
        }
    }
}
#pragma endregion




//Main gametick function.
void UpdateGame(void)
{
    UpdateLizardFreeCam(EditMode, Vector3Zero());

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        LastMousePos = (Vector2){ GetScreenWidth() / 2, GetScreenHeight() / 2 };
        SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))
    {
        SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
    }

    for (int i = 0; i < 256; i++)
    {
        Vector3 POS = Vector3Add(Vector3One(), Vector3Zero());
    }

    BeginTextureMode(target);
        ClearBackground(BLACK);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(ViewCam);

        //DrawGrid(10, 1.0f);

        DrawChunks(ViewCam);

        EndMode3D();

        //DrawRectangle(10, 10, 320, 93, Fade(SKYBLUE, 0.5f));
        //DrawRectangleLines(10, 10, 320, 93, BLUE);

        //DrawText("Free camera default controls:", 20, 20, 10, BLACK);
        //DrawText("- Mouse Wheel to Zoom in-out", 40, 40, 10, DARKGRAY);
        //DrawText("- Mouse Wheel Pressed to Pan", 40, 60, 10, DARKGRAY);
        //DrawText("- Z to zoom to (0, 0, 0)", 40, 80, 10, DARKGRAY);

    EndTextureMode();
    
        DrawTexturePro(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height }, (Rectangle){ 0, 0, (float)target.texture.width, (float)target.texture.height }, (Vector2){ 0, 0 }, 0.0f, WHITE);
      //  DrawTexture(LOGO, 150, 0, WHITE);

        if (IsKeyDown(KEY_F))
        {
            DrawText("Hopefully it works - Lizard, 2024", GetScreenWidth() / 2, GetScreenHeight() / 2, 20, RED);
        }
        else
        {
            DrawText("Hopefully it works - Lizard, 2024", GetScreenWidth() / 2, GetScreenHeight() / 2, 20, BLUE);
        }
        DrawFPS(32, GetScreenHeight() - 32);

    EndDrawing();
}

//Entry point 
int main(void)
{
    
    InitWindow(screenWidth, screenHeight, "raylib gamejam template");
    target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
    InitLizardFreeCam(70.0f);
    LOGO = LoadTexture("resources/logo.png");
    BLOCKS = LoadTexture("resources/blocks.png");
    InitChunks();

    rlDisableBackfaceCulling();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateGame, 60, 1);
#else
    SetTargetFPS(60);     
    while (!WindowShouldClose())
    {
        UpdateGame();
    }
#endif

    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}