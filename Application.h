#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include <stdio.h>
#include <HAL/HAL.h>

#define MAX_PLAYERS 4


typedef enum
{
    Title, Settings, Instructions, Game, Results, Scores
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
char* words[30] = {"elephant", "airplane", "guitar","Swimming","Balloon","Whisper","Robot","Spider","Dancing","Pirate","Fireworks","Chef","Lion","Sleeping","Rainbow","Doctor","Superhero","Fishing","Laughing","Astronaut","Washing Machine","Dinosaur","Painting","Surfing","Clapping","Ghost","Bowling","Magician","Juggling","Campfire"};

/* Function prototypes */
void drawTitle(void);
void drawAccelData(void);
void displayWord(void);
void displayScore(void);
void displayTimeRemaining(void);
void next_word(void);
void reset_timer(void);
void applicationLoop(Application *app, HAL *hal);
Application* applicationConstruct();
void handleTitle(Application *app, HAL *hal);
void handleInstructions(Application *, HAL *hal);
void handleGame(Application *app, HAL *hal);
void handleSettings(Application *app, HAL *hal);
void initialize();
void drawInstructions();
void drawGame();
void drawSettings();


#endif
