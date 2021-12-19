/*
 * File parser
 * Developed by elioudakis, to perform statistic analysis of the AVR-based sudoku solver
 *
 * HOW TO CALL
 * file_parser.exe <filename>
 * (example call) file_parser.exe easy_out.log
 *
 */
 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include <math.h>
 
 #define MAX_LINE_LENGTH 1024
 
 #define MAX_NUM_OF_TESTS 100
 
void parse_log_file(FILE* logfile){
	
	double results[MAX_NUM_OF_TESTS];
	int results_counter = 0;
	char * line_buffer  =(char*)malloc(MAX_LINE_LENGTH*sizeof(char));
	memset(line_buffer, 0, MAX_LINE_LENGTH);
	
	/* Time units, used to compare */
	char micro[] = "µs";
	char milli[] = "ms";
	/*char sec[]   = "s";*/
	
	/* Only the lines containing "Solved in: " matter for us */
	while(fgets(line_buffer, MAX_LINE_LENGTH, logfile) != NULL){
	
		if(strstr(line_buffer, "Solved in:")){
				/* The first characters are "21:22:43 [INFO] Solved in: "  */
				const char ch = ':';
				
				/* Get the part "xxms\n" */
				char * line_without_prefix =  strrchr(line_buffer, ch) + 2; 
				
				/* Get the time unit(microsec, millisec or sec) 
					The smallest unit we will find is micro (because one cycle is 0.1microsec and a sudoku needs more than 10 cycles to be solved) */
				char * time_unit;
				
				double measure = strtod(line_without_prefix, &time_unit);

				/* Convert all the measurements to milliseconds (to calculate the mean value later) */
				if(strstr(time_unit, micro)){
					results[results_counter] = measure/(1000.0);
				}
				else if(strstr(time_unit, milli)){
					results[results_counter] = measure;
				}
				else{ /* if(strcmp(time_unit, sec) == 0){ */
					results[results_counter] = measure*(1000.0);
				}
				results_counter++;
				/* Clear the line buffer */
				memset(line_buffer, 0, MAX_LINE_LENGTH);
		}

	}
	
	/* Clear memory */
	free(line_buffer);
	
	/* In the last loop, the results_counter has been incremented */
	results_counter--;
	
	/* Calculate the mean value */
	double mean = 0.0;
	
	for(int i=0; i<results_counter; i++){
		mean += results[i];
	}
	mean = mean/results_counter;
	printf("The mean execution time is %lf msec.\n", mean);
	
	/* Calculate the standard deviation sigma(X), where (sigma(X))^2 = 1/Ν * sum_{i=1}^{N} (xi - mean)^2 */
	double xi_minus_mean = 0.0;
	for(int i=0; i<results_counter; i++){
		xi_minus_mean += pow(results[i] - mean,2);
	}
	xi_minus_mean = xi_minus_mean/results_counter;
	double sd = sqrt(xi_minus_mean);
	printf("The standard deviation of the sample is %lf msec.\n", sd);

}

int main(int argc, char* argv[]){
	
	if(argc <1){
		printf("Give the log file name as argument!\n");
	}
		
	FILE* logfile = fopen(argv[1], "r");
	if(logfile != NULL){
		parse_log_file(logfile);
	}
	else{
		printf("Error opening log file!\n");
	}
	
	return 0;
}