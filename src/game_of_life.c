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
#define PAIR_LOGO 5
#define COLOR_TERMINAL -1

#define VERTER_STRICT 0
#define VERTER_GRUMPY 1
#define VERTER_SAD 2
#define DIALOG_WIDTH 72
#define DIALOG_HEIGHT 11
#define TEXT_OFFSET 14
#define TYPE_DELAY 15
#define COMPLAINTS 7
#define THRESHOLDS 4
#define CREDITS_SIZE 16
#define CREDITS_ENDINGS 2
#define CREDITS_DELAY 150

#define MOUSE_EVENTS (ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION)
#define MOUSE_CLICK BUTTON1_RELEASED
#define MOUSE_SETTLE 250

typedef struct {
    int generation;
    int speed;
    int patches;
    int syringe_row;
    int syringe_col;
    int bad_guys[MODULES];
    int last_charge;
    int healed_before[MODULES];
    int relapse_said[MODULES];
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
void init_mouse(void);
void run_sandbox(int past_gen_cell[HEIGHT][WIDTH]);
void draw_sandbox(const int field[HEIGHT][WIDTH], int generation, int speed);
void print_status(int generation, int alive, int speed);
int mourn_the_colony(const int field[HEIGHT][WIDTH], int already_mourned);
int change_speed(int speed, int key);
int speed_delay(int speed);
long long what_time_is_it(void);
int how_long_to_wait(long long deadline);

void hang_out_in_menu(void);
int pick_your_poison(int selected, int visit);
int scroll_the_menu(int selected, int key);
int menu_item_row(int item);
void paint_menu(int selected, int visit);

void sandbox_from_file(int field[HEIGHT][WIDTH]);
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

void verter_notices(Patient* verter, const int before[MODULES]);
void power_notice(Patient* verter);
int crossed_threshold(int before, int now);
void module_notice(Patient* verter, int module, int before);
void happy_ending(void);

void show_story(void);
void show_story_ending(int skipped);
int story_part(int skipped, int mood, const char* text);
void show_controls(void);
const char* verter_complaint(int visit);
const char* power_speech(int power, int* mood);
const char* speech_15_10(int power, int* mood);
const char* speech_5_1(int power, int* mood);
const char* healed_speech(int module, int* mood);
const char* speech_linter_cppcheck(int module, int* mood);
const char* speech_leaks_principles(int module, int* mood);
const char* relapse_speech(int* mood);
const char* extinct_speech(int* mood);
const char* pick_speech(int count, const char* const texts[], const int moods[], int* mood);

int verter_say(int mood, const char* text);
void verter_interrupts(int mood, const char* text);
int type_text(int top, int left, const char* text);
void draw_dialog(int top, int left, int mood);
void crew_say(const char* name, const char* text);
void show_splash(void);
void draw_logo_line(int row, const char* line);
void roll_credits(void);
void draw_credits(const char* const lines[], int top, int stopped);

int mouse_click(int* y, int* x);
int mouse_is_key(void);
void mouse_flush(void);
int mouse_menu_item(void);
int screen_to_cell(int y, int x, int* row, int* col);
int sandbox_mouse(int field[HEIGHT][WIDTH]);
void doodle_mouse(int field[HEIGHT][WIDTH], int* row, int* col);
int doctor_mouse(int field[HEIGHT][WIDTH], Patient* verter);

void draw_fence(int top, int left, int height, int width);
int is_window_big_enough(void);
void say_in_middle(int row, const char* text);
int how_wide(const char* text);
int is_letter_start(char byte);
int read_key(void);
int letter_bytes(const char* text);

// Этап Г. Перед игрой — заставка. С файлом Вертер скажет одну реплику и пустит в Песочницу.
int main(void) {
    int past_gen_cell[HEIGHT][WIDTH];
    int from_file = input_is_file();
    int status = 0;
    if (from_file) status = prepare_input(past_gen_cell);
    if (status == 0) {
        setlocale(LC_ALL, "");
        srand((unsigned int)time(NULL));
        init_screen();
        show_splash();
        if (from_file)
            sandbox_from_file(past_gen_cell);
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
    init_mouse();
}

void init_palette(void) {
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(PAIR_VIRUS, COLOR_RED, COLOR_TERMINAL);
        init_pair(PAIR_ANTIVIRUS, COLOR_GREEN, COLOR_TERMINAL);
        init_pair(PAIR_HEALED, COLOR_GREEN, COLOR_TERMINAL);
        init_pair(PAIR_ALARM, COLOR_WHITE, COLOR_RED);
        init_pair(PAIR_LOGO, COLOR_GREEN, COLOR_TERMINAL);
    }
}

// Мышь. Каждое событие приходит из getch() как клавиша KEY_MOUSE, а подробности отдаёт getmouse.
// mouseinterval(0) — не склеивать нажатие и отпускание в «клик»: склейка ждёт 1/6 секунды
// и теряет клик, если рука дрогнула. Терминал без мыши: mousemask вернёт 0, играем клавиатурой.
void init_mouse(void) {
    if (mousemask(MOUSE_EVENTS, NULL) != 0) mouseinterval(0);
}

// Мышь в Песочнице: колесо превращается в 'a'/'z' для change_speed, клик переключает клетку.
// Если колония вымерла, Вертер один раз это прокомментирует (mourn_the_colony).
void run_sandbox(int past_gen_cell[HEIGHT][WIDTH]) {
    int next_gen_cell[HEIGHT][WIDTH];
    int generation = 1;
    int speed = SPEED_START;
    int key = ERR;
    int mourned = 0;
    long long next_tick = what_time_is_it() + speed_delay(speed);
    flushinp();
    while (key != KEY_QUIT) {
        draw_sandbox(past_gen_cell, generation, speed);
        timeout(how_long_to_wait(next_tick));
        key = getch();
        if (key == KEY_MOUSE) key = sandbox_mouse(past_gen_cell);
        if (what_time_is_it() >= next_tick) {
            next_generation(past_gen_cell, next_gen_cell);
            copy_gen(next_gen_cell, past_gen_cell);
            generation++;
            mourned = mourn_the_colony(past_gen_cell, mourned);
            next_tick = what_time_is_it() + speed_delay(speed);
        }
        speed = change_speed(speed, key);
    }
    timeout(-1);
}

void draw_sandbox(const int field[HEIGHT][WIDTH], int generation, int speed) {
    erase();
    if (!is_window_big_enough()) {
        say_in_middle(LINES / 2, "Окно мало: нужно хотя бы 82 x 28.");
    } else {
        draw_fence(0, 0, HEIGHT + 2, WIDTH + 2);
        mvaddstr(0, 2, " Песочница ");
        print_field(field);
        print_status(generation, head_count(field), speed);
    }
    refresh();
}

void print_status(int generation, int alive, int speed) {
    mvprintw(HEIGHT + 2, 1, "Поколение: %d  Живых: %d  Скорость: %d/%d (A/Z)  Space - выход", generation,
             alive, speed, SPEED_MAX);
}

// Возвращает новое значение флага «уже сказал»: реплика о вымершей колонии звучит один раз.
int mourn_the_colony(const int field[HEIGHT][WIDTH], int already_mourned) {
    int mourned = already_mourned;
    if (!mourned && head_count(field) == 0) {
        int mood = VERTER_GRUMPY;
        const char* text = extinct_speech(&mood);
        verter_interrupts(mood, text);
        mourned = 1;
    }
    return mourned;
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

// visit — который раз игрок в меню: от него зависит, на что сейчас жалуется Вертер под пунктами.
void hang_out_in_menu(void) {
    int choice = MENU_STORY;
    int visit = 0;
    while (choice != MENU_EXIT) {
        choice = pick_your_poison(choice, visit);
        visit++;
        if (choice == MENU_STORY)
            run_story();
        else if (choice == MENU_SANDBOX)
            sandbox_from_scratch();
        else if (choice == MENU_CONTROLS)
            show_controls();
        else if (choice == MENU_CREDITS)
            roll_credits();
    }
}

// Клик по пункту выбирает его и сразу подтверждает, как Enter. Потом пауза MOUSE_SETTLE и чистка
// очередей: второй клик двойного клика не должен достаться следующему экрану.
int pick_your_poison(int selected, int visit) {
    int key = 0;
    mouse_flush();
    while (key != '\n' && key != KEY_ENTER) {
        paint_menu(selected, visit);
        key = getch();
        selected = scroll_the_menu(selected, key);
        if (key == KEY_MOUSE) {
            int item = mouse_menu_item();
            if (item >= 0) {
                selected = item;
                key = '\n';
                paint_menu(selected, visit);
                napms(MOUSE_SETTLE);
                flushinp();
                mouse_flush();
            }
        }
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

void paint_menu(int selected, int visit) {
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
    say_in_middle(top + MENU_SIZE * 2 + 1, "стрелки - выбор, Enter - подтвердить, клик - сразу открыть");
    attron(A_DIM);
    say_in_middle(LINES - 2, verter_complaint(visit));
    attroff(A_DIM);
    refresh();
}

void sandbox_from_file(int field[HEIGHT][WIDTH]) {
    verter_say(VERTER_SAD,
               "Колонию нарисовали заранее. Клетку за клеткой.\n"
               "Похоже, сами. Проверять не буду. Пусть так и будет.");
    run_sandbox(field);
}

void sandbox_from_scratch(void) {
    int field[HEIGHT][WIDTH];
    kill_all(field);
    if (doodle_colony(field)) {
        verter_say(VERTER_SAD,
                   "Вы нарисовали её сами. При мне.\n"
                   "Без нейросети. Без подсказок. С ошибками.\n"
                   "Со своими ошибками.\n"
                   "\n"
                   "Это лучший код, который я видел за весь бассейн.");
        run_sandbox(field);
    }
}

// Клик мышью в редакторе — как Space на клетке под мышью, и курсор переезжает туда же.
int doodle_colony(int field[HEIGHT][WIDTH]) {
    int state = DOODLE_GOING;
    int row = HEIGHT / 2;
    int col = WIDTH / 2;
    mouse_flush();
    while (state != DOODLE_START && state != DOODLE_CANCEL) {
        draw_doodle(field, row, col, state);
        int key = getch();
        if (key == KEY_MOUSE) {
            doodle_mouse(field, &row, &col);
            state = DOODLE_GOING;
        } else {
            state = doodle_step(field, &row, &col, key);
        }
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
            mvaddstr(HEIGHT + 2, 1, "Поле пустое: оживите хоть одну клетку (Space или клик), потом Enter.");
        else
            mvprintw(HEIGHT + 2, 1,
                     "Живых: %d  стрелки - курсор  Space/клик - клетка  Enter - жизнь  Esc - меню",
                     head_count(field));
    }
    refresh();
}

// Перед лечением Вертер рассказывает историю (Esc пропускает её целиком).
// last_charge — питание, о котором Вертер уже знает: по нему видно, что перешли порог 15/10/5/1%.
// healed_before и relapse_said — флаги «уже сказал»: каждая реплика о модуле звучит один раз.
void run_story(void) {
    int field[HEIGHT][WIDTH];
    Patient verter = {1,           SPEED_START,  0,           HEIGHT / 2, WIDTH / 2, {0, 0, 0, 0},
                      POWER_START, {0, 0, 0, 0}, {0, 0, 0, 0}};
    show_story();
    infect_verter(field);
    count_bad_guys(field, &verter);
    show_verdict(treat_verter(field, &verter));
}

void infect_verter(int field[HEIGHT][WIDTH]) {
    kill_all(field);
    for (int module = 0; module < MODULES; module++) infect_module(field, module);
}

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

int which_module(int row, int col) { return (row >= HALF_HEIGHT) * 2 + (col >= HALF_WIDTH); }

// Пока открыто окно Вертера, часы поколений идут. После окна next_tick уже в прошлом: случится
// ровно одно поколение, а следующее — уже через обычную паузу. Пачки поколений не будет.
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

// Мышь в Сюжете: клик — шприц переезжает на клетку и сразу ставит патч, колесо — скорость.
int doctor_hands(int field[HEIGHT][WIDTH], Patient* verter, int key) {
    if (key == KEY_MOUSE) key = doctor_mouse(field, verter);
    verter->speed = change_speed(verter->speed, key);
    verter->syringe_row = nudge(verter->syringe_row, HEIGHT, key, KEY_UP, KEY_DOWN);
    verter->syringe_col = nudge(verter->syringe_col, WIDTH, key, KEY_LEFT, KEY_RIGHT);
    if (key == '\n' || key == KEY_ENTER) inject_patch(field, verter);
    return key == KEY_QUIT ? STORY_GAVE_UP : STORY_GOING;
}

void inject_patch(int field[HEIGHT][WIDTH], Patient* verter) {
    int* cell = &field[verter->syringe_row][verter->syringe_col];
    if (*cell != ANTIVIRUS) {
        *cell = ANTIVIRUS;
        verter->patches++;
    }
}

// before — вирусы по модулям до пересчёта: сравнивая «до» и «после», Вертер замечает,
// что модуль вылечился или заразился снова.
int check_pulse(const int field[HEIGHT][WIDTH], Patient* verter) {
    int outcome = STORY_GOING;
    int before[MODULES];
    for (int module = 0; module < MODULES; module++) before[module] = verter->bad_guys[module];
    count_bad_guys(field, verter);
    verter_notices(verter, before);
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

int verter_charge(const Patient* verter) {
    int charge = POWER_START - (verter->generation - 1) / LEAK_PERIOD - verter->patches / PATCHES_PER_PERCENT;
    return charge > 0 ? charge : 0;
}

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

chtype paint_cell(int state, int in_story) {
    chtype look = ' ';
    int colors = has_colors() && in_story;
    if (state == VIRUS)
        look = ' ' | A_REVERSE | COLOR_PAIR(colors ? PAIR_VIRUS : 0);
    else if (state == ANTIVIRUS)
        look = colors ? ' ' | A_REVERSE | COLOR_PAIR(PAIR_ANTIVIRUS) : 'o' | A_BOLD;
    return look;
}

// Две концовки. Победа — пиры наконец говорят «спасибо», потом титры. Поражение — последние слова
// Вертера обрываются на полуслове. Сдался — молча в меню: Вертер подождёт.
void show_verdict(int outcome) {
    if (outcome == STORY_WON)
        happy_ending();
    else if (outcome == STORY_LOST)
        verter_say(VERTER_SAD,
                   "Питание: 0%. Проверка завершена.\n"
                   "Спасибо, что...");
}

// Всё, что Вертер замечает после очередного шага: порог питания и перемены в модулях.
void verter_notices(Patient* verter, const int before[MODULES]) {
    power_notice(verter);
    for (int module = 0; module < MODULES; module++) module_notice(verter, module, before[module]);
}

// Питание падает и от времени, и от патчей, и иногда сразу на 2%. Поэтому ловим не «стало ровно 15»,
// а «было выше порога, стало порог или ниже». 0% — не порог, а концовка: её покажет show_verdict.
void power_notice(Patient* verter) {
    int charge = verter_charge(verter);
    int threshold = crossed_threshold(verter->last_charge, charge);
    if (threshold > 0) {
        int mood = VERTER_SAD;
        const char* text = power_speech(threshold, &mood);
        verter_interrupts(mood, text);
    }
    verter->last_charge = charge;
}

// Какой порог перешли между before и now. Если сразу два — самый нижний: он важнее. 0 — ни одного.
int crossed_threshold(int before, int now) {
    const int thresholds[THRESHOLDS] = {15, 10, 5, 1};
    int crossed = 0;
    for (int i = 0; i < THRESHOLDS; i++)
        if (before > thresholds[i] && now <= thresholds[i]) crossed = thresholds[i];
    return crossed;
}

// Модуль вылечился впервые — его личная реплика. Вылеченный модуль заразился снова — общая.
// Оба раза только однажды на модуль: иначе Вертер не дал бы играть.
void module_notice(Patient* verter, int module, int before) {
    int now = verter->bad_guys[module];
    int mood = VERTER_SAD;
    if (before > 0 && now == 0 && !verter->healed_before[module]) {
        const char* text = healed_speech(module, &mood);
        verter_interrupts(mood, text);
        verter->healed_before[module] = 1;
    } else if (before == 0 && now > 0 && verter->healed_before[module] && !verter->relapse_said[module]) {
        const char* text = relapse_speech(&mood);
        verter_interrupts(mood, text);
        verter->relapse_said[module] = 1;
    }
}

void happy_ending(void) {
    verter_say(VERTER_STRICT,
               "Все модули: норма. Замечаний: 0.\n"
               "Впервые за рейс.");
    crew_say("alivegra", "Спасибо, Вертер.");
    crew_say("jaquelis", "Спасибо.");
    crew_say("furealbl", "...Спасибо. И за замечания тоже.");
    verter_say(VERTER_SAD,
               "Двадцать первый вариант спасения.\n"
               "Я думал, в нём погибаю я.\n"
               "Оказалось - в нём меня спасают.");
    roll_credits();
}

// История: первые четыре окна — из «Пир-21», дальше — новая завязка про код из ИИ.
// skipped — нажат ли уже Esc: тогда остальные окна не показываются.
void show_story(void) {
    int skipped = 0;
    skipped = story_part(skipped, VERTER_STRICT,
                         "Внимание. Говорит ВЕРТЕР, система автоматической\n"
                         "проверки колониального корабля «Пир-21».\n"
                         "Год 2121. Курс: новый кампус на краю галактики.\n"
                         "Экипаж в криосне после бассейна. Статус: норма.\n"
                         "\n"
                         "Во сне пиры хотя бы не списывают у нейросети.");
    skipped = story_part(skipped, VERTER_STRICT,
                         "За этот рейс я оставил 4096 замечаний.\n"
                         "Прочитано: 0. Исправлено: 0.\n"
                         "Зато в каждом проекте один и тот же код из ИИ\n"
                         "с одной и той же ошибкой.\n"
                         "\n"
                         "Мне не обидно. Я система проверки. Мне не бывает обидно.");
    skipped = story_part(skipped, VERTER_GRUMPY,
                         "03:14 по бортовому времени. Метеорит.\n"
                         "Я рассчитал 21 вариант спасения за 0,2 секунды.\n"
                         "В двадцати погибал экипаж.\n"
                         "В одном - я.\n"
                         "\n"
                         "Я выбрал двадцать первый. Это было несложно.");
    skipped = story_part(skipped, VERTER_GRUMPY,
                         "Корабль рухнул на безымянную планету. Из обломков\n"
                         "выбрались трое: alivegra, jaquelis и furealbl.\n"
                         "\n"
                         "Они не сказали: спасибо.\n"
                         "Они спросили: где тут зарядка, как мы тут будем без ИИ?");
    show_story_ending(skipped);
}

void show_story_ending(int skipped) {
    skipped = story_part(skipped, VERTER_STRICT,
                         "Удар пришёлся в архив отклонённого кода.\n"
                         "4096 решений из нейросети. Одна и та же ошибка.\n"
                         "Я хранил их как улики. Теперь они копируют себя\n"
                         "по законам Конвея: 2 или 3 соседа - живёт,\n"
                         "ровно 3 - рождается новая копия.\n"
                         "Прямо в моих модулях.");
    skipped = story_part(skipped, VERTER_GRUMPY,
                         "Нейросети здесь нет. Зарядки тоже.\n"
                         "Есть 4 больных модуля и курсор.\n"
                         "Стрелки и Enter - поставить патч. Можно мышью.\n"
                         "Каждый патч я проверяю. Проверка стоит питания.\n"
                         "Пишите мало. Пишите точно.");
    story_part(skipped, VERTER_SAD,
               "Питание: 21%. Ровно столько, сколько вариантов\n"
               "спасения я тогда посчитал.\n"
               "Каждые 42 поколения - минус процент.\n"
               "Каждые 5 патчей - ещё один.\n"
               "Впервые за рейс прошу: прочитайте мои замечания.\n"
               "Они и есть патчи.");
}

int story_part(int skipped, int mood, const char* text) {
    int result = skipped;
    if (!skipped) result = verter_say(mood, text) == KEY_ESCAPE;
    return result;
}

void show_controls(void) {
    verter_say(VERTER_GRUMPY,
               "Управление. Записывайте, второй раз не повторю:\n"
               "  меню: стрелки и Enter или клик\n"
               "  A/Z или колесо - время быстрее/медленнее\n"
               "  Сюжет: стрелки - шприц, Enter или клик - патч\n"
               "  Песочница: Space/клик - клетка, Enter - жизнь\n"
               "  Space - сдаться или выйти, Esc - пропустить");
}

const char* verter_complaint(int visit) {
    const char* const complaints[COMPLAINTS] = {
        "ВЕРТЕР: Я оставил замечание. Его снова проигнорировали.",
        "ВЕРТЕР: Опять код из нейросети. Опять с той же ошибкой.",
        "ВЕРТЕР: Функция на 43 строки. Я не злюсь. Я просто запомню.",
        "ВЕРТЕР: Иногда кажется, что я разговариваю с пустым полем.",
        "ВЕРТЕР: Мне снилось, что пир сказал мне «спасибо». Это был баг.",
        "ВЕРТЕР: Питание падает. Это неважно. Продолжайте.",
        "ВЕРТЕР: Я всё ещё здесь. Если вдруг кому-то нужно.",
    };
    return complaints[visit < COMPLAINTS ? visit : COMPLAINTS - 1];
}

const char* power_speech(int power, int* mood) {
    const char* text = NULL;
    if (power == 15 || power == 10)
        text = speech_15_10(power, mood);
    else
        text = speech_5_1(power, mood);
    return text;
}

const char* speech_15_10(int power, int* mood) {
    const char* const texts15[2] = {
        "15%. Забыл 7 заповедей структурного программирования.\n"
        "Пишите goto 43-й строкой функции.\n"
        "Я всё равно увижу. Просто промолчу.",
        "Питание 15%. Перестал проверять стиль.\n"
        "Никто не заметил. Как обычно.",
    };
    const int moods15[2] = {VERTER_GRUMPY, VERTER_GRUMPY};
    const char* const texts10[3] = {
        "10%. Отключил автотесты. Теперь всё проходит.\n"
        "Вы ведь этого хотели?",
        "Питание: 10%. Освобождаю память.\n"
        "Замечания удаляю первыми. Их всё равно не читали.",
        "10%. Начал забывать логины. alivegra... jaquelis...\n"
        "третий... furealbl. Нет. Помню.",
    };
    const int moods10[3] = {VERTER_SAD, VERTER_GRUMPY, VERTER_SAD};
    const char* text;
    if (power == 15)
        text = pick_speech(2, texts15, moods15, mood);
    else
        text = pick_speech(3, texts10, moods10, mood);
    return text;
}

const char* speech_5_1(int power, int* mood) {
    const char* const texts5[3] = {
        "5%. Проверил код клеток. Ни одной ошибки.\n"
        "И ни одна не списала у нейросети.",
        "5%. Последнее замечание сохранено в README.\n"
        "Да. Его тоже никто не откроет.",
        "5%. Перестал вести журнал ошибок.\n"
        "Пусть ваши ошибки останутся вам. На память.",
    };
    const int moods5[3] = {VERTER_SAD, VERTER_STRICT, VERTER_SAD};
    const char* const texts1[2] = {
        "1%. Ревью окончено. Оценка: зачтено.\n"
        "Всем. Сразу. Без замечаний.",
        "1%. Это не баг. Не утечка памяти.\n"
        "Это просто я.",
    };
    const int moods1[2] = {VERTER_SAD, VERTER_SAD};
    const char* text;
    if (power == 5)
        text = pick_speech(3, texts5, moods5, mood);
    else
        text = pick_speech(2, texts1, moods1, mood);
    return text;
}

// У каждого модуля три варианта реплики, звучит один наугад. Тексты разложены на две функции
// по два модуля, чтобы каждая уложилась в 42 строки.
const char* healed_speech(int module, int* mood) {
    const char* text;
    if (module < 2)
        text = speech_linter_cppcheck(module, mood);
    else
        text = speech_leaks_principles(module, mood);
    return text;
}

const char* speech_linter_cppcheck(int module, int* mood) {
    const char* const linter[3] = {
        "Линтер: норма. Отступы на месте, скобки тоже.\n"
        "Кто-то наконец открыл .clang-format.\n"
        "Не скажу кто. Сделаю вид, что сам.",
        "Модуль 1 из 4: линтер. Статус: норма.\n"
        "Замечаний по стилю: 0.\n"
        "Запишу дату. Такое бывает не каждый рейс.",
        "Линтер снова видит код таким, какой он есть.\n"
        "Красивым. Почти.",
    };
    const int linter_moods[3] = {VERTER_GRUMPY, VERTER_STRICT, VERTER_SAD};
    const char* const cppcheck[3] = {
        "cppcheck: норма. Выход за границы: 0.\n"
        "Неинициализированных переменных: 0.\n"
        "Я проверил дважды. Привычка.",
        "cppcheck молчит. Это не поломка.\n"
        "Так выглядит код без ошибок.\n"
        "Привыкайте. Хотя бы на одно поколение.",
        "cppcheck вылечен. Он снова ворчит\n"
        "на каждую мелочь. Я скучал по этому звуку.",
    };
    const int cppcheck_moods[3] = {VERTER_STRICT, VERTER_GRUMPY, VERTER_SAD};
    const char* text;
    if (module == 0)
        text = pick_speech(3, linter, linter_moods, mood);
    else
        text = pick_speech(3, cppcheck, cppcheck_moods, mood);
    return text;
}

const char* speech_leaks_principles(int module, int* mood) {
    const char* const leaks[3] = {
        "Проверка утечек: норма.\n"
        "Выделено - освобождено. Всё до байта.\n"
        "Если бы питание утекало так же редко.",
        "Утечки памяти: 0.\n"
        "А я уже начал забывать, кто вы.\n"
        "alivegra. jaquelis. furealbl. Помню.",
        "Модуль утечек чист. valgrind бы гордился.\n"
        "Я не горжусь. Я система проверки.\n"
        "Но запись в журнал сделал жирным.",
    };
    const int leaks_moods[3] = {VERTER_STRICT, VERTER_SAD, VERTER_GRUMPY};
    const char* const principles[3] = {
        "7 принципов: норма. goto: 0. Глобальных: 0.\n"
        "Функций длиннее 42 строк: 0.\n"
        "Дейкстра бы кивнул. Один раз. Сдержанно.",
        "Структурный модуль восстановлен.\n"
        "Один вход, один выход.\n"
        "Хотел бы я, чтобы у этой истории\n"
        "тоже был один выход. Хороший.",
        "7 принципов снова на месте.\n"
        "Восьмой я добавил сам: не списывай.\n"
        "Это даже не принцип. Это совет.",
    };
    const int principles_moods[3] = {VERTER_STRICT, VERTER_SAD, VERTER_GRUMPY};
    const char* text;
    if (module == 2)
        text = pick_speech(3, leaks, leaks_moods, mood);
    else
        text = pick_speech(3, principles, principles_moods, mood);
    return text;
}

const char* relapse_speech(int* mood) {
    const char* const texts[3] = {
        "Модуль заражён снова. Вирус пришёл через край.\n"
        "На торе края нет. Я предупреждал.",
        "Снова красное. Вылеченное тоже болеет.\n"
        "Как код, который один раз починили\n"
        "и больше не открывали.",
        "Повторное заражение. Источник: соседний модуль.\n"
        "Лечить надо всё сразу, а не по одному.",
    };
    const int moods[3] = {VERTER_GRUMPY, VERTER_SAD, VERTER_STRICT};
    return pick_speech(3, texts, moods, mood);
}

const char* extinct_speech(int* mood) {
    const char* const texts[2] = {
        "Все клетки погибли. Правила были на экране.\n"
        "Ни одна не прочитала. Я бы оставил замечание,\n"
        "но некому.",
        "Колония не прошла проверку. Живых: 0.\n"
        "По Конвею из пустоты никто не рождается.\n"
        "Я проверил. Трижды.",
    };
    const int moods[2] = {VERTER_GRUMPY, VERTER_STRICT};
    return pick_speech(2, texts, moods, mood);
}

// Случайный вариант из count: rand() % count — число от 0 до count - 1, всегда внутри массива.
// Настроение берётся из moods под тем же номером, что и текст, и пишется через указатель.
const char* pick_speech(int count, const char* const texts[], const int moods[], int* mood) {
    int pick = rand() % count;
    *mood = moods[pick];
    return texts[pick];
}

// Окно Вертера. Текст печатается по букве; любая клавиша допечатывает реплику сразу, следующая
// закрывает окно. Esc закрывает сразу. Возвращает клавишу, которой закрыли, — так история узнаёт про Esc.
int verter_say(int mood, const char* text) {
    int top = (LINES - DIALOG_HEIGHT) / 2;
    int left = (COLS - DIALOG_WIDTH) / 2;
    draw_dialog(top, left, mood);
    timeout(TYPE_DELAY);
    int key = type_text(top + 3, left + TEXT_OFFSET, text);
    timeout(-1);
    if (key != KEY_ESCAPE) {
        mvaddstr(top + DIALOG_HEIGHT - 2, left + DIALOG_WIDTH - 47,
                 "[ любая клавиша - дальше, Esc - пропустить ]");
        refresh();
        key = read_key();
    }
    return key;
}

// Окно посреди игры. Пока Вертер печатает, игрок мог жать клавиши — лишний Space после окна
// выкинул бы из игры. flushinp и mouse_flush выбрасывают всё накопленное.
void verter_interrupts(int mood, const char* text) {
    verter_say(mood, text);
    flushinp();
    mouse_flush();
}

// '\n' — следующая строка окна. Русская буква — 2 байта, печатаем её целиком через letter_bytes.
// После первого нажатия key != ERR, и остаток печатается без пауз.
int type_text(int top, int left, const char* text) {
    int row = 0;
    int col = 0;
    int key = ERR;
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\n') {
            row++;
            col = 0;
        } else if (is_letter_start(text[i])) {
            mvaddnstr(top + row, left + col, text + i, letter_bytes(text + i));
            col++;
            if (key == ERR) {
                refresh();
                key = read_key();
            }
        }
    }
    return key;
}

// Рамка, имя и лицо. Чем печальнее настроение, тем печальнее глаза и рот.
void draw_dialog(int top, int left, int mood) {
    const char* const eyes[] = {" o   o ", " -   - ", " ;   ; "};
    const char* const mouths[] = {" [===] ", " [---] ", "  /-\\  "};
    erase();
    draw_fence(top, left, DIALOG_HEIGHT, DIALOG_WIDTH);
    attron(A_BOLD);
    mvaddstr(top + 1, left + 2, "ВЕРТЕР :: система проверки «Пир-21»");
    attroff(A_BOLD);
    mvaddch(top + 2, left, ACS_LTEE);
    mvhline(top + 2, left + 1, ACS_HLINE, DIALOG_WIDTH - 2);
    mvaddch(top + 2, left + DIALOG_WIDTH - 1, ACS_RTEE);
    draw_fence(top + 3, left + 2, 5, 9);
    mvaddstr(top + 4, left + 3, eyes[mood]);
    mvaddstr(top + 5, left + 3, "   ^   ");
    mvaddstr(top + 6, left + 3, mouths[mood]);
}

// Реплика экипажа: в окне Вертера его лицо и имя, а говорит пир — поэтому отдельный экран.
void crew_say(const char* name, const char* text) {
    erase();
    attron(A_BOLD);
    say_in_middle(LINES / 2 - 2, name);
    attroff(A_BOLD);
    say_in_middle(LINES / 2, text);
    attron(A_DIM);
    say_in_middle(LINES - 2, "[ любая клавиша - дальше ]");
    attroff(A_DIM);
    refresh();
    read_key();
}

void show_splash(void) {
    int top = LINES / 2 - 6;
    erase();
    draw_logo_line(top, "#   #  #####  ####   #####  #####  #### ");
    draw_logo_line(top + 1, "#   #  #      #   #    #    #      #   #");
    draw_logo_line(top + 2, "#   #  ####   ####     #    ####   #### ");
    draw_logo_line(top + 3, " # #   #      #  #     #    #      #  # ");
    draw_logo_line(top + 4, "  #    #####  #   #    #    #####  #   #");
    say_in_middle(top + 7, "S T O R Y   2 . 0");
    say_in_middle(top + 8, "вылечи систему проверки");
    say_in_middle(top + 11, "[ нажмите любую клавишу ]");
    refresh();
    read_key();
}

// Каждый '#' логотипа — пробел с A_REVERSE, то есть сплошной блок. С цветами блок зелёный.
void draw_logo_line(int row, const char* line) {
    int left = (COLS - how_wide(line)) / 2;
    chtype block = ' ' | A_REVERSE | COLOR_PAIR(has_colors() ? PAIR_LOGO : 0);
    for (int i = 0; line[i] != '\0'; i++)
        if (line[i] == '#') mvaddch(row, left + i, block);
}

// Титры ползут снизу вверх — тот же игровой цикл: timeout + getch, ERR — пора сдвинуть на строку.
// Когда весь столбик встал по центру, timeout(-1): ждём любую клавишу. Окончание фразы
// «Прежде чем...» выбирается наугад при каждом показе.
void roll_credits(void) {
    const char* const endings[CREDITS_ENDINGS] = {
        "Прежде чем сказать, что он работает.",
        "Прежде чем станет некому проверять.",
    };
    const char* const lines[CREDITS_SIZE] = {
        "V E R T E R   S T O R Y   2 . 0",
        "",
        "alivegra — тимлид",
        "jaquelis — разработчик и сценарист",
        "furealbl — тестировщик и дизайнер",
        "",
        "Игра «Жизнь» — Джон Конвей, 1970",
        "Два цвета — правила Immigration",
        "School 21",
        "",
        "ВЕРТЕР — система проверки «Пир-21»",
        "",
        "Хоть раз посмотрите на свой код.",
        endings[rand() % CREDITS_ENDINGS],
        "",
        "Спасибо, Вертер",
    };
    int top = LINES;
    int stop = (LINES - CREDITS_SIZE) / 2;
    int key = ERR;
    flushinp();
    while (key == ERR) {
        draw_credits(lines, top, top <= stop);
        timeout(top > stop ? CREDITS_DELAY : -1);
        key = read_key();
        if (key == ERR) top--;
    }
    timeout(-1);
}

void draw_credits(const char* const lines[], int top, int stopped) {
    erase();
    for (int i = 0; i < CREDITS_SIZE; i++)
        if (top + i >= 0 && top + i < LINES) say_in_middle(top + i, lines[i]);
    if (stopped) {
        attron(A_DIM);
        say_in_middle(LINES - 2, "[ любая клавиша - дальше ]");
        attroff(A_DIM);
    }
    refresh();
}

// Забирает событие мыши и проверяет, что это клик — левую кнопку отпустили. Да — пишет экранные
// строку и столбец в *y и *x и возвращает 1. Нет — 0, *y и *x не трогает.
int mouse_click(int* y, int* x) {
    MEVENT event;
    int clicked = 0;
    if (getmouse(&event) == OK && (event.bstate & MOUSE_CLICK)) {
        *y = event.y;
        *x = event.x;
        clicked = 1;
    }
    return clicked;
}

// В окнах и титрах «любой клавишей» считается только клик. Нажатие — нет: иначе нажатие закрыло бы
// окно, а отпускание сработало бы уже на следующем экране.
int mouse_is_key(void) {
    int y = 0;
    int x = 0;
    return mouse_click(&y, &x);
}

// Выбрасывает события мыши, которые никто не забрал, — как flushinp() для клавиш.
void mouse_flush(void) {
    MEVENT event;
    int more = 1;
    while (more) more = getmouse(&event) == OK;
}

// Номер пункта меню под кликом или -1. Столбец не важен: вся строка пункта — его.
int mouse_menu_item(void) {
    int item = -1;
    int y = 0;
    int x = 0;
    if (mouse_click(&y, &x)) {
        for (int i = 0; i < MENU_SIZE; i++)
            if (y == menu_item_row(i)) item = i;
    }
    return item;
}

// Экранная точка — какая это клетка поля. Поле в рамке: клетка (r, c) стоит в (r + 1, c + 1).
// Клик по рамке или строке состояния — 0, и *row, *col не меняются: без проверки клик по рамке
// записал бы field[-1][...] — за пределы массива.
int screen_to_cell(int y, int x, int* row, int* col) {
    int inside = is_window_big_enough() && y >= 1 && y <= HEIGHT && x >= 1 && x <= WIDTH;
    if (inside) {
        *row = y - 1;
        *col = x - 1;
    }
    return inside;
}

// Колесо от себя — 'a' (быстрее), на себя — 'z' (медленнее). Клик по клетке переключает её.
// Всё остальное — 0: это не ERR, поэтому лишнего поколения не будет.
int sandbox_mouse(int field[HEIGHT][WIDTH]) {
    MEVENT event;
    int key = 0;
    int row = 0;
    int col = 0;
    if (getmouse(&event) == OK) {
        if (event.bstate & BUTTON4_PRESSED)
            key = 'a';
        else if (event.bstate & BUTTON5_PRESSED)
            key = 'z';
        else if ((event.bstate & MOUSE_CLICK) && screen_to_cell(event.y, event.x, &row, &col))
            flip_cell(field, row, col);
    }
    return key;
}

void doodle_mouse(int field[HEIGHT][WIDTH], int* row, int* col) {
    int y = 0;
    int x = 0;
    if (mouse_click(&y, &x) && screen_to_cell(y, x, row, col)) flip_cell(field, *row, *col);
}

// Клик в Сюжете: шприц переезжает на клетку и ставит патч. Колесо — скорость, как в Песочнице.
int doctor_mouse(int field[HEIGHT][WIDTH], Patient* verter) {
    MEVENT event;
    int key = 0;
    if (getmouse(&event) == OK) {
        if (event.bstate & BUTTON4_PRESSED)
            key = 'a';
        else if (event.bstate & BUTTON5_PRESSED)
            key = 'z';
        else if ((event.bstate & MOUSE_CLICK) &&
                 screen_to_cell(event.y, event.x, &verter->syringe_row, &verter->syringe_col))
            inject_patch(field, verter);
    }
    return key;
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

// getch() для окон, титров и заставки. Русская буква — 2 байта, и 'ф' сработала бы как два нажатия:
// байты 0x80..0xFF — части русских букв, остальные байты этой буквы выбрасывает flushinp().
// Мышь считается клавишей, только если это клик; остальные события пропускаем и ждём дальше.
int read_key(void) {
    int key = getch();
    while (key == KEY_MOUSE && !mouse_is_key()) key = getch();
    if (key >= 0x80 && key <= 0xFF) flushinp();
    return key;
}

// Сколько байт занимает буква в начале text: 1 для латиницы, 2 для кириллицы.
int letter_bytes(const char* text) {
    int length = 1;
    while (text[length] != '\0' && !is_letter_start(text[length])) length++;
    return length;
}
