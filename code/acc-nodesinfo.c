/**
///=========================================================================/
 * KTH Royal Institute of Technology
 *   School of Electrical Engineering
 *      Dept. of Automatic Control
 * ------------------------------------------------------------------------
 * @author: António Gonga <gonga@ee.kth.se>, PhD candidate
 * @date: March 9th 2015
 * @file: dnde-neigh.c
 * @update: Added functionality for asymmetric neighbor discovery.
 * @brief: file contains functions for manipulating the neighbors list
///=========================================================================/ 
 * @info: Auxiliary functions file for a generic deterministic neighbor
 *               discovery with epidemics.
///=========================================================================/ 
*/


#include "lib/list.h"
#include "lib/memb.h"
#include "lib/random.h"
#include "contiki-conf.h"
#include "net/rime/rimeaddr.h"

#include "./acc-nodesinfo.h"

#include <string.h>
#include "acc-nodesinfo.h"

///=========================================================================/
#define  PROBE_ABS(t1) (signed short)(t1) < 0 ?\
    (signed short)(-(t1)) : ((signed short)(t1))
///=========================================================================/
///=========================================================================/
#define CDEBUG 1
#if CDEBUG
#include <stdio.h>
//volatile char *cooja_debug_ptr;
volatile char *cooja_debug_ptr;
#define COOJA_DEBUG_PRINTF(...) \
    do{ char tmp[100]; sprintf(tmp,__VA_ARGS__); cooja_debug_ptr=tmp; } while(0);
#else //COOJA_DEBUG_PRINTF
#define COOJA_DEBUG_PRINTF(...)
#endif //COOJA_DEBUG_PRINTF

///=========================================================================/
///=========================================================================/
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else   //DEBUG
#define PRINTF(...)
#endif //DEBUG
///=========================================================================/
///=========================================================================/
LIST(neighs_list);
#if CONF_NETWORK_SIZE != 0
MEMB(neighs_memb, struct nodelist_item, CONF_NETWORK_SIZE);
#else
MEMB(neighs_memb, struct nodelist_item, 2);
#endif //CONF_NETWORK_SIZE != 0
///=========================================================================/
///=========================================================================/
static volatile uint8_t num_items = 0;
static struct nodelist_item *nodeList[CONF_NETWORK_SIZE];
///=========================================================================/
///=========================================================================/
/**
 * @brief neighs_init
 */
void neighs_init(){
    uint8_t k;
    list_init(neighs_list);
    memb_init(&neighs_memb);

    for(k = 0; k < CONF_NETWORK_SIZE; k++){
        nodeList[k] = NULL;
    }
    
    neighs_flush_all();
}
///=========================================================================/
///=========================================================================/
/**
 * @brief rlocked
 */
static uint8_t rlocked;
#define GET_RLOCK() rlocked++
static void RELEASE_RLOCK(void) {
    if(rlocked == 1) {
        /*if(lock_on) {
      on();
      lock_on = 0;
    }
    if(lock_off) {
      off();
      lock_off = 0;
    }*/
    }
    rlocked--;
}
///=========================================================================/
///=========================================================================/
/**
 * @brief compute_node_period
 * @param node_duty_cycle
 * @return
 */
uint8_t compute_node_period(uint8_t node_duty_cycle){
    uint8_t periodLength = 0;
    if(node_duty_cycle != 0){
        periodLength = (2*100)/ node_duty_cycle;
    }
    return periodLength;
}
///=========================================================================/
///=========================================================================/
/**
 * @brief neighs_get
 * @param nodeid
 * @return
 */
struct nodelist_item *neighs_get(uint8_t nodeid){
    struct nodelist_item *localp = NULL;
    
    localp =list_head(neighs_list);

    for( ; localp != NULL; localp = list_item_next(localp)){
        if (localp->node_id == nodeid){
            return localp;
        }
    }
    return NULL;
}
///=========================================================================/
///=========================================================================/
/**
 * @brief neighs_getlist
 * @return
 */
