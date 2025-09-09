#include "prefix_sum.h"
#include "helpers.h"
#include <pthread.h>

void* compute_prefix_sum(void *a)
{
    prefix_sum_args_t *args = (prefix_sum_args_t *)a;


    const int n = args->n_vals;
    const int n2 = next_power_of_two(n);
    const int nThreads = args->n_threads;
    const int tId = args->t_id;


    int min = (nThreads > args->n_vals) ? args->n_vals : nThreads;

    int binSize = args->n_vals / min;


    const int idx0 = tId*binSize;
    if((tId+1)==nThreads)
        binSize = n - binSize*(nThreads-1);    
    const int idx1 = ((idx0+binSize) > (args->n_vals-1) ? (args->n_vals-1) : idx0+binSize-1);

    int nLevels = 0;
    while ((1 << nLevels) < n2) ++nLevels;

    //printf("\nLevels=%d; n=%d; binSize=%d; Thread=%d, idx0=%d; idx1=%d; totalThreads=%d\n", nLevels, n, binSize, tId, idx0, idx1, nThreads);

    for(int i=idx0;i<=idx1;i++)
    {
        args->output_vals[i]=args->input_vals[i];
    }        
    pthread_barrier_wait(args->barrier);

    for(int lvl=1;lvl<nLevels;lvl++)
    {
        int step = 1 << lvl;
        for(int i=idx0;i<=idx1;i++)
        {
            if(i<0 || i>=n)
                continue;
            if((i+1)%step==0)
            {
                int i0 = i-step/2;
                int val = (i0>=0 ? args->output_vals[i0]: 0);
                args->output_vals[i] = args->op(args->output_vals[i], val, args->n_loops);
            }
        }
        pthread_barrier_wait(args->barrier);
    }

    //printf("\n\n%d, %d, %d, %d, %d;", args->output_vals[0], args->output_vals[1], args->output_vals[2], args->output_vals[3], args->output_vals[4]);
    int lvl = nLevels - 1;
    int step = 1 << lvl;        
    int acc = 0;
    //int lastFoundIdx = 0;
    //int i = idx1;
    do
    {
        step = 1 << lvl;
        int offset = ((idx1+1)%step );
        int j = idx1 - (offset > 0 ? offset : step);
        if(j>=0 || j<n)
        {
            acc = args->op(acc, args->output_vals[j], args->n_loops);
            //lastFoundIdx = j+1;                   
        }        
        /*
        for(int j = ((i-step)>lastFoundIdx? (i-step):lastFoundIdx); j<i; j++)
        {
            if(j<0 || j>=n)
                continue;
            if((j+1)%step==0)
            {
                acc = args->op(acc, args->output_vals[j], args->n_loops);
                lastFoundIdx = j+1;
                break;                    
            }
        }
        */
        lvl--;
        step = 1 << lvl;
    } while((idx1+1)%step!=0 && lvl>0);
    //if(i>=0 && i<n)
        args->output_vals[idx1] = args->op(acc, args->output_vals[idx1], args->n_loops);
    for(int i1 = idx1-1;i1>=idx0;i1--)
    {
        if(i1<0 || i1>=n)
            continue;
        args->output_vals[i1] = args->op(args->output_vals[i1+1], (-1*args->input_vals[i1+1]), args->n_loops);
    }

    pthread_barrier_wait(args->barrier);


    return 0;
}
