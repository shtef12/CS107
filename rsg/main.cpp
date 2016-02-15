#include <map>
#include <fstream>
#include "definition.h"
#include "production.h"
using namespace std;

/**
 * Takes a reference to a legitimate infile (one that's been set up
 * to layer over a file) and populates the grammar map with the
 * collection of definitions that are spelled out in the referenced
 * file.  The function is written under the assumption that the
 * referenced data file is really a grammar file that's properly
 * formatted.  You may assume that all grammars are in fact properly
 * formatted.
 *
 * @param infile a valid reference to a flat text file storing the grammar.
 * @param grammar a reference to the STL map, which maps nonterminal strings
 *                to their definitions.
 */

static void readGrammar(ifstream& infile, map<string, Definition>& grammar)
{
  while (true) {
    string uselessText;
    getline(infile, uselessText, '{');
    if (infile.eof()) return;  // true? we encountered EOF before we saw a '{': no more productions!
    infile.putback('{');
    Definition def(infile);
    grammar[def.getNonterminal()] = def;
  }
}

/**
 * Builds sentences based on the nonterminal given. It will recursively call itself if
 * it encounters another nonterminal string.
 *
 * @param grammar a reference to the STL map, which holds nonterminal strings
 *          to their definitions
 * @param nonterminal a string of a nonterminal to use as the key for the grammar map
 */
void buildSentences(map<string,Definition> &grammar, string nonterminal)
{
    Production prod = grammar[nonterminal].getRandomProduction();
    Production::iterator itr = prod.begin();
    while(itr != prod.end()){
        string word = *itr;
        if(word[0] != '<'){
            cout << word << " ";
        }else{
            buildSentences(grammar,word);
        }
        itr++;
    }
}

/**
 * Performs the rudimentary error checking needed to confirm that
 * the client provided a grammar file.  It then continues to
 * open the file, read the grammar into a map<string, Definition>,
 * and then print out the total number of Definitions that were read
 * in.  You're to update and decompose the main function to print
 * three randomly generated sentences, as illustrated by the sample
 * application.
 *
 * @param argc the number of tokens making up the command that invoked
 *             the RSG executable.  There must be at least two arguments,
 *             and only the first two are used.
 * @param argv the sequence of tokens making up the command, where each
 *             token is represented as a '\0'-terminated C string.
 */

int main(int argc, char *argv[])
{
  if (argc == 1) {
    cerr << "You need to specify the name of a grammar file." << endl;
    cerr << "Usage: rsg <path to grammar text file>" << endl;
    return 1; // non-zero return value means something bad happened
  }

  ifstream grammarFile(argv[1]);
  if (grammarFile.fail()) {
    cerr << "Failed to open the file named \"" << argv[1] << "\".  Check to ensure the file exists. " << endl;
    return 2; // each bad thing has its own bad return value
  }

  // things are looking good...
  map<string, Definition> grammar;
  readGrammar(grammarFile, grammar);
  buildSentences(grammar,"<start>");

 // cout << "The grammar file called \"" << argv[1] << "\" contains "
   //    << grammar.size() << " definitions." << endl;

  return 0;
}

