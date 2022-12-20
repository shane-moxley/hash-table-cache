// CMSC 341 - Fall 2022 - Project 4
#include "cache.h"
Cache::Cache(int size, hash_fn hash){
    
    //checks that the size is within range and prime then dynamically allocates a person array
    if(size < MINPRIME || size > MAXPRIME || !isPrime(size)) {
        size = findNextPrime(size);
    }
    m_currentTable = new Person[size];

    //populates other member variables
    m_hash = hash;
    m_currentCap = size;
    m_currentSize = 0;
    m_currNumDeleted = 0;
    m_oldTable = nullptr;
    m_oldCap = 0;
    m_oldSize = 0;
    m_oldNumDeleted = 0;
}

Cache::~Cache(){

    //deletes both tables
    delete[] m_currentTable;
    if(m_oldTable != nullptr) {
        delete[] m_oldTable;
    }
}

bool Cache::insert(Person person){

    //if we are in the process of rehashing, move 25% of nodes before doing anything else
    if(m_oldTable != nullptr) {
        rehash25Percent();
    }

    //if the load factor is greater than 0.5 and the table is already the largest it can be, no more entries can be accepted
    if(lambda() >= MAXLOAD && m_currentCap == MAXPRIME) {
        return false;
    }

    //checks whether getPerson returns the empty Person object, signifying that the current person to insert is not already in the list and whether the id is out of range
    if(!(getPerson(person.m_key, person.m_id) == EMPTY)) {
        return false;
    }

    //gets a valid index for the hash table then fills that bucket with the person and increments m_currentSize
    int i = 0;
    int index = ((m_hash(person.m_key) % m_currentCap) + (i * i)) % m_currentCap;
    while(!m_currentTable[index].m_key.empty()) {
        i++;
        index = ((m_hash(person.m_key) % m_currentCap) + (i * i)) % m_currentCap;
    }
    m_currentTable[index] = person;
    m_currentSize++;

    //checks whether the table needs to be rehashed
    if(lambda() > MAXLOAD) {
        startRehash();
    }

    return true;
}

bool Cache::remove(Person person){

    if(getPerson(person.m_key, person.m_id) == EMPTY) {
        return false;
    }

    //if only m_currentTable exists then only m_currentTable is checked for removal of the person, otherwise both tables are checked
    if(m_oldTable == nullptr) {
        int i = 0;
        int index = ((m_hash(person.m_key) % m_currentCap) + (i * i)) % m_currentCap;
        while(!(m_currentTable[index] == person)) {
            i++;
            index = ((m_hash(person.m_key) % m_currentCap) + (i * i)) % m_currentCap;
        }
        m_currentTable[index] = DELETED;
        m_currNumDeleted++;
    }else {
        int i = 0;
        int index1 = ((m_hash(person.m_key) % m_currentCap) + (i * i)) % m_currentCap;
        int index2 = ((m_hash(person.m_key) % m_oldCap) + (i * i)) % m_oldCap;
        while(!(m_currentTable[index1] == person) && !(m_oldTable[index2] == person)) {
            i++;
            index1 = ((m_hash(person.m_key) % m_currentCap) + (i * i)) % m_currentCap;
            index2 = ((m_hash(person.m_key) % m_oldCap) + (i * i)) % m_oldCap;
        }
        if(m_currentTable[index1] == person) {
            m_currentTable[index1] = DELETED;
            m_currNumDeleted++;
        }else {
            m_oldTable[index2] = DELETED;
            m_oldNumDeleted++;
        }
    }

    if(m_oldTable != nullptr) {
        rehash25Percent();
    }

    //checks whether the deleted ratio surpasses the max and necessitates rehashing
    if(deletedRatio() > MAXDELRATIO) {
        startRehash();
    }

    return true;
}

Person Cache::getPerson(string key, int id) const{
    
    for(int i = 0; i < m_currentCap; i++) {
        if(m_currentTable[i].m_key == key && m_currentTable[i].m_id == id) {
            return m_currentTable[i];
        }
    }
    for(int i = 0; i < m_oldCap; i++) {
        if(m_oldTable[i].m_key == key && m_oldTable[i].m_id == id) {
            return m_oldTable[i];
        }
    }
    return EMPTY;
}

