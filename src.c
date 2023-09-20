#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#define LEN(a) (sizeof((a))/sizeof(*(a)))

typedef unsigned int uint;

void radix_sort(uint *arr, uint len)
{
    // should replace with another fast allocator to support
    // big arrays
    uint *swap_array = alloca(sizeof(uint)*len);
    uint swap = 0;
    for(uint s = 0; s<32; s+=8) {
        uint *in = arr;
        uint *sorted = swap_array;
        if (swap) {
            in = swap_array;
            sorted = arr;
        }
        swap^=1;

        uint prefix_sum[256] = {0};
        for(uint i = 0; i<len; i++) {
            prefix_sum[(in[i]>>s) & 0xff]++;
        }
        uint sum = 0;
        for(uint i = 0; i<256; i++) {
            uint count = prefix_sum[i];
            prefix_sum[i] = sum;
            sum+=count;
        }
        for(uint i = 0; i<len; i++) {
            sorted[prefix_sum[(in[i]>>s) & 0xff]++] = in[i];
        }
    }
}

void quicksort(uint *arr, int elements) // from https://alienryderflex.com/quicksort/ 
{
#define  MAX_LEVELS  300
    int piv, beg[MAX_LEVELS], end[MAX_LEVELS], i=0, L, R, swap;

    beg[0]=0; end[0]=elements;
    while (i>=0)
    {
        L=beg[i]; R=end[i]-1;
        if (L<R)
        {
            piv=arr[L];
            while (L<R)
            {
                while (arr[R]>=piv && L<R) R--; if (L<R) arr[L++]=arr[R];
                while (arr[L]<=piv && L<R) L++; if (L<R) arr[R--]=arr[L];
            }
            arr[L]=piv; beg[i+1]=L+1; end[i+1]=end[i]; end[i++]=L;
            if (end[i]-beg[i]>end[i-1]-beg[i-1])
            {
                swap=beg[i]; beg[i]=beg[i-1]; beg[i-1]=swap;
                swap=end[i]; end[i]=end[i-1]; end[i-1]=swap;
            }
        }
        else
        {
            i--;
        }
    }
}

void 
quicksort_r(int *arr, int low, int high) // from https://gist.github.com/rcgary/3347663
{
  int pivot, i, j, temp;
  if(low < high) {
    pivot = low; // select a pivot element
    i = low;
    j = high;
    while(i < j) {
      // increment i till you get a number greater than the pivot element
      while(arr[i] <= arr[pivot] && i <= high)
        i++;
      // decrement j till you get a number less than the pivot element
      while(arr[j] > arr[pivot] && j >= low)
        j--;
      // if i < j swap the elements in locations i and j
      if(i < j) {
        temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
      }
    }

    // when i >= j it means the j-th position is the correct position
    // of the pivot element, hence swap the pivot element with the
    // element in the j-th position
    temp = arr[j];
    arr[j] = arr[pivot];
    arr[pivot] = temp;
    // Repeat quicksort for the two sub-arrays, one to the left of j
    // and one to the right of j
    quicksort_r(arr, low, j-1);
    quicksort_r(arr, j+1, high);
  }
}

int compare_uint(const void *aa, const void *bb)
{
    uint a = *((uint *)aa);
    uint b = *((uint *)bb);
    return (a > b ) - (b > a);
}

#define MSEC(a) ((a)*1000 / CLOCKS_PER_SEC)

int main()
{
    // should probably have at least 5mibs of stack configured for this
    // test to work since the radix sort uses alloca
    uint len = 1024*1024;
    uint *arr = malloc(sizeof(uint)*len);
    uint *arr_quicksort_std = malloc(sizeof(uint)*len);
    uint *arr_quicksort = malloc(sizeof(uint)*len);
    uint *arr_quicksort_r = malloc(sizeof(uint)*len);

    // generate some so-so random values
    srand(0);
    for(int i = 0; i<len; i++) {
        uint and = (1 << (i & 31))-1;
        arr[i] = ((uint) rand()) & and;
    }
    memcpy(arr_quicksort, arr, sizeof(uint)*len);
    memcpy(arr_quicksort_r, arr, sizeof(uint)*len);
    memcpy(arr_quicksort_std, arr, sizeof(uint)*len);

    clock_t qsort_std_time;
    clock_t qsort_time;
    clock_t qsort_recursive_time;
    clock_t radix_time;
    clock_t start;

    start = clock();
    qsort(arr_quicksort_std, len, sizeof(uint), compare_uint);
    qsort_std_time = clock() - start;

    start = clock();
    quicksort(arr_quicksort,len);
    qsort_time = clock() - start;

    start = clock();
    quicksort_r(arr_quicksort_r,0, len-1);
    qsort_recursive_time = clock() - start;

    start = clock();
    radix_sort(arr, len);
    radix_time = clock() - start;

    printf("stdlib qsort time: %ld\nquicksort time: %ld\nquicksort recursive time: %ld\nradix sort time: %ld\n", MSEC(qsort_std_time),MSEC(qsort_time), MSEC(qsort_recursive_time), MSEC(radix_time));

    for(int i = 0; i<len; i++) {
        assert(arr[i] == arr_quicksort[i] && arr_quicksort[i]  == arr_quicksort_std[i] && arr_quicksort_std[i] == arr_quicksort_r[i]);
    }

    free(arr);
    free(arr_quicksort);
    free(arr_quicksort_r);
    free(arr_quicksort_std);

    return 0;
}

