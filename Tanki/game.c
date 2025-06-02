#include "main.h"

// Глобальные переменные
Tank player;
Tank bots[MAX_BOTS];
Bullet bullets[MAX_BULLETS * (MAX_BOTS + 1)];
Powerup powerups[MAX_POWERUPS];
bool keys[2048] = { 0 };
double last_time = 0;
double delta_time = 0;
GameState game_state = GAME_MENU;
int level = 1;
int max_level = 3;
char game_message[100] = "";
int message_timer = 0;
int powerup_spawn_timer = 0;
int menu_selection = 0;
float animation_time = 0.2f;
float menu_animation = 0.3f;

// Информация о уровне
LevelInfo level_info[MAX_LEVEL + 1] = {
    {"", "", 0, 0}, // Нулевой индекс не используется
    {"Стартовый уровень", "Тренировочная локация", 1, 1},
    {"Обычный уровень", "Стандартная карта с средним количеством укрытий", 1, 1},
    {"Обычный уровень (Дополнение)", "Карта как и на прошлом уровне, но врагов 2", 2, 1},
    {"Лабиринт", "Карта с множеством поворотов и узкими проходами", 2, 1},
    {"Арена", "Открытая арена с минимумом укрытий", 3, 1},
};

// Цвета для различных типов танков и объектов
float player_color[3] = { 0.0f, 0.7f, 0.0f };
float bot_colors[BOT_COUNT][3] = {
    {0.8f, 0.2f, 0.2f}, // Обычный - красный
    {1.0f, 0.6f, 0.0f}, // Быстрый - оранжевый
    {0.6f, 0.2f, 0.8f}, // Тяжелый - фиолетовый
    {0.2f, 0.2f, 0.8f}  // Снайпер - синий
};

float powerup_colors[POWERUP_COUNT][3] = {
    {1.0f, 0.2f, 0.2f}, // Здоровье - красный
    {0.2f, 0.8f, 0.2f}, // Скорость - зеленый
    {1.0f, 0.8f, 0.0f}, // Скорострельность - желтый
    {0.0f, 0.6f, 1.0f}, // Щит - голубой
    {1.0f, 0.4f, 0.8f}  // Тройной выстрел - розовый
};

// Инициализация игры
void init_game() {
    // Сбрасываем все данные
    memset(&player, 0, sizeof(Tank));
    memset(bots, 0, sizeof(Tank) * MAX_BOTS);
    memset(bullets, 0, sizeof(Bullet) * MAX_BULLETS * (MAX_BOTS + 1));
    memset(powerups, 0, sizeof(Powerup) * MAX_POWERUPS);
    memset(particles, 0, sizeof(Particle) * MAX_PARTICLES);

    // Инициализация игрока
    player.x = WIDTH / 2;
    player.y = HEIGHT / 2;
    player.angle = -90.0f;
    player.movement_angle = -90.0f;
    player.target_angle = 0.0f;
    player.rotation_speed = 180.0f; // 180 градусов в секунду
    player.health = 100;
    player.max_health = 100;
    player.speed = TANK_SPEED;
    player.base_speed = TANK_SPEED;
    player.active = true;
    player.score = 0;

    game_state = GAME_MENU;
    level = 1;
    strcpy(game_message, "");
    message_timer = 0;
}

