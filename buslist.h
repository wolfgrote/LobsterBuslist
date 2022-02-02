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
	char name[BUS_NAME_MAX];		// Name of variable
	char tag[BUS_TAG_MAX];			// Tag no. of variable
	char unit[BUS_UNIT_MAX];		// Physical unit of entity
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
	bool rw;					// value read or write in this program: read=0, write=1
	int channelErr;				// on channel error use: (-1: lower limit of range, 1: upper limit of range, 0: nominal value)
	double nominalVal;			// nominal value
	bool errMsg;				// display error message on screen on channel error
	// constructor:
	busObj():num(0), name("empty"),tag("empty"),unit("1"),datatype(0),bval(false),ival(0),ulval(0),dval(0.0),busval(0),llMeas(0.0), ulMeas(1.0), startByte(0), bit(0), rw(false),channelErr(0), nominalVal(0.0), errMsg(false) { }
};




#endif /* BUSLIST_H_ */
