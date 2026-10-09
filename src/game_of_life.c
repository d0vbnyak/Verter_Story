#include <stdio.h>

#define HEIGHT 25
#define WIDTH 80
#define DEAD 0
#define ALIVE 1

void kill_all(int field[HEIGHT][WIDTH]);
void print_field(const int field[HEIGHT][WIDTH]);
void get_pattern(int field[HEIGHT][WIDTH]);
int why_is_my_neighbor(const int field[HEIGHT][WIDTH], int row, int col);
int cell_fate(int state, int neighbors);
void next_generation(const int field[HEIGHT][WIDTH], int next[HEIGHT][WIDTH]);
void copy_field(const int from[HEIGHT][WIDTH], int to[HEIGHT][WIDTH]);

int main(void) {
    int past_gen_cell[HEIGHT][WIDTH];
    int next_gen_cell[HEIGHT][WIDTH];
    get_pattern(past_gen_cell);
    print_field(past_gen_cell);

    // Временная проверка шага 4: одно поколение. Уйдёт, когда появится игровой цикл.
    next_generation(past_gen_cell, next_gen_cell);
    copy_field(next_gen_cell, past_gen_cell);
    printf("\n");
    print_field(past_gen_cell);

    return 0;
}

void kill_all(int field[HEIGHT][WIDTH]) {
    for (int row = 0; row < HEIGHT; row++)
        for (int col = 0; col < WIDTH; col++) field[row][col] = DEAD;
}

void get_pattern(int field[HEIGHT][WIDTH]) {
    kill_all(field);

    int row = 0, col = 0;

    int lastchar = getchar();
    while (lastchar != EOF && row < HEIGHT) {
        if (lastchar == '\n') {
            row++;
            col = 0;
        } else if (col < WIDTH) {
            field[row][col] = lastchar == '*' ? ALIVE : DEAD;
            col++;
        }
        lastchar = getchar();
    }
}

void print_field(const int field[HEIGHT][WIDTH]) {
    for (int row = 0; row < HEIGHT; row++) {
        for (int col = 0; col < WIDTH; col++) {
            printf("%c", field[row][col] == ALIVE ? '@' : '.');
        }
        printf("\n");
    }
}

int why_is_my_neighbor(const int field[HEIGHT][WIDTH], int row, int col) {
    int neighbor_count = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if ((dr != 0 || dc != 0) &&
                field[(row + dr + HEIGHT) % HEIGHT][(col + dc + WIDTH) % WIDTH] == ALIVE)
                neighbor_count++;
        }
    }
    return neighbor_count;
}

// Правила Конвея для одной клетки: живая выживает при 2 или 3 соседях,
// мёртвая оживает ровно при 3. Во всех остальных случаях клетка мертва.
int cell_fate(int state, int neighbors_count) {
    int fate = DEAD;
    if (state == ALIVE && (neighbors_count == 2 || neighbors_count == 3)) fate = ALIVE;
    if (state == DEAD && neighbors_count == 3) fate = ALIVE;
    return fate;
}

// Новое поколение считается во второй массив: field только читается,
// поэтому все клетки меняются одновременно, глядя на одно и то же прошлое.
void next_generation(const int field[HEIGHT][WIDTH], int next[HEIGHT][WIDTH]) {
    for (int row = 0; row < HEIGHT; row++)
        for (int col = 0; col < WIDTH; col++)
            next[row][col] = cell_fate(field[row][col], why_is_my_neighbor(field, row, col));
}

void copy_field(const int from[HEIGHT][WIDTH], int to[HEIGHT][WIDTH]) {
    for (int row = 0; row < HEIGHT; row++)
        for (int col = 0; col < WIDTH; col++) to[row][col] = from[row][col];
}
