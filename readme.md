**HRY411 - Semester Project**

26 November:\
Simulation with "simulation_26_11.stim" (feeds AVR with the sudoku given in the assignment)\
Commands AT, C, N, and P work fine\
The sudoku was solved successfully (started on 297331.7 usec and ended on 322455.7 usec).\
More tests tomorrow (sending the results back is not tested, they are simply written correctly to SRAM).\


28 November:\
Many improvents in code, after the zoom vc (created header file sudoku.h)
Simulation with "simulation_27_11.stim" (feeds AVR with the sudoku given in the assignment)\
Commands AT, C, N, P, S, T and OK(from PC) work fine\
The sudoku is received correctly, solved, and sent back correctly.\
TODO: 1) Migrate to c++, 2)merge with the fast sudoku solver, 3)do simulations for B and D (debug commands), 4)feed with more than 1 sudokus, one after another

30 November:\
Simulations with "break_test.stim", "break_test_debug.stim" and "two_sudokus.stim". \
Commands B and D work fine. B has been tested only for the solving-break. The transmitting-break is tricky in the simulation, so it will be tested on the hardware soon. \
Also, the program can handle one sudoku after another. \
TODO: 1) Migrate to C++, 2)use the fast algorithm, 3) download to STK



