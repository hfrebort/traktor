#include <gpiod.h>
#include <exception>

#include "harrow.h"

gpiod_line* open_pin(gpiod_chip* chip, unsigned int gpio_number)
{
    gpiod_line* line = gpiod_chip_get_line(chip, gpio_number);
    if ( line == NULL ) {
        throw std::runtime_error( "gpiod_chip_get_line(" + std::to_string(gpio_number) + ")");
    }
    return line;
}

void set_pin_to_output(gpiod_line* line)
{
    if ( gpiod_line_request_output(line, "BumsMichl", 0) == -1 ) {
        throw std::runtime_error( "gpiod_line_request_output" );
    }
}

Harrow::Harrow()
{
    const std::string chipname("/dev/gpiochip0");

    if ( (_chip = gpiod_chip_open(chipname.c_str())) == NULL) {
        perror("chip_open failed");
        throw std::runtime_error("gpiod_chip_open(" + chipname + ")");
    }

    _lineRight  = open_pin(_chip, 23);
    _lineLeft   = open_pin(_chip, 24);
    _lineMiddle = open_pin(_chip, 14);

    set_pin_to_output(_lineRight);
    set_pin_to_output(_lineLeft);
    set_pin_to_output(_lineMiddle);

}

Harrow::~Harrow()
{
    gpiod_line_release(_lineRight);
    gpiod_line_release(_lineLeft);
    gpiod_line_release(_lineMiddle);

    gpiod_chip_close(_chip);
}

void Harrow::move(HARROW_DIRECTION direction)
{
    gpiod_line_set_value(_lineRight,  0);
    gpiod_line_set_value(_lineLeft,   0);
    gpiod_line_set_value(_lineMiddle, 0);

    gpiod_line* line = nullptr;
    switch (direction) {
        case HARROW_DIRECTION::RIGHT:  line = _lineRight;  break;
        case HARROW_DIRECTION::LEFT:   line = _lineLeft;   break;
        case HARROW_DIRECTION::STOP:   line = _lineMiddle; break;
    }
    if ( line != nullptr) {
        gpiod_line_set_value(line, 1);
    }

}
