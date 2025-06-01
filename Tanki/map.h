#ifndef MAP_H
#define MAP_H

#include "main.h"

// Карта с препятствиями
extern int map[MAP_HEIGHT][MAP_WIDTH];

// Текстурная карта для визуальных вариаций
extern int texture_map[MAP_HEIGHT][MAP_WIDTH];

// Прототипы функций для работы с картой
void generate_random_map(void);
void generate_maze_map(void);
void generate_arena_map(void);
bool check_map_collision(float x, float y, float radius);
void find_spawn_point(float* x, float* y);

#endif /* MAP_H */
