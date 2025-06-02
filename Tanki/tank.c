#include "main.h"

// Расчет расстояния между двумя точками
float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

// Проверка столкновения танка с другими танками
bool check_tank_collision(float x, float y, Tank* exclude_tank) {
    float radius = TANK_SIZE / 2;

    // Проверка столкновения с игроком
    if (player.active && exclude_tank != &player) {
        float dist = distance(x, y, player.x, player.y);
        if (dist < radius * 2) {
            return true;
        }
    }

    // Проверка столкновения с ботами
    for (int i = 0; i < MAX_BOTS; i++) {
        if (bots[i].active && &bots[i] != exclude_tank) {
            float dist = distance(x, y, bots[i].x, bots[i].y);
            if (dist < radius * 2) {
                return true;
            }
        }
    }

    return false;
}

// Обновление танков ботов
void update_bots(float dt) {
    for (int i = 0; i < MAX_BOTS; i++)
    {
        if (bots[i].active)
        {
            // Обновляем счетчики бота
            if (bots[i].cooldown > 0) bots[i].cooldown--;
            if (bots[i].invulnerable_timer > 0) bots[i].invulnerable_timer--;

            // Обновляем ИИ бота
            bots[i].ai_timer += dt;
            bots[i].ai_state_timer -= dt;

            // Проверяем стоит ли бот на специальном тайле
            int tile_x = (int)(bots[i].x / TILE_SIZE);
            int tile_y = (int)(bots[i].y / TILE_SIZE);

            float speed_multiplier = 1.0f;
            if (tile_x >= 0 && tile_x < MAP_WIDTH && tile_y >= 0 && tile_y < MAP_HEIGHT) {
                if (map[tile_y][tile_x] == TILE_ICE) {
                    speed_multiplier = 1.5f; // Ускорение на льду
                }
                else if (map[tile_y][tile_x] == TILE_WATER) {
                    speed_multiplier = 0.5f; // Замедление в воде
                }
            }

            // Вычисляем скорость бота с учетом типа и текущих условий
            bots[i].speed = bots[i].base_speed * speed_multiplier;

            // Логика ИИ ботов
            switch (bots[i].ai_state) {
            case 0: // Патрулирование
                if (bots[i].ai_state_timer <= 0) {
                    // Выбор новой точки патрулирования
                    float angle = (float)(rand() % 360) * M_PI / 180.0f;
                    bots[i].target_x = bots[i].x + cosf(angle) * 300.0f;
                    bots[i].target_y = bots[i].y + sinf(angle) * 300.0f;

                    // Ограничиваем точку границами карты
                    bots[i].target_x = fmaxf(TILE_SIZE, fminf(WIDTH - TILE_SIZE, bots[i].target_x));
                    bots[i].target_y = fmaxf(TILE_SIZE, fminf(HEIGHT - TILE_SIZE, bots[i].target_y));

                    bots[i].ai_state_timer = 2.0f + (rand() % 30) / 10.0f; // 2-5 секунд
                }

                // Движение к точке патрулирования
                float dx = bots[i].target_x - bots[i].x;
                float dy = bots[i].target_y - bots[i].y;
                float dist = sqrtf(dx * dx + dy * dy);

                if (dist > 5.0f) {
                    float angle = atan2f(dy, dx);
                    bots[i].angle = angle * 180.0f / M_PI;

                    float move_x = cosf(angle) * bots[i].speed * dt;
                    float move_y = sinf(angle) * bots[i].speed * dt;

                    float new_x = bots[i].x + move_x;
                    float new_y = bots[i].y + move_y;

                    // Проверка столкновений с картой
                    if (!check_map_collision(new_x, bots[i].y, TANK_SIZE / 2)) {
                        bots[i].x = new_x;
                    }
                    else {
                        // Если столкнулись со стеной, выбираем новую цель
                        bots[i].ai_state_timer = 0;
                    }

                    if (!check_map_collision(bots[i].x, new_y, TANK_SIZE / 2)) {
                        bots[i].y = new_y;
                    }
                    else {
                        bots[i].ai_state_timer = 0;
                    }
                }

                // Периодически проверяем, виден ли игрок
                if (bots[i].ai_timer >= 0.5f) {
                    bots[i].ai_timer = 0;

                    // Проверяем, есть ли прямая видимость до игрока
                    if (player.active) {
                        float dx = player.x - bots[i].x;
                        float dy = player.y - bots[i].y;
                        float dist = sqrtf(dx * dx + dy * dy);

                        // Если игрок достаточно близко
                        if (dist < 400.0f) {
                            // Проверяем прямую видимость
                            bool line_of_sight = true;
                            float step_x = dx / dist;
                            float step_y = dy / dist;
                            float check_x = bots[i].x;
                            float check_y = bots[i].y;

                            for (int j = 0; j < (int)dist; j += 10) {
                                check_x += step_x * 10;
                                check_y += step_y * 10;

                                int tile_x = (int)(check_x / TILE_SIZE);
                                int tile_y = (int)(check_y / TILE_SIZE);

                                if (tile_x >= 0 && tile_x < MAP_WIDTH && tile_y >= 0 && tile_y < MAP_HEIGHT) {
                                    if (map[tile_y][tile_x] == TILE_WALL || map[tile_y][tile_x] == TILE_BREAKABLE) {
                                        line_of_sight = false;
                                        break;
                                    }
                                }
                            }

                            if (line_of_sight) {
                                bots[i].ai_state = 1; // Переходим в состояние преследования
                                bots[i].ai_state_timer = 5.0f; // Преследуем 5 секунд
                            }
                        }
                    }
                }
                break;

            case 1: // Преследование игрока
                if (bots[i].ai_state_timer <= 0 || !player.active) {
                    bots[i].ai_state = 0; // Возвращаемся к патрулированию
                    bots[i].ai_state_timer = 1.0f;
                }
                else {
                    // Преследование игрока
                    float dx = player.x - bots[i].x;
                    float dy = player.y - bots[i].y;
                    float dist = sqrtf(dx * dx + dy * dy);

                    if (dist > 0) {
                        float angle = atan2f(dy, dx);
                        bots[i].angle = angle * 180.0f / M_PI;

                        // В зависимости от типа бота, поведение может различаться
                        bool should_move = true;
                        float shoot_range = 300.0f;

                        switch (bots[i].type) {
                        case BOT_SNIPER:
                            // Снайпер держит дистанцию
                            shoot_range = 500.0f;
                            if (dist < 200.0f) {
                                // Отступаем, если игрок слишком близко
                                angle += M_PI;
                            }
                            else if (dist < 300.0f) {
                                // Прекращаем движение, если на оптимальной дистанции
                                should_move = false;
                            }
                            break;
                        case BOT_FAST:
                            // Быстрый бот агрессивно сближается
                            shoot_range = 200.0f;
                            break;
                        case BOT_HEAVY:
                            // Тяжелый бот предпочитает среднюю дистанцию
                            if (dist < 150.0f) {
                                angle += M_PI;
                            }
                            break;
                        default:
                            // BOT_REGULAR - стандартное поведение
                            break;
                        }

                        if (should_move) {
                            float move_x = cosf(angle) * bots[i].speed * dt;
                            float move_y = sinf(angle) * bots[i].speed * dt;

                            float new_x = bots[i].x + move_x;
                            float new_y = bots[i].y + move_y;

                            if (!check_map_collision(new_x, bots[i].y, TANK_SIZE / 2) &&
                                !check_tank_collision(new_x, bots[i].y, &bots[i])) {
                                bots[i].x = new_x;
                            }

                            if (!check_map_collision(bots[i].x, new_y, TANK_SIZE / 2) &&
                                !check_tank_collision(bots[i].x, new_y, &bots[i])) {
                                bots[i].y = new_y;
                            }
                        }

                        // Стрельба по игроку
                        if (dist < shoot_range && bots[i].cooldown <= 0) {
                            float aim_angle = atan2f(dy, dx) * 180.0f / M_PI;
                            bots[i].angle = aim_angle;

                            // Поиск свободного слота для снаряда
                            for (int j = MAX_BULLETS; j < MAX_BULLETS * (MAX_BOTS + 1); j++) {
                                if (!bullets[j].active) {
                                    float rad = aim_angle * M_PI / 180.0f;
                                    bullets[j].x = bots[i].x + cosf(rad) * TANK_SIZE;
                                    bullets[j].y = bots[i].y + sinf(rad) * TANK_SIZE;
                                    bullets[j].dx = cosf(rad) * BULLET_SPEED;
                                    bullets[j].dy = sinf(rad) * BULLET_SPEED;
                                    bullets[j].active = true;
                                    bullets[j].is_player = false;
                                    bullets[j].damage = 20; // Базовый урон бота
                                    bullets[j].life_time = 2.0f; // Время жизни снаряда

                                    // Усиленный урон для снайпера
                                    if (bots[i].type == BOT_SNIPER) {
                                        bullets[j].damage = 40;
                                    }

                                    // Эффект выстрела
                                    add_particles(
                                        bullets[j].x, bullets[j].y,
                                        1.0f, 0.5f, 0.2f,  // Оранжевые частицы
                                        8,                  // Количество частиц
                                        40.0f,              // Скорость разлета
                                        2.5f,               // Размер
                                        0.4f                // Время жизни
                                    );
                                    break;
                                }
                            }

                            // Установка перезарядки в зависимости от типа бота
                            switch (bots[i].type) {
                            case BOT_SNIPER:
                                bots[i].cooldown = RELOAD_TIME * 2;
                                break;
                            case BOT_FAST:
                                bots[i].cooldown = RELOAD_TIME * 0.8;
                                break;
                            case BOT_HEAVY:
                                bots[i].cooldown = RELOAD_TIME * 1.5;
                                break;
                            default:
                                bots[i].cooldown = RELOAD_TIME;
                                break;
                            }
                        }
                    }
                }
                break;
            }
        }
    }
}

