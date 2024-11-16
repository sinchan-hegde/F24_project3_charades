
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
volatile uint8_t waitToPrint = 4;
/* Words to display */
static int word_index = 0;

/* Score variable */
static int score = 0;
static volatile bool initialized = false;
/* Timer-related variables */
#define LCD_WIDTH 128    // LCD screen width for centering text
#define TIMER_VALUE 60   //timer value is 60 seconds
#define CLK_FRQ 48000000 //clock frequency
/* Buzzer GPIO Pin */
#define BUZZER_PORT GPIO_PORT_P2
#define BUZZER_PIN GPIO_PIN7



/*
 *
 * Main function
 */
void sleep()
{
    // The Launchpad Green LED is used to signify the processor is in low-power
    // mode. From the human perspective, it should seem the processor is always
    // asleep except for fractions of second here and there.

    TurnOn_LLG();
    // Enters the Low Power Mode 0 - the processor is asleep and only responds to
    // interrupts
    PCM_gotoLPM0();
    TurnOff_LLG();
}

int main(void)
{
    initialize();
    HAL hal = *(HAL_construct());
    Application app = applicationConstruct();

    while (1)
    {
        sleep();  // Low-power mode

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
initTimer();


    /* Start ADC conversion */
    MAP_ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);
    MAP_ADC14_enableConversion();
    MAP_ADC14_toggleConversionTrigger();

    initButtons();
    MAP_Interrupt_enableMaster();

}

Application applicationConstruct()
{
    Application app;
    app.state = Title;
    app.printScreen = true;
    return app;
}

void initTimer()
{
    MAP_Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,
    TIMER32_PERIODIC_MODE);
    MAP_Timer32_setCount(TIMER32_0_BASE, TIMER_COUNT_VALUE); // 60-second timer (48 MHz)
    MAP_Interrupt_enableInterrupt(INT_T32_INT1);
    MAP_Timer32_enableInterrupt(TIMER32_0_BASE);
    MAP_Timer32_startTimer(TIMER32_0_BASE, true);
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

    case Game:
    {
        handleGame(app, hal);
        break;
    }
    case Results:
    {
        handleResults(app, hal);
            break;
    }
//    case Scores:
//    {
//        handleScores(app, hal);
//        break;
//    }
    default:
    {
        handleTitle(app, hal);
        break;
    }

    }
}

void handleScores(Application *app, HAL *hal)
{

    if (BB1tapped())
    {
        app->printScreen = true;
        app->state = Title;
        Application app = applicationConstruct();
    }

}
void handleResults(Application *app, HAL *hal)
{
    if(app->printScreen)
    {

           end_game();
           app->printScreen = false;
           score = 0;
    }

            if(JSBtapped()){
                app->state = Title;
                app->printScreen = true;
                (*app) = applicationConstruct();
               // initTimer();
             //   MAP_Timer32_haltTimer(TIMER32_0_BASE);
               // MAP_Timer32_setCount(TIMER32_0_BASE, TIMER_COUNT_VALUE);

            }
}

//Converts int to string
void intToString(int num, char *str)
{
    sprintf(str, "%d", num);
}

void printScores(Application *app, HAL *hal)
{
    GFX GFX = hal->GFX;
    GFX_print(&GFX, "Scores        ", 0, 0);
    //GFX_print(&GFX, "Mean:", 7, 0);
    int i;
    double total = 0;
    char *score;
    for (i = 0; i < app->totalPlayers; i++)
    {
        intToString(app->scores[i], score);
        GFX_print(&GFX, score, 2 + i, 3);
    }

    char meanTime[12];
    intToString(total, meanTime);
    GFX_print(&GFX, meanTime, 7, 4);
    GFX_print(&GFX, "ms", 7, 6);

    GFX_print(&GFX, "                                  ", 8, 0);

    GFX_print(&GFX, "Press BB2 to end    ", 9, 0);
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
        MAP_Timer32_haltTimer(TIMER32_0_BASE);
        MAP_Timer32_setCount(TIMER32_0_BASE, 2880000000); // 60-second timer (48 MHz)
        app->printScreen = true;
        app->state = Game;

    }
    if (BB2tapped())
    {
        app->printScreen = true;
        app->state = Instructions;
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
        resetgameOver();
        app->printScreen = false;
        MAP_Timer32_startTimer(TIMER32_0_BASE, true);
        drawGame();
        displayWord();
                   displayScore();
    }

    drawAccelData();
    if(gameIsOver() || JSBtapped()){
        app->state = Results;
        app->printScreen = true;
    }

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
    GrContextFontSet(&g_sContext, &g_sFontCmss12i);

    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "Instructions:",
    AUTO_STRING_LENGTH,
                                64, 10, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "Look up:'",
    AUTO_STRING_LENGTH,
                                64, 25, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "'Charades Heads Up!'",
    AUTO_STRING_LENGTH,
                                64, 40, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext,
                                (int8_t*) "Follow the instructions",
                                AUTO_STRING_LENGTH,
                                64, 55, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "keeping the LCD",
    AUTO_STRING_LENGTH,
                                64, 70, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "perpendicular",
    AUTO_STRING_LENGTH,
                                64, 85, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "to the ground",
    AUTO_STRING_LENGTH,
                                64, 100, OPAQUE_TEXT);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);

}

