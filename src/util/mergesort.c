// merge sort. returns sorted indices
#include "../a.h"

// x    - data to sort
// r    - the return vector containing sorted *indices*
// t    - temp object for scatch space
// low  - lower index of range being sorted
// high - higher index of range being sorted
// asc  - bool, ascending? 
void mergeSortIndex(K x, K r, K t, uint64_t low, uint64_t high, bool asc){
    if (high <= low)
        return;

    uint64_t mid = ((high - low) / 2) + low;

    // recursively sort both subarrays
    mergeSortIndex(x, r, t, low, mid, asc);
    mergeSortIndex(x, r, t, mid+1, high, asc);

    // index for left and right parts to merge
    uint64_t left_idx = low;
    uint64_t right_idx = mid + 1;

    // merge
    if (asc){
        for (uint64_t i = low; i <= high; ++i){
            if (left_idx == mid+1){
                ti[i] = ri[right_idx++];
            }
            else if (right_idx == high+1){
                ti[i] = ri[left_idx++];
            }
            else if (xi[ri[left_idx]] <= xi[ri[right_idx]]){
                ti[i] = ri[left_idx++];
            }
            else {
                ti[i] = ri[right_idx++];
            }
        }
    }
    else {
        for (uint64_t i = low; i <= high; ++i){
            if (left_idx == mid+1){
                ti[i] = ri[right_idx++];
            }
            else if (right_idx == high+1){
                ti[i] = ri[left_idx++];
            }
            else if (xi[ri[left_idx]] > xi[ri[right_idx]]){
                ti[i] = ri[left_idx++];
            }
            else {
                ti[i] = ri[right_idx++];
            }
        }
    }

    for (uint64_t i = low; i <= high; ++i)
        ri[i] = ti[i];
}
