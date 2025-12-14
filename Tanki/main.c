#include "main.h"


int main() {
    if (!glfwInit()) {
        fprintf(stderr, "Ошибка инициализации GLFW\n");
        return -1;
    }
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "2D TANKS", monitor, NULL);
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

    // перед циклом
    last_time = glfwGetTime();

    const double FIXED_DT = 1.0 / FPS;
    double accumulator = 0.0;

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        double frame_dt = now - last_time;
        last_time = now;

        
        if (frame_dt > 0.25) frame_dt = 0.25;

        accumulator += frame_dt;
        animation_time += frame_dt;

        
        glfwPollEvents();

        
        while (accumulator >= FIXED_DT) {
            delta_time = FIXED_DT;              
            process_input(window);

            if (game_state == GAME_PLAYING) {
                update_game((float)FIXED_DT);
            }
            else if (game_state == GAME_LEVEL_TRANSITION) {
                update_level_transition((float)FIXED_DT);
            }

            accumulator -= FIXED_DT;
        }

        
        render();
        render_warning_pulse();
        render_darkness_overlay();

        glfwSwapBuffers(window);
    }


        glfwTerminate();
        return 0;
    }