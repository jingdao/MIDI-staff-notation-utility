#include <string.h>

char *midi_key_lookuptable[][2] = {
    {"Cb major","Ab minor"},
    {"Gb major","Eb minor"},
    {"Db major","Bb minor"},
    {"Ab major","F minor"},
    {"Eb major","C minor"},
    {"Bb major","G minor"},
    {"F major","D minor"},
    {"C major","A minor"},
    {"G major","E minor"},
    {"D major","B minor"},
    {"A major","F# minor"},
    {"E major","C# minor"},
    {"B major","G# minor"},
    {"F# major","D# minor"},
    {"C# major","A# minor"},
};

char *midi_instrument_lookuptable[][2] = {
{"Piano","Acoustic Grand Piano"},
{"Piano","Bright Acoustic Piano"},
{"Piano","Electric Grand Piano"},
{"Piano","Honky-tonk Piano"},
{"Piano","Electric Piano 1"},
{"Piano","Electric Piano 2"},
{"Piano","Harpsichord"},
{"Piano","Clavichord"},
{"Chromatic Percussion","Celesta"},
{"Chromatic Percussion","Glockenspiel"},
{"Chromatic Percussion","Music Box"},
{"Chromatic Percussion","Vibraphone"},
{"Chromatic Percussion","Marimba"},
{"Chromatic Percussion","Xylophone"},
{"Chromatic Percussion","Tubular Bells"},
{"Chromatic Percussion","Dulcimer"},
{"Organ","Drawbar Organ"},
{"Organ","Percussive Organ"},
{"Organ","Rock Organ"},
{"Organ","Church Organ"},
{"Organ","Reed Organ"},
{"Organ","Accordion Organ"},
{"Organ","Harmonica"},
{"Organ","Tango Organ"},
{"Guitar","Nylon Guitar"},
{"Guitar","Steel Guitar"},
{"Guitar","Jazz Guitar"},
{"Guitar","Clean Guitar"},
{"Guitar","Muted Guitar"},
{"Guitar","Overdrive Guitar"},
{"Guitar","Distorted Guitar"},
{"Guitar","Harmonic Guitar"},
{"Bass","Acoustic Bass"},
{"Bass","Fingered Bass"},
{"Bass","Pick Bass"},
{"Bass","Fretless Bass"},
{"Bass","Slap Bass 1"},
{"Bass","Slap Bass 2"},
{"Bass","Synth Bass 1"},
{"Bass","Synth Bass 2"},
{"Strings","Violin"},
{"Strings","Viola"},
{"Strings","Cello"},
{"Strings","Double Bass"},
{"Strings","Tremolo Strings"},
{"Strings","Pizzicato Strings"},
{"Strings","Harp"},
{"Strings","Timpani"},
{"Ensemble","String Ensemble 1"},
{"Ensemble","String Ensemble 2"},
{"Ensemble","SynthStrings 1"},
{"Ensemble","SynthStrings 2"},
{"Ensemble","Choir Aahs"},
{"Ensemble","Voice Oohs"},
{"Ensemble","Synth Voice"},
{"Ensemble","Orchestra Hit"},
{"Brass","Trumpet"},
{"Brass","Trombone"},
{"Brass","Tuba"},
{"Brass","Muted Trumpet"},
{"Brass","French Horn"},
{"Brass","Brass Section"},
{"Brass","SynthBrass 1"},
{"Brass","SynthBrass 2"},
{"Reed","Soprano Sax"},
{"Reed","Alto Sax"},
{"Reed","Tenor Sax"},
{"Reed","Baritone Sax"},
{"Reed","Oboe"},
{"Reed","English Horn"},
{"Reed","Bassoon"},
{"Reed","Clarinet"},
{"Pipe","Piccolo"},
{"Pipe","Flute"},
{"Pipe","Recorder"},
{"Pipe","Pan Flute"},
{"Pipe","Blow Bottle"},
{"Pipe","Shakuhachi"},
{"Pipe","Whistle"},
{"Pipe","Ocarina"},
{"Synth Lead","Square Lead"},
{"Synth Lead","Sawtooth Lead"},
{"Synth Lead","Calliope Lead"},
{"Synth Lead","Chiff Lead"},
{"Synth Lead","Charang Lead"},
{"Synth Lead","Voice Lead"},
{"Synth Lead","Fifths Lead"},
{"Synth Lead","Bass Lead"},
{"Synth Pad","New Age Pad"},
{"Synth Pad","Warm Pad"},
{"Synth Pad","Polysynth Pad"},
{"Synth Pad","Choir Pad"},
{"Synth Pad","Bowed Pad"},
{"Synth Pad","Metallic Pad"},
{"Synth Pad","Halo Pad"},
{"Synth Pad","Sweep Pad"},
{"Synth Effects","Rain FX"},
{"Synth Effects","Sountrack FX"},
{"Synth Effects","Crystal FX"},
{"Synth Effects","Atmosphere FX"},
{"Synth Effects","Brightness FX"},
{"Synth Effects","Goblins FX"},
{"Synth Effects","Echoes FX"},
{"Synth Effects","Sci-fi FX"},
{"Ethnic","Sitar"},
{"Ethnic","Banjo"},
{"Ethnic","Shamisen"},
{"Ethnic","Koto"},
{"Ethnic","Kalimba"},
{"Ethnic","Bag Pipe"},
{"Ethnic","Fiddle"},
{"Ethnic","Shanai"},
{"Percussive","Tinkle Bell"},
{"Percussive","Agogo"},
{"Percussive","Steel Drums"},
{"Percussive","Woodblock"},
{"Percussive","Taiko Drums"},
{"Percussive","Melodic Tom"},
{"Percussive","Synth Drums"},
{"Percussive","Reverse Cymbal"},
{"Sound Effects","Guitar Fret Noise"},
{"Sound Effects","Breath Noise"},
{"Sound Effects","Seashore"},
{"Sound Effects","Bird Tweet"},
{"Sound Effects","Telephone Ring"},
{"Sound Effects","Helicopter"},
{"Sound Effects","Applause"},
{"Sound Effects","Gunshot"},
};

char* midi_key_from_id(int sharps, int major) {
	if (sharps>=-7&&sharps<=7&&major>=0&&major<=1)
		return midi_key_lookuptable[sharps+7][major];
	else
		return midi_key_lookuptable[7][0];
}

char* midi_instrument_from_id(int id) {
	if (id<=0&&id<=127)
		return midi_instrument_lookuptable[id][1];
	else
		return midi_instrument_lookuptable[0][1];
}

void midi_id_from_key(char* name, int *sharps, int *major) {
	for (*sharps==-7;*sharps<=7;(*sharps)++) {
		for (*major==0;*major<=1;*major++) {
			if (strcmp(name,midi_key_lookuptable[*sharps][*major])==0) {
				return;
			}
		}
	}
	*sharps=0;
	*major=0;
}

int midi_id_from_instrument(char* name) {
	int id;
	for (id=0;id<=127;id++) {
		if (strcmp(name,midi_instrument_lookuptable[id][1]))
			return id;
	}
	return 0;
}
