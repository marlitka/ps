#include "prefix_sum.h"
#include "helpers.h"
#include <pthread.h>

void* compute_prefix_sum(void *a)
{
    prefix_sum_args_t *args = (prefix_sum_args_t *)a;

    /************************
     * Your code here...    *
     * or wherever you like *
     ************************/
    int n = args->n_vals/args->n_threads;

    // z:
    for(int j=1;j<n;j+=2)
    {
        int i=args->t_id*n+j;
        args->output_vals[i] = args->input_vals[i-1]+args->input_vals[i];
    }
    pthread_barrier_wait(args->barrier);
    // w:
    for(int j=3;j<n;j+=4)
    {
        int i=args->t_id*n+j;
        args->output_vals[i] = args->output_vals[i-2]+args->output_vals[i];        
    }
    pthread_barrier_wait(args->barrier);
    // y
    for(int j=0;j<n/4;j++)
    {
        int shift = j*4;
        int prefix = 0;
        if(args->t_id >0)
        {
            prefix=args->output_vals[args->t_id*4-1];
        }
        args->output_vals[shift]=args->input_vals[shift]+prefix;
        args->output_vals[shift+1]+=prefix;
        args->output_vals[shift+2]=args->output_vals[shift]+args->output_vals[shift+1];
        args->output_vals[shift+3]+=prefix;
    }
    pthread_barrier_wait(args->barrier);
    return 0;
}
