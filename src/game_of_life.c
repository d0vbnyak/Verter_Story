#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>

#define HEIGHT 25
#define WIDTH 80
#define DEAD 0
#define ALIVE 1

#define TERMINAL_PATH "/dev/tty"
#define KEY_QUIT ' '
#define SPEED_MIN 1
#define SPEED_MAX 10
#define SPEED_START 5
#define DELAY_FASTEST 30
#define DELAY_STEP 12

void kill_all(int field[HEIGHT][WIDTH]);
void print_field(const int field[HEIGHT][WIDTH]);
void get_pattern(int field[HEIGHT][WIDTH]);
int why_is_my_neighbor(const int field[HEIGHT][WIDTH], int row, int col);
int cell_fate(int state, int neighbors_count);
void next_generation(const int field[HEIGHT][WIDTH], int next[HEIGHT][WIDTH]);
void copy_gen(const int from[HEIGHT][WIDTH], int to[HEIGHT][WIDTH]);

int prepare_input(int field[HEIGHT][WIDTH]);
int input_is_file(void);
int reopen_keyboard(void);
void init_screen(void);
void run_sandbox(int past_gen_cell[HEIGHT][WIDTH]);
void print_status(int generation, int speed);
int change_speed(int speed, int key);
int speed_delay(int speed);

// Всё, что может сломаться (нет файла, нет терминала), проверяется до initscr():
// пока ncurses не заняла экран, ошибку можно напечатать обычным текстом в stderr.
int main(void) {
    int past_gen_cell[HEIGHT][WIDTH];
    int status = prepare_input(past_gen_cell);
    if (status == 0) {
        setlocale(LC_ALL, "");
        init_screen();
        run_sandbox(past_gen_cell);
        endwin();
    }
    return status;
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

// Теперь рисует через ncurses: mvaddch(строка, столбец, символ) ставит символ в точку экрана.
// На экран он попадёт только после refresh() — его вызывает run_sandbox, когда кадр готов.
void print_field(const int field[HEIGHT][WIDTH]) {
    for (int row = 0; row < HEIGHT; row++)
        for (int col = 0; col < WIDTH; col++) mvaddch(row, col, field[row][col] == ALIVE ? '@' : '.');
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

int cell_fate(int state, int neighbors_count) {
    int fate = DEAD;
    if (state == ALIVE && (neighbors_count == 2 || neighbors_count == 3)) fate = ALIVE;
    if (state == DEAD && neighbors_count == 3) fate = ALIVE;
    return fate;
}

void next_generation(const int field[HEIGHT][WIDTH], int next[HEIGHT][WIDTH]) {
    for (int row = 0; row < HEIGHT; row++)
        for (int col = 0; col < WIDTH; col++)
            next[row][col] = cell_fate(field[row][col], why_is_my_neighbor(field, row, col));
}

void copy_gen(const int from[HEIGHT][WIDTH], int to[HEIGHT][WIDTH]) {
    for (int row = 0; row < HEIGHT; row++)
        for (int col = 0; col < WIDTH; col++) to[row][col] = from[row][col];
}

// Шаг 5. Поле читается из файла, потом stdin переключается на клавиатуру.
// Возвращает 0, если можно начинать игру; 1 — ошибка, она уже напечатана в stderr.
int prepare_input(int field[HEIGHT][WIDTH]) {
    int status = 1;
    if (!input_is_file()) {
        fprintf(stderr, "Запуск: ./game_of_life < src/patterns/glider.txt\n");
    } else {
        get_pattern(field);
        int keyboard = reopen_keyboard();
        if (keyboard)
            status = 0;
        else
            fprintf(stderr, "Нет терминала для клавиш (%s). Запустите игру из терминала.\n", TERMINAL_PATH);
    }
    return status;
}

// isatty() отвечает, подключён ли stdin к терминалу. Если нет — поле подали файлом через <.
int input_is_file(void) { return !isatty(STDIN_FILENO); }

// После чтения stdin всё ещё смотрит в файл, и клавиш оттуда не дождаться.
// /dev/tty — терминал, из которого запустили программу, куда бы ни перенаправили stdin.
// freopen подменяет stdin на него; NULL — терминала нет (например, запуск из скрипта).
int reopen_keyboard(void) { return freopen(TERMINAL_PATH, "r", stdin) != NULL; }

// Шаг 6. cbreak — клавиша приходит сразу, без Enter; noecho — нажатая буква не печатается;
// curs_set(0) прячет мигающий курсор.
void init_screen(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
}

// Шаг 7. Игровой цикл: нарисовать -> подождать -> шаг.
// timeout(delay): getch ждёт клавишу не дольше delay мс. Не нажали — getch вернёт ERR,
// значит пора следующему поколению. Нажали A/Z — меняется скорость, Space — выход.
void run_sandbox(int past_gen_cell[HEIGHT][WIDTH]) {
    int next_gen_cell[HEIGHT][WIDTH];
    int generation = 1;
    int speed = SPEED_START;
    int key = ERR;
    while (key != KEY_QUIT) {
        print_field(past_gen_cell);
        print_status(generation, speed);
        refresh();
        timeout(speed_delay(speed));
        key = getch();
        if (key == ERR) {
            next_generation(past_gen_cell, next_gen_cell);
            copy_gen(next_gen_cell, past_gen_cell);
            generation++;
        }
        speed = change_speed(speed, key);
    }
}

// Строка под полем. clrtoeol() стирает хвост строки: число стало короче — старые цифры не останутся.
void print_status(int generation, int speed) {
    mvprintw(HEIGHT, 0, "Поколение: %d  Скорость: %d/%d  A/Z - быстрее/медленнее  Space - выход", generation,
             speed, SPEED_MAX);
    clrtoeol();
}

// Строчные буквы тоже подходят, чтобы не мешал Caps Lock. На краях уровень просто не меняется.
int change_speed(int speed, int key) {
    int result = speed;
    if ((key == 'a' || key == 'A') && speed < SPEED_MAX)
        result = speed + 1;
    else if ((key == 'z' || key == 'Z') && speed > SPEED_MIN)
        result = speed - 1;
    return result;
}

// Пауза между поколениями в мс: 1002 мс на уровне 1, 30 мс на уровне 10.
// Растёт как квадрат числа оставшихся уровней, поэтому каждое нажатие заметно на глаз.
int speed_delay(int speed) {
    int steps = SPEED_MAX - speed;
    return DELAY_FASTEST + DELAY_STEP * steps * steps;
}
