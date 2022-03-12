#include "os.h"
#include <stdlib.h>


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





/******************************************************************/


static uint64_t* get_entry(uint64_t vpn, uint64_t ppn, size_t depth) {
	
	uint64_t* ppn_virt = (uint64_t*) phys_to_virt(ppn);
	uint64_t offset = vpn & ( 0x1ff000 << ((4 - depth) * 9) ); // at path traversal's depth: depth, the next 9 bits in VPN that refer to the next entry in the current PPN, are at a location of [12 ... 20] + ((4 - depth) * 9)
	uint64_t* pa_virt = ppn_virt + offset;
	
	return pa_virt;
}




void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {

}




uint64_t page_table_query(uint64_t pt, uint64_t vpn) {
	uint64_t* pa_virt = get_entry(vpn, pt, 0);
	if (pa_virt) return 0;
	return 0;
}
