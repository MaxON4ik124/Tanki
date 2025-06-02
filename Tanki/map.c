#include "main.h"

// Карта с препятствиями
int map[MAP_HEIGHT][MAP_WIDTH] = { 0 };

// Текстурная карта для визуальных вариаций
int texture_map[MAP_HEIGHT][MAP_WIDTH] = { 0 };

// Generation map from file
void generate_map(char* filename)
{
    memset(map, TILE_EMPTY, sizeof(map));
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (x == 0 || y == 0 || x == MAP_WIDTH - 1 || y == MAP_HEIGHT - 1) {
                map[y][x] = TILE_WALL;
            }
        }
    }
    FILE* mapfile = fopen(filename, "r");
    for (int y = 1; y < MAP_HEIGHT - 1; y++) {
        for (int x = 1; x < MAP_WIDTH - 1; x++) {
            fscanf(mapfile, "%d", &map[y][x]);
            texture_map[y][x] = rand() % 4;
            printf("%d", map[y][x]);

        }
        printf("\n");
    }
    fclose(mapfile);
}

// Проверка столкновения танка с картой
bool check_map_collision(float x, float y, float radius) {
    int tile_size = TILE_SIZE;

    // Проверяем ближайшие тайлы
    int start_x = (int)((x - radius) / tile_size);
    int end_x = (int)((x + radius) / tile_size);
    int start_y = (int)((y - radius) / tile_size);
    int end_y = (int)((y + radius) / tile_size);

    for (int ty = start_y; ty <= end_y; ty++) {
        for (int tx = start_x; tx <= end_x; tx++) {
            if (tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT) {
                if (map[ty][tx] == TILE_WALL || map[ty][tx] == TILE_BREAKABLE) {
                    // Проверяем ближайшую точку на тайле
                    float tile_center_x = tx * tile_size + tile_size / 2;
                    float tile_center_y = ty * tile_size + tile_size / 2;

                    float closest_x = fmaxf(tile_center_x - tile_size / 2, fminf(x, tile_center_x + tile_size / 2));
                    float closest_y = fmaxf(tile_center_y - tile_size / 2, fminf(y, tile_center_y + tile_size / 2));

                    float distance_x = x - closest_x;
                    float distance_y = y - closest_y;
                    float distance_squared = distance_x * distance_x + distance_y * distance_y;

                    if (distance_squared < radius * radius) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

// Поиск свободной точки для появления
void find_spawn_point(float* x_pos, float* y_pos, int tankType) {
    for (int y = 1;y < MAP_HEIGHT - 1;y++)
    {
        for (int x = 1;x < MAP_WIDTH - 1;x++)
        {
            if (map[y][x] == 10 && tankType == 0)
            {
                *x_pos = x * TILE_SIZE;
                *y_pos = y * TILE_SIZE;
                map[y][x] = TILE_EMPTY;
                return;
            }
            if (map[y][x] == 11 && tankType == 1)
            {
                *x_pos = x * TILE_SIZE;
                *y_pos = y * TILE_SIZE;
                map[y][x] = TILE_EMPTY;
                return;
            }
        }
    }
}