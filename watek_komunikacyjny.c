#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( TRUE ) {
	//debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		pthread_mutex_lock(&zegarMut);
		if(zegar > pakiet.ts)
			zegar++;
		else
			zegar = pakiet.ts+1;
		pthread_mutex_unlock(&zegarMut);
		queue_element_t el;
		pthread_mutex_lock(&stateMut);
        switch ( status.MPI_TAG ) {
	    case PAIR_SZUKAM:
    		el.priority = pakiet.data;
    		el.process = pakiet.src;
    		insertElement(&processQueue, el);
			debug("Otrzymałem PAIR_SZUKAM od %d. Moja kolejka procesów - początek: [%d:%d, %d:%d, %d:%d, %d:%d]",
					pakiet.src,
					processQueue.data[0].priority, processQueue.data[0].process,
					processQueue.data[1].priority, processQueue.data[1].process,
					processQueue.data[2].priority, processQueue.data[2].process,
					processQueue.data[3].priority, processQueue.data[3].process);
			sendPacket(0, pakiet.src, PAIR_SYNC);
			break;
		case PAIR_SYNC:
			debug("Otrzymałem PAIR_SYNC od %d", pakiet.src);
			if( stan = QUEUE)
			{
				pairCounter++;
				if(pairCounter == size-1)
				{
					pairCounter = 0;
					int mPos = findProcess(&processQueue, rank);
					if(mPos%2 == 1)
					{						
						przeciwnik = processQueue.data[mPos-1].process;
						debug("Moim przeciwnikiem jest %d", przeciwnik);
						sendPacket(0, przeciwnik, PAIR_JESTEM);
						removeNFirstElements(&processQueue,mPos+1);
						debug("Zmieniam stan na {ARMING}");
						changeState(ARMING);
						onStartArming();
					}
					else
					{
						removeNFirstElements(&processQueue,mPos);
						debug("Zmieniam stan na {QUEUE_WAIT}");
						changeState(QUEUE_WAIT);
					}
				}
			}
			break;
		case PAIR_JESTEM:
			debug("Otrzymałem PAIR_JESTEM od %d", pakiet.src);
			if( stan == QUEUE_WAIT)
			{
				przeciwnik = pakiet.src;
				debug("Moim przeciwnikiem jest %d", przeciwnik);
				removeProcess(&processQueue, rank);
				removeProcess(&processQueue, przeciwnik);
				debug("Zmieniam stan na {ARMING}");
				changeState(ARMING);
				onStartArming();
			}
	    	break;
	    default:
	    break;
        }
		pthread_mutex_unlock(&stateMut);
    }
}

void onStartArming()
{	
	argument = rand()%3;
    int msgType;
    if(argument == PINEZKA)
    {
        ackCounterP = 0;
        msgType = REQ_PINEZKA;
		debug("Wybieram *PINEZKĘ*");
    }
    else if(argument == MICHA)
    {
        ackCounterM = 0;
        msgType = REQ_MICHA;
		debug("Wybieram *MICHĘ*");
    }
    else if(argument == SLIPY)
    {
        ackCounterS = 0;
        msgType = REQ_SLIPY;
		debug("Wybieram *SLIPY*");
    }
    int pr = zegar;
    for(int i = 0; i < size; i++)
    {
        if(i!=rank)
        {
            packet_t * pkt = malloc(sizeof(packet_t));
            pkt->data = pr;
            sendPacket( pkt, i, msgType);
        }    
    }
    ackPriority = pr;
}

void onStartLocating()
{

}