// CMSC 341 - Fall 2022 - Project 4
#include "cache.h"
#include <random>
#include <vector>
const int MINSEARCH = 0;
const int MAXSEARCH = 7;
// the following array defines sample search strings for testing
string searchStr[MAXSEARCH+1]={"c++","python","java","scheme","prolog","c#","c","js"};
enum RANDOM {UNIFORMINT, UNIFORMREAL, NORMAL};
const string SUCCESS = "Test passed!\n";
const string FAIL = "Test failed.\n";
class Random {
public:
    Random(int min, int max, RANDOM type=UNIFORMINT, int mean=50, int stdev=20) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL){
            //the case of NORMAL to generate integer numbers with normal distribution
            m_generator = std::mt19937(m_device());
            //the data set will have the mean of 50 (default) and standard deviation of 20 (default)
            //the mean and standard deviation can change by passing new values to constructor 
            m_normdist = std::normal_distribution<>(mean,stdev);
        }
        else if (type == UNIFORMINT) {
            //the case of UNIFORMINT to generate integer numbers
            // Using a fixed seed value generates always the same sequence
            // of pseudorandom numbers, e.g. reproducing scientific experiments
            // here it helps us with testing since the same sequence repeats
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_unidist = std::uniform_int_distribution<>(min,max);
        }
        else{ //the case of UNIFORMREAL to generate real numbers
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_uniReal = std::uniform_real_distribution<double>((double)min,(double)max);
        }
    }
    void setSeed(int seedNum){
        // we have set a default value for seed in constructor
        // we can change the seed by calling this function after constructor call
        // this gives us more randomness
        m_generator = std::mt19937(seedNum);
    }

    int getRandNum(){
        // this function returns integer numbers
        // the object must have been initialized to generate integers
        int result = 0;
        if(m_type == NORMAL){
            //returns a random number in a set with normal distribution
            //we limit random numbers by the min and max values
            result = m_min - 1;
            while(result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else if (m_type == UNIFORMINT){
            //this will generate a random number between min and max values
            result = m_unidist(m_generator);
        }
        return result;
    }

    double getRealRandNum(){
        // this function returns real numbers
        // the object must have been initialized to generate real numbers
        double result = m_uniReal(m_generator);
        // a trick to return numbers only with two deciaml points
        // for example if result is 15.0378, function returns 15.03
        // to round up we can use ceil function instead of floor
        result = std::floor(result*100.0)/100.0;
        return result;
    }
    
    private:
    int m_min;
    int m_max;
    RANDOM m_type;
    std::random_device m_device;
    std::mt19937 m_generator;
    std::normal_distribution<> m_normdist;//normal distribution
    std::uniform_int_distribution<> m_unidist;//integer uniform distribution
    std::uniform_real_distribution<double> m_uniReal;//real uniform distribution

};

unsigned int hashCode(const string str) {
   unsigned int val = 0 ;
   const unsigned int thirtyThree = 33 ;  // magic number from textbook
   for ( unsigned int i = 0 ; i < str.length(); i++)
      val = val * thirtyThree + str[i] ;
   return val ;
}

class Tester{

public:

    bool insertTest1() {
        //tests whether insert functions correctly when the inserting multiple colliding keys

        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MAXPRIME, hashCode);
        vector <Person> personVector;

        //inserts 3000 data points and checks at each point that m_currentSize is correct
        int peopleInCache = 0;
        for(int i = 0; i < 3000; i++) {
            Person person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
            if(cache.insert(person)) {
                peopleInCache++;
                personVector.push_back(person);
                if(cache.m_currentSize != peopleInCache) {
                    return false;
                }
            }
        }

        //checks whether each person was correctly inserted into the correct bucket
        int correctIndexFound = false;
        int i = 0;
        while(personVector.size() > 0) {
            if(cache.getPerson(personVector.back().m_key, personVector.back().m_id) == EMPTY) {
                return false;
            }else {
                correctIndexFound = false;
                i = 0;
                while(i < cache.m_currentSize && correctIndexFound == false) {
                    if(cache.m_currentTable[((hashCode(personVector.back().m_key) % cache.m_currentCap) + (i * i)) % cache.m_currentCap] == personVector.back()) {
                        correctIndexFound = true;
                        personVector.pop_back();
                    }
                    i++;
                }
                if(correctIndexFound == false) {
                    return false;
                }
            }
        }

        //inserts 3000 more data points and checks at each point that m_currentSize is correct
        for(int i = 0; i < 3000; i++) {
            Person person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
            if(cache.insert(person)) {
                peopleInCache++;
                personVector.push_back(person);
                if(cache.m_currentSize != peopleInCache) {
                    return false;
                }
            }
        }

        //checks whether each person was correctly inserted into the correct bucket
        correctIndexFound = false;
        i = 0;
        while(personVector.size() > 0) {
            if(cache.getPerson(personVector.back().m_key, personVector.back().m_id) == EMPTY) {
                return false;
            }else {
                correctIndexFound = false;
                i = 0;
                while(i < cache.m_currentSize && correctIndexFound == false) {
                    if(cache.m_currentTable[((hashCode(personVector.back().m_key) % cache.m_currentCap) + (i * i)) % cache.m_currentCap] == personVector.back()) {
                        correctIndexFound = true;
                        personVector.pop_back();
                    }
                    i++;
                }
                if(correctIndexFound == false) {
                    return false;
                }
            }
        }

        return true;
    }

    bool insertTest2() {
        //tests whether insert functions correctly when rehashing is triggered
        
        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MINPRIME, hashCode);
        vector <Person> personVector;

        //inserts 50 data points so that the load factor is 50 / 101 = 0.495
        int peopleInCache = 0;
        while (peopleInCache != 50) {
            Person person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
            if(cache.insert(person)) {
                peopleInCache++;
                personVector.push_back(person);
            }
        }

        //inserts 1 more data point so that the load factor is 51 / 101 > 0.5 and rehashing is triggered
        bool inserted = false;
        while(!inserted) {
           Person person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
           if(cache.insert(person)) {
            inserted = true;
            peopleInCache++;
            personVector.push_back(person);
           }
        }

        //checks that rehash was triggered by checking that m_oldTable was created with the old values of m_currentTable and that 25 data points, floor(m_oldSize / 4), were moved
        //to m_currentTable
        if(cache.m_oldTable == nullptr || cache.m_oldCap != MINPRIME || cache.m_oldSize != 51 || cache.m_oldNumDeleted != 12 || cache.m_currentCap != 211 || \
        cache.m_currentSize != 12 || cache.m_currNumDeleted != 0) {
            return false;
        }

        //checks that all the people inserted still exist in m_currentTable or m_oldTable and that their spot complies with the hash function
        while(personVector.size() > 0) {
            if(cache.getPerson(personVector.back().m_key, personVector.back().m_id).m_key == "") {
                return false;
            }else {
                int i = 0;
                int index1 = ((hashCode(personVector.back().m_key) % cache.m_currentCap) + (i * i)) % cache.m_currentCap;
                int index2 = ((hashCode(personVector.back().m_key) % cache.m_oldCap) + (i * i)) % cache.m_oldCap;
                while( i < cache.m_currentCap && !(cache.m_currentTable[index1] == personVector.back()) && !(cache.m_oldTable[index2] == personVector.back())) {
                    i++;
                    index1 = ((hashCode(personVector.back().m_key) % cache.m_currentCap) + (i * i)) % cache.m_currentCap;
                    index2 = ((hashCode(personVector.back().m_key) % cache.m_oldCap) + (i * i)) % cache.m_oldCap;
                }
                if(cache.m_currentTable[index1] == personVector.back() || cache.m_oldTable[index2] == personVector.back()) {
                    personVector.pop_back();
                }else {
                    return false;
                }
            }
        }

        return true;
    }

    bool insertTest3() {
        //tests whether insert correctly transfers nodes from m_oldTable to m_currentTable during the rehashing process

        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MINPRIME, hashCode);
        vector <Person> personVector;

        //inserts 50 data points so that the load factor is 50 / 101 = 0.495
        int peopleInCache = 0;
        while (peopleInCache != 50) {
            Person person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
            if(cache.insert(person)) {
                peopleInCache++;
                personVector.push_back(person);
            }
        }

        //inserts 1 more data point so that the load factor is 51 / 101 > 0.5 and rehashing is triggered
        bool inserted = false;
        while(!inserted) {
           Person person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
           if(cache.insert(person)) {
            inserted = true;
            personVector.push_back(person);
           }
        }

        //checks that rehash was triggered by checking that m_oldTable was created with the old values of m_currentTable and that 25 data points, floor(m_oldCap / 4), were moved
        //to m_currentTable and its new cap is correct
        if(cache.m_oldTable == nullptr || cache.m_oldCap != MINPRIME || cache.m_oldSize != 51 || cache.m_oldNumDeleted != 12 || cache.m_currentCap != 211 || \
        cache.m_currentSize != 12 || cache.m_currNumDeleted != 0) {
            return false;
        }

        //inserts another node to trigger rehash again
        inserted = false;
        while(!inserted) {
           Person person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
           if(cache.insert(person)) {
            inserted = true;
            personVector.push_back(person);
           }
        }

        //checks 12 more nodes were moved over and that the last node was inserted into m_currentTable
        if(cache.m_oldTable == nullptr || cache.m_oldCap != MINPRIME || cache.m_oldSize != 51 || cache.m_oldNumDeleted != 24 || cache.m_currentCap != 211 || \
        cache.m_currentSize != 25 || cache.m_currNumDeleted != 0) {
            return false;
        }

         //inserts another node to trigger rehash again
        inserted = false;
        while(!inserted) {
           Person person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
           if(cache.insert(person)) {
            inserted = true;
            personVector.push_back(person);
           }
        }

        //checks that 12 more nodes were moved to m_currentTable
        if(cache.m_oldTable == nullptr || cache.m_oldCap != MINPRIME || cache.m_oldSize != 51 || cache.m_oldNumDeleted != 36 || cache.m_currentCap != 211 || cache.m_currentSize != 38 || \
        cache.m_currNumDeleted != 0) {
            return false;
        }

        //inserts another node to trigger rehash again
        inserted = false;
        while(!inserted) {
           Person person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
           if(cache.insert(person)) {
            inserted = true;
            personVector.push_back(person);
           }
        }

        //checks that 12 more nodes were moved over
        if(cache.m_oldTable == nullptr || cache.m_oldCap != MINPRIME || cache.m_oldSize != 51 || cache.m_oldNumDeleted != 48 || cache.m_currentCap != 211 || cache.m_currentSize != 51 || \
        cache.m_currNumDeleted != 0) {
            return false;
        }

        //inserts another node to trigger rehash again
        inserted = false;
        while(!inserted) {
           Person person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
           if(cache.insert(person)) {
            inserted = true;
            personVector.push_back(person);
           }
        }

        //checks that the last 3 nodes were moved over and m_oldTable values were cleared
        if(cache.m_oldTable != nullptr || cache.m_oldCap != 0 || cache.m_oldSize != 0 || cache.m_oldNumDeleted != 0 || cache.m_currentCap != 211 || cache.m_currentSize != 55 || \
        cache.m_currNumDeleted != 0) {
            return false;
        }else {
            while(personVector.size() > 0) {
                int i = 0;
                int index = ((hashCode(personVector.back().m_key) % cache.m_currentCap) + (i * i)) % cache.m_currentCap;
                while(i < cache.m_currentCap && !(cache.m_currentTable[index] == personVector.back())) {
                    i++;
                    index = ((hashCode(personVector.back().m_key) % cache.m_currentCap) + (i * i)) % cache.m_currentCap;
                }
                if(cache.m_currentTable[index] == personVector.back()) {
                    personVector.pop_back();
                }else {
                    return false;
                }
            }
        }
        return true;
    }

    bool removeTest1() {
        //tests whether remove functions correctly when removing a few data points in m_currentTable

        Cache cache(MAXPRIME, hashCode);
        vector <Person> personVector;
        vector <Person> peopleRemoved;

        //inserts 3000 data points and deletes 2000 of them
        for (int i = 0; i < 3000; i++) {
            Person person(searchStr[i % 8], MINID + i);
            if(cache.insert(person)) {
                if(i >= 2000) {
                    personVector.push_back(person);
                }
            }
        }

        for (int i = 0; i < 2000; i++) {
            Person person(searchStr[i % 8], MINID + i);
            cache.remove(person);
            peopleRemoved.push_back(person);
        }

        //checks that all the people inserted and not deleted can still be found   
        while (personVector.size() > 0) {
            if(cache.getPerson(personVector.back().m_key, personVector.back().m_id) == personVector.back()) {
                personVector.pop_back();
            }else {
                return false;
            }
        }

        //checks that all the people removed cannot be found in the cache
        while(peopleRemoved.size() > 0) {
            if(cache.getPerson(peopleRemoved.back().m_key, peopleRemoved.back().m_id) == EMPTY) {
                peopleRemoved.pop_back();
            }else {
                return false;
            }
        }

        return true;
    }

    bool removeTest2() {
        //tests whether remove functions correctly during the rehashing process

        Cache cache(107, hashCode);
        vector <Person> personVector;
        vector <Person> peopleRemoved;

        //populates cache with 53 nodes then deletes 43 of them to trigger rehashing (43 / 53 > 0.8)
        for(int i = 0; i < 53; i++) {
            Person person(searchStr[i % 8], MINID + i);
            cache.insert(person);
            if(i >= 43) {
                personVector.push_back(person);
            }
        }

        for(int i = 0; i < 43; i++) {
            Person person(searchStr[i % 8], MINID + i);
            cache.remove(person);
            peopleRemoved.push_back(person);
        }

        //checks that all of the data points were rehashed at once
        if(cache.m_oldTable != nullptr || cache.m_currentCap != MINPRIME || cache.m_currentSize != 10 || cache.m_currNumDeleted != 0) {
            return false;
        }

        //checks that all of the remaining data was transferred to the correct buckets and there are no buckets tagged as deleted
        while(personVector.size() > 0) {
            if(cache.getPerson(personVector.back().m_key, personVector.back().m_id) == personVector.back()) {
                int i = 0;
                int index = ((hashCode(personVector.back().m_key) % cache.m_currentCap) + (i * i)) % cache.m_currentCap;
                while(i < cache.m_currentCap && !(cache.m_currentTable[index] == personVector.back())) {
                    i++;
                    index = ((hashCode(personVector.back().m_key) % cache.m_currentCap) + (i * i)) % cache.m_currentCap;
                }
                if(cache.m_currentTable[index] == personVector.back()) {
                    personVector.pop_back();
                }else {
                    return false;
                }
            }else {
                return false;
            }
        }
        while(peopleRemoved.size() > 0) {
            if(cache.getPerson(peopleRemoved.back().m_key, peopleRemoved.back().m_id) == EMPTY) {
                peopleRemoved.pop_back();
            }else {
                return false;
            }
        }
        return true;
    }

        bool removeTest3() {
        //tests whether remove functions correctly when removing a data point from m_oldTable during the rehashing process
        
        Cache cache(MINPRIME, hashCode);
        vector <Person> personVector;

        //populates cache with 51 / 101 nodes so that rehashing is triggered, m_currentTable now has 12 / 211 nodes and m_oldTable had 39 / 101 nodes
        for(int i = 0; i < 51; i++) {
            Person person(searchStr[i % 8], MINID + i);
            cache.insert(person);
            personVector.push_back(person);
        }

        //creates the last person in m_oldTable to be removed
        Person person("c", 1014);

        //removes the last person in m_oldTable, meaning that rehashing is now done
        cache.remove(person);
        if(cache.m_oldTable != nullptr && cache.m_oldCap == MINPRIME && cache.m_oldSize == 51 && cache.m_oldNumDeleted == 25) {
            if(cache.getPerson(person.m_key, person.m_id) == EMPTY) {
                return true;
            }else {
                return false;
            }
        }else {
            return false;
        }
    }

    bool getPersonTest1() {
        //tests whether getPerson functions correctly when searching for non-colliding keys in m_currentTable

        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MINPRIME, hashCode);
        vector <Person> personVector;

        //inserts 8 non-colliding keys
        for(int i = 0; i < 8; i++) {
            Person person(searchStr[i], MINID + i);
            cache.insert(person);
            personVector.push_back(person);
        }

        //searchs for each non-colliding key
        for(int i = 0; i < 8; i++) {
            if(cache.getPerson(personVector.back().m_key, personVector.back().m_id) == EMPTY) {
                return false;
            }else {
                personVector.pop_back();
            }
        }
        return true;
    }

    bool getPersonTest2() {
        //tests whether getPerson functions correctly when searching for non-colliding keys in m_oldTable

        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MINPRIME, hashCode);
        vector <Person> personVector;

        //inserts 8 non-colliding keys and checks at each step that the person inserted can be found
        for(int i = 0; i < 50; i++) {
            Person person(searchStr[i % 8], MINID + i);
            cache.insert(person);
        }

        //gets the last 25 data points that will be in m_oldTable when rehashing starts 
        int dataPointsInserted = 0;
        int i = MINPRIME - 1;
        while (dataPointsInserted < 25) {
            if(!(cache.m_currentTable[i] == EMPTY)) {
                personVector.push_back(cache.m_currentTable[i]);
                dataPointsInserted++;
            }
            i--;
        }
        
        //inserts 1 more person so that rehashing starts
        Person person("c", MINID + 50);
        cache.insert(person);

        //calls getPerson on 25 data points in m_oldTable to see whether getPerson can find them
        while(personVector.size() > 0) {
            if(cache.getPerson(personVector.back().m_key, personVector.back().m_id) == personVector.back()) {
                personVector.pop_back();
            }else {
                return false;
            }
        }

        return true;
    }

    bool getPersonTest3() {
        //tests whether getPerson functions correctly when searching for a number of colliding keys in m_currentTable

        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MAXPRIME, hashCode);
        vector <Person> personVector;

        //inserts 3000 colliding keys into cache
        for(int i = 0; i < 3000; i++) {
            Person person("c", RndID.getRandNum());
            cache.insert(person);
            personVector.push_back(person);
        }

        //checks that each of the 3000 colliding nodes inserted can be found with getPerson
        while(personVector.size() > 0) {
            if(cache.getPerson(personVector.back().m_key, personVector.back().m_id) == personVector.back()) {
                personVector.pop_back();
            }else {
                return false;
            }
        }

        return true;
    }

    bool getPersonTest4() {
        //tests whether getPerson functions correctly when searching for a number of colliding keys in m_oldTable
        
        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MINPRIME, hashCode);
        vector <Person> personVector;

        //inserts 50 colliding keys into cache
        for(int i = 0; i < 50; i++) {
            Person person("c", MINID + i);
            cache.insert(person);
            personVector.push_back(person);
        }

        int dataPointsInserted = 0;
        int i = MINPRIME - 1;
        while(dataPointsInserted < 25) {
            if(!(cache.m_currentTable[i] == EMPTY)) {
                personVector.push_back(cache.m_currentTable[i]);
                dataPointsInserted++;
            }
            i--;
        }

        Person person("c", MINID + 50);
        cache.insert(person);

        while(personVector.size() > 0) {
            if(cache.getPerson(personVector.back().m_key, personVector.back().m_id) == personVector.back()) {
                personVector.pop_back();
            }else {
                return false;
            }
        }

        return true;
    }

    bool getPersonTest5() {
        //tests whether getPerson functions correctly when in the process of rehashing without triggering a rehash

        Random RndID(MINID,MAXID);
        Cache cache(MINPRIME, hashCode);

        //inserts 50 colliding keys and checks at each step that the person inserted can be found
        int peopleInserted = 0;
        while(peopleInserted != 50) {
            Person person("c", RndID.getRandNum());
            if(cache.insert(person)) {
                peopleInserted++;
                if(!(cache.getPerson(person.m_key, person.m_id) == person)) {
                    return false;
                }
            }
        }  

        //inserts 1 more colliding key to trigger rehash, then calls getPerson and checks whether doing so triggered a rehash
        bool inserted = false;
        while(!inserted) {
            Person person("c", RndID.getRandNum());
            if(cache.insert(person)) {
                inserted = true;
                if(cache.getPerson(person.m_key, person.m_id) == person) {
                    if(cache.m_oldNumDeleted == 12 && cache.m_currentSize == 12) {
                        return true;
                    }else {
                        return false;
                    }
                }else {
                    return false;
                }
            }
        }
        return false;      
    }

    bool getPersonTest6() {
        //tests whether getPerson functions correctly when searching for a person that does not exist in the database
        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MINPRIME, hashCode);
        Person person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
        if(cache.getPerson(person.m_key, person.m_id).m_key == "" && cache.getPerson(person.m_key, person.m_id).m_id == 0) {
            return true;
        }else {
            return false;
        }
    }

    bool lambdaTest1() {
        //tests whether lamda functions correctly when Cache is only populated with availible nodes

        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MAXPRIME, hashCode);

        //inserts 3000 data points
        int peopleInCache = 0;
        for(int i = 0; i < 3000; i++) {
            Person person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
            if(cache.insert(person)) {
                peopleInCache++;
            }
        }

        if(cache.lambda() == float(peopleInCache) / MAXPRIME) {
            return true;
        }else {
            return false;
        }
    }

    bool lambdaTest2() {
        //tests whether lambda functions correctly when Cache is populated with availible nodes and deleted nodes
        
        Cache cache(MAXPRIME, hashCode);

        //inserts 3000 data points then deletes 300 of them
        int peopleInCache = 0;
        for(int i = 0; i < 3000; i++) {
            Person person(searchStr[i % 8], MINID + i);
            if(cache.insert(person)) {
                peopleInCache++;
            }
        }

        for(int i = 0; i < 300; i++) {
            Person person(searchStr[i % 8], MINID + i);
            cache.remove(person);
        }

        if(cache.lambda() == float(peopleInCache) / MAXPRIME) {
            return true;
        }else {
            return false;
        }
    }


    bool lambdaTest3() {
        //tests whether lambda functions correctly when Cache is empty

        Cache cache(MINPRIME, hashCode);
        if(cache.lambda() == 0) {
            return true;
        }else {
            return false;
        }
    }

    bool deletedRatioTest1() {
        //tests whether deletedRatio functions correctly when Cache is populated with availible and deleted nodes
        
        Cache cache(MAXPRIME, hashCode);

        //inserts 3000 data points then deletes 2000 of them
        int peopleInCache = 0;
        int peopleDeleted = 0;
        for(int i = 0; i < 3000; i++) {
            Person person(searchStr[i % 8], MINID + i);
            if(cache.insert(person)) {
                peopleInCache++;
            }
        }

        for(int i = 0; i < 2000; i++) {
            Person person(searchStr[i % 8], MINID + i);
            cache.remove(person);
            peopleDeleted++;
        }

        if(cache.deletedRatio() == (float(peopleDeleted) / peopleInCache)) {
            return true;
        }else {
            return false;
        }
    }

    bool deletedRatioTest2() {
        //tests whether deletedRatio functions correctly when Cache is only populated with availible nodes
        
        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MAXPRIME, hashCode);

        //inserts 3000 data points
        int peopleInCache = 0;
        for(int i = 0; i < 3000; i++) {
            Person person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
            if(cache.insert(person)) {
                peopleInCache++;
            }
        }

        if(cache.deletedRatio() == 0) {
            return true;
        }else {
            return false;
        }
    }

    bool deletedRatioTest3() {
        //tests whether deletedRatio functions correctly when Cache is empty
        
        Cache cache(MINPRIME, hashCode);
        if(cache.deletedRatio() == 0) {
            return true;
        }else {
            return false;
        }
    }

};

