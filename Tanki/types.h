#ifndef TYPES_H
#define TYPES_H
#include "main.h"
// Типы усилений (powerups)
typedef enum {
    POWERUP_HEALTH,
    POWERUP_SPEED,
    POWERUP_RAPID_FIRE,
    POWERUP_SHIELD,
    POWERUP_TRIPLE_SHOT,
    POWERUP_COUNT
} PowerupType;

// Состояния игры
typedef enum {
    GAME_MENU,
    GAME_PLAYING,
    GAME_PAUSED,
    GAME_OVER,
    GAME_WIN,
    GAME_LEVEL_TRANSITION
} GameState;

// Типы ботов
typedef enum {
    BOT_REGULAR,
    BOT_FAST,
    BOT_HEAVY,
    BOT_SNIPER,
    BOT_COUNT
} BotType;

// Структура объекта танка
typedef struct {
    float x, y;
    float angle;          // Угол поворота башни (для стрельбы)
    float movement_angle; // Угол поворота корпуса (для движения)
    float target_angle;   // Целевой угол поворота башни (для плавного вращения)
    float rotation_speed; // Скорость вращения башни
    int health;
    int max_health;
    bool shooting;
    int cooldown;
    float speed;
    float base_speed;
    int rapid_fire_timer;
    bool active;
    int respawn_timer;
    int score;
    int invulnerable_timer;
    int shield_timer;
    int triple_shot_timer;
    BotType type;

    // ИИ параметры
    float target_x, target_y;
    float ai_timer;
    float ai_state_timer;
    int ai_state;
    bool ai_has_target;
} Tank;

// Структура объекта снаряда
typedef struct {
    float x, y;
    float dx, dy;
    bool active;
    bool is_player;
    int damage;
    float life_time;
} Bullet;

// Структура объекта усиления
typedef struct {
    float x, y;
    PowerupType type;
    bool active;
    int timer;
    float rotation;
    float scale;
    bool scaling_up;
} Powerup;

// Структура для безопасного создания объектов
typedef struct {
    int x, y;
    float distance;
    bool valid;
} SpawnPoint;

// Структура для частиц эффектов
typedef struct {
    float x, y;
    float dx, dy;
    float life;
    float max_life;
    float size;
    float r, g, b;
    bool active;
} Particle;

// Информация о уровне
typedef struct {
    char name[50];
    char description[100];
    int bot_count;
    int difficulty;
} LevelInfo;

#endif /*TYPES_H */