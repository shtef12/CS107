using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <cstdio>
#include <string.h>

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;

  cout << actorFileName << endl;

  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.
fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const 
{
  //cout << "get credits" << endl;
  int numOfMovies = *((int*)movieFile);
  void* offsetLoc = (char*)movieFile + 4; //the start of the movie offset section
  void* nextOffset = (char*)offsetLoc + 4; //the next movie offset to calculate delta
  int delta = *(int*)nextOffset - *(int*)offsetLoc; //how big the data is for the movie (size of data)
  bool bInAtLeastOneMovie = false;

  //cout << "movie delta: " << delta << endl;
  for(int i = 0; i < numOfMovies; i++){
     void* loc = (char*)offsetLoc + (i*4);//calculate the next movie offset
     int offset = *((int*)loc);
     if(isActorInMovie(player,offset,films)){
       bInAtLeastOneMovie = true;
     }
  }
  return bInAtLeastOneMovie; 
}

bool imdb::isActorInMovie(const string& player, int movieOffset, vector<film>& films) const
{
  int bytesUsed = 0;
  void* currLocation = (char*)movieFile + movieOffset;
  void* movieRecord = (char*)movieFile + movieOffset; //the start of the movie record
  //title of movie, terminated with '\0', behave like c-string
  char* movieName = (char*)currLocation;
  
  //year film released as single byte, (year - 1900) year is delta from 1900
  //get byte after the movie name (year released)
  int len = strlen(movieName);
  bytesUsed += len * sizeof(char);
  char* lastLetter = movieName + len;
  currLocation = (char*)lastLetter + 1;//points to byte right after last letter in movie name which is the year released
  bytesUsed += 1;

  int yearReleased = (int)(*((char*)currLocation));
  currLocation = (char*)currLocation + 1; //points to byte right after year released
  bytesUsed += 1;

  //if total num bytes used to encode name and year of movie is odd, then an extra '\0' sits in between the 1-byte year and data that follows
  if(bytesUsed % 2 != 0){
    currLocation = (char*)currLocation + 1;
    bytesUsed += 1;
  }
  
  short numOfActors = *((short*)currLocation);
  bytesUsed += sizeof(short);
  currLocation = (char*)currLocation + 1;

  if(bytesUsed % 4 != 0){
    currLocation = (char*)currLocation + 2;
    bytesUsed += 2;
  }

   //array of actor offsets
  currLocation = (char*)currLocation + 1;
  
  int* actorInfo = (int*)currLocation;
  int actorOffset = *(actorInfo);
  int sizeOfArrayBytes = (int)(numOfActors * sizeof(int));

  //if the actor is equal add movie to the movie vector
  vector<string> cast = getActorsFromMovie(currLocation,numOfActors);
  for(int i = 0; i < cast.size(); i++){
    if(strcmp(cast[i].c_str(),player.c_str()) == 0){
      film newFilm;
      newFilm.title = movieName;
      newFilm.year = yearReleased;
      films.push_back(newFilm);
      return true;
    }
    if(player < cast[i]){
      return false;
    }
  }

  return false;
}

vector<string> imdb::getActorsFromMovie(void* pActorOffsets, short numOfActors) const
{
  vector<string> actors;
  int* actorInfo = (int*)pActorOffsets;
  int actorOffset = *(actorInfo);
  
  for(int i = 0; i < numOfActors; i++){
    int* offset = actorInfo + i;
    int numOffset = *(offset);
    string actor = getActorName(numOffset);
    actors.push_back(actor);
  }
  return actors;
}

string imdb::getActorName(int offset) const
{
  void* actorName = (char*)actorFile + offset;
  char* name = (char*)actorName;
  string returnName(name);
  return returnName;
}

bool imdb::getCast(const film& movie, vector<string>& players) const 
{ 
  bool bHasAtLeastOneActor = false;
  string movieTitle = movie.title;
  int numMovies = *((int*)movieFile);
  int movieOffset = binarySearchFile(1,numMovies,movieTitle,movieFile);
  if(movieOffset != -1){
    char* name = (char*)movieFile + movieOffset;
 
  void* movieRecord = (char*)movieFile + movieOffset;

  int bytesUsed = 0;
  void* currLocation = movieRecord;
 
  //year film released as single byte, (year - 1900) year is delta from 1900
  char* title = (char*)movieRecord;
  //get byte after the movie name (year released)
  int len = strlen(title);
  bytesUsed += len * sizeof(char);
  char* lastLetter = title + len;
  currLocation = (char*)lastLetter + 1;//points to byte right after last letter in movie name which is the year released
  bytesUsed += 1;

  currLocation = (char*)currLocation + 1; //points to byte right after year released
  bytesUsed += 1;

  //if total num bytes used to encode name and year of movie is odd, then an extra '\0' sits in between the 1-byte year and data that follows
  if(bytesUsed % 2 != 0){
    currLocation = (char*)currLocation + 1;
    bytesUsed += 1;
  }
  
  short numOfActors = *((short*)currLocation);
  bytesUsed += sizeof(short);
  currLocation = (char*)currLocation + 1;

  if(bytesUsed % 4 != 0){
    currLocation = (char*)currLocation + 2;
    bytesUsed += 2;
  }

   //array of actor offsets
  currLocation = (char*)currLocation + 1;

  int* actorInfo = (int*)currLocation;
  int actorOffset = *(actorInfo);
  int sizeOfArrayBytes = (int)(numOfActors * sizeof(int));

  //get all the actors from the movie
  vector<string> cast = getActorsFromMovie(currLocation,numOfActors);
  //add the cast to the players vector
  if(cast.size() >= 1)
    bHasAtLeastOneActor = true;
  for(int i = 0; i < cast.size(); i++){
    players.push_back(cast[i]);
    }
  }
  
  return bHasAtLeastOneActor; 
}

int imdb::binarySearchFile(int min, int max, string toFind, const void* file) const
{
  if(max < min)
    return -1;
  else{
    int mid = (max + min) / 2;  //calculate mid point
    void* midOffset = (char*)file + (mid * 4); //calculate offset
    int offset = *((int*)midOffset);
    void* record = (char*)file + offset; //go to the record in file
    char* name = (char*)record;
    string sName(name);
    if(toFind == sName){
      return offset;
    }else if(toFind < sName){
      return binarySearchFile(0,mid,toFind,file);
    }else if(toFind > sName){
      return binarySearchFile(mid,max,toFind,file);
    }
  }
}

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}

