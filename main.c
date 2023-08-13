#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include <SDL2/SDL.h>

const int WINDOW_INIT_WIDTH  = 1020;
const int WINDOW_INIT_HEIGHT = 680;

#define     FIELD_WIDTH  10
#define     FIELD_HEIGHT 20
const float FIELD_ASPECT_RATIO = (float)(FIELD_HEIGHT)/(float)(FIELD_WIDTH);
const float FIELD_HEIGHT_PERCENT = 0.8;

const uint64_t MOVE_TIME_MS = 800;

const SDL_Color RED                = {255, 0  , 0  , 255};
const SDL_Color BLUE               = {0  , 0  , 255, 255};
const SDL_Color CYAN               = {0  , 255, 255, 255};
const SDL_Color PINK               = {255, 0  , 127, 255};
const SDL_Color BLACK              = {0  , 0  , 0  , 255};
const SDL_Color WHITE              = {255, 255, 255, 255};
const SDL_Color GREEN              = {0  , 255, 0  , 255};
const SDL_Color AZURE              = {0  , 127, 255, 255};
const SDL_Color ORANGE             = {255, 127, 0  , 255};
const SDL_Color YELLOW             = {255, 255, 0  , 255};
const SDL_Color VIOLET             = {127, 0  , 255, 255};
const SDL_Color MAGENTA            = {255, 0  , 255, 255};
const SDL_Color SPRING_GREEN       = {0  , 255, 127, 255};
const SDL_Color CHARTEUSE_GREEN    = {127, 255, 0  , 255};

const SDL_Color color_pool[12] = {
  RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, ORANGE, PINK, CHARTEUSE_GREEN, SPRING_GREEN, AZURE, VIOLET};

void SetRenderDrawColor(SDL_Renderer* renderer, SDL_Color color) {
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);}

SDL_Point add(SDL_Point l, SDL_Point r){
  return (SDL_Point){l.x + r.x, l.y + r.y};}

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

SDL_Window*   window   = NULL;
SDL_Renderer* renderer = NULL;

SDL_Rect field_rect;

SDL_Event event;
uint64_t last_move_time;

bool running = false;

size_t score = 0;

typedef struct{
  size_t state;
  SDL_Point field_position;
  SDL_Point tile_positions[8][4]; // for each state
  SDL_Color color;
} Tetromino;

Tetromino current_tetromino;

bool taken_positions[FIELD_HEIGHT][FIELD_WIDTH];
SDL_Color field_colors[FIELD_HEIGHT][FIELD_WIDTH];
  
