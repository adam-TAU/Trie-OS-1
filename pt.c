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

/* Removes a page table mapping, if it exists. In case it does exist, it also cares to free the nodes that held the entries of the VPN's search path in the Page Table's,
 * if nodes were emptied due to the removal of the mapping - this doesn't apply to the root node of the Page Table tho. */
static bool remove_mapping(uint64_t *current_node_ppn, uint64_t vpn, size_t depth);


/* Given a page table's pointer, a desired VPN, and a PPN that is a node that takes part in the search path of VPN in the Page Table's Trie,
 * return the address of the node that continues VPN in the search path.
 * Arguments:
 *		vpn: the virtual page number that we're searching for in the Trie search path
 * 		ppn: the physical page number of the current node that we are at in the search path 
 * 		depth: the depth of the search - the amount of nodes we have already gone through in the search. That is crucial since it's what determines where the next entry that belongs to VPN inside the current PPN is. (using bit wise operations on VPN with Depth to extract the next entry's position)
 */
static uint64_t* get_entry(uint64_t vpn, uint64_t ppn, size_t depth);

/* Given a node's ppn, determine of its empty of any VPN's mapping. */
static bool is_node_empty(uint64_t node_ppn);

/* Given a node's ppn, free its Physical Page*/
static void free_node(uint64_t node_ppn);

/* If a node is empty of any VPN's mapping, free its Physical Page, and make sure to change the node's entry in the Page Table to NO_MAPPING.
 * uint64_t *node_ppn: a pointer to the node's entry in the Page Table.
 * This function returns true IF the node was in fact emptied and freed, ELSE, it returns false. */
static bool free_node_safe(uint64_t *node_ppn);







/**************** AUXILIARY FUNCTION DEFINITIONS ******************/

static void insert_mapping(uint64_t pt, uint64_t vpn, uint64_t ppn) {

	uint64_t current_node_ppn = pt;
	
	for (size_t depth = 0; depth < 5; depth++) { /* Personal calculations showed that the Page Table should be of height 5, with 512 entries for each node */
		uint64_t* entry = get_entry(vpn, current_node_ppn, depth);
	
		if (depth == 4) { // we reached the end of the search path - we'll now insert the mentioned VPN->PPN mapping
			*entry = ppn;
			
		} else { // we're still inside the search path
		
			// the entry of the VPN in the current node isn't mapped to a new node -> create a new node 
			if (*entry == NO_MAPPING) *entry = alloc_page_frame(); 
			
			// continue to the next node in the search path				
			current_node_ppn = *entry;
		}
	}

}


/* This recursive function will return true, if the current node that we're at in the recursion
 * has had any of its entried emptied (Nullified (= NO_MAPPING)) */
static bool remove_mapping(uint64_t *current_node_ppn, uint64_t vpn, size_t depth) {
	uint64_t* entry = get_entry(vpn, *current_node_ppn, depth);
	
	/* If we reached the end of the search path:
	 * either one of the nodes in the search path is empty (*entry == NO_MAPPING)
	 * or this is the last node in the search path (depth == 4)
	 * In both cases, nullify (= NO_MAPPING) the entry.
	 * IF the entry was empty to begin with:
	 		return false to signify to our parent node in the recursion,
	 		that he can terminate the recursion since none of its entries were emptied
	 * ELSE:
	 		return true to signify to our next parent that he had one of its entried emptied.
	 */
	if (depth == 4 || (*entry == NO_MAPPING) ) {
		
		if (depth != 4) { // in this case, there was nothing to remove to begin with
			return false;
		} else if (*entry == NO_MAPPING) { // in this case, the entry of the VPN->PPN was empty to begin with
			return true;
		} else { // in this case, there was in fact a mapping that VPN held, and we'll nullify it as desired
			*entry = NO_MAPPING;
			return true;
		}

	/* if we still need to advance in the search for the entry, simply advance to the next node in the search path */
	} else { 
		/* IF the recursion returned that the node that we're at hasn't had any entries emptied:
		 		we return false to signify to our parent node in the recursion,
	 			that he can terminate the recursion since none of its entries were emptied
		 * ELSE
		 		we continue on to the next step,
		 		of determining if the node we're at needs to be freed due to any of its entries being emptied,
		 		which is guaranteed due to <true> being returned.
		 */
		if (remove_mapping(entry, vpn, depth + 1) == false) {
			return false;
		}
	}

	/* if we aren't in the page table's root node (which shouldn't be freed at all costs):
	 * 		check if the node is empty of mapping:
	 *			 IF so:
	 				 it frees it, and changes the entry that held the PPN of the node to NO_MAPPING.
	 				 Then, we return true to signify to our next parent that he had one of its entried emptied.
	 * 			 ELSE:
	 				 the current node wasn't freed, and we return false to signify to our parent node in the recursion,
	 				 that he can terminate the recursion since none of its entries were emptied. 
	 */
	if (depth != 0) {
		return free_node_safe(current_node_ppn);
	}
	
	return false;
}




static uint64_t* get_entry(uint64_t vpn, uint64_t node_ppn, size_t depth) {
	
	uint64_t* node_ppn_virt = (uint64_t*) phys_to_virt(node_ppn);
	uint64_t offset = vpn & ( 0x1ff000 << ((4 - depth) * 9) ); // at the search path's depth: depth, the next 9 bits in VPN that refer to the next entry in the current PPN, are at a location of [12 ... 20] + ((4 - depth) * 9)
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


static bool free_node_safe(uint64_t *node_ppn) {
	if (is_node_empty(*node_ppn) == true) {
		free_node(*node_ppn);
		*node_ppn = NO_MAPPING;
		return true;
	}
	
	return false;
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
	uint64_t current_node_ppn = pt;
	
	for (size_t depth = 0; depth < 5; depth++) { /* Personal calculations showed that the Page Table should be of height 5, with 512 entries for each node */
		uint64_t* entry = get_entry(vpn, current_node_ppn, depth);
	
		if (depth == 4 || (*entry == NO_MAPPING) ) { // we reached the end of the search path - we'll return the corresponding entry
			return *entry;
			
		} else { // we're still inside the search path, continue to the next node in the search path	
			current_node_ppn = *entry;
		}
	}
	
	return NO_MAPPING;
}
