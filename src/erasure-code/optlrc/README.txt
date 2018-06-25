***   Implementation of Optimal-LRC plugin for Ceph   ***
Written by: Oleg Kolosov, Tel Aviv University

The code for the implementation of USENIX ATC 2018 paper 
### On Fault Tolerance, Locality, and Optimality in Locally Repairable Codes ###
https://www.usenix.org/conference/atc18/presentation/kolosov

Authors: 
Oleg Kolosov, School of Electrical Engineering, Tel Aviv University; 
Gala Yadgar, Computer Science Department, Technion and School of Electrical Engineering, Tel Aviv University; 
Matan Liram, Computer Science Department, Technion; 
Itzhak Tamo, School of Electrical Engineering, Tel Aviv University; 
Alexander Barg, Department of ECE/ISR, University of Maryland

-----------------------
Optimal-LRC
-----------------------
Optimal-LRC is a recently proposed [1] full-LRC . In this code, k data blocks and m global parities are divided into groups of size r, and a local parity is added to each group, allowing repair of any lost block by the r surviving blocks in its group. r does not necessarily divide m + k, but Optimal-LRC requires that n mod (r + 1) != 1.


-----------------------
Implementation content
-----------------------
ceph/src/erasure-code/optlrc/CMakeLists.txt
* Build file

ceph/src/erasure-code/optlrc/ErasureCodeOptLrc.cc
* The implementation of Optimal-LRC plugin

ceph/src/erasure-code/optlrc/ErasureCodeOptLrc.h
* .h file of the implementaton of Optimal-LRC plugin, contains description of functions

ceph/src/erasure-code/optlrc/ErasureCodeOptLrc_configs.h
* Holds generator matrices of Optimal-LRC

ceph/src/erasure-code/optlrc/ErasureCodePluginOptLrc.cc
* Used for integration in Ceph's erasure-code infrastructure. 

ceph/src/erasure-code/optlrc/ErasureCodePluginOptLrc.h

ceph/src/erasure-code/optlrc/Makefile.am
* Build file

ceph/src/erasure-code/CMakeLists.txt
* Build file

-----------------------
Implementation
-----------------------
ErasureCodeOptLrc.cc contains the main implementation of Optimal-LRC. It contains existing Ceph erasure-code functions used to intergrate with the Ceph erasure-code plugin and CRUSH, and also implementation of encode and decode functions for Optimal-LRC.
For encoding we use the external function jerasure_matrix_encode from the Jerasure library [2].

The encode proecess works the following:
1) Pass pointer to first chunk of data and coding chunks to a special optlrc encode function
2) optlrc_encode fetches the relevant generator matrix from ErasureCodeOptLrc_configs.h based on the code's n,k,r values
3) Convert the generator matrix to the format acceptable by jerasure_matrix_encode
4) Pass the generator matrix, pointers to data and coding chunks and blocksize to jerasure_matrix_encode for encoding

Decode process:
minimum_to_decode: Calculates the minimum number and IDs of chunks required to decode an erased chunk. It's based on the main assumption of our implementation, that we have at most one erasure in each local group.
If a chunk in a local group is erased, compiles a list of all surviving chunks.

decode_chunks:
1) Detects all erased chunks in the stripe (assumed to have at most one erasure in a local group)
2) Per each erasure, compiles a list of pointers to surviving blocks in its local group and passess them to optlrc_decode_local

optlrc_decode_local:
The structure of Optimal-LRC allows us to decode a chunk using a XOR operation on all other chunk it its local group.
The function has a list of all surviving chunks in the local group and it goes over them one by one, XORing each one with the XOR of all previous chunks. The result is placed in the location of the erased chunk finalizing the reconstruction process.

-----------------------
Reference
-----------------------
[1] I. Tamo and A. Barg. A family of optimal locally recoverable codes. IEEE Transactions on Information Theory, 60(8):4661–4676, Aug 2014
[2] http://jerasure.org/jerasure-2.0/
