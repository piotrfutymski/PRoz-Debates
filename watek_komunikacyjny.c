#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
	MPI_Status status;
	int is_message = FALSE;
	packet_t pakiet;
	while (TRUE)
	{
		MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		pthread_mutex_lock(&zegarMut);
		if (zegar > pakiet.ts)
			zegar++;
		else
			zegar = pakiet.ts + 1;
		pthread_mutex_unlock(&zegarMut);
		pthread_mutex_lock(&stateMut);

		switch (status.MPI_TAG)
		{
		case PAIR_SZUKAM:
			debug("Otrzymałem PAIR_SZUKAM od %d", pakiet.src);
			case_PAIR_SZUKAM(pakiet);
			break;
		case PAIR_SYNC:
			debug("Otrzymałem PAIR_SYNC od %d", pakiet.src);
			case_PAIR_SYNC(pakiet);
			break;
		case PAIR_JESTEM:
			debug("Otrzymałem PAIR_JESTEM od %d", pakiet.src);
			case_PAIR_JESTEM(pakiet);
			break;
		case REQ_MICHA:
			//debug("Otrzymałem REQ_MICHA od %d", pakiet.src);
			case_REQ_MICHA(pakiet);
			break;
		case ACK_MICHA:
			debug("Otrzymałem ACK_MICHA od %d", pakiet.src);
			case_ACK_MICHA(pakiet);
			break;
		case REQ_SLIPY:
			//debug("Otrzymałem REQ_SLIPY od %d", pakiet.src);
			case_REQ_SLIPY(pakiet);
			break;
		case ACK_SLIPY:
			debug("Otrzymałem ACK_SLIPY od %d", pakiet.src);
			case_ACK_SLIPY(pakiet);
			break;
		case REQ_PINEZKA:
			//debug("Otrzymałem REQ_PINEZKA od %d", pakiet.src);
			case_REQ_PINEZKA(pakiet);
			break;
		case ACK_PINEZKA:
			debug("Otrzymałem ACK_PINEZKA od %d", pakiet.src);
			case_ACK_PINEZKA(pakiet);
			break;
		case REQ_SALKA:
			//debug("Otrzymałem REQ_SALKA od %d", pakiet.src);
			case_REQ_SALKA(pakiet);
			break;
		case ACK_SALKA:
			debug("Otrzymałem ACK_SALKA od %d", pakiet.src);
			case_ACK_SALKA(pakiet);
			break;
		case DEBATE_READY:
			//debug("Otrzymałem DEBATE_READY od %d", pakiet.src);
			case_DEBATE_READY(pakiet);
			break;
		default:
			break;
		}
		pthread_mutex_unlock(&stateMut);
	}
}

void case_PAIR_SZUKAM(packet_t pakiet)
{
	queue_element_t el;
	el.priority = pakiet.data;
	el.process = pakiet.src;
	insertElement(&processQueue, el);
	if (size >= 4)
		debug("Moja kolejka procesów - początek: [%d, %d, %d, %d, ...",
			  processQueue.data[0].process, processQueue.data[1].process, processQueue.data[2].process, processQueue.data[3].process);
	sendPacket(0, pakiet.src, PAIR_SYNC);
}

void case_PAIR_SYNC(packet_t pakiet)
{
	if (stan == QUEUE)
	{
		pairCounter++;
		if (pairCounter == size - 1)
		{
			pairCounter = 0;
			int mPos = findProcess(&processQueue, rank);
			if (mPos % 2 == 1)
			{
				przeciwnik = processQueue.data[mPos - 1].process;
				debug("Moim przeciwnikiem jest %d", przeciwnik);
				removeNFirstElements(&processQueue, mPos + 1);
				changeState(LOCATING, "LOCATING", onStartLocating);
			}
			else
			{
				if (jestem)
				{
					jestem = FALSE;
					przeciwnik = processQueue.data[mPos + 1].process;
					debug("Moim przeciwnikiem jest %d", przeciwnik);
					removeNFirstElements(&processQueue, mPos + 2);
					changeState(ARMING, "ARMING", onStartArming);
				}
				else
				{
					removeNFirstElements(&processQueue, mPos);
					changeState(QUEUE_WAIT, "QUEUE_WAIT", onStartQueueWait);
				}
			}
		}
	}
}

void case_PAIR_JESTEM(packet_t pakiet)
{
	if (stan == QUEUE_WAIT)
	{
		przeciwnik = pakiet.src;
		debug("Moim przeciwnikiem jest %d", przeciwnik);
		removeProcess(&processQueue, rank);
		removeProcess(&processQueue, przeciwnik);
		changeState(ARMING, "ARMING", onStartArming);
	}
	else if (stan == QUEUE)
	{
		jestem = TRUE;
	}
}

