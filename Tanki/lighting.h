#ifndef LIGHTING_H
#define LIGHTING_H

#include "main.h"

// Константы для системы освещения
#define FOV_ANGLE 60.0f          // Угол поля зрения в градусах
#define MAX_VISIBILITY_RAYS 360  // Количество лучей для расчета видимости
#define WARNING_DURATION 180     // 3 секунды предупреждения (60 FPS * 3)
#define WARNING_PULSES 3         // Количество пульсаций
#define DARKNESS_START_TIME 900  // 15 секунд до начала темноты (60 FPS * 15)

// Структура для точки видимости
typedef struct {
    float x, y;
    float distance;
    bool blocked;
} VisibilityPoint;

// Глобальные переменные для системы освещения
extern bool darkness_active;
extern bool warning_active;
extern int darkness_timer;
extern int warning_timer;
extern int pulse_count;
extern float pulse_intensity;
extern VisibilityPoint visibility_polygon[MAX_VISIBILITY_RAYS];
extern int visibility_point_count;

// Функции системы освещения
void init_lighting(void);
void update_lighting(float dt);
void calculate_visibility(float player_x, float player_y, float player_angle);
bool is_point_visible(float x, float y, float player_x, float player_y, float player_angle);
void play_warning_sound(void);
void render_darkness_overlay(void);
void render_warning_pulse(void);

#endif /* LIGHTING_H */