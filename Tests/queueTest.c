#include "../utilities/queue.h"
#include "../Testing/testing_suite.h"

//extraído de https://codereview.stackexchange.com/questions/141238/implementing-a-generic-queue-in-c
//modificado para que acompañe la estructura del framework

void initialize_test();
void peek_value_test();
void queue_erase_test();

int main()
{
    create_suite("Testing the Queue");

    add_test(initialize_test);
    add_test(peek_value_test);
    add_test(queue_erase_test);

    run_suite();
    
    clear_suite();

    return 0;
}

void initialize_test(){
    int aux = 0;
    Queue q;
    queueInit(&q, sizeof(int));
    aux = q.sizeOfQueue;
    clearQueue(&q);
    assert_true(aux == 0);
}

void peek_value_test(){
    int val = 3, val2 = 0;
    Queue q;
    queueInit(&q, sizeof(int));
    enqueue(&q, &val);
    queuePeek(&q, &val2);
    clearQueue(&q);
    assert_true(val2 == 3);
}

void queue_erase_test(){
    int val = 1, aux = 0;
    Queue q;
    queueInit(&q, sizeof(int));
    enqueue(&q, &val);
    dequeue(&q, &val);
    aux = q.sizeOfQueue;
    clearQueue(&q);
    assert_true(aux == 0);
}