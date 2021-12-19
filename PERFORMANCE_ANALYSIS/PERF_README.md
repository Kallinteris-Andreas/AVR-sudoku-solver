# HRY411_Performance_Analysis
Performance analysis program for the AVR sudoku solver


Using a batch file, the interface program is called 100 times for each difficulty level, redirecting its output to the appropriate log file.
After solving all the sudokus, using the log file parsing program (file_parser.c), the mean value and standard deviation of each file's measuments 
are calculated and printed on the command prompt.


The tests have been performed on Sunday 19 December 2021, using the vesion 1.5 of the interface.
The output log files (easy_out.log, medium_out.log, hard_out.log and ultra_out.log) are placed in the folder OUTPUT folder.
Additionally, the cmd's output (showing the mean values and standard deviation) is written to the file "cmd_output_19122021.txt".


The program supposes that the STK is connected to the COM3 port, else it should be changed in the batch file.


All the files of the "HRY411_Performance_Analysis" repository are also submitted on eclass, except from the output log files, because of their large size (~8MB).
