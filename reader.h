#ifndef READER_H
#define READER_H

using namespace std;

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <chrono>

#include <vector>
#include <map>
#include <ctime>

enum readerErrors {BAD_CALCULATION, INVALID_SCORE};

class reader {

struct Occurances {
    size_t count;
    vector<size_t> paragraphNum;
    vector<size_t> lineNum;
};

private:
    size_t wordCount;
    size_t sentenceCount;
    size_t paragraphCount;
    int syllableCount;

    map<char, size_t> totalWords;
    map<string, Occurances> wordLocations;

    size_t fleschScore;
    string fileName;
    size_t procTime;

    void copy(const reader &other);
    void deleteAll();

    string getParagraph(ifstream &fin);
    void processParagraph(string &paragraph);
    vector<string> splitString(const string &input, size_t &lineCount);

    void countSyllable(string input);
    bool isVowel(char &ch);
    bool isValid(char &ch);

    void printTopTen(ostream &out);
    bool askToPrintOccurances();
    bool askToPrintLoc();
    bool askToPrintFile();
    bool getInput();
    string askForFileName();

    bool checkFileExist(const string &fileName);

    double getFleschScore();
    string translateScore(const double &score);

public:
    reader();
    reader(string fileName = "");
    reader(const reader  &other);
    reader &operator=(const reader &other);
    ~reader();

    void process();
    void printInfo(ostream &out);
    void printToFile();

    void printFileName() const;

};


