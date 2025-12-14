#include "main.h"

// -----------------------------------------------------------------------------
// Lighting / Darkness system
//
// Optimizations included in this version:
//  - Visibility ray casting uses tile-grid DDA (instead of tiny step marching).
//  - Max ray distance (screen diagonal) is computed once in init_lighting().
//  - Player "light circle" uses a cached unit circle (no per-frame sinf/cosf).
//  - Several checks use squared distances to avoid unnecessary sqrtf.
// -----------------------------------------------------------------------------

bool darkness_active = false;
bool warning_active = false;
int darkness_timer = 0;
int warning_timer = 0;
int pulse_count = 0;
float pulse_intensity = 0.0f;

VisibilityPoint visibility_polygon[MAX_VISIBILITY_RAYS];
int visibility_point_count = 0;

// Precomputed "far enough" distance (screen diagonal) for visibility rays.
static float g_max_ray_distance = 0.0f;

// Cached unit circle for the small player light radius circle.
enum { PLAYER_LIGHT_STEP_DEG = 10, PLAYER_LIGHT_POINTS = 360 / PLAYER_LIGHT_STEP_DEG + 1 };
static float g_unit_circle_x[PLAYER_LIGHT_POINTS];
static float g_unit_circle_y[PLAYER_LIGHT_POINTS];
static bool g_unit_circle_ready = false;

static void init_unit_circle(void)
{
    if (g_unit_circle_ready) return;

    const float deg2rad = (float)M_PI / 180.0f;
    for (int i = 0; i < PLAYER_LIGHT_POINTS; i++) {
        float a = (float)(i * PLAYER_LIGHT_STEP_DEG) * deg2rad;
        g_unit_circle_x[i] = cosf(a);
        g_unit_circle_y[i] = sinf(a);
    }
    g_unit_circle_ready = true;
}

// Tile-grid DDA ray cast.
// dir_x/dir_y must be normalized. max_distance is in world units (pixels).
// Returns true if a wall/breakable tile blocks the ray (or the ray exits the map).
static bool raycast_dda(float x0, float y0,
    float dir_x, float dir_y,
    float max_distance,
    float* hit_x, float* hit_y, float* hit_distance)
{
    // Convert to tile coordinates.
    int map_x = (int)(x0 / TILE_SIZE);
    int map_y = (int)(y0 / TILE_SIZE);

    // If we start outside the map, treat as blocked immediately.
    if (map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT) {
        *hit_x = x0;
        *hit_y = y0;
        *hit_distance = 0.0f;
        return true;
    }

    // If starting tile is a wall, also treat as blocked.
    if (map[map_y][map_x] == TILE_WALL || map[map_y][map_x] == TILE_BREAKABLE) {
        *hit_x = x0;
        *hit_y = y0;
        *hit_distance = 0.0f;
        return true;
    }

    int step_x = (dir_x < 0.0f) ? -1 : 1;
    int step_y = (dir_y < 0.0f) ? -1 : 1;

    // Distance (t) to the first vertical/horizontal boundary.
    float next_boundary_x = (step_x > 0) ? ((map_x + 1) * TILE_SIZE) : (map_x * TILE_SIZE);
    float next_boundary_y = (step_y > 0) ? ((map_y + 1) * TILE_SIZE) : (map_y * TILE_SIZE);

    // Use a large number for near-zero components.
    const float INF = FLT_MAX;

    float t_max_x = (dir_x == 0.0f) ? INF : (next_boundary_x - x0) / dir_x;
    float t_max_y = (dir_y == 0.0f) ? INF : (next_boundary_y - y0) / dir_y;

    if (t_max_x < 0.0f) t_max_x = 0.0f;
    if (t_max_y < 0.0f) t_max_y = 0.0f;

    float t_delta_x = (dir_x == 0.0f) ? INF : (TILE_SIZE / fabsf(dir_x));
    float t_delta_y = (dir_y == 0.0f) ? INF : (TILE_SIZE / fabsf(dir_y));

    float t = 0.0f;
    while (t <= max_distance) {
        if (t_max_x < t_max_y) {
            map_x += step_x;
            t = t_max_x;
            t_max_x += t_delta_x;
        }
        else {
            map_y += step_y;
            t = t_max_y;
            t_max_y += t_delta_y;
        }

        // Exited the map.
        if (map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT) {
            if (t > max_distance) t = max_distance;
            *hit_x = x0 + dir_x * t;
            *hit_y = y0 + dir_y * t;
            *hit_distance = t;
            return true;
        }

        // Hit a blocking tile.
        if (map[map_y][map_x] == TILE_WALL || map[map_y][map_x] == TILE_BREAKABLE) {
            if (t > max_distance) t = max_distance;
            *hit_x = x0 + dir_x * t;
            *hit_y = y0 + dir_y * t;
            *hit_distance = t;
            return true;
        }
    }

    // No hit: end at max distance.
    *hit_x = x0 + dir_x * max_distance;
    *hit_y = y0 + dir_y * max_distance;
    *hit_distance = max_distance;
    return false;
}

