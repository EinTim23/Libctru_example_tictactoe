#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio> 
#include <math.h>
#include <3ds.h>
#include <iostream>
#include "icon_bin.h"
using namespace std;
void* buffer;
u32 size;
int choice;
int player = 1,game_i;
#define STACKSIZE (4 * 1024)
#define SAMPLERATE 48000
#define SAMPLESPERBUF 4096
#define BYTESPERSAMPLE 4

void fill_buffer(void *audioBuffer, size_t size, int16_t* buffer) {
    u32 *dest = (u32*)audioBuffer;
    
    for (int i=0; i<size; i++) {
        
        int16_t sample = buffer[i];
        
        dest[i] = (sample<<16) | (sample & 0xffff);
    }
    
    DSP_FlushDataCache(audioBuffer, size);
}

volatile bool runThreads = true;
void threadMain(void *arg)
{
    ndspWaveBuf waveBuf[2];
    u32 *audioBuffer = (u32*)linearAlloc(SAMPLESPERBUF*BYTESPERSAMPLE*2);
    bool fillBlock = false;
    ndspInit();
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
    ndspChnSetRate(0, SAMPLERATE);
    ndspChnSetFormat(0, NDSP_FORMAT_STEREO_PCM16);
    
    memset(waveBuf,0,sizeof(waveBuf));
    waveBuf[0].data_vaddr = &audioBuffer[0];
    waveBuf[0].nsamples = SAMPLESPERBUF;
    waveBuf[1].data_vaddr = &audioBuffer[SAMPLESPERBUF];
    waveBuf[1].nsamples = SAMPLESPERBUF;
    ndspChnWaveBufAdd(0, &waveBuf[0]);
    ndspChnWaveBufAdd(0, &waveBuf[1]);
    FILE *file = fopen("romfs:/templeos.bin", "rb");
    
    off_t file_size;
    char* file_buffer;
    
    if(file == NULL) {
        printf("Error - the file could not be opened. %s\n", strerror(errno));
    } else {
        
        fseek(file, 0, SEEK_END);
        file_size = ftell(file);
        
        fseek(file, 0, SEEK_SET);
        file_buffer = (char*)malloc(file_size);
        
        off_t bytesRead = fread(file_buffer, 1, file_size, file);
        
        if(ferror(file) || file_buffer == NULL) {
            printf("Error - the file could not be loaded. %s\n", strerror(errno));
        }
        
        fclose(file);
        
    }
    
    unsigned long position = 0;
    
    int16_t* file_audio_buffer = (int16_t*)malloc(SAMPLESPERBUF * sizeof(int16_t));
    
    if(file_audio_buffer == NULL) {
        printf("Error - the audio buffer could not be allocated.\n");
    }
    
    //printf("Loaded audio file!\n");
	while (runThreads)
    {
      if (waveBuf[fillBlock].status == NDSP_WBUF_DONE) {
            for(unsigned int i=0; i < SAMPLESPERBUF; i++) {
                file_audio_buffer[i] = ( file_buffer[position * 2] | file_buffer[position * 2 + 1] << 8);
                position = (position + 1) % (file_size / 2);
            }
            
            fill_buffer(waveBuf[fillBlock].data_pcm16, waveBuf[fillBlock].nsamples, file_audio_buffer);
            
            ndspChnWaveBufAdd(0, &waveBuf[fillBlock]);
            
            fillBlock = !fillBlock;
        }
        
        svcSleepThread(10 * 1000000);
    }
    ndspExit();
    linearFree(audioBuffer);
    free(file_buffer);
    printf("Freeing audio buffer");
    free(file_audio_buffer);
}
char square[10] = {'o','1','2','3','4','5','6','7','8','9'};
int checkwin()
{
    if (square[1] == square[2] && square[2] == square[3])

        return 1;
    else if (square[4] == square[5] && square[5] == square[6])

        return 1;
    else if (square[7] == square[8] && square[8] == square[9])

        return 1;
    else if (square[1] == square[4] && square[4] == square[7])

        return 1;
    else if (square[2] == square[5] && square[5] == square[8])

        return 1;
    else if (square[3] == square[6] && square[6] == square[9])

        return 1;
    else if (square[1] == square[5] && square[5] == square[9])

        return 1;
    else if (square[3] == square[5] && square[5] == square[7])

        return 1;
    else if (square[1] != '1' && square[2] != '2' && square[3] != '3' 
                    && square[4] != '4' && square[5] != '5' && square[6] != '6' 
                  && square[7] != '7' && square[8] != '8' && square[9] != '9')

        return 0;
    else
        return -1;
}