////////////////////////////////////////////////////////////////////////////////////////
/// CONSTRUCTORS
///
reader::reader(string filename) {
    wordCount = 0;
    paragraphCount = 0;
    sentenceCount = 0;
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

reader::~reader() {}

void reader::copy(const reader &other) {
    wordCount = other.wordCount;
    paragraphCount = other.paragraphCount;
    sentenceCount = other.sentenceCount;
    syllableCount = other.syllableCount;
    fleschScore = other.fleschScore;
    fileName = other.fileName;
}

///////////////////////////////////////////////////////////////////////////////
/// PRIMARY FUNCTIONS
///
void reader::process() {

    // Records time before operation
    auto t_start = chrono::high_resolution_clock::now();

    ifstream fin;
    fin.open(fileName.c_str());

    string paragraph;
    while(!fin.eof()) {

        // Retrieves the next paragraph
        paragraph = getParagraph(fin);

        // Processes each word of the paragraph
        processParagraph(paragraph);

    }

    fin.close();

    // Records final time and prints processing time
    auto t_end = chrono::high_resolution_clock::now();
    procTime = chrono::duration_cast<chrono::milliseconds>(t_end - t_start).count();
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

void reader::processParagraph(string &paragraph) {

    // Splits string and stores in vector
    size_t lineCount = 1;
    vector<string> words = splitString(paragraph, lineCount);

    wordCount += words.size();

    // Indexes the word in the appropriate vectors
    for(size_t i = 0; i < words.size(); ++i) {
        if(isalpha(words[i][0])) {
            ++wordLocations[words[i]].count;
            ++totalWords[words[i][0]];
            wordLocations[words[i]].paragraphNum.push_back(paragraphCount);
            countSyllable(words[i]);
        }
    }

    sentenceCount += --lineCount;
}


/// Splits string by whitespace and inserts elements into vector
vector<string> reader::splitString(const string &input, size_t &lineCount) {

    vector<string> words;
    string temp;

    stringstream su(input);

    // Adds words to vector 'words' using whitespace as delimiter
    while (su >> temp) {

        // Removes non-alphabetical chars except ' and counts sentences
        for(size_t i = 0; i < temp.size(); ++i) {
            if((temp[i] == '.' || temp[i] == '!' || temp[i] == '?'))
                ++lineCount;
            if((isdigit(temp[i]) || ispunct(temp[i])) && temp[i] != '\'') {
                temp.erase(temp.begin() + i);
                --i;
            }
        }

        // Remove ending quotes after punctuations like "water?"
        if(temp[temp.size() - 1] == '\"' || temp[temp.size() - 1] == '\'')
            temp.pop_back();

        // Deletes words starting with quotes
        if(temp[0] == '\'' || temp[0] == '\"')
            temp.erase(temp.begin());

        // Capitalizes first char. before indexing
        temp[0] = toupper(temp[0]);

        // Stores the line number of word within paragraph
        if(isalpha(temp[0]))
            wordLocations[temp].lineNum.push_back(lineCount);

        words.push_back(temp);
    }

    return words;
}

/// Roughly estimates syllable count for flesch score
void reader::countSyllable(string input) {

    size_t tempCount = 0;
    size_t i = 0;

    // Ignores groups of vowels and consonants
    while (i < input.size()) {

        // Skips over non-vowels and ensures against buffer-overflow
        while(!isVowel(input[i]) && isValid(input[i]))
            ++i;

        // Does not do final increment if ends in consonants
        if(i >= input.size()) break;

        // Skips blocks of vowels before adding one
        while(isVowel(input[i])) ++i;

        ++tempCount;
    }

    // Ignores silent e at end (Most cases in English)
    if(input[input.size() - 1] == 'e' && tempCount != 1)
        --tempCount;

    syllableCount += tempCount;
}

bool reader::isVowel(char &ch) {
    ch = tolower(ch);
    return ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u' || ch == 'y';
}

bool reader::isValid(char &ch) {
    return (isalnum(ch) || ispunct(ch));
}

/// Retrieves score which determines reading level of piece
double reader::getFleschScore() {
    if(sentenceCount == 0) ++sentenceCount;
    if(sentenceCount == 0 || wordCount == 0) throw BAD_CALCULATION;
    return 206.835 - (1.015 * (wordCount * 1.0  / sentenceCount)) -
                     (84.6 * (syllableCount * 1.0 / wordCount));
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
    else if(score > 100)
        return "Below 5th Grade";
    else throw INVALID_SCORE;
}

///////////////////////////////////////////////////////////////////////
/// PRINTING FUNCTIONS
///
void reader::printInfo(ostream &out) {

    out << endl << "******* TEXT INFORMATION ********" << endl;
    out << "Processing Time :  " << procTime  << "ms"  << endl;
    out << "Word count      :  " << wordCount          << endl;
    out << "Paragraph count :  " << paragraphCount     << endl;
    out << "Reading Level   :  " << translateScore(getFleschScore());
    out << endl << "*********************************" << endl;

    // Prints out the number of words starting with each letter
    string barrier("-----------------------------------------");
    out << barrier << endl;
    out << "Number of words starting with each letter" << endl;
    for(auto elem : totalWords)
       out << elem.first << " : " << elem.second << endl;
    out << barrier << endl;

    // Prints top ten words
    printTopTen(out);
    out << barrier << endl;

    // Asks user if they want to print occurances
    bool printOcc = askToPrintOccurances();

    // Prints out all occurances and asks if user wants to print locations
    if(printOcc) {

        bool printLoc = askToPrintLoc();

        map<string, Occurances>::iterator iter = wordLocations.begin();
        while(iter != wordLocations.end()) {
            out << "Word: " << iter->first
                << " | Occurances: " << iter->second.count << endl;

            // WARNING :: THIS WILL PRINT OUT EVERY LOCATION OF EACH WORD
            if(printLoc) {
            for(size_t i = 0; i < iter->second.paragraphNum.size(); ++i)
                out << " " << "Paragraph: " << iter->second.paragraphNum[i]
                    << " | Sentence: " << iter->second.lineNum[i] << endl;
            }
            ++iter;
        }
    }
    out << endl;
}

void reader::printTopTen(ostream &out) {

    out << "Top ten most frequent words" << endl;

    // Creates a map with # of occurances as key
    multimap<size_t, string> reverseMap;
    for(map<string, Occurances>::const_iterator it = wordLocations.begin();
        it != wordLocations.end(); ++it)
            reverseMap.insert(pair<size_t, string>(it -> second.count, it -> first));

    // Prints out first ten values
    size_t breaker = 0;
    multimap<size_t, string>::const_reverse_iterator iter = reverseMap.rbegin();
    while(++breaker <= 10 && iter != reverseMap.rend()) {
        out << "Count: " << iter->first << " " << iter->second << endl;
        ++iter;
    }

}

bool reader::askToPrintOccurances() {
    cout << "Do you want to print the number of occurances of each word? (Y/N) : ";
    return getInput();
}

bool reader::askToPrintLoc() {
    cout << "Do you want to print the locations of each word?" << endl;
    cout << "WARNING: This is highly unrecommended for large texts! (Y/N) : ";
    return getInput();
}

bool reader::askToPrintFile() {
    cout << "Do you want to print the information to a file? (Y/N) : ";
    return getInput();
}

bool reader::getInput() {
    string input;
    getline(cin, input);
    return (toupper(input[0]) == 'Y') ? true : false;
}

void reader::printToFile() {
    if(askToPrintFile()) {
        string outFileName;
        outFileName = askForFileName();

        ofstream fout;
        fout.open(outFileName.c_str());
        printInfo(fout);
        fout.close();
        cout << "Successfully saved info to " << outFileName << endl;
    }
}

string reader::askForFileName() {

    string input;
    cout << "Please enter a filename: ";
    getline(cin, input);
    input = input.substr(0, input.find(" "));
    cout << input << endl;

    // Keeps asking for filename if already exists
    while(checkFileExist(input)) {
        cout << "Error: File already exists." << endl;
        cout << "Please enter another filename: ";
        getline(cin, input);
        input = input.substr(0, input.find(" "));
        cout << input << endl;
    }

    return input;
}

bool reader::checkFileExist(const string &fileName) {
    ifstream fin;
    fin.open(fileName.c_str());
    fin.close();
    return fin.good();
}

void reader::printFileName() const {
    cout << "File name is " << fileName << endl;
}


#endif // READER_H