void case_REQ_MICHA(packet_t pakiet)
{
	reqZ(pakiet, MICHA, ACK_MICHA);
}
void case_ACK_MICHA(packet_t pakiet)
{
	ackZ(pakiet, MICHA, N_MICHA);
}
void case_REQ_SLIPY(packet_t pakiet)
{
	reqZ(pakiet, SLIPY, ACK_SLIPY);
}
void case_ACK_SLIPY(packet_t pakiet)
{
	ackZ(pakiet, SLIPY, N_SLIPY);
}
void case_REQ_PINEZKA(packet_t pakiet)
{
	reqZ(pakiet, PINEZKA, ACK_PINEZKA);
}
void case_ACK_PINEZKA(packet_t pakiet)
{
	ackZ(pakiet, PINEZKA, N_PINEZKA);
}

void case_REQ_SALKA(packet_t pakiet)
{
	queue_element_t el;
	if (((stan == DEBATE || stan == ARMING) && rezSalke) || (stan == LOCATING && pakiet.data > ackSPriority) || 
	(stan == LOCATING && pakiet.data == ackSPriority) && rank < pakiet.src)
	{
		el.priority = pakiet.data;
		el.process = pakiet.src;
		insertElement(&waitQueueS, el);
	}
	else
	{
		packet_t *pkt = malloc(sizeof(packet_t));
		pkt->data = pakiet.data;
		sendPacket(pkt, pakiet.src, ACK_SALKA);
	}
}

void case_ACK_SALKA(packet_t pakiet)
{
	if (stan == LOCATING && pakiet.data == ackSPriority)
	{
		ackCounterS++;
		if (ackCounterS == size - N_SALKA)
		{
			ackCounterS = 0;
			sendPacket(0, przeciwnik, PAIR_JESTEM);
			changeState(ARMING, "ARMING", onStartArming);
		}
	}
}

void case_DEBATE_READY(packet_t pakiet)
{
	opponentReady = TRUE;
}

void onStartArming()
{
	argument = rand() % 3;
	int msgType;
	ackCounterZ = 0;
	if (argument == PINEZKA)
	{
		msgType = REQ_PINEZKA;
		debug("Wybieram *PINEZKĘ*");
	}
	else if (argument == MICHA)
	{
		msgType = REQ_MICHA;
		debug("Wybieram *MICHĘ*");
	}
	else if (argument == SLIPY)
	{
		msgType = REQ_SLIPY;
		debug("Wybieram *SLIPY*");
	}
	int pr = zegar;
	for (int i = 0; i < size; i++)
	{
		if (i != rank)
		{
			packet_t *pkt = malloc(sizeof(packet_t));
			pkt->data = pr;
			sendPacket(pkt, i, msgType);
		}
	}
	ackZPriority = pr;
}

void onStartLocating()
{
	rezSalke = TRUE;
	ackCounterS = 0;
	int pr = zegar;
	for (int i = 0; i < size; i++)
	{
		if (i != rank)
		{
			packet_t *pkt = malloc(sizeof(packet_t));
			pkt->data = pr;
			sendPacket(pkt, i, REQ_SALKA);
		}
	}
	ackSPriority = pr;
}

void onStartDebate()
{
	packet_t *pkt = malloc(sizeof(packet_t));
	sendPacket(0, przeciwnik, DEBATE_READY);
}

void onStartQueueWait()
{
}

void reqZ(packet_t pakiet, argument_t arg, int msgT)
{
	queue_element_t el;
	if (argument == arg && (stan == DEBATE || (stan == ARMING && pakiet.data > ackZPriority) || (stan == ARMING && pakiet.data == ackZPriority && rank < pakiet.src)))
	{
		el.priority = pakiet.data;
		el.process = pakiet.src;
		insertElement(&waitQueueZ, el);
	}
	else
	{
		packet_t *pkt = malloc(sizeof(packet_t));
		pkt->data = pakiet.data;
		sendPacket(pkt, pakiet.src, msgT);
	}
	if (pakiet.src == przeciwnik)
		opponentArgument = arg;
}

void ackZ(packet_t pakiet, argument_t arg, int N)
{
	if (stan == ARMING && argument == arg && pakiet.data == ackZPriority)
	{
		ackCounterZ++;
		if (ackCounterZ >= size - N)
		{
			ackCounterZ = 0;
			changeState(DEBATE, "DEBATE", onStartDebate);
		}
	}
}