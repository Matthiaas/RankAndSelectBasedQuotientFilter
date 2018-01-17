#include "CQF.h"


namespace CountingFilter{

CQF::CQF(double false_positive_rate, int n) {
	int p = (int) (log2(n / false_positive_rate));
	if (p > 64) {
		ERROR(FALSE_POSITIVE_REDUCE);
		p = 64;
	}
	hashMask = BIT_MASK(p);
	remMask = BIT_MASK(REMAINDER_LEN);

	quotient = p - REMAINDER_LEN;
	remainder = REMAINDER_LEN;
	size = ((uint64_t) 1) << (quotient);

	//This only works for REMAINDER_LEN > 1
	encodingBase = (((uint64_t) 1) << (REMAINDER_LEN))-2;

	uint64_t value = 1l << 63;

	blocks = new Block[size / 64 + 1];

	//Set all fields to zero 
	//and set every first runend und occupied bit in each block
	for (uint64_t i = 0; i < size / 64; i++) {
		blocks[i].offset = 0;
		blocks[i].used = 0;
		blocks[i].occupied = value;
		blocks[i].runend = value;
		for (int j = 0; j < REMAINDER_LEN; j++)
			blocks[i].remainder[j] = 0;
	}

}


CQF::~CQF() {
	delete[] blocks;
}


// returns select( runend, rank(occupied , h0))
uint64_t CQF::rankSelect(uint64_t h0) {
	if (h0 % 64 == 0) {
		return blocks[h0 / 64].offset + h0;
	} else {
		uint64_t i = BLOCK_I(h0);
		uint64_t d = rankOcc(&blocks[i], 1, BIT_I(h0) - 1Ul);
		if(d==0)
			return i * 64 + blocks[i].offset ;
		else{
			uint64_t BI = BLOCK_I(i * 64 + blocks[i].offset + 1);
			uint8_t BITI = BIT_I(blocks[i].offset + 1);
			uint64_t t = selectRunEnd(&blocks[BI], BITI, d);
			return i * 64 + blocks[i].offset + t +1;
		}
	}

}

//Finds the first unused slot beyond or at the position of x
uint64_t CQF::find_first_unused_slot(uint64_t x) {
	uint64_t s = rankSelect(x);
	while (x <= s) {
		x = s + 1;
		s = rankSelect(x);
	}
	return x;
}

//Clears a remainder in a block at a given index
inline void clearRemainder(Block &b, uint64_t index) {
	index *= REMAINDER_LEN;
	b.remainder[index / 64] &= ~(BIT_MASK(REMAINDER_LEN) << (64 - REMAINDER_LEN - (index % 64)));
}
//Sets a remainder in a block at a given index
inline void setRemainder(Block &b, uint64_t index, uint64_t value) {
	index *= REMAINDER_LEN;
	b.remainder[index / 64] |= value << (64 - REMAINDER_LEN - (index % 64));
}
//Gets a remainder in a block at a given index
inline uint64_t getRemainder(Block &b, uint64_t index, uint64_t remMask) {
	index *= REMAINDER_LEN;
	return (b.remainder[index / 64] >> (64 - REMAINDER_LEN - (index % 64)))
			& remMask;
}

//Shifts a remainder by one 
//returns carry if remainder was shifted out
uint64_t shiftRemainder(Block &b, uint64_t from, uint64_t to,
		uint64_t remMask) {
	if (from > to)
		return 0;
	from *= REMAINDER_LEN;
	to = to * REMAINDER_LEN + REMAINDER_LEN - 1;
	uint64_t startBlock = BLOCK_I(from);
	uint64_t endBlock = BLOCK_I(to);
	uint64_t pushCarry = 0;
	if (startBlock == endBlock) {
		if (BIT_I(to) == 63) {
			pushCarry = b.remainder[startBlock] & remMask;
		}
		shiftRight(&b.remainder[startBlock], BIT_I(from), BIT_I(to),
				REMAINDER_LEN);
		if (BIT_I(to) == 63) {
			if (startBlock < REMAINDER_LEN - 1) {
				b.remainder[startBlock + 1] |= pushCarry
						<< (64 - REMAINDER_LEN);
			} else {
				return pushCarry;
			}
		}
	} else {
		uint64_t carry = b.remainder[startBlock] & remMask;
		shiftRight(&b.remainder[startBlock], BIT_I(from), 63, REMAINDER_LEN);
		for (uint64_t i = startBlock + 1; i < endBlock; i++) {
			uint64_t carry1 = b.remainder[i] & remMask;
			b.remainder[i] >>= REMAINDER_LEN; //Shift by remainder len
			b.remainder[i] |= carry << (64 - REMAINDER_LEN); // Set carry
			carry = carry1;
		}
		if (BIT_I(to) == 63) {
			pushCarry = b.remainder[endBlock] & remMask;
		}
		shiftRight(&b.remainder[endBlock], 0, BIT_I(to), REMAINDER_LEN);
		b.remainder[endBlock] |= carry << (64 - REMAINDER_LEN); // Set carry
		if (BIT_I(to) == 63) {
			if (endBlock < REMAINDER_LEN - 1) {
				b.remainder[endBlock + 1] |= pushCarry << (64 - REMAINDER_LEN);
			} else {
				return pushCarry;
			}
		}
	}
	return 0;

}


inline uint64_t getNextRem(uint64_t pos , Block * blocks , uint64_t  h0, uint64_t remMask){
	if(pos >0 && pos-1 >= h0 && !isBit(blocks[BLOCK_I(pos-1)].runend, BIT_I(pos-1))){
		return getRemainder(blocks[BLOCK_I(pos-1)], BIT_I(pos-1), remMask);

	}else{
		return remMask +2; // A remainder, which is not possible
	}
}

//returns ONE, TWO, THREE or COUNT
//if ONE, TWO or THREE is returned this is the actual count of the Remainder rem
//if COUNT is returned pos points to the position of the endcoded counter
uint8_t CQF::runContainsRemainder(Block * blocks, uint64_t h0, uint64_t rem, uint64_t &pos  ) {
	uint64_t lastr = 0;
	bool firstLoop = true;
	do {
		uint64_t r = getRemainder(blocks[BLOCK_I(pos)], BIT_I(pos), remMask);

		if(lastr > r){// A Counter was found
			//Skip Counter
			do{
				pos--;
				r = getRemainder(blocks[BLOCK_I(pos)], BIT_I(pos), remMask);
			}while(r != lastr);

		}else if (rem == r) { // We found the first part of the Counter encoding
			r  = getNextRem(pos ,blocks, h0, remMask);
			if(r > rem  ){

				if(rem != 0){
					// The Encoding is One because there is only one occurrence of rem and rem is not zero
					return ONE;
				}else{
					// The encoding can be higher than one if there are two zeros (0, c_{l−1} , . . . , c_0 , 0, 0) in the run
					uint64_t varPos = pos;
					do{
						lastr = r;
						r  = getNextRem(varPos ,blocks, h0, remMask);
						varPos--;
					}while((lastr || r) &&  r != remMask +2);// repeat until both are Zero or the run ends;
					if(! lastr && !r ){
						// A zero counter was found, so the index of the postion of the counter will be returned
						pos = varPos+2; // +2 , because of the last two zeros
						return COUNT;
					}else{
						// This was zero counter
						return ONE;
					}
				}
			}else if(r == rem){
				// two times r in a row
				pos--;
				if(rem == 0 ){
					//The Encoding must be two or three only if rem = 0 ther can be 3 zeros in a row
					r  = getNextRem(pos ,blocks, h0, remMask);
					if(r == rem){
						//three times r in a row
						return THREE;
					}else{
						return TWO;
					}
				}else{
					// The Ecoding must be two
					return TWO;
				}
			}else{
				pos--;
				do{
					r = getNextRem(pos ,blocks, h0, remMask);
					pos--;
				}while(r!= rem);
				pos++;//Set the index to the first index of the counter
				return COUNT;
			}



		}
		else if(r > rem ){
			return false;

		}else if(r == 0){
			//This could be a  counter for zeros!
			uint64_t varPos = pos;
			uint64_t saveR = r;
			r++;
			do{
				lastr = r;
				r  = getNextRem(varPos ,blocks, h0, remMask);
				varPos--;
			}while((lastr || r) &&  r != remMask +2);// repeat until both are Zero or the run ends;
			if(! lastr && !r ){
				pos = varPos ;//Set the index to t
				lastr = 0;
			}else{
				r = saveR;
			}

		}
		lastr = r;
		pos--;
		if(pos == ULONG_MAX) return false; // return because position == -1
		firstLoop = false;
	} while (pos >= h0 && !isBit(blocks[BLOCK_I(pos)].runend, BIT_I(pos)));
	return false;
}

//Decodes one singel encodeing number to its real number
inline uint64_t encodeingToNumber(uint64_t x , uint64_t enc){
	if(enc > x && x!= 0)
		return enc - 2;
	else
		return enc - 1;
}
// encodes one singel number to its encoding
inline uint64_t numberToEncoding(uint64_t x , uint64_t number){
	number++;
	if(number == x)
		return number +1;
	else
		return number;
}


//returns Carry
inline bool addOneToEncoding(uint64_t x , uint64_t &enc , uint64_t remMask){
	//zero was not allowed
	if(enc == 0) enc++;

	if(enc == x){
		enc++;
	}
	enc++;
	if(enc == x){
		enc++;
	}
	if(enc >= remMask+1){
		//Return carry and set the number of the encoding to 0
		if(x == 1) {
			enc = 2;
		}else{
			enc = 1;
		}
		return 1;
	}
	return false;



}

// Shifts the runends between index s and the next free slot by one to the rigth 
// Shifts the remainders between index slot and the next free slot by one to the rigth 
 
// returns true if insert is possible
bool CQF::shiftRunendAndRemaindersByOne(uint64_t s, uint64_t slot){
	//Find next free slot
	uint64_t n = find_first_unused_slot(s);

	//Check if this is inside the filter
	if (n >= size) {
		ERROR(FILTER_TO_SMALL);
		return false;
	}
	uint64_t blockIndexS = BLOCK_I(s);
	uint64_t blockIndexN = BLOCK_I(n-1);
	uint64_t blockIndexSLOT = BLOCK_I(slot);

	//UPDATE OFFSTES
	for (uint64_t i = blockIndexN; i >= 0; i--) {
		if ((s < i * 64 || s - i * 64 <= blocks[i].offset)
				&& blocks[i].offset <= n - 1 - i * 64) {
			if (blocks[i].offset >= 127)
				ERROR(OFFSET_TO_SMALL);
			else
				blocks[i].offset++;
		} else if (s > i * 64 && blocks[i].offset < s - i * 64) {
			break;
		}
	}



	// Carry values that should be pushed in the next block
	bool pushCarry = 0;
	uint64_t pushCarryRem = 0;

	//SHIFTING REMAINDER RIGTH 
	if(blockIndexSLOT >= blockIndexN){
		//Shifting inside of one Block
		pushCarryRem = shiftRemainder(blocks[blockIndexSLOT], BIT_I(slot),BIT_I(n - 1), remMask);
	}else{
		//Shifting over blocks

		//First block
		uint64_t carryRem = blocks[blockIndexSLOT].remainder[REMAINDER_LEN - 1]& remMask;
		shiftRemainder(blocks[blockIndexSLOT], BIT_I(slot), 63, remMask);

		//In between blocks
		for (uint64_t i = blockIndexSLOT + 1; i < blockIndexN; i++) {
			uint64_t carryRem1 = blocks[i].remainder[REMAINDER_LEN - 1] & remMask; //Remember Carry
			shiftRemainder(blocks[i], 0, 63, remMask);// Shift rigth by one
			blocks[i].remainder[0] |= carryRem << (64 - REMAINDER_LEN);//Set Carry
			carryRem = carryRem1;
		}

		//last block
		pushCarryRem = shiftRemainder(blocks[blockIndexN], 0, BIT_I(n - 1),	remMask);// Shift rigth by one
		blocks[blockIndexN].remainder[0] |= carryRem<< (64 - REMAINDER_LEN); //Set Carry

	}


	//SHIFTING RUNEND RIGTH
	
	if (blockIndexS >= blockIndexN) {
		//Shifting inside of one Block
		if (BIT_I(n - 1) == 63) {
			pushCarry = blocks[blockIndexS].runend & 1;
		}
		shiftRight(&blocks[blockIndexS].runend, BIT_I(s), BIT_I(n - 1), 1);
	} else {
		//Shifting over blocks
		bool carry = blocks[blockIndexS].runend & 1;
		shiftRight(&blocks[blockIndexS].runend, BIT_I(s), 63, 1);
		for (uint64_t i = blockIndexS + 1; i < blockIndexN; i++) {
			bool carry1 = blocks[i].runend & 1;
			blocks[i].runend = blocks[i].runend >> 1;
			if (carry) {
				setBit(blocks[i].runend, 0);
			}
			carry = carry1;
		}
		if (BIT_I(n - 1) == 63) {
			pushCarry = blocks[blockIndexN].runend & 1;
		}
		shiftRight(&blocks[blockIndexN].runend, 0, BIT_I(n - 1), 1);
		if (carry) {
			setBit(blocks[blockIndexN].runend, 0);
		}
	}
	

	//Push carry in next block
	if (BIT_I(n - 1) == 63) {
		setRemainder(blocks[blockIndexN + 1], 0, pushCarryRem);
		if (pushCarry) {
			setBit(blocks[blockIndexN + 1].runend, 0);
		}
	}


	return true;
}

//Iserts an Element into the filter
void CQF::insert(Element e) {
	uint64_t h = e.hash1() & hashMask;
	uint64_t h0 = h >> remainder;
	uint64_t rem = h & remMask;

	uint64_t blockIndex = BLOCK_I(h0);
	uint8_t bitIndex = BIT_I(h0);

	uint64_t s = rankSelect(h0);
	if (h0 % 64 == 0 && !blocks[blockIndex].used) {
		//if this is the first bit in the Block and it is not used already
		setRemainder(blocks[BLOCK_I(h0 + blocks[blockIndex].offset)],
				BIT_I(h0 + blocks[blockIndex].offset), rem);
		blocks[blockIndex].used = 1;
	} else if (h0 > s) {
		//Home slot is free
		setRemainder(blocks[blockIndex], bitIndex, rem);
		setBit(blocks[blockIndex].runend, bitIndex);
	} else {
		s++;

		// this can be 0 , 1 or 2
		// If and oly if the remainder is 1 this can be 2 and newCounterField2 will be used!
		uint8_t newAllocForCounter = 0;

		uint64_t newCounterField1 = 0;
		uint64_t newCounterField2 = 0;

		uint64_t slot;
		// If slot is Already occupied, the filter may needs a counter
		if(isBit(blocks[blockIndex].occupied, bitIndex)){

			slot = s - 1;
			uint64_t count  = runContainsRemainder(blocks, h0, rem, slot);
			if(count == TWO && rem == 0){
				slot ++;
			}else if((count == TWO || count == THREE ) && rem != 1){
				// new counter will be 1 --> Ecoded as 0 + 3 = 3
				newAllocForCounter = 1;
				newCounterField1 = 1;
				slot ++;
			}else if(rem == 1 && count == TWO){
				//new counter will be 0,2 because 2 is greater than one we have to push a zero to the front
				newAllocForCounter = 2;
				newCounterField1 = 2;
				newCounterField2 = 0;
				slot ++;
			}else if(count == COUNT){
				//Increase Counter
				uint64_t num  = getRemainder(blocks[BLOCK_I(slot)], BIT_I(slot), remMask);
				uint64_t lastSet = 0;
				bool carry;
				do{
					carry = addOneToEncoding(rem , num , remMask);
					lastSet = num;
					clearRemainder(blocks[BLOCK_I(slot)] , BIT_I(slot)  );
					setRemainder(blocks[BLOCK_I(slot)] , BIT_I(slot) , num );
					slot++;
					num  = getRemainder(blocks[BLOCK_I(slot)], BIT_I(slot), remMask);
				}while(rem != num && carry );
				if(carry){
					newAllocForCounter = 1;
					newCounterField1 = numberToEncoding(rem , 1);
				}else if(rem != 0 && rem == num &&lastSet >= rem){
					//Contradiction with counting invariants, push zero in front encoded 0 = 1
					newAllocForCounter = 1;
					newCounterField1 = 0;
				}else{
					return;
				}

			}else{
				slot++;
			}
			//else // No counter needed

		}else{
			slot = s;
		}



		//Shifting
		//Quit if shift did not work
		if(!shiftRunendAndRemaindersByOne(s, slot)){
			return;
		}
		if(newAllocForCounter == 2){
			if(!shiftRunendAndRemaindersByOne(s+1, slot+1)){
				ERROR(FILTER_BROKEN);
				return;
			}
		}

		//Update offset if element was added to homeslot with bit index 0
		if (BIT_I(h0) == 0) {
			if (blocks[blockIndex].offset >= 127){
				ERROR(OFFSET_TO_SMALL);
			}
			else{
				blocks[blockIndex].offset++;
				if(newAllocForCounter == 2){
					if (blocks[blockIndex].offset >= 127){
						ERROR(OFFSET_TO_SMALL);
					}else{
						blocks[blockIndex].offset++;
					}
					
				}
			}
		}


		//Set remainder
		switch (newAllocForCounter) {
		case 0:
			// Element was inserted first or second time (or maybe the third time if rem = 0)
			setRemainder(blocks[BLOCK_I(slot)], BIT_I(slot), rem);
			break;
		case 1:
			//new part of the counter was added
			setRemainder(blocks[BLOCK_I(slot)], BIT_I(slot), newCounterField1);
			break;
		case 2:
			// this only happens if a rem  = 3 is inserted the third time in the same homeslot 
			setRemainder(blocks[BLOCK_I(slot)], BIT_I(slot), newCounterField1);
			setRemainder(blocks[BLOCK_I(slot+1)], BIT_I(slot+1), newCounterField2);
			break;
		default:
			ERROR(UNDF_BEHV);
		}
		



		//Unset old runend bit (if there was one)
		if (isBit(blocks[blockIndex].occupied, bitIndex) ) {
			unSetBit(blocks[BLOCK_I(s-1)].runend, BIT_I(s-1));
		}
		//Update s for resetting
		if(newAllocForCounter == 2){
			s++;
		}

		//reset runend bit
		setBit(blocks[BLOCK_I(s)].runend, BIT_I(s));

	}

	//Set occupied bit (if not already set)
	setBit(blocks[blockIndex].occupied, bitIndex);
}


uint64_t CQF::query(Element e) {
	uint64_t h = e.hash1() & hashMask;
	uint64_t h0 = h >> remainder;
	uint64_t rem = h & remMask;

	if (!isBit(blocks[BLOCK_I(h0)].occupied, BIT_I(h0))
			|| (h0 % 64 == 0 && !blocks[BLOCK_I(h0)].used)) {
		return false;
	}
	uint64_t l = rankSelect(h0);
	uint64_t result= runContainsRemainder(blocks, h0, rem, l);

	if(result == COUNT){
		//Decode the counter
		uint64_t curMul = 1;
		// get the correct base (base differs for rem = 0)
		uint64_t base = GET_BASE(rem);
		uint64_t num  = getRemainder(blocks[BLOCK_I(l)], BIT_I(l), remMask);
		result = 0;
		do{
			if(num != 0)//we would get some wrong decoding here
				result += encodeingToNumber(rem , num)*curMul;
			curMul*= base;
			l++;
			num  = getRemainder(blocks[BLOCK_I(l)], BIT_I(l), remMask);

		}while(rem != num );
		if(rem == 0){
			//count is encoded as count-4 if rem = 0
			return result + 4;
		}else{
			//count is encoded as count-3 if rem != 0
			return result + 3;
		}


	}else{
		return result;
	}

}


}