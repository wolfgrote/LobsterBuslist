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
#define PN_RECVSIZE 1440
#define PN_SENDSIZE 1440

#define MAX_I 20000				// maximum integer number of measurement value (20mA)
#define MIN_I 4000				// maximum integer number of measurement value (4mA)
#define TOLERANCE 0.0			// relative tolerance for out-of-range detection of measurement

struct busObj bus[1440];
uint8_t PNsend[PN_SENDSIZE]={0}, PNrecv[PN_RECVSIZE]={0};


int main(void) {
	int i;
	readBuslist(FILENAME, bus, 1440);
	i = 0;
	do {
		printf("A[%3d] = %3d, %s, %s, %4s, %lf, %lf, %s, %3d, %d->%d.%d\n", i, bus[i].num, bus[i].tag, bus[i].name, bus[i].ctrdef, bus[i].llMeas, bus[i].ulMeas, bus[i].unit, bus[i].datatype, bus[i].write, bus[i].startByte, bus[i].bit);
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
	readBus(bus, PNrecv, 100);
	for (i=0; i<100; i++){
		printf("%s = %lf or in int: %u \n",bus[i].name, bus[i].dval, bus[i].busval);
	}
	printf("Closing...\n");
	return EXIT_SUCCESS;
}

int readBuslist(const char *busfile, struct busObj *bus, int n){
	FILE *fp;
	char strbuf1[BUF], strbuf2[BUF];
	char *tok, *cptr;
	int k, i, j;
	uint16_t offsetAddr = 0;
	if ((fp=fopen(busfile,"r")) == NULL) {
		fprintf(stderr, "Problem opening file '%s' : %s\n", busfile, strerror(errno));
		return EXIT_FAILURE;
	}
	i=0;
	while (fgets(strbuf1, BUF, fp) && i<n){	//row-wise extraction of strings
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
					bus[i].write = false;
				}
				else bus[i].write = true; // output / write
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
	fclose(fp);
	return EXIT_SUCCESS;
}

double buscode2double(struct busObj *bus, uint8_t *recvstream) {
	// convert uint16 coded value to double value
	double retval, a, b;
	const int span = MAX_I-MIN_I;
	char errormsg[200];
	// read number from byte stream and put into busval:
	bus->busval = (recvstream[bus->startByte] << 8) | (recvstream[bus->startByte+1]); // convert from big endian to to little endian
	// coefficients for conversion to real world values:
	a = (bus->ulMeas-bus->llMeas) / ((double) (MAX_I - MIN_I));
	b = bus->llMeas - a*(double) MIN_I;
	// check for channel error:
	if (bus->busval==0) { // 0 is sent for submit channel error
		if (bus->chn_err_trt < 0){ // use lower limit of measurement range
			if (bus->chn_err_msg) {
				sprintf(errormsg, "WARNING: Channel error in %s, %s: using lower limit of range", bus->tag, bus->name);
				fprintf(stderr,"%s\n",errormsg);
			}
			bus->busval = MIN_I; // set to MIN_I
			retval = a * (double) (bus->busval) + b;
		}
		else if (bus->chn_err_trt > 0) { // use upper limit of measurement range
			if (bus->chn_err_msg) {
				sprintf(errormsg, "WARNING: Channel error in %s, %s: using upper limit of range", bus->tag, bus->name);
				fprintf(stderr,"%s\n",errormsg);
			}
			bus->busval = MAX_I; // set to MAX_I
			retval = a * (double) (bus->busval) + b;
		}
		else { 					// set to default value
			if (bus->chn_err_msg) {
				sprintf(errormsg, "WARNING: Channel error in %s, %s: using nominal value", bus->tag, bus->name);
				fprintf(stderr,"%s\n",errormsg);
			}
			retval = a * (double) (bus->nominalVal) + b;
		}
	}
	// check for "out of range" and set to reasonable values:
	else if (bus->busval < MIN_I - (uint16_t) (TOLERANCE*span)) {
		bus->busval = MIN_I - (uint16_t) (TOLERANCE*span); // set to MIN_I - TOLERANCE*span
		retval = a * (double) (bus->busval) + b;
	}
	else if (bus->busval > MAX_I + (uint16_t) (TOLERANCE*span)) {
		bus->busval = MAX_I + (uint16_t) (TOLERANCE*span); // set to MAX_I + TOLERANCE*span
		retval = a * (double) (bus->busval) + b;
	}
	else { // This is default...
		retval = a * (double) (bus->busval) + b;
	}
	bus->dval = retval;
	return retval;
}

int readBus(struct busObj *bus, uint8_t *recvstream, int n){
	// function advances trough bus struct array and fills its respective values using the byte array recvstream
	int i;

	// loop over all n busObj and case-distinct between type of values
	for (i=0; i<n; i++){
		if (bus[i].write == false) { // it is a read variable:
			switch (bus[i].datatype){
			case 1:		// BOOL
				bus[i].bval = getBit(recvstream[bus[i].startByte], bus[i].bit);
				break;
			case 2: 	// INT = int16_t
				bus[i].ival = (int) getBEint16((int16_t*) recvstream[bus[i].startByte]); // TODO: check if correct
				break;
			case 3: 	// UINT = uint16_t
				bus[i].ival = (int) getBEuint16((uint16_t*) recvstream[bus[i].startByte]); // TODO: check if correct
				break;
			case 4: 	// UDINT = uint32_t
				bus[i].ival = (int) getBEuint32((uint32_t*) recvstream[bus[i].startByte]); // TODO: check if correct
				break;
			case 5: 	// Scaled Real
				buscode2double(&bus[i], recvstream);
				break;
			case 6:		// REAL = 32 bit float
				bus[i].dval = (double) getBEreal((float*) recvstream[bus[i].startByte]);
				break;
			default :
				printf("WARNING: Case not existent in function readBus.\n");
				return EXIT_FAILURE;
			}
		}
	}
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


