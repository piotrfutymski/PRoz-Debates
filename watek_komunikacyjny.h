#ifndef WATEK_KOMUNIKACYJNY_H
#define WATEK_KOMUNIKACYJNY_H
#include "process_queue.h"
/* wątek komunikacyjny: odbieranie wiadomości i reagowanie na nie poprzez zmiany stanu */
void *startKomWatek(void *ptr);

void onStartArming();
void onStartLocating();
void onStartDebate();
void onStartQueueWait();

void case_PAIR_SZUKAM(packet_t pakiet);
void case_PAIR_SYNC(packet_t pakiet);
void case_PAIR_JESTEM(packet_t pakiet);
void case_REQ_MICHA(packet_t pakiet);
void case_ACK_MICHA(packet_t pakiet);
void case_REQ_SLIPY(packet_t pakiet);
void case_ACK_SLIPY(packet_t pakiet);
void case_REQ_PINEZKA(packet_t pakiet);
void case_ACK_PINEZKA(packet_t pakiet);
void case_REQ_SALKA(packet_t pakiet);
void case_ACK_SALKA(packet_t pakiet);
void case_DEBATE_READY(packet_t pakiet);

void reqZ(packet_t pakiet, argument_t arg, int msgT);
void ackZ(packet_t pakiet, argument_t arg, int N);


#endif
