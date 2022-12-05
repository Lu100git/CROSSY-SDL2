#include <SDL2/SDL.h>
#include <iostream>
#include <string>
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
using std::cout;
using std::endl;
using std::string;

class Sprite {
  private:
    SDL_Texture *m_texture;
    SDL_Rect m_src_rect;
    float speed = 8.0f;
    double angle = 0;

  public:
    SDL_Rect m_pos_rect;
    int textureWidth, textureHeight;
    Sprite(int, int, int, int, string, SDL_Renderer *);
    void draw(SDL_Renderer *);
    void update();
    void setSpeed(float);
    ~Sprite();
};

Sprite::Sprite(int x, int y, int w, int h, string file, SDL_Renderer *renderer) {
  SDL_Surface *surface = SDL_LoadBMP(file.c_str());
  if (surface == NULL) cout << "error loading texture from class: " << SDL_GetError() << endl;
  else{
    
    m_texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (m_texture == NULL) cout << "error, creating texture from class: " << SDL_GetError() << endl;
  }

  SDL_FreeSurface(surface);

  SDL_QueryTexture(m_texture, NULL, NULL, &textureWidth, &textureHeight);
  m_src_rect.x = 0;
  m_src_rect.y = 0;
  m_src_rect.w = textureWidth;
  m_src_rect.h = textureHeight;
  m_pos_rect.x = x;
  m_pos_rect.y = y;
  m_pos_rect.w = w;
  m_pos_rect.h = h;
}

void Sprite::draw(SDL_Renderer *renderer) { SDL_RenderCopyEx(renderer, m_texture, &m_src_rect, &m_pos_rect, angle,nullptr, SDL_FLIP_NONE); }

void Sprite::update() {
  m_pos_rect.x += speed;
  angle += 10;
  if (angle > 359) angle = 0;
  
  if (m_pos_rect.x > WINDOW_WIDTH - m_pos_rect.w) { speed = -abs(speed);} 
  else if (m_pos_rect.x < 10) { speed = abs(speed);}
}

void Sprite::setSpeed(float s) { speed = s; }

Sprite::~Sprite() {
  SDL_DestroyTexture(m_texture);
  m_texture = nullptr;
}

class Player {
  private:
    SDL_Texture *m_texture;
    SDL_Rect m_src_rect;
    float speed = 5.0f;
    bool active = false;
    float frameCount = 0.0f;

  public:
    SDL_Rect m_pos_rect;
    int textureWidth, textureHeight;
    Player(int, int, int, int, int, int, string, SDL_Renderer *);
    Player();
    SDL_Texture *loadTexture(string, SDL_Renderer *);
    bool collidesWith(Sprite &);
    void draw(SDL_Renderer *);
    void update(const Uint8 *, float);
    void showPos();
    ~Player();
};

Player::Player(int x, int y, int w, int h, int frameX, int frameY, string file, SDL_Renderer *renderer) {
  m_texture = loadTexture(file, renderer);
  SDL_QueryTexture(m_texture, NULL, NULL, &textureWidth, &textureHeight);
  m_src_rect.x = (textureWidth / frameX) * 1;
  m_src_rect.y = (textureHeight / frameY) * 2;
  m_src_rect.w = textureWidth / frameX;
  m_src_rect.h = textureHeight / frameY;
  m_pos_rect.x = x;
  m_pos_rect.y = y;
  m_pos_rect.w = w;
  m_pos_rect.h = h;
}

SDL_Texture *Player::loadTexture(string file, SDL_Renderer *renderer) {
  SDL_Texture *texture = nullptr;
  SDL_Surface *surface = SDL_LoadBMP(file.c_str());
  if (surface == NULL) cout << "error loading texture from class: " << SDL_GetError() << endl;
  else {
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) cout << "error, creating texture from class: " << SDL_GetError() << endl;
  }

  SDL_FreeSurface(surface);
  return texture;
}

bool Player::collidesWith(Sprite &otherSprite) {
  if ((m_pos_rect.x + m_pos_rect.w < otherSprite.m_pos_rect.x) | (m_pos_rect.x > otherSprite.m_pos_rect.x + otherSprite.m_pos_rect.w)) { return false;} 
  else if ((m_pos_rect.y + m_pos_rect.h < otherSprite.m_pos_rect.y) | (m_pos_rect.y > otherSprite.m_pos_rect.y + otherSprite.m_pos_rect.h)) { return false;} 
  else { return true; }
}

void Player::draw(SDL_Renderer *renderer) { SDL_RenderCopyEx(renderer, m_texture, &m_src_rect, &m_pos_rect, 0, nullptr, SDL_FLIP_NONE); }

