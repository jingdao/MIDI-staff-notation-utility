#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifdef USE_SQLITE3
#include <sqlite3.h>
#else
#include "midi.h"
#endif
#ifndef MAX
#define MAX(x,y) (((x)>(y))?(x):(y))
#endif
#ifndef MIN
#define MIN(x,y) (((x)<(y))?(x):(y))
#endif

#define DEFAULT_BUFFER_SIZE 2048 

struct Note_struct {
	int value;
	int start;
	int end;
	int volume;
	struct Note_struct* next;
};

struct Dynamics_struct {
	int value;
	int beat;
	int subbeat;
	struct Dynamics_struct* next;
};

char* note_volume[7] = {"ppp","pp","p","mp","mf","f","ff"};
char* note_pitch[12] = {"C","C#","D","Eb","E","F","F#","G","G#","A","Bb","B"};
typedef struct Note_struct Note;
typedef struct Dynamics_struct Dynamics;
int showWarnings=1;
int buffer_size=DEFAULT_BUFFER_SIZE;

Note* AddNote(int value,int start,int volume) {
	Note* n = (Note*)malloc(sizeof(Note));
	n->value=value;
	n->start=start;
	n->end=-1;
	n->volume=(volume-15)/16; 
	n->next=NULL;
	return n;	
}

void DeleteNotes(Note* note_list) {
	Note* n;
	while (note_list) {
		n=note_list;
		note_list=note_list->next;
		free(n);
	}
}

Dynamics* AddDynamics(int value,int beat,int subbeat) {
	Dynamics* n = (Dynamics*)malloc(sizeof(Dynamics));
	n->value=value;
	n->beat=beat;
	n->subbeat=subbeat;
	n->next=NULL;
	return n;	
}

void DeleteDynamics(Dynamics* dynamics_list) {
	Dynamics* n;
	while (dynamics_list) {
		n=dynamics_list;
		dynamics_list=dynamics_list->next;
		free(n);
	}
}

int byte2int(signed char* c,int numBytes) {
	int i=0;
	while (numBytes>0) {
		i = (i<<8) + *c;
		c++;
		numBytes--;
	}
	return i;
}

unsigned int byte2uint(unsigned char* c,int numBytes) {
	unsigned int i=0;
	while (numBytes>0) {
		i = (i<<8) + *c;
		c++;
		numBytes--;
	}
	return i;
}

int varbyte2int(unsigned char* c,int* i) {
	*i=0;
	int numBytes=0;
	while (1) {
		*i = (*i<<7) + (*c&0x7F);
		numBytes++;
		if (!(*c&0x80)) break;
		c++;
	}
	return numBytes;
}

void wordAlign(int numLines, char **lines, char** buffer, int bufferSize,char separator) {
	int i;
	int linesParsed=0;
	int *offset = malloc(numLines*sizeof(int));
	int *bytesRead = malloc(numLines*sizeof(int));
	int *bytesWritten = malloc(numLines*sizeof(int));
	for (i=0;i<numLines;i++) {
		offset[i]=0;
		bytesRead[i]=0;
		bytesWritten[i]=0;
	}
	while (1) {
		int maxWidth=0;
		for (i=0;i<numLines;i++) {
			if (*(lines[i]+bytesRead[i])=='\0') continue;
			do { offset[i]++; }
			while (*(lines[i]+bytesRead[i]+offset[i])&&*(lines[i]+bytesRead[i]+offset[i])!=separator);
			if (offset[i]>maxWidth) maxWidth=offset[i];
		}
//		if (separator=='|') printf("max: %d\n",maxWidth);
		for (i=0;i<numLines;i++) {
			if (maxWidth+bytesWritten[i]>bufferSize-1) {
				if (showWarnings) {
					showWarnings=0;
					printf("Warning: buffer write incomplete (bytes: %d bufferLoad: %d)\n",maxWidth,bytesWritten[i]);
					exit(1);
				}
			} else {
				if (offset[i]>0) {
					strncpy(buffer[i]+bytesWritten[i],lines[i]+bytesRead[i],offset[i]);
					memset(buffer[i]+bytesWritten[i]+offset[i],' ',maxWidth-offset[i]);
				} else if (separator==' ') {
					continue;
				} else {
					*(buffer[i]+bytesWritten[i])=separator;
					memset(buffer[i]+bytesWritten[i]+1,' ',maxWidth-1);
				}
				bytesRead[i]+=offset[i];
				bytesWritten[i]+=maxWidth;
				offset[i]=0;
				buffer[i][bytesWritten[i]]='\0';
			}
			if (*(lines[i]+bytesRead[i])=='\0') {
				linesParsed|=(1<<i);	
				if (linesParsed==(1<<numLines)-1) { 
					free(offset); 
					free(bytesRead);
					free(bytesWritten);
					return;
				}
			}
		}	
	}
	
}