struct nodelist_item *neighs_getlist(/*nodelist_item *list_head*/){
    //*list_head = list_head(neighs_list);
    return list_head(neighs_list);
}
///=========================================================================/
///=========================================================================/
/**
 * @brief neighs_add_itself
 */
void neighs_add_itself(){

    struct nodelist_item *ais = NULL;
    
    ais = neighs_get(rimeaddr_node_addr.u8[0]);
    
    if(ais == NULL){
        
        //we add a new element..
        ais = memb_alloc(&neighs_memb);

        if(ais != NULL){
            ais->node_id    = rimeaddr_node_addr.u8[0];


            ais->updated    = 0;
            ais->hopcount   = 0;
            ais->offset     = 0;
            ais->offsetj    = 0;
            ais->max_txhop2 = 0;
            ais->tconfirmed = 0;
            ais->tknown     = 0;
            ais->next       = NULL;

            ais->t_anchor   = 0;
            ais->j_factor   = 0;
            ais->period     = get_node_period();

            //add first reference here...
            if(nodeList[num_items] == NULL){
                nodeList[num_items] = ais;
                num_items++;
            }

            //add new element to the list..
            list_add(neighs_list, ais);

            return;
        }
    }
}
///=========================================================================/
///=========================================================================/
/**
 * @brief get_fraction_nodes
 * @return
 */
uint8_t neighs_num_nodes(){
    uint8_t frac_neigh = 0;

    struct nodelist_item *lpf = NULL;
    
    lpf = list_head(neighs_list);
    
    for( ; lpf != NULL; lpf = list_item_next(lpf)){
        frac_neigh = frac_neigh + 1;
    }

    return frac_neigh;
}
///=========================================================================/
///=========================================================================/
/**
 * @brief neighs_xhops
 * @param xhops
 * @return
 */
uint8_t neighs_xhops(uint8_t xhops){
    uint8_t frac_neigh = 0;

    struct nodelist_item *lpf = NULL;

    lpf = list_head(neighs_list);
    
    for( ; lpf != NULL; lpf = list_item_next(lpf)){
        
        if (1 /*lpf->hopcount == xhops*/){
            frac_neigh = frac_neigh + 1;
        }
    }
    
    return frac_neigh;
}
///=========================================================================/
///=========================================================================/
/**
 * @brief neighs_h2_indirect
 * @return
 */
uint8_t neighs_h2_indirect(){
    uint8_t frac_neigh = 0;
    uint16_t t1, t2;

    struct nodelist_item *lpf = NULL;

    lpf = list_head(neighs_list);
    
    
    for( ; lpf != NULL; lpf = list_item_next(lpf)){

        t1 = lpf->tknown;
        t2 = lpf->tconfirmed;

        if ((lpf->hopcount == 1) && ((t2-t1) > 0)){
            frac_neigh = frac_neigh + 1;
        }
    }
    
    return frac_neigh;
}
///=========================================================================/
///=========================================================================/
/**
 * @brief neighs_full
 * @return
 */
uint8_t neighs_all_found(){
    uint8_t i = 0;

    i = list_length(neighs_list);

    if (i ==num_neighbors){
        return 1;
    }
    return 0;
}
///=========================================================================/
///=========================================================================/
/**
 * @brief neighs_remove
 * @param nodeid
 */
void neighs_remove(uint8_t nodeid){

    struct nodelist_item *n2r = NULL;
    
    n2r = neighs_get(nodeid);
    
    if(n2r != NULL){
        list_remove(neighs_list, n2r);
        memb_free(&neighs_memb, n2r);
    }
}
///=========================================================================/
///=========================================================================/
/**
 * @brief neighs_flush_all
 */
void neighs_flush_all(void){
    uint8_t k;
    struct nodelist_item *n2fa = NULL;

    num_items = 0;
    for( k = 0; k < CONF_NETWORK_SIZE; k++){
        nodeList[k] = NULL;
    }

    while(1){

        n2fa = list_pop(neighs_list);

        if(n2fa != NULL) {
            list_remove(neighs_list, n2fa);
            memb_free(&neighs_memb, n2fa);
        }else {
            ///add first entry here ??
            neighs_add_itself();

            break;
        }
    } ///end of while(1)
}
///=========================================================================/
///=========================================================================/
/**
 * @brief time_neighbor_anchor
 * @param n_item
 * @return
 */
