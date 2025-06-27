#include "main.h"


int main() {
    if (!glfwInit()) {
        fprintf(stderr, "Ошибка инициализации GLFW\n");
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "2D TANKS", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Ошибка создания окна GLFW\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSwapInterval(1);


    srand((unsigned int)time(NULL));
    init_game();


    while (!glfwWindowShouldClose(window)) {
        double current_time = glfwGetTime();
        delta_time = current_time - last_time;
        last_time = current_time;
        animation_time += delta_time;

        glfwPollEvents();
        process_input(window);

        if (game_state == GAME_PLAYING) {
            update_game(delta_time);
        }
        else if (game_state == GAME_LEVEL_TRANSITION) {
            update_level_transition(delta_time);
        }


        render();
        render_warning_pulse();
        render_darkness_overlay();
        glfwSwapBuffers(window);
    }

        glfwTerminate();
        return 0;
    }