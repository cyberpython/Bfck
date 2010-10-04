/*
*	The MIT License
*
*	Copyright (c) 2010 Georgios Migdos <cyberpython@gmail.com>
*
*	Permission is hereby granted, free of charge, to any person obtaining a copy
*	of this software and associated documentation files (the "Software"), to deal
*	in the Software without restriction, including without limitation the rights
*	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*	copies of the Software, and to permit persons to whom the Software is
*	furnished to do so, subject to the following conditions:
*
*	The above copyright notice and this permission notice shall be included in
*	all copies or substantial portions of the Software.
*
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*	THE SOFTWARE.
*/
#include <stdio.h>
#include <stdlib.h>

#define PROGRAM_NAME "Bfck"
#define VERSION_NUMBER "0.1"
#define PROGRAM_INFO_MSG "A simple Brainf*ck interpreter witten in C.\nDistributed under the terms of the MIT license.\nCopyright © 2010 Georgios Migdos.\n"

#define DATA_ARRAY_SIZE 30720 //30KB
#define EXECUTION_FINISHED_MSG "\n\n--Execution finished.--\n"
#define FILE_OPEN_FAILED_ERROR_MSG "Error opening file\n"
#define UNMATCHED_BRACKET_ERROR_MSG "Unmatched bracket.\n"



char data[DATA_ARRAY_SIZE];
int dataIndex = 0;
int maxDataIndex = DATA_ARRAY_SIZE-1;
FILE *pSourceCodeFile= NULL;



void error(const char *errorMsg){
	fputs(errorMsg, stderr);
}

void errorC(int c){
	printf("%d\n", c);
	fputc(c, stderr);
}

void fatalError(const char *errorMsg){
	fputs(errorMsg, stderr);
	exit(EXIT_FAILURE);
}

void msg(const char *message){
	fputs(message, stdout);
}

void msgC(int c){
	fputc(c, stdout);
}

void printProgramInfo(){
	fprintf(stdout, "%s version %s\n%s", PROGRAM_NAME, VERSION_NUMBER, PROGRAM_INFO_MSG);
}


char readC(){
	char c = fgetc(stdin);
	__fpurge(stdin); // Use of non-standard C function fpurge
	return c;
}



void initData(){
	int i=0;
	for(i=0; i<DATA_ARRAY_SIZE; i++){
		data[i] = 0;
	}
}



int isValidCmd(int cmd){
	if ( (cmd == '>') || (cmd=='<') || (cmd=='+') || (cmd=='-') || (cmd=='.') || (cmd==',') || (cmd=='[') || (cmd==']') ){
		return 0;
	}
	return 1;
}

int getNextCommand(){
	
	if(pSourceCodeFile == NULL){
		return EOF;
	}
	
	char buf[1];
	
	fread( buf, sizeof(char), 1, pSourceCodeFile);
	int cmd = buf[0];
	
	while ((isValidCmd(cmd) != 0) &&(!feof(pSourceCodeFile)) ){
		fread( buf, sizeof(char), 1, pSourceCodeFile);
		cmd = buf[0];
	}
	return cmd;
}



int incrementDataPointer(){
	if(dataIndex<maxDataIndex){
		dataIndex++;
		return 0;
	}
	return 1;
}

int decrementDataPointer(){
	if(dataIndex>0){
		dataIndex--;
		return 0;
	}
	return 1;
}

int incrementData(){
	data[dataIndex] ++;
	return 0;
}

int decrementData(){
	data[dataIndex] --;
	return 0;
}

int writeByte(){
	msgC(data[dataIndex]);
	return 0;
}

int readByte(){
	data[dataIndex] = readC();
	return 0;
}

long int findClosingBracket(){
	long int currentPos = ftell(pSourceCodeFile);
	long int openBrackets = 1;
	long int result = -1;
	char buf[1];
	int cmd = 0;
	while(!feof(pSourceCodeFile) && (openBrackets>0)){
		fread( buf, sizeof(char), 1, pSourceCodeFile);
		cmd = buf[0];
		if(cmd==']'){
			openBrackets--;
		}else if(cmd=='['){
			openBrackets++;
		}
	}
	if(openBrackets == 0){
		result = ftell(pSourceCodeFile);
	}
	fseek(pSourceCodeFile, currentPos, SEEK_SET);
	return result;	
}

long int findOpeningBracket(){
	long int currentPos = ftell(pSourceCodeFile);
	long int closedBrackets = 1;
	long int result = -1;
	char buf[1];
	int cmd = 0;
	while(!(ftell(pSourceCodeFile)<=0) && (closedBrackets>0)){
		fseek(pSourceCodeFile, -2, SEEK_CUR);
		fread( buf, sizeof(char), 1, pSourceCodeFile);
		cmd = buf[0];
		if(cmd=='['){
			closedBrackets--;
		}else if(cmd==']'){
			closedBrackets++;
		}
	}
	if(closedBrackets == 0){
		result = ftell(pSourceCodeFile);
	}
	fseek(pSourceCodeFile, currentPos, SEEK_SET);
	return result;	
}

int jumpZero(){
	long int matchingBracketPosition = findClosingBracket();
	if(matchingBracketPosition == -1){
		fatalError(UNMATCHED_BRACKET_ERROR_MSG);
		return 1;
	}else{
		if(data[dataIndex] == 0){
			fseek(pSourceCodeFile, matchingBracketPosition, SEEK_SET);
		}
	}
	return 0;
}

int jumpBackNonZero(){
	long int matchingBracketPosition = findOpeningBracket();
	if(matchingBracketPosition == -1){
		fatalError(UNMATCHED_BRACKET_ERROR_MSG);
		return 1;
	}else{
		if(data[dataIndex] != 0){
			fseek(pSourceCodeFile, matchingBracketPosition, SEEK_SET);
		}
	}
	return 0;
}



int execCommand(int cmd){
	if(isValidCmd(cmd) == 0){
		
		if(cmd == '>'){
			return incrementDataPointer();
		}else if (cmd == '<'){
			return decrementDataPointer();
		}else if (cmd == '+'){
			return incrementData();
		}else if (cmd == '-'){
			return decrementData();
		}else if (cmd == '.'){
			return writeByte();
		}else if (cmd == ','){
			return readByte();
		}else if (cmd == '['){
			return jumpZero();
		}else if (cmd == ']'){
			return jumpBackNonZero();
		}
		
	}
	return 0;
}

int main(int argc, char *argv[]){

	if(argc <= 1){
		printProgramInfo();
		return 0;
	}

	initData();
	dataIndex = 0;
	
	pSourceCodeFile = fopen(argv[1], "rb");
	int cmd = 0;
	if (pSourceCodeFile==NULL){
		fatalError(FILE_OPEN_FAILED_ERROR_MSG);
	}
	while(!feof(pSourceCodeFile)){
		cmd = getNextCommand();
		if(cmd!=EOF){
			execCommand(cmd);
		}
	}
	fclose(pSourceCodeFile);
	msg(EXECUTION_FINISHED_MSG);
	return 0;
}