uint16_t time_neighbor_anchor(struct nodelist_item *n_item){
    uint8_t  periodL = n_item->period;
    uint16_t ta = n_item->t_anchor + n_item->offset + periodL*n_item->j_factor;

    return ta;
}
///=========================================================================/
///=========================================================================/
/**
 * @brief neighs_jfactor_update:
 */
void neighs_jfactor_update(){
    struct nodelist_item *localp = NULL;
    
    localp = list_head(neighs_list);
    
    for( ; localp != NULL; localp = list_item_next(localp)){
        if (localp->node_id != rimeaddr_node_addr.u8[0]){

            //local variable enables tracking offset to neighbors
            uint16_t ta_i = get_anchor_time();
            uint16_t ta_j = time_neighbor_anchor(localp);

            uint16_t tp_i   = get_node_period();
            uint16_t tp_j   = localp->period;

            if(ta_i > ta_j){
                if (tp_i <= tp_j){
                    localp->j_factor = localp->j_factor + 1;
                }else{
                    localp->j_factor = localp->j_factor + tp_i/tp_j;
                }
            }

            ta_j = time_neighbor_anchor(localp);

            //only update if nodes have different periods...
            /**According to Searchlight, two nodes with the same period
              *tp_i,j always have their anchors at the same offset.
              */
            if (/*(localp->hopcount == 1) &&*/ (ta_j > ta_i) &&  tp_j != tp_i){
                uint8_t offsetnode = ta_j - ta_i;
                //set the new offset for broadcast.
                localp->offsetj    = offsetnode;
            }
        }
    }
}
///=========================================================================/
///=========================================================================/
static void print_gains(void){
    uint8_t i;
    COOJA_DEBUG_PRINTF("%d Items: ", num_items);

    for(i = 0; i < num_items; i++){
        struct nodelist_item *item = nodeList[i];
        if(item != NULL){
            COOJA_DEBUG_PRINTF("(ID:%d,G:%2u, %d) ",item->node_id,
                               item->slot_gain, item->offsetj);
        }
    }
    COOJA_DEBUG_PRINTF("\n");
}
///=========================================================================/
///=========================================================================/
static void bubble_sort(){
    uint8_t i, j, nsteps=0;
    uint8_t llen = list_length(neighs_list);

    for(i = 0; i < num_items; i++){

        for(j = llen-1; j > i; j--){
            struct nodelist_item **X1 = &nodeList[i];
            struct nodelist_item **X2 = &nodeList[j];
            struct nodelist_item **tmp= NULL;

            if( ((*X1) != NULL) & ((*X2) != NULL)){
                if((*X1)->slot_gain < (*X2)->slot_gain){
                    *tmp = *X1;
                    *X1  = *X2;
                    *X2  = *tmp;
                }
            }
            nsteps++;
        }
        //print_gains ();
        //PRINTF("Number steps:%d\n", nsteps);
    }
}
///=========================================================================/
///=========================================================================/
static uint8_t compute_m_t0_t( uint8_t t1){
    uint8_t m_t0_t = 0;
    struct nodelist_item *hp1 = list_head(neighs_list);
    for(; hp1 != NULL; hp1 = list_item_next(hp1)){
        //if their anchors are before T1, increment number of opportunities
        if(hp1->offsetj < t1){
            m_t0_t++;
        }
    }
    return m_t0_t;
}
///=========================================================================/
///=========================================================================/
static uint16_t get_slot_gain(void){
  uint16_t gains_slots = 0;
  
    struct nodelist_item *hp = list_head(neighs_list);


    for(; hp != NULL; hp = list_item_next(hp)){
	if(hp->node_id != rimeaddr_node_addr.u8[0]){
	    gains_slots = gains_slots + (hp->tmp_div*hp->spat_sim);
	}
    }   
  return gains_slots;
}
///=========================================================================/
///=========================================================================/
/**
 * @brief compute_slot_gain:
 * @param p_offset
 */