float Cache::lambda() const {

    if(m_currentCap == 0 || m_currentSize == 0) {
        return 0.0;
    }else {
        float lambda = float(m_currentSize) / m_currentCap; 
        return lambda;
    }
}

float Cache::deletedRatio() const {

    if(m_currentCap == 0 || m_currentSize == 0 || m_currNumDeleted == 0) {
        return 0.0;
    }else {
        float deletedRatio = float(m_currNumDeleted) / m_currentSize;
        return deletedRatio;
    }
}

void Cache::dump() const {
    cout << "Dump for the current table: " << endl;
    if (m_currentTable != nullptr)
        for (int i = 0; i < m_currentCap; i++) {
            cout << "[" << i << "] : " << m_currentTable[i] << endl;
        }
    cout << "Dump for the old table: " << endl;
    if (m_oldTable != nullptr)
        for (int i = 0; i < m_oldCap; i++) {
            cout << "[" << i << "] : " << m_oldTable[i] << endl;
        }
}

bool Cache::isPrime(int number){
    bool result = true;
    for (int i = 2; i <= number / 2; ++i) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

int Cache::findNextPrime(int current){
    //we always stay within the range [MINPRIME-MAXPRIME]
    //the smallest prime starts at MINPRIME
    if (current < MINPRIME) current = MINPRIME-1;
    for (int i=current; i<MAXPRIME; i++) { 
        for (int j=2; j*j<=i; j++) {
            if (i % j == 0) 
                break;
            else if (j+1 > sqrt(i) && i != current) {
                return i;
            }
        }
    }
    //if a user tries to go over MAXPRIME
    return MAXPRIME;
}

void Cache::startRehash() {

    //populates m_oldTable's values with m_currentTable's values 
    m_oldTable = m_currentTable;
    m_oldCap = m_currentCap;
    m_oldSize = m_currentSize;
    m_oldNumDeleted = m_currNumDeleted;

    //creates a new table with the proper size and populates m_currentTable's values with 0
    m_currentCap = findNextPrime(4 * (m_currentSize - m_currNumDeleted));
    m_currentTable = new Person[m_currentCap];
    m_currentSize = 0;
    m_currNumDeleted = 0;
    
    //moves the first 25% of nodes
    rehash25Percent();
}

void Cache::rehash25Percent() {

    //sets the number of buckets that can be moved in conjunction with a single insert/remove operation (25% of m_oldTable's size)
    int bucketsLeft = m_oldSize / 4;
    int i = 0;

    //iterates through m_oldTable until it finds an availible key and then inserts it into m_currentTable, or until the end of the table is reached(done hashing), 
    //or until 25% of availible data points have been transferred
    while((i < m_oldCap) && (bucketsLeft > 0)) {
        if(m_oldTable[i].m_key != "" && m_oldTable[i].m_key != "DELETED") {

            //gets a valid index for the hash table then fills that bucket with the person and increments m_currentSize
            int j = 0;
            int index = ((m_hash(m_oldTable[i].m_key) % m_currentCap) + (j * j)) % m_currentCap;
            while(!m_currentTable[index].m_key.empty()) {
                j++;
                index = ((m_hash(m_oldTable[i].m_key) % m_currentCap) + (j * j)) % m_currentCap;
            }
            m_currentTable[index] = m_oldTable[i];
            m_currentSize++;
            m_oldTable[i] = DELETED;
            m_oldNumDeleted++;
            bucketsLeft--;
        }
        i++;
    }

    if(m_oldNumDeleted == m_oldSize) {
        delete[] m_oldTable;
        m_oldTable = nullptr;
        m_oldCap = 0;
        m_oldSize = 0;
        m_oldNumDeleted = 0;
    }
    
}

ostream& operator<<(ostream& sout, const Person &person ) {
    if (!person.m_key.empty())
        sout << person.m_key << " (ID " << person.m_id << ")";
    else
        sout << "";
  return sout;
}

bool operator==(const Person& lhs, const Person& rhs){
    return ((lhs.m_key == rhs.m_key) && (lhs.m_id == rhs.m_id));
}
