#include "Test.h"

#include<cstring>

void doSpeedTest(double falsePositiveRate ,long long n){

      
   
    NEW_TEST;
    std::vector<Element> insert(n);
    std::vector<Element> lookups(n);
    std::vector<int> count(n);
    createDistinctTestData(n , insert , lookups);

    //RSQF SPEED TEST:
    NEW_TEST;
    print("RSQF:"); NEW_LINE;
    print("Remainder length:\t" << REMAINDER_LEN); NEW_LINE;
    NEW_LINE;
    speedTest<Filter::RSQF>(falsePositiveRate, n, insert , insert , lookups , count);


    //Bloomfilter SPEED TEST:
    NEW_TEST;
    print("Bloomfilter:"); NEW_LINE;
    NEW_LINE;
    speedTest<Filter::BloomFilter>(falsePositiveRate, n, insert  , insert, lookups , count);

    //CQF SPEED TEST:
    NEW_TEST;
    print("CQF:"); NEW_LINE;
    print("Remainder length:\t" << REMAINDER_LEN); NEW_LINE;
    speedTest<CountingFilter::CQF>(falsePositiveRate, n, insert  , insert , lookups , count);

    //CountingBloomfilter SPEED TEST:
    NEW_TEST;
    print("CountingBloomfilter:"); NEW_LINE;
    speedTest<CountingFilter::CountingBloomFilter>(falsePositiveRate, n, insert  , insert , lookups , count);
}

template<class T,typename = std::enable_if<std::is_base_of<AQM, T>::value>>
void doSingleSpeedTest(double falsePositiveRate, int n){
    NEW_TEST;
    std::vector<Element> insert(n);
    std::vector<Element> lookups(n);
    std::vector<int> count(n);
    createDistinctTestData(n , insert , lookups);

    NEW_TEST;
    NEW_LINE;
    speedTest<T>(falsePositiveRate, n, insert , insert , lookups , count);
}

void doCountingSpeedTest(double falsePositiveRate ,long long n , int maxCount){

    NEW_TEST;
        print("False positive rate:\t" << falsePositiveRate); NEW_LINE;
        print("Element count:\t\t" << n); NEW_LINE;
    NEW_TEST;
    NEW_TEST;
    std::vector<Element> insert(n);
    std::vector<int> count(n);
    std::vector<Element> lookups(n);
    createCountingTestData( n , insert, lookups , count , maxCount);

    //CQF SPEED TEST:
    NEW_TEST;
    print("CQF:"); NEW_LINE;
    print("Remainder length:\t" << REMAINDER_LEN); NEW_LINE;
    speedTest<CountingFilter::CQF>(falsePositiveRate, n/(maxCount/2), insert , insert , lookups , count);

    //CountingBloomfilter SPEED TEST:
    NEW_TEST;
    print("CountingBloomfilter:"); NEW_LINE;
    speedTest<CountingFilter::CountingBloomFilter>(falsePositiveRate, n/(maxCount/2), insert , insert , lookups , count);   
}

void doSingleSpeedTest(char * c , double falsePositiveRate ,long long n){
    if(!strcmp(c , "rsqf")){
        doSingleSpeedTest<Filter::RSQF>(falsePositiveRate ,n );
    }else if(!strcmp(c , "cqf")){
        doSingleSpeedTest<CountingFilter::CQF>(falsePositiveRate ,n );
    }else if(!strcmp(c , "bf")){
        doSingleSpeedTest<Filter::BloomFilter>(falsePositiveRate ,n );
    }else if(!strcmp(c , "cbf")){
        doSingleSpeedTest<CountingFilter::CountingBloomFilter>(falsePositiveRate ,n );
    }else if(!strcmp(c , "nbrsqf")){
        doSingleSpeedTest<Filter::RSQF_WITHOUT_BLOCKING>(falsePositiveRate ,n );
    }else if(!strcmp(c, "norsqf")){
        doSingleSpeedTest<Filter::RSQF_WITHOUT_OFFSETS>(falsePositiveRate ,n );
    }else{
        NEW_LINE;
        print("No match!"); NEW_LINE; 
        print("For singel speed test use -rsqf -cqf -bf -cbf"); NEW_LINE; 
        print("For non counting filter use -nbrsqf to use no blocking"); NEW_LINE; 
        print("For non counting filter use -norsqf to use no blocking and no offsets (this option might be very slow!)"); NEW_LINE;  
    }
}



int main(int argc, char* argv[]){

    double falsePositiveRate = 0.0001;
    long long n = 10000000;
    int maxCount = 1;
    if(argc == 2){
        if(argv[1][0] == '-' && argv[1][1] == '-'){
            if(!strcmp(&argv[1][2] , "help")){
                
                print("This is the speedtest for the (Counting-)Rank-and-Select-Based-Quotientfilter and the (Counting-)Bloomfilter"); NEW_LINE; 
                NEW_LINE; 
                print("arguments filters are:\t\t\tfalse_positive_rate  element_count"); NEW_LINE; 
                print("arguments for counting filters are:\tfalse_positive_rate  element_count  max_count"); NEW_LINE; 
                NEW_LINE;
                print("element_count: amount of elements that should be inserted"); NEW_LINE; 
                print("max_count: maximum occurrence count of one element"); NEW_LINE; 

                NEW_LINE;
                print("For singel speed test use -rsqf -cqf -bf -cbf"); NEW_LINE; 
                print("For non counting filter use -nbrsqf to use no blocking"); NEW_LINE; 
                print("For non counting filter use -norsqf to use no blocking and no offsets (this option might be very slow!)"); NEW_LINE; 
            }
            NEW_LINE;
            return 0;
        }else if(argv[1][0] == '-'){
            doSingleSpeedTest(&argv[1][1] , falsePositiveRate , n);
            return 0;
        }
    }else if (argc <= 3  ){
        if(argc == 3){
            falsePositiveRate = strtod(argv[1] , NULL);
            n = atoll(argv[2]);
        }else{
            print("Default values have been choosen!"); NEW_LINE;
        }
        NEW_TEST;
        print("False positive rate:\t" << falsePositiveRate); NEW_LINE;
        print("Element count:\t\t" << n); NEW_LINE;
        NEW_TEST;
        print("Filter test:");NEW_LINE;
        doSpeedTest(falsePositiveRate , n);
        return 0;
    }else if(argc == 4){
        if(argv[1][0] == '-'){
            falsePositiveRate = strtod(argv[2] , NULL); 
            n = atoll(argv[3]);
            doSingleSpeedTest(&argv[1][1] , falsePositiveRate , n);
            return 0;
        }else{
            falsePositiveRate = strtod(argv[1] , NULL); 
            n = atoll(argv[2]);
            maxCount = atoi(argv[3]);
            NEW_TEST;
            print("False positive rate:\t" << falsePositiveRate); NEW_LINE;
            print("Element count:\t\t" << n); NEW_LINE;
            NEW_TEST;
            print("Counting test:");NEW_LINE;
            doCountingSpeedTest(falsePositiveRate , n , maxCount);
            return 0;
        }
    }
    
    
    
}

