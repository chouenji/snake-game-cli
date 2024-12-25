#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define ROWS 20
#define COLS 20
#define LEN 2
#define TERM_PADDING (140 - COLS * 2) / 2
#define DELAY 100000 // Microseconds

// g_ = global var
size_t g_is_game_over = 0;
size_t g_up = 0, g_down = 0, g_right = 1, g_left = 0;
size_t g_updated_len = LEN;

typedef struct Node {
  int row, col;
  unsigned char type;
  struct Node *next;
} Node;

typedef struct Snake {
  Node *tail, *head;
} Snake;

Node *g_board[ROWS][COLS];

// Main functions
void init_board();
void init_snake(Snake *snake);
void add_food();
void print_board();
void check_direction();
void move_snake(Snake *snake);

// Helper functions
void set_padding();
void set_text_padding();
void update_food(size_t *row, size_t *col);
void move_head(Snake *snake, int row_offset, int col_offset);
void move_tail(Snake *snake);

// Handler functions
int getch(void);
void handle_sigint();
void free_board();

int getch(void) {
  int ch;
  struct termios oldt;
  struct termios newt;

  tcgetattr(STDIN_FILENO, &oldt); /*store old settings */
  newt = oldt;                    /* copy old settings to new settings */
  newt.c_lflag &=
      ~(ICANON | ECHO); /* make one change to old settings in new settings */

  tcsetattr(STDIN_FILENO, TCSANOW,
            &newt); /*apply the new settings immediatly */

  ch = getchar(); /* standard getchar call */

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt); /*reapply the old settings */

  return ch; /*return received char */
}

int kbhit() {
  static const int STDIN = 0;
  static size_t initialized = 0;

  if (initialized == 0) {
    // Use termios to turn off line buffering
    struct termios term;
    tcgetattr(STDIN, &term);
    term.c_lflag &= ~ICANON;
    tcsetattr(STDIN, TCSANOW, &term);
    setbuf(stdin, NULL);
    initialized = 1;
  }

  int bytesWaiting;
  ioctl(STDIN, FIONREAD, &bytesWaiting);
  return bytesWaiting;
}

void handle_sigint() {
  g_is_game_over = 1;
  printf("\nGame interrupted by user (Ctrl + C). Exiting...\n");
}

void free_board() {
  for (size_t i = 0; i < ROWS; i++) {
    for (size_t j = 0; j < COLS; j++) {
      free(g_board[i][j]);
    }
  }
}

int main() {
  Snake *snake = malloc(sizeof(Snake));

  if (snake == NULL) {
    perror("Memory allocation failed - snake");
    exit(EXIT_FAILURE);
  }

  signal(SIGINT, handle_sigint);

  init_board();
  init_snake(snake);
  add_food();

  while (!g_is_game_over) {
    print_board();
    check_direction();
    usleep(DELAY);
    move_snake(snake);
    system("clear");
  }

  print_board();

  free_board();
  free(snake);

  return EXIT_SUCCESS;
}

void init_board() {
  for (size_t i = 0; i < ROWS; i++) {
    for (size_t j = 0; j < COLS; j++) {
      g_board[i][j] = malloc(sizeof(Node));

      if (g_board[i][j] == NULL) {
        perror("Memory allocation failed for board nodes.");
        exit(EXIT_FAILURE);
      }

      g_board[i][j]->type = '*';
      g_board[i][j]->row = i;
      g_board[i][j]->col = j;
    }
  }
}

void init_snake(Snake *snake) {
  snake->head = g_board[ROWS / 2][COLS / 2];
  snake->head->type = 'S';

  if (LEN > 1) {
    snake->tail = g_board[ROWS / 2][abs((COLS / 2) - LEN + 1)];
    snake->tail->type = 'S';
  }

  else {
    snake->tail = snake->head;
  }

  Node *temp = snake->tail;

  while (temp != NULL && temp != snake->head) {
    temp->next = g_board[temp->row][temp->col + 1];
    temp = g_board[temp->row][temp->col + 1];
  }
}

void add_food() {
  srand(time(NULL));

  size_t row = 0, col = 0;

  do {
    update_food(&row, &col);
  } while (g_board[row][col]->type == 'S');
}

void update_food(size_t *row, size_t *col) {
  *row = rand() % ROWS;
  *col = rand() % COLS;

  if (g_board[*row][*col]->type != 'S') {
    g_board[*row][*col]->type = 'F';
  }
}

void print_board() {

  set_text_padding();
  printf("Length: %zd\n\n", g_updated_len);

  set_text_padding();
  if (g_is_game_over) {
    printf("Game Over!");
  }

  printf("\n\n");

  for (size_t i = 0; i < ROWS; i++) {
    set_padding();

    for (size_t j = 0; j < COLS; j++) {
      printf("%c ", g_board[i][j]->type);
    }

    printf("\n");
  }

  printf("\n");
}

void set_padding() {
  for (size_t j = 0; j < TERM_PADDING; j++) {
    printf(" ");
  }
}

void set_text_padding() {
  for (size_t j = 0; j < TERM_PADDING + (COLS / 2) + 5; j++) {
    printf(" ");
  }
}

void check_direction() {
  if (kbhit()) {
    char direction = getch();

    switch (direction) {
    case 'w':
      if (g_down == 0) {
        g_up = 1;
        g_down = 0;
        g_right = 0;
        g_left = 0;
      }
      break;
    case 's':
      if (g_up == 0) {
        g_up = 0;
        g_down = 1;
        g_right = 0;
        g_left = 0;
      }
      break;
    case 'd':
      if (g_left == 0) {
        g_up = 0;
        g_down = 0;
        g_right = 1;
        g_left = 0;
      }
      break;
    case 'a':
      if (g_right == 0) {
        g_up = 0;
        g_down = 0;
        g_right = 0;
        g_left = 1;
      }
      break;
    }
  }
}

void move_snake(Snake *snake) {
  Node *next;
  int row_offset = 0, col_offset = 0;

  if (g_up) {
    row_offset = -1;
  } else if (g_down) {
    row_offset = 1;
  } else if (g_right) {
    col_offset = 1;
  } else if (g_left) {
    col_offset = -1;
  } else {
    return;
  }

  next = g_board[snake->head->row + row_offset][snake->head->col + col_offset];

  // Check for out-of-bounds
  if (next == NULL || snake->head->row + row_offset < 0 ||
      snake->head->row + row_offset >= ROWS ||
      snake->head->col + col_offset < 0 ||
      snake->head->col + col_offset >= COLS) {
    g_is_game_over = 1;
    return;
  }
  switch (next->type) {
  case '*':
    move_head(snake, row_offset, col_offset);
    move_tail(snake);
    break;

  case 'F':
    g_updated_len++;
    move_head(snake, row_offset, col_offset);
    add_food();
    break;
  default:
    g_is_game_over = 1;
    break;
  }
}

void move_head(Snake *snake, int row_offset, int col_offset) {
  snake->head->next =
      g_board[snake->head->row + (row_offset)][snake->head->col + (col_offset)];

  snake->head = snake->head->next;

  snake->head->type = 'S';
}

void move_tail(Snake *snake) {
  snake->tail->type = '*';

  if (snake->tail->next != NULL) {
    snake->tail = snake->tail->next;
    snake->tail->type = 'S';
  }
}
