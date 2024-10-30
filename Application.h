#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include <stdio.h>
#include <HAL/HAL.h>

#define MAX_PLAYERS 4


typedef enum
{
    Title, Configurations, Instructions, Test, Results, Scores
} State;

struct _Application
{
int players;
int scores[MAX_PLAYERS];
int currentPlayer;
State state;
bool printScreen;
};
typedef struct _Application Application;


/* Enum to represent tilt state */
enum accel_state {UP, NORMAL, DOWN};
static enum accel_state my_state = NORMAL;

/* Words to display */
char* words[3] = {"elephant", "ball", "Paris"};

/* Function prototypes */
void drawTitle(void);
void drawAccelData(void);
void displayWord(void);
void displayScore(void);
void displayTimeRemaining(void);
void next_word(void);
void reset_timer(void);
void applicationLoop(Application *app);
Application applicationConstruct();
void handleTitle(Application *app);
void handleInstructions(Application *app);
void handleGame(Application *app);
void initADC();
void initSystem();



#endif
