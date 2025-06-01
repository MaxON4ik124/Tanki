#include "main.h"

// Карта с препятствиями
int map[MAP_HEIGHT][MAP_WIDTH] = { 0 };

// Текстурная карта для визуальных вариаций
int texture_map[MAP_HEIGHT][MAP_WIDTH] = { 0 };

// Генерация случайной карты
void generate_random_map() {
    // Сначала очищаем карту
    memset(map, TILE_EMPTY, sizeof(map));

    // Заполняем границы стенами
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (x == 0 || y == 0 || x == MAP_WIDTH - 1 || y == MAP_HEIGHT - 1) {
                map[y][x] = TILE_WALL;
            }
        }
    }

    // Добавляем случайные стены и разрушаемые блоки
    for (int y = 1; y < MAP_HEIGHT - 1; y++) {
        for (int x = 1; x < MAP_WIDTH - 1; x++) {
            int r = rand() % 100;
            if (r < 10) {
                map[y][x] = TILE_WALL;
            }
            else if (r < 25) {
                map[y][x] = TILE_BREAKABLE;
            }
            else if (r < 30) {
                map[y][x] = TILE_WATER;
            }
            else if (r < 35) {
                map[y][x] = TILE_ICE;
            }

            // Добавляем вариации текстур
            texture_map[y][x] = rand() % 4;
        }
    }

    // Убедимся, что на карте достаточно свободного пространства
    int empty_count = 0;
    for (int y = 1; y < MAP_HEIGHT - 1; y++) {
        for (int x = 1; x < MAP_WIDTH - 1; x++) {
            if (map[y][x] == TILE_EMPTY) {
                empty_count++;
            }
        }
    }

    // Если свободного пространства меньше 40%, удаляем некоторые препятствия
    if (empty_count < (MAP_WIDTH - 2) * (MAP_HEIGHT - 2) * 0.4) {
        for (int y = 1; y < MAP_HEIGHT - 1; y++) {
            for (int x = 1; x < MAP_WIDTH - 1; x++) {
                if (map[y][x] != TILE_EMPTY && rand() % 3 == 0) {
                    map[y][x] = TILE_EMPTY;
                }
            }
        }
    }
}

// Генерация карты-лабиринта
void generate_maze_map() {
    // Заполняем карту стенами
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            map[y][x] = TILE_WALL;
            texture_map[y][x] = rand() % 4;
        }
    }

    // Создаем лабиринт с использованием алгоритма "пьяного ходока"
    int x = 1, y = 1;
    map[y][x] = TILE_EMPTY;

    int steps = (MAP_WIDTH * MAP_HEIGHT) * 2; // Количество шагов для алгоритма

    for (int i = 0; i < steps; i++) {
        int direction = rand() % 4;
        int nx = x, ny = y;

        switch (direction) {
        case 0: ny -= 1; break; // Вверх
        case 1: ny += 1; break; // Вниз
        case 2: nx -= 1; break; // Влево
        case 3: nx += 1; break; // Вправо
        }

        if (nx > 0 && nx < MAP_WIDTH - 1 && ny > 0 && ny < MAP_HEIGHT - 1) {
            x = nx;
            y = ny;
            map[y][x] = TILE_EMPTY;
        }
    }

    // Добавляем разрушаемые блоки и специальные тайлы
    for (int y = 1; y < MAP_HEIGHT - 1; y++) {
        for (int x = 1; x < MAP_WIDTH - 1; x++) {
            if (map[y][x] == TILE_EMPTY && rand() % 10 == 0) {
                map[y][x] = TILE_BREAKABLE;
            }
            else if (map[y][x] == TILE_EMPTY && rand() % 20 == 0) {
                map[y][x] = TILE_WATER;
            }
            else if (map[y][x] == TILE_EMPTY && rand() % 20 == 0) {
                map[y][x] = TILE_ICE;
            }
        }
    }
}

// Генерация карты-арены
void generate_arena_map() {
    // Очищаем карту
    memset(map, TILE_EMPTY, sizeof(map));

    // Создаем границы
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (x == 0 || y == 0 || x == MAP_WIDTH - 1 || y == MAP_HEIGHT - 1) {
                map[y][x] = TILE_WALL;
            }
            texture_map[y][x] = rand() % 4;
        }
    }

    // Добавляем несколько препятствий для укрытия
    for (int i = 0; i < 10; i++) {
        int x = rand() % (MAP_WIDTH - 4) + 2;
        int y = rand() % (MAP_HEIGHT - 4) + 2;
        int size = rand() % 3 + 2;

        for (int dy = -size / 2; dy <= size / 2; dy++) {
            for (int dx = -size / 2; dx <= size / 2; dx++) {
                int nx = x + dx;
                int ny = y + dy;
                if (nx > 0 && nx < MAP_WIDTH - 1 && ny > 0 && ny < MAP_HEIGHT - 1) {
                    if (rand() % 3 == 0) {
                        map[ny][nx] = TILE_WALL;
                    }
                    else {
                        map[ny][nx] = TILE_BREAKABLE;
                    }
                }
            }
        }
    }

    // Добавляем специальные области
    for (int i = 0; i < 4; i++) {
        int x = rand() % (MAP_WIDTH - 6) + 3;
        int y = rand() % (MAP_HEIGHT - 6) + 3;
        int type = (rand() % 2) ? TILE_WATER : TILE_ICE;

        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int nx = x + dx;
                int ny = y + dy;
                if (nx > 0 && nx < MAP_WIDTH - 1 && ny > 0 && ny < MAP_HEIGHT - 1 && map[ny][nx] == TILE_EMPTY) {
                    map[ny][nx] = type;
                }
            }
        }
    }
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
void find_spawn_point(float* x, float* y) {
    int attempts = 0;
    bool found = false;

    while (!found && attempts < 100) {
        int tile_x = rand() % (MAP_WIDTH - 2) + 1;
        int tile_y = rand() % (MAP_HEIGHT - 2) + 1;

        if (map[tile_y][tile_x] == TILE_EMPTY) {
            float spawn_x = tile_x * TILE_SIZE + TILE_SIZE / 2;
            float spawn_y = tile_y * TILE_SIZE + TILE_SIZE / 2;

            // Проверяем, не занята ли эта точка другими танками
            bool collision = false;

            if (player.active) {
                if (distance(spawn_x, spawn_y, player.x, player.y) < TANK_SIZE * 2) {
                    collision = true;
                }
            }

            for (int i = 0; i < MAX_BOTS; i++) {
                if (bots[i].active) {
                    if (distance(spawn_x, spawn_y, bots[i].x, bots[i].y) < TANK_SIZE * 2) {
                        collision = true;
                        break;
                    }
                }
            }

            if (!collision) {
                *x = spawn_x;
                *y = spawn_y;
                found = true;
            }
        }

        attempts++;
    }

    // Если не удалось найти свободную точку, размещаем в центре
    if (!found) {
        *x = WIDTH / 2;
        *y = HEIGHT / 2;
    }
}