void Player::update(const Uint8 *keystate, float deltaTime) {
  // preventing out of bounds
  if (m_pos_rect.y < 10) { m_pos_rect.y = 10;} 
  else if (m_pos_rect.y > 420) { m_pos_rect.y = 420; }

  if (keystate[SDL_SCANCODE_UP]) {
    m_pos_rect.y -= speed;
    m_src_rect.y = (textureHeight / 4) * 0;
    active = true;
  }

  else if (keystate[SDL_SCANCODE_DOWN]) {
    m_pos_rect.y += speed;
    m_src_rect.y = (textureHeight / 4) * 2;
    active = true;
  }

  else {
    active = false;
    m_pos_rect.x = m_pos_rect.x;
    m_pos_rect.y = m_pos_rect.y;
    m_src_rect.y = (textureHeight / 4) * 2;
  }

  if (active) {
    frameCount += deltaTime;
    if (frameCount >= 0.25f) {
      frameCount = 0;
      m_src_rect.x %= textureWidth - m_src_rect.w;
      m_src_rect.x += m_src_rect.w;
    }
  }
}

void Player::showPos() {
  cout << "PlayerX: " << m_pos_rect.x << " PlayerY: " << m_pos_rect.y << endl;
}

Player::~Player() {
  SDL_DestroyTexture(m_texture);
  m_texture = nullptr;
}

class Game {
  private:
    int level;
    int lives;
    bool died = false;
    bool win = false;

  public:
    Game(SDL_Window *, SDL_Renderer *, int, int);
    void exit();
};

Game::Game(SDL_Window *window, SDL_Renderer *renderer, int lvl, int liv) {
  level = lvl;
  lives = liv;

  SDL_Event event;
  const Uint8 *keystate;
  bool run = true;

  int currentTime = 0;
  int previousTime = 0;
  float deltaTime = 0.0f;

  // creating the sprites
  Player Player(294, 400, 50, 50, 3, 4, "sprites/healer.bmp", renderer);
  Sprite background(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, "sprites/background.bmp",renderer);
  Sprite enemy(12, 350, 40, 40, "sprites/enemy.bmp", renderer);
  Sprite enemy2(600, 85, 50, 50, "sprites/enemy.bmp", renderer);
  Sprite enemy3(600, 220, 40, 40, "sprites/enemy.bmp", renderer);
  Sprite enemy4(12, 85, 50, 50, "sprites/enemy.bmp", renderer);
  Sprite chest(294, 12, 50, 50, "sprites/treasure.bmp", renderer);
  Sprite heart(50, 20, 50,50, "sprites/heart.bmp", renderer);
  enemy3.setSpeed(14.0);
  enemy4.setSpeed(18.0);

  while (run) {
    // events
    while (SDL_PollEvent(&event) != 0) {
      if (event.type == SDL_QUIT)
        run = false;
      else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
          run = false;
        }
      }
    }

    // animation and delta time
    previousTime = currentTime;
    currentTime = SDL_GetTicks();
    deltaTime = (currentTime - previousTime) / 1000.0f;

    // key press
    keystate = SDL_GetKeyboardState(NULL);

    // movement
    Player.update(keystate, deltaTime);
    enemy.update();
    enemy2.update();
    if (level > 1)
      enemy3.update();
    if (level > 2)
      enemy4.update();

    // render
    SDL_RenderClear(renderer);

    background.draw(renderer);
    chest.draw(renderer);
    enemy.draw(renderer);
    enemy2.draw(renderer);
    if (level > 1) enemy3.draw(renderer);
    if (level > 2) enemy4.draw(renderer);
    Player.draw(renderer);
    for(int i = 0; i < lives; i++){
      heart.m_pos_rect.x = (i * 52);
      heart.draw(renderer);
    }

    SDL_RenderPresent(renderer);

    if (Player.collidesWith(enemy) || Player.collidesWith(enemy2) || Player.collidesWith(enemy3) || Player.collidesWith(enemy4)) {
      cout << "COLLISION!" << endl;
      died = true;
      run = false;
    }
    if (Player.collidesWith(chest)) {
      cout << "chest collision " << endl;
      win = true;
      run = false;
    }
  }

  if (died) {
    if (lives <= 1) {
      cout << "game over" << endl;
    } else {
      lives--;
      died = false;
      SDL_Delay(1000);
      Game new_game(window, renderer, level, lives);
    }
  }

  if (win) {
    cout << "current level " << level << endl;
    if (level >= 3) {
      cout << "YOU WIN!" << endl;
    } else {
      level++;
      win = false;
      SDL_Delay(1000);
      Game new_game(window, renderer, level, lives);
    }
  }
}

int main(int argc, char *argv[]) {
  // mandatory, window, and renderer,
  SDL_Window *window;
  SDL_Renderer *renderer;

  // INITIALIZE VIDEO AND CREATES THE WINDOW AND RENDERER
  if (SDL_Init(SDL_INIT_VIDEO) < 0) cout << "video init error: " << SDL_GetError() << endl;
  else {
    window = SDL_CreateWindow("SDL2 TEMPLATE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) cout << "window creation error: " << SDL_GetError() << endl;
    else {

      renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
      Game main_game(window, renderer, 1, 3);
    }
  }
  // free the texture, and quit the SDL
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  window = nullptr;
  renderer = nullptr;
  SDL_Quit();
  return 0;
}
