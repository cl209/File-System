// CMSC 341 - Fall 2024 - Project 4
#include "filesys.h"
#include <math.h>
#include <algorithm>
#include <random>
#include <vector>
using namespace std;
enum RANDOM {UNIFORMINT, UNIFORMREAL, NORMAL, SHUFFLE};
class Random {
public:
    Random(){}
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
        else if (type == UNIFORMREAL) { //the case of UNIFORMREAL to generate real numbers
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_uniReal = std::uniform_real_distribution<double>((double)min,(double)max);
        }
        else { //the case of SHUFFLE to generate every number only once
            m_generator = std::mt19937(m_device());
        }
    }
    void setSeed(int seedNum){
        // we have set a default value for seed in constructor
        // we can change the seed by calling this function after constructor call
        // this gives us more randomness
        m_generator = std::mt19937(seedNum);
    }
    void init(int min, int max){
        m_min = min;
        m_max = max;
        m_type = UNIFORMINT;
        m_generator = std::mt19937(10);// 10 is the fixed seed value
        m_unidist = std::uniform_int_distribution<>(min,max);
    }
    void getShuffle(vector<int> & array){
        // this function provides a list of all values between min and max
        // in a random order, this function guarantees the uniqueness
        // of every value in the list
        // the user program creates the vector param and passes here
        // here we populate the vector using m_min and m_max
        for (int i = m_min; i<=m_max; i++){
            array.push_back(i);
        }
        shuffle(array.begin(),array.end(),m_generator);
    }

    void getShuffle(int array[]){
        // this function provides a list of all values between min and max
        // in a random order, this function guarantees the uniqueness
        // of every value in the list
        // the param array must be of the size (m_max-m_min+1)
        // the user program creates the array and pass it here
        vector<int> temp;
        for (int i = m_min; i<=m_max; i++){
            temp.push_back(i);
        }
        std::shuffle(temp.begin(), temp.end(), m_generator);
        vector<int>::iterator it;
        int i = 0;
        for (it=temp.begin(); it != temp.end(); it++){
            array[i] = *it;
            i++;
        }
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

    string getRandString(int size){
        // the parameter size specifies the length of string we ask for
        // to use ASCII char the number range in constructor must be set to 97 - 122
        // and the Random type must be UNIFORMINT (it is default in constructor)
        string output = "";
        for (int i=0;i<size;i++){
            output = output + (char)getRandNum();
        }
        return output;
    }
    
    int getMin(){return m_min;}
    int getMax(){return m_max;}
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
   for (unsigned int i = 0 ; i < str.length(); i++)
      val = val * thirtyThree + str[i] ;
   return val ;
}

string namesDB[6] = {"driver.cpp", "test.cpp", "test.h", "info.txt", "mydocument.docx", "tempsheet.xlsx"};

class Tester{
    public:
    bool testNormalInsertion();
    bool testGetFileError();
    bool testGetFileNormal();
    bool testRemoveNormal();
    bool testRemoveError();
    bool testRehashInsert();
    bool testRehashRemove();
    bool testChangePolicy();
    bool testUpdateDiskBlock();

};

//Function test to check if insertion works correctly
bool Tester::testNormalInsertion() {
    vector<File> dataList;
    Random RndID(DISKMIN,DISKMAX);
    Random RndName(0,5);// selects one from the namesDB array
    FileSys filesys(MINPRIME, hashCode, QUADRATIC);
    bool result = true;

    for (int i=0;i<49;i++){
        // generating random data
        File dataObj = File(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);
        // saving data for later use
        dataList.push_back(dataObj);
        result = result && filesys.insert(dataObj);
        result = result && (filesys.m_currentSize == i + 1);
        
    }
    // checking whether all data are inserted
    for (vector<File>::iterator it = dataList.begin(); it != dataList.end(); it++){
        result = result && (*it == filesys.getFile((*it).getName(), (*it).getDiskBlock()));

        File checkFile = filesys.getFile((*it).getName(), (*it).getDiskBlock());
        result = result && (checkFile.getName() == (*it).getName() && checkFile.getDiskBlock() == (*it).getDiskBlock());

        
        /*int index = filesys.findFile(*it, QUADRATIC, filesys.m_currentTable, filesys.m_currentCap);
        result = result && (filesys.m_currentTable[index]->getName() == (*it).getName());
        result = result && (filesys.m_currentTable[index]->getDiskBlock() == (*it).getDiskBlock());*/
    }
    return result;
}

