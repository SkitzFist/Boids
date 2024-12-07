// #ifndef BOIDS_TILE_MAP_SIMD_H
// #define BOIDS_TILE_MAP_SIMD_H

// #include "ThreadPool.h"
// #include "Settings.h"
// #include <cmath>
// #include <immintrin.h>
// #include <mutex>
// #include <vector>

// struct TileMapSIMD {
//     // Array of tiles, where each tile stores the indices of entities currently in that tile.
//     std::vector<std::vector<int>> tiles;

//     // Entity to tile mapping to track which tile an entity is currently in.
//     std::vector<int> entityToTile;

//     // Lock for each tile to ensure thread safety.
//     std::vector<std::mutex> tileLocks;

//     TileMapSIMD(size_t numTiles, size_t numEntities)
//         : tiles(numTiles), entityToTile(numEntities, -1), tileLocks(numTiles) {}

//     // Clear the tile map using multithreading.
//     void clear(ThreadPool& threadPool) {
//         size_t numThreads = threadPool.numThreads;
//         size_t tilesPerThread = tiles.size() / numThreads;
//         size_t remainder = tiles.size() % numThreads;

//         for (size_t thread = 0; thread < numThreads; ++thread) {
//             size_t startIndex = thread * tilesPerThread;
//             size_t endIndex = startIndex + tilesPerThread;
//             if (thread == numThreads - 1) {
//                 endIndex += remainder; // Last thread takes care of the remainder
//             }

//             threadPool.enqueue([startIndex, endIndex, this]() {
//                 for (size_t i = startIndex; i < endIndex; ++i) {
//                     std::lock_guard<std::mutex> lock(tileLocks[i]);
//                     tiles[i].clear();
//                 }
//             });
//         }

//         threadPool.awaitCompletion();
//     }

//     // Assign entities to tiles using SIMD for position calculations.
//     void doEntityToTileSIMD(ThreadPool& threadPool, const std::vector<float>& posX, const std::vector<float>& posY) {
//         size_t entityCount = posX.size();
//         size_t numThreads = threadPool.numThreads;
//         size_t entitiesPerThread = entityCount / numThreads;
//         size_t remainder = entityCount % numThreads;

//         for (size_t thread = 0; thread < numThreads; ++thread) {
//             size_t startIndex = thread * entitiesPerThread;
//             size_t endIndex = startIndex + entitiesPerThread;
//             if (thread == numThreads - 1) {
//                 endIndex += remainder; // Last thread takes care of the remainder
//             }

//             threadPool.enqueue([startIndex, endIndex, &posX, &posY, this]() {
//                 size_t i = startIndex;

//                 // SIMD processing: process 8 entities at a time
//                 for (; i + 7 < endIndex; i += 8) {
//                     // Load positions using AVX
//                     __m256 xPositions = _mm256_loadu_ps(&posX[i]);
//                     __m256 yPositions = _mm256_loadu_ps(&posY[i]);

//                     // Calculate tile columns and rows
//                     __m256 tileCol_f = _mm256_floor_ps(_mm256_div_ps(xPositions, _mm256_set1_ps(WorldSettings::TILE_WIDTH)));
//                     __m256 tileRow_f = _mm256_floor_ps(_mm256_div_ps(yPositions, _mm256_set1_ps(WorldSettings::TILE_HEIGHT)));

//                     // Convert to integer indices
//                     __m256i tileCol = _mm256_cvttps_epi32(tileCol_f);
//                     __m256i tileRow = _mm256_cvttps_epi32(tileRow_f);

//                     // Calculate tile index: tileIndex = tileRow * WORLD_COLUMNS + tileCol
//                     __m256i tileIndex = _mm256_add_epi32(
//                         _mm256_mullo_epi32(tileRow, _mm256_set1_epi32(WorldSettings::WORLD_COLUMNS)),
//                         tileCol);

//                     // Store the tile indices back
//                     alignas(32) int indices[8];
//                     _mm256_store_si256((__m256i*)indices, tileIndex);

//                     // Assign entities to tiles
//                     for (int j = 0; j < 8; ++j) {
//                         int entityIndex = i + j;
//                         int previousTile = entityToTile[entityIndex];
//                         int newTile = indices[j];

//                         if (previousTile != -1) {
//                             // Remove entity from previous tile
//                             std::lock_guard<std::mutex> lock(tileLocks[previousTile]);
//                             auto& tileEntities = tiles[previousTile];
//                             tileEntities.erase(std::remove(tileEntities.begin(), tileEntities.end(), entityIndex), tileEntities.end());
//                         }

//                         // Add entity to the new tile
//                         {
//                             std::lock_guard<std::mutex> lock(tileLocks[newTile]);
//                             tiles[newTile].push_back(entityIndex);
//                         }

//                         // Update the entity to tile mapping
//                         entityToTile[entityIndex] = newTile;
//                     }
//                 }

//                 // Scalar processing for the remaining entities
//                 for (; i < endIndex; ++i) {
//                     int tileCol = static_cast<int>(posX[i] / WorldSettings::TILE_WIDTH);
//                     int tileRow = static_cast<int>(posY[i] / WorldSettings::TILE_HEIGHT);
//                     int tileIndex = tileRow * WorldSettings::WORLD_COLUMNS + tileCol;

//                     int previousTile = entityToTile[i];
//                     if (previousTile != -1) {
//                         std::lock_guard<std::mutex> lock(tileLocks[previousTile]);
//                         auto& tileEntities = tiles[previousTile];
//                         tileEntities.erase(std::remove(tileEntities.begin(), tileEntities.end(), i), tileEntities.end());
//                     }

//                     {
//                         std::lock_guard<std::mutex> lock(tileLocks[tileIndex]);
//                         tiles[tileIndex].push_back(i);
//                     }

//                     entityToTile[i] = tileIndex;
//                 }
//             });
//         }

//         threadPool.awaitCompletion();
//     }
// };

// #endif // BOIDS_TILE_MAP_SIMD_H
