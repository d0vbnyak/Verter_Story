#include <stdio.h>

#define HEIGHT 25
#define WIDTH 80
#define DEAD 0
#define ALIVE 1

void kill_all(int field[HEIGHT][WIDTH]);
void print_field(const int field[HEIGHT][WIDTH]);
void get_pattern(int field[HEIGHT][WIDTH]);
int why_is_my_neighbor(const int field[HEIGHT][WIDTH], int row, int col);

int main(void) {
    int field[HEIGHT][WIDTH];
    get_pattern(field);
    print_field(field);
    printf("%d", why_is_my_neighbor(field, 0, 0));

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
