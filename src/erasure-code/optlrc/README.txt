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
Optimal-LRC is a recently proposed [1] full-LRC . In this code, k data blocks and m global parities are divided into groups of size r, 
and a local parity is added to each group, allowing repair of any lost block by the r surviving blocks in its group. 
r does not necessarily divide m + k, but Optimal-LRC requires that n mod (r + 1) != 1.








-----------------------
Reference
-----------------------
[1] I. Tamo and A. Barg. A family of optimal locally recoverable codes. IEEE Transactions on Information Theory, 60(8):4661–4676, Aug 2014
