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
			if(size >= 4)
			debug("Otrzymałem PAIR_SZUKAM od %d. Moja kolejka procesów - początek: [%d, %d, %d, %d, ...",
					pakiet.src, processQueue.data[0].process, processQueue.data[1].process, processQueue.data[2].process, processQueue.data[3].process);
			
			sendPacket(0, pakiet.src, PAIR_SYNC);

			break;
		case PAIR_SYNC:
			debug("Otrzymałem PAIR_SYNC od %d", pakiet.src);
			if( stan == QUEUE)
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
						if(jestem)
						{
							jestem = FALSE;
							przeciwnik = processQueue.data[mPos+1].process;
							debug("Moim przeciwnikiem jest %d", przeciwnik);
							removeNFirstElements(&processQueue,mPos+2);
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
			if(stan == QUEUE)
			{
				jestem = TRUE;
			}
	    	break;
		case REQ_MICHA:
			//debug("Otrzymałem REQ_MICHA od %d", pakiet.src);
			if(argument == MICHA && (stan == LOCATING || stan == DEBATE || (stan == ARMING && pakiet.data > ackPriority)))
			{
				el.priority = pakiet.data;
    			el.process = pakiet.src;
				insertElement(&waitQueueM, el);
			}
			else
			{
				packet_t * pkt = malloc(sizeof(packet_t));
            	pkt->data = pakiet.data;
            	sendPacket( pkt, pakiet.src, ACK_MICHA);
			}
			if(pakiet.src == przeciwnik)
				opponentArgument = MICHA;
			break;
		case ACK_MICHA:
			//debug("Otrzymałem ACK_MICHA od %d", pakiet.src);
			if(stan == ARMING && argument == MICHA && pakiet.data == ackPriority)
			{
				ackCounterM++;
				if(ackCounterM >= size - N_MICHA)
				{
					ackCounterM = 0;
					if(przeciwnik < rank)
					{
						debug("Przeciwnik ma niższe id więc zarezerwuje mi salkę")
						debug("Zmieniam stan na {DEBATE}");
						changeState(DEBATE);
						onStartDebate();
					}
					else
					{
						debug("Przeciwnik ma wyższe id więc ja zarezerwuje salkę")
						debug("Zmieniam stan na {LOCATING}");
						changeState(LOCATING);
						onStartLocating();
					}
				}
			}
			break;
		case REQ_SLIPY:
			//debug("Otrzymałem REQ_SLIPY od %d", pakiet.src);
			if(argument == SLIPY && (stan == LOCATING || stan == DEBATE || (stan == ARMING && pakiet.data > ackPriority)))
			{
				el.priority = pakiet.data;
    			el.process = pakiet.src;
				insertElement(&waitQueueG, el);
			}
			else
			{
				packet_t * pkt = malloc(sizeof(packet_t));
            	pkt->data = pakiet.data;
            	sendPacket( pkt, pakiet.src, ACK_SLIPY);
			}
			if(pakiet.src == przeciwnik)
				opponentArgument = SLIPY;
			break;
		case ACK_SLIPY:
			//debug("Otrzymałem ACK_SLIPY od %d", pakiet.src);
			if(stan == ARMING && argument == SLIPY && pakiet.data == ackPriority)
			{
				ackCounterG++;
				if(ackCounterG >=  size - N_SLIPY)
				{
					ackCounterG = 0;
					if(przeciwnik < rank)
					{
						debug("Przeciwnik ma niższe id więc zarezerwuje mi salkę")
						debug("Zmieniam stan na {DEBATE}");
						changeState(DEBATE);
						onStartDebate();
					}
					else
					{
						debug("Przeciwnik ma wyższe id więc ja zarezerwuje salkę")
						debug("Zmieniam stan na {LOCATING}");
						changeState(LOCATING);
						onStartLocating();
					}
				}
			}
			break;
		case REQ_PINEZKA:
			//debug("Otrzymałem REQ_PINEZKA od %d", pakiet.src);
			if(argument == PINEZKA && (stan == LOCATING || stan == DEBATE || (stan == ARMING && pakiet.data > ackPriority)))
			{
				el.priority = pakiet.data;
    			el.process = pakiet.src;
				insertElement(&waitQueueP, el);
			}
			else
			{
				packet_t * pkt = malloc(sizeof(packet_t));
            	pkt->data = pakiet.data;
            	sendPacket( pkt, pakiet.src, ACK_PINEZKA);
			}
			if(pakiet.src == przeciwnik)
				opponentArgument = PINEZKA;
			break;
		case ACK_PINEZKA:
			//debug("Otrzymałem ACK_PINEZKA od %d", pakiet.src);
			if(stan == ARMING && argument == PINEZKA && pakiet.data == ackPriority)
			{
				ackCounterP++;
				if(ackCounterP >=  size - N_PINEZKA)
				{
					ackCounterP = 0;
					if(przeciwnik < rank)
					{
						debug("Przeciwnik ma niższe id więc zarezerwuje mi salkę")
						debug("Zmieniam stan na {DEBATE}");
						changeState(DEBATE);
						onStartDebate();
					}
					else
					{
						debug("Przeciwnik ma wyższe id więc ja zarezerwuje salkę")
						debug("Zmieniam stan na {LOCATING}");
						changeState(LOCATING);
						onStartLocating();
					}
				}
			}
			break;
		case REQ_SALKA:
			//debug("Otrzymałem REQ_SALKA od %d", pakiet.src);
			if((stan == DEBATE && rank <= przeciwnik) || (stan == LOCATING && pakiet.data > ackSPriority))
			{
				el.priority = pakiet.data;
    			el.process = pakiet.src;
				insertElement(&waitQueueS, el);
			}
			else
			{
				packet_t * pkt = malloc(sizeof(packet_t));
            	pkt->data = pakiet.data;
            	sendPacket( pkt, pakiet.src, ACK_SALKA);
			}
			break;
		case ACK_SALKA:
			//debug("Otrzymałem ACK_SALKA od %d", pakiet.src);
			if(stan == LOCATING && pakiet.data == ackSPriority)
			{
				ackCounterS++;
				if(ackCounterS == size - N_SALKA)
				{
					ackCounterS = 0;
					debug("Zmieniam stan na {DEBATE}");
					changeState(DEBATE);
					onStartDebate();
				}
			}
			break;
		case DEBATE_READY:
			//debug("Otrzymałem DEBATE_READY od %d", pakiet.src);
			opponentReady = TRUE;
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
	ackCounterS = 0;
	int pr = zegar;
    for(int i = 0; i < size; i++)
    {
        if(i!=rank)
        {
            packet_t * pkt = malloc(sizeof(packet_t));
            pkt->data = pr;
            sendPacket( pkt, i, REQ_SALKA);
        }    
    }
    ackSPriority = pr;
}

void onStartDebate()
{
	packet_t * pkt = malloc(sizeof(packet_t));
    sendPacket( 0, przeciwnik, DEBATE_READY);
}