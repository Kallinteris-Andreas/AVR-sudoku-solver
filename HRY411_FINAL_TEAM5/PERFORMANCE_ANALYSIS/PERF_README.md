# HRY411_Performance_Analysis
Performance analysis program for the AVR sudoku solver


Using a batch file, the interface program is called 100 times for each difficulty level, redirecting its output to the appropriate log file.
After solving all the sudokus, using the log file parsing program (file_parser.c), the mean value and standard deviation of each file's measuments 
are calculated and printed on the command prompt.


The tests have been performed on Wednesday 22 December 2021, using the vesion 1.6 of the interface (available at https://github.com/the-rectifier/ace411_sudoku/releases/tag/1.6 ).
The output log files (easy_out.log, medium_out.log, hard_out.log and ultra_out.log) are placed in the folder OUTPUT folder.
Additionally, the cmd's output (showing the mean values and standard deviation) is written to the file "cmd_output_22122021.txt".
The boards that were solved are placed in the folder easy_sudokus, medium_sudokus, hard_sudokus and ultra_sudokus.


Approximately 100 minutes were needed to complete the tests (~21 minutes for the easy boards, ~18 minutes for the medium boards, ~18 minutes for the hard
boards and ~39 minutes for the ultra boards). For the easy, medium and hard boards, the delay is due to the comminication, while for the ultra boards, the solving
process takes a large portion of the total time.


The program supposes that the STK is connected to the COM3 port, else it should be changed in the batch file.


Τhe output log files are not submitted on eclass, because of their large size (~8MB).
