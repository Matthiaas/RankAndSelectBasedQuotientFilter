#include "Test.h"




//Creates duplicate free data with high probability
void createDistinctTestData(int n, std::vector<Element> &insert , std::vector<Element> &lookUps){
    print("Creating testdata: \t"); 
    for(int i = 0; i < n ; i++){
	    long long in =((long long)rand() << 32) | rand();
	    Element e(in);
        insert[i]= e;
        #if ENABLE_WAITING_PRINTS
        if(!(i % (n/5))) {
            print("."); 
        }
        #endif
    }
    for(int i = 0; i < n ; i++){
	    long long in =((long long)rand() << 32) | rand();
	    Element e(in);
        lookUps[i]= e;
        #if ENABLE_WAITING_PRINTS
        if(!(i % (n/5))) {
            print("."); 
        }
        #endif
    }

    print(" done"); NEW_LINE;
}


void createCountingTestData(int n,  std::vector<Element> &insert  , std::vector<Element> &lookUps, std::vector<int> &counts , int maxCount){
    print("Creating testdata: \t"); 
    int distinct = 0;
    int c = 0;
    int i;
    for(i = 0; c < n; i++){

       counts[i] =  (rand() % maxCount) % (n-c);
       c+= counts[i] +1;
       
        long long in =((long long)rand() << 32) | rand();
        Element e(in);
        insert[i]= e;

        #if ENABLE_WAITING_PRINTS
        if(!(i % (n/5))) {
            print("."); 
        }
        #endif
    }
    
    insert.resize(i);
    counts.resize(i);

    for(i = 0; i < n ; i++){
	    long long in =((long long)rand() << 32) | rand();
	    Element e(in);
        lookUps[i]= e;
        #if ENABLE_WAITING_PRINTS
        if(!(i % (n/5))) {
            print("."); 
        }
        #endif
    }
    print(" done"); NEW_LINE;

}




