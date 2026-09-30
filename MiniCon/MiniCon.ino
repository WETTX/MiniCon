#define CLK 2
#define CS 3
#define DAT 4
#define B_RIGHT_PIN 5
#define B_DOWN_PIN 6
#define B_LEFT_PIN 7
#define B_UP_PIN 8
#define B_OK_PIN 9
#define RANDOM_PIN A0 // пустой пин для сида рандома

#define UB_DEB_TIME 50     // дебаунс (до 255)
#define UB_HOLD_TIME 0     // время до перехода в состояние "удержание"
#define UB_STEP_TIME 0     // время до перехода в состояние "импульсное удержание"
#define UB_STEP_PRD 200    // период импульсов
#define UB_CLICK_TIME 500  // ожидание кликов
#include <uButton.h>
uButton b_right(B_RIGHT_PIN);
uButton b_down(B_DOWN_PIN);
uButton b_left(B_LEFT_PIN);
uButton b_up(B_UP_PIN);
uButton b_ok(B_OK_PIN);

#include <GyverMAX7219.h>
MAX7219<1, 1, CS, DAT, CLK> mtx;

#define UP_ROTATION Rotation(0)
#define RIGHT_ROTATION Rotation(1)
#define DOWN_ROTATION Rotation(2)
#define LEFT_ROTATION Rotation(3)
#define VOID_POS Position(-1, -1) // позиция, которая когда на объекте, означает что объекта нет
#define MAX_SCALE_snake 64

// ===================================================================================================================================================
// ОБЩЕЕ

class Position {
  public:
    int x;
    int y;

    Position() {
      x = 0;
      y = 0;
    }

    Position(int x_m, int y_m) {
      x = x_m;
      y = y_m;
    }

    bool operator==(Position other) {
      return ((*this).x == other.x) && ((*this).y == other.y);
    }
};

class Rotation {
  public:
    int r;

    Rotation() {
      setR(0);
    }

    Rotation(int newR) {
      setR(newR);
    }

    bool operator==(Rotation other) const {
      return (*this).getR() == other.getR();
    }

    Rotation& operator=(int newR) {
      setR(newR);
      return *this;
    }

    Rotation operator+(int addend) const {
      return Rotation(r + addend);
    }

    Rotation operator-(int addend) const {
      return Rotation(r - addend);
    }

    Rotation& operator++() {
      setR(r + 1);
      return *this;
    }

    Rotation operator++(int) {
      Rotation temp = *this;
      ++(*this);
      return temp;
    }

    Rotation& operator--() {
      setR(r - 1);
      return *this;
    }

    Rotation operator--(int) {
      Rotation temp = *this;
      --(*this);
      return temp;
    }

    int getR() const {
      return r;
    }

  private:
    void setR(int newR) {
      r = validateRotation(newR);
    }

    int validateRotation(int rotation) {
      // делает int значение в пределах [0; 3]

      while (true) {
        if (0 <= rotation && rotation <= 3) {
          return rotation;
        }

        rotation -= 4;
      }
    }
};

void copyList(Position* l, Position* src, int scale){
  for (int i = 0; i < scale; i++){
    l[i] = src[i];
  }
}

void gameOver() {
  Serial.println("game over!");
}

// ===================================================================================================================================================

// ===================================================================================================================================================
// ЗМЕЙКА

int snakeSpeed_snake = 1000;  // время в миллисекундах, за которое змея проходит 1 клетку

Position snakePositions_snake[MAX_SCALE_snake];
Position applePosition_snake;
Rotation rotate_snake = 2;
int tailScale_snake = 3;  // длина змеи без учета головы
int applesQuantity_snake = 1;
unsigned long sTime_snake;
bool isRotatedAlready_snake; // чтобы нельзя было повернуть несколько раз за 1 тик змеи

void start_snake() {
  copyList(snakePositions_snake, new Position[64]{Position(2, 3), Position(2, 2), Position(2, 1), Position(2, 0)}, MAX_SCALE_snake);
  rotate_snake = 2;
  tailScale_snake = 3;
  Position applePosition_snake = Position(random(0, 8), random(0, 8));

}

