/*
 * Project 3 Fall 2024 ECE 2564 - Charades
 * Authors: Yuri Braga and Sinchan Hegde
 * Version: November 2024
 */

#include "Application.h"


/* Graphic library context */
Graphics_Context g_sContext;

/* ADC results buffer */
static uint16_t resultsBuffer[3];

/* Enum to represent tilt state */
enum accel_state {UP, NORMAL, DOWN};
static enum accel_state my_state = NORMAL;

/* Words to display */
char* words[30] = {"elephant", "airplane", "guitar","Swimming","Balloon","Whisper","Robot","Spider","Dancing","Pirate","Fireworks","Chef","Lion","Sleeping","Rainbow","Doctor","Superhero","Fishing","Laughing","Astronaut","Washing Machine","Dinosaur","Painting","Surfing","Clapping","Ghost","Bowling","Magician","Juggling","Campfire"};
static int word_index = 0;

/* Score variable */
static int score = 0;

/* Timer-related variables */
#define LCD_WIDTH 128             // LCD screen width for centering text

/*
 * Main function
 */
int main(void)
{
    initialize();
    HAL hal = *(HAL_construct());
    Application app = *(applicationConstruct());

    while (1)
    {
        MAP_PCM_gotoLPM0();  // Low-power mode
        applicationLoop(&app, &hal);
    }
}

void initialize()
{
    /* Halting WDT and disabling master interrupts */
    MAP_WDT_A_holdTimer();
    MAP_Interrupt_disableMaster();

    /* Set the core voltage level to VCORE1 */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);

    /* Set 2 flash wait states for Flash bank 0 and 1 */
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

    /* Initializes Clock System */
    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    /* Initializes display */
    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

    /* Initializes graphics context */
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128,
                         &g_sCrystalfontz128x128_funcs);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);

    //  drawTitle();

    /* Configures ADC input pins */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(
            GPIO_PORT_P4, GPIO_PIN0 | GPIO_PIN2, GPIO_TERTIARY_MODULE_FUNCTION);
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(
            GPIO_PORT_P6, GPIO_PIN1, GPIO_TERTIARY_MODULE_FUNCTION);

    /* Initializing ADC */
    MAP_ADC14_enableModule();
    MAP_ADC14_initModule(ADC_CLOCKSOURCE_ADCOSC, ADC_PREDIVIDER_64,
    ADC_DIVIDER_8,
                         0);
    MAP_ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM2, true);
    MAP_ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,
    ADC_INPUT_A14,
                                        ADC_NONDIFFERENTIAL_INPUTS);
    MAP_ADC14_configureConversionMemory(ADC_MEM1, ADC_VREFPOS_AVCC_VREFNEG_VSS,
    ADC_INPUT_A13,
                                        ADC_NONDIFFERENTIAL_INPUTS);
    MAP_ADC14_configureConversionMemory(ADC_MEM2, ADC_VREFPOS_AVCC_VREFNEG_VSS,
    ADC_INPUT_A11,
                                        ADC_NONDIFFERENTIAL_INPUTS);

    /* Enabling ADC interrupt */
    MAP_ADC14_enableInterrupt(ADC_INT2);
    MAP_Interrupt_enableInterrupt(INT_ADC14);

    /* Timer32 configuration */
    MAP_Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,
    TIMER32_PERIODIC_MODE);
    MAP_Timer32_setCount(TIMER32_0_BASE, 480000000); // 1-second interval (48 MHz)
    MAP_Interrupt_enableInterrupt(INT_T32_INT1);
    MAP_Timer32_enableInterrupt(TIMER32_0_BASE);
    MAP_Timer32_startTimer(TIMER32_0_BASE, true);

    /* Start ADC conversion */
    MAP_ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);
    MAP_ADC14_enableConversion();
    MAP_ADC14_toggleConversionTrigger();

    MAP_Interrupt_enableMaster();
    initButtons();
}

Application* applicationConstruct()
{
    Application app;
    app.state = Title;
    app.printScreen = true;
    return &app;
}

void applicationLoop(Application *app, HAL *hal)
{
    switch (app->state)
    {
    case Title:
    {
        handleTitle(app, hal);
        break;
    }
    case Instructions:
    {
        handleInstructions(app, hal);
        break;
    }
    case Settings:
    {
        handleSettings(app, hal);
        break;

    }
    case Game:
    {
        handleGame(app, hal);
        break;
    }
    }
}

void handleTitle(Application *app, HAL *hal)
{
    if (app->printScreen)
    {
        app->printScreen = false;
        drawTitle();
    }

    if (BB1tapped())
    {
        app->printScreen = true;
        app->state = Settings;
    }
    if (BB2tapped())
    {
        app->printScreen = true;
        app->state = Instructions;
    }

}