void compute_slot_gain(uint8_t p_offset){

    struct nodelist_item *hl = list_head(neighs_list);

    for(; hl != NULL; hl = list_item_next(hl)){

        //if
        if((hl->offsetj > p_offset) &&
                (hl->node_id != rimeaddr_node_addr.u8[0])){

            uint8_t detltaT = p_offset - (uint8_t)hl->offsetj;
            uint8_t tmp_div = compute_m_t0_t(hl->offsetj);

            hl->tmp_div = tmp_div;

            uint16_t slot_gain = get_slot_gain();
	    

            //compute SLOT GAIN..
            hl->slot_gain = (slot_gain*200)/detltaT;
        }else{
            hl->slot_gain = 0;
        }

    }//end of for.
    
    //sort time slots..
    if(num_items > 1){
        //quicksort(0, num_items);
        bubble_sort ();
        //print Slot Gains here..
        print_gains();
    }
}
///=========================================================================/
///=========================================================================/
/**
 * @brief isthere_anchor
 * @param hops_away
 * @param curr_time
 * @return
 */
uint8_t isthere_anchor(uint8_t topKslots, uint16_t curr_time){
    uint8_t k = 0;
    uint16_t time_anchor = 0;

    for(k = 0; (k < num_items) && (k < topKslots); k++){
        struct nodelist_item *lpf = nodeList[k];
        time_anchor = time_neighbor_anchor(lpf);
        if((time_anchor == curr_time) || (time_anchor == curr_time + 1)){
            return 1;
        }
    }

    return 0;
}
///=========================================================================/
///=========================================================================/
#define N_ABS(my_val) (((signed short)(my_val)) < 0) ? 0-(my_val) : (my_val)


///=========================================================================/
///=========================================================================/
void 
add_neighbor(uint8_t src_id, int16_t offset, uint8_t period, uint8_t hopc){
    struct nodelist_item *nli = NULL;

    nli = neighs_get(src_id);
    
    if(nli == NULL){
        //we add a new element..
        nli = memb_alloc(&neighs_memb);

        if(nli != NULL){
            //set the id of this node
            nli->node_id    = src_id;
            //extract the hopcount..
            nli->hopcount   = hopc;
            //we do not know yet :)
            nli->max_txhop2 = 0;

            //get the time of reception
            nli->tknown     = get_discovery_time();
            //equal time of confirmation if node is discovered
            //without help of epidemics
            nli->tconfirmed = get_discovery_time();

            //if node is received as hop=2, jfactor is what allows
            //to locate it.. we explain later..
            nli->j_factor   = 0;
            nli->offset     = offset;
            nli->offsetj    = offset;

            //what is the anchor time when this node was received
            nli->t_anchor   = get_anchor_time();

            //retrieve the period of node.
            nli->period = period;

            nli->next     = NULL;

            /** add reference here..*/
            if(nodeList[num_items] == NULL){
                nodeList[num_items] = nli;
                //increment number of nodes
                num_items++;
            }

            //add new element to the list..
            list_add(neighs_list, nli);
        }
    }else{
        //test if it's 1 hop node.. if so reset it..
        if(hopc == 1){
            nli->hopcount   = hopc ;
            nli->tconfirmed = get_discovery_time();
            nli->offset     = offset;
            nli->offsetj    = offset;

            //generic discovery
            nli->j_factor   = 0;
            nli->t_anchor   = get_anchor_time();

            COOJA_DEBUG_PRINTF("%u UPDT_i(%u) offset:%2u\n", rimeaddr_node_addr.u8[0], src_id, offset);
        }
    }
}
///=========================================================================/
///=========================================================================/
/**
 * @brief neighs_register: receive a packet whose payload contains nodes
 *                         that are 1 hop and 2 hops away.
 * @param pkt_hdr: The pointer to the packet received buffer
 * @return
 */
