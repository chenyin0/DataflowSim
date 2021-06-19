// #include "viterbi.h"

// int viterbi( tok_t obs[N_OBS], prob_t init[N_STATES], prob_t transition[N_STATES*N_STATES], prob_t emission[N_STATES*N_TOKENS], state_t path[N_OBS] )
// {
//   prob_t llike[N_OBS][N_STATES];
//   step_t t;
//   state_t prev, curr;
//   prob_t min_p, p;
//   state_t min_s, s;
//   // All probabilities are in -log space. (i.e.: P(x) => -log(P(x)) )
 
//   // Initialize with first observation and initial probabilities
//   L_init: for( s=0; s<N_STATES; s++ ) 
//   {
//     llike[0][s] = init[s] + emission[s*N_TOKENS+obs[0]];
//   }

  // // Iteratively compute the probabilities over time
  // L_timestep: for( t=1; t<N_OBS; t++ ) 
  // {
  //   L_curr_state: for( curr=0; curr<N_STATES; curr++ ) 
	// {
  //     // 7 instr.
  //     // Compute likelihood HMM is in current state and where it came from.
  //     prev = 0;
  //     min_p = llike[t-1][prev] +
  //             transition[prev*N_STATES+curr] +
  //             emission[curr*N_TOKENS+obs[t]];
  //     // 8 instr.
  //     L_prev_state: for( prev=1; prev<N_STATES; prev++ ) 
	//   {
  //       p = llike[t-1][prev] +
  //           transition[prev*N_STATES+curr] +
  //           emission[curr*N_TOKENS+obs[t]];
  //       if( p<min_p ) 
	// 	{
  //         min_p = p;
  //       }
  //     }
  //     llike[t][curr] = min_p;
  //   }
  // }

  // Iteratively compute the probabilities over time
  for( t=1; t<N_OBS; t++ ) 
  {
    for( curr=0; curr<N_STATES; curr++ ) 
  	{
      // 7 instr.
      // Compute likelihood HMM is in current state and where it came from.
      prev = 0;
      llike = llike[t-1][prev];
      prev_N = prev * N_STATES;
      prev_N_curr = prev_N + curr;
      curr_N = curr*N_TOKENS;
      curr_N_obs = curr_N + obs;
      transition = transition[prev_N_curr];
      emission = emission[curr_N_obs];
      llike_transition = llike + transition;
      min_p = llike_transition + emission;

      // 8 instr.
      for( prev_=1; prev_<N_STATES; prev_++ ) 
	    {
        llike_ = llike[t-1][prev];
        prev_N_ = prev_ * N_STATES;
        prev_N_curr_ = prev_N_ + curr;
        curr_N_ = curr*N_TOKENS;
        curr_N_obs_ = curr_N_ + obs_;
        transition_ = transition[prev_N_curr];
        emission_ = emission[curr_N_obs_];
        llike_transition_ = llike_ + transition_;
        p = llike_transition_ + emission_;

        // if( p<min_p ) 
		    // {
        //   min_p = p;
        // }
      }

      llike[t][curr] = min_p;
    }
  }

//   // Identify end state
//   min_s = 0;
//   min_p = llike[N_OBS-1][min_s];
//   L_end: for( s=1; s<N_STATES; s++ ) {
//     p = llike[N_OBS-1][s];
//     if( p<min_p ) {
//       min_p = p;
//       min_s = s;
//     }
//   }
//   path[N_OBS-1] = min_s;

//   // Backtrack to recover full path
//   L_backtrack: for( t=N_OBS-2; t>=0; t-- ) 
//   {
//     min_s = 0;
//     min_p = llike[t][min_s] + transition[min_s*N_STATES+path[t+1]];
//     L_state: for( s=1; s<N_STATES; s++ ) 
// 	{
//       p = llike[t][s] + transition[s*N_STATES+path[t+1]];
//       if( p<min_p ) 
// 	  {
//         min_p = p;
//         min_s = s;
//       }
//     }
//     path[t] = min_s;
//   }

//   return 0;
// }

