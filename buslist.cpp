//============================================================================
// Name        : buslist.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include "buslist.h"

#define FILENAME "buslist.csv"
#define BUF 4096

struct busObj bus[1440];

char* replace_char(char* str, char find, char replace);

int16_t getBEint16(int16_t *bigEndianNumber);

uint16_t getBEuint16(uint16_t *bigEndianNumber);

uint32_t getBEuint32(uint32_t *bigEndianNumber);

float getBEreal(float *bigEndianNumber);

bool getBit(uint8_t bitArray, int n);

char *printBitSequence(void *var, size_t bytes, char *strng);

template <typename T>
T swap_endian(T u) {
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

    union {
        T u;
        uint8_t u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];

    return dest.u;
}

template <typename T>
void print_hex(T input) {
	size_t i;
	union {
		T input;
		uint8_t array[sizeof(T)];
	} field;
	field.input = input;
	for (i=0; i<sizeof(T); i++){
		printf("%x",field.array[i]);
	}
}


int main(void) {
	FILE *fp;
	char strbuf1[BUF], strbuf2[BUF];
	char busfile[] = {FILENAME};
	char *tok, *cptr;
	int k, i, j;
	uint16_t offsetAddr = 0;
	if ((fp=fopen(busfile,"r")) == NULL) {
		fprintf(stderr, "Problem opening file '%s' : %s\n", busfile, strerror(errno));
		return EXIT_FAILURE;
	}
	i=0;
	while (fgets(strbuf1, BUF, fp)){	//row-wise extraction of strings
	  tok = strtok(strbuf1,";");
	  k = 1;
	  if (isdigit((int)tok[0])){
		  j = strtol(tok, NULL, 0);
		  bus[i].num = j;
		  while (tok != NULL) {
			tok = strtok (NULL, ";");
			k++;
			switch (k){ // extract the columns
			case 3:		// Tag Number
				snprintf(bus[i].tag, BUS_NAME_MAX-1, tok);
				break;
			case 4:		// Signal name
				snprintf(bus[i].name, BUS_TAG_MAX-1, tok);
				break;
			case 5:		// Lower limit measurement range
				sprintf(strbuf2, tok);
				replace_char(strbuf2, ',', '.');
				bus[i].llMeas = (double) strtof(strbuf2, NULL);
				break;
			case 6:		// Upper limit measurement range
				sprintf(strbuf2, tok);
				replace_char(strbuf2, ',', '.');
				bus[i].ulMeas = (double) strtof(strbuf2, NULL);
				break;
			case 7:		// Unit
				snprintf(bus[i].unit, BUS_UNIT_MAX-1, tok);
				break;
			case 10:	// Data type
				if (strcmp(tok, "BOOL")==0)  			// boolean signal
					bus[i].datatype = 1;
				else if (strcmp(tok, "INT")==0)			// int signal
					bus[i].datatype = 2;
				else if (strcmp(tok, "UINT")==0)		// uint signal
					bus[i].datatype = 3;
				else if (strcmp(tok, "UDINT")==0)		// udint signal
					bus[i].datatype = 4;
				else if (strcmp(tok, "Scaled REAL")==0)	// Scaled real signal
					bus[i].datatype = 5;
				else if (strcmp(tok, "REAL")==0)		// Real signal
					bus[i].datatype = 6;
				else bus[i].datatype = -1;				// error
				break;
			case 11: 	// start address
				if (tok[0] == 'A'){ // AW means input / read for MPC:
					bus[i].rw = false;
				}
				else bus[i].rw = true; // output / write
				bus[i].startByte = (uint16_t) strtol(tok+2, NULL, 0);
				if (i==0) offsetAddr = bus[i].startByte;
				bus[i].startByte -= offsetAddr;	// subtract first address value
				if (bus[i].datatype == 1) { // BOOL needs position of bit, too:
					cptr = strstr(tok, ".");
					bus[i].bit = (uint8_t) strtol(cptr+1, NULL, 0);
				}
				break;
			}
		  }
		  i++;
	  }
	}
	i = 0;
	do {
		printf("A[%3d] = %3d, %s, %s, %lf, %lf, %s, %3d, %d->%d.%d\n", i, bus[i].num, bus[i].tag, bus[i].name, bus[i].llMeas, bus[i].ulMeas, bus[i].unit, bus[i].datatype, bus[i].rw, bus[i].startByte, bus[i].bit);
		i++;
	}
	while (bus[i].num > bus[i-1].num && i < 1440);
	printf("float has %lu bits\n",sizeof(float)*CHAR_BIT);
	int16_t num = 12345;
	uint16_t num2 = 65000;
	uint32_t num3 = 0xFFFFFFF1;
	float num4 = 12.888;
	printf("%d,\t\t %d\n", num, getBEint16(&num));
	printf("%x,\t\t %x\n", num, getBEint16(&num));
	printf("%d,\t\t %d\n", num2, getBEuint16(&num2));
	printf("%x,\t\t %x\n", num2, getBEuint16(&num2));
	printf("%u,\t\t %u\n", num3, getBEuint32(&num3));
	printf("%x,\t\t %x\n", num3, getBEuint32(&num3));
	printf("%f,\t\t %f\n", num4, getBEreal(&num4));
	print_hex(num4);
	printf(",\t\t ");
	print_hex(getBEreal(&num4));
	printf("\n");
	printf("%f,\t\t %f\n", num4, swap_endian<float>(num4));
	print_hex(num4);
	printf(",\t\t ");
	print_hex(swap_endian<float>(num4));
	printf("\n");
	uint8_t bitArray = 0x1;
	for (i=7; i>=0; i--){
		printf("%d ",getBit(bitArray, i));
	}
	char buffer3[65];
	//printBitSequence((void*) &num2, 2, buffer3);
	printf("\nVariable in decimal: %u, variable binary. %s\n", num3, printBitSequence((void*) &num3, 4, buffer3));
	printf("\n");
	printf("Closing...\n");
	fclose(fp);
	return EXIT_SUCCESS;
}

