/*
 * zigzag_configs.hpp
 *
 *  Created on: Aug 19, 2016
 *      Author: ido
 */

#ifndef OPTLRC_CONFIGS_HPP_
#define OPTLRC_CONFIGS_HPP_

#define MAX_DATA_CHUNKS     16
#define MAX_CHUNKS  20 
#define MAX_LOCALITY_CHUNKS   8
#define MAX_MATRIX          256
#define MAX_OPTLRC_CHUNKS   4

typedef struct OptLRC {
    int optlrc_encode[MAX_MATRIX][MAX_MATRIX];
    int optlrc_perm[MAX_MATRIX];

} OptLRC, *POptLRC;

struct OptLRC optlrc_9_4_2 = {
optlrc_encode : {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1},
        {2, 2, 0, 0},
        {0, 0, 1, 2},
        {0, 0, 0, 1},
        {2, 2, 0, 0},
        {0, 0, 1, 2}
},
optlrc_perm : {0,1,3,4,2,5,6,7,8}


};

typedef struct OptLRC_Configs
{
    POptLRC configs[MAX_CHUNKS][MAX_DATA_CHUNKS][MAX_LOCALITY_CHUNKS];
    OptLRC_Configs()
    {
        /*
         * Total Chunks
         * Data Chunks
         * Locality
         */
        configs[9][4][2]  =  &optlrc_9_4_2;
    }
} OptLRC_Configs, *POptLRC_Configs;

#endif /* OPTLRC_CONFIGS_HPP_ */


