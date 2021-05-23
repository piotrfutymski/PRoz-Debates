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
                changeState(QUEUE, "QUEUE", onStartQueueing);
                if(size >= 4)
                debug("Moja kolejka procesów - początek: [%d, %d, %d, %d, ...",
			  processQueue.data[0].process, processQueue.data[1].process, processQueue.data[2].process, processQueue.data[3].process);
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
                    changeState(REST, "REST", onStartResting);
                    pthread_mutex_unlock( &stateMut );   
                }
                else
                {
                    debug("Kończę debatę z %d. PRZEGRAŁEM :(", przeciwnik);
                    changeState(REST, "REST", onStartResting);
                    pthread_mutex_unlock( &stateMut );   
                    sleep( LOOSE_TIME );
                }                                          
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
    int msgType;
    if (argument == PINEZKA)
		msgType = ACK_PINEZKA;
	else if (argument == MICHA)
		msgType = ACK_MICHA;
	else if (argument == SLIPY)
		msgType = ACK_SLIPY;
    for(int i = 0; i < waitQueueZ.size; i++)
    {
        packet_t * pkt = malloc(sizeof(packet_t));
        pkt->data = waitQueueZ.data[i].priority;
        sendPacket( pkt, waitQueueZ.data[i].process, msgType);
    }
    removeNFirstElements(&waitQueueZ, waitQueueZ.size);
    for(int i = 0; i < waitQueueS.size; i++)
    {
        packet_t * pkt = malloc(sizeof(packet_t));
        pkt->data = waitQueueS.data[i].priority;
        sendPacket( pkt, waitQueueS.data[i].process, ACK_SALKA);
    }
    removeNFirstElements(&waitQueueS, waitQueueS.size);
    opponentReady = FALSE;
    rezSalke = FALSE;
}

int calculateWinner()
{
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

