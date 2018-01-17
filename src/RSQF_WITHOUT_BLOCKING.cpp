#include "RSQF_WITHOUT_BLOCKING.h"



namespace Filter{

RSQF_WITHOUT_BLOCKING::RSQF_WITHOUT_BLOCKING(double false_positive_rate, int n) {
	int p = (int) (log2(n / false_positive_rate));
	if (p > 64) {
		std::cout << "false positive rate was reduced!" << std::endl;
		p = 64;
	}
	hashMask = BIT_MASK(p);
	remMask = BIT_MASK(REMAINDER_LEN);

	quotient = p - REMAINDER_LEN;
	remainder = (p - quotient);
	uint64_t slots = ((uint64_t) 1) << (quotient);

	occupieds.resize(slots);
	runends.resize(slots);
	for(int i = 0; i < slots; i+= 64){
		occupieds[i] = 1;
		runends[i] = 1;
	}

	remainders.resize(slots * remainder);
	offset.resize(slots / 64);
	offsetUsed.resize(slots/64);

}

RSQF_WITHOUT_BLOCKING::~RSQF_WITHOUT_BLOCKING() {

}

uint64_t RSQF_WITHOUT_BLOCKING::rankSelect(uint64_t h0) {
	uint64_t i = ((h0 / 64) * 64);//Index of next lower offset	
	if (i == h0) {
		return offset[i / 64] + h0;
	} else {
		uint64_t d = rank(occupieds, i + 1, h0 - i - 1);
		if(d== 0){
			return  i + offset[i / 64];
		}else{
			uint64_t t = select(runends, i + offset[i / 64] +1, d);
			return  i + offset[i / 64] + t +1;
		}	
	}
}

uint64_t RSQF_WITHOUT_BLOCKING::find_first_unused_slot(uint64_t x) {
	uint64_t s = rankSelect(x);
	while (x <= s) {
		x = s + 1;
		s = rankSelect(x);
	}
	return x;
}

void RSQF_WITHOUT_BLOCKING::insert(Element e) {
	uint64_t h = e.hash1() & hashMask;
	uint64_t h0 = h >> remainder;


	std::vector<bool> h1(remainder);
	writeValue(h1, h & remMask, remainder);

	uint64_t s = rankSelect(h0);

	if(h0 % 64 == 0 && !offsetUsed[h0/64]){
		offsetUsed[h0/64] = 1;
		bitVectorSet(remainders, (h0  + offset[h0/64] )* remainder, h1, 0, remainder);
	}else if (h0 > s ) {
		bitVectorSet(remainders, h0 * remainder, h1, 0, remainder);
		runends[h0] = 1;
	} else {
		s = s + 1;
		uint64_t n = find_first_unused_slot(s);

		if(n >= occupieds.size()){
			std::cout <<"Insert not possible!"<< std::endl
					<<"\t --> Create bigger filter!"<<std::endl;
			return;
		}

		while (n > s) {
			//shift right
			bitVectorSet(remainders, n * remainder, remainders,(n - 1) * remainder, remainder);
			runends[n] = runends[n - 1];
			// Find offset
			n--;
			if(runends[n])
				for (int i = (n) / 64; i >= 0; i--) {
					if (offset[i] == (n - i * 64)) {
						// Update offset
						offset[i]++;
						break;
					}else if(offset[i] < (n - i * 64)){
						break;
					}
				}

		}

		bitVectorSet(remainders, s * remainder, h1, 0, remainder);
		if (occupieds[h0]) {
			runends[s - 1] = 0;
		}

		if(h0 % 64 == 0 && occupieds[h0] ){

			offset[h0/64]++;
		}
		runends[s] = 1;
	}
	occupieds[h0] = 1;

}

uint64_t RSQF_WITHOUT_BLOCKING::query(Element e) {
	uint64_t h = e.hash1() & hashMask;
	uint64_t h0 = h >> remainder; 

	if (!occupieds[h0]|| (h0 % 64 == 0 && !offsetUsed[h0/64]) ) {
		return false;
	}
	uint64_t l = rankSelect(h0);
	std::vector<bool> v(remainder);
	writeValue(v, h & remMask, remainder);
	do {
		if (bitVectorEquals(remainders, l * remainder, v, 0, remainder)) {
			return true;
		}
		l--;
	} while ((l >= h0 && !runends[l]));

	return false;

}

}