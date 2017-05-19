#include <iostream>
#include <fstream>
#include <sstream>

#include "reader.h"

using namespace std;

enum mainErrors {IMPROPER_USE, FILE_DOES_NOT_EXIST};

void readCommandLine(int argc, char *argv[], string &fileName);
void process(ifstream &fin, const string &fileName);
void checkFileExist(ifstream &fin, const string &fileName);

/////////////////////////////////////////////////////////////////
///
int main(int argc, char *argv[]) {
    string fileName;
    ifstream fin;
    try {
        readCommandLine(argc, argv, fileName);
        process(fin, fileName);
    }
    catch (mainErrors e) {
        if(e == IMPROPER_USE)
            cout << "Proper usage: ./process <filename>" << endl;
        if(e == FILE_DOES_NOT_EXIST)
            cout << "File does not exist!" << endl;
    }
    catch (readerErrors f) {
        if(f == BAD_CALCULATION)
            cout << "Error processing file." << endl;
        if(f == INVALID_SCORE)
            cout << "Invlaid Flesch score." << endl;
    }
    catch (...) {
        cout << "Unknown error occured." << endl;
    }
    return 0;
}

////////////////////////////////////////////////////////////////
///
void readCommandLine(int argc, char *argv[], string &fileName) {
    if (argc == 2)
        fileName = argv[1];
    else
        throw IMPROPER_USE;
}

////////////////////////////////////////////////
///
void process(ifstream &fin, const string &fileName) {
    checkFileExist(fin, fileName);
    reader read(fileName);
    read.process();
    read.printFileName();
}

////////////////////////////////////////////////////////////
///
void checkFileExist(ifstream &fin, const string &fileName) {
    fin.open(fileName.c_str());
    fin.close();
    if(!fin.good())
        throw FILE_DOES_NOT_EXIST;
}
