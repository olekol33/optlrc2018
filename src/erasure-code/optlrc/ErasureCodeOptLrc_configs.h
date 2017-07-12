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
#define MAX_GROUPS 10

typedef struct OptLRC {
    int optlrc_encode[MAX_MATRIX][MAX_MATRIX];
    int optlrc_perm[MAX_MATRIX];
    int optlrc_coef[MAX_GROUPS][MAX_LOCALITY_CHUNKS];

} OptLRC, *POptLRC;

/*struct OptLRC optlrc_9_4_2 = {
optlrc_encode : {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1},
        {214, 215, 0, 0},
        {0, 0, 214, 215},
        {214,222,113,120},
        {41,33,99,106},
        {9,1,51,58}
},
optlrc_perm : {0,1,3,4,2,5,6,7,8},
optlrc_coef: {
	{214,215,1},
	{214,215,1},
	{1,214,215}
}
};*/

struct OptLRC optlrc_9_4_2 = {
optlrc_encode : {
{1, 0, 0, 0},
{0, 1, 0, 0},
{0, 0, 1, 0},
{0, 0, 0, 1},
{1, 0, 0, 1},
{3, 2, 176, 102},
{101, 178, 2, 3},
{102, 176, 178, 101},
{0, 1, 1, 0}
},
optlrc_perm : {5,7,6,3,4,0,2,1,8},
optlrc_coef : {
{1,1,1},
{1,1,1},
{1,1,1}
}
};

struct OptLRC optlrc_12_6_3 = {
optlrc_encode : {
        {1, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0},
        {0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 1},
        {1, 1, 1, 0, 0, 0},
        {0, 0, 0, 1, 1, 1},
        {168,114,188,55,31,79},
		{206,20,188,80,120,79},
		{206,114,218,80,31,40},
        {168,20,218,55,120,40}

},
optlrc_perm : {0,1,2,4,5,6,3,7,8,9,10,11},
optlrc_coef: {
	{1,1,1,1},
	{1,1,1,1},
	{1,1,1,1}
}
};

struct OptLRC optlrc_8_3_2 = {
optlrc_encode : {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1},
        {214,215,0},
        {1,1,1},
        {215,215,1},
        {34,214,245},
        {34,214,245}
},
optlrc_perm : {0,1,3,2,4,5,6,7},
optlrc_coef: {
	{214,215,1},
	{1,214,215},
	{1,1}
}
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
        configs[8][3][2]  =  &optlrc_8_3_2;
        configs[12][6][3]  =  &optlrc_12_6_3;
    }
} OptLRC_Configs, *POptLRC_Configs;

#endif /* OPTLRC_CONFIGS_HPP_ */
