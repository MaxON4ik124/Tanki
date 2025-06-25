#include "main.h"

// Простая функция для рендера текста (упрощенная)
void draw_text(const char* text, float x, float y, float scale, float r, float g, float b) {
    if (!text || strlen(text) == 0) return;

    // Буфер для вершин (каждый символ может занимать до 270 байт)
    static char buffer[99999];
    int num_quads = stb_easy_font_print(x, y, (char*)text, NULL, buffer, sizeof(buffer));

    if (num_quads > 0) {
        // Сохраняем текущее состояние OpenGL
        glPushMatrix();

        // Применяем масштаб
        glScalef(scale, scale, 1.0f);

        // Устанавливаем цвет
        glColor3f(r, g, b);

        // Включаем массивы вершин
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 16, buffer);

        // Рисуем текст
        glDrawArrays(GL_QUADS, 0, num_quads * 4);

        // Отключаем массивы вершин
        glDisableClientState(GL_VERTEX_ARRAY);

        // Восстанавливаем матрицу
        glPopMatrix();
    }
}

// Отрисовка карты
void draw_map() {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            float x_pos = x * TILE_SIZE;
            float y_pos = y * TILE_SIZE;
            // Рисуем пол (фон)
            glColor3f(0.2f, 0.2f, 0.2f);
            glBegin(GL_QUADS);
            glVertex2f(x_pos, y_pos);
            glVertex2f(x_pos + TILE_SIZE, y_pos);
            glVertex2f(x_pos + TILE_SIZE, y_pos + TILE_SIZE);
            glVertex2f(x_pos, y_pos + TILE_SIZE);
            glEnd();

            // Добавляем текстуру пола
            int texture_variant = texture_map[y][x];
            glColor3f(0.25f, 0.25f, 0.25f);

            if (texture_variant == 1) {
                glBegin(GL_LINES);
                glVertex2f(x_pos + TILE_SIZE / 4, y_pos + TILE_SIZE / 4);
                glVertex2f(x_pos + TILE_SIZE * 3 / 4, y_pos + TILE_SIZE * 3 / 4);
                glVertex2f(x_pos + TILE_SIZE * 3 / 4, y_pos + TILE_SIZE / 4);
                glVertex2f(x_pos + TILE_SIZE / 4, y_pos + TILE_SIZE * 3 / 4);
                glEnd();
            }
            else if (texture_variant == 2) {
                glBegin(GL_LINE_LOOP);
                glVertex2f(x_pos + TILE_SIZE / 4, y_pos + TILE_SIZE / 4);
                glVertex2f(x_pos + TILE_SIZE * 3 / 4, y_pos + TILE_SIZE / 4);
                glVertex2f(x_pos + TILE_SIZE * 3 / 4, y_pos + TILE_SIZE * 3 / 4);
                glVertex2f(x_pos + TILE_SIZE / 4, y_pos + TILE_SIZE * 3 / 4);
                glEnd();
            }
            else if (texture_variant == 3) {
                glBegin(GL_POINTS);
                for (int i = 0; i < 5; i++) {
                    glVertex2f(x_pos + TILE_SIZE / 4 + i * TILE_SIZE / 8, y_pos + TILE_SIZE / 2);
                }
                glEnd();
            }

            switch (map[y][x]) {
            case TILE_WALL:
            {
                // Рисуем неразрушимую стену
                glColor3f(0.5f, 0.5f, 0.5f);
                glBegin(GL_QUADS);
                glVertex2f(x_pos, y_pos);
                glVertex2f(x_pos + TILE_SIZE, y_pos);
                glVertex2f(x_pos + TILE_SIZE, y_pos + TILE_SIZE);
                glVertex2f(x_pos, y_pos + TILE_SIZE);
                glEnd();

                // Добавляем текстуру стены
                glColor3f(0.4f, 0.4f, 0.4f);
                glBegin(GL_LINES);
                for (int i = 0; i < TILE_SIZE; i += TILE_SIZE / 4) {
                    glVertex2f(x_pos, y_pos + i);
                    glVertex2f(x_pos + TILE_SIZE, y_pos + i);
                }
                for (int i = 0; i < TILE_SIZE; i += TILE_SIZE / 4) {
                    glVertex2f(x_pos + i, y_pos);
                    glVertex2f(x_pos + i, y_pos + TILE_SIZE);
                }
                glEnd();
                break;
            }
            case TILE_BREAKABLE:
            {
                // Рисуем разрушаемую стену
                glColor3f(0.6f, 0.3f, 0.0f);
                glBegin(GL_QUADS);
                glVertex2f(x_pos, y_pos);
                glVertex2f(x_pos + TILE_SIZE, y_pos);
                glVertex2f(x_pos + TILE_SIZE, y_pos + TILE_SIZE);
                glVertex2f(x_pos, y_pos + TILE_SIZE);
                glEnd();

                // Добавляем "трещины" на разрушаемой стене
                glColor3f(0.5f, 0.25f, 0.0f);
                glBegin(GL_LINES);
                glVertex2f(x_pos + TILE_SIZE / 4, y_pos + TILE_SIZE / 4);
                glVertex2f(x_pos + TILE_SIZE * 3 / 4, y_pos + TILE_SIZE / 2);

                glVertex2f(x_pos + TILE_SIZE / 2, y_pos + TILE_SIZE / 4);
                glVertex2f(x_pos + TILE_SIZE / 4, y_pos + TILE_SIZE * 3 / 4);

                glVertex2f(x_pos + TILE_SIZE * 3 / 4, y_pos + TILE_SIZE / 4);
                glVertex2f(x_pos + TILE_SIZE / 2, y_pos + TILE_SIZE * 3 / 4);
                glEnd();
                break;
            }

            case TILE_WATER:
            {
                float water_offset = sinf((animation_time * 3.0f) + (x * 0.5f) + (y * 0.3f)) * 2.0f;

                glColor3f(0.0f, 0.3f, 0.7f);
                glBegin(GL_QUADS);
                glVertex2f(x_pos, y_pos);
                glVertex2f(x_pos + TILE_SIZE, y_pos);
                glVertex2f(x_pos + TILE_SIZE, y_pos + TILE_SIZE);
                glVertex2f(x_pos, y_pos + TILE_SIZE);
                glEnd();

                // Волны
                glColor3f(0.0f, 0.4f, 0.8f);
                glBegin(GL_LINES);
                for (int i = 0; i < TILE_SIZE; i += TILE_SIZE / 5) {
                    glVertex2f(x_pos, y_pos + i + water_offset);
                    glVertex2f(x_pos + TILE_SIZE, y_pos + i + water_offset);
                }
                glEnd();
                break;
            }

            case TILE_ICE:
            {
                // Рисуем лед
                glColor3f(0.8f, 0.9f, 1.0f);
                glBegin(GL_QUADS);
                glVertex2f(x_pos, y_pos);
                glVertex2f(x_pos + TILE_SIZE, y_pos);
                glVertex2f(x_pos + TILE_SIZE, y_pos + TILE_SIZE);
                glVertex2f(x_pos, y_pos + TILE_SIZE);
                glEnd();

                // Узор льда
                glColor3f(0.7f, 0.8f, 0.9f);
                glBegin(GL_LINES);
                glVertex2f(x_pos, y_pos);
                glVertex2f(x_pos + TILE_SIZE, y_pos + TILE_SIZE);

                glVertex2f(x_pos + TILE_SIZE, y_pos);
                glVertex2f(x_pos, y_pos + TILE_SIZE);
                glEnd();
                break;
            }
            }
        }
    }
}

