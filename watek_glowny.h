#ifndef WATEK_GLOWNY_H
#define WATEK_GLOWNY_H
#include "process_queue.h"
/* pętla główna aplikacji: zmiany stanów itd */
void mainLoop();

void onStartQueueing();
void onStartResting();
int calculateWinner();

#endif