//Function test to check if getFile works correctly
bool Tester::testGetFileError(){
    vector<File> dataList;
    Random RndID(DISKMIN,DISKMAX);
    Random RndName(0,5);// selects one from the namesDB array
    FileSys filesys(MINPRIME, hashCode, LINEAR);
    bool result = true;

    //Generating random data and inserting them into the file system
    //Won't cause rehashing
    for (int i=0;i<49;i++){
        // generating random data
        File dataObj = File(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);
        // saving data for later use
        dataList.push_back(dataObj);
        result = result && filesys.insert(dataObj);       
    }

    //Making DNE file
    File DNEFile = File("DNE", 100, true);
    File foundFile = filesys.getFile(DNEFile.getName(), DNEFile.getDiskBlock());
    //Check that DNE file is not in the file system
    result = result && (foundFile.getName() == "");

    return result;
}

//Function test to check if getFile works correctly
bool Tester::testGetFileNormal(){
    vector<File> dataList; // to store the data for later use
    Random RndID(DISKMIN,DISKMAX);
    Random RndName(0,5);// selects one from the namesDB array
    FileSys filesys(MINPRIME, hashCode, QUADRATIC);
    bool result = true;

    //Generating random data and inserting them into the file system
    //Won't cause rehashing
    for (int i=0;i<49;i++){
        // generating random data
        File dataObj = File(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);
        // saving data for later use
        dataList.push_back(dataObj);
        result = result && filesys.insert(dataObj);       
    }
    //Checking that all files are in the file system
    for (vector<File>::iterator it = dataList.begin(); it != dataList.end(); it++){
        //Checking that the file is in the file system
        result = result && (*it == filesys.getFile((*it).getName(), (*it).getDiskBlock()));

        //Getting file from the file system
        File checkFile = filesys.getFile((*it).getName(), (*it).getDiskBlock());

        //Checking that the file is the same as the one we inserted
        /*int index = filesys.findFile(*it, QUADRATIC, filesys.m_currentTable, filesys.m_currentCap);
        result = result && (filesys.m_currentTable[index]->getName() == (*it).getName());
        result = result && (filesys.m_currentTable[index]->getDiskBlock() == (*it).getDiskBlock());*/
    }

    return result;
}

//Function test to check if remove works correctly
bool Tester::testRemoveNormal(){
    vector<File> dataList;
    Random RndID(DISKMIN,DISKMAX);
    Random RndName(0,5);
    FileSys filesys(MINPRIME, hashCode, DOUBLEHASH);
    bool result = true;

    // Insert files
    for (int i=0;i<49;i++){
        File dataObj = File(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);
        dataList.push_back(dataObj);
        result = result && filesys.insert(dataObj);       
    }

    int removeSize = (filesys.m_currentCap * .7);
    int removeCount = 0;

    // Removing files
    for (vector<File>::iterator it = dataList.begin(); it != dataList.end(); it++) {
        // Only try to remove if we haven't hit our limit
        if (removeCount < removeSize) {
            File checkFile = filesys.getFile((*it).getName(), (*it).getDiskBlock());
            
            // Only attempt removal if file exists
            if (checkFile.getName() != "" && removeCount < removeSize) {
                // Only increment removeCount if removal was successful
                if (filesys.remove(checkFile)) {
                    removeCount++;
                }
            }

            File removedFile = filesys.getFile((*it).getName(), (*it).getDiskBlock());
            result = result && (removedFile == File());
            result = result && (filesys.m_currNumDeleted == removeCount);
        }
    }    
    
    return result;
}

