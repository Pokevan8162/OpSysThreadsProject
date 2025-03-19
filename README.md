Translate logical addresses to physical using a virtual memory space of 2^16 or 65,536. 
Logical addresses are listed in addresses.txt, stored in base 10. 
We must take these logical addresses, look them up in the TLB or Transition lookAside Buffer. This contains the page number and frames
If the process is in the TLB, grab the respective frame number and grab it from our virtual memory
If the process is not in the TLB, grab the frame number from the page table, grab it from virtual memory, storing it in the TLB using LRU algorithm
Put the byte into output.txt
