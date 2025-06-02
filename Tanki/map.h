#ifndef MAP_H
#define MAP_H

#include "main.h"

// Карта с препятствиями
extern int map[MAP_HEIGHT][MAP_WIDTH];

// Текстурная карта для визуальных вариаций
extern int texture_map[MAP_HEIGHT][MAP_WIDTH];

// Прототипы функций для работы с картой
bool check_map_collision(float x, float y, float radius);
void find_spawn_point(float* x_pos, float* y_pos, int tankType);
void generate_map(char* filename);

#endif /* MAP_H */
