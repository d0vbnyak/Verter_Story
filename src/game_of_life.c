#define _POSIX_C_SOURCE 200809L

#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define HEIGHT 25
#define WIDTH 80
#define DEAD 0
#define ALIVE 1

#define TERMINAL_PATH "/dev/tty"
#define KEY_QUIT ' '
#define KEY_ESCAPE 27
#define ESCAPE_DELAY 25
#define SPEED_MIN 1
#define SPEED_MAX 10
#define SPEED_START 5
#define DELAY_FASTEST 30
#define DELAY_STEP 12

#define MENU_STORY 0
#define MENU_SANDBOX 1
#define MENU_CONTROLS 2
#define MENU_CREDITS 3
#define MENU_EXIT 4
#define MENU_SIZE 5

#define DOODLE_GOING 0
#define DOODLE_START 1
#define DOODLE_CANCEL 2
#define DOODLE_EMPTY 3
#define KEY_FLIP ' '
#define CURSOR_ON_EMPTY '+'

void kill_all(int field[HEIGHT][WIDTH]);
void print_field(const int field[HEIGHT][WIDTH]);
void get_pattern(int field[HEIGHT][WIDTH]);
int why_is_my_neighbor(const int field[HEIGHT][WIDTH], int row, int col);
int cell_fate(int state, int neighbors_count);
void next_generation(const int field[HEIGHT][WIDTH], int next[HEIGHT][WIDTH]);
void copy_gen(const int from[HEIGHT][WIDTH], int to[HEIGHT][WIDTH]);
int head_count(const int field[HEIGHT][WIDTH]);

int prepare_input(int field[HEIGHT][WIDTH]);
int input_is_file(void);
int reopen_keyboard(void);
void init_screen(void);
void run_sandbox(int past_gen_cell[HEIGHT][WIDTH]);
void print_status(int generation, int alive, int speed);
int change_speed(int speed, int key);
int speed_delay(int speed);
long long what_time_is_it(void);
int how_long_to_wait(long long deadline);

void hang_out_in_menu(void);
int pick_your_poison(int selected);
int scroll_the_menu(int selected, int key);
int menu_item_row(int item);
void paint_menu(int selected);
void coming_soon(const char* what);

void sandbox_from_scratch(void);
int doodle_colony(int field[HEIGHT][WIDTH]);
int doodle_step(int field[HEIGHT][WIDTH], int* row, int* col, int key);
int nudge(int position, int size, int key, int back, int forward);
void flip_cell(int field[HEIGHT][WIDTH], int row, int col);
void draw_doodle(const int field[HEIGHT][WIDTH], int row, int col, int state);

void draw_fence(int top, int left, int height, int width);
int is_window_big_enough(void);
void say_in_middle(int row, const char* text);
int how_wide(const char* text);
int is_letter_start(char byte);

