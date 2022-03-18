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

struct busObj bus[500];
uint8_t PNsend[PN_SENDSIZE]={0}, PNrecv[PN_RECVSIZE]={0};


int main(void) {
	int i, n, tmp;
	bool loop = true;
	char keystr[2], key;
	uint16_t be_num=0xe02e;
	n=readBuslist(FILENAME, bus, 1440);
	printf("Bus list has n=%d elements.\n",n);
	i = 0;
	printf("float has %lu bits\n",sizeof(float)*CHAR_BIT);
	for (i=0; i<PN_RECVSIZE/2;i++){
		memcpy(&PNrecv[2*i], &be_num, 2*sizeof(uint8_t));
	}
	for (i=0; i<n; i++){
		if (bus[i].write == true){
			if (bus[i].datatype == 5){ //buscode
				bus[i].dval = (bus[i].ulMeas + bus[i].llMeas) / 2.0;
			}
			else if (bus[i].datatype == 2){ //int
				bus[i].ival = 3;
			}
			else if (bus[i].datatype == 1){ //bool
				bus[i].bval = true;
			}
		}
	}
	readBus(bus, PNrecv, n, true);
	writeBus(bus, PNsend, n, true);
	for (i=0; i<n; i++){
		if (bus[i].datatype == 5 || bus[i].datatype == 6)
			printf("%s = %g or in int: %u \n",bus[i].name, bus[i].dval, bus[i].busval);
		else if (bus[i].datatype == 2 || bus[i].datatype == 3)
			printf("%s = %d\n",bus[i].name, bus[i].ival);
		else if (bus[i].datatype == 4)
			printf("%s = %lu\n",bus[i].name, bus[i].ulval);
		else if (bus[i].datatype == 1){
			printf("%s = %d\n",bus[i].name, bus[i].bval);
		}
	}
	for (i=0; i<PN_SENDSIZE/2; i++){
		printf("PNsend[%d|%d] = %x|%x  ",2*i, 2*i+1, PNsend[2*i], PNsend[2*i+1]);
	}
	while (loop){ // infinite loop for signal checking
		printf("i: read input from PCS7, o: change output variable to PCS7, l: display bus list, s: print send stream, r: print receive stream, q: quit\n");
		printf("Enter key: ");
		scanf("%s", keystr);
		printf("\n");
		key = keystr[0];
		switch (key) {
		case 'i':
			printf("Enter signal number: ");
			scanf("%d", &i);
			printf("\n");
			i--;		// decrement to get the array element
			if (i < n && bus[i].write==false){
				if (bus[i].datatype == 5 || bus[i].datatype == 6)
					printf("sig %d: %s = %g or in int: %u \n",i+1, bus[i].name, bus[i].dval, bus[i].busval);
				else if (bus[i].datatype == 2 || bus[i].datatype == 3)
					printf("sig %d: %s = %d\n", i+1, bus[i].name, bus[i].ival);
				else if (bus[i].datatype == 4)
					printf("sig %d: %s = %lu\n",i+1, bus[i].name, bus[i].ulval);
				else if (bus[i].datatype == 1){
					printf("sig %d: %s = %d\n",i+1, bus[i].name, bus[i].bval);
				}
			}
			else {
				printf("Failed: Signal number is beyond range or write signal.\n");
			}
			break;
		case 'o':
			printf("Enter signal number: ");
			scanf("%d", &i);
			printf("\n");
			i--;		// decrement to get the array element
			if (i < n && bus[i].write==true){
				if (bus[i].datatype == 5 || bus[i].datatype == 6){ // real
					printf("sig %d: %s = %g ... %g -> real value = ",i+1, bus[i].name, bus[i].llMeas, bus[i].ulMeas);
					scanf("%lf", &bus[i].dval);
					printf("\n");
					writeBus(bus, PNsend, n, true);
					printf("sig %d: value = %lf, uint16 coded: %u\n", i+1, bus[i].dval, bus[i].busval);
				}
				else if (bus[i].datatype == 2 || bus[i].datatype == 3){ // (unsigned) short integer
					printf("sig %d: %s integer value = ",i+1, bus[i].name);
					scanf("%d", &bus[i].ival);
					printf("\n");
					writeBus(bus, PNsend, n, true);
					printf("sig %d: value = %d\n", i+1, bus[i].ival);
				}
				else if (bus[i].datatype == 4){ // unsigned long
					printf("sig %d: %s unsigned long integer value = ",i+1, bus[i].name);
					scanf("%lu", &bus[i].ulval);
					printf("\n");
					writeBus(bus, PNsend, n, true);
					printf("sig %d: value = %lu\n", i+1, bus[i].ulval);
				}
				else if (bus[i].datatype == 1){ // boolean
					printf("sig %d: %s boolean value (0/1) = ",i+1, bus[i].name);
					scanf("%d", &tmp);
					bus[i].bval = tmp;
					printf("\n");
					writeBus(bus, PNsend, n, true);
					printf("sig %d: value = %d\n", i+1, bus[i].bval);
				}
			}
			else {
				printf("Failed: Signal number is beyond range or read signal.\n");
			}
			break;
		case 'l':	// display bus list
			i = 0;
			do {
				printf("A[%3d] = %3d, %s, %s, %4s, %lf, %lf, %s, %3d, %d->%d.%d\n", i, bus[i].num, bus[i].tag, bus[i].name, bus[i].ctrdef, bus[i].llMeas, bus[i].ulMeas, bus[i].unit, bus[i].datatype, bus[i].write, bus[i].startByte, bus[i].bit);
				i++;
			}
			while (bus[i].num > bus[i-1].num && i < 1440);
			break;
		case 's':	// display send stream
			for (i=0; i<PN_SENDSIZE/2; i++){
				printf("PNsend[%d|%d] = %x|%x  ",2*i, 2*i+1, PNsend[2*i], PNsend[2*i+1]);
			}
			printf("\n");
			break;
		case 'r':	// display receive stream
			for (i=0; i<PN_RECVSIZE/2; i++){
				printf("PNrecv[%d|%d] = %x|%x  ",2*i, 2*i+1, PNrecv[2*i], PNrecv[2*i+1]);
			}
			printf("\n");
			break;
		case 'q':
			loop = false;
			break;
		default :
			printf("Key %c not assigned. Select different.\n", key);
			break;
		}
	}


	printf("\nClosing...\n");
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
		return -1;
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
	return i;
}

