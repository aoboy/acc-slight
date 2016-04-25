#include "lib/list.h"
#include "lib/memb.h"
#include "lib/random.h"
#include "contiki-conf.h"
#include "net/rime/rimeaddr.h"

#include "./acc-nodesinfo.h"

#include <string.h>

///=========================================================================/
/* merge the lists.. */
/**
 * @brief merge
 * @param head_one
 * @param head_two
 * @return
 */
static struct nodelist_item*
merge(struct nodelist_item *head_one, struct nodelist_item *head_two) {
    struct nodelist_item *head_three;

    if(head_one == NULL)
        return head_two;

    if(head_two == NULL)
        return head_one;

    if(head_one->slot_gain < head_two->slot_gain) {
        head_three = head_one;
        head_three->next = merge(head_one->next, head_two);
    } else {
        head_three = head_two;
        head_three->next = merge(head_one, head_two->next);
    }

    return head_three;
}

///=========================================================================/
/* preform merge sort on the linked list */
struct nodelist_item *mergesort(struct nodelist_item *head) {
    struct nodelist_item  *head_one;
    struct nodelist_item *head_two;

    if((head == NULL) || (head->next == NULL))
        return head;

    head_one = head;
    head_two = head->next;
    while((head_two != NULL) && (head_two->next != NULL)) {
        head = head->next;
        head_two = head->next->next;
    }
    head_two = head->next;
    head->next = NULL;

    return merge(mergesort(head_one), mergesort(head_two));
}
///=========================================================================/
/* second attempt */
/**
 * @brief sorted_insert
 * @param head_ref
 * @param new_node
 */
static void sorted_insert(struct nodelist_item** head_ref,
              struct nodelist_item* new_node)
{
    struct nodelist_item* current;
    //special case for the head end
    if (*head_ref == NULL || (*head_ref)->slot_gain < new_node->slot_gain)
    {
        new_node->next = *head_ref;
        *head_ref = new_node;
    }else{
        //locate the node before the point of insertion
        current = *head_ref;
        while (current->next!=NULL &&
               current->next->slot_gain > new_node->slot_gain)
        {
            current = list_item_next(current); //current->next;
        }
        new_node->next = current->next;
        current->next = new_node;

    }
}
///=========================================================================/
// sort a linked list : insertion sort
/**
 * @brief insertion_sort
 * @param head_ref
 */
void insertion_sort(struct nodelist_item **head_ref)
{
    // Initialize sorted linked list
    struct nodelist_item *sorted = NULL;

    // go through the given linked list and insert every
    // node to sorted
    struct nodelist_item *current_h = *head_ref;
    while (current_h != NULL){
        // Store next for next iteration
        struct nodelist_item *next = list_item_next(current_h);

        // insert current in sorted linked list
        sorted_insert(&sorted, current_h);

        // Update current
        current_h = next;
    }

    // Update head_ref to point to sorted linked list
    *head_ref = sorted;
}
///=========================================================================/
///=========================================================================/
void bubble_sort(){
    uint8_t i, j;
    //uint8_t llen = list_length(neighs_list);

    for(i = 0; i < num_items; i++){
        struct nodelist_item **X1  = &nodeList[i];

        //for(j = llen-1; j > i; j--){
        for(j = num_items-1; j > i; j--){

            struct nodelist_item **X2  = &nodeList[j];
            struct nodelist_item **tmp = NULL;

            if( ((*X1) != NULL) & ((*X2) != NULL)){
                if((*X1)->slot_gain < (*X2)->slot_gain){
                    *tmp = *X1;
                    *X1  = *X2;
                    *X2  = *tmp;
                }
            }
        }
        //PRINTF("Number steps:%d\n", nsteps);
    }
}
///=========================================================================/
///=========================================================================/
/*
 * The following function moves the top item in the linked list
 * to its correct position.  This is similar to insertion sort.
 * The item that was the second item in the list becomes the
 * top item. The list should contain at least 2 items and
 * from the second item on, the list should already be sorted.
 */
/**
 * @brief move_item
 * @param x
 * @return
 */
static struct nodelist_item *move_item(struct nodelist_item *x ){
    struct nodelist_item *n, *p, *ret;

    p = x;
    n = x->next;
    ret = n;
    while( n != NULL && x->slot_gain > n->slot_gain ) {
        p = n;
        n =  n->next;
    }
    /* we now move the top item between p and n */
    p->next = x;
    x->next = n;
    return ret;
}
///=========================================================================/
/**
 * @brief sort_items
 * @param start
 * @return
 */
struct nodelist_item  *sort_items( struct nodelist_item *start ){
    if( start == NULL ){
        return NULL;
    }
    start->next = sort_items(start->next);
    if( start->next != NULL && start->slot_gain > start->next->slot_gain ) {
        start = move_item( start );
    }
    return start;
}
///=========================================================================/
static void qsort_swap(int index1, int index2){
    struct nodelist_item **tmp_e = NULL;
    struct nodelist_item **X1_e;
    struct nodelist_item **X2_e;
    X1_e = &nodeList[index1];
    X2_e = &nodeList[index2];
    *tmp_e  = *X1_e;
    *X1_e   = *X2_e;
    *X2_e   = *tmp_e;
}
///=========================================================================/
/**
 * @brief quicksort
 * @param first
 * @param last
 */
void quicksort(int first, int last){
    int pivot,j,i;

    //qssteps++;
    if(first<last){
        pivot=first;
        i=first;
        j=last;

        while(i<j){
            struct nodelist_item **X1 = &nodeList[i];
            struct nodelist_item **X2 = &nodeList[pivot];

            while(( (*X1)->slot_gain >= (*X2)->slot_gain) && (i < last) ){
                i++;
                X1 = &nodeList[i];
            }

            X1 = &nodeList[j];

            while( (*X1)->slot_gain < (*X2)->slot_gain ){
                j--;
                X1 = &nodeList[j];
            }
            if(i<j){
                qsort_swap (i,j);
            }
        }
        //print_items();
        qsort_swap (pivot,j);

        quicksort(first,j-1);
        quicksort(j+1,last);
    }
}