// Этап Б. С файлом (./game_of_life < файл) — сразу Песочница, как требует ТЗ. Без файла — меню.
int main(void) {
    int past_gen_cell[HEIGHT][WIDTH];
    int from_file = input_is_file();
    int status = 0;
    if (from_file) status = prepare_input(past_gen_cell);
    if (status == 0) {
        setlocale(LC_ALL, "");
        init_screen();
        if (from_file)
            run_sandbox(past_gen_cell);
        else
            hang_out_in_menu();
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

// Поле теперь стоит в рамке, поэтому клетка (row, col) рисуется в (row + 1, col + 1).
// Живая клетка — пробел с A_REVERSE: цвета меняются местами, и получается сплошной квадрат.
void print_field(const int field[HEIGHT][WIDTH]) {
    for (int row = 0; row < HEIGHT; row++)
        for (int col = 0; col < WIDTH; col++)
            mvaddch(row + 1, col + 1, field[row][col] == ALIVE ? ' ' | A_REVERSE : ' ');
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

// Сколько живых клеток на поле: редактор не пускает в жизнь пустое поле, строка состояния их показывает.
int head_count(const int field[HEIGHT][WIDTH]) {
    int alive = 0;
    for (int row = 0; row < HEIGHT; row++)
        for (int col = 0; col < WIDTH; col++)
            if (field[row][col] == ALIVE) alive++;
    return alive;
}

// Теперь вызывается только в режиме файла: проверку «файл или терминал» делает main.
int prepare_input(int field[HEIGHT][WIDTH]) {
    int status = 1;
    get_pattern(field);
    int keyboard = reopen_keyboard();
    if (keyboard)
        status = 0;
    else
        fprintf(stderr, "Нет терминала для клавиш (%s). Запустите игру из терминала.\n", TERMINAL_PATH);
    return status;
}

int input_is_file(void) { return !isatty(STDIN_FILENO); }

int reopen_keyboard(void) { return freopen(TERMINAL_PATH, "r", stdin) != NULL; }

// После Esc ncurses по умолчанию секунду ждёт, не начало ли это кода стрелки.
// set_escdelay сокращает ожидание до 25 мс, и Esc в редакторе срабатывает сразу.
void init_screen(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    set_escdelay(ESCAPE_DELAY);
}

// Поколение меняется по часам, а не по «никто ничего не нажал». Раньше зажатая A останавливала
// жизнь: каждое нажатие сбрасывало ожидание. Теперь next_tick — момент следующего поколения,
// getch ждёт ровно до него, а клавиши, нажатые в это время, ход часов не сбивают.
// flushinp() выбрасывает клавиши, нажатые до начала игры (например, лишний Space в редакторе).
void run_sandbox(int past_gen_cell[HEIGHT][WIDTH]) {
    int next_gen_cell[HEIGHT][WIDTH];
    int generation = 1;
    int speed = SPEED_START;
    int key = ERR;
    long long next_tick = what_time_is_it() + speed_delay(speed);
    flushinp();
    while (key != KEY_QUIT) {
        erase();
        draw_fence(0, 0, HEIGHT + 2, WIDTH + 2);
        mvaddstr(0, 2, " Песочница ");
        print_field(past_gen_cell);
        print_status(generation, head_count(past_gen_cell), speed);
        refresh();
        timeout(how_long_to_wait(next_tick));
        key = getch();
        if (what_time_is_it() >= next_tick) {
            next_generation(past_gen_cell, next_gen_cell);
            copy_gen(next_gen_cell, past_gen_cell);
            generation++;
            next_tick = what_time_is_it() + speed_delay(speed);
        }
        speed = change_speed(speed, key);
    }
    timeout(-1);
}

void print_status(int generation, int alive, int speed) {
    mvprintw(HEIGHT + 2, 1, "Поколение: %d  Живых: %d  Скорость: %d/%d (A/Z)  Space - выход", generation,
             alive, speed, SPEED_MAX);
}

int change_speed(int speed, int key) {
    int result = speed;
    if ((key == 'a' || key == 'A') && speed < SPEED_MAX)
        result = speed + 1;
    else if ((key == 'z' || key == 'Z') && speed > SPEED_MIN)
        result = speed - 1;
    return result;
}

int speed_delay(int speed) {
    int steps = SPEED_MAX - speed;
    return DELAY_FASTEST + DELAY_STEP * steps * steps;
}

// Миллисекунды по монотонным часам: они идут только вперёд, даже если кто-то переведёт системное время.
long long what_time_is_it(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (long long)now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

// Сколько мс осталось до deadline. Если он уже прошёл — 0: getch не ждёт, а сразу возвращает ERR.
int how_long_to_wait(long long deadline) {
    long long left = deadline - what_time_is_it();
    return left > 0 ? (int)left : 0;
}

// Меню крутится, пока не выбран «Выход». selected запоминает пункт: вернулись — курсор на нём же.
void hang_out_in_menu(void) {
    int choice = MENU_STORY;
    while (choice != MENU_EXIT) {
        choice = pick_your_poison(choice);
        if (choice == MENU_STORY)
            coming_soon("Сюжет");
        else if (choice == MENU_SANDBOX)
            sandbox_from_scratch();
        else if (choice == MENU_CONTROLS)
            coming_soon("Управление");
        else if (choice == MENU_CREDITS)
            coming_soon("Титры");
    }
}

// Стрелки двигают выделение, Enter подтверждает. Возвращает номер выбранного пункта.
int pick_your_poison(int selected) {
    int key = 0;
    while (key != '\n' && key != KEY_ENTER) {
        paint_menu(selected);
        key = getch();
        selected = scroll_the_menu(selected, key);
    }
    return selected;
}

// По кругу: вверх с первого пункта — на последний, вниз с последнего — на первый.
int scroll_the_menu(int selected, int key) {
    int result = selected;
    if (key == KEY_UP)
        result = (selected + MENU_SIZE - 1) % MENU_SIZE;
    else if (key == KEY_DOWN)
        result = (selected + 1) % MENU_SIZE;
    return result;
}

// Пункты идут через строку чуть выше середины окна.
int menu_item_row(int item) { return LINES / 2 - MENU_SIZE + item * 2; }

void paint_menu(int selected) {
    const char* const items[MENU_SIZE] = {"Сюжет", "Песочница", "Управление", "Титры", "Выход"};
    int top = menu_item_row(0);
    erase();
    attron(A_BOLD);
    say_in_middle(top - 3, "=== V E R T E R   S T O R Y ===");
    attroff(A_BOLD);
    for (int i = 0; i < MENU_SIZE; i++) {
        if (i == selected) attron(A_REVERSE);
        say_in_middle(menu_item_row(i), items[i]);
        if (i == selected) attroff(A_REVERSE);
    }
    say_in_middle(top + MENU_SIZE * 2 + 1, "стрелки - выбор, Enter - подтвердить");
    refresh();
}

// Временный экран для пунктов, которые появятся на следующих этапах.
void coming_soon(const char* what) {
    erase();
    say_in_middle(LINES / 2 - 1, what);
    say_in_middle(LINES / 2 + 1, "скоро будет. Любая клавиша - назад в меню.");
    refresh();
    getch();
}

// Песочница из меню: пустое поле, игрок рисует колонию и запускает её. Esc в редакторе — назад в меню.
void sandbox_from_scratch(void) {
    int field[HEIGHT][WIDTH];
    kill_all(field);
    if (doodle_colony(field)) run_sandbox(field);
}

// Редактор. Каждая клавиша — один шаг doodle_step, и он же говорит, что дальше:
// рисуем (DOODLE_GOING, DOODLE_EMPTY), запуск (DOODLE_START) или отмена (DOODLE_CANCEL).
// Курсор (row, col) живёт здесь, а двигает его doodle_step через указатели.
int doodle_colony(int field[HEIGHT][WIDTH]) {
    int state = DOODLE_GOING;
    int row = HEIGHT / 2;
    int col = WIDTH / 2;
    while (state != DOODLE_START && state != DOODLE_CANCEL) {
        draw_doodle(field, row, col, state);
        state = doodle_step(field, &row, &col, getch());
    }
    return state == DOODLE_START;
}

// Стрелки двигают курсор, Space переключает клетку, Enter запускает жизнь, Esc отменяет.
// Enter на пустом поле не запускает, а возвращает DOODLE_EMPTY: под полем появится подсказка.
int doodle_step(int field[HEIGHT][WIDTH], int* row, int* col, int key) {
    int state = DOODLE_GOING;
    *row = nudge(*row, HEIGHT, key, KEY_UP, KEY_DOWN);
    *col = nudge(*col, WIDTH, key, KEY_LEFT, KEY_RIGHT);
    if (key == KEY_FLIP)
        flip_cell(field, *row, *col);
    else if (key == '\n' || key == KEY_ENTER)
        state = head_count(field) > 0 ? DOODLE_START : DOODLE_EMPTY;
    else if (key == KEY_ESCAPE)
        state = DOODLE_CANCEL;
    return state;
}

// Шаг курсора по одной оси. Поле — тор, поэтому с края курсор выходит с другой стороны.
int nudge(int position, int size, int key, int back, int forward) {
    int result = position;
    if (key == back)
        result = (position + size - 1) % size;
    else if (key == forward)
        result = (position + 1) % size;
    return result;
}

void flip_cell(int field[HEIGHT][WIDTH], int row, int col) {
    field[row][col] = field[row][col] == ALIVE ? DEAD : ALIVE;
}

// Курсор на пустой клетке — '+', на живой — та же клетка без инверсии: видно, куда попадёшь.
void draw_doodle(const int field[HEIGHT][WIDTH], int row, int col, int state) {
    erase();
    if (!is_window_big_enough()) {
        say_in_middle(LINES / 2, "Окно мало: нужно хотя бы 82 x 28.");
    } else {
        draw_fence(0, 0, HEIGHT + 2, WIDTH + 2);
        mvaddstr(0, 2, " Песочница: нарисуйте колонию ");
        print_field(field);
        mvaddch(row + 1, col + 1, field[row][col] == ALIVE ? ' ' : CURSOR_ON_EMPTY | A_BOLD);
        if (state == DOODLE_EMPTY)
            mvaddstr(HEIGHT + 2, 1, "Поле пустое: оживите хоть одну клетку (Space), потом Enter.");
        else
            mvprintw(HEIGHT + 2, 1, "Живых: %d  стрелки - курсор  Space - клетка  Enter - жизнь  Esc - меню",
                     head_count(field));
    }
    refresh();
}

// Рамка из псевдографики ncurses. Если терминал её не умеет, ncurses подставит + - |.
void draw_fence(int top, int left, int height, int width) {
    mvhline(top, left + 1, ACS_HLINE, width - 2);
    mvhline(top + height - 1, left + 1, ACS_HLINE, width - 2);
    mvvline(top + 1, left, ACS_VLINE, height - 2);
    mvvline(top + 1, left + width - 1, ACS_VLINE, height - 2);
    mvaddch(top, left, ACS_ULCORNER);
    mvaddch(top, left + width - 1, ACS_URCORNER);
    mvaddch(top + height - 1, left, ACS_LLCORNER);
    mvaddch(top + height - 1, left + width - 1, ACS_LRCORNER);
}

// Поле 80 x 25, рамка вокруг и строка состояния под ней: нужно окно 82 x 28.
int is_window_big_enough(void) { return LINES >= HEIGHT + 3 && COLS >= WIDTH + 2; }

void say_in_middle(int row, const char* text) { mvaddstr(row, (COLS - how_wide(text)) / 2, text); }

// Ширина текста на экране. Русская буква в UTF-8 занимает 2 байта, но на экране — 1 столбец,
// поэтому считаем не байты, а начала букв.
int how_wide(const char* text) {
    int width = 0;
    for (int i = 0; text[i] != '\0'; i++) width += is_letter_start(text[i]);
    return width;
}

// Второй байт русской буквы в UTF-8 имеет вид 10xxxxxx: с него буква не начинается.
int is_letter_start(char byte) { return ((unsigned char)byte & 0xC0) != 0x80; }
