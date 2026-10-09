#define _POSIX_C_SOURCE 200809L

#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define HEIGHT 25
#define WIDTH 80
#define DEAD 0
#define ALIVE 1
#define VIRUS ALIVE
#define ANTIVIRUS 2

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

#define MODULES 4
#define HALF_HEIGHT (HEIGHT / 2)
#define HALF_WIDTH (WIDTH / 2)
#define POWER_START 21
#define LEAK_PERIOD 42
#define PATCHES_PER_PERCENT 5
#define POWER_ALARM 5

#define STORY_GOING 0
#define STORY_GAVE_UP 1
#define STORY_WON 2
#define STORY_LOST 3

#define SHAPES 7
#define SHAPE_CELLS 9
#define NO_SHAPE -1
#define GLIDER 0
#define BLINKER 1
#define BLOCK 2
#define TOAD 3
#define R_PENTOMINO 4
#define LWSS 5
#define BEEHIVE 6

#define PAIR_VIRUS 1
#define PAIR_ANTIVIRUS 2
#define PAIR_HEALED 3
#define PAIR_ALARM 4
#define COLOR_TERMINAL -1

typedef struct {
    int generation;
    int speed;
    int patches;
    int syringe_row;
    int syringe_col;
    int bad_guys[MODULES];
} Patient;

void kill_all(int field[HEIGHT][WIDTH]);
void print_field(const int field[HEIGHT][WIDTH]);
void get_pattern(int field[HEIGHT][WIDTH]);
int why_is_my_neighbor(const int field[HEIGHT][WIDTH], int row, int col);
int how_many_good_guys(const int field[HEIGHT][WIDTH], int row, int col);
int cell_fate(int state, int neighbors_count, int good_guys);
void next_generation(const int field[HEIGHT][WIDTH], int next[HEIGHT][WIDTH]);
void copy_gen(const int from[HEIGHT][WIDTH], int to[HEIGHT][WIDTH]);
int head_count(const int field[HEIGHT][WIDTH]);

int prepare_input(int field[HEIGHT][WIDTH]);
int input_is_file(void);
int reopen_keyboard(void);
void init_screen(void);
void init_palette(void);
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

void run_story(void);
void infect_verter(int field[HEIGHT][WIDTH]);
void infect_module(int field[HEIGHT][WIDTH], int module);
void drop_shape(int field[HEIGHT][WIDTH], int shape, int top, int left);
int which_module(int row, int col);
int treat_verter(int field[HEIGHT][WIDTH], Patient* verter);
int doctor_hands(int field[HEIGHT][WIDTH], Patient* verter, int key);
void inject_patch(int field[HEIGHT][WIDTH], Patient* verter);
int check_pulse(const int field[HEIGHT][WIDTH], Patient* verter);
void count_bad_guys(const int field[HEIGHT][WIDTH], Patient* verter);
int total_bad_guys(const Patient* verter);
int healed_modules(const Patient* verter);
int verter_charge(const Patient* verter);
void draw_ward(const int field[HEIGHT][WIDTH], const Patient* verter);
void draw_module_labels(const Patient* verter);
void draw_ward_status(const Patient* verter);
chtype paint_cell(int state, int in_story);
void show_verdict(int outcome);

void draw_fence(int top, int left, int height, int width);
int is_window_big_enough(void);
void say_in_middle(int row, const char* text);
int how_wide(const char* text);
int is_letter_start(char byte);

int main(void) {
    int past_gen_cell[HEIGHT][WIDTH];
    int from_file = input_is_file();
    int status = 0;
    if (from_file) status = prepare_input(past_gen_cell);
    if (status == 0) {
        setlocale(LC_ALL, "");
        srand((unsigned int)time(NULL));
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

void print_field(const int field[HEIGHT][WIDTH]) {
    for (int row = 0; row < HEIGHT; row++)
        for (int col = 0; col < WIDTH; col++) mvaddch(row + 1, col + 1, paint_cell(field[row][col], 0));
}

int why_is_my_neighbor(const int field[HEIGHT][WIDTH], int row, int col) {
    int neighbor_count = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if ((dr != 0 || dc != 0) &&
                field[(row + dr + HEIGHT) % HEIGHT][(col + dc + WIDTH) % WIDTH] != DEAD)
                neighbor_count++;
        }
    }
    return neighbor_count;
}

