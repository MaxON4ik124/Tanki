#include "main.h"

// Обновление усилений
void update_powerups(float dt) {
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (powerups[i].active) {
            powerups[i].timer--;

            if (powerups[i].timer <= 0) {
                powerups[i].active = false;
                continue;
            }

            // Анимация вращения
            powerups[i].rotation += 90.0f * dt;
            if (powerups[i].rotation > 360.0f) {
                powerups[i].rotation -= 360.0f;
            }

            // Анимация пульсации
            if (powerups[i].scaling_up) {
                powerups[i].scale += 0.5f * dt;
                if (powerups[i].scale >= 1.2f) {
                    powerups[i].scaling_up = false;
                }
            }
            else {
                powerups[i].scale -= 0.5f * dt;
                if (powerups[i].scale <= 0.8f) {
                    powerups[i].scaling_up = true;
                }
            }
        }
    }
}

// Создание усиления на карте
void spawn_powerup() {
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (!powerups[i].active) {
            // Ищем подходящую позицию
            float x, y;
            bool valid_position = false;
            int attempts = 0;

            while (!valid_position && attempts < 50) {
                int tile_x = rand() % (MAP_WIDTH - 2) + 1;
                int tile_y = rand() % (MAP_HEIGHT - 2) + 1;

                if (map[tile_y][tile_x] == TILE_EMPTY) {
                    x = tile_x * TILE_SIZE + TILE_SIZE / 2;
                    y = tile_y * TILE_SIZE + TILE_SIZE / 2;

                    // Проверяем, что усиление не появляется слишком близко к танкам
                    bool too_close = false;

                    if (player.active && distance(x, y, player.x, player.y) < TANK_SIZE * 2) {
                        too_close = true;
                    }

                    for (int j = 0; j < MAX_BOTS; j++) {
                        if (bots[j].active && distance(x, y, bots[j].x, bots[j].y) < TANK_SIZE * 2) {
                            too_close = true;
                            break;
                        }
                    }

                    if (!too_close) {
                        valid_position = true;
                    }
                }

                attempts++;
            }

            if (valid_position) {
                powerups[i].x = x;
                powerups[i].y = y;
                powerups[i].type = rand() % POWERUP_COUNT;
                powerups[i].active = true;
                powerups[i].timer = 600; // 10 секунд до исчезновения
                powerups[i].rotation = 0;
                powerups[i].scale = 1.0f;
                powerups[i].scaling_up = false;

                // Эффект появления
                add_particles(
                    powerups[i].x, powerups[i].y,
                    powerup_colors[powerups[i].type][0],
                    powerup_colors[powerups[i].type][1],
                    powerup_colors[powerups[i].type][2],
                    15,                 // Количество
                    60.0f,              // Скорость
                    2.5f,               // Размер
                    0.6f                // Время жизни
                );

                break;
            }
        }
    }
}

// Применение усиления
void apply_powerup(PowerupType type) {
    switch (type) {
    case POWERUP_HEALTH:
        // Восстановление здоровья
        player.health = fmin(player.max_health, player.health + 50);
        strcpy(game_message, "Получено лечение!");
        break;

    case POWERUP_SPEED:
        // Увеличение скорости
        player.base_speed = TANK_SIZE * 5.0f;
        strcpy(game_message, "Скорость увеличена!");
        // Эффект скорости длится 10 секунд, затем сбрасывается
        for (int i = 0; i < MAX_BULLETS * (MAX_BOTS + 1); i++) {
            if (!bullets[i].active) {
                bullets[i].active = true;
                bullets[i].is_player = true;
                bullets[i].x = player.x;
                bullets[i].y = player.y;
                bullets[i].dx = 0;
                bullets[i].dy = 0;
                bullets[i].damage = 0;
                bullets[i].life_time = 10.0f; // Таймер сброса скорости
                break;
            }
        }
        break;

    case POWERUP_RAPID_FIRE:
        // Ускоренная стрельба
        player.rapid_fire_timer = 300; // 5 секунд
        strcpy(game_message, "Скорострельность повышена!");
        break;

    case POWERUP_SHIELD:
        // Щит
        player.shield_timer = 300; // 5 секунд
        strcpy(game_message, "Щит активирован!");
        break;

    case POWERUP_TRIPLE_SHOT:
        // Тройной выстрел
        player.triple_shot_timer = 300; // 5 секунд
        strcpy(game_message, "Тройной выстрел активирован!");
        break;
    }

    message_timer = 120; // Сообщение на 2 секунды
}

// Отрисовка усиления
void draw_powerup(Powerup powerup) {
    if (!powerup.active) return;

    glPushMatrix();
    glTranslatef(powerup.x, powerup.y, 0);
    glRotatef(powerup.rotation, 0, 0, 1);
    glScalef(powerup.scale, powerup.scale, 1.0f);

    // Выбираем цвет в зависимости от типа усиления
    float* color = powerup_colors[powerup.type];
    glColor3f(color[0], color[1], color[2]);

    // Рисуем основу усиления (круг)
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0); // Центр круга
    for (int i = 0; i <= 360; i += 20) {
        float radian = i * M_PI / 180.0f;
        glVertex2f(cos(radian) * POWERUP_SIZE / 2, sin(radian) * POWERUP_SIZE / 2);
    }
    glEnd();

    // Рисуем внутреннюю часть в зависимости от типа
    glColor3f(1.0f, 1.0f, 1.0f);
    switch (powerup.type) {
    case POWERUP_HEALTH:
        // Рисуем символ "+"
        glBegin(GL_QUADS);
        // Вертикальная часть
        glVertex2f(-2, -10);
        glVertex2f(2, -10);
        glVertex2f(2, 10);
        glVertex2f(-2, 10);
        // Горизонтальная часть
        glVertex2f(-10, -2);
        glVertex2f(10, -2);
        glVertex2f(10, 2);
        glVertex2f(-10, 2);
        glEnd();
        break;

    case POWERUP_SPEED:
        // Рисуем символ молнии
        glBegin(GL_TRIANGLES);
        glVertex2f(-5, 10);
        glVertex2f(0, 0);
        glVertex2f(5, 0);

        glVertex2f(0, 0);
        glVertex2f(5, 0);
        glVertex2f(0, -10);
        glEnd();
        break;

    case POWERUP_RAPID_FIRE:
        // Рисуем символ пули
        glBegin(GL_TRIANGLES);
        glVertex2f(-8, 0);
        glVertex2f(0, -8);
        glVertex2f(0, 8);

        glVertex2f(0, -8);
        glVertex2f(0, 8);
        glVertex2f(8, 0);
        glEnd();
        break;

    case POWERUP_SHIELD:
        // Рисуем символ щита
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0, 0); // Центр
        for (int i = -90; i <= 90; i += 15) {
            float radian = i * M_PI / 180.0f;
            glVertex2f(cos(radian) * POWERUP_SIZE / 3, sin(radian) * POWERUP_SIZE / 3);
        }
        glEnd();
        break;

    case POWERUP_TRIPLE_SHOT:
        // Рисуем три линии
        glBegin(GL_LINES);
        // Левая линия
        glVertex2f(-8, -8);
        glVertex2f(-4, 8);
        // Средняя линия
        glVertex2f(0, -10);
        glVertex2f(0, 10);
        // Правая линия
        glVertex2f(8, -8);
        glVertex2f(4, 8);
        glEnd();
        break;
    }

    glPopMatrix();
}