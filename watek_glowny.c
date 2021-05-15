#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
    srandom(rank);
    while (TRUE) {
        if(stan == REST)
        {   
            sleep( SEC_IN_STATE);
            int perc = random()%100; 
            pthread_mutex_lock( &stateMut );
            if (perc<STATE_CHANGE_PROB)
            {
                debug("Zmieniam stan na {QUEUE}");
                changeState(QUEUE);
                onStartQueueing();            
            }
            pthread_mutex_unlock( &stateMut );
        }
        else if(stan == DEBATE)
        {
            if(opponentReady)
            {
                debug("Rozpoczynam debatę z %d", przeciwnik);
                sleep( SEC_IN_STATE);
                pthread_mutex_lock( &stateMut );
                if(calculateWinner())
                {
                    debug("Kończę debatę z %d. WYGRAŁEM :D", przeciwnik);
                }
                else
                {
                    debug("Kończę debatę z %d. PRZEGRAŁEM :(", przeciwnik);
                    sleep( LOOSE_TIME );
                }            
                debug("Zmieniam stan na {REST}");
                changeState(REST);
                onStartResting();
                pthread_mutex_unlock( &stateMut );                  
            }
        }
    }
}

void onStartQueueing()
{
    int pr = zegar;
    for(int i = 0; i < size; i++)
    {
        if(i!=rank)
        {
            packet_t * pkt = malloc(sizeof(packet_t));
            pkt->data = pr;
            sendPacket( pkt, i, PAIR_SZUKAM);
        }    
    }

    queue_element_t el;
    el.priority = pr;
    el.process = rank;
    insertElement(&processQueue, el);
    pairCounter = 0;
}

void onStartResting()
{
    for(int i = 0; i < waitQueueG.size; i++)
    {
        packet_t * pkt = malloc(sizeof(packet_t));
        pkt->data = waitQueueG.data[i].priority;
        sendPacket( pkt, waitQueueG.data[i].process, ACK_SLIPY);
    }
    removeNFirstElements(&waitQueueG, waitQueueG.size);
    for(int i = 0; i < waitQueueP.size; i++)
    {
        packet_t * pkt = malloc(sizeof(packet_t));
        pkt->data = waitQueueP.data[i].priority;
        sendPacket( pkt, waitQueueP.data[i].process, ACK_PINEZKA);
    }
    removeNFirstElements(&waitQueueP, waitQueueG.size);
    for(int i = 0; i < waitQueueM.size; i++)
    {
        packet_t * pkt = malloc(sizeof(packet_t));
        pkt->data = waitQueueM.data[i].priority;
        sendPacket( pkt, waitQueueM.data[i].process, ACK_MICHA);
    }
    removeNFirstElements(&waitQueueM, waitQueueG.size);
    for(int i = 0; i < waitQueueS.size; i++)
    {
        packet_t * pkt = malloc(sizeof(packet_t));
        pkt->data = waitQueueS.data[i].priority;
        sendPacket( pkt, waitQueueS.data[i].process, ACK_SALKA);
    }
    removeNFirstElements(&waitQueueS, waitQueueG.size);
    opponentReady = FALSE;
}

int calculateWinner()
{
    debug("Moim argumentem jest: %d, a przeciwnika %d", argument, opponentArgument);
    if(argument == opponentArgument)
    {
        if(przeciwnik > rank)
            return TRUE;
        return FALSE; 
    }
    if(argument == SLIPY && opponentArgument == PINEZKA)
        return TRUE;
    if(argument == PINEZKA && opponentArgument == MICHA)
        return TRUE;
    if(argument == MICHA && opponentArgument == SLIPY)
        return TRUE;
    return FALSE;
}