// Обновление снарядов
void update_bullets(float dt) {
    for (int i = 0; i < MAX_BULLETS * (MAX_BOTS + 1); i++) {
        if (bullets[i].active) {
            // Обновление позиции снаряда
            bullets[i].x += bullets[i].dx * dt;
            bullets[i].y += bullets[i].dy * dt;
            bullets[i].life_time -= dt;

            // Проверка времени жизни
            if (bullets[i].life_time <= 0) {
                bullets[i].active = false;
                continue;
            }

            // Проверка выхода за границы карты
            if (bullets[i].x < 0 || bullets[i].x > WIDTH || bullets[i].y < 0 || bullets[i].y > HEIGHT) {
                bullets[i].active = false;
                continue;
            }

            // Проверка столкновения со стенами
            int tile_x = (int)(bullets[i].x / TILE_SIZE);
            int tile_y = (int)(bullets[i].y / TILE_SIZE);

            if (tile_x >= 0 && tile_x < MAP_WIDTH && tile_y >= 0 && tile_y < MAP_HEIGHT) {
                if (map[tile_y][tile_x] == TILE_WALL) {
                    // Частицы при попадании в стену
                    add_particles(
                        bullets[i].x, bullets[i].y,
                        0.8f, 0.8f, 0.8f,  // Серые частицы
                        15,                 // Количество
                        70.0f,              // Скорость
                        2.0f,               // Размер
                        0.7f                // Время жизни
                    );
                    bullets[i].active = false;
                }
                else if (map[tile_y][tile_x] == TILE_BREAKABLE) {
                    // Разрушаем блок
                    map[tile_y][tile_x] = TILE_EMPTY;

                    // Частицы разрушения
                    add_particles(
                        tile_x * TILE_SIZE + TILE_SIZE / 2, tile_y * TILE_SIZE + TILE_SIZE / 2,
                        0.8f, 0.4f, 0.0f,  // Коричневые частицы
                        25,                 // Количество
                        100.0f,             // Скорость
                        3.0f,               // Размер
                        1.0f                // Время жизни
                    );

                    bullets[i].active = false;

                    // Шанс создать усиление при разрушении блока
                    if (rand() % 10 == 0) {
                        spawn_powerup();
                    }
                }
            }
        }
    }
}