char* replace_char(char* str, char find, char replace){
    char *current_pos = strchr(str,find);
    while (current_pos) {
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}

int16_t getBEint16(int16_t *bigEndianNumber){
	uint8_t buf[2];
	memcpy(buf, bigEndianNumber, 2*sizeof(uint8_t));
	return (int16_t) (buf[1] << 0 | buf[0] << 8);
}

uint16_t getBEuint16(uint16_t *bigEndianNumber){
	uint8_t buf[2];
	memcpy(buf, bigEndianNumber, 2*sizeof(uint8_t));
	return (uint16_t) (buf[1] << 0 | buf[0] << 8);
}

uint32_t getBEuint32(uint32_t *bigEndianNumber){
	uint8_t buf[4];
	memcpy(buf, bigEndianNumber, 4*sizeof(uint8_t));
	return (uint32_t) (buf[3] << 0 | buf[2] << 8 | buf[1] << 16 | buf[0] << 24);
}

float getBEreal(float *bigEndianNumber){
	union swapType {
		float floatNumber;
		uint8_t byteAarray[4];
	} source, dest;
	int i;
	source.floatNumber = *bigEndianNumber;
	for (i=0; i<4; i++){
		dest.byteAarray[i] = source.byteAarray[3-i];
	}
	return dest.floatNumber;
}

bool getBit(uint8_t bitArray, int n){
	if (n<0 || n>7) {
		n = 0;
		printf("WARNING: n is not in correct range (function getBit).\n");
	}
	return (bool) (bitArray & (1 << n));
}

char *printBitSequence(void *var, size_t bytes, char *strng){
	uint8_t buf[8];
	int i,j;
	if (bytes > 8 || bytes <1) bytes = 1;
	memcpy(buf, var, bytes);
	for (i=bytes-1; i>=0; i--){
		for (j=7; j>=0; j--){
			if (getBit(buf[i],j) == true){
				strng[bytes*8-1-(j+(i*8))] = '1';
			}
			else {
				strng[bytes*8-1-(j+(i*8))] = '0';
			}
		}
	}
	printf("\n");
	strng[bytes*8] = '\0';
	return strng;
}