//Function test to check if remove works correctly
bool Tester::testRemoveError(){
    vector<File> dataList;
    Random RndID(DISKMIN,DISKMAX);
    Random RndName(0,5);
    FileSys filesys(MINPRIME, hashCode, DOUBLEHASH);
    bool result = true;

    // Insert files
    for (int i=0;i<49;i++){
        File dataObj = File(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);
        dataList.push_back(dataObj);
        result = result && filesys.insert(dataObj);       
    }

    //Create a file that doesn't exist
    File DNEFile = File("DNE", 100, true);

    //Check that the file doesn't exist
    result = result && (filesys.remove(DNEFile) == false);

    return result;
}

//Function test to check if rehashing is triggered and works correctly
//Load factor
bool Tester::testRehashInsert(){
    vector<File> dataList;
    Random RndID(DISKMIN,DISKMAX);
    Random RndName(0,5);// selects one from the namesDB array
    FileSys filesys(MINPRIME, hashCode, QUADRATIC);
    bool result = true;
    bool rehashTriggered = false;


    //51 to trigger rehash
    for (int i=0;i<65;i++){
        // generating random data
        File dataObj = File(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);
        // saving data for later use
        dataList.push_back(dataObj);
        filesys.insert(dataObj);
        //Checking if rehash is triggered
        if (!rehashTriggered && filesys.lambda() > 0.5) {
            rehashTriggered = true;
        }
    }
    //Verify that rehash is triggered
    result = result && (filesys.m_oldTable == nullptr); 
    result = result && rehashTriggered;

    //Checking that all files are in the file system
    for (vector<File>::iterator it = dataList.begin(); it != dataList.end(); it++){
        //Checking that the file is in the file system
        result = result && (*it == filesys.getFile((*it).getName(), (*it).getDiskBlock()));

    }
    
    return result;
}

//Function test to check if rehashing is triggered and works correctly
//Deleted ratio
bool Tester::testRehashRemove() {
    vector<File> dataList;
    Random RndID(DISKMIN,DISKMAX);
    Random RndName(0,5);
    FileSys filesys(MINPRIME, hashCode, LINEAR);
    bool result = true;
    bool rehashTriggered = false;

    // Insert fewer files so deletion ratio can reach 0.8
    for (int i = 0; i < 30; i++) {
        File dataObj = File(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);
        dataList.push_back(dataObj);
        filesys.insert(dataObj);
    }
    int removeSize = (30 * .81);
    int removeCount = 0;
    //vector to store in removed files
    vector<File> removedFiles;

    // Try to remove all files to get high deleted ratio
    for (vector<File>::iterator it = dataList.begin(); it != dataList.end(); it++) {
        File checkFile = filesys.getFile((*it).getName(), (*it).getDiskBlock());
  
        // Only attempt removal if file exists
        if (checkFile.getName() != "" && removeCount < removeSize) {
            //Insert file into removedFiles
            removedFiles.push_back(checkFile);
            // Only increment removeCount if removal was successful
            if (filesys.remove(checkFile)) {
                removeCount++;
            }
            //Checking if rehash is triggered
            if (!rehashTriggered && filesys.deletedRatio() > 0.8) {
                rehashTriggered = true;
            }
        }
    }

    //Verify that rehash is triggered
    result = result && (filesys.m_oldTable == nullptr);
    result = result && (filesys.m_currNumDeleted == removeCount);

    //Search through removed files to verify they were removed
    for (vector<File>::iterator it = removedFiles.begin(); it != removedFiles.end(); it++) {
        File removedFile = filesys.getFile((*it).getName(), (*it).getDiskBlock());
        result = result && (removedFile.getUsed() ==  false);
    }

    return result;
}

