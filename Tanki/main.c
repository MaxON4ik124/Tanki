#include "main.h"

// Keep the OpenGL viewport in sync with the actual framebuffer size.
// This matters on HiDPI displays and also when running at a scaled video mode.
static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    if (width > 0 && height > 0) {
        glViewport(0, 0, width, height);
    }
}

int main(void) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // --- Memory-focused default framebuffer hints ---
    // 1) Disable MSAA to avoid multi-sampled color/depth buffers.
    glfwWindowHint(GLFW_SAMPLES, 0);

    // 2) We keep stencil for the darkness overlay. Depth is unused in this 2D renderer.
    //    Note: many platforms provide stencil as a combined depth+stencil buffer; in that case
    //    the driver may still allocate depth.
    glfwWindowHint(GLFW_DEPTH_BITS, 0);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    // 3) Prefer a 16-bit RGB framebuffer and no alpha if available.
    //    This can reduce bandwidth and (on some systems) memory footprint.
    glfwWindowHint(GLFW_RED_BITS, 5);
    glfwWindowHint(GLFW_GREEN_BITS, 6);
    glfwWindowHint(GLFW_BLUE_BITS, 5);
    glfwWindowHint(GLFW_ALPHA_BITS, 0);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();

    // Create a smaller fullscreen video mode to reduce default framebuffer memory.
    // WIDTH/HEIGHT remain the logical coordinate system used by the game.
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "2D TANKS", NULL, NULL);

    if (!window) {
        // Fallback: if the scaled video mode isn't supported, try the logical size.
        window = glfwCreateWindow(WIDTH, HEIGHT, "2D TANKS", monitor, NULL);
    }

    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Ensure viewport is initialized to the real framebuffer size.
    int fb_w = 0, fb_h = 0;
    glfwGetFramebufferSize(window, &fb_w, &fb_h);
    framebuffer_size_callback(window, fb_w, fb_h);

    glfwSwapInterval(1);

    srand((unsigned int)time(NULL));
    init_game();

    last_time = glfwGetTime();

    const double FIXED_DT = 1.0 / FPS;
    double accumulator = 0.0;

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        double frame_dt = now - last_time;
        last_time = now;

        // Prevent "spiral of death" if the window is paused/hitched.
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

        // render() already draws all overlays; don't draw them twice here.
        render();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