void board()
{
    cout << "\n\n\tTic Tac Toe\n\n";

    cout << endl;

    cout << "     |     |     " << endl;
    cout << "  " << square[1] << "  |  " << square[2] << "  |  " << square[3] << endl;

    cout << "_____|_____|_____" << endl;
    cout << "     |     |     " << endl;

    cout << "  " << square[4] << "  |  " << square[5] << "  |  " << square[6] << endl;

    cout << "_____|_____|_____" << endl;
    cout << "     |     |     " << endl;

    cout << "  " << square[7] << "  |  " << square[8] << "  |  " << square[9] << endl;

    cout << "     |     |     " << endl << endl;
}
void keyboard(){
	static SwkbdState swkbd;
	static char buf[60];
	std::string placeholder = "Wähle ein Feld.";
	swkbdInit(&swkbd, SWKBD_TYPE_WESTERN, 1, -1);
	swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, SWKBD_FILTER_DIGITS | SWKBD_FILTER_AT | SWKBD_FILTER_PERCENT | SWKBD_FILTER_BACKSLASH | SWKBD_FILTER_PROFANITY, 2);
	swkbdSetFeatures(&swkbd, SWKBD_MULTILINE);
	swkbdSetHintText(&swkbd, placeholder.c_str());
	swkbdInputText(&swkbd, buf, sizeof(buf));
	choice = atoi(buf);
}
void menu(){
	std::cout << "Tic Tac Toe 3ds Edition\n\n";
	std::cout << "[A] Play [B] Exit\n";
    
}

int main(int argc, char* argv[])
{
	gfxInitDefault();
    srvInit();
	aptInit();
	hidInit();
    csndInit();
    romfsInit();
    consoleInit(GFX_BOTTOM, NULL);
	consoleInit(GFX_TOP, NULL);
	gfxSetDoubleBuffering(GFX_BOTTOM, false);
	u8* fb = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	memcpy(fb, (u8*)icon_bin, icon_bin_size);
    s32 prio = 0;
	svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
    Thread audiothread = threadCreate(threadMain, (void*)((1)*250), STACKSIZE, prio-1, -2, false);
	char mark;
	bool playing = false;
	menu();
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();
		
		u32 kDown = hidKeysDown();
		if (kDown & KEY_B){
            game_i = 1;
            playing = false;
			break; 
		}
		if (kDown & KEY_A){
			printf("\x1b[2J");
			playing = true;
		}
		if(playing){
					do
    {
        board();
        player=(player%2)?1:2;

        keyboard();
		printf("\x1b[2J");
        mark=(player == 1) ? 'X' : 'O';

        if (choice == 1 && square[1] == '1')

            square[1] = mark;
        else if (choice == 2 && square[2] == '2')

            square[2] = mark;
        else if (choice == 3 && square[3] == '3')

            square[3] = mark;
        else if (choice == 4 && square[4] == '4')

            square[4] = mark;
        else if (choice == 5 && square[5] == '5')

            square[5] = mark;
        else if (choice == 6 && square[6] == '6')

            square[6] = mark;
        else if (choice == 7 && square[7] == '7')

            square[7] = mark;
        else if (choice == 8 && square[8] == '8')

            square[8] = mark;
        else if (choice == 9 && square[9] == '9')

            square[9] = mark;
        else
        {
            cout<< "Dieser Zug ist nicht moeglich!";

            player--;
            cin.ignore();
            cin.get();
        }
        game_i=checkwin();

        player++;
    }while(game_i==-1);
    board();
    if(game_i==1){
		printf("\x1b[2J");
        cout<<"==>\aSpieler "<<--player<<" hat gewonnen. \n";
		square[0] = 'o';
		square[1] = '1';
		square[2] = '2';
		square[3] = '3';
		square[4] = '4';
		square[5] = '5';
		square[6] = '6';
		square[7] = '7';
		square[8] = '8';
		square[9] = '9';
		menu();
		playing = false;
	}
    else{
		printf("\x1b[2J");
        cout<<"==>\aUnentschieden!\n";
		square[0] = 'o';
		square[1] = '1';
		square[2] = '2';
		square[3] = '3';
		square[4] = '4';
		square[5] = '5';
		square[6] = '6';
		square[7] = '7';
		square[8] = '8';
		square[9] = '9';
		menu();
		playing = false;
	}

    cin.ignore();
    cin.get();

		}

		if (kDown & KEY_START){
            game_i = 1;
            playing = false;
			break; 
        }
	}
    runThreads = false;
    threadJoin(audiothread, U64_MAX);
	threadFree(audiothread);
	gfxExit();
    romfsExit();
	return 0;
}
