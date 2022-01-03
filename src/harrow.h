#pragma once

enum HARROW_DIRECTION {
      STOP
    , RIGHT
    , LEFT
};

class Harrow {

public:

    void move(HARROW_DIRECTION);

    int isLifted();
    int isZweitRechts();
    int isZweitLinks();

    Harrow();
    ~Harrow();

private:

    struct gpiod_chip *_chip;

    struct gpiod_line *_lineDummyMiddle; 

    struct gpiod_line *_lineHydraulicRight; 
    struct gpiod_line *_lineHydraulicLeft; 

    struct gpiod_line *_lineSensorUp;
    struct gpiod_line *_lineSensorRight;
    struct gpiod_line *_lineSensorLeft;

};