// Проверка столкновений
void check_collisions() {
    // Проверка столкновений снарядов с танками
    for (int i = 0; i < MAX_BULLETS * (MAX_BOTS + 1); i++) {
        if (!bullets[i].active) continue;

        // Столкновение с игроком
        if (!bullets[i].is_player && player.active) {
            float dist = distance(bullets[i].x, bullets[i].y, player.x, player.y);
            if (dist < TANK_SIZE / 2 + BULLET_SIZE / 2) {
                // Если у игрока есть щит, снаряд отражается
                if (player.shield_timer > 0) {
                    bullets[i].is_player = true;
                    bullets[i].dx = -bullets[i].dx;
                    bullets[i].dy = -bullets[i].dy;

                    // Эффект отражения
                    add_particles(
                        bullets[i].x, bullets[i].y,
                        0.0f, 0.6f, 1.0f,  // Голубые частицы
                        10,                 // Количество
                        60.0f,              // Скорость
                        2.0f,               // Размер
                        0.5f                // Время жизни
                    );
                }
                // Иначе игрок получает урон
                else if (player.invulnerable_timer <= 0) {
                    player.health -= bullets[i].damage;

                    // Эффект попадания
                    create_explosion(
                        bullets[i].x, bullets[i].y,
                        1.0f, 0.3f, 0.1f,  // Оранжевые частицы для взрыва
                        20,                 // Количество
                        3.0f                // Размер
                    );

                    bullets[i].active = false;

                    // Если здоровье закончилось
                    if (player.health <= 0) {
                        player.active = false;
                        /*player.respawn_timer = RESPAWN_TIME;*/

                        // Большой взрыв при уничтожении танка
                        create_explosion(
                            player.x, player.y,
                            1.0f, 0.5f, 0.0f,  // Оранжевые частицы
                            50,                 // Количество
                            5.0f                // Размер
                        );
                    }
                }
            }
        }

        // Столкновение с ботами
        for (int j = 0; j < MAX_BOTS; j++) {
            if (bots[j].active && bullets[i].is_player) {
                float dist = distance(bullets[i].x, bullets[i].y, bots[j].x, bots[j].y);
                if (dist < TANK_SIZE / 2 + BULLET_SIZE / 2) {
                    if (bots[j].invulnerable_timer <= 0) {
                        bots[j].health -= bullets[i].damage;

                        // Эффект попадания
                        create_explosion(
                            bullets[i].x, bullets[i].y,
                            1.0f, 0.3f, 0.1f,  // Оранжевые частицы
                            15,                 // Количество
                            2.5f                // Размер
                        );

                        bullets[i].active = false;

                        // Если бот уничтожен
                        if (bots[j].health <= 0) {
                            bots[j].active = false;
                            bots[j].respawn_timer = RESPAWN_TIME * 2; // Более долгий респаун для ботов

                            // Большой взрыв при уничтожении
                            create_explosion(
                                bots[j].x, bots[j].y,
                                1.0f, 0.4f, 0.0f,  // Оранжевые частицы
                                40,                 // Количество
                                4.0f                // Размер
                            );

                            // Начисление очков игроку
                            player.score += 100 * (bots[j].type + 1);

                            // Шанс создать усиление при уничтожении бота
                            if (rand() % 3 == 0) {
                                spawn_powerup();
                            }
                        }
                    }
                }
            }
        }
    }

    // Проверка сбора усилений игроком
    if (player.active) {
        for (int i = 0; i < MAX_POWERUPS; i++) {
            if (powerups[i].active) {
                float dist = distance(player.x, player.y, powerups[i].x, powerups[i].y);
                if (dist < TANK_SIZE / 2 + POWERUP_SIZE / 2) {
                    apply_powerup(powerups[i].type);

                    // Эффект сбора усиления
                    add_particles(
                        powerups[i].x, powerups[i].y,
                        powerup_colors[powerups[i].type][0],
                        powerup_colors[powerups[i].type][1],
                        powerup_colors[powerups[i].type][2],
                        20,                 // Количество
                        80.0f,              // Скорость
                        3.0f,               // Размер
                        0.8f                // Время жизни
                    );

                    powerups[i].active = false;
                }
            }
        }
    }
}

