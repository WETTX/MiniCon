#define CLK 2
#define CS 3
#define DAT 4
#define B_RIGHT_PIN 5
#define B_DOWN_PIN 6
#define B_LEFT_PIN 7
#define B_UP_PIN 8
#define B_OK_PIN 9

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

// ===================================================================================================================================================

// ===================================================================================================================================================
// ЗМЕЙКА

int snake_snakeSpeed = 1000;  // время в миллисекундах, за которое змея проходит 1 клетку

Position positions_snake[64]{ Position(0, 0), Position(0, 0), Position(0, 0) };
Rotation rotate_snake = 2;
int tailScale_snake = 4;  // длина змеи без учета головы
int sTime_snake;

void update_snake() {
  // цикл игры
  b_right.tick();
  b_down.tick();
  b_left.tick();
  b_up.tick();
  b_ok.tick();

  if (b_right.press() && (rotate_snake - 1 == RIGHT_ROTATION || rotate_snake + 1 == RIGHT_ROTATION)) {
    rotate_snake = RIGHT_ROTATION;
  }

  if (b_down.press() && (rotate_snake - 1 == DOWN_ROTATION || rotate_snake + 1 == DOWN_ROTATION)) {
    rotate_snake = DOWN_ROTATION;
  }

  if (b_left.press() && (rotate_snake - 1 == LEFT_ROTATION || rotate_snake + 1 == LEFT_ROTATION)) {
    rotate_snake = LEFT_ROTATION;
  }

  if (b_up.press() && (rotate_snake - 1 == UP_ROTATION || rotate_snake + 1 == UP_ROTATION)) {
    rotate_snake = UP_ROTATION;
  }

  if (millis() - sTime_snake > snake_snakeSpeed) {
    for (int i = tailScale_snake; i > 0; i--) {
      positions_snake[i] = positions_snake[i - 1];
    }

    switch (rotate_snake.getR()) {
      case 0:
        positions_snake[0].y--;
        break;

      case 1:
        positions_snake[0].x++;
        break;

      case 2:
        positions_snake[0].y++;
        break;

      case 3:
        positions_snake[0].x--;
        break;
    }
    Serial.print(positions_snake[0].x);
    Serial.print(", ");
    Serial.println(positions_snake[0].y);

    sTime_snake = millis();
  }
}

void draw_snake() {
  // рисует голову и хвост нужной формы
  for (int i = 0; i <= tailScale_snake; i++) {
    mtx.dot(positions_snake[i].x, positions_snake[i].y);
  }
}
// ===================================================================================================================================================

// ===================================================================================================================================================
// ОСНОВНОЙ

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  mtx.begin();
  mtx.setBright(5);
  mtx.setRotation(3);
  mtx.update();
}

void loop() {
  mtx.clear();

  update_snake();
  draw_snake();

  mtx.update();
}
// ===================================================================================================================================================