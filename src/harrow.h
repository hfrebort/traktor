#pragma once

enum HARROW_DIRECTION {
      STOP
    , RIGHT
    , LEFT
};

class Harrow {

public:
    void move(HARROW_DIRECTION);

    Harrow();
    ~Harrow();

private:

    struct gpiod_chip *_chip;
    struct gpiod_line *_lineRight; 
    struct gpiod_line *_lineLeft; 
    struct gpiod_line *_lineMiddle; 

};