const Tetromino tetromino_pool[5] = {
  {0, {FIELD_WIDTH / 2 - 1, 0}, {
      {{0, 2}, {1, 2}, {2, 2}, {3, 2}},
      {{2, 0}, {2, 1}, {2, 2}, {2, 3}},
      {{0, 2}, {1, 2}, {2, 2}, {3, 2}},
      {{2, 0}, {2, 1}, {2, 2}, {2, 3}},

      {{0, 2}, {1, 2}, {2, 2}, {3, 2}},
      {{2, 0}, {2, 1}, {2, 2}, {2, 3}},
      {{0, 2}, {1, 2}, {2, 2}, {3, 2}},
      {{2, 0}, {2, 1}, {2, 2}, {2, 3}}
 
    }, BLACK},

  {0, {FIELD_WIDTH / 2 - 1, 0}, {
      {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
      {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
      {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
      {{0, 0}, {0, 1}, {1, 0}, {1, 1}},

      {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
      {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
      {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
      {{0, 0}, {0, 1}, {1, 0}, {1, 1}}
    }, BLACK},

  {0, {FIELD_WIDTH / 2 - 1, 0}, {
      {{0, 1}, {1, 1}, {2, 1}, {2, 2}},
      {{1, 0}, {1, 1}, {1, 2}, {0, 2}},
      {{0, 0}, {0, 1}, {1, 1}, {2, 1}},
      {{1, 2}, {1, 1}, {1, 0}, {2, 0}},

      {{0, 1}, {1, 1}, {2, 1}, {2, 0}},
      {{1, 0}, {1, 1}, {1, 2}, {0, 0}},
      {{0, 2}, {0, 1}, {1, 1}, {2, 1}},
      {{1, 2}, {1, 1}, {1, 0}, {2, 2}}
    }, BLACK},
  
  {0, {FIELD_WIDTH / 2 - 1, 0}, {
      {{0, 1}, {1, 1}, {2, 1}, {1, 2}},
      {{1, 0}, {1, 1}, {1, 2}, {0, 1}},
      {{0, 1}, {1, 1}, {2, 1}, {1, 0}},
      {{1, 0}, {1, 1}, {1, 2}, {2, 1}},
      
      {{0, 1}, {1, 1}, {2, 1}, {1, 0}},
      {{1, 0}, {1, 1}, {1, 2}, {0, 1}},
      {{0, 1}, {1, 1}, {2, 1}, {1, 2}},
      {{1, 0}, {1, 1}, {1, 2}, {2, 1}}
    }, BLACK},

  {0, {FIELD_WIDTH / 2 - 1, 0}, {
      {{0, 1}, {1, 1}, {1, 2}, {2, 2}},
      {{2, 0}, {2, 1}, {1, 1}, {1, 2}},
      {{0, 1}, {1, 1}, {1, 2}, {2, 2}},
      {{2, 0}, {2, 1}, {1, 1}, {1, 2}},
      
      {{0, 2}, {1, 1}, {1, 2}, {2, 1}},
      {{1, 0}, {2, 1}, {1, 1}, {2, 2}},
      {{0, 2}, {1, 1}, {1, 2}, {2, 1}},
      {{1, 0}, {2, 1}, {1, 1}, {2, 2}}
    }, BLACK}
};


void render_tetromino(Tetromino* t);
void render_tile(SDL_Point p, SDL_Color c);
void render_placed();

SDL_Scancode scancode_pool[7] = {
  SDL_SCANCODE_RIGHT,
  SDL_SCANCODE_LEFT,
  SDL_SCANCODE_DOWN,
  SDL_SCANCODE_Z,
  SDL_SCANCODE_X,
  SDL_SCANCODE_C,
  SDL_SCANCODE_V
};

void move(Tetromino* t, void (*move_function)(Tetromino*));
void move_right(Tetromino* t);
void move_left(Tetromino* t);
void move_down(Tetromino* t);
void rotate_clockwise(Tetromino* t);
void rotate_counterclockwise(Tetromino* t);
void mirror_vertical(Tetromino* t);
void mirror_horizontal(Tetromino* t);

bool is_line_full(size_t line);
bool is_fitted(Tetromino* t);
void place(Tetromino* t);
void remove_line(size_t line);

void init();
void input();
void update();
void render();
void quit();

int main(){
  init();

  while(running){
    input();
    update();
    render();
  }

  quit();
}

bool is_line_full(size_t line){
  for(size_t i = 0; i < FIELD_WIDTH; i++)
    if(!taken_positions[line][i]) return false;

  return true;
}

void move_line_down(size_t line){
  for(size_t i = 0; i < FIELD_WIDTH; i++){
    taken_positions[line + 1][i] = taken_positions[line][i];
    field_colors[line + 1][i] = field_colors[line][i];
  }
}

void remove_line(size_t line){
  for(size_t j = line - 1; j != 0; j--)
    move_line_down(j);
}

bool is_fitted(Tetromino* t){
  for(size_t i = 0; i < 4; i++){
    SDL_Point p = add(t->field_position, t->tile_positions[t->state][i]);
    if(p.y == FIELD_HEIGHT - 1) return true;
    if(taken_positions[p.y + 1][p.x]) return true;
  }
  
  return false;
}

void place(Tetromino* t){
  for(size_t i = 0; i < 4; i++){
    SDL_Point p = add(t->field_position, t->tile_positions[t->state][i]);
    taken_positions[p.y][p.x] = true;
    field_colors[p.y][p.x] = t->color;
  }
}

bool is_valid_position(Tetromino* t){
  for(size_t i = 0; i < 4; i++){
    SDL_Point p = add(t->field_position, t->tile_positions[t->state][i]);
    if(p.x < 0 || p.x >= FIELD_WIDTH || p.y < 0 || p.y >= FIELD_HEIGHT) return false;
    if(taken_positions[p.y][p.x]) return false;
  }
  
  return true;
}

void move(Tetromino* t, void (*move_function)(Tetromino*)){
  Tetromino t0 = *t; 

  move_function(t);

  if(!is_valid_position(t)) *t = t0;
}

void move_right(Tetromino* t){
  Tetromino t0 = *t;
  
  int right_most = 0;

  for(size_t i = 0; i < 4; i++)
    right_most = max(right_most, (t->field_position.x +
			  t->tile_positions[t->state][i].x));

  t->field_position.x++;

  if(!is_valid_position(t)) *t = t0;
}

void move_left(Tetromino* t){
  int left_most = FIELD_WIDTH;

  for(size_t i = 0; i < 4; i++)
    left_most = min(left_most, (t->field_position.x +
				t->tile_positions[t->state][i].x));
  t->field_position.x--;
}

void move_down(Tetromino* t){
  int down_most = 0;

  for(size_t i = 0; i < 4; i++)
    down_most = max(down_most, (t->field_position.y +
				t->tile_positions[t->state][i].y));

  t->field_position.y++;
}

void rotate_clockwise(Tetromino* t){
  if(t->state < 4) t->state = (t->state + 1) % 4;
  else t->state = 4 + (t->state - 1) % 4;
}

void rotate_counterclockwise(Tetromino* t){
  if(t->state < 4) t->state = (t->state + 3) % 4;
  else t->state = 4 + (t->state + 1) % 4;
}

void mirror_horizontal(Tetromino* t){
  t->state = (t->state + 4) % 8;
}

void mirror_vertical(Tetromino* t){
  if(t->state % 4 < 2) t->state = (t->state + 6) % 8;
  else t->state = (t->state + 2) % 8;
}


void render_tetromino(Tetromino* t){
  
  for(size_t i = 0; i < 4; i++)
    render_tile(add(t->field_position, t->tile_positions[t->state][i]), t->color);
  
}

Tetromino get_random_tetromino(){
  srand(time(NULL));

  Tetromino t = tetromino_pool[rand() % 5];
  t.color = color_pool[rand() % 12];
  t.state = rand() % 8;

  return t;
}

void clear_field(){
  for(size_t i = 0; i < FIELD_HEIGHT; i++)
    for(size_t j = 0; j < FIELD_WIDTH; j++){
      taken_positions[i][j] = false;
      field_colors[i][j] = WHITE;
    }
}


void render_placed(){

  for(size_t i = 0; i < FIELD_HEIGHT; i++)
    for(size_t j = 0; j < FIELD_WIDTH; j++){
      if(!taken_positions[i][j]) continue;
      render_tile((SDL_Point){j, i}, field_colors[i][j]);
    }
}

void render_tile(SDL_Point p, SDL_Color c){
  float tile_size = field_rect.h / (float)FIELD_HEIGHT;

  SetRenderDrawColor(renderer, c);

  SDL_Rect r = {
    field_rect.x + p.x * tile_size,
    field_rect.y + p.y * tile_size,
    tile_size + 1, tile_size + 1};

  SDL_RenderFillRect(renderer, &r);
}


void init(){
  SDL_Init(SDL_INIT_EVERYTHING);

  running = true;
  
  SDL_CreateWindowAndRenderer(WINDOW_INIT_WIDTH, WINDOW_INIT_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);

  for(size_t i = 0; i < FIELD_HEIGHT; i++)
    for(size_t j = 0; j < FIELD_WIDTH; j++){
      taken_positions[i][j] = false;
      field_colors[i][j] = WHITE;
    }

  current_tetromino = get_random_tetromino();
  last_move_time = SDL_GetTicks64();
}

void input(){
  while(SDL_PollEvent(&event)){
    switch(event.type){
    case SDL_QUIT: running = false; break;
    }
  }
}

void update(){

  uint8_t* key_scancodes = (uint8_t*)SDL_GetKeyboardState(NULL);

  if(key_scancodes[SDL_SCANCODE_RIGHT] == 1) move(&current_tetromino, move_right);
  if(key_scancodes[SDL_SCANCODE_LEFT] == 1)  move(&current_tetromino, move_left); 
  if(key_scancodes[SDL_SCANCODE_DOWN] == 1)  move(&current_tetromino, move_down);
  if(key_scancodes[SDL_SCANCODE_X] == 1)     move(&current_tetromino, rotate_clockwise);
  if(key_scancodes[SDL_SCANCODE_Z] == 1)     move(&current_tetromino, rotate_counterclockwise);
  if(key_scancodes[SDL_SCANCODE_V] == 1)     move(&current_tetromino, mirror_vertical);
  if(key_scancodes[SDL_SCANCODE_C] == 1)     move(&current_tetromino, mirror_horizontal);

  for(size_t i = 0; i < 7; i++)
    key_scancodes[scancode_pool[i]] = 0;
  

  if(SDL_GetTicks64() - last_move_time > MOVE_TIME_MS){
    last_move_time = SDL_GetTicks64();
    move(&current_tetromino, move_down);
  }

      
  for(size_t line = FIELD_HEIGHT - 1; line != 1; line--){
    if(is_line_full(line)){
      remove_line(line);
      score++;
    }
  }
  
  if(is_fitted(&current_tetromino)){
    place(&current_tetromino);

    for(size_t i = 0; i < 4; i++)
      for(size_t j = 0; j < FIELD_WIDTH; j++)
	if(taken_positions[i][j]){
	  score = 0;
	  clear_field();
	}
    
    current_tetromino = get_random_tetromino();

  }
}

void quit(){
  printf("\n");
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void render(){
  SetRenderDrawColor(renderer, WHITE);
  SDL_RenderClear(renderer);
  
  int window_width, window_height;
  SDL_GetWindowSize(window, &window_width, &window_height);

  field_rect.h = FIELD_HEIGHT_PERCENT * window_height;
  field_rect.w = field_rect.h / FIELD_ASPECT_RATIO; 
  field_rect.y = 0.5 * (window_height - field_rect.h);
  field_rect.x = 0.5 * (window_width - field_rect.w);
    
  render_tetromino(&current_tetromino);
  render_placed();

  SetRenderDrawColor(renderer, BLACK);
  SDL_RenderDrawRect(renderer, &field_rect);

  SDL_RenderPresent(renderer);

  printf("\rScore: %ld                               ", score);
}