void handleSettings(Application *app, HAL *hal)
{
    if (app->printScreen)
    {
        app->printScreen = false;
        drawSettings();
    }

    if (BB1tapped())
    {
        app->printScreen = true;
        app->state = Game;
    }
}

void handleInstructions(Application *app, HAL *hal)
{
    if (app->printScreen)
    {
        app->printScreen = false;
        drawInstructions();
    }

    if (BB2tapped())
    {
        app->printScreen = true;
        app->state = Title;
    }
}

void handleGame(Application *app, HAL *hal)
{
    if (app->printScreen)
    {
        app->printScreen = false;
        drawGame();
    }
    drawAccelData();

}

void drawTitle()
{
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "Welcome to Charades:",
    AUTO_STRING_LENGTH,
                                64, 30, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "Press BB1 to proceed.",
    AUTO_STRING_LENGTH,
                                64, 60, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "Press BB2 for instr.",
       AUTO_STRING_LENGTH,
                                   64, 90, OPAQUE_TEXT);
}

void drawInstructions()
{
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "Instructions:",
    AUTO_STRING_LENGTH,
                                64, 30, OPAQUE_TEXT);
}

void drawSettings()
{
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "Settings:",
    AUTO_STRING_LENGTH,
                                64, 30, OPAQUE_TEXT);
}

void drawGame()
{
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "Charades:",
    AUTO_STRING_LENGTH,
                                64, 30, OPAQUE_TEXT);
}

void displayWord()
{
    Graphics_drawStringCentered(&g_sContext, (int8_t*) words[word_index],
    AUTO_STRING_LENGTH,
                                64, 64, OPAQUE_TEXT);
}

void displayScore()
{
    char scoreStr[10];
    sprintf(scoreStr, "Score: %d", score);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) scoreStr,
    AUTO_STRING_LENGTH,
                                64, 90, OPAQUE_TEXT);
}

void displayTimeRemaining()
{
    char timeStr[20];
    int timer_value = MAP_Timer32_getValue(TIMER32_0_BASE);
    int remaining_time = timer_value / 48000000;
    sprintf(timeStr, "Time: %d s", remaining_time);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) timeStr,
    AUTO_STRING_LENGTH,
                                64, 110, OPAQUE_TEXT);
}


void next_word() {
    word_index = rand()%30;
    reset_timer();  // Reset the timer when the word changes
}
int get_remaining_time()
{
    int time = MAP_Timer32_getValue(TIMER32_0_BASE);
    int time_remaining = time / 48000000;
    return time_remaining;
}

void reset_timer()
{
    MAP_Timer32_haltTimer(TIMER32_0_BASE);  // Stop the timer
    MAP_Timer32_setCount(TIMER32_0_BASE, 480000000);  // Reload the timer count
    MAP_Timer32_startTimer(TIMER32_0_BASE, false);
    displayTimeRemaining();  // Immediately update the time display
}

void drawAccelData()
{
    switch (my_state)
    {
    case NORMAL:
        MAP_Timer32_startTimer(TIMER32_0_BASE, false);
        if (get_remaining_time() == 0)
        {
            Graphics_clearDisplay(&g_sContext);
            next_word();
            displayWord();
            displayScore();
            displayTimeRemaining();
        }

        displayWord();
        displayScore();
        displayTimeRemaining();  // Display the remaining time
        if (resultsBuffer[2] < 7000)
        {
            Graphics_clearDisplay(&g_sContext);
            next_word();
            reset_timer();
            my_state = DOWN;
        }
        else if (resultsBuffer[2] > 11500)
        {
            Graphics_clearDisplay(&g_sContext);
            next_word();
            reset_timer();
            my_state = UP;
        }
        break;
    case DOWN:
        score++;
        displayWord();
        displayScore();
        displayTimeRemaining();
        my_state = NORMAL;
        break;
    case UP:
        displayWord();
        displayScore();
        displayTimeRemaining();
        if (resultsBuffer[2] < 11000)
        {
            my_state = NORMAL;
        }
        break;
    }
}

void ADC14_IRQHandler(void)
{
    uint64_t status = MAP_ADC14_getEnabledInterruptStatus();
    MAP_ADC14_clearInterruptFlag(status);

    if (status & ADC_INT2)
    {
        resultsBuffer[0] = ADC14_getResult(ADC_MEM0);
        resultsBuffer[1] = ADC14_getResult(ADC_MEM1);
        resultsBuffer[2] = ADC14_getResult(ADC_MEM2);

        //drawAccelData();  // Update display based on accelerometer data
    }
}
