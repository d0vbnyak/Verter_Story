#include <stdio.h>

#define HEIGHT 25
#define WIDTH 80
#define DEAD 0
#define ALIVE 1

void kill_all(int field [HEIGHT][WIDTH]);
void place_glider(int field [HEIGHT][WIDTH]);
void print_field(const int field [HEIGHT][WIDTH]);

int main()
{
    int field[HEIGHT][WIDTH];
    kill_all(field);
    place_glider(field);
    print_field(field);
    return 0;
}

void kill_all(int field [HEIGHT][WIDTH])
{
    for(int row = 0; row < HEIGHT; row++)
        for(int col = 0; col < WIDTH; col++)
            field[row][col] = DEAD;
}

void place_glider(int field [HEIGHT][WIDTH])
{
    field[2][9] = ALIVE;
    field[3][10] = ALIVE;
    field[4][8] = ALIVE;
    field[4][9] = ALIVE;
    field[4][10] = ALIVE;
}

void print_field(const int field [HEIGHT][WIDTH])
{
    for(int row = 0; row < HEIGHT; row++)
    {
        for(int col = 0; col < WIDTH; col++)
        {
            printf("%c", field[row][col] == ALIVE ? '@' : '.');
        }
        printf("\n");
    }
}