// Инициализация конкретного уровня
void init_level(int level_num) {
    // Очищаем объекты с предыдущего уровня
    memset(bullets, 0, sizeof(Bullet) * MAX_BULLETS * (MAX_BOTS + 1));
    memset(powerups, 0, sizeof(Powerup) * MAX_POWERUPS);
    memset(particles, 0, sizeof(Particle) * MAX_PARTICLES);

    // Генерируем карту в зависимости от уровня
    switch (level_num) {
    case LEVEL_START:
        generate_map("map_start.txt");
        find_spawn_point(&player.x, &player.y, 0);
        break;
    case LEVEL_TOWN:
        generate_map("map_town.txt");
        find_spawn_point(&player.x, &player.y, 0);
        break;
    case LEVEL_TOWN_UP:
        generate_map("map_town_pro.txt");
        find_spawn_point(&player.x, &player.y, 0);
        break;
    case LEVEL_MAZE:
        generate_map("map_maze.txt");
        find_spawn_point(&player.x, &player.y, 0);
        break;
    case LEVEL_ARENA:
        generate_map("map_arena.txt");
        find_spawn_point(&player.x, &player.y, 0);
        break;
    }

    // Сбрасываем позицию игрока
    
    player.angle = 90.0f;
    player.movement_angle = 90.0f;
    player.target_angle = 0.0f;
    player.health = player.max_health;
    player.active = true;
    player.cooldown = 0;
    player.rapid_fire_timer = 0;
    player.shield_timer = 0;
    player.triple_shot_timer = 0;
    player.invulnerable_timer = 180; // 3 секунды неуязвимости при старте

    // Настройка ботов
    int bot_count = level_info[level_num].bot_count;
    for (int i = 0; i < MAX_BOTS; i++) {
        if (i < bot_count) {
            // Случайно выбираем тип бота с учетом сложности
            bots[i].type = rand() % BOT_COUNT;

            // Конфигурируем параметры бота в зависимости от типа
            switch (bots[i].type) {
            case BOT_FAST:
                bots[i].health = 50;
                bots[i].max_health = 50;
                bots[i].speed = BOT_SPEED * 1.5f;
                bots[i].base_speed = BOT_SPEED * 1.5f;
                bots[i].cooldown = RELOAD_TIME * 1.2;
                bots[i].rotation_speed = 210.0f; // Быстрее вращается
                break;
            case BOT_HEAVY:
                bots[i].health = 150;
                bots[i].max_health = 150;
                bots[i].speed = BOT_SPEED * 0.7f;
                bots[i].base_speed = BOT_SPEED * 0.7f;
                bots[i].cooldown = RELOAD_TIME * 1.5;
                bots[i].rotation_speed = 90.0f; // Медленнее вращается
                break;
            case BOT_SNIPER:
                bots[i].health = 75;
                bots[i].max_health = 75;
                bots[i].speed = BOT_SPEED * 0.9f;
                bots[i].base_speed = BOT_SPEED * 0.9f;
                bots[i].cooldown = RELOAD_TIME * 2.0;
                bots[i].rotation_speed = 150.0f; // Средняя скорость
                break;
            default: // BOT_REGULAR
                bots[i].health = 100;
                bots[i].max_health = 100;
                bots[i].speed = BOT_SPEED;
                bots[i].base_speed = BOT_SPEED;
                bots[i].cooldown = RELOAD_TIME;
                bots[i].rotation_speed = 180.0f; // Стандартная скорость
                break;
            }

            bots[i].active = true;
            bots[i].angle = (float)(rand() % 360);
            bots[i].movement_angle = bots[i].angle - 90.0f; // Изначально одинаковый угол
            bots[i].target_angle = bots[i].angle;
            find_spawn_point(&bots[i].x, &bots[i].y, 1);

            // Инициализация параметров ИИ
            bots[i].ai_timer = 0;
            bots[i].ai_state = 0;
            bots[i].ai_state_timer = (float)(rand() % 100) / 10.0f;
            bots[i].ai_has_target = false;
        }
        else {
            bots[i].active = false;
        }
    }

    // Создаем начальные усиления
    for (int i = 0; i < 2; i++) {
        spawn_powerup();
    }

    game_state = GAME_PLAYING;
    sprintf(game_message, "Уровень %d: %s", level_num, level_info[level_num].name);
    message_timer = 180; // 3 секунды
    powerup_spawn_timer = 300; // 5 секунд до появления нового усиления
}

// Обновление состояния игры
void update_game(float dt) {
    // Обновляем счетчики
    if (player.cooldown > 0) player.cooldown--;
    if (player.rapid_fire_timer > 0) player.rapid_fire_timer--;
    if (player.shield_timer > 0) player.shield_timer--;
    if (player.triple_shot_timer > 0) player.triple_shot_timer--;
    if (player.invulnerable_timer > 0) player.invulnerable_timer--;
    if (message_timer > 0) message_timer--;
    if (powerup_spawn_timer > 0) powerup_spawn_timer--;

    // Временные эффекты на скорость
    player.speed = player.base_speed;

    // Проверяем, стоит ли игрок на льду
    int tile_x = (int)(player.x / TILE_SIZE);
    int tile_y = (int)(player.y / TILE_SIZE);

    if (tile_x >= 0 && tile_x < MAP_WIDTH && tile_y >= 0 && tile_y < MAP_HEIGHT) {
        if (map[tile_y][tile_x] == TILE_ICE) {
            player.speed *= 1.5f; // На льду увеличивается скорость
        }
        else if (map[tile_y][tile_x] == TILE_WATER) {
            player.speed *= 0.5f; // В воде замедление
        }
    }

    // Периодически создаем новые усиления
    if (powerup_spawn_timer <= 0 && game_state == GAME_PLAYING) {
        spawn_powerup();
        powerup_spawn_timer = 300 + rand() % 600; // 5-15 секунд до следующего усиления
    }

    // Обновление компонентов игры
    update_bots(dt);
    update_bullets(dt);
    update_powerups(dt);
    update_particles(dt);
    check_collisions();

    // Проверка победы
    bool all_bots_dead = true;
    for (int i = 0; i < MAX_BOTS; i++) {
        if (bots[i].active) {
            all_bots_dead = false;
            break;
        }
    }

    if (all_bots_dead) {
        if (level < MAX_LEVEL) {
            level++;
            init_level(level);
        }
        else {
            game_state = GAME_WIN;
        }
    }

    // Проверка поражения
    if (!player.active && player.respawn_timer <= 0) {
        game_state = GAME_OVER;
    }
}