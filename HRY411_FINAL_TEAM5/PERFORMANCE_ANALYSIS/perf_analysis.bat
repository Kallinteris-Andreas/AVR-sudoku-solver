@ECHO OFF

DEL /F easy_out.log
DEL /F medium_out.log
DEL /F hard_out.log
DEL /F ultra_out.log

DEL /F file_parser.exe
gcc file_parser.c -o file_parser

REM Script to generate and solve 100 sudokus of each difficulty level
REM To be used on Windows systems
REM by elioudakis, Dec. 2021

REM EASY SUDOKU TESTS
ace411_sudoku_win_v1_5.exe list > easy_out.log 
FOR /L %%A IN (1,1,100) DO (
	ECHO NEW GAME %%A >> easy_out.log
	ace411_sudoku_win_v1_5.exe run -d Easy -r 9600 -u COM3 <batch_input.txt  >> easy_out.log
)

REM MEDIUM SUDOKU TESTS 
ace411_sudoku_win_v1_5.exe list > medium_out.log 
FOR /L %%A IN (1,1,100) DO (
	ECHO NEW GAME %%A >> medium_out.log
	ace411_sudoku_win_v1_5.exe run -d Medium -r 9600 -u COM3 <batch_input.txt  >> medium_out.log
)

REM HARD SUDOKU TESTS
ace411_sudoku_win_v1_5.exe list > hard_out.log 
FOR /L %%A IN (1,1,100) DO (
	ECHO NEW GAME %%A >> hard_out.log
	ace411_sudoku_win_v1_5.exe run -d Hard -r 9600 -u COM3 <batch_input.txt  >> hard_out.log
)

REM ULTRA SUDOKU TESTS
ace411_sudoku_win_v1_5.exe list > ultra_out.log 
FOR /L %%A IN (1,1,100) DO (
	ECHO NEW GAME %%A >> ultra_out.log
	ace411_sudoku_win_v1_5.exe run -d Ultra -r 9600 -u COM3 <batch_input.txt  >> ultra_out.log
)
 
 
ECHO ON

ECHO EASY LEVEL STATISTICS
file_parser.exe easy_out.log

ECHO MEDIUM LEVEL STATISTICS
file_parser.exe medium_out.log

ECHO HARD LEVEL STATISTICS
file_parser.exe hard_out.log

ECHO ULTRA LEVEL STATISTICS
file_parser.exe ultra_out.log


PAUSE