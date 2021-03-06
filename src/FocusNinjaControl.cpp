#include "FocusNinjaControl.h"
#include <Arduino.h>

FocusNinjaControl::FocusNinjaControl()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PIN_ENDSTOP, INPUT_PULLDOWN);
    pinMode(PIN_DIRECTION, OUTPUT);
    pinMode(PIN_STEP, OUTPUT);
    pinMode(PIN_CAMERA_RELEASE, OUTPUT);
    digitalWrite(PIN_CAMERA_RELEASE, LOW);
    //homeCarriage();
}

void FocusNinjaControl::moveCarriage(float millimeter, bool direction)
{
    int degreesToRotate = (millimeter / MILLIMETER_PER_ROTATION) * 360;
    if (direction == FORWARDS)
    {
        carriageDirection = 1;
    }
    else
    {
        carriageDirection = -1;
    }
    rotateStepper(degreesToRotate, DEFAULT_SPEED, direction);
}

//Home the carriage
void FocusNinjaControl::homeCarriage()
{
    homed = false;
    carriageDirection = -1;
    rotateStepper(1, DEFAULT_SPEED, BACKWARDS);
    log("log Homing.");
}

bool FocusNinjaControl::isMoving()
{
    return numberOfPulses > 0;
}

void FocusNinjaControl::releaseShutter()
{
    digitalWrite(PIN_CAMERA_RELEASE, HIGH);
    delayMicroseconds(20000);
    digitalWrite(PIN_CAMERA_RELEASE, LOW);
}

void FocusNinjaControl::stop()
{
    numberOfPulses = 0;
    carriageDirection = 0;
    log("log Stopped.");
}

//This needs to be called from the loop
void FocusNinjaControl::motorControl()
{
    if (!carriageDirection)
    {
        // not moving in any direction
        return;
    }

    if (homed)
    {
        //Minumum position reached, stop moving
        if (position <= 0 and carriageDirection == -1)
        {
            numberOfPulses = 0;
        }
        //Maximum position reached, stop moving
        if (position >= MAX_POSITION and carriageDirection == 1)
        {
            numberOfPulses = 0;
            log("log Reached the end.");
        }
    }
    //Move the carriage
    if (numberOfPulses > 0)
    {
        if (homed)
        {
            numberOfPulses -= 1;
        }
        position = position + (mmPerStep * carriageDirection);
        reportPosition(position);
        digitalWrite(PIN_STEP, HIGH);
        delayMicroseconds(pulseWidthMs);
        digitalWrite(PIN_STEP, LOW);
        delayMicroseconds(pulseWidthMs);
    }
    //Read endstop en feedback to led
    bool endStop = digitalRead(PIN_ENDSTOP);
    digitalWrite(LED_BUILTIN, endStop);

    //If homing, stop when endstop triggers
    if ((endStop == HIGH) && !homed)
    {
        homed = true;
        if (position)
        {
            reportPosition(0);
        }
        position = 0;
        numberOfPulses = 0;
        carriageDirection = 0;
        log("log Homed.");
    }
}

//Basic function to rotate the steppermotor
void FocusNinjaControl::rotateStepper(float degrees, int rotationSpeed, bool rotationDirection)
{
    numberOfPulses = MICROSTEPPING * degrees / DEGREES_PER_STEP;
    pulseWidthMs = (1000000 / (2 * DEGREES_PER_STEP * 360 / DEGREES_PER_STEP)) / rotationSpeed;
    digitalWrite(PIN_DIRECTION, rotationDirection);
}

void FocusNinjaControl::setLogger(void (*fun_ptr)(const char *))
{
    logger = fun_ptr;
}

void FocusNinjaControl::log(const char *s)
{
    if (logger)
    {
        logger(s);
    }
}

void FocusNinjaControl::reportPosition(float pos)
{
    char buf[32];

    snprintf(buf, 32, "pos %f", pos);
    log(buf);
}