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

void set_pin_to_input(gpiod_line* line)
{
    if ( gpiod_line_request_input(line, "BumsMichl") == -1 ) {
        throw std::runtime_error( "gpiod_line_request_imput" );
    }
}

Harrow::Harrow()
{
    const std::string chipname("/dev/gpiochip0");

    if ( (_chip = gpiod_chip_open(chipname.c_str())) == NULL) {
        perror("chip_open failed");
        throw std::runtime_error("gpiod_chip_open(" + chipname + ")");
    }
    //
    // setup lines/pins for hydraulic
    //
    _lineHydraulicRight  = open_pin(_chip, 23);
    _lineHydraulicLeft   = open_pin(_chip, 24);

    set_pin_to_output(_lineHydraulicRight);
    set_pin_to_output(_lineHydraulicLeft);
    //
    // setup lines/pins for "ReCenter sensors"
    //
    _lineSensorUp    = open_pin(_chip, 17);     // when harrow is lifted up
    _lineSensorRight = open_pin(_chip, 22);
    _lineSensorLeft  = open_pin(_chip, 27);

    set_pin_to_input(_lineSensorUp);
    set_pin_to_input(_lineSensorRight);
    set_pin_to_input(_lineSensorLeft);
    //
    // turn hydraulic movement "off"
    //
    gpiod_line_set_value(_lineHydraulicRight,  0);
    gpiod_line_set_value(_lineHydraulicLeft,   0);

}

Harrow::~Harrow()
{
    gpiod_line_release(_lineHydraulicRight);
    gpiod_line_release(_lineHydraulicLeft);

    gpiod_line_release(_lineSensorUp);
    gpiod_line_release(_lineSensorLeft);
    gpiod_line_release(_lineSensorRight);

    gpiod_chip_close(_chip);
}

void line_set_value(struct gpiod_line *line, const int value, const char* linename)
{
    if ( gpiod_line_set_value(line,  value) == -1 )
    {
        fprintf(stderr, "E: line_set_value. (%s)\n", linename);
    }
}

void Harrow::move(HARROW_DIRECTION direction)
{
    const std::lock_guard<std::mutex> lock(this->_mutex_move);

    if (direction == HARROW_DIRECTION::STOP)
    {
        line_set_value(_lineHydraulicRight,  0, "HydraulicRight");
        line_set_value(_lineHydraulicLeft,   0, "HydraulicLeft");
    }
    else if (direction == HARROW_DIRECTION::LEFT)
    {
        line_set_value(_lineHydraulicRight,  0, "HydraulicRight");
        line_set_value(_lineHydraulicLeft,   1, "HydraulicLeft");
    }
    else if (direction == HARROW_DIRECTION::RIGHT)
    {
        line_set_value(_lineHydraulicLeft,   0, "HydraulicLeft");
        line_set_value(_lineHydraulicRight,  1, "HydraulicRight");
    }
}

int line_get_value(struct gpiod_line *line, const char* linename)
{
    const int val = gpiod_line_get_value(line);
    
    if ( val == -1 )
    {
        fprintf(stderr, "E: line_get_value. (%s)\n", linename);
    }

    return val;
}

int Harrow::isLifted()
{
    return line_get_value(_lineSensorUp, "SensorUp");
}

int Harrow::isZweitRechts()
{
    return line_get_value(_lineSensorRight, "SensorRight");
}

int Harrow::isZweitLinks()
{
    return line_get_value(_lineSensorLeft, "SensorLeft");
}