// Отрисовка интерфейса
void draw_ui() {
    // Рисуем фон для интерфейса
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(WIDTH, 0);
    glVertex2f(WIDTH, 40);
    glVertex2f(0, 40);
    glEnd();

    glDisable(GL_BLEND);

    // Рисуем информацию о здоровье
    char health_text[50];
    sprintf(health_text, "Health: %d/%d", player.health, player.max_health);
    draw_text(health_text, 5, 2, 4.0f, 1.0f, 1.0f, 1.0f);

    // Рисуем счет
    char score_text[50];
    sprintf(score_text, "Score: %d", player.score);
    draw_text(score_text, 150, 2, 4.0f, 1.0f, 1.0f, 1.0f);

    // Рисуем текущий уровень
    char level_text[50];
    sprintf(level_text, "Level: %d/%d", level, MAX_LEVEL);
    draw_text(level_text, 250, 2, 4.0f, 1.0f, 1.0f, 1.0f);

    // Рисуем активные усиления
    float powerup_x = 350;

    if (player.shield_timer > 0) {
        glColor3f(0.0f, 0.6f, 1.0f);
        draw_text("Shield", powerup_x, 2, 4.0f, 0.0f, 0.6f, 1.0f);
        powerup_x += 50;
    }

    if (player.rapid_fire_timer > 0) {
        draw_text("Rapidety", powerup_x, 2, 4.0f, 1.0f, 0.8f, 0.0f);
        powerup_x += 50;
    }

    if (player.triple_shot_timer > 0) {
        draw_text("Multiple shot", powerup_x, 2, 4.0f, 1.0f, 0.4f, 0.8f);
        powerup_x += 50;
    }
    if (player.speed_timer > 0) {
        draw_text("Speed", powerup_x, 2, 4.0f, 0.0f, 1.0f, 0.0f);
        powerup_x += 50;
    }

    // Рисуем сообщение
    if (message_timer > 0) {
        float alpha = (message_timer < 60) ? message_timer / 60.0f : 1.0f;
        glColor4f(1.0f, 1.0f, 1.0f, alpha);
        glEnable(GL_BLEND);

        float message_width = strlen(game_message) * 15.0f; // Примерная ширина текста
        draw_text(game_message, (WIDTH - message_width) / 2, HEIGHT - 50, 4.0f, 1.0f, 1.0f, 1.0f);

        glDisable(GL_BLEND);
    }
}

