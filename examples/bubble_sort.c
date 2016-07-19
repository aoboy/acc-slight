

/**
 * \file
 *         Testing the bubble sort for Contiki
 * \author
 *         Antonio Gonga <gonga@kth.se> :::::
	   that is it....
 */



#include "contiki.h"
#include "net/rime.h"
#include "random.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>
///=========================================================================/
//small change

struct bubble{
    struct bubble *next;
    uint8_t data;
};
///=========================================================================/
///=========================================================================/
#define MAX_ITEMS2SORT 10
///=========================================================================/
///=========================================================================/
LIST(datalist);
MEMB(datalistmemb, struct bubble, MAX_ITEMS2SORT);

//struct bubble data_ptr[MAX_ITEMS2SORT];
struct bubble *data_2ptr[MAX_ITEMS2SORT];
struct bubble *ptr_2data[MAX_ITEMS2SORT];

static volatile uint8_t num_items = 0;
///=========================================================================/
///=========================================================================/
PROCESS(example_bubblesort_process, "SORTING Algorthims Example");
AUTOSTART_PROCESSES(&example_bubblesort_process);
///=========================================================================/
///=========================================================================/
static void initialize(void){
    uint8_t k;
    list_init(datalist);
    memb_init(&datalistmemb);

    //initialize pointer list
    for( k = 0; k < MAX_ITEMS2SORT; k++){
        data_2ptr[k] = NULL;
        ptr_2data[k] = NULL;
    }
}
///=========================================================================/
///=========================================================================/
static void add_item(int item_data){
    struct bubble *item = NULL;
    
    item = memb_alloc(&datalistmemb);
    
    if(item != NULL){
        ((struct bubble*)item)->data = item_data;
        item->next = NULL;

        if(data_2ptr[num_items] == NULL){
            data_2ptr[num_items] = item;
            //increment number of itemsHi Prof,
            num_items++;
        }

        list_add(datalist, item);
    }
}
///=========================================================================/
///=========================================================================/
static void free_item(void* item){
    if(item != NULL){
        list_remove(datalist, item);
        memb_free(&datalistmemb, item);
    }
}
///=========================================================================/
///=========================================================================/
static void free_item_all(void){
    uint8_t k;
    for( k = 0; k < MAX_ITEMS2SORT; k++){
        data_2ptr[k] = NULL;
        ptr_2data[k] = NULL;
    }

    struct bubble *headl = NULL;

    while(1){
        struct bubble *headl=list_pop(datalist);
        if(headl != NULL){
            list_remove(datalist, headl);
            memb_free(&datalistmemb, headl);
        }else{
            num_items = 0;
            //exit the loop
            break;
        }
    }
}
///=========================================================================/
///=========================================================================/
static void print_items(void){
    struct bubble *hl =  list_head(datalist);

    printf("items: ");
    /*for(; hl != NULL; hl = list_item_next(hl)){
        printf("%2d ", hl->data);
    }*/
    uint8_t k;
    for(k = 0; k < num_items; k++){
        struct bubble *item = data_2ptr[k];
        printf("%2d ", item->data);
    }
    printf("\n");
}
///=========================================================================/
///=========================================================================/
/**
 *
 *
 */
///=========================================================================/
static int qssteps = 0;

void quicksort(int first,int last){
    int pivot,j,temp,i;

    //qssteps++;
    if(first<last){
        pivot=first;
        i=first;
        j=last;

        qssteps++;

        while(i<j){
            struct bubble **X1 = &data_2ptr[i];
            struct bubble **X2 = &data_2ptr[pivot];
            while(( (*X1)->data >= (*X2)->data) && (i < last) ){
                i++;
                X1 = &data_2ptr[i];
            }

            X1 = &data_2ptr[j];
            while( (*X1)->data < (*X2)->data ){
                j--;
                X1 = &data_2ptr[j];
            }
            if(i<j){
                X1 = &data_2ptr[i];
                X2 = &data_2ptr[j];
                struct bubble **tmp= NULL;
                *tmp  = *X1;
                *X1  = *X2;
                *X2  = *tmp;
            }
        }

        struct bubble **X1 = &data_2ptr[pivot];
        struct bubble **X2 = &data_2ptr[j];
        struct bubble **tmp= NULL;

        //print_items();

        *tmp = *X1;
        *X1  = *X2;
        *X2  = *tmp;

        quicksort(first,j-1);
        quicksort(j+1,last);

    }
}
///=========================================================================/
///=========================================================================/
static void bubble_sort(){
    uint8_t i, j, nsteps=0;
    uint8_t llen = list_length(datalist);

    for(i = 0; i < num_items; i++){

        for(j = llen-1; j > i; j--){
            /*struct bubble **X1 = &data_2ptr[i];
            struct bubble **X2 = &data_2ptr[j];
            struct bubble **tmp= NULL;

            if( ((*X1) != NULL) & ((*X2) != NULL)){
                if((*X1)->data < (*X2)->data){
                    *tmp = *X1;
                    *X1  = *X2;
                    *X2  = *tmp;
                }
            }*/
            struct bubble *X1 = data_2ptr[i];
            struct bubble *X2 = data_2ptr[j];
            struct bubble *tmp= NULL;

            if( ((X1) != NULL) & ((X2) != NULL)){
                if(X1->data < X2->data){
                    tmp = X1;
                    X1  = X2;
                    X2  = tmp;
                }
            }
            nsteps++;
        }
        //print_items();
        //printf("Number steps:%d\n", nsteps);
    }
}

///=========================================================================/
///=========================================================================/
PROCESS_THREAD(example_bubblesort_process, ev, data)
{
    static struct etimer et;

    static rtimer_clock_t t0, t1;

    static uint8_t k, data_i;

    PROCESS_BEGIN();

    initialize();

    for(k = 0; k < MAX_ITEMS2SORT; k++){
        data_i = (random_rand()%255);
        add_item(data_i);
    }

    print_items();

    printf("Run Bubble Sort\n");
    t0 = RTIMER_NOW();
    bubble_sort();
    t1 = RTIMER_NOW();
    print_items();

    printf("Cost: %u\n", t1-t0);

    free_item_all ();

    for(k = 0; k < MAX_ITEMS2SORT; k++){
        data_i = random_rand()%255;
        add_item(data_i);
    }

    print_items();

    printf("Run QuickSort\n");
    t0 = RTIMER_NOW();

    quicksort(0, MAX_ITEMS2SORT-1);

    t1 = RTIMER_NOW();

    print_items();

    printf("num items: %2d, steps:%d\n",MAX_ITEMS2SORT, qssteps);

    printf("Cost: %u\n", t1-t0);

    /*while(1) {

    // Delay 2-4 seconds
    etimer_set(&et, random_rand() % (CLOCK_SECOND * 4));

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }*/

    PROCESS_END();
}
///=========================================================================/
///=========================================================================/
/** This section implements bubble_sort of a linked list in Contiki 
 *
 */
///=========================================================================/
///=========================================================================/

