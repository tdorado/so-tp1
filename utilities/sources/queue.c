#include "../queue.h"

//extraído de https://codereview.stackexchange.com/questions/141238/implementing-a-generic-queue-in-c


Queue * newQueue(){
    Queue * q = malloc(sizeof(*q));
    return q;
}

void freeQueue(Queue * q){
    clearQueue(q);
    free(q);
}

void queueInit(Queue *q, size_t memSize)
{
    q->head = q->tail = NULL;
    q->sizeOfQueue = 0;
    q->memSize = memSize;
}

int enqueue(Queue *q, const void *data)
{
    node *newNode = (node *)malloc(sizeof(node));

    if(newNode == NULL)
    {
        return -1;
    }

    newNode->data = malloc(q->memSize);

    if(newNode->data == NULL)
    {
        free(newNode);
        return -1;
    }

    newNode->next = NULL;

    memcpy(newNode->data, data, q->memSize);

    if(q->sizeOfQueue == 0)
    {
        q->head = q->tail = newNode;
    }
    else
    {
        q->tail->next = newNode;
        q->tail = newNode;
    }

    q->sizeOfQueue++;
    return 0;
}

void dequeue(Queue *q, void *data)
{
    if(q->sizeOfQueue > 0)
    {
        node *temp = q->head;
        memcpy(data, temp->data, q->memSize);

        if(q->sizeOfQueue > 1)
        {
            q->head = q->head->next;
        }
        else
        {
            q->head = NULL;
            q->tail = NULL;
        }

        q->sizeOfQueue--;
        free(temp->data);
        free(temp);
    }
}

void queuePeek(Queue *q, void *data)
{
    if(q->sizeOfQueue > 0)
    {
       node *temp = q->head;
       memcpy(data, temp->data, q->memSize);
    }
}

void clearQueue(Queue *q)
{
  node *temp;

  while(q->sizeOfQueue > 0)
  {
      temp = q->head;
      q->head = temp->next;
      free(temp->data);
      free(temp);
      q->sizeOfQueue--;
  }

  q->head = q->tail = NULL;
}

int getQueueSize(Queue *q)
{
    return q->sizeOfQueue;
}