// Отрисовка меню
void draw_menu() {
    darkness_timer = 0;
    warning_active = false;
    darkness_active = false;
    // Градиентный фон
    glBegin(GL_QUADS);
    glColor3f(0.1f, 0.1f, 0.3f);
    glVertex2f(0, 0);
    glVertex2f(WIDTH, 0);
    glColor3f(0.2f, 0.2f, 0.4f);
    glVertex2f(WIDTH, HEIGHT);
    glVertex2f(0, HEIGHT);
    glEnd();

    // Анимированный фон (звезды)
    glPointSize(2.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 200; i++) {
        float x = (sin(i * 3.14f + animation_time / 2) + 1) / 2 * WIDTH;
        float y = (cos(i * 2.71f + animation_time / 3) + 1) / 2 * HEIGHT;
        glVertex2f(x, y);
    }
    glEnd();

    // Заголовок
    float title_y = HEIGHT * 0.015f + sin(animation_time * 2) * 10;
    draw_text("2D TANKS", (WIDTH / 2 - 150) * 0.14f, title_y, 7.0f, 0.0f, 0.7f, 0.0f);

    // Пункты меню
    float menu_y = HEIGHT * 0.05f;
    float menu_spacing = 30;

    // Новая игра
    if (menu_selection == 0) {
        glColor3f(0.0f, 1.0f, 0.0f);
        draw_text("> New Game <", (WIDTH / 2 - 25) * 0.13f, menu_y, 6.0f, 0.0f, 1.0f, 0.0f);
    }
    else {
        glColor3f(0.8f, 0.8f, 0.8f);
        draw_text("New Game", (WIDTH / 2 + 75) * 0.13f, menu_y, 6.0f, 0.8f, 0.8f, 0.8f);
    }

    // Выбор уровня
    menu_y += menu_spacing;
    if (menu_selection == 1) {
        char level_text[150];
        sprintf(level_text, "> Level: %d - %s <", level, level_info[level].name);
        draw_text(level_text, (WIDTH / 2 - 250) * 0.13f, menu_y, 6.0f, 0.0f, 1.0f, 0.0f);
    }
    else {
        char level_text[150];
        sprintf(level_text, "Level: %d - %s", level, level_info[level].name);
        draw_text(level_text, (WIDTH / 2 - 150) * 0.13f, menu_y, 6.0f, 0.8f, 0.8f, 0.8f);
    }

    // Выход
    menu_y += menu_spacing;
    if (menu_selection == 2) {
        draw_text("> Exit <", (WIDTH / 2 + 100) * 0.13f, menu_y, 6.0f, 0.0f, 1.0f, 0.0f);
    }
    else {
        draw_text("Exit", (WIDTH / 2 + 200) * 0.13f, menu_y, 6.0f, 0.8f, 0.8f, 0.8f);
    }
    menu_y += 3*menu_spacing;
    // Инструкции
    glColor3f(0.7f, 0.7f, 0.7f);
    draw_text("Controls: WASD - movement, SAPCE - shoot, QE - tower rotation", (WIDTH / 2 - 350) * 0.13f, menu_y, 4.0f, 0.7f, 0.7f, 0.7f);
    menu_y += menu_spacing/2;
    draw_text("P - pause, ESC - main menu", (WIDTH / 2 + 300) * 0.13f, menu_y, 4.0f, 0.7f, 0.7f, 0.7f);
}