int main(){
    
    Tester test;

    cout << "\nINSERT TEST\n";

    cout << "Testing whether insert functions correctly when the inserting multiple colliding and non-colliding keys...";
    if(test.insertTest1()) {
        cout << SUCCESS;
    }else {
        cout << FAIL;
    }

    cout << "Testing whether insert functions correctly when rehashing is triggered...";
    if(test.insertTest2()) {
        cout << SUCCESS;
    }else {
        cout << FAIL;
    }

    cout << "Testing whether insert correctly transfers nodes from m_oldTable to m_currentTable during the rehashing process...";
    if(test.insertTest3()) {
        cout << SUCCESS;
    }else {
        cout << FAIL;
    }

    cout << "\nREMOVE TEST\n";

    cout << "Testing whether remove functions correctly when removing many colliding and non-colliding keys from m_currentTable...";
    if(test.removeTest1()) {
        cout << SUCCESS;
    }else {
        cout << FAIL;
    }

    cout << "Testing whether remove functions correctly during the rehashing process...";
    if(test.removeTest2()) {
        cout << SUCCESS;
    }else {
        cout << FAIL;
    }

    cout << "Testing whether remove functions correctly when removing a data point in m_oldTable during the rehashing process...";
    if(test.removeTest3()) {
        cout << SUCCESS;
    }else {
        cout << FAIL;
    }

    cout << "\nGETPERSON TEST\n";

    cout << "Testing whether getPerson functions correctly when searching for non-colliding keys in m_currentTable...";
    if(test.getPersonTest1()) {
        cout << SUCCESS;
    }else {
        cout << FAIL;
    }

    cout << "Testing whether getPerson functions correctly when searching for non-colliding keys in m_oldTable...";
    if(test.getPersonTest2()) {
        cout << SUCCESS;
    }else {
        cout << FAIL;
    }

    cout << "Testing whether getPerson functions correctly when searching for a number of colliding keys in m_currentTable...";
    if(test.getPersonTest3()) {
        cout << SUCCESS;
    }else {
        cout << FAIL;
    }

    cout << "Testing whether getPerson functions correctly when searching for a number of colliding keys in m_oldTable...";
    if(test.getPersonTest4()) {
        cout << SUCCESS;
    }else {
        cout << FAIL;
    }

    cout << "Testing whether getPerson functions correctly when in the process of rehashing without triggering a rehash...";
    if(test.getPersonTest5()) {
        cout << SUCCESS;
    }else {
        cout << FAIL;
    }

    cout << "Testing whether getPerson functions correctly when searching for a person that does not exist in the database...";
    if(test.getPersonTest6()) {
        cout << SUCCESS;
    }else {
        cout << FAIL;
    }

    cout << "\nLAMBDA TEST\n";

    cout << "Testing whether lambda functions correctly when Cache is only populated with availible nodes...";
    if(test.lambdaTest1()) {
        cout << SUCCESS;
    }else{
        cout << FAIL;
    }

    cout << "Testing whether lambda functions correctly when Cache is populated with availible nodes and deleted nodes...";
    if(test.lambdaTest2()) {
        cout << SUCCESS;
    }else{
        cout << FAIL;
    }

    cout << "Testing whether lamda functions correctly when Cache is empty...";
    if(test.lambdaTest3()) {
        cout << SUCCESS;
    }else{
        cout << FAIL;
    }

    cout << "\nDELETEDRATIO TEST\n";

    cout << "Testing whether deletedRatio functions correctly when Cache is populated with availible nodes and deleted nodes...";
    if(test.deletedRatioTest1()) {
        cout << SUCCESS;
    }else{
        cout << FAIL;
    }

     cout << "Testing whether deletedRatio functions correctly when Cache is only populated with availible nodes...";
    if(test.deletedRatioTest2()) {
        cout << SUCCESS;
    }else{
        cout << FAIL;
    }

     cout << "Testing whether deletedRatio functions correctly when Cache is empty...";
    if(test.deletedRatioTest3()) {
        cout << SUCCESS;
    }else{
        cout << FAIL;
    }

    return 0;
}