//Function test to check if changePolicy works correctly
bool Tester::testChangePolicy() {
    vector<File> dataList;
    Random RndID(DISKMIN,DISKMAX);
    Random RndName(0,5);
    FileSys filesys(MINPRIME, hashCode, QUADRATIC);
    bool result = true;

    // Insert enough to trigger rehash (>50% load factor)
    for (int i = 0; i < 20; i++) {
        File dataObj = File(namesDB[RndName.getRandNum()], 
                          RndID.getRandNum(), 
                          true);
        dataList.push_back(dataObj);
        result = result && filesys.insert(dataObj);
    }

    // Change policy
    filesys.changeProbPolicy(LINEAR);

    // Insert to new policy
    for (int i = 0; i < 49; i++) {
        File dataObj = File(namesDB[RndName.getRandNum()], 
                          RndID.getRandNum(), 
                          true);
        dataList.push_back(dataObj);
        result = result && filesys.insert(dataObj);
    }
    // Verify policy change
    result = result && (filesys.m_newPolicy == LINEAR);

    // Verify all data exists
    for(vector<File>::iterator it = dataList.begin(); it != dataList.end(); it++) {
        result = result && (*it == filesys.getFile((*it).getName(), (*it).getDiskBlock()));
    }

    return result;
}

//Function test to check if updateDiskBlock works correctly
bool Tester::testUpdateDiskBlock() {
    vector<File> newDataList;
    vector<File> oldDataList;
    Random RndID(DISKMIN,DISKMAX);
    Random RndName(0,5);
    FileSys filesys(MINPRIME, hashCode, QUADRATIC);
    bool result = true;

    //Insert - wont cause rehash
    for (int i = 0; i < 20; i++) {
        File dataObj = File(namesDB[RndName.getRandNum()], 
                          RndID.getRandNum(), 
                          true);
        oldDataList.push_back(dataObj);
        result = result && filesys.insert(dataObj);
    }

    //Update disk block
    for (vector<File>::iterator it = oldDataList.begin(); it != oldDataList.end(); it++) {
        int newDiskBlock = RndID.getRandNum();
        result = result && filesys.updateDiskBlock((*it), newDiskBlock);
        //Insert new ID into new list
        newDataList.push_back(File((*it).getName(), newDiskBlock, true));

    }

    //Check that old and new disk blocks are different
    for (int i = 0; i < 20; i++) {
        result = result && (oldDataList[i].getDiskBlock() != newDataList[i].getDiskBlock());
    }

    return result;
}


int main(){
    Tester test;
    if (test.testNormalInsertion()){
        cout << "Normal Insertion: Test passed" << endl;
    }
    else{
        cout << "Normal Insertion: Test failed" << endl;
    }
    if (test.testGetFileError()){
        cout << "GetFile Error: Test passed" << endl;
    }
    else{
        cout << "GetFile Error: Test failed" << endl;
    }
    if (test.testGetFileNormal()){
        cout << "GetFile Normal: Test passed" << endl;
    }
    else{
        cout << "GetFile Normal: Test failed" << endl;
    }
    if (test.testRemoveNormal()){
        cout << "Remove Normal: Test passed" << endl;
    }
    else{
        cout << "Remove Normal: Test failed" << endl;
    }
    if (test.testRemoveError()){
        cout << "Remove Error: Test passed" << endl;
    }
    else{
        cout << "Remove Error: Test failed" << endl;
    }
    if (test.testRehashInsert()){
        cout << "Rehash Insert: Test passed" << endl;
    }
    else{
        cout << "Rehash Insert: Test failed" << endl;
    }
    if (test.testRehashRemove()){
        cout << "Rehash Remove: Test passed" << endl;
    }
    else{
        cout << "Rehash Remove: Test failed" << endl;
    }
    if (test.testChangePolicy()){
        cout << "Change Policy: Test passed" << endl;
    }
    else{
        cout << "Change Policy: Test failed" << endl;
    }
    if (test.testUpdateDiskBlock()){
        cout << "Update Disk Block: Test passed" << endl;
    }
    else{
        cout << "Update Disk Block: Test failed" << endl;
    }
    return 0;
}