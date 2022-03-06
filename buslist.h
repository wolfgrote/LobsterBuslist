/*
 * buslist.h
 *
 *  Created on: Nov 25, 2021
 *      Author: wolfgang
 */

#ifndef BUSLIST_H_
#define BUSLIST_H_
#define BUS_NAME_MAX 100
#define BUS_TAG_MAX 50
#define BUS_UNIT_MAX 20


struct busObj { 				// Object for bus communication
	int num; 					// Number of signal in bus list
	char name[BUS_NAME_MAX];	// Name of variable
	char tag[BUS_TAG_MAX];		// Tag no. of variable
	char unit[BUS_UNIT_MAX];	// Physical unit of entity
	int datatype;				// 1=BOOL, 2=INT, 3=UINT, 4=UDINT, 5=SCALED REAL, 6 = REAL
	bool bval;					// Boolean value
	int ival;					// Integer value
	unsigned long ulval;		// Unsigned long value
	double dval;				// double value
	uint16_t busval;			// scaled value as used by bus (valid range: 4,000 - 20,000)
	double llMeas;				// min value of measurement range corresponding to busval = 4,000
	double ulMeas;				// max value of measurement range corresponding to busval = 20,000
	uint16_t startByte;			// start byte in stream
	uint8_t	bit;				// bit in start byte for boolean values
	bool write;					// value read or write in this program: read=0, write=1
	double nominalVal;			// nominal value
	bool chn_err;				// channel error state
	bool chn_err_msg;			// display error message on screen on channel error
	int chn_err_trt;			// on channel error use: (-1: lower limit of range, 1: upper limit of range, 0: nominal value)
	char ctrdef[5];				// definition of control quantity (u, w, y, d) followed by number (1, 2, ..) e.g. w3 for 3rd set point
	// constructor:
	busObj():num(0), name("empty"),tag("empty"),unit("1"),datatype(0),bval(false),ival(0),ulval(0),dval(0.0),busval(0),llMeas(0.0), ulMeas(1.0), startByte(0), bit(0), write(false), nominalVal(0.0), chn_err(false), chn_err_msg(false), chn_err_trt(1), ctrdef("none") { }
};

int readBuslist(const char *busfile, struct busObj *bus, int n);

double buscode2double(struct busObj *bus, uint16_t buscode);

uint16_t double2buscode(struct busObj *bus);

int readBus(struct busObj *bus, const uint8_t *recvstream, int n, bool bigendian);

int writeBus(struct busObj *bus, uint8_t *sendstream, int n, bool bigendian);

char* replace_char(char* str, char find, char replace);

int16_t getBEint16(int16_t *bigEndianNumber);

uint16_t getBEuint16(uint16_t *bigEndianNumber);

uint32_t getBEuint32(uint32_t *bigEndianNumber);

float getBEreal(float *bigEndianNumber);

bool getBit(uint8_t bitArray, int n);

void setBit(int byte_indx, int bit_indx, uint8_t *sendstream, bool value);

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


#endif /* BUSLIST_H_ */