void drawSettings()
{
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "Settings:",
    AUTO_STRING_LENGTH,
                                64, 30, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "Press bb1 for animals",
        AUTO_STRING_LENGTH,
                                    64, 60, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "Press bb2 for objects",
            AUTO_STRING_LENGTH,
                                        64, 90, OPAQUE_TEXT);
}

void drawGame()
{
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) "Charades:",
    AUTO_STRING_LENGTH,
                                64, 30, OPAQUE_TEXT);
    Graphics_drawString(&g_sContext, "Time:   s",
       AUTO_STRING_LENGTH,
                                   30, 110, OPAQUE_TEXT);
    Graphics_drawString(&g_sContext, "Score: ",
       AUTO_STRING_LENGTH,
                                   40, 90, OPAQUE_TEXT);
   /* Graphics_drawString(&g_sContext, "Word: ",
       AUTO_STRING_LENGTH,
                                   10, 50, OPAQUE_TEXT);*/
}

void displayWord()
{
    char word[20];
        sprintf(word, " %s", words[word_index]);
        GrContextFontSet(&g_sContext, &g_sFontCmss24b);

        Graphics_drawStringCentered(&g_sContext, "                ",
             AUTO_STRING_LENGTH,
                                         65, 65, OPAQUE_TEXT);
        Graphics_drawStringCentered(&g_sContext, (int8_t*) word,
        AUTO_STRING_LENGTH,
                                    65, 65, OPAQUE_TEXT);
        GrContextFontSet(&g_sContext, &g_sFontFixed6x8);

}

void displayScore()
{
    char scoreStr[10];
    sprintf(scoreStr, " %d", score);
    Graphics_drawString(&g_sContext, "        ",
    AUTO_STRING_LENGTH,
                                75, 90, OPAQUE_TEXT);
    Graphics_drawString(&g_sContext, (int8_t*) scoreStr,
    AUTO_STRING_LENGTH,
                                75, 90, OPAQUE_TEXT);
}

void next_word()
{
    word_index = rand() % 30;
}
int get_remaining_time()
{
    int time = MAP_Timer32_getValue(TIMER32_0_BASE);
    int time_remaining = time / 48000000;
    return time_remaining;
}

void end_game()
{
    char final_score[30];
    Graphics_clearDisplay(&g_sContext);
    sprintf(final_score, "Your final score: %d ", score);
    Graphics_drawStringCentered(&g_sContext, (int8_t*) final_score,
    AUTO_STRING_LENGTH,
                                64, 50, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, "Press JSB to return.",
     AUTO_STRING_LENGTH,
                                 64, 90, OPAQUE_TEXT);

}

void displayTimeRemaining()
{
    char timeStr[10];
    uint32_t timer_value = MAP_Timer32_getValue(TIMER32_0_BASE);
    uint32_t remaining_time = timer_value / 48000000;
    sprintf(timeStr, "%d", remaining_time);
 /*   Graphics_drawStringCentered(&g_sContext, "  ",
    AUTO_STRING_LENGTH,
                                70, 110, OPAQUE_TEXT);*/
    if(remaining_time == 9)
    {
       Graphics_drawString(&g_sContext, "  ",
      AUTO_STRING_LENGTH,
                                  64, 110, OPAQUE_TEXT);
    }

    Graphics_drawString(&g_sContext, (int8_t*) timeStr,
     AUTO_STRING_LENGTH,
                                 64, 110, OPAQUE_TEXT);
}

void drawAccelData()
{
    switch (my_state)
    {
    case NORMAL:
        //MAP_Timer32_startTimer(TIMER32_0_BASE, true);
//        if (get_remaining_time() == 0)
//        {
//            Graphics_clearDisplay(&g_sContext);
//           // end_game();
//        }

     //   displayWord();
      //  displayScore();
        waitToPrint = (waitToPrint + 1 % 5);

        if(waitToPrint == 0)
        displayTimeRemaining();  // Display the remaining time

        if (resultsBuffer[2] < 7000)
        {
           // Graphics_clearDisplay(&g_sContext);
           // next_word();
            my_state = DOWN;
           // displayWord();
           // displayScore();


        }
        else if (resultsBuffer[2] > 10500)
        {
          //  Graphics_clearDisplay(&g_sContext);
            //next_word();
            my_state = UP;
           // displayWord();
           // displayScore();

        }
        break;
    case DOWN:
        waitToPrint = (waitToPrint + 1 % 5);
        if(waitToPrint == 0)
                displayTimeRemaining();  // Display the remaining time      //  displayWord();
      //  displayScore();
        if (resultsBuffer[2] > 7500)
        {
            score++;

            my_state = NORMAL;
            next_word();

           displayWord();
           displayScore();


        }
        // my_state = NORMAL;
        break;
    case UP:
        waitToPrint = (waitToPrint + 1 % 5);

        if(waitToPrint == 0)
                displayTimeRemaining();  // Display the remaining time
        if (resultsBuffer[2] < 10000)
        {
            next_word();
            displayWord();
           displayScore();
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

    }
}