void update_snake() {
  // цикл игры

  b_right.tick();
  b_down.tick();
  b_left.tick();
  b_up.tick();
  b_ok.tick();

  //---------------------------------------------------------------------------------------------------------

  if (!isRotatedAlready_snake) {
    if (b_right.press() && (rotate_snake - 1 == RIGHT_ROTATION || rotate_snake + 1 == RIGHT_ROTATION)) {
      rotate_snake = RIGHT_ROTATION;
      isRotatedAlready_snake = true;
    }

    else if (b_down.press() && (rotate_snake - 1 == DOWN_ROTATION || rotate_snake + 1 == DOWN_ROTATION)) {
      rotate_snake = DOWN_ROTATION;
      isRotatedAlready_snake = true;
    }

    else if (b_left.press() && (rotate_snake - 1 == LEFT_ROTATION || rotate_snake + 1 == LEFT_ROTATION)) {
      rotate_snake = LEFT_ROTATION;
      isRotatedAlready_snake = true;
    }

    else if (b_up.press() && (rotate_snake - 1 == UP_ROTATION || rotate_snake + 1 == UP_ROTATION)) {
      rotate_snake = UP_ROTATION;
      isRotatedAlready_snake = true;
    }
    if (b_ok.press()){
      start_snake();  
    }
  }
  //---------------------------------------------------------------------------------------------------------
  if (millis() - sTime_snake > snakeSpeed_snake) {
    bool nextCellIsApple_snake;

    switch (rotate_snake.getR()) {
      case 0:
        nextCellIsApple_snake = (Position(snakePositions_snake[0].x, snakePositions_snake[0].y - 1) == applePosition_snake);
        break;

      case 1:
        nextCellIsApple_snake = (Position(snakePositions_snake[0].x + 1, snakePositions_snake[0].y) == applePosition_snake);
        break;

      case 2:
        nextCellIsApple_snake = (Position(snakePositions_snake[0].x, snakePositions_snake[0].y + 1) == applePosition_snake);
        break;

      case 3:
        nextCellIsApple_snake = (Position(snakePositions_snake[0].x - 1, snakePositions_snake[0].y) == applePosition_snake);
        break;
    }
    //---------------------------------------------------------------------------------------------------------
    if (nextCellIsApple_snake) {
      applePosition_snake = VOID_POS;

      Serial.println("apple");

      for (int i = tailScale_snake + 1; i > 0; i--) {
        snakePositions_snake[i] = snakePositions_snake[i - 1];
      }

      switch (rotate_snake.getR()) {
        case 0:
          snakePositions_snake[0] = Position(snakePositions_snake[1].x, snakePositions_snake[1].y - 1);
          break;

        case 1:
          snakePositions_snake[0] = Position(snakePositions_snake[1].x + 1, snakePositions_snake[1].y);
          break;

        case 2:
          snakePositions_snake[0] = Position(snakePositions_snake[1].x, snakePositions_snake[1].y + 1);
          break;

        case 3:
          snakePositions_snake[0] = Position(snakePositions_snake[1].x - 1, snakePositions_snake[1].y);
          break;
      }

      tailScale_snake++;
    }
    //---------------------------------------------------------------------------------------------------------
    else {

      for (int i = tailScale_snake; i > 0; i--) {
        snakePositions_snake[i] = snakePositions_snake[i - 1];
      }

      switch (rotate_snake.getR()) {
        case 0:
          snakePositions_snake[0].y--;
          break;

        case 1:
          snakePositions_snake[0].x++;
          break;

        case 2:
          snakePositions_snake[0].y++;
          break;

        case 3:
          snakePositions_snake[0].x--;
          break;
      }
    }

    //---------------------------------------------------------------------------------------------------------

    if (applePosition_snake == VOID_POS) {
      while (true) {
        bool aproved = true;

        applePosition_snake = Position(random(0, 8), random(0, 8));
        for (int i = 0; i <= tailScale_snake; i++) {
          if (applePosition_snake == snakePositions_snake[i]) {
            aproved = false;
            break;
          }
        }

        if (aproved) {
          break;
        }
      }
    }

    //---------------------------------------------------------------------------------------------------------

    bool isCrashed = false;

    for (int i = 1; i <= tailScale_snake; i++) {
      if (snakePositions_snake[0] == snakePositions_snake[i]) {
        isCrashed = true;
        Serial.println(i);
        break;
      }
    }

    if (snakePositions_snake[0].x < 0 || 7 < snakePositions_snake[0].x || snakePositions_snake[0].y < 0 || 7 < snakePositions_snake[0].y) {
      isCrashed = true;
    }

    if (isCrashed) {
      gameOver();
    }

    //---------------------------------------------------------------------------------------------------------
    isRotatedAlready_snake = false;
    sTime_snake = millis();

  }
}

void draw_snake() {
  // рисует яблоки, голову и хвост нужной формы
  for (int i = 0; i <= tailScale_snake; i++) {
    mtx.dot(snakePositions_snake[i].x, snakePositions_snake[i].y);
  }
  mtx.dot(applePosition_snake.x, applePosition_snake.y);
}
// ===================================================================================================================================================

// ===================================================================================================================================================
// ОСНОВНОЙ

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  randomSeed(analogRead(RANDOM_PIN));
  randomSeed(random(random(1000)));

  mtx.begin();
  mtx.setBright(5);
  mtx.setRotation(3);
  mtx.update();

  start_snake();
}

void loop() {
  mtx.clear();

  update_snake();
  draw_snake();

  mtx.update();
}
// ===================================================================================================================================================