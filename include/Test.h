#ifndef TEST_H
#define TEST_H


#include <iostream>
#include <vector>
#include <list>
#include <algorithm>

#include "AQM.h"
#include "RSQF.h"
#include "CQF.h"

#include "RSQF_WITHOUT_BLOCKING.h"
#include "RSQF_WITHOUT_OFFSETS.h"

#include "BloomFilter.h"
#include "CountingBloomFilter.h"




// If true this may increas runtime!
#define ENABLE_WAITING_PRINTS true

#define print(x) std::cout<<x<<std::flush
#define NEW_LINE std::cout<<std::endl
#define NEW_TEST std::cout<<"-----------------------------------------------"<<std::endl



void createDistinctTestData(int n, std::vector<Element> &insert , std::vector<Element> &lookUps);
void createCountingTestData(int n, std::vector<Element> &insert , std::vector<Element> &lookUps , std::vector<int> &count , int maxCount);
//Can not be defined in another sourec file
//Make sure T is an AMQ
template<class T,typename = std::enable_if<std::is_base_of<AQM, T>::value>>
int speedTest(double falsePositiveRate , int n , std::vector<Element> &insert 
        , std::vector<Element> &lookUps , std::vector<Element> &randomlookUps ,std::vector<int> &count ){

    

    print("inti: \t\t\t" );
    clock_t start = clock();

    T qf(falsePositiveRate , n );

    clock_t end = clock();
    double time = (double) (end-start) / CLOCKS_PER_SEC ;
	print( time << "s" );
    
    NEW_LINE;


   

    //Random Inserts:
    print("Random inserts: \t");

    start = clock();


    for(int i = 0; i < insert.size(); i++){
        for(int j = 0; j <= count[i]; j++)
            qf.insert(insert[i]);
        #if ENABLE_WAITING_PRINTS
        if(!(i % ( insert.size()/10))) {
            print("."); 
        }
        #endif

    }
	

    end = clock();
    time = (double) (end-start) / CLOCKS_PER_SEC ;
    print(" " << time << "s" );

    NEW_LINE;
    

    //Lookups:
    print("Lookups:\t\t");
    start = clock();

    int res = 0;
    for(int i = 0; i < lookUps.size() ; i++){
        res+= qf.query(lookUps[i]);
        #if ENABLE_WAITING_PRINTS
        if(!(i % (lookUps.size()/10))) {
            print("."); 
        }
        #endif
    }
	 

    end = clock();
    time = (double) (end-start) / CLOCKS_PER_SEC ;
    print(" " << time << "s"  << "\t \t" << res << " elements found (should be:" << n << ")");

    NEW_LINE;   

    //Random lookups:
    print("Random lookups: \t");
    start = clock();

    res = 0;
    for(int i = 0; i < randomlookUps.size() ; i++){
        qf.query(randomlookUps[i]);
        #if ENABLE_WAITING_PRINTS
        if(!(i % (randomlookUps.size()/10))) {
            print("."); 
        }
        #endif
    }
	

    end = clock();
    time = (double) (end-start) / CLOCKS_PER_SEC ;

    print(" " << time << "s" );

    NEW_LINE;
    NEW_LINE;

}


#endif