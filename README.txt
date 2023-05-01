Project 2- Clocks,Mulitcast and Commit

Assignment 2:- 

Implementation of Casual and non casual ordering

Casual ordering and non casual ordering are implemented in two different programs namely 

For casual ordering :- “project2_2_casual.cpp” 

For non casual ordering :- “project2_2_noncasual.cpp” 

Both programs are implemented to show how the ordering is carried out throughout the processes. 


Instructions to execute-

1.Given a make file which have the required commands written to execute the program or

2. we can also compile using command line args

	For casual ordering :-
     		g++ -std=c++11 -o casual project2_2_casual.cpp -pthread
	For Non causal ordering :-
     		g++ -std=c++11 -o non_casual project2_2_noncausal.cpp -pthread

After compilation we can run using the basic commands 


ex - ./causal 0 msg

./causal 1 msg

./causal 2 msg


./noncasual 0 msg

./noncasual 1 msg

./noncasual 2 msg



