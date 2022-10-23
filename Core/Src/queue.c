#include "queue.h"

void initQueue(queue_handler *ptr_queue, array queue, uint16_t MaxElements)
{
    memset(queue, 0, MaxElements);
    
    ptr_queue->capacity = MaxElements; 
    ptr_queue->queue    = queue;
    
    makeEmpty(ptr_queue);
}

void makeEmpty(queue_handler *ptr_queue)
{
    ptr_queue->front = 0;
    ptr_queue->rear  = 0;
}

bool isEmpty(queue_handler *ptr_queue)
{
    return (ptr_queue->front == ptr_queue->rear);
}

bool isFull(queue_handler *ptr_queue)
{
    return (((ptr_queue->rear + 1) %  ptr_queue->capacity) == ptr_queue->front);
}

bool enQueue(queue_handler *ptr_queue, uint32_t element)
{
    if (isFull(ptr_queue))
    {
        return false;
    }
    
    (ptr_queue->queue)[ptr_queue->rear] = element;
    ptr_queue->rear = (ptr_queue->rear + 1) %  ptr_queue->capacity;
    
    return true;
}

bool deQueue(queue_handler *ptr_queue, uint32_t *element)
{
    if (isEmpty(ptr_queue))
    {
        return false;
    }
    
    *element = (ptr_queue->queue)[ptr_queue->front];
    ptr_queue->front = (ptr_queue->front + 1) %  ptr_queue->capacity;
    
    return true;
}