// Сколько соседей — антивирусы. why_is_my_neighbor теперь считает живых любого цвета (!= DEAD),
// а эта функция — только зелёных. В Песочнице зелёных нет, поэтому там всегда 0.
int how_many_good_guys(const int field[HEIGHT][WIDTH], int row, int col) {
    int good_guys = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if ((dr != 0 || dc != 0) &&
                field[(row + dr + HEIGHT) % HEIGHT][(col + dc + WIDTH) % WIDTH] == ANTIVIRUS)
                good_guys++;
        }
    }
    return good_guys;
}

// Правила Immigration — Конвей для двух цветов. Выживание как раньше: 2 или 3 соседа любого цвета,
// и клетка остаётся своего цвета. Рождение — ровно 3 соседа, а цвет новой клетки — как у большинства
// из этих трёх: 2 или 3 зелёных — антивирус, иначе вирус. В Песочнице зелёных нет: рождается ALIVE,
// а VIRUS == ALIVE, поэтому обычный Конвей работает без изменений.
int cell_fate(int state, int neighbors_count, int good_guys) {
    int fate = DEAD;
    if (state != DEAD && (neighbors_count == 2 || neighbors_count == 3)) fate = state;
    if (state == DEAD && neighbors_count == 3) fate = good_guys >= 2 ? ANTIVIRUS : VIRUS;
    return fate;
}

void next_generation(const int field[HEIGHT][WIDTH], int next[HEIGHT][WIDTH]) {
    for (int row = 0; row < HEIGHT; row++)
        for (int col = 0; col < WIDTH; col++)
            next[row][col] = cell_fate(field[row][col], why_is_my_neighbor(field, row, col),
                                       how_many_good_guys(field, row, col));
}

void copy_gen(const int from[HEIGHT][WIDTH], int to[HEIGHT][WIDTH]) {
    for (int row = 0; row < HEIGHT; row++)
        for (int col = 0; col < WIDTH; col++) to[row][col] = from[row][col];
}

int head_count(const int field[HEIGHT][WIDTH]) {
    int alive = 0;
    for (int row = 0; row < HEIGHT; row++)
        for (int col = 0; col < WIDTH; col++)
            if (field[row][col] == ALIVE) alive++;
    return alive;
}

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

void init_screen(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    set_escdelay(ESCAPE_DELAY);
    init_palette();
}

// Цвета включаются, только если терминал их умеет. use_default_colors разрешает цвет -1 —
// «как у терминала»: фон под клетками остаётся родным и в тёмной, и в светлой теме.
void init_palette(void) {
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(PAIR_VIRUS, COLOR_RED, COLOR_TERMINAL);
        init_pair(PAIR_ANTIVIRUS, COLOR_GREEN, COLOR_TERMINAL);
        init_pair(PAIR_HEALED, COLOR_GREEN, COLOR_TERMINAL);
        init_pair(PAIR_ALARM, COLOR_WHITE, COLOR_RED);
    }
}

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

