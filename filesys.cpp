// CMSC 341 - Fall 2024 - Project 4
#include "filesys.h"
FileSys::FileSys(int size, hash_fn hash, prob_t probing = DEFPOLCY){
    //Check size
    if (size < MINPRIME) size = MINPRIME;
    if (size > MAXPRIME) size = MAXPRIME;

    //Check for prime
    if (!isPrime(size)) size = findNextPrime(size);

    m_hash = hash; //Set the hash function
    m_currProbing = probing; //Set current probing
    m_newPolicy = probing; //Set the probing policy

    //Intialize curr table
    m_currentCap = size;
    m_currentSize = 0;
    m_currNumDeleted = 0;

    //Create a File
    m_currentTable = new File*[m_currentCap];
    //Intialize curr to null
    for (int i = 0; i < m_currentCap; i++) {
        m_currentTable[i] = nullptr;
    }

    //Intialize old table
    m_oldCap = 0;
    m_oldTable = nullptr;
    m_oldSize = 0;
    m_oldNumDeleted = 0;
    m_oldProbing = m_currProbing;

    //Transfer index
    m_transferIndex = 0;
}

FileSys::~FileSys(){
    //Delete the current table
    if (m_currentTable != nullptr) {
        //Delete the files
        for (int i = 0; i < m_currentCap; i++) {
            if (m_currentTable[i] != nullptr) {
                delete m_currentTable[i];
            }
        }
        //Delete the table
        delete[] m_currentTable;
    }
    //Delete the old table
    if (m_oldTable != nullptr) {
        //Delete the files
        for (int i = 0; i < m_oldCap; i++) {
            if (m_oldTable[i] != nullptr) {
                delete m_oldTable[i];
            }
        }
        //Delete the table
        delete[] m_oldTable;
    }
    //Set the current table to null
    m_currentTable = nullptr;
    //Set the old table to null
    m_oldTable = nullptr;
}

void FileSys::changeProbPolicy(prob_t policy){
    m_newPolicy = policy;
}

bool FileSys::insert(File file){
    //Check block number
    if (file.getDiskBlock() < DISKMIN || file.getDiskBlock() > DISKMAX) {
        return false;
    }

    //Check if file exist in current table
    if (findFile(file, m_currProbing, m_currentTable, m_currentCap) != -1) {
        return false;
    }
    //Check if file exist in old table
    if (m_oldTable != nullptr) {
        if (findFile(file, m_oldProbing, m_oldTable, m_oldCap) != -1) {
            return false;
        }
    }

    //Get index
    int index = m_hash(file.getName()) % m_currentCap;

    //Check for probing
    int i = 0;
    if (!(isEmpty(index, m_currentTable))) {
        if (m_currProbing == LINEAR){
            do {
                index = ((m_hash(file.getName()) % m_currentCap) + (i * 1)) % m_currentCap;
                if (isEmpty(index, m_currentTable)) {
                    m_currentTable[index] = new File(file);
                    m_currentSize++;
                    return true;
                }
                i++;
            } while (!(isEmpty(index, m_currentTable)));

        } else if (m_currProbing == QUADRATIC){
            do {
                index = ((m_hash(file.getName()) % m_currentCap) + (i * i)) % m_currentCap;
                if (isEmpty(index, m_currentTable)) {
                    m_currentTable[index] = new File(file);
                    m_currentSize++;
                    return true;
                }
                i++;
            } while (!(isEmpty(index, m_currentTable)));
        }
        else if (m_currProbing == DOUBLEHASH){
            do {
                index = ((m_hash(file.getName()) % m_currentCap) + i * (11-(m_hash(file.getName()) % 11))) % m_currentCap;
                if (isEmpty(index, m_currentTable)) {
                    m_currentTable[index] = new File(file);
                    m_currentSize++;
                    return true;
                }
                i++;
            } while (!(isEmpty(index, m_currentTable)));
        }
    }

    //Check index
    if (index < 0 || index > m_currentCap) {
        return false;
    }
    //Insert file
    m_currentTable[index] = new File(file);
    m_currentSize++;

    //Check load factor
    if (lambda() > 0.5 || deletedRatio() > 0.8) {
        reHashTable();
    }
    
    return true;
    
}
bool FileSys::remove(File file){
    bool result = true;
    
    // Get indexes for both tables
    int currentIndex = findFile(file, m_currProbing, m_currentTable, m_currentCap);
    int oldIndex = findFile(file, m_oldProbing, m_oldTable, m_oldCap);
    
    // Check current table first
    if (currentIndex != -1) {
        m_currentTable[currentIndex]->setUsed(false);
        m_currNumDeleted++;
    } 
    // Then check old table if exists
    else if (oldIndex != -1) {
        m_oldTable[oldIndex]->setUsed(false);
        m_oldNumDeleted++;
    }
    else {
        result = false;
    }

    // Check if rehash needed
    if (deletedRatio() > 0.8 || m_oldTable != nullptr || lambda() > 0.5) {
        reHashTable();
    }

    return result;
}
//Reseting size and table
void FileSys::reSetTable(){
    //Set new capacity
    int capacity = findNextPrime((m_currentSize - m_currNumDeleted)*4);
    
    //Reset member variables
    m_currentSize = 0;
    m_currNumDeleted = 0;
    m_currentCap = capacity;
    
    //Create new table
    m_currentTable = new File*[m_currentCap];

    //Intialize curr to null
    for (int i = 0; i < m_currentCap; i++) {
        m_currentTable[i] = nullptr;
    }

}