// Отрисовка экрана победы
void draw_win_screen() {
    // Фон с градиентом
    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.2f, 0.0f);
    glVertex2f(0, 0);
    glVertex2f(WIDTH, 0);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(WIDTH, HEIGHT);
    glVertex2f(0, HEIGHT);
    glEnd();

    // Анимированные звезды
    glPointSize(3.0f);
    glColor3f(1.0f, 1.0f, 0.3f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 300; i++) {
        float x = (sin(i * 1.23f + animation_time) + 1) / 2 * WIDTH;
        float y = (cos(i * 2.34f + animation_time * 1.5f) + 1) / 2 * HEIGHT;
        glVertex2f(x, y);
    }
    glEnd();

    // Текст победы
    float scale = 9.0f + sin(animation_time * 3) * 0.2f;
    glColor3f(0.0f, 1.0f, 0.0f);
    draw_text("VICTORY!", (WIDTH / 2 - 280) * 0.13f, (HEIGHT / 2 + 150) * 0.015f, scale, 0.0f, 1.0f, 0.0f);

    // Счет
    char score_text[50];
    sprintf(score_text, "Total score: %d", player.score);
    glColor3f(1.0f, 1.0f, 1.0f);
    draw_text(score_text, (WIDTH / 2 - 100) * 0.13f, (HEIGHT / 2 + 200) * 0.15f, 6.0f, 1.0f, 1.0f, 1.0f);

    // Поздравления
    glColor3f(0.9f, 0.9f, 0.0f);
    draw_text("Congratulations! You completed all levels!", (WIDTH / 2 - 520) * 0.13f, (HEIGHT / 2) * 0.15f, 6.0f, 0.9f, 0.9f, 0.0f);

    // Инструкция для продолжения
    if ((int)(animation_time * 2) % 2 == 0) {
        glColor3f(0.8f, 0.8f, 0.8f);
        draw_text("Press ENTER to exit in main menu", (WIDTH / 2 - 500) * 0.13f, (HEIGHT / 2 + 400) * 0.15f, 6.0f, 0.8f, 0.8f, 0.8f);
    }
}

// Отрисовка экрана паузы
void draw_pause_menu() {

    // Полупрозрачный фон
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(WIDTH, 0);
    glVertex2f(WIDTH, HEIGHT);
    glVertex2f(0, HEIGHT);
    glEnd();

    // Текст паузы
    float scale = 8.0f + sin(animation_time * 2) * 0.1f;
    glColor3f(1.0f, 1.0f, 1.0f);
    draw_text("PAUSE", (WIDTH / 2 - 200) * 0.13f, (HEIGHT / 2 - 450) * 0.15f, scale, 1.0f, 1.0f, 1.0f);

    // Инструкции
    glColor3f(0.8f, 0.8f, 0.8f);
    draw_text("Press P for continue", (WIDTH / 2 - 150) * 0.13f, (HEIGHT / 2) * 0.15f, 6.0f, 0.8f, 0.8f, 0.8f);
    draw_text("Press ESC for exit in main menu", (WIDTH / 2 - 380) * 0.13f, (HEIGHT / 2 + 300) * 0.15f, 6.0f, 0.8f, 0.8f, 0.8f);

    glDisable(GL_BLEND);
}