long long what_time_is_it(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (long long)now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

int how_long_to_wait(long long deadline) {
    long long left = deadline - what_time_is_it();
    return left > 0 ? (int)left : 0;
}

void hang_out_in_menu(void) {
    int choice = MENU_STORY;
    while (choice != MENU_EXIT) {
        choice = pick_your_poison(choice);
        if (choice == MENU_STORY)
            run_story();
        else if (choice == MENU_SANDBOX)
            sandbox_from_scratch();
        else if (choice == MENU_CONTROLS)
            coming_soon("Управление");
        else if (choice == MENU_CREDITS)
            coming_soon("Титры");
    }
}

int pick_your_poison(int selected) {
    int key = 0;
    while (key != '\n' && key != KEY_ENTER) {
        paint_menu(selected);
        key = getch();
        selected = scroll_the_menu(selected, key);
    }
    return selected;
}

int scroll_the_menu(int selected, int key) {
    int result = selected;
    if (key == KEY_UP)
        result = (selected + MENU_SIZE - 1) % MENU_SIZE;
    else if (key == KEY_DOWN)
        result = (selected + 1) % MENU_SIZE;
    return result;
}

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

void coming_soon(const char* what) {
    erase();
    say_in_middle(LINES / 2 - 1, what);
    say_in_middle(LINES / 2 + 1, "скоро будет. Любая клавиша - назад в меню.");
    refresh();
    getch();
}

void sandbox_from_scratch(void) {
    int field[HEIGHT][WIDTH];
    kill_all(field);
    if (doodle_colony(field)) run_sandbox(field);
}

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

// Сюжет: заразить Вертера, лечить, пока все 4 модуля не чисты или питание не кончится.
// Состояние пациента (поколение, скорость, патчи, шприц, вирусы по модулям) — в одной структуре
// Patient: функциям передаётся указатель на неё, глобальных переменных нет.
void run_story(void) {
    int field[HEIGHT][WIDTH];
    Patient verter = {1, SPEED_START, 0, HEIGHT / 2, WIDTH / 2, {0, 0, 0, 0}};
    infect_verter(field);
    count_bad_guys(field, &verter);
    show_verdict(treat_verter(field, &verter));
}

void infect_verter(int field[HEIGHT][WIDTH]) {
    kill_all(field);
    for (int module = 0; module < MODULES; module++) infect_module(field, module);
}

// У каждого модуля своя зараза. Фигуры встают в случайное место внутри модуля, но так, чтобы
// целиком в нём помещаться: строки top+1..top+8, столбцы left+2..left+31, фигура не больше 4 x 5.
// Утечки памяти получают R-пентомино: оно растёт сотни поколений, как настоящая утечка.
void infect_module(int field[HEIGHT][WIDTH], int module) {
    const int plan[MODULES][3] = {{GLIDER, GLIDER, NO_SHAPE},
                                  {BLINKER, BLOCK, TOAD},
                                  {R_PENTOMINO, NO_SHAPE, NO_SHAPE},
                                  {LWSS, BEEHIVE, NO_SHAPE}};
    int top = (module / 2) * HALF_HEIGHT;
    int left = (module % 2) * HALF_WIDTH;
    for (int i = 0; i < 3; i++)
        if (plan[module][i] != NO_SHAPE)
            drop_shape(field, plan[module][i], top + 1 + rand() % 8, left + 2 + rand() % 30);
}

// Фигура — список клеток (строка, столбец) от её левого верхнего угла. sizes — сколько клеток
// у каждой фигуры: строки таблицы одинаковой длины, лишние места просто не читаются.
void drop_shape(int field[HEIGHT][WIDTH], int shape, int top, int left) {
    const int sizes[SHAPES] = {5, 3, 4, 6, 5, 9, 6};
    const int cells[SHAPES][SHAPE_CELLS][2] = {
        {{0, 1}, {1, 2}, {2, 0}, {2, 1}, {2, 2}},
        {{0, 0}, {0, 1}, {0, 2}},
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        {{0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 1}, {1, 2}},
        {{0, 1}, {0, 2}, {1, 0}, {1, 1}, {2, 1}},
        {{0, 1}, {0, 4}, {1, 0}, {2, 0}, {2, 4}, {3, 0}, {3, 1}, {3, 2}, {3, 3}},
        {{0, 1}, {0, 2}, {1, 0}, {1, 3}, {2, 1}, {2, 2}},
    };
    for (int i = 0; i < sizes[shape]; i++)
        field[(top + cells[shape][i][0]) % HEIGHT][(left + cells[shape][i][1]) % WIDTH] = VIRUS;
}

// Номер модуля по клетке: 0 — линтер (слева сверху), 1 — cppcheck (справа сверху),
// 2 — утечки (слева снизу), 3 — 7 принципов (справа снизу). Сравнение даёт 0 или 1.
int which_module(int row, int col) { return (row >= HALF_HEIGHT) * 2 + (col >= HALF_WIDTH); }

// Игровой цикл Сюжета — как в Песочнице: поколения по часам, клавиши в промежутках.
// Каждый круг: нарисовать, дождаться клавиши или часов, применить клавишу, сделать шаг,
// проверить пульс. Выход один — когда outcome перестал быть STORY_GOING.
int treat_verter(int field[HEIGHT][WIDTH], Patient* verter) {
    int next[HEIGHT][WIDTH];
    int outcome = STORY_GOING;
    long long next_tick = what_time_is_it() + speed_delay(verter->speed);
    flushinp();
    while (outcome == STORY_GOING) {
        draw_ward(field, verter);
        timeout(how_long_to_wait(next_tick));
        outcome = doctor_hands(field, verter, getch());
        if (outcome == STORY_GOING && what_time_is_it() >= next_tick) {
            next_generation(field, next);
            copy_gen(next, field);
            verter->generation++;
            next_tick = what_time_is_it() + speed_delay(verter->speed);
        }
        if (outcome == STORY_GOING) outcome = check_pulse(field, verter);
    }
    timeout(-1);
    return outcome;
}

// Руки доктора: A/Z — скорость, стрелки — шприц (по тору, как курсор в редакторе),
// Enter — патч, Space — сдаться. Возвращает STORY_GAVE_UP на Space, иначе STORY_GOING.
int doctor_hands(int field[HEIGHT][WIDTH], Patient* verter, int key) {
    verter->speed = change_speed(verter->speed, key);
    verter->syringe_row = nudge(verter->syringe_row, HEIGHT, key, KEY_UP, KEY_DOWN);
    verter->syringe_col = nudge(verter->syringe_col, WIDTH, key, KEY_LEFT, KEY_RIGHT);
    if (key == '\n' || key == KEY_ENTER) inject_patch(field, verter);
    return key == KEY_QUIT ? STORY_GAVE_UP : STORY_GOING;
}

// Патч превращает клетку под шприцем в антивирус — и пустую, и заражённую. Уже зелёную клетку
// второй раз не лечим: питание за это не списывается.
void inject_patch(int field[HEIGHT][WIDTH], Patient* verter) {
    int* cell = &field[verter->syringe_row][verter->syringe_col];
    if (*cell != ANTIVIRUS) {
        *cell = ANTIVIRUS;
        verter->patches++;
    }
}

// Пересчитать вирусы по модулям и решить: вирусов нет — победа, питания нет — поражение.
// Победа проверяется первой: если последний вирус убит последним процентом, Вертер спасён.
int check_pulse(const int field[HEIGHT][WIDTH], Patient* verter) {
    int outcome = STORY_GOING;
    count_bad_guys(field, verter);
    if (total_bad_guys(verter) == 0)
        outcome = STORY_WON;
    else if (verter_charge(verter) == 0)
        outcome = STORY_LOST;
    return outcome;
}

void count_bad_guys(const int field[HEIGHT][WIDTH], Patient* verter) {
    for (int module = 0; module < MODULES; module++) verter->bad_guys[module] = 0;
    for (int row = 0; row < HEIGHT; row++)
        for (int col = 0; col < WIDTH; col++)
            if (field[row][col] == VIRUS) verter->bad_guys[which_module(row, col)]++;
}

int total_bad_guys(const Patient* verter) {
    int total = 0;
    for (int module = 0; module < MODULES; module++) total += verter->bad_guys[module];
    return total;
}

int healed_modules(const Patient* verter) {
    int healed = 0;
    for (int module = 0; module < MODULES; module++) healed += verter->bad_guys[module] == 0;
    return healed;
}

// Питание: 21% минус 1% за каждые 42 поколения и минус 1% за каждые 5 патчей. Ниже нуля не падает.
// Деление целых отбрасывает остаток: 41 поколение — ещё 0 процентов утечки, 42 — уже один.
int verter_charge(const Patient* verter) {
    int charge = POWER_START - (verter->generation - 1) / LEAK_PERIOD - verter->patches / PATCHES_PER_PERCENT;
    return charge > 0 ? charge : 0;
}

// Палата: рамка, подписи модулей, клетки, шприц и строка состояния.
// Мёртвые клетки на границах модулей (строка 12 и столбец 40) рисуются тусклой точкой:
// так видно, где кончается один модуль и начинается другой, а живые клетки ничем не закрыты.
// Шприц — '+' поверх клетки: у живой клетки сохраняется цвет, меняется только символ.
void draw_ward(const int field[HEIGHT][WIDTH], const Patient* verter) {
    erase();
    if (!is_window_big_enough()) {
        say_in_middle(LINES / 2, "Окно мало: нужно хотя бы 82 x 28.");
    } else {
        draw_fence(0, 0, HEIGHT + 2, WIDTH + 2);
        draw_module_labels(verter);
        for (int row = 0; row < HEIGHT; row++)
            for (int col = 0; col < WIDTH; col++) {
                chtype look = paint_cell(field[row][col], 1);
                if (field[row][col] == DEAD && (row == HALF_HEIGHT || col == HALF_WIDTH)) look = '.' | A_DIM;
                mvaddch(row + 1, col + 1, look);
            }
        int row = verter->syringe_row;
        int col = verter->syringe_col;
        mvaddch(row + 1, col + 1, (paint_cell(field[row][col], 1) & ~A_CHARTEXT) | '+' | A_BOLD);
        draw_ward_status(verter);
    }
    refresh();
}

// Подписи модулей врезаны в рамку: верхние — в верхнюю линию, нижние — в нижнюю.
// Число — сколько в модуле вирусов; вылеченный модуль пишется зелёным и со словом «чисто».
void draw_module_labels(const Patient* verter) {
    const char* const names[MODULES] = {"ЛИНТЕР", "CPPCHECK", "УТЕЧКИ ПАМЯТИ", "7 ПРИНЦИПОВ"};
    for (int module = 0; module < MODULES; module++) {
        int row = module < 2 ? 0 : HEIGHT + 1;
        int col = 2 + (module % 2) * HALF_WIDTH;
        int healed = verter->bad_guys[module] == 0;
        int pair = healed && has_colors() ? PAIR_HEALED : 0;
        attron(COLOR_PAIR(pair) | A_BOLD);
        if (healed)
            mvprintw(row, col, " %s: чисто ", names[module]);
        else
            mvprintw(row, col, " %s: %d ", names[module], verter->bad_guys[module]);
        attroff(COLOR_PAIR(pair) | A_BOLD);
    }
}

// Строка под рамкой. «до -1%» — сколько патчей ещё можно поставить, пока не спишется процент.
// Питание POWER_ALARM% и меньше — белым по красному.
void draw_ward_status(const Patient* verter) {
    int charge = verter_charge(verter);
    int pair = charge <= POWER_ALARM && has_colors() ? PAIR_ALARM : 0;
    move(HEIGHT + 2, 1);
    attron(COLOR_PAIR(pair) | A_BOLD);
    printw(" Питание: %d%% ", charge);
    attroff(COLOR_PAIR(pair) | A_BOLD);
    printw(" Патчей до -1%%: %d  Модули: %d/%d  Скорость: %d/%d  Enter - патч  Space - сдаться",
           PATCHES_PER_PERCENT - verter->patches % PATCHES_PER_PERCENT, healed_modules(verter), MODULES,
           verter->speed, SPEED_MAX);
}

// Как выглядит клетка. Живая — сплошной квадрат: пробел с A_REVERSE, цвет фона становится цветом клетки.
// В Сюжете вирус красный, антивирус зелёный. Без цветов (или в Песочнице) вирус — белый квадрат,
// а антивирус — буква 'o', чтобы их всё равно можно было различить.
chtype paint_cell(int state, int in_story) {
    chtype look = ' ';
    int colors = has_colors() && in_story;
    if (state == VIRUS)
        look = ' ' | A_REVERSE | COLOR_PAIR(colors ? PAIR_VIRUS : 0);
    else if (state == ANTIVIRUS)
        look = colors ? ' ' | A_REVERSE | COLOR_PAIR(PAIR_ANTIVIRUS) : 'o' | A_BOLD;
    return look;
}

// Итог Сюжета. На следующем этапе здесь будут окна Вертера и концовки.
void show_verdict(int outcome) {
    const char* text = "Вы сдались. Вертер ждёт вас в меню.";
    if (outcome == STORY_WON)
        text = "Все модули чисты. Вертер спасён!";
    else if (outcome == STORY_LOST)
        text = "Питание: 0%. Вертер отключился.";
    erase();
    say_in_middle(LINES / 2, text);
    say_in_middle(LINES / 2 + 2, "Любая клавиша - в меню.");
    refresh();
    getch();
}

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

int is_window_big_enough(void) { return LINES >= HEIGHT + 3 && COLS >= WIDTH + 2; }

void say_in_middle(int row, const char* text) { mvaddstr(row, (COLS - how_wide(text)) / 2, text); }

int how_wide(const char* text) {
    int width = 0;
    for (int i = 0; text[i] != '\0'; i++) width += is_letter_start(text[i]);
    return width;
}

int is_letter_start(char byte) { return ((unsigned char)byte & 0xC0) != 0x80; }
