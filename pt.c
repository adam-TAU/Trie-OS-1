#include "os.h"
#include <stdlib.h>
#define bool int
#define true 1
#define false 0
/* Submitter: Adam
 * Operating Systems, Tel Aviv University, 2022A.
 * ==============================================
 * An implementation of a Multi-Level (Trie) Page Table
 */




/**************** AUXILIARY FUNCTION DECLARATIONS ******************/

/* Given a page table's pointer, a VPN, and its desired (valid (!= NO_MAPPING)) PPN mapping, insert the mapping into the page table */
static void insert_mapping(uint64_t pt, uint64_t vpn, uint64_t ppn);

/* Removes a page table mapping, if it exists. In case it does exist, it also cares to free the nodes that held the entries of the VPN's path in the Page Table's,
 * if nodes were emptied due to the removal of the mapping - this doesn't apply to the root node of the Page Table tho. */
static bool remove_mapping(uint64_t *current_node_ppn, uint64_t vpn, size_t depth);


/* Given a page table's pointer, a desired VPN, and a PPN that is a node that takes part in the path of VPN in the Page Table's Trie,
 * return the address of the node that continues VPN in the path.
 * Arguments:
 *		vpn: the virtual page number that we're searching for in the Trie path
 * 		ppn: the physical page number of the current node that we are at in the path traversal
 * 		depth: the depth of the search - the amount of nodes we have already gone through in the search. That is crucial since it's what determines where the next entry that belongs to VPN inside the current PPN is. (using bit wise operations on VPN with Depth to extract the next entry's position)
 */
static uint64_t* get_entry(uint64_t vpn, uint64_t ppn, size_t depth);

/* Given a node's ppn, determine of its empty of any VPN's mapping. */
static bool is_node_empty(uint64_t node_ppn);

/* Given a node's ppn, free its Physical Page*/
static void free_node(uint64_t node_ppn);

/* If a node is empty of any VPN's mapping, free its Physical Page, and make sure to change the node's entry in the Page Table to NO_MAPPING.
 * uint64_t *node_ppn: a pointer to the node's entry in the Page Table. */
static void free_node_safe(uint64_t *node_ppn);


/**************** AUXILIARY FUNCTION DEFINITIONS ******************/


static void insert_mapping(uint64_t pt, uint64_t vpn, uint64_t ppn) {

	uint64_t current_node_ppn = pt;
	
	for (size_t depth = 0; depth < 5; depth++) { /* Personal calculations showed that the Page Table should be of height 5, with 512 entries for each node */
		uint64_t* entry = get_entry(vpn, current_node_ppn, depth);
	
		if (depth == 4) { // we reached the end of the path traversal - we'll now insert the mentioned VPN->PPN mapping
			*entry = ppn;
			
		} else { // we're still inside the path search
		
			// the entry of the VPN in the current node isn't mapped to a new node -> create a new node 
			if (*entry == NO_MAPPING) *entry = alloc_page_frame(); 
			
			// continue to the next node in the path				
			current_node_ppn = *entry;
		}
		
		/* Advancing in depth */
		depth++;
	}

}



static bool remove_mapping(uint64_t *current_node_ppn, uint64_t vpn, size_t depth) {
	uint64_t* entry = get_entry(vpn, *current_node_ppn, depth);
	
	if (depth == 4 || (*entry == NO_MAPPING) ) { // if we reached the end of the path (either one of the entries in the path is empty (*entry == NO_MAPPING), or if this is the last node in the path (depth == 4)), nullify (= NO_MAPPING) the entry 
		*entry = NO_MAPPING;
		if (depth != 4) return false;
		else return true;
	} else { // if we still need to advance in the search for the entry, simply advance to the next node in the path
		if (remove_mapping(entry, vpn, depth + 1) == false) {
			return false;
		}
	}

	if (depth != 0) { // if we aren't in the page table's root node (which shouldn't be freed at all costs), check if the node is empty of mapping, and if so, it frees it, and changes the entry that held the PPN of the node to NO_MAPPING
		free_node_safe(current_node_ppn);
	}
	
	return true;
}




static uint64_t* get_entry(uint64_t vpn, uint64_t node_ppn, size_t depth) {
	
	uint64_t* node_ppn_virt = (uint64_t*) phys_to_virt(node_ppn);
	uint64_t offset = vpn & ( 0x1ff000 << ((4 - depth) * 9) ); // at path traversal's depth: depth, the next 9 bits in VPN that refer to the next entry in the current PPN, are at a location of [12 ... 20] + ((4 - depth) * 9)
	uint64_t* entry_pa_virt = node_ppn_virt + offset;
	
	return entry_pa_virt;
}


static bool is_node_empty(uint64_t node_ppn) {
	
	/* DETERMINE IF ALL ENTRIES ARE EMPTY */
	uint64_t* node_ppn_virt = (uint64_t*) phys_to_virt(node_ppn);
	bool empty = true; // assume the node has all empty entries
	
	for (size_t i = 0; i < 512; i++) {
		if (node_ppn_virt[i] != NO_MAPPING) { // if there's an entry that isn't empty, the node isn't empty 
			empty = false;
		}
	}
	
	return empty;
}


static void free_node(uint64_t node_ppn) {
	free_page_frame(node_ppn);
}


static void free_node_safe(uint64_t *node_ppn) {
	if (is_node_empty(*node_ppn) == true) {
		free_node(*node_ppn);
		*node_ppn = NO_MAPPING;
	}
}

/**************** MAIN MECHANISM's DEFINITIONS ******************/


void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
	
	if (ppn != NO_MAPPING) { // if the desired action was to create/update the mapping
		insert_mapping(pt, vpn, ppn);
	} else { // if the desired action was to remove the mapping
		remove_mapping(&pt, vpn, 0);
	}

}




uint64_t page_table_query(uint64_t pt, uint64_t vpn) {
	uint64_t* pa_virt = get_entry(vpn, pt, 0);
	if (pa_virt) return 0;
	return 0;
}