// Функция рендеринга
void draw_game_over() {

    glBegin(GL_QUADS);
    glColor3f(0.2f, 0.0f, 0.0f);
    glVertex2f(0, 0);
    glVertex2f(WIDTH, 0);
    glColor3f(0.5f, 0.0f, 0.0f);
    glVertex2f(WIDTH, HEIGHT);
    glVertex2f(0, HEIGHT);
    glEnd();

    // Текст "Game Over" - исправлено позиционирование
    float y_offset = sin(animation_time * 2) * 20.0f; // Анимация колебания
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f); // Добавлен альфа-канал
    draw_text("GAME OVER", (WIDTH / 2 - 250) * 0.13f, (HEIGHT / 2 - 450 + y_offset) * 0.15f, 8.0f, 1.0f, 0.0f, 0.0f);

    // Счет - исправлено позиционирование и масштаб
    char score_text[50];
    sprintf(score_text, "Your Score: %d", player.score);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    draw_text(score_text, (WIDTH / 2) * 0.13f, (HEIGHT / 2) * 0.15f, 6.0f, 1.0f, 1.0f, 1.0f);

    // Инструкция для продолжения - исправлено позиционирование и масштаб
    if ((int)(animation_time * 2) % 2 == 0) {
        glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
        draw_text("Press ENTER to return to main menu", (WIDTH / 2 - 450) * 0.13f, (HEIGHT / 2 + 500) * 0.15f, 6.0f, 0.8f, 0.8f, 0.8f);
    }

    glDisable(GL_BLEND);
}

// Исправленная основная функция рендеринга
void render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Добавлена очистка stencil buffer

    // Настройка проекции
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Отрисовка в зависимости от состояния игры
    if (game_state == GAME_MENU) {
        draw_menu();
    }

    if (game_state == GAME_PLAYING) {
        // Рисуем игровые объекты
        draw_map();

        // Рисуем усиления
        for (int i = 0; i < MAX_POWERUPS; i++) {
            if (powerups[i].active) {
                draw_powerup(powerups[i]);
            }
        }

        // Рисуем снаряды
        for (int i = 0; i < MAX_BULLETS * (MAX_BOTS + 1); i++) {
            if (bullets[i].active) {
                draw_bullet(bullets[i]);
            }
        }

        // Рисуем танки
        if (player.active) {
            draw_tank(player, true);
        }

        for (int i = 0; i < MAX_BOTS; i++) {
            if (bots[i].active) {
                draw_tank(bots[i], false);
            }
        }

        // Рисуем частицы
        draw_particles();

        // ВАЖНО: Сначала рисуем затемнение, потом интерфейс
        render_darkness_overlay();
        render_warning_pulse();

        // Рисуем интерфейс поверх затемнения
        draw_ui();
    }

    if (game_state == GAME_LEVEL_TRANSITION)
        draw_level_transition();

    if (game_state == GAME_PAUSED) {
        // Рисуем игровой мир под меню паузы
        draw_map();

        // Рисуем усиления
        for (int i = 0; i < MAX_POWERUPS; i++) {
            if (powerups[i].active) {
                draw_powerup(powerups[i]);
            }
        }

        // Рисуем снаряды
        for (int i = 0; i < MAX_BULLETS * (MAX_BOTS + 1); i++) {
            if (bullets[i].active) {
                draw_bullet(bullets[i]);
            }
        }

        // Рисуем танки
        if (player.active) {
            draw_tank(player, true);
        }

        for (int i = 0; i < MAX_BOTS; i++) {
            if (bots[i].active) {
                draw_tank(bots[i], false);
            }
        }

        // Рисуем частицы
        draw_particles();

        // Затемнение и предупреждение
        //render_darkness_overlay();
        render_warning_pulse();

        // Рисуем интерфейс
        draw_ui();

        // Рисуем меню паузы поверх всего
        draw_pause_menu();
    }

    if (game_state == GAME_OVER)
        draw_game_over();

    if (game_state == GAME_WIN)
        draw_win_screen();
}

// Отрисовка перехода между уровнями
void draw_level_transition() {
    float alpha = 0.0f;

    if (transition_fade_out) {
        // Фаза затемнения - альфа увеличивается от 0 до 1
        alpha = transition_timer / FADE_OUT_TIME;
        if (alpha > 1.0f) alpha = 1.0f;
    }
    else {
        // Фаза осветления - альфа уменьшается от 1 до 0
        alpha = 1.0f - (transition_timer / FADE_IN_TIME);
        if (alpha < 0.0f) alpha = 0.0f;
    }

    // Рисуем игровой мир (если он еще не затемнен полностью)
    if (!transition_fade_out || alpha < 1.0f) {
        draw_map();

        // Рисуем усиления
        for (int i = 0; i < MAX_POWERUPS; i++) {
            if (powerups[i].active) {
                draw_powerup(powerups[i]);
            }
        }

        // Рисуем снаряды
        for (int i = 0; i < MAX_BULLETS * (MAX_BOTS + 1); i++) {
            if (bullets[i].active) {
                draw_bullet(bullets[i]);
            }
        }

        // Рисуем танки
        if (player.active) {
            draw_tank(player, true);
        }

        for (int i = 0; i < MAX_BOTS; i++) {
            if (bots[i].active) {
                draw_tank(bots[i], false);
            }
        }

        // Рисуем частицы
        draw_particles();

        // Рисуем интерфейс
        draw_ui();
    }

    // Затемняющий слой
    glColor4f(0.0f, 0.0f, 0.0f, alpha);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(WIDTH, 0);
    glVertex2f(WIDTH, HEIGHT);
    glVertex2f(0, HEIGHT);
    glEnd();

    // Текст перехода (показываем в середине перехода)
    if (transition_fade_out && alpha > 0.5f) {
        float text_alpha = (alpha - 0.5f) * 2.0f; // Появляется во второй половине затемнения

        glColor4f(1.0f, 1.0f, 1.0f, text_alpha);

        char transition_text[100];
        if (next_level <= MAX_LEVEL) {
            sprintf(transition_text, "Level %d", next_level);
            draw_text(transition_text, (WIDTH / 2 + 350) * 0.13f, (HEIGHT / 2 - 350) * 0.15f, 5.0f, 1.0f, 1.0f, 1.0f);

            sprintf(transition_text, "%s", level_info[next_level].name);
            draw_text(transition_text, (WIDTH / 2 + 275) * 0.13f, (HEIGHT / 2) * 0.15f, 5.0f, 0.8f, 0.8f, 0.8f);

            draw_text(level_info[next_level].description, (WIDTH / 2) * 0.13f, (HEIGHT / 2 + 550) * 0.15f, 4.0f, 0.6f, 0.6f, 0.6f);
            
            if(next_level == LEVEL_TOWN_UP)
                draw_text("ATTENTION! After 15 secons lights will be put off!", (WIDTH / 2 - 450) * 0.13f, (HEIGHT / 2 + 670) * 0.15f, 5.0f, 1.0f, 0.0f, 0.0f);
        }
    }

    // Анимированные эффекты для красоты
    if (alpha > 0.3f) {
        glPointSize(2.0f + sin(animation_time * 4) * 0.5f);
        glColor4f(0.3f, 0.6f, 1.0f, alpha * 0.7f);
        glBegin(GL_POINTS);
        for (int i = 0; i < 100; i++) {
            float x = (sin(i * 1.23f + animation_time * 2) + 1) / 2 * WIDTH;
            float y = (cos(i * 2.34f + animation_time * 1.5f) + 1) / 2 * HEIGHT;
            glVertex2f(x, y);
        }
        glEnd();
    }

    glDisable(GL_BLEND);
}