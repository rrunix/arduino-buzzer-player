#include "songs.h"
#include <string.h>

//Defining arduino flags.
//Change your pc architecture and compiler if it doesn't match
//with  __GNUC__ && __x86_64__
//Or simply, put only the ARDUINE FLAG
//without the if preprocessor instructionw

//The porpouse of this is for being
//able to compile and test the program
//without an arduino. Just print
//the notes and duration
//Modify the main method for 
//other behaviours.
#if  __GNUC__ && __x86_64__
	#define ARDUINE 0
	#include <stdio.h>
	#include <unistd.h>
	#include <stdlib.h>
	#include <time.h>
	#include "StringFormat.h"
#else
	#define ARDUINE 1
#endif

//Defining constants
#define START_STOP_PIN 2
#define NEXT_SONG_PIN 3
#define BUZE_PIN A1
#define DELAY_ON_STOP 500
#define TIME_ON_STATE_CHANGE 1000
#define SWITCH_CHANGE LOW
#define UPDATE_SONG_NAME 1
#define MAX_NAME_SIZE 25
#define DELAY_BETWEEN_SONGS 2000


//Defining types

//Program state
typedef enum {
	RUN = 1,
	STOP = 0
} Player_state;

typedef enum {
	NORMAL = 1,
	RANDOM = 2
} Player_behaviour;

//Attributes

//ActualSong (pointing to the actual note)
int *actualMusic;

//Last update (Player_state) change
//Wait a few time between changes in order
//to avoid the noisy in the circuit
int lastChangeStateMilli;

//Last song change
//Wait a few time between changes in order
//to avoid the noisy in the circuit
int lastChangeSongMilli;

//Array of songs
//Be free of add or remove songs
//Be sure
int *songs[] =
	{ starWars, simpsons, indiana, knighRider, misionImp, topGun, bond};

//The number of songs sizeof(songs)/sizeof(int*)
unsigned long numberOfSongs;

//The number of the actualSong
int actualSong = 0;

//The state of the player, by default STOP.
Player_state player_state = STOP;

//The max song name's length
char actualSongName[MAX_NAME_SIZE];

//The next song behaviour
Player_behaviour player_behaviour = RANDOM;

// functions signature

/**
 *
 *setup method, set up the arduino pins and load the first song. 
 * If ARDUINE is 0, only load the first song
 */
void setup(void);

/**
 * Play the actual note. If the song doesn't have
 * more notes return 0, else 1.
 *
 * Arduine : buzze and delay
 * Not arduine : just print the tone and delay
 *
 */
int playActualNote(void);

/**
 * Load the song pointed
 * by songs[actualSong % numberOfSongs]
 * And updates the song name
 * The max song's length is defined by MAX_NAME_SIZE 
 */
void loadSong(void);

/**
 * Play the actual note. If the song has'nt more notes
 * load the next song.
 *
 * If ARDUINE, listen for stop/start and nextSong events
 *
 */
void loop(void);

/**
 * Point to the next song
 * If the list is ended
 * point to the first. You
 * can change this behaviohur
 * and add shuffle or whatever
 * you want. Just make sure that
 * actualSong is in the bounds of the array
 * of songs
 */
void nextSong(void);

/**
 * Updates the actual song's name
 *
 */
void updateSongName();

/**
 * Do a delay of millis
 *
 */
void doDelay(int millis);

/**
 * Returns a random int
 * between 0 and max
 *
 */
int randomValue(int max);



//FUNCTIONS BODY

int randomValue(int max) {
#if ARDUINE
	return ramdom(max);
#else
	return rand() % max;
#endif
}

void doDelay(int millis) {
#if ARDUINE
	delay(millis);
#else
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = millis*1000000;
	nanosleep(&ts, NULL);
#endif
}


void setup()
{
numberOfSongs = sizeof(songs)/sizeof(int*);
#if ARDUINE
	pinMode(START_STOP_PIN, INPUT);
	pinMode(NEXT_SONG_PIN, INPUT);
	pinMode(BUZE_PIN, INPUT);
	lastChangeStateMilli = millis();
	lastChangeSongMilli = millis();
        randomSeed(analogRead(0));
#else
	srand(time(NULL));
#endif
loadSong();
}

int playActualNote()
{
	int actualNote = *actualMusic;
	if (actualNote != 0) {
		int tone = ((actualNote >> 16) & 0xFFFF);
		int duration = (actualNote & 0xFFFF);
		if (tone != 0) {
#if ARDUINE
			tone(BUZE_PIN, tone);
#endif
		}
		doDelay(duration);
#if ! ARDUINE
		printf("tone : %d, duration : %d \n", tone, duration);
#endif
		actualMusic++;
		return 1;
	}
	return 0;
}


void loadSong()
{
	actualMusic = songs[actualSong % numberOfSongs];
	if(*actualMusic == 0) {
		actualMusic++;
		while(*actualMusic != 0) {
			actualMusic++;
		}
		actualMusic++;
	}
	updateSongName();
#if ! ARDUINE
	printf("\n\n\n%ssong %s %s\n\n\n",GREEN,actualSongName,RESET);
#endif
}

void updateSongName() {	
	int* song = songs[actualSong % numberOfSongs];
	if(*song == 0) {
		song++;//Pointing to the first number
		int sizeName = 0; 
		int size = 0;
		while(*song != 0 && size < MAX_NAME_SIZE ) {
			int actualChunck = *song;
			int i;
			for(i = 0; i < 4 && size < MAX_NAME_SIZE ; i++) {
				const char actualChar = (char) actualChunck % 0xFF;
				actualSongName[size++] = actualChar;
				actualChunck = actualChunck >> 8;	
			}
			song++;
		}
		for(; size < MAX_NAME_SIZE; size++) {
			actualSongName[size] = '\0';
		}
	}
}

void loop()
{
#if ARDUINE
	if (digitalRead(START_STOP_PIN) == SWITCH_CHANGE
		&& (millis() - lastChangeStateMilli) > TIME_ON_STATE_CHANGE) {
		if (player_state == RUN) {
			player_state = STOP;
		} else {
			player_state = RUN;
		}
		lastChangeStateMilli = millis();
	}

	if (digitalRead(NEXT_SONG_PIN) == SWITCH_CHANGE
		&& (millis() - lastChangeSongMilli) > TIME_ON_STATE_CHANGE) {
		nextSong();
		lastChangeSongMilli = millis();
	}
#endif
	if (player_state == RUN) {
		int state = playActualNote();
		if (state == 0) {
			nextSong();
		}
	} else {
		doDelay(DELAY_ON_STOP);
	}

}

void nextSong()
{
	if(player_behaviour == RANDOM) {
		actualSong = randomValue(numberOfSongs);
	} else {
		actualSong++;
	}
	doDelay(DELAY_BETWEEN_SONGS);
	loadSong();
}

#if ! ARDUINE
int main(int argc, char **argv)
{
	setup();
	player_state = RUN;
	while (1) {
		loop();
	}
}
#endif