void writeToBuffer(char* dest, char* src, int *bufferLoad, char* format, ... ) {
	va_list args;
	va_start(args,format);
	int bytesWritten = vsnprintf(src,buffer_size-*bufferLoad,format,args);
	if (bytesWritten<0||bytesWritten>=buffer_size-*bufferLoad) {
		if (showWarnings) {
			showWarnings=0;
			printf("Warning: buffer write of %s incomplete (bytes: %d bufferLoad: %d)\n",format,bytesWritten,*bufferLoad);
			exit(1);
		}
	} else {
		*bufferLoad+=bytesWritten;
		strcat(dest,src);
	}
	va_end(args);
}

void printUsage() {
	printf("Usage: ./mid2stf [-t timeDivision] [-w wrapChars] [-b bufferSize] <src> <dest>\n");
}

int main(int argc,char* argv[]) {

	//parse command line options
	char *src=NULL,*dest=NULL;
	int i=0,j=0;
	int forced_timeDivision=0,wrapChars=0;
	for (i=1;i<argc;i++) {
		if (i!=argc-1) {
			if (strcmp(argv[i],"-t")==0) {
				forced_timeDivision=atoi(argv[i+1]);
				i++;
			} else if (strcmp(argv[i],"-w")==0) {
				wrapChars=atoi(argv[i+1]);
				i++;
			} else if (strcmp(argv[i],"-b")==0) {
				buffer_size=atoi(argv[i+1]);
				i++;
			} else if (!src) src=argv[i];
			else if (!dest) dest=argv[i];
		} else if (!src) src=argv[i];
		else if (!dest) dest=argv[i];

	}
	if (!src||!dest) {
		printUsage();
		return 1;
	}

	//initialize variables
	FILE *in,*out;
	in=fopen(src,"rb");
	if (!in) { printf("Cannot open file %s\n",src); return 1;}
	out=fopen(dest,"w");
	if (!out) { printf("Cannot open file %s\n",dest); return 1;}
#ifdef USE_SQLITE3
	sqlite3 *db;
	int rc = sqlite3_open("midi.db",&db);
	if (rc) {
		printf("Cannot open database: %s\n",sqlite3_errmsg(db)); 
		return 1;
	}
	sqlite3_stmt *key_stmt,*inst_stmt;
	sqlite3_prepare(db,"SELECT name FROM key WHERE sharps=? AND major=?",-1,&key_stmt,NULL);
	sqlite3_prepare(db,"SELECT name FROM instruments WHERE id=?",-1,&inst_stmt,NULL);
#endif
	fseek(in,0,SEEK_END);
	unsigned int fileSize=ftell(in);
	rewind(in);
	printf("Reading from %s (%d bytes) ...\n",src,fileSize);
	unsigned char* data=(unsigned char*)malloc(fileSize);
	if(!data){
		printf("Memory allocation error\n");
		return 1;
	}
	fread(data,1,fileSize,in);
	if (strncmp("MThd",data,4)!=0) {
		printf("Invalid MIDI file\n");
		return 1;
	}
	int format = byte2uint(data+8,2);
	int numTracks = byte2uint(data+10,2);
	int timeDivision = byte2uint(data+12,2);
	if (forced_timeDivision>0) timeDivision=forced_timeDivision;
	printf("MIDI format: %d, tracks: %d, time division: %d\n\n",format,numTracks,timeDivision);
	
	unsigned char* data_p = data+14;
	unsigned char* event_p;
	int tempo=120,time_sig_numer=4,time_sig_denom=4,key_sig_sharps=0,key_sig_major=0;
	int anticipation=0, beats_per_bar=4, maxBars=-1;
	int bar,beat;
	char stringBuffer[buffer_size];
	Note *note_list=NULL, *current_note=NULL, *played_note=NULL;
	int numStaffs = (numTracks-1)*2+1;
	char ** trackBuffer = malloc(numStaffs*sizeof(char*));
	char ** alignBuffer = malloc(numStaffs*sizeof(char*));
	Dynamics **dynamics_list = malloc((numTracks-1)*sizeof(Dynamics*));
	Dynamics *current_dynamics=NULL;
	for (i=0;i<numStaffs;i++) {
//		trackBuffer[i] = malloc(buffer_size);
		trackBuffer[i] = calloc(buffer_size,1);
		alignBuffer[i] = malloc(buffer_size);
//		trackBuffer[i][0] = '\0';
	}
	for (i=0;i<numTracks-1;i++) {
		dynamics_list[i]=NULL;
	}
	
	for (i=0;i<numTracks;i++) {
		data_p+=4;
		int chunkSize=byte2uint(data_p,4);
		data_p+=4;
		event_p=data_p;
		int delta,etype,channel,param1,param2;
		int timer=0,octave=-1,volume=-1;
		int bufferLoad = 0, altBufferLoad = 0;
		bar=beat=0;
		if (i==0) writeToBuffer(trackBuffer[0],stringBuffer,&bufferLoad,"|");
		else writeToBuffer(trackBuffer[i*2-1],stringBuffer,&bufferLoad,"|");
		//parse MIDI events
		while (1) {
			if (event_p-data_p>=chunkSize) break;
			event_p+=varbyte2int(event_p,&delta);
			etype=byte2uint(event_p,1);
			channel=etype&0xF;
			etype=etype>>4;
			event_p++;
			param1=byte2uint(event_p,1);
			event_p++;
			if (etype!=0xC&&etype!=0xD) {
				param2=byte2uint(event_p,1);
				event_p++;	
			}
			if (etype==0x8) { //Note off event
				timer+=delta;
				Note* n = current_note;
				while (n) {
					if (n->end<0&&n->value==param1) {
						n->end=timer;
						break;
					}
					n=n->next;
				}
				while (current_note->next&&current_note->end>=0)
					current_note=current_note->next;
			} else if (etype==0x9) { //Note on event
				timer+=delta;
				if (note_list) {
					played_note=AddNote(param1,timer,param2);
					if (note_list->start==played_note->start&&note_list->value>played_note->value) {
						played_note->next=note_list;
						current_note=note_list=played_note;
					} else {
						Note* n = current_note;
						while (n->next&&(n->next->start!=played_note->start||n->next->value<played_note->value)) {
							n=n->next;
						}
						played_note->next=n->next;
						n->next=played_note;
					}
				} else current_note = note_list = AddNote(param1,timer,param2);
			} else if (etype==0xC) { //Instrument event
				if (channel==9) writeToBuffer(trackBuffer[i*2],stringBuffer,&altBufferLoad,"|Drums|");
				else {
#ifdef USE_SQLITE3
					sqlite3_bind_int(inst_stmt,1,param1+1);
					sqlite3_step(inst_stmt);
					const unsigned char* inst_name = sqlite3_column_text(inst_stmt,0);
					writeToBuffer(trackBuffer[i*2],stringBuffer,&altBufferLoad,"|%s|",inst_name);
					sqlite3_reset(inst_stmt);
#else
					writeToBuffer(trackBuffer[i*2],stringBuffer,&altBufferLoad,"|%s|",midi_instrument_from_id(param1));
#endif
				}
			} else if (etype==0xF) {
				if (param1==0x2F) { //End of track
				} else if (param1==0x51) { //Tempo
					tempo=60*1000000/byte2uint(event_p,3);
					timer+=delta;
					while(timer>=timeDivision*beats_per_bar*bar) {
						writeToBuffer(trackBuffer[0],stringBuffer,&bufferLoad,"| ");
						bar++;
					}
					writeToBuffer(trackBuffer[0],stringBuffer,&bufferLoad,"!=%d ",tempo);
				} else if (param1==0x58) { //Time signature
					time_sig_numer = byte2uint(event_p,1);
					time_sig_denom = byte2uint(event_p+1,1);
					if (time_sig_denom==3&&(time_sig_numer==6||time_sig_numer==9||time_sig_numer==12)) beats_per_bar=time_sig_numer/3;
					else beats_per_bar=time_sig_numer;
//					fprintf(out,"time %d/%d\n",time_sig_numer,1<<time_sig_denom);
					timer+=delta;
					while(timer>=timeDivision*beats_per_bar*bar) {
						writeToBuffer(trackBuffer[0],stringBuffer,&bufferLoad,"| ");
						bar++;
					}
					writeToBuffer(trackBuffer[0],stringBuffer,&bufferLoad,"t=%d/%d ",time_sig_numer,1<<time_sig_denom);
				} else if (param1==0x59) { //Key signature
					key_sig_sharps = byte2int(event_p,1);
					key_sig_major = byte2uint(event_p+1,1);
					timer+=delta;
					while(timer>=timeDivision*beats_per_bar*bar) {
						writeToBuffer(trackBuffer[0],stringBuffer,&bufferLoad,"| ");
						bar++;
					}
#ifdef USE_SQLITE3
					sqlite3_bind_int(key_stmt,1,key_sig_sharps);
					sqlite3_bind_int(key_stmt,2,key_sig_major);
					sqlite3_step(key_stmt);
					const unsigned char* key_name = sqlite3_column_text(key_stmt,0);
//					fprintf(out,"key %s\n",key_name);
					writeToBuffer(trackBuffer[0],stringBuffer,&bufferLoad,"(%s) ",key_name);
					sqlite3_reset(key_stmt);
#else
//					fprintf(out,"key %s\n",midi_key_from_id(key_sig_sharps,key_sig_major));
					writeToBuffer(trackBuffer[0],stringBuffer,&bufferLoad,"(%s) ",midi_key_from_id(key_sig_sharps,key_sig_major));
#endif
				}
				event_p+=param2;
			}
		}
		data_p+=chunkSize;
		timer=0;

		//Finished parsing, now write out notes as text to buffer
		current_note=note_list;
		while(current_note) {
			if (bar==0&&beat==0) {
				writeToBuffer(trackBuffer[i*2-1],stringBuffer,&bufferLoad,"|");
			}
			if (timer+timeDivision<=current_note->start) { //pause
				writeToBuffer(trackBuffer[i*2-1],stringBuffer,&bufferLoad,".");
			} else {
				//determine how many notes each beat is divided into
				played_note = current_note;
				int division=timeDivision;
				while (played_note&&timer+timeDivision>played_note->start) {
					if (played_note->end>timer) {
						division=MIN(division,MIN(timer+timeDivision,played_note->end)-MAX(played_note->start,timer));
						if (division<=0&&showWarnings) {
							showWarnings=0;
							printf("Warning: nonpositive time division %d %d %d %d %d\n",division,timer,played_note->value,played_note->start,played_note->end);
							division=timeDivision;
						}
					}
					played_note=played_note->next;
				}
				j=timer;
				while (current_note&&j<timer+timeDivision) {
					if (j<current_note->start) { //pause
						writeToBuffer(trackBuffer[i*2-1],stringBuffer,&bufferLoad,".");
						j+=division;
						continue;
					}
					if (current_note->next&&current_note->end > current_note->next->start) { //multiple notes
						writeToBuffer(trackBuffer[i*2-1],stringBuffer,&bufferLoad,"(");
						Note* n = current_note;
						while (n&&n->start<j+division&&n!=played_note) {
							if (n->end<=j) {
								writeToBuffer(trackBuffer[i*2-1],stringBuffer,&bufferLoad,".");
							} else if (j>n->start)
								writeToBuffer(trackBuffer[i*2-1],stringBuffer,&bufferLoad,"-");
							else {
								writeToBuffer(trackBuffer[i*2-1],stringBuffer,&bufferLoad,"%s",note_pitch[n->value%12]);
								if (n->value/12!=octave) {
									writeToBuffer(trackBuffer[i*2-1],stringBuffer,&bufferLoad,"%d",n->value/12);
									octave=n->value/12;
								}
								if (n->volume!=volume) {
									if (!dynamics_list[i-1]) current_dynamics=dynamics_list[i-1]=AddDynamics(n->volume,bar*beats_per_bar+beat,(j-timer)/division);
									else {
										current_dynamics->next=AddDynamics(n->volume,bar*beats_per_bar+beat,(j-timer)/division);
										current_dynamics=current_dynamics->next;
									} 
								}
								volume=current_note->volume;
							} 
							n=n->next;
						}
						writeToBuffer(trackBuffer[i*2-1],stringBuffer,&bufferLoad,")");
					} else {
						if (j>current_note->start)
							writeToBuffer(trackBuffer[i*2-1],stringBuffer,&bufferLoad,"-");
						else {
							writeToBuffer(trackBuffer[i*2-1],stringBuffer,&bufferLoad,"%s",note_pitch[current_note->value%12]);
							if (current_note->value/12!=octave) {
								writeToBuffer(trackBuffer[i*2-1],stringBuffer,&bufferLoad,"%d",current_note->value/12);
								octave=current_note->value/12;
							}
							if (current_note->volume!=volume) {
								if (!dynamics_list[i-1]) current_dynamics=dynamics_list[i-1]=AddDynamics(current_note->volume,bar*beats_per_bar+beat,(j-timer)/division);
								else {
									current_dynamics->next=AddDynamics(current_note->volume,bar*beats_per_bar+beat,(j-timer)/division);
									current_dynamics=current_dynamics->next;
								} 
								volume=current_note->volume;
							}
						}
					}
					j+=division;
					while (current_note&&j>=current_note->end)
						current_note=current_note->next;
				}
			}				
			//add space between beats
			writeToBuffer(trackBuffer[i*2-1],stringBuffer,&bufferLoad," ");
			beat++;
			timer+=timeDivision;
			if (beat==beats_per_bar) {
				//add barlines
				writeToBuffer(trackBuffer[i*2-1],stringBuffer,&bufferLoad,"|");
				beat=0;
				bar++;
			}
		}
		DeleteNotes(note_list);
		played_note = current_note = note_list = NULL;
		maxBars=MAX(maxBars,bar);
	}
	
	//insert bar numbers in meta track
	j=0;
	bar=0;
	for (i=0;i<buffer_size;i++) {
		char c = *(trackBuffer[0]+i);
		if (!c) {
			while (bar<=maxBars) {
				j+=snprintf(&(stringBuffer[j]),buffer_size-j,"|%d",bar);
				bar++;
			} 
			break;
		} else if (c=='|'){
			j+=snprintf(&(stringBuffer[j]),buffer_size-j,"|%d",bar);
			bar++;
		} else stringBuffer[j++] = c;
	}
	strcpy(trackBuffer[0],stringBuffer);
	//make every track have the same number of bar lines
	for (i=1;i<numStaffs;i++) {
		char *c = trackBuffer[i]+1;
		bar=0,beat=beats_per_bar,j=0;
		while (*c) {
			if (*c=='|') {
				bar++;
				beat=0;
			} else if (*c==' '&&*(c+1)!=' ') beat++;
			c++;
		}
		while(j<maxBars-bar+1&&c<trackBuffer[i]+buffer_size-1) {
			if (i%2&&beat<beats_per_bar) {
				*c++ = '.';
				*c++ = ' ';
				beat++;
			} else {
				*c++ = '|';
				beat=0;
				j++;
			}
		 }	
	}
	//align buffers so that bars and beats line up
	char **musicTracks = malloc((numTracks-1)*sizeof(char*));
	for (i=0;i<numTracks-1;i++)
		musicTracks[i]=trackBuffer[i*2+1];
	wordAlign(numTracks-1,musicTracks,alignBuffer,buffer_size,' ');
	for (i=0;i<numTracks-1;i++)
		memcpy(trackBuffer[i*2+1],alignBuffer[i],buffer_size);
	wordAlign(numStaffs,trackBuffer,alignBuffer,buffer_size,'|');
	//insert dynamics
	for (i=0;i<numTracks-1;i++) {
		beat=0;
		current_dynamics=dynamics_list[i];
		char *c = alignBuffer[i*2+1];
		while (*c&&current_dynamics) {
			if (*c==' '&&*(c+1)!=' ') {
				beat++;
				if (beat==current_dynamics->beat) {
					while (!(*c>='A'&&*c<='G'||*c=='(')) c++;
					strncpy(c-alignBuffer[i*2+1]+alignBuffer[i*2+2],note_volume[current_dynamics->value],strlen(note_volume[current_dynamics->value]));
					current_dynamics=current_dynamics->next;
				}
			}
			c++;
		}
		DeleteDynamics(dynamics_list[i]);
	}
	

	//write out buffers to file
	if (wrapChars<=0) {
		for (i=0;i<numStaffs;i++) {
			fprintf(out,"%s\n",alignBuffer[i]);
			//fprintf(out,"%s\n",trackBuffer[i]);
		}
	} else {
		//find location of bar lines to wrap text
		char *c = alignBuffer[0];
		int lineStart=0,lineEnd=0,numChars=0;
		while (*c++) {
			if (numChars==wrapChars-1) {
//				printf("%d %d\n",numChars,lineEnd-lineStart);
				if (lineStart>lineEnd) lineEnd=lineStart+numChars-1; 
				for (i=0;i<numStaffs;i++) {
					fwrite(alignBuffer[i]+lineStart,1,lineEnd-lineStart+1,out);
					fprintf(out,"%*s",wrapChars-lineEnd+lineStart,"|\n");
				}
				fputc('\n',out);
//				fputc('\n',out);
				numChars-=(lineEnd-lineStart);
				lineStart=lineEnd+1;
//				lineEnd=lineStart+numChars;
			}
			else if (*c=='|') {
				lineEnd=lineStart+numChars;
				numChars++;
			}
			else numChars++;
		}
	}
	
	for (i=0;i<numStaffs;i++) {
		free(trackBuffer[i]);
		free(alignBuffer[i]);
	}
	free(dynamics_list);
	free(musicTracks);
	free(trackBuffer);
	free(alignBuffer);
	free(data);
#ifdef USE_SQLITE3
	sqlite3_finalize(key_stmt);
	sqlite3_finalize(inst_stmt);
	sqlite3_close(db);
#endif
	fclose(in);
	fclose(out);
}
