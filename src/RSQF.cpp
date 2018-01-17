#include "RSQF.h"



namespace Filter{

RSQF::RSQF(double false_positive_rate, int n) {
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

	uint64_t value = 1l << 63;
	blocks = new Block[size / 64 + 1];

	//Set all fields to zero 
	//and set every first runend und occupied bit in each block
	for (uint64_t i = 0; i < size / 64; i++) {
		blocks[i].offset = 0;
		blocks[i].used = 0;
		blocks[i].occupied = value;
		blocks[i].runend = value;
		for(int j = 0; j < REMAINDER_LEN; j++)
			blocks[i].remainder[j] = 0;
	}

}

RSQF::~RSQF() {
	delete[] blocks;
}


// returns select( runend, rank(occupied , h0))
uint64_t RSQF::rankSelect(uint64_t h0) {
	if (h0 % 64 == 0) {
		return blocks[h0 / 64].offset + h0;
	} else {
		uint64_t i = BLOCK_I(h0);
		uint64_t d = rankOcc(&blocks[i], 1, BIT_I(h0) - 1Ul);
		uint64_t BI = BLOCK_I(i*64 + blocks[i].offset +1);
		uint8_t BITI = BIT_I( blocks[i].offset + 1);
		uint64_t t = selectRunEnd(&blocks[BI], BITI, d);
		if(d==0)
			return i * 64 + blocks[i].offset + t ;
		else
			return i * 64 + blocks[i].offset + t +1;
	}

}

//Finds the first unused slot beyond or at the position of x
uint64_t RSQF::find_first_unsed_slot(uint64_t x) {
	uint64_t s = rankSelect(x);
	while (x <= s) {
		x = s + 1;
		s = rankSelect(x);
	}
	return x;
}

//Sets a remainder in a block at a given index
inline void setRemainder(Block &b , uint64_t index , uint64_t value){
	index *= REMAINDER_LEN;
	b.remainder[index/64] |= value << (64 - REMAINDER_LEN -(index % 64));
}

//Gets a remainder in a block at a given index
inline uint64_t getRemainder(Block &b , uint64_t index , uint64_t remMask){
	index *= REMAINDER_LEN;
	return (b.remainder[index/64] >> (64 - REMAINDER_LEN -(index % 64)) ) & remMask;
}

//Shifts a remainder by one in a block
//returns carry if remainder was shifted out
uint64_t shiftRemainder(Block &b , uint64_t from ,uint64_t to  , uint64_t remMask){
	if(from > to) return 0;
	from *= REMAINDER_LEN;
	to = to *REMAINDER_LEN + REMAINDER_LEN -1;
	uint64_t startBlock = BLOCK_I(from);
	uint64_t endBlock = BLOCK_I(to);
	uint64_t pushCarry = 0;
	if(startBlock == endBlock){
		if(BIT_I(to) == 63){
			pushCarry = b.remainder[startBlock] & remMask;
		}
		shiftRight(&b.remainder[startBlock], BIT_I(from) , BIT_I(to) , REMAINDER_LEN);
		if(BIT_I(to) == 63){
			if(startBlock < REMAINDER_LEN - 1){
				b.remainder[startBlock+1] |= pushCarry << (64 - REMAINDER_LEN);
			}else{
				return pushCarry;
			}
		}
	}else{
		uint64_t carry = b.remainder[startBlock] & remMask;
		shiftRight(&b.remainder[startBlock], BIT_I(from) , 63 , REMAINDER_LEN);
		for(int i = startBlock +1; i < endBlock ; i++){
			uint64_t carry1 = b.remainder[i] & remMask;
			b.remainder[i] >>= REMAINDER_LEN;//Shift by remainder len
			b.remainder[i] |= carry << (64 - REMAINDER_LEN); // Set carry
			carry = carry1;
		}
		if(BIT_I(to) == 63){
			pushCarry = b.remainder[endBlock] & remMask;
		}
		shiftRight(&b.remainder[endBlock],  0, BIT_I(to) , REMAINDER_LEN);
		b.remainder[endBlock] |= carry << (64 - REMAINDER_LEN); // Set carry
		if(BIT_I(to) == 63){
			if(endBlock < REMAINDER_LEN - 1){
				b.remainder[endBlock+1] |= pushCarry << (64 - REMAINDER_LEN);
			}else{
				return pushCarry;
			}
		}
	}
	return 0;


}

bool RSQF::shiftRunendAndRemaindersByOne(uint64_t s){

	//Find next free slot
	uint64_t n = find_first_unsed_slot(s);
	if (n >= size) {
		ERROR(FILTER_TO_SMALL);
		return false;
	}
	uint64_t blockIndexS = BLOCK_I(s);
	uint64_t blockIndexN = BLOCK_I(n-1);

	//UPDATE OFFSTES
	for (uint64_t i = blockIndexN; i >= 0; i--) {
		if ((s < i * 64 || s - i * 64 <= blocks[i].offset) && blocks[i].offset <= n - 1 - i * 64) {
			if(blocks[i].offset >= 127) 
				ERROR(OFFSET_TO_SMALL);
			blocks[i].offset++;
		} else if (s > i * 64 && blocks[i].offset < s - i * 64) {
			break;
		}
	}

	// Carry values that should be pushed in the next block
	bool pushCarry = 0;
	uint64_t pushCarryRem = 0;

	//SHIFTING REMAINDER RIGTH 
	if (blockIndexS >= blockIndexN) {
		//Shifting inside of one Block
		if(BIT_I(n - 1) == 63){
			pushCarry = blocks[blockIndexS].runend & 1;
		}

		//TODO: set carry in next block if necessary
		shiftRight(&blocks[blockIndexS].runend, BIT_I(s), BIT_I(n - 1), 1);
		pushCarryRem = shiftRemainder(blocks[blockIndexS] , BIT_I(s) , BIT_I(n-1) , remMask);

	} else {
		//Shifting over blocks

		//Get carrys
		bool carry = blocks[blockIndexS].runend & 1;
		//uint64_t carryRem = blocks[blockIndexS].remainder[REMAINDER_LEN-1] & remMask;
		

		//First block
		shiftRight(&blocks[blockIndexS].runend, BIT_I(s), 63 , 1);
		uint64_t carryRem = shiftRemainder(blocks[blockIndexS] , BIT_I(s) , 63, remMask);

		//In between blocks
		for (uint64_t i = blockIndexS +1; i < blockIndexN; i++) {
			//Get carrys
			bool carry1 = blocks[i].runend & 1;
			uint64_t carryRem1 = blocks[i].remainder[REMAINDER_LEN-1] & remMask;

			//Shift
			blocks[i].runend = blocks[i].runend >> 1;
			shiftRemainder(blocks[i] , 0 , 63 , remMask);

			//Set carrys
			if (carry) {
				setBit(blocks[i].runend, 0);
			}
			blocks[i].remainder[0] |= carryRem << (64 - REMAINDER_LEN);

			carry = carry1;
			carryRem = carryRem1;
		}
		//get carry
		if(BIT_I(n - 1) == 63){
			pushCarry = blocks[blockIndexN].runend & 1;
		}
		//Shift
		shiftRight(&blocks[blockIndexN].runend, 0, BIT_I(n - 1) , 1);
		pushCarryRem = shiftRemainder(blocks[blockIndexN] , 0 , BIT_I(n - 1) , remMask);
		
		//Set Carry
		if (carry) {
			setBit(blocks[blockIndexN].runend, 0);
		}
		blocks[blockIndexN].remainder[0] |= carryRem << (64 - REMAINDER_LEN);
		//TODO: set carry in next block if necessary
	}
	//Push carry in next block
	if(BIT_I(n - 1) == 63){
		setRemainder(blocks[blockIndexN+1] , 0 , pushCarryRem);
		if(pushCarry){
			setBit(blocks[blockIndexN+1].runend,0);
		}
	}
	return true;
}


//Iserts an Element into the filter
void RSQF::insert(Element e) {
	uint64_t h = e.hash1() & hashMask;
	uint64_t h0 = h >> remainder;
	uint64_t rem = h & remMask;

	uint64_t blockIndex = BLOCK_I(h0);
	uint8_t bitIndex = BIT_I(h0);

	uint64_t s = rankSelect(h0);

	
	if(h0 % 64 == 0 && !blocks[blockIndex].used){
		//Check if this is the first bit in the Block and it is not used already
		setRemainder(blocks[BLOCK_I(h0 + blocks[blockIndex].offset)], BIT_I(h0 + blocks[blockIndex].offset)  , rem);
		blocks[blockIndex].used = 1;
	}else if (h0 > s) {
		//Home slot is free
		setRemainder(blocks[blockIndex], bitIndex  , rem);
		setBit(blocks[blockIndex].runend, bitIndex);
	} else {
		s++;
		
		//Shifting
		//Quit if shift did not work
		if(!shiftRunendAndRemaindersByOne(s)){
			return;
		}

		//Unset old runend bit (if there was one)
		if (isBit(blocks[blockIndex].occupied, bitIndex)) {
			unSetBit(blocks[BLOCK_I(s-1)].runend, BIT_I(s-1));
		}

		//Update offset if element was added to homeslot with bit index 0
		if (BIT_I(h0)== 0) {
			if(blocks[blockIndex].offset >= 127) 
				ERROR(OFFSET_TO_SMALL);
			blocks[blockIndex].offset++;
		}

		uint64_t BIs1 = BLOCK_I(s);
		uint64_t BITIs1 = BIT_I(s);
		//Set remainder in slot s
		setRemainder(blocks[BIs1], BITIs1  , rem);
		//reset runend bit in slot s
		setBit(blocks[BIs1].runend, BITIs1);

	}
	setBit(blocks[blockIndex].occupied, bitIndex);
}

uint64_t RSQF::query(Element e) {
	uint64_t h = e.hash1() & hashMask;
	uint64_t h0 = h >> remainder;
	uint64_t r = h & remMask;

	if (!isBit(blocks[BLOCK_I(h0)].occupied, BIT_I(h0))|| (h0 % 64 == 0 && !blocks[BLOCK_I(h0)].used ) ) {
		return false;
	}
	uint64_t l = rankSelect(h0);
	do {
		if (r == getRemainder(blocks[BLOCK_I(l)] , BIT_I(l)  , remMask)) {
			return true;
		}

		l--;
	} while (l >= h0 && !isBit(blocks[BLOCK_I(l)].runend, BIT_I(l)));

	return false;

}


}