uint8_t 
neighs_register(data_packet_t *pkt_hdr, int pldLen, uint8_t probe_counter){
    uint8_t k = 0, sndr_id, sndr_p;
    int16_t offsetH1, offsetH2;
    uint8_t spatSim = 1;

    //extract sender ID and period
    sndr_id = pkt_hdr->src_id;
    
    //period
    sndr_p  = pkt_hdr->period;
    
    //computes the offset to the sender of the packet.
    int16_t ownOffset  = ((int16_t)probe_counter - pkt_hdr->offset);
    
    //uint8_t periodLength = compute_node_period(pkt_hdr->energy);
    
    offsetH1 = ownOffset;
    
    //if the offset is negative, we compute the positive offset by summing
    //the sender's period length.
    if (ownOffset < 0){
        offsetH1 = pkt_hdr->period + ownOffset;

        //COOJA_DEBUG_PRINTF("dc:%u\n",compute_node_period(pkt_hdr->energy));
    }
    
    struct nodelist_item *srcL = NULL;
    srcL = neighs_get(sndr_id);
    if((srcL == NULL) && sndr_id != rimeaddr_node_addr.u8[0] ){
        //add the sender node here..
        add_neighbor(sndr_id, offsetH1, sndr_p, 1);
    }else{
        //node already exists..
        spatSim++;
    }

    
    //go though all items in the packet and add them accordingly..
    for ( k = 0; k < pldLen; k++){

        uint8_t dpos = k*DATA_ITEM_LEN;

        struct data_item_t *ditem = (struct data_item_t*)(&pkt_hdr->data[dpos]);

        //filter packets based on hop-count number, remove also my id
        if ((ditem->node_id != 0 && (ditem->dc_hopc & HOPC_MASK) ) &&
                (ditem->node_id != rimeaddr_node_addr.u8[0]) &&
                (ditem->dc_hopc  <= MAX_HOPCOUNT)){

            struct nodelist_item *nli = NULL;

            //check if the nodeID is already registered/received..
            nli = neighs_get(ditem->node_id);

            if(nli == NULL){
                //add a new node here..

                //compute offset of a hop 2 neighbor.
                offsetH2    = ( ditem->offset + ownOffset);

                if(offsetH2 < 0){
                    offsetH2 =  nli->period + offsetH2;
                }

                add_neighbor(ditem->node_id, offsetH2, ditem->period, 2);

                COOJA_DEBUG_PRINTF("%u Epid(h2)-> %u offset:%2d\n",rimeaddr_node_addr.u8[0], ditem->node_id, offsetH2);

            }else{
                //node already exists..
                spatSim++;
            }

        }
    } // for ( k = 0;

    //update spatial similarity
    struct nodelist_item *nli = NULL;
    nli = neighs_get(sndr_id);
    if(nli != NULL){
        nli->spat_sim = spatSim;
    }

    return 0;
}
///=========================================================================/
///=========================================================================/
/**
 * @brief neighs_add2payload
 * @param data_ptr
 * @param isAnchor
 * @param probe_offset
 * @return
 */
uint8_t 
neighs_add2payload(uint8_t *data_ptr, uint8_t isAnchor, uint8_t probe_offset){
    uint8_t pkt_offset = 0, hopc=0;

    struct nodelist_item *headl = NULL;
    
    headl = list_head(neighs_list);
    
    for(;   headl != NULL;    headl = list_item_next(headl)){

        if(headl->node_id != 0 && headl->hopcount < MAX_HOPCOUNT){

            struct data_item_t *d2a = (struct data_item_t *)(&data_ptr[pkt_offset]);

            d2a->node_id       = headl->node_id;

            d2a->dc_hopc       = headl->hopcount + 1;
            d2a->period        = headl->period;

            d2a->offset        = (uint8_t)(0x00FF &headl->offsetj); //headl->offset;

            pkt_offset = pkt_offset + DATA_ITEM_LEN;

            //COOJA_DEBUG_PRINTF("Added %u,%u\n", d2a->node_id, hopc);
        }
    }
    if(pkt_offset){
        return pkt_offset;
    }
    //this is a serious error.. :(
    COOJA_DEBUG_PRINTF("AddedNothing\n");
    return 0;
}
///=========================================================================/
///=========================================================================/
/**
 * @brief log2 - computes the logarithm base 2, i.e, log(N,2) of a number N
 * @param n
 * @return
 */