// Отрисовка танка с раздельным поворотом корпуса и башни
void draw_tank(Tank tank, bool is_player) {
    if (!tank.active) return;

    // Определяем цвет танка
    float* color;
    if (is_player) {
        color = player_color;
    }
    else {
        color = bot_colors[tank.type];
    }

    // Рисуем полоску здоровья (фиксированное положение над танком)
    glPushMatrix();
    glTranslatef(tank.x, tank.y - TANK_SIZE / 1.5f, 0);

    float health_ratio = (float)tank.health / tank.max_health;
    float health_width = TANK_SIZE;

    // Фон полоски здоровья
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(-health_width / 2, -2);
    glVertex2f(health_width / 2, -2);
    glVertex2f(health_width / 2, 2);
    glVertex2f(-health_width / 2, 2);
    glEnd();

    // Заполнение полоски здоровья
    if (health_ratio > 0.7f) {
        glColor3f(0.0f, 1.0f, 0.0f); // Зеленый
    }
    else if (health_ratio > 0.3f) {
        glColor3f(1.0f, 1.0f, 0.0f); // Желтый
    }
    else {
        glColor3f(1.0f, 0.0f, 0.0f); // Красный
    }

    glBegin(GL_QUADS);
    glVertex2f(-health_width / 2, -2);
    glVertex2f(-health_width / 2 + health_width * health_ratio, -2);
    glVertex2f(-health_width / 2 + health_width * health_ratio, 2);
    glVertex2f(-health_width / 2, 2);
    glEnd();
    glPopMatrix();

    // Рисуем корпус танка (вращается в направлении движения)
    glPushMatrix();
    glTranslatef(tank.x, tank.y, 0);

    // Для игрока используем угол движения (хранится в tank.movement_angle)
    // Для ботов используем обычный угол (хранится в tank.angle)
    float body_angle = is_player ? tank.movement_angle : tank.angle;
    glRotatef(body_angle, 0, 0, 1);

    // Если танк неуязвим, мигаем прозрачностью
    if (tank.invulnerable_timer > 0 && ((tank.invulnerable_timer / 5) % 2 == 0)) {
        glColor4f(color[0], color[1], color[2], 0.5f);
    }
    else {
        glColor3f(color[0], color[1], color[2]);
    }

    // Рисуем гусеницы
    glBegin(GL_QUADS);
    // Левая гусеница
    glVertex2f(-TANK_SIZE / 2, -TANK_SIZE / 2);
    glVertex2f(-TANK_SIZE / 2 + TANK_SIZE / 5, -TANK_SIZE / 2);
    glVertex2f(-TANK_SIZE / 2 + TANK_SIZE / 5, TANK_SIZE / 2);
    glVertex2f(-TANK_SIZE / 2, TANK_SIZE / 2);

    // Правая гусеница
    glVertex2f(TANK_SIZE / 2 - TANK_SIZE / 5, -TANK_SIZE / 2);
    glVertex2f(TANK_SIZE / 2, -TANK_SIZE / 2);
    glVertex2f(TANK_SIZE / 2, TANK_SIZE / 2);
    glVertex2f(TANK_SIZE / 2 - TANK_SIZE / 5, TANK_SIZE / 2);
    glEnd();

    // Рисуем корпус
    glColor3f(color[0] * 0.8f, color[1] * 0.8f, color[2] * 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(-TANK_SIZE / 3, -TANK_SIZE / 3);
    glVertex2f(TANK_SIZE / 3, -TANK_SIZE / 3);
    glVertex2f(TANK_SIZE / 3, TANK_SIZE / 3);
    glVertex2f(-TANK_SIZE / 3, TANK_SIZE / 3);
    glEnd();

    // Сбрасываем поворот корпуса перед рисованием башни
    glRotatef(-body_angle, 0, 0, 1);

    // Вращаем башню в направлении стрельбы (или указателя мыши для игрока)
    glRotatef(tank.angle, 0, 0, 1);

    // Рисуем башню
    glColor3f(color[0] * 0.7f, color[1] * 0.7f, color[2] * 0.7f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0); // Центр круга
    for (int i = 0; i <= 360; i += 30) {
        float radian = i * M_PI / 180.0f;
        glVertex2f(cos(radian) * TANK_SIZE / 4, sin(radian) * TANK_SIZE / 4);
    }
    glEnd();

    // Рисуем ствол
    glColor3f(color[0] * 0.6f, color[1] * 0.6f, color[2] * 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(0, -TANK_SIZE / 20);
    glVertex2f(TANK_SIZE / 2 + TANK_SIZE / 10, -TANK_SIZE / 20);
    glVertex2f(TANK_SIZE / 2 + TANK_SIZE / 10, TANK_SIZE / 20);
    glVertex2f(0, TANK_SIZE / 20);
    glEnd();

    // Рисуем щит, если он активен
    if (tank.shield_timer > 0) {
        float shield_alpha = 0.3f + 0.2f * sinf(animation_time * 5.0f);
        glColor4f(0.0f, 0.6f, 1.0f, 0.5f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0, 0); // Центр круга
        for (int i = 0; i <= 360; i += 20) {
            float radian = i * M_PI / 180.0f;
            glVertex2f(cos(radian) * (TANK_SIZE / 1.5f), sin(radian) * (TANK_SIZE / 1.5f));
        }
        glEnd();
    }

    glPopMatrix();
}

// Отрисовка снаряда
void draw_bullet(Bullet bullet) {
    if (!bullet.active) return;

    glPushMatrix();
    glTranslatef(bullet.x, bullet.y, 0);


    // Цвет снаряда в зависимости от источника
    if (bullet.is_player) {
        glColor3f(0.0f, 0.8f, 0.0f); // Зеленый для игрока
    }
    else {
        glColor3f(1.0f, 0.3f, 0.0f); // Оранжевый для ботов
    }

    // Рисуем круглый снаряд
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0); // Центр круга
    for (int i = 0; i <= 360; i += 30) {
        float radian = i * M_PI / 180.0f;
        glVertex2f(cos(radian) * BULLET_SIZE / 2, sin(radian) * BULLET_SIZE / 2);
    }
    glEnd();

    // Добавляем след снаряда (частицы)
    if (rand() % 3 == 0) {
        float r = bullet.is_player ? 0.0f : 1.0f;
        float g = bullet.is_player ? 0.5f : 0.2f;
        float b = bullet.is_player ? 0.0f : 0.0f;

        add_particles(bullet.x, bullet.y, r, g, b, 1, 10.0f, 2.0f, 0.2f);
    }

    glPopMatrix();
}