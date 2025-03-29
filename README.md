Translate logical addresses to physical using a virtual memory space of 2^16 or 65,536. 
Logical addresses are listed in addresses.txt, stored in base 10. 
We must take these logical addresses, look them up in the TLB or Transition lookAside Buffer. This contains the page number and frames
If the process is in the TLB, grab the respective frame number and grab it from our virtual memory
If the process is not in the TLB, grab the frame number from the page table, grab it from virtual memory, storing it in the TLB using LRU algorithm
Put the byte into output.txt

The logical addresses have a page number and offset consisting of the last 8 bits respectfully
We will have 256 frames of 2^8 bytes. However, only 16 or 2^4 entries in TLB. 

Steps:

1. Read address from addresses.txt
2. Translate that address from logical to physical with TLB and page table
3. Find that address in .bin file
4. Output the contents at that address