uint8_t log2_n(uint16_t n){
    int bits = 1;
    int b;
    for (b = 8; b >= 1; b/=2){
        int s = 1 << b;
        if (n >= s){
            n >>= b;
            bits += b;
        }
    }
    bits = bits - 1;
    return bits;
}
///=========================================================================/
///=========================================================================/
/**
 * @brief random_int
 * @param size_num
 * @return
 */
uint16_t random_int(uint16_t size_num){

    uint16_t rand_num = random_rand();

    if(size_num == 0){

        return 0;
    }

    uint16_t val = (65535 / size_num);
    
    if(rand_num < val){
        
        PRINTF("Hereee: %u\n", rand_num);
        return 0;

    }else{

        uint16_t k;

        for(k = 1; k < size_num; k++){

            if (rand_num >= k*val && rand_num <= (k+1)*val){

                return k;
            }
        }
    }
    return 0;
}
///=========================================================================/
///=========================================================================/
/**
 * @brief random_int
 * @param size_num
 * @return
 */
uint16_t randomint_between(uint16_t num_a, uint16_t num_b){

    ///Compute the difference so as we have the size of
    /// the sample...
    uint16_t size_num = num_b - num_a;

    ///generate a random value between 0 and 65535
    uint16_t rand_num = random_rand();

    if(size_num == 0){
        COOJA_DEBUG_PRINTF("DC1:%u\n",size_num);
        return 0;
    }

    uint16_t val = (65535 / size_num);
    if ( rand_num < val){
        return num_a;
    }else{
        uint16_t k;
        for(k = 1; k < size_num; k++){
            if (rand_num >= k*val && rand_num <= (k+1)*val){
                return (num_a + k);
            }
        }
    }
    return 0;
}
///=========================================================================/
///=========================================================================/
/**
 * @brief calc_sqrt: babylon method for calculating the square root.
 * @param num2comp : the value to compute the square root..
 * @return         : the square root value of 'num2comp'
 */
uint16_t calc_sqrt(const uint16_t num2comp){
    uint8_t i = 0;

    uint16_t x1 = 0, sqrt_val = 100;

    for(i = 0; i < 15; i++){
        sqrt_val = (sqrt_val + num2comp/sqrt_val)/2;

        if(i > 0 && x1 == sqrt_val){
            break;
        }else{
            x1 = sqrt_val;
        }
    }

    return  sqrt_val;
}
///=========================================================================/
///=========================================================================/
///=========================================================================/
///=========================================================================/
void quicksort(uint8_t first, uint8_t last){
    uint8_t pivot,j,i;

    //qssteps++;
    if(first<last){
        pivot=first;
        i=first;
        j=last;

        while(i<j){
            struct nodelist_item **X1 = &nodeList[i];
            struct nodelist_item **X2 = &nodeList[pivot];
            if((*X1 != NULL) && (*X2 != NULL)){
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
                    struct nodelist_item **tmp= NULL;
                    X1 = &nodeList[i];
                    X2 = &nodeList[j];
                    *tmp  = *X1;
                    *X1   = *X2;
                    *X2   = *tmp;
                }
            }else{
                break;
            }
        }

        struct nodelist_item **X1 = &nodeList[pivot];
        struct nodelist_item **X2 = &nodeList[j];
        struct nodelist_item **tmp= NULL;

        //print_items();

        *tmp = *X1;
        *X1  = *X2;
        *X2  = *tmp;

        quicksort(first,j-1);
        quicksort(j+1,last);

    }
}


/**
 * replacing existing fixed networks with cheap, flexible and even mobile low-power wireless networks, constitutes the
cornerstone to enable the envisioned Internet of Things (IoT).*/