bool ray_intersects_wall(float x1, float y1, float x2, float y2,
    float* hit_x, float* hit_y, float* distance)
{
    float dx = x2 - x1;
    float dy = y2 - y1;

    float len2 = dx * dx + dy * dy;
    if (len2 <= 0.000001f) {
        *hit_x = x1;
        *hit_y = y1;
        *distance = 0.0f;
        return false;
    }

    float len = sqrtf(len2);
    float inv = 1.0f / len;

    float dir_x = dx * inv;
    float dir_y = dy * inv;

    return raycast_dda(x1, y1, dir_x, dir_y, len, hit_x, hit_y, distance);
}

void init_lighting(void) {
    darkness_active = false;
    warning_active = false;
    darkness_timer = 0;
    warning_timer = 0;
    pulse_count = 0;
    pulse_intensity = 0.0f;
    visibility_point_count = 0;

    // Compute once: screen diagonal in world units (logical coords).
    g_max_ray_distance = sqrtf((float)WIDTH * (float)WIDTH + (float)HEIGHT * (float)HEIGHT);

    init_unit_circle();
}

void update_lighting(float dt) {
    (void)dt;

    if (level < 3 || game_state != GAME_PLAYING) {
        darkness_active = false;
        warning_active = false;
        return;
    }

    darkness_timer++;

    if (darkness_timer >= DARKNESS_START_TIME && !warning_active && !darkness_active) {
        warning_active = true;
        warning_timer = 0;
        pulse_count = 0;
    }

    if (warning_active) {
        warning_timer++;

        float pulse_cycle = (float)(warning_timer % (WARNING_DURATION / WARNING_PULSES));
        float pulse_progress = pulse_cycle / (float)(WARNING_DURATION / WARNING_PULSES);

        if (pulse_progress <= 0.5f) {
            pulse_intensity = pulse_progress * 2.0f;
        }
        else {
            pulse_intensity = 2.0f - pulse_progress * 2.0f;
        }

        if (warning_timer >= WARNING_DURATION) {
            warning_active = false;
            darkness_active = true;
            pulse_intensity = 0.0f;
        }
    }

    if (darkness_active) {
        calculate_visibility(player.x, player.y, player.angle);
    }
}

void calculate_visibility(float player_x, float player_y, float player_angle) {
    visibility_point_count = 0;

    float half_fov = FOV_ANGLE * 0.5f;
    float start_angle = player_angle - half_fov;

    // Normalize start to [0, 360)
    while (start_angle < 0.0f) start_angle += 360.0f;
    while (start_angle >= 360.0f) start_angle -= 360.0f;

    // Rays density (old code used FOV_ANGLE * 5).
    int rays_in_fov = (int)(FOV_ANGLE * 5.0f);
    if (rays_in_fov < 1) rays_in_fov = 1;

    const float deg2rad = (float)M_PI / 180.0f;

    for (int i = 0; i <= rays_in_fov; i++) {
        float angle = start_angle + (FOV_ANGLE * (float)i / (float)rays_in_fov);

        // Normalize to [0, 360)
        while (angle < 0.0f) angle += 360.0f;
        while (angle >= 360.0f) angle -= 360.0f;

        float rad = angle * deg2rad;

        float dir_x = cosf(rad);
        float dir_y = sinf(rad);

        float hit_x, hit_y, dist;
        bool hit = raycast_dda(player_x, player_y, dir_x, dir_y, g_max_ray_distance, &hit_x, &hit_y, &dist);

        if (visibility_point_count < MAX_VISIBILITY_RAYS) {
            visibility_polygon[visibility_point_count].x = hit_x;
            visibility_polygon[visibility_point_count].y = hit_y;
            visibility_polygon[visibility_point_count].distance = dist;
            visibility_polygon[visibility_point_count].blocked = hit ? 1 : 0;
            visibility_point_count++;
        }
    }
}

bool is_point_visible(float x, float y, float player_x, float player_y, float player_angle) {
    if (!darkness_active) return true;

    float dx = x - player_x;
    float dy = y - player_y;

    float point_angle = atan2f(dy, dx) * 180.0f / (float)M_PI;
    while (point_angle < 0.0f) point_angle += 360.0f;
    while (point_angle >= 360.0f) point_angle -= 360.0f;

    float half_fov = FOV_ANGLE * 0.5f;
    float angle_diff = fabsf(point_angle - player_angle);
    if (angle_diff > 180.0f) angle_diff = 360.0f - angle_diff;

    if (angle_diff > half_fov) return false;

    // Ray cast to the point.
    float dist2 = dx * dx + dy * dy;
    if (dist2 <= 0.000001f) return true;

    float point_dist = sqrtf(dist2);
    float inv = 1.0f / point_dist;

    float dir_x = dx * inv;
    float dir_y = dy * inv;

    float hit_x, hit_y, hit_dist;
    bool blocked = raycast_dda(player_x, player_y, dir_x, dir_y, point_dist, &hit_x, &hit_y, &hit_dist);

    // Compare squared distances to avoid another sqrt.
    return (!blocked) || (hit_dist * hit_dist >= dist2);
}

void render_darkness_overlay(void) {
    if (!darkness_active) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_STENCIL_TEST);

    glStencilMask(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    // Visibility fan (FOV polygon)
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(player.x, player.y);
    for (int i = 0; i < visibility_point_count; i++) {
        glVertex2f(visibility_polygon[i].x, visibility_polygon[i].y);
    }
    if (visibility_point_count > 0) {
        glVertex2f(visibility_polygon[0].x, visibility_polygon[0].y);
    }
    glEnd();

    // Small circular light around player (cached unit circle -> no trig here)
    const float player_light_radius = 60.0f;
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(player.x, player.y);
    for (int i = 0; i < PLAYER_LIGHT_POINTS; i++) {
        glVertex2f(player.x + g_unit_circle_x[i] * player_light_radius,
            player.y + g_unit_circle_y[i] * player_light_radius);
    }
    glEnd();

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glColor4f(0.0f, 0.0f, 0.0f, 0.65f);
    glBegin(GL_QUADS);
    glVertex2f(0, 40);
    glVertex2f(WIDTH, 40);
    glVertex2f(WIDTH, HEIGHT);
    glVertex2f(0, HEIGHT);
    glEnd();

    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
}

void render_warning_pulse(void) {
    if (!warning_active) return;

    glColor4f(1.0f, 0.0f, 0.0f, pulse_intensity * 0.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(WIDTH, 0);
    glVertex2f(WIDTH, HEIGHT);
    glVertex2f(0, HEIGHT);
    glEnd();

    glDisable(GL_BLEND);
}