double buscode2double(struct busObj *bus, uint16_t buscode){
	// convert uint16 coded value to double value
	double retval, a, b;
	const int span = MAX_I-MIN_I;
	char errormsg[200];
	// read number from byte stream and put into busval:
	bus->busval = buscode; // convert from big endian to to little endian
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

uint16_t double2buscode(struct busObj *bus){
	// convert double to little endian uint16 coded bus value
	double a, b;
	const int span = MAX_I-MIN_I;
	// coefficients for conversion to real world values:
	a = (bus->ulMeas-bus->llMeas) / ((double) (MAX_I - MIN_I));
	b = bus->llMeas - a*(double) MIN_I;
	// check for error and set buscode to 0:
	if (bus->chn_err == true){
		bus->busval = 0;
	}
	else {
		bus->busval = (uint16_t) ((bus->dval - b) / a);
	}
	// check if out of bounds:
	if (bus->busval > MAX_I + (uint16_t) (TOLERANCE*span) || bus->busval < MIN_I - (uint16_t) (TOLERANCE*span)){
		bus->chn_err = true;
		bus->busval = 0;
	}
	return bus->busval;
}

int readBus(struct busObj *bus, const uint8_t *recvstream, int n, bool bigendian){
	// function advances trough bus struct array and fills its respective values using the byte array recvstream
	int i;
	int16_t int16_buf=0;
	uint16_t uint16_buf=0;
	uint32_t uint32_buf=0;
	float float_buf=0.0;
	// loop over all n busObj and case-distinct between type of values
	for (i=0; i<n; i++){
		if (bus[i].write == false) { // it is a read variable:
			switch (bus[i].datatype){
			case 1:		// BOOL
				bus[i].bval = getBit(recvstream[bus[i].startByte], bus[i].bit);
				break;
			case 2: 	// INT = int16_t
				memcpy(&int16_buf, &recvstream[bus[i].startByte], 2*sizeof(uint8_t));
				if (bigendian == true){
					bus[i].ival = (int) getBEint16(&int16_buf); // TODO: check if correct
				}
				else {
					bus[i].ival = (int) int16_buf;
				}
				break;
			case 3: 	// UINT = uint16_t
				memcpy(&uint16_buf, &recvstream[bus[i].startByte], 2*sizeof(uint8_t));
				if (bigendian == true){
					bus[i].ival = (int) getBEuint16(&uint16_buf); // TODO: check if correct
				}
				else {
					bus[i].ival = (int) uint16_buf;
				}
				break;
			case 4: 	// UDINT = uint32_t
				memcpy(&uint32_buf, &recvstream[bus[i].startByte], 4*sizeof(uint8_t));
				if (bigendian == true){
					bus[i].ulval = (unsigned long) getBEuint32(&uint32_buf); // TODO: check if correct
				}
				else {
					bus[i].ulval = (unsigned long) uint32_buf;
				}
				break;
			case 5: 	// Scaled Real
				memcpy(&uint16_buf, &recvstream[bus[i].startByte], 2*sizeof(uint8_t));
				if (bigendian == true){
					buscode2double(&bus[i], getBEuint16(&uint16_buf));
				}
				else {
					buscode2double(&bus[i], uint16_buf);
				}
				break;
			case 6:		// REAL = 32 bit float
				memcpy(&float_buf, &recvstream[bus[i].startByte], 4*sizeof(uint8_t));
				if (bigendian == true){
					bus[i].dval = (double) getBEreal(&float_buf);
				}
				else {
					bus[i].dval = (double) float_buf;
				}
				break;
			default :
				printf("WARNING: Case not existent in function readBus.\n");
				return EXIT_FAILURE;
			}
		}
	}
	return EXIT_SUCCESS;
}

int writeBus(struct busObj *bus, uint8_t *sendstream, int n, bool bigendian){
	// function advances trough bus struct array and fills the sendstram array according to busObj values
	int i;
	int16_t int16_buf=0, int16_buf_be=0;
	uint16_t uint16_buf=0, uint16_buf_be=0;
	uint32_t uint32_buf=0, uint32_buf_be=0;
	float float_buf=0.0, float_buf_be=0.0;
	// loop over all n busObj and case-distinct between type of values
	for (i=0; i<n; i++){
		if (bus[i].write == true) { // it is a write variable:
			switch (bus[i].datatype){
			case 1:		// BOOL
				setBit(bus[i].startByte, bus[i].bit, sendstream, bus[i].bval);
				break;
			case 2: 	// INT = int16_t
				int16_buf 		= (int16_t) bus[i].ival;
				if (bigendian == true){
					int16_buf_be 	= getBEint16(&int16_buf);
					memcpy(&sendstream[bus[i].startByte], &int16_buf_be, 2*sizeof(uint8_t));
				}
				else {
					memcpy(&sendstream[bus[i].startByte], &int16_buf, 2*sizeof(uint8_t));
				}

				break;
			case 3: 	// UINT = uint16_t
				uint16_buf 		= (uint16_t) bus[i].ival;
				if (bigendian == true){
					uint16_buf_be 	= getBEuint16(&uint16_buf);
					memcpy(&sendstream[bus[i].startByte], &uint16_buf_be, 2*sizeof(uint8_t));
				}
				else {
					memcpy(&sendstream[bus[i].startByte], &uint16_buf, 2*sizeof(uint8_t));
				}
				break;
			case 4: 	// UDINT = uint32_t
				uint32_buf 		= (uint32_t) bus[i].ival;
				if (bigendian == true){
					uint32_buf_be 	= getBEuint32(&uint32_buf);
					memcpy(&sendstream[bus[i].startByte], &uint32_buf_be, 4*sizeof(uint8_t));
				}
				else {
					memcpy(&sendstream[bus[i].startByte], &uint32_buf, 4*sizeof(uint8_t));
				}
				break;
			case 5: 	// Scaled Real
				uint16_buf = double2buscode(&bus[i]);
				if (bigendian == true){
					uint16_buf_be = getBEuint16(&uint16_buf);
					memcpy(&sendstream[bus[i].startByte], &uint16_buf_be, 2*sizeof(uint8_t));
				}
				else {
					memcpy(&sendstream[bus[i].startByte], &uint16_buf, 2*sizeof(uint8_t));
				}
				break;
			case 6:		// REAL = 32 bit float
				float_buf 		= (float) bus[i].dval;
				if (bigendian == true){
					float_buf_be 	=  getBEreal(&float_buf);
					memcpy(&sendstream[bus[i].startByte], &float_buf_be, 4*sizeof(uint8_t));
				}
				else {
					memcpy(&sendstream[bus[i].startByte], &float_buf, 4*sizeof(uint8_t));
				}
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

void setBit(int byte_indx, int bit_indx, uint8_t *sendstream, bool value) {
	// sets the bit_indx bit within byte_indx to val.
	uint8_t mask = 1 << bit_indx;
	if (value==true){ // set the bit to 1
		sendstream[byte_indx] = mask | sendstream[byte_indx];
	}
	else {// set the bit to 0
		sendstream[byte_indx] = (~mask) & sendstream[byte_indx];
	}
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




