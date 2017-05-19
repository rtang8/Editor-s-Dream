#ifndef READER_H
#define READER_H

using namespace std;

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include <sstream>

#include <vector>
#include <map>
#include <ctime>

enum readerErrors {BAD_CALCULATION, INVALID_SCORE};

class reader {

struct Occurances {
    size_t count;
    size_t paragraphNum;
    size_t lineNum;
};

private:
    size_t wordCount;
    size_t sentenceCount;
    size_t paragraphCount;
    size_t syllableCount;

    map<string, size_t> totalWords;
    map<string, Occurances> wordLocations;

    size_t fleschScore;

    string fileName;

    void copy(const reader &other);
    void deleteAll();

    string getParagraph(ifstream &fin);
    void processParagraph(ifstream &fin, const string paragraph);
    string getNextWord(stringstream &ss, size_t &lineCount);
    void countSyllable();

    double getFleschScore();
    string translateScore(const double &score);


public:
    reader();
    reader(string fileName = "");
    reader(const reader  &other);
    reader &operator=(const reader &other);
    ~reader();

    void process();

    void printFileName() const;

};


////////////////////////////////////////////////////////////////////////////////////////
/// CONSTRUCTORS
///
reader::reader(string filename) {
    wordCount = 0;
    paragraphCount = 0;
    syllableCount = 0;
    fleschScore = 0;
    fileName = filename;
}


reader::reader(const reader &other) {
    copy(other);
}


reader &reader::operator=(const reader &other) {
    if(this != &other)
        copy(other);
    return *this;
}


reader::~reader() {
}


void reader::copy(const reader &other) {
    wordCount = other.wordCount;
    paragraphCount = other.paragraphCount;
    syllableCount = other.syllableCount;
    fleschScore = other.fleschScore;
    fileName = other.fileName;
}

///////////////////////////////////////////////////////////////////////////////
/// PRIMARY FUNCTIONS
///
void reader::process() {

    ifstream fin;
    fin.open(fileName.c_str());

    string paragraph;
    while(!fin.eof()) {

        // Retrieves the next paragraph
        paragraph = getParagraph(fin);

        // Processes each word of the paragraph
        processParagraph(fin, paragraph);


        /// DEBUG OUTPUT
        cout << paragraph << endl;

    }
    cout << "Total word count: " << wordCount << endl;
    cout << "Total paragraph count: " << paragraphCount << endl;
    fin.close();
}


string reader::getParagraph(ifstream &fin) {

    string output, line;

    // Adds to the string until end of paragraph
    while (getline(fin, line))
        if (line.empty()) break;
        else output += line + ' ';

    // Gets rid of last space assuming line isn't empty
    if(output.length() != 0)
        output.erase(output.length() - 1);

    // Increases paragraph count by one
    if(!output.empty())
        ++paragraphCount;

    return output;

}

void reader::processParagraph(ifstream &fin, const string paragraph) {

    // Puts the string into a stream
    stringstream ss;
    ss << paragraph;

    // Adds one to corresponding word on map
    string stringIndex;
    size_t lineCount = 0;
    while(((stringIndex = getNextWord(ss, lineCount)) != "" )) {
        //++totalWords[stringIndex];
        ++wordCount;
        ++wordLocations[stringIndex].count;
        wordLocations[stringIndex].paragraphNum = paragraphCount;
        wordLocations[stringIndex].lineNum = lineCount;
    }

    // DEBUG :: PRINTS OUT MAP
    map<string, Occurances>::iterator iter = wordLocations.begin();
    while(iter != wordLocations.end()) {
        cout << iter->first << ' ' << iter->second.count << endl;
        cout << iter->second.paragraphNum << " | " << iter->second.lineNum << endl;
        ++iter;
    }
}

string reader::getNextWord(stringstream &ss, size_t &lineCount) {

    char strScan;
    string fullWord;

    // Gets first character
    strScan = ss.get();

    // Ignores characters that are not in the alphabet
    while(!(isalpha(strScan)) && !ss.eof())
        strScan = ss.get();

    // Add good characters onto string until space
    while(isalpha(strScan) || strScan == '\'') {
        fullWord += tolower(strScan);
        strScan = ss.get();
    }

    if(strScan == '.' || strScan == '!' || strScan == '?')
        ++lineCount;

    // Capitalizes first letter of word before indexing
    fullWord[0] = toupper(fullWord[0]);

    return fullWord;

}

void reader::countSyllable() {

}


double reader::getFleschScore() {
    if(sentenceCount == 0 || wordCount == 0) throw BAD_CALCULATION;
    return 206.835 - (1.015 * (wordCount / sentenceCount) - (84.6 * (syllableCount / wordCount)));
}

string reader::translateScore(const double &score) {
    if(score < 30 && score >= 0)
        return "College Graduate";
    else if(score < 50)
        return "College";
    else if(score < 60)
        return "10th to 12th Grade";
    else if(score < 70)
        return "8th to 9th Grade";
    else if(score < 80)
        return "7th Grade";
    else if(score < 90)
        return "6th Grade";
    else if(score < 100)
        return "5th Grade";
    else throw INVALID_SCORE;
}

void reader::printFileName() const {
    cout << "File name is " << fileName << endl;
}


#endif // READER_H
