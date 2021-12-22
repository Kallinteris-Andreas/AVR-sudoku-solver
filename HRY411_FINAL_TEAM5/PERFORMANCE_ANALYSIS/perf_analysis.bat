@ECHO OFF

DEL /F easy_out.log
DEL /F medium_out.log
DEL /F hard_out.log
DEL /F ultra_out.log

DEL /F file_parser.exe
gcc file_parser.c -o file_parser

REM Script to solve 100 sudokus of each difficulty level
REM The sudokus of each level are placed in the corresponding folder
REM To be used on Windows systems
REM by elioudakis, Dec. 2021

REM EASY SUDOKU TESTS
ace411_sudoku_win.exe list > easy_out.log 
FOR %%f IN (easy_sudokus\*) DO (
	ECHO NEW GAME %%f >> easy_out.log
	ace411_sudoku_win.exe prog -b %%f -r 9600 -u COM3 -i <batch_input_gen.txt  >> easy_out.log
)

@ECHO ON
ECHO EASY LEVEL STATISTICS
file_parser.exe easy_out.log
@ECHO OFF

REM MEDIUM SUDOKU TESTS 
ace411_sudoku_win.exe list > medium_out.log 
FOR %%f IN (medium_sudokus\*) DO (
	ECHO NEW GAME %%f >> medium_out.log
	ace411_sudoku_win.exe prog -b %%f -r 9600 -u COM3 -i <batch_input_gen.txt  >> medium_out.log
)

@ECHO ON
ECHO MEDIUM LEVEL STATISTICS
file_parser.exe medium_out.log
@ECHO OFF

REM HARD SUDOKU TESTS
ace411_sudoku_win.exe list > hard_out.log 
FOR %%f IN (hard_sudokus\*) DO (
	ECHO NEW GAME %%f >> hard_out.log
	ace411_sudoku_win.exe prog -b %%f -r 9600 -u COM3 -i <batch_input_gen.txt  >> hard_out.log
)

@ECHO ON
ECHO HARD LEVEL STATISTICS
file_parser.exe hard_out.log
@ECHO OFF

REM ULTRA SUDOKU TESTS
ace411_sudoku_win.exe list > ultra_out.log 
FOR %%f IN (ultra_sudokus\*) DO (
	ECHO NEW GAME %%f >> ultra_out.log
	ace411_sudoku_win.exe prog -b %%f -r 9600 -u COM3 -i <batch_input_gen.txt  >> ultra_out.log
)

@ECHO ON
ECHO ULTRA LEVEL STATISTICS
file_parser.exe ultra_out.log

PAUSE