//Used for copying data during rehashing
bool FileSys::reHashInsert(File file){
    //Making sure there's no duplicates
    if (findFile(file, m_currProbing, m_currentTable, m_currentCap) != -1) {
        return false;
    }
    //Check for old table
    if (m_oldTable != nullptr) {
        if (findFile(file, m_oldProbing, m_oldTable, m_oldCap) != -1) {
            return false;
        }
    }
    //Check for valid block number
    if (file.getDiskBlock() < DISKMIN || file.getDiskBlock() > DISKMAX) {
        return false;
    }

    //Store index
    int index = m_hash(file.getName()) % m_currentCap;

    //Check for probing
    if (!(isEmpty(index, m_currentTable))) {
        //Temporary index
        int i = 0; 
        if (m_currProbing == LINEAR){
            do {
                index = ((m_hash(file.getName()) % m_currentCap) + (i * 1)) % m_currentCap;
                i++;
            } while (!(isEmpty(index, m_currentTable)));

        } else if (m_currProbing == QUADRATIC){
            do {
                index = ((m_hash(file.getName()) % m_currentCap) + (i * i)) % m_currentCap;
                i++;
            } while (!(isEmpty(index, m_currentTable)));
        }
        else if (m_currProbing == DOUBLEHASH){
            do {
                index = ((m_hash(file.getName()) % m_currentCap) + i * (11-(m_hash(file.getName()) % 11))) % m_currentCap;
                if (isEmpty(index, m_currentTable)) {
                    m_currentTable[index] = new File(file);
                    m_currentSize++;
                    return true;
                }
                i++;
            } while (!(isEmpty(index, m_currentTable)));
        }
    }
    //Check index
    if (index < 0 || index > m_currentCap) {
        return false;
    }

    //Insert file
    m_currentTable[index] = new File(file);
    m_currentSize++;

    return true;
}
void FileSys::reHashTable() {
    // Update probing policy if changed
    if (m_currProbing != m_newPolicy) {
        m_currProbing = m_newPolicy;
    }

    // First time rehashing - setup old table
    if (m_oldTable == nullptr) {
        // Save current table as old table
        m_oldCap = m_currentCap;
        m_oldNumDeleted = m_currNumDeleted;
        m_oldSize = m_currentSize;
        m_oldProbing = m_currProbing;
        m_oldTable = m_currentTable;

        // Create new current table with proper size
        reSetTable();
    }

    // Transfer 25% of data
    int transfered = (m_oldSize * 0.25);
    int inserted = 0;

    // Transfer data from old table to current table
    while (inserted < transfered && m_transferIndex < m_oldCap) {
        if (!(isEmpty(m_transferIndex, m_oldTable))) {
            string name = m_oldTable[m_transferIndex]->getName();
            int block = m_oldTable[m_transferIndex]->getDiskBlock();

            // Delete the file from old table
            m_oldTable[m_transferIndex]->setUsed(false);
            m_oldNumDeleted++;

            // Insert the file into new table
            File file(name, block);
            if (reHashInsert(file)) {
                inserted++;
            }
        }
        // Increment transfer index
        m_transferIndex++;
    }

    // Clear old table only if all data has been transferred
    if (m_transferIndex >= m_oldCap) {
        for (int i = 0; i < m_oldCap; i++) {
            if (m_oldTable[i] != nullptr) {
                delete m_oldTable[i];
            }
        }
        delete[] m_oldTable;
        m_oldTable = nullptr;
        m_transferIndex = 0;
    }
}
// Helper function
bool FileSys::isEmpty(int index, File** table) {
    return table[index] == nullptr || (table[index] && !table[index]->getUsed());
}


const File FileSys::getFile(string name, int block) const{
    // Check if current table is null
    if (m_currentTable == nullptr) {
        return File();
    }

    // Check current table
    int i = 0; // Counter
    int index = m_hash(name) % m_currentCap; // Get index
    while (i < m_currentCap) {
        if (m_currentTable[index] != nullptr && 
            m_currentTable[index]->getUsed() && 
            m_currentTable[index]->getName() == name &&
            m_currentTable[index]->getDiskBlock() == block) {
                return *m_currentTable[index];
        }
        
        // Check for probing
        if (m_currProbing == LINEAR) {
            index = ((m_hash(name) % m_currentCap) + (i * 1)) % m_currentCap;
        } else if (m_currProbing == QUADRATIC) {
            index = ((m_hash(name) % m_currentCap) + (i * i)) % m_currentCap;
        } else if (m_currProbing == DOUBLEHASH) {
            index = ((m_hash(name) % m_currentCap) + i * (11-(m_hash(name) % 11))) % m_currentCap;
        }
        i++;
    }

    // Check old table if exists
    if (m_oldTable != nullptr) {
        i = 0;
        index = m_hash(name) % m_oldCap;
        while (i < m_oldCap) {
            if (m_oldTable[index] != nullptr && 
                m_oldTable[index]->getUsed() && 
                m_oldTable[index]->getName() == name &&
                m_oldTable[index]->getDiskBlock() == block) {
                    return *m_oldTable[index];
            }
            
            // Check for probing
            if (m_oldProbing == LINEAR) {
                index = ((m_hash(name) % m_oldCap) + (i * 1)) % m_oldCap;
            } else if (m_oldProbing == QUADRATIC) {
                index = ((m_hash(name) % m_oldCap) + (i * i)) % m_oldCap;
            } else if (m_oldProbing == DOUBLEHASH) {
                index = ((m_hash(name) % m_oldCap) + i * (11-(m_hash(name) % 11))) % m_oldCap;
            }
            i++;
        }
    }

    return File();
}

bool FileSys::updateDiskBlock(File file, int block){
    //Check if file exist in current table
    int index = findFile(file, m_currProbing, m_currentTable, m_currentCap);
    if (index != -1) {
        m_currentTable[index]->setDiskBlock(block);
        return true;
    }
    //Check if file exist in old table
    int oldIndex = findFile(file, m_oldProbing, m_oldTable, m_oldCap);
    if (oldIndex != -1) {
        m_oldTable[oldIndex]->setDiskBlock(block);
        return true;
    }

    //File doesn't exist
    return false;
}
int FileSys::findFile(const File& file, prob_t probe, File** table, int capacity){
    if (table == nullptr) return -1;  // Check table exists first

    // Get index
    int index = m_hash(file.getName()) % capacity;

    // Search through table
    int i = 0;
    while (i < capacity) {
        // Check null first before accessing members
        if (table[index] != nullptr) {
            if (table[index]->getUsed() && 
                table[index]->getName() == file.getName() &&
                table[index]->getDiskBlock() == file.getDiskBlock()) {
                return index;
            }
        }

        // Update index based on probe type
        if (probe == LINEAR) {
            index = ((m_hash(file.getName()) % capacity) + (i * 1)) % capacity;
        } else if (probe == QUADRATIC) {
            index = ((m_hash(file.getName()) % capacity) + (i * i)) % capacity;
        } else if (probe == DOUBLEHASH) {
            index = ((m_hash(file.getName()) % capacity) + i * (11-(m_hash(file.getName()) % 11))) % capacity;
        }
        i++;
    }
    return -1;
}
float FileSys::lambda() const {
    //Calculate the lambda
    return float(m_currentSize) / m_currentCap;
}

float FileSys::deletedRatio() const {
    //Calculate the deleted ratio
    return float(m_currNumDeleted) / m_currentCap;
}

void FileSys::dump() const {
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

bool FileSys::isPrime(int number){
    bool result = true;
    for (int i = 2; i <= number / 2; ++i) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

int FileSys::findNextPrime(int current){
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