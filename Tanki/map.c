#include "main.h"


int map[MAP_HEIGHT][MAP_WIDTH] = { 0 };


int texture_map[MAP_HEIGHT][MAP_WIDTH] = { 0 };


void generate_map(char* filename)
{
    memset(map, TILE_EMPTY, sizeof(map));
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (x == 0 || y == 0 || x == MAP_WIDTH - 1 || y == MAP_HEIGHT - 1) {
                map[y][x] = TILE_WALL;
            }
        }
    }
    FILE* mapfile = fopen(filename, "r");
    for (int y = 1; y < MAP_HEIGHT - 1; y++) {
        for (int x = 1; x < MAP_WIDTH - 1; x++) {
            fscanf(mapfile, "%d", &map[y][x]);
            texture_map[y][x] = rand() % 4;
        }
    }
    fclose(mapfile);
}


bool check_map_collision(float x, float y, float radius) {
    int tile_size = TILE_SIZE;

    
    int start_x = (int)((x - radius) / tile_size);
    int end_x = (int)((x + radius) / tile_size);
    int start_y = (int)((y - radius) / tile_size);
    int end_y = (int)((y + radius) / tile_size);

    for (int ty = start_y; ty <= end_y; ty++) {
        for (int tx = start_x; tx <= end_x; tx++) {
            if (tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT) {
                if (map[ty][tx] == TILE_WALL || map[ty][tx] == TILE_BREAKABLE) {
                    float tile_center_x = tx * tile_size + tile_size / 2;
                    float tile_center_y = ty * tile_size + tile_size / 2;

                    float closest_x = fmaxf(tile_center_x - tile_size / 2, fminf(x, tile_center_x + tile_size / 2));
                    float closest_y = fmaxf(tile_center_y - tile_size / 2, fminf(y, tile_center_y + tile_size / 2));

                    float distance_x = x - closest_x;
                    float distance_y = y - closest_y;
                    float distance_squared = distance_x * distance_x + distance_y * distance_y;

                    if (distance_squared < radius * radius) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

void find_spawn_point(float* x_pos, float* y_pos, int tankType) {
    for (int y = 1;y < MAP_HEIGHT - 1;y++)
    {
        for (int x = 1;x < MAP_WIDTH - 1;x++)
        {
            if (map[y][x] == 10 && tankType == 0)
            {
                *x_pos = x * TILE_SIZE;
                *y_pos = y * TILE_SIZE;
                map[y][x] = TILE_EMPTY;
                return;
            }
            if (map[y][x] == 11 && tankType == 1)
            {
                *x_pos = x * TILE_SIZE;
                *y_pos = y * TILE_SIZE;
                map[y][x] = TILE_EMPTY;
                return;
            }
        }
    }
}

void parse_node(char* line, BotGraph* node, int i)
{
    node->id = i;
    sscanf(line, " { %d , %d } ", &node->x, &node->y);

    char* p = strchr(line, '}');
    if (!p) {
        node->next_index = 0;
        node->nextinds = NULL;
        node->next = NULL;
        return;
    }
    p++;
    node->nextinds = malloc(sizeof(int) * 8);
    node->next_index = 0;

    while (*p)
    {
        while (*p && isspace(*p))
            p++;

        if (isdigit(*p))
        {
            int value = strtol(p, &p, 10);
            node->nextinds[node->next_index++] = value;
        }
        else
        {
            p++;
        }
    }
}
void link_nodes(BotGraph* nodes, int count)
{
    for (int i = 0; i < count; i++)
    {
        BotGraph* cur = &nodes[i];

        if (cur->next == NULL || cur->nextinds == NULL) {
            continue;  // Пропускаем узлы без связей
        }

        for (int j = 0; j < cur->next_index; j++)
        {
            int index = cur->nextinds[j];
            if (index >= 0 && index < count) {  // Проверка границ
                cur->next[j] = &nodes[index];  // Используем адрес, а не значение
            }
        }
    }
}


int load_graph(const char* filename, BotGraph** out_nodes)
{
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("Failed to open file");
        return -1;
    }


    int count = 0;
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), f))
        count++;

    rewind(f);

    BotGraph* nodes = calloc(count, sizeof(BotGraph));
    int i = 0;
    while (fgets(buffer, sizeof(buffer), f))
    {
        buffer[strcspn(buffer, "\n")] = 0;
        parse_node(buffer, &nodes[i], i+1);
        i++;
    }

    fclose(f);

    link_nodes(nodes, count);

    *out_nodes = nodes;
    return count;
}
void free_graph(BotGraph* nodes, int count) {
    if (nodes == NULL) return;

    for (int i = 0; i < count; i++) {
        if (nodes[i].nextinds != NULL) {
            free(nodes[i].nextinds);
        }
        if (nodes[i].next != NULL) {
            free(nodes[i].next);
        }
    }
    free(nodes);
}


