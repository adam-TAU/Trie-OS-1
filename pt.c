#include "os.h"
#include <stdlib.h>
#define 

/* Submitter: Adam
 * Operating Systems, Tel Aviv University, 2022A.
 * ==============================================
 * An implementation of a Multi-Level (Trie) Page Table
 */




/**************** AUXILIARY FUNCTION DECLARATIONS ******************/


/* Given a page table's pointer, a desired VPN, and a PPN that is a node that takes part in the path of VPN in the Page Table's Trie,
 * return the address of the node that continues VPN in the path.
 * Arguments:
 *		vpn: the virtual page number that we're searching for in the Trie path
 * 		ppn: the physical page number of the current node that we are at in the path traversal
 * 		depth: the depth of the search - the amount of nodes we have already gone through in the search. That is crucial since it's what determines where the next entry that belongs to VPN inside the current PPN is. (using bit wise operations on VPN with Depth to extract the next entry's position)
 */
static uint64_t* get_entry(uint64_t vpn, uint64_t ppn, size_t depth);

/* Given an entry's address and its physical page number, nullify the value (remove the mapping) of the entry,
 * and free the Page of the node it relies in if it has gotten empty of mappings. */ 
static void remove_mapping(uint64_t node_ppn, uint64_t* entry_pa_virt)

/* Given a node's ppn, determine of its empty of any VPN's mapping. */
static int is_node_empty(uint64_t node_ppn);

/* Given a node's ppn, free its Physical Page*/
static void free_node(uint64_t node_ppn);

/* If a node is empty of any VPN's mapping, free its Physical Page */
static void free_node_safe(uint64_t node_ppn);


/**************** AUXILIARY FUNCTION DEFINITIONS ******************/


static uint64_t* get_entry(uint64_t vpn, uint64_t node_ppn, size_t depth) {
	
	uint64_t* node_ppn_virt = (uint64_t*) phys_to_virt(node_ppn);
	uint64_t offset = vpn & ( 0x1ff000 << ((4 - depth) * 9) ); // at path traversal's depth: depth, the next 9 bits in VPN that refer to the next entry in the current PPN, are at a location of [12 ... 20] + ((4 - depth) * 9)
	uint64_t* entry_pa_virt = node_ppn_virt + offset;
	
	return entry_pa_virt;
}


static void remove_mapping(uint64_t node_ppn, uint64_t* entry_pa_virt) {
	/* nullifythe entry_pa_virt's value somehow */
	free_node_safe(node_ppn);
}


static int is_node_empty(uint64_t node_ppn) {
	/* FIND A WAY TO DETERMINE IF ENTRIES ARE EMPTY */
	uint64_t* node_ppn_virt = (uint64_t*) phys_to_virt(node_ppn);
	return (node_ppn_virt == NULL);
}


static void free_node(uint64_t node_ppn) {
	free_page_frame(node_ppn);
}


static void free_node_safe(uint64_t node_ppn) {
	if (is_node_empty(node_ppn)) {
		free_node(node_ppn);
	}
}



/**************** MAIN MECHANISM's DEFINITIONS ******************/

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
	
	uint64_t current_node_ppn = pt;
	
	for (size_t depth = 0; depth < 5; depth++) { /* Personal calculations showed that the Page Table should be of height 5 */
		uint64_t* entry = get_entry(vpn, current_node_ppn, depth);
		
		if (*entry 
	}
}




uint64_t page_table_query(uint64_t pt, uint64_t vpn) {
	uint64_t* pa_virt = get_entry(vpn, pt, 0);
	if (pa_virt) return 0;
	return 0;
}
