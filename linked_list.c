#include "linked_list.h"
#include <stdio.h>

pthread_mutex_t locker = PTHREAD_MUTEX_INITIALIZER;

void list_init(Node** head, size_t size)
{
    mem_init(size);
    *head = NULL;
    printf("List initialized successfully!\n");
}

void list_insert(Node** head, uint16_t data)
{
    Node* newNode = mem_alloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;

    pthread_mutex_lock(&locker);

    Node* walker = *head;
    if(walker)
    {
        while (walker->next)
        {
            walker = walker->next;
        }
        walker->next = newNode;
    }
    else
        *head = newNode;
    pthread_mutex_unlock(&locker);
    // printf("Node inserted successfully!\n");
}

void list_insert_after(Node* prev_node, uint16_t data)
{
    Node* newNode = mem_alloc(sizeof(Node));
    newNode->data = data;
    pthread_mutex_lock(&locker);
    if(prev_node)
    {
        if (prev_node->next)
        {
            newNode->next = prev_node->next;
        }
        else
            newNode->next = NULL;
        prev_node->next = newNode;
    }
    else
        printf("invalid prev_node");
    pthread_mutex_unlock(&locker);
}

void list_insert_before(Node** head, Node* next_to, uint16_t data)
{
    Node* newNode = mem_alloc(sizeof(Node));
    newNode->data = data;
    newNode->next = next_to;
    pthread_mutex_lock(&locker);
    if(*head == next_to)
    {
        *head = newNode;
        newNode->next = next_to;   
    }
    else
    {
        Node* walker = *head;
        while(walker->next && walker->next != next_to)
        {
            walker = walker->next;
        }
        walker->next = newNode;
    }
    pthread_mutex_unlock(&locker);
}

void list_delete(Node** head, uint16_t data)
{   
    pthread_mutex_lock(&locker);
    Node* toDel = *head;
    if(toDel == NULL)
        printf("No nodes to delete!");
    else if(toDel->data == data)
    {
        if(toDel->next)
            *head = toDel->next;
        else
            *head = NULL;
        mem_free(toDel);
    }
    else
    {
        Node* walker = *head;
        while(walker->next && walker->next->data != data)
            // if(walker->next->data == data)
            //     break;
            walker = walker->next;
        toDel = walker->next;

        if(toDel->next)
            walker->next = toDel->next;
        else
            walker->next = NULL;

        mem_free(toDel);
    }
    pthread_mutex_unlock(&locker);
}

Node* list_search(Node** head, uint16_t data)
{
    Node* walker = *head;

    while(walker->next && walker->data != data)
    {
        walker = walker->next;
    }

    if(walker->data == data)
        return walker;
    return NULL;
}

void list_display(Node** head) 
{
    list_display_range(head, NULL, NULL);
}

void list_display_range(Node** head, Node* start_node, Node* end_node)
{
    Node* from = start_node;
    Node* to = end_node;

    if(to)
        to = to->next;

    if(!start_node)
        from = *head;

    printf("[");
    while(from != to)
    {
        printf("%d",from->data);
        from = from->next;
        if(from != to)
            printf(", ");
    }
    printf("]");
}

int list_count_nodes(Node** head)
{
    int counter = 0;
    pthread_mutex_lock(&locker);
    Node* current = *head;
    if(current != NULL && current->next != NULL)
    {
        counter++;
        while(current->next)
        {
            counter++;
            current = current->next;
        }
    }
    pthread_mutex_unlock(&locker);
    printf("NODES: %d \n", counter);
    return counter;
}

void list_cleanup(Node** head)
{
    if(*head == NULL)
        return;
    else
    {
        Node* walker = *head;

        while (walker)
        {
            Node* next = walker->next;
            
            mem_free(walker);

            walker = next;
        }
    }
    mem_deinit(*head);
    *head = NULL;
}


// int main()
// {
//     Node* head = NULL;
//     list_init(&head, 0);
//     return 0;
// }