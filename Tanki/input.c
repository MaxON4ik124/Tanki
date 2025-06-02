#include "main.h"

// Обработка ввода пользователя
void process_input(GLFWwindow* window) {
    if (game_state == GAME_PLAYING) {
        float speed = player.speed * delta_time;
        float dx = 0.0f, dy = 0.0f;
        bool movement_input = false;

        // Обработка движения по всем четырем направлениям независимо
        if (keys[GLFW_KEY_W]) {
            dy -= speed;
            movement_input = true;
        }
        if (keys[GLFW_KEY_S]) {
            dy += speed;
            movement_input = true;
        }
        if (keys[GLFW_KEY_A]) {
            dx -= speed;
            movement_input = true;
        }
        if (keys[GLFW_KEY_D]) {
            dx += speed;
            movement_input = true;
        }

        // Если игрок двигается, обновляем направление корпуса танка
        if (dx != 0.0f || dy != 0.0f) {
            // Расчет угла поворота корпуса на основе направления движения
            player.movement_angle = atan2f(dy, dx) * 180.0f / M_PI;

            // Нормализация диагонального движения (для сохранения постоянной скорости)
            if (dx != 0.0f && dy != 0.0f) {
                float length = sqrtf(dx * dx + dy * dy);
                dx = dx / length * speed;
                dy = dy / length * speed;
            }

            // Проверка столкновений и перемещение по осям X и Y отдельно
            float new_x = player.x + dx;
            float new_y = player.y + dy;

            if (!check_map_collision(new_x, player.y, TANK_SIZE / 2)) {
                player.x = new_x;
            }
            if (!check_map_collision(player.x, new_y, TANK_SIZE / 2)) {
                player.y = new_y;
            }
        }

        // Поворот башни с помощью стрелок (или у нас может быть поворот в сторону мыши)
        // В этом примере башня вращается с использованием стрелок
        if (keys[GLFW_KEY_Q]) {
            player.target_angle -= player.rotation_speed * delta_time;
        }
        if (keys[GLFW_KEY_E]) {
            player.target_angle += player.rotation_speed * delta_time;
        }

        // Плавный поворот башни к целевому углу
        float angle_diff = player.target_angle - player.angle;
        // Нормализация разности углов
        while (angle_diff > 180.0f) angle_diff -= 360.0f;
        while (angle_diff < -180.0f) angle_diff += 360.0f;

        // Плавное вращение башни
        if (fabsf(angle_diff) > 0.1f) {
            float rotation_amount = player.rotation_speed * delta_time;
            if (fabsf(angle_diff) < rotation_amount) {
                player.angle = player.target_angle;
            }
            else if (angle_diff > 0) {
                player.angle += rotation_amount;
            }
            else {
                player.angle -= rotation_amount;
            }

            // Нормализация угла до диапазона [0, 360)
            while (player.angle >= 360.0f) player.angle -= 360.0f;
            while (player.angle < 0.0f) player.angle += 360.0f;
        }

        // Стрельба
        if (keys[GLFW_KEY_SPACE] && player.cooldown <= 0) {
            player.shooting = true;

            int shots = 1;
            float angle_offset = 0;

            if (player.triple_shot_timer > 0) {
                shots = 3;
                angle_offset = 15.0f;
            }

            for (int i = 0; i < shots; i++) {
                float shot_angle = player.angle;
                if (shots > 1) shot_angle += angle_offset * (i - 1);

                float rad = shot_angle * M_PI / 180.0f;

                for (int j = 0; j < MAX_BULLETS; j++) {
                    if (!bullets[j].active) {
                        bullets[j].x = player.x + cosf(rad) * TANK_SIZE;
                        bullets[j].y = player.y + sinf(rad) * TANK_SIZE;
                        bullets[j].dx = cosf(rad) * BULLET_SPEED;
                        bullets[j].dy = sinf(rad) * BULLET_SPEED;
                        bullets[j].active = true;
                        bullets[j].is_player = true;
                        bullets[j].damage = 25;
                        bullets[j].life_time = 3.0f;

                        add_particles(
                            bullets[j].x, bullets[j].y,
                            1.0f, 0.8f, 0.0f,
                            10,
                            50.0f,
                            3.0f,
                            0.5f
                        );

                        break;
                    }
                }
            }

            player.cooldown = (player.rapid_fire_timer > 0)
                ? RELOAD_TIME / 2
                : RELOAD_TIME;
        }
        else {
            player.shooting = false;
        }
    }
    else if (game_state == GAME_MENU) {
        // Обработка выбора в меню
        if (keys[GLFW_KEY_UP] && !keys[GLFW_KEY_UP + 1000]) {
            keys[GLFW_KEY_UP + 1000] = true; // Флаг для предотвращения повторного срабатывания
            menu_selection = (menu_selection + 2) % 3;
        }

        if (keys[GLFW_KEY_DOWN] && !keys[GLFW_KEY_DOWN + 1000]) {
            keys[GLFW_KEY_DOWN + 1000] = true;
            menu_selection = (menu_selection + 1) % 3;
        }

        if (keys[GLFW_KEY_ENTER] && !keys[GLFW_KEY_ENTER + 1000]) {
            keys[GLFW_KEY_ENTER + 1000] = true;

            switch (menu_selection) {
            case 0: // Новая игра
                init_level(level);
                game_state = GAME_PLAYING;
                break;
            case 1: // Выбор уровня
                level = (level % MAX_LEVEL) + 1;
                break;
            case 2: // Выход
                glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
                break;
            }
        }
    }
    else if (game_state == GAME_PAUSED) {
        if (keys[GLFW_KEY_P] && !keys[GLFW_KEY_P + 1000]) {
            keys[GLFW_KEY_P + 1000] = true;
            game_state = GAME_PLAYING;
        }

        if (keys[GLFW_KEY_ESCAPE] && !keys[GLFW_KEY_ESCAPE + 1000]) {
            keys[GLFW_KEY_ESCAPE + 1000] = true;
            game_state = GAME_MENU;
        }
    }
    else if (game_state == GAME_OVER || game_state == GAME_WIN) {
        if (keys[GLFW_KEY_ENTER] && !keys[GLFW_KEY_ENTER + 1000]) {
            keys[GLFW_KEY_ENTER + 1000] = true;
            game_state = GAME_MENU;
        }
    }

    // Обновление флагов для предотвращения повторного срабатывания
    if (!keys[GLFW_KEY_UP]) keys[GLFW_KEY_UP + 1000] = false;
    if (!keys[GLFW_KEY_DOWN]) keys[GLFW_KEY_DOWN + 1000] = false;
    if (!keys[GLFW_KEY_ENTER]) keys[GLFW_KEY_ENTER + 1000] = false;
    if (!keys[GLFW_KEY_P]) keys[GLFW_KEY_P + 1000] = false;
    if (!keys[GLFW_KEY_ESCAPE]) keys[GLFW_KEY_ESCAPE + 1000] = false;
}

// Обработчик клавиатуры
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Обработка клавиш движения
    if (key == GLFW_KEY_W) {
        keys[GLFW_KEY_W] = (action != GLFW_RELEASE);
    }
    if (key == GLFW_KEY_S) {
        keys[GLFW_KEY_S] = (action != GLFW_RELEASE);
    }
    if (key == GLFW_KEY_A) {
        keys[GLFW_KEY_A] = (action != GLFW_RELEASE);
    }
    if (key == GLFW_KEY_D) {
        keys[GLFW_KEY_D] = (action != GLFW_RELEASE);
    }
    if (key == GLFW_KEY_Q) {
        keys[GLFW_KEY_Q] = (action != GLFW_RELEASE);
    }
    if (key == GLFW_KEY_E) {
        keys[GLFW_KEY_E] = (action != GLFW_RELEASE);
    }
    // Обработка клавиши стрельбы
    if (key == GLFW_KEY_SPACE) {
        keys[GLFW_KEY_SPACE] = (action != GLFW_RELEASE);
    }

    // Обработка клавиши паузы
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        keys[GLFW_KEY_P] = true;
        if (game_state == GAME_PLAYING) {
            game_state = GAME_PAUSED;
        }
        else if (game_state == GAME_PAUSED) {
            game_state = GAME_PLAYING;
        }
    }

    // Обработка клавиши Escape для выхода в меню
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        keys[GLFW_KEY_ESCAPE] = true;
        if (game_state == GAME_PLAYING || game_state == GAME_PAUSED) {
            game_state = GAME_MENU;
        }
    }

    // Обработка клавиш навигации в меню
    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        keys[GLFW_KEY_UP] = true;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        keys[GLFW_KEY_DOWN] = true;
    }
    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        keys[GLFW_KEY_ENTER] = true;
    }
}