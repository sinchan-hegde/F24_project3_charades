/*
 * button.c
 *
 *  Created on: Apr 6, 2021
 *      Author: leyla
 */

#include "HAL/Button.h"

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#include "HAL/LED.h"
#include "HAL/Timer.h"

// A boolean variable that is true when a high-to-low transition is sensed on
// JSB For a global variable, the keyword static limits the scope of the
// variable to this function only. This means functions in other files of the
// project cannot access this variable
volatile static bool JSBmodified;
volatile static bool BB1modified;
volatile static bool BB2modified;

volatile static bool LB1modified;
volatile static bool LB2modified;

// 500 ms debouncing wait
#define DEBOUNCE_WAIT 500

// An internal function that initializes a button and enables the high-to-low
// transition
void initButton(uint_fast8_t selectedPort, uint_fast16_t selectedPins)
{
    // This sets up an input with pull-up resistor. If the input does not need the
    // pull-up resistor, the extra added resistor is harmless.
    GPIO_setAsInputPinWithPullUpResistor(selectedPort, selectedPins);

    // clear interrupt on port selectedPort, pin selectedPins
    // If we fail to this, we "might" get an interrupt as soon we enable interrupt
    GPIO_clearInterruptFlag(selectedPort, selectedPins);

    // enable interrupt on port selectedPort, pin selectedPins
    GPIO_enableInterrupt(selectedPort, selectedPins);

    // the interrupt is triggered on high to low transition (tapping)
    GPIO_interruptEdgeSelect(selectedPort, selectedPins,
    GPIO_HIGH_TO_LOW_TRANSITION);
}

void initButtons()
{
    initButton(GPIO_PORT_P4, GPIO_PIN1);  // JSB
    initButton(GPIO_PORT_P5, GPIO_PIN1);  // BB1
    initButton(GPIO_PORT_P3, GPIO_PIN5);  // BB1

    initButton(LAUNCHPAD_S1_PORT, LAUNCHPAD_S1_PIN); // Launchpad S1
    initButton(LAUNCHPAD_S2_PORT, LAUNCHPAD_S2_PIN);
    // enable the port 4 interrupt related to JSB
    Interrupt_enableInterrupt(INT_PORT4);
    Interrupt_enableInterrupt(INT_PORT5);
    Interrupt_enableInterrupt(INT_PORT1);
    Interrupt_enableInterrupt(INT_PORT3);

    // This allows us to start from a clean slate
    JSBmodified = false;
    BB1modified = false;
    BB2modified = false;

    LB1modified = false;
    LB2modified = false;

}

void PORT4_IRQHandler()
{
    // We check to see if the port4 interrupt came from JSB
    if (GPIO_getInterruptStatus(GPIO_PORT_P4, GPIO_PIN1))
    {
        JSBmodified = true;

        // A very critical step: If we don't clear the interrupt, the ISR will be
        // called again and again.
        GPIO_clearInterruptFlag(GPIO_PORT_P4, GPIO_PIN1);
    }
}

void PORT3_IRQHandler()
{
    // We check to see if the port4 interrupt came from BB2
    if (GPIO_getInterruptStatus(GPIO_PORT_P3, GPIO_PIN5))
    {
        BB2modified = true;

        // A very critical step: If we don't clear the interrupt, the ISR will be
        // called again and again.
        GPIO_clearInterruptFlag(GPIO_PORT_P3, GPIO_PIN5);
    }
}

void PORT5_IRQHandler()
{
    // We check to see if the port4 interrupt came from BB1
    if (GPIO_getInterruptStatus(GPIO_PORT_P5, GPIO_PIN1))
    {
        BB1modified = true;

        // A very critical step: If we don't clear the interrupt, the ISR will be
        // called again and again.
        GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN1);
    }
}

void PORT1_IRQHandler()
{
    // We check to see if the port4 interrupt came from LB1
    if (GPIO_getInterruptStatus(GPIO_PORT_P1, GPIO_PIN1))
    {
        LB1modified = true;

        // A very critical step: If we don't clear the interrupt, the ISR will be
        // called again and again.
        GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    }
    // We check to see if the port4 interrupt came from LB2
    else if (GPIO_getInterruptStatus(GPIO_PORT_P1, GPIO_PIN4))
    {
        LB2modified = true;

        // A very critical step: If we don't clear the interrupt, the ISR will be
        // called again and again.
        GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN4);
    }
}

bool JSBtapped()
{
    // This variable is true if we are in debouncing state (we are ignoring the
    // extra transitions) This variable has to be static so that it "remembers"
    static bool debouncing = false;

    // The SW timer used for debouncing. It has to be static, otherwise it won't
    // work.
    static SWTimer debounceTimer;

    // the single output of the FMS
    bool tapped = false;

    // If we are in debouncing state and the debouncing timer is expired, in other
    // words, if wait time is over, we should leave the debouncing state.
    if (debouncing && SWTimer_expired(&debounceTimer))
        debouncing = false;

    // if we are not in the debouncing state and a transition is detected
    if (!debouncing && JSBmodified)
    {
        // We are not in debouncing and the first transition is detected
        tapped = true;

        // Let's enter debouncing state
        debouncing = true;

        // We should setup a timer for how much to wait
        debounceTimer = SWTimer_construct(DEBOUNCE_WAIT);
        SWTimer_start(&debounceTimer);
    }

    // This is a very critical step similar to clearing interrupt flag.
    // If we don't refresh this variable, next time we enter this function, we
    // think a new transition has happened.
    JSBmodified = false;

    return tapped;
}


bool BB1tapped()
{
    // This variable is true if we are in debouncing state (we are ignoring the
    // extra transitions) This variable has to be static so that it "remembers"
    static bool debouncing = false;

    // The SW timer used for debouncing. It has to be static, otherwise it won't
    // work.
    static SWTimer debounceTimer;

    // the single output of the FMS
    bool tapped = false;

    // If we are in debouncing state and the debouncing timer is expired, in other
    // words, if wait time is over, we should leave the debouncing state.
    if (debouncing && SWTimer_expired(&debounceTimer))
        debouncing = false;

    // if we are not in the debouncing state and a transition is detected
    if (!debouncing && BB1modified)
    {
        // We are not in debouncing and the first transition is detected
        tapped = true;

        // Let's enter debouncing state
        debouncing = true;

        // We should setup a timer for how much to wait
        debounceTimer = SWTimer_construct(DEBOUNCE_WAIT);
        SWTimer_start(&debounceTimer);
    }

    // This is a very critical step similar to clearing interrupt flag.
    // If we don't refresh this variable, next time we enter this function, we
    // think a new transition has happened.
    BB1modified = false;

    return tapped;
}

bool BB2tapped()
{
    // This variable is true if we are in debouncing state (we are ignoring the
    // extra transitions) This variable has to be static so that it "remembers"
    static bool debouncing = false;

    // The SW timer used for debouncing. It has to be static, otherwise it won't
    // work.
    static SWTimer debounceTimer;

    // the single output of the FMS
    bool tapped = false;

bool b2mod = BB2modified;
bool timerExpired = SWTimer_expired(&debounceTimer);
    // If we are in debouncing state and the debouncing timer is expired, in other
    // words, if wait time is over, we should leave the debouncing state.
    if (debouncing && timerExpired)
        debouncing = false;

    // if we are not in the debouncing state and a transition is detected
    if (!debouncing && BB2modified)
    {
        // We are not in debouncing and the first transition is detected
        tapped = true;

        // Let's enter debouncing state
        debouncing = true;

        // We should setup a timer for how much to wait
        debounceTimer = SWTimer_construct(DEBOUNCE_WAIT);
        SWTimer_start(&debounceTimer);
    }

    // This is a very critical step similar to clearing interrupt flag.
    // If we don't refresh this variable, next time we enter this function, we
    // think a new transition has happened.
    BB2modified = false;

    return tapped;
}

bool LB1tapped()
{
    // This variable is true if we are in debouncing state (we are ignoring the
    // extra transitions) This variable has to be static so that it "remembers"
    static bool debouncing = false;

    // The SW timer used for debouncing. It has to be static, otherwise it won't
    // work.
    static SWTimer debounceTimer;

    // the single output of the FMS
    bool tapped = false;

    // If we are in debouncing state and the debouncing timer is expired, in other
    // words, if wait time is over, we should leave the debouncing state.
    if (debouncing && SWTimer_expired(&debounceTimer))
        debouncing = false;

    // if we are not in the debouncing state and a transition is detected
    if (!debouncing && LB1modified)
    {
        // We are not in debouncing and the first transition is detected
        tapped = true;

        // Let's enter debouncing state
        debouncing = true;

        // We should setup a timer for how much to wait
        debounceTimer = SWTimer_construct(DEBOUNCE_WAIT);
        SWTimer_start(&debounceTimer);
    }

    // This is a very critical step similar to clearing interrupt flag.
    // If we don't refresh this variable, next time we enter this function, we
    // think a new transition has happened.
    LB1modified = false;

    return tapped;
}

bool LB2tapped()
{
    // This variable is true if we are in debouncing state (we are ignoring the
    // extra transitions) This variable has to be static so that it "remembers"
    static bool debouncing = false;

    // The SW timer used for debouncing. It has to be static, otherwise it won't
    // work.
    static SWTimer debounceTimer;

    // the single output of the FMS
    bool tapped = false;

    // If we are in debouncing state and the debouncing timer is expired, in other
    // words, if wait time is over, we should leave the debouncing state.
    if (debouncing && SWTimer_expired(&debounceTimer))
        debouncing = false;

    // if we are not in the debouncing state and a transition is detected
    if (!debouncing && LB2modified)
    {
        // We are not in debouncing and the first transition is detected
        tapped = true;

        // Let's enter debouncing state
        debouncing = true;

        // We should setup a timer for how much to wait
        debounceTimer = SWTimer_construct(DEBOUNCE_WAIT);
        SWTimer_start(&debounceTimer);
    }

    // This is a very critical step similar to clearing interrupt flag.
    // If we don't refresh this variable, next time we enter this function, we
    // think a new transition has happened.
    LB2modified = false;

    return tapped;
}

// This function calls all the functions that check button status and stores
// them in one structure This will allow the user to reliably get the latest
// button status. If we choose not to use this method, the user has to be
// careful to call JSBtapped() or any similar function only once in the main
// loop.
buttons_t updateButtons()
{
    buttons_t buttons;

    buttons.JSBtapped = JSBtapped();
    buttons.BB1tapped = BB1tapped();
    buttons.LB1tapped = LB1tapped();
    buttons.LB2tapped = LB2tapped();
    buttons.BB2tapped = BB2tapped();

    return (buttons);
}

/**
 * Constructs a button as a GPIO pushbutton, given a proper port and pin.
 * Initializes the debouncing and output FSMs.
 *
 * @param port:     The GPIO port used to initialize this button
 * @param pin:      The GPIO pin  used to initialize this button
 *
 * @return a constructed button with debouncing and output FSMs initialized
 */
Button Button_construct(uint8_t port, uint16_t pin)
{
    // The button object which will be returned at the end of construction
    Button button;

    // Initialize the member variables for port and pin of the button.
    button.port = port;
    button.pin = pin;

    // Here's a trick: All buttons on the board can be initialized with a
    // pullup resistor, since a double pullup resistor has no impact on the
    // input voltage of the button.
    GPIO_setAsInputPinWithPullUpResistor(port, pin);

    // Initialize all FSM variables for the button to their RELEASED states
    button.debounceState = StableR;
    button.timer = SWTimer_construct(DEBOUNCE_TIME_MS);
    SWTimer_start(&button.timer);

    // Initialize all buffered outputs of the button
    button.pushState = RELEASED;
    button.isTapped = false;

    // Return the constructed Button object to the user
    return button;
}

/**
 * A getter method which should just return whether the user currently has held
 * down the button. This should be determined using the pushState which was
 * computed last time the Button object was refreshed. It does NOT update the
 * internal FSM and does NOT directly check the GPIO signal directly - that is,
 * do NOT use [GPIO_*()] functions here.
 *
 * @param button:   The Button object from which to retrieve the push state
 *
 * @return true if the button is depressed, and false if it is not
 */
bool Button_isPressed(Button *button)
{
    return button->pushState == PRESSED;
}

/**
 * A getter method which should just return whether the user currently has
 * tapped the button. This should NOT update the internal FSM for debouncing
 * and instead should simply fetch the result which was determined last time
 * the button was refreshed. A tap is defined to be true when the button was
 * not held down two refreshes ago but was held down one refresh ago.
 *
 * @param button:   The Button object from which to retrieve the tapped state
 *
 * @return true if the button was tapped, and false otherwise
 */
bool Button_isTapped(Button *button)
{
    return button->isTapped;
}

/**
 * Refreshes the input of the provided Button by polling for the new GPIO input
 * pin value and advancing the debouncing FSM by one step.
 *
 * @param button:   The Button object to refresh
 */
void Button_refresh(Button *button)
{
    // Retrieve the port and pin targets
    uint8_t port = button->port;
    uint16_t pin = button->pin;

    // Poll for updated values from port and pin status through GPIO directly
    uint16_t rawButtonStatus = GPIO_getInputPinValue(port, pin);
    int newPushState = RELEASED;

    // Main debouncing FSM
    switch (button->debounceState)
    {
    // Released State - transition only if the new raw state is pressed
    case StableR:
        if (rawButtonStatus == PRESSED)
        {
            SWTimer_start(&button->timer);
            button->debounceState = TransitionRP;
        }
        newPushState = RELEASED;
        break;

        // Pressed State - transition only if the new raw state is released
    case StableP:
        if (rawButtonStatus == RELEASED)
        {
            SWTimer_start(&button->timer);
            button->debounceState = TransitionPR;
        }
        newPushState = PRESSED;
        break;

        // Transition State - transition if either the timer expires OR if
        //                    the input becomes polluted with an erroneous
        //                    RELEASED input.
    case TransitionRP:
        if (rawButtonStatus == RELEASED)
        {
            button->debounceState = StableR;
        }
        else if (SWTimer_expired(&button->timer))
        {
            button->debounceState = StableP;
        }
        newPushState = RELEASED;
        break;

        // Transition State - transition if either the timer expires OR if
        //                    the input becomes polluted with an erroneous
        //                    PRESSED input.
    case TransitionPR:
        if (rawButtonStatus == PRESSED)
        {
            button->debounceState = StableP;
        }
        else if (SWTimer_expired(&button->timer))
        {
            button->debounceState = StableR;
        }
        newPushState = PRESSED;
    }

    // Outputs of the FSM: The button is tapped if the old debounced state was
    // RELEASED and the new state is PRESSED.
    button->isTapped = newPushState == PRESSED && button->pushState == RELEASED;
    button->pushState = newPushState;
}

