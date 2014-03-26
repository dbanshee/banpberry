/*
 * bbLedServer.c
 * 
 * Banpberry LEd Server - Banshee 2014
 * 
 *      mail: dbanshee@gmail.com
 * 
 * Servidor de control de leds.
 */


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#include "../libs/logger.h"

#define RANDOM_DEVICE   "/dev/random"
#define URANDOM_DEVICE  "/dev/urandom"

#define SPI_DEVICE      "/dev/spidev0.0"
#define NUM_LEDS        25
#define NUM_BYTES_LED   3



// Types
typedef u_int8_t ledColor_t[NUM_BYTES_LED];
typedef u_int8_t ledArray_t[NUM_LEDS * NUM_BYTES_LED];

void initSPI();
void closeSPI();
ssize_t spiWriteBuffer(u_int8_t *buff, size_t buffSize);


void showLeds(ledArray_t leds);
void setLedColor(ledArray_t leds, int numLed, ledColor_t color);
void clearLeds();
void fillLeds(ledArray_t leds, ledColor_t color);

void randomMode(int secDelay, long nSecDelay);
void rayMode(long raySpeed, int secInterval, ledColor_t color);
void rainbowMode(long speed);

// Constants
const u_int8_t LED_BUFF_SIZE = NUM_LEDS * NUM_BYTES_LED;

// Signal Handler
void signalHandler(int sigNum);



// Global Vars
int spiFd       = -1;
int randomFd    = -1;
int urandomFd   = -1;


// Main Control Server
int main(int argc, char *argv){
    
    // Signal Handlers
    signal(SIGTERM, signalHandler);
    signal(SIGINT,  signalHandler);
    
    
    printf("-----------------------------------------\n");
    printf("-- Banpberry Led Server    \n");
    printf("--     Banshee 2014            \n");
    printf("-- Start at : %s\n", getCurrentDate());
    printf("-----------------------------------------\n\n\n");
    
    
    initSPI();
    
    if(spiFd == -1){
        blog(LOG_ERROR, "Could not open SPI device. Aborting server. :(");
        return -1;
    }
    

    // Random mode Sample
    //randomMode(2, 0L);
    
    // Ray mode sample
    //rayMode(100000000, 2, "\x00\xff\x00");
    
    // Rainbow Mode
    rainbowMode(1000000L);
    
    
    closeSPI();
    return 0;
}

// SO Signal handlers
void signalHandler(int sigNum){
    
    if(sigNum == SIGTERM)
        blog(LOG_INFO, "Signal SIGTERM received");
    else if(sigNum == SIGINT)
        blog(LOG_INFO, "Signal SIGINT received");
    else
        return;
    
    if(spiFd =! -1){
        blog(LOG_INFO, "Turning off Leds");
        clearLeds();
    }
        
    closeSPI();
    exit(0);
}

void initSPI(){
    blog(LOG_INFO, "Initializing SPI device %s", SPI_DEVICE);
    if((spiFd = open(SPI_DEVICE, O_WRONLY)) == -1)
        blog(LOG_ERROR, "Error opening SPI device : %s", SPI_DEVICE);
}

void closeSPI(){
    int res;
    
    blog(LOG_INFO, "Closing SPI device %s", SPI_DEVICE);
    if((res = close(spiFd)) == -1)
        blog(LOG_ERROR, "Error closing SPI device : %s", SPI_DEVICE);
}

void openRandom(){

    if((randomFd = open(RANDOM_DEVICE, O_WRONLY)) == -1)
        blog(LOG_ERROR, "Error opening %s", RANDOM_DEVICE);
    
    if((urandomFd = open(URANDOM_DEVICE, O_WRONLY)) == -1)
        blog(LOG_ERROR, "Error opening %s", URANDOM_DEVICE);
}

void closeRandom(){
    int res;
    
    if((res = close(randomFd)) == -1)
        blog(LOG_ERROR, "Error closing %s", RANDOM_DEVICE);
    
    if((res = close(urandomFd)) == -1)
        blog(LOG_ERROR, "Error closing %s", URANDOM_DEVICE);
}

ssize_t spiWriteBuffer(u_int8_t *buff, size_t buffSize){
    
    if(fcntl(spiFd, F_GETFL) == -1){
        blog(LOG_ERROR, "Error. Output server socket is not ready.");
        return -1;
    }   
    
    if(buffSize == -1)
        buffSize = strlen(buff);
        
    size_t nwrite = write(spiFd, buff, buffSize);
    
    if(nwrite == -1){
        blog(LOG_ERROR, "Error sending buffer.");
        return -1;
    }else{
        blog(LOG_DEBUG, "Write to SPI (%d bytes) : '%s'", nwrite, buff);
//        blog(LOG_DEBUG, "Write to SPI (%d bytes)", nwrite);
        return nwrite;
    }
}


void showLeds(ledArray_t leds){
    spiWriteBuffer(leds, LED_BUFF_SIZE);
}

void setLedColor(ledArray_t leds, int numLed, ledColor_t color){
//    memcpy(&leds[numLed*NUM_BYTES_LED], color, NUM_BYTES_LED);
    leds[numLed*NUM_BYTES_LED]           = color[0];
    leds[(numLed*NUM_BYTES_LED)+1]       = color[1];
    leds[(numLed*NUM_BYTES_LED)+2]       = color[2];
}

void fillLeds(ledArray_t leds, ledColor_t color){
    int i;
    
    for(i = 0; i < NUM_LEDS; i++)
        setLedColor(leds, i, color);
}

// Led Common
void clearLeds(){
    ledColor_t buff;
    
    // Set black frame
    memset(buff, 0, LED_BUFF_SIZE);
    
    // Send to leds
    showLeds(buff);
}

//void fillLeds()

// Led Modes
void randomMode(int secDelay, long nSecDelay){
    ledArray_t  buff;
    ledArray_t  buff2;
    struct      timespec timeDelay;
    struct      timespec timeDelayR;
    
    while(1){
        strcpy(buff2, buff);
        read(randomFd, buff, LED_BUFF_SIZE);
        
        if(strcasecmp(buff, buff2) == 0)
            blog(LOG_WARN, "Not random!");
        
        showLeds(buff);
    
        timeDelay.tv_sec        = secDelay;
        timeDelay.tv_nsec       = nSecDelay;
        
        blog(LOG_INFO, "Waiting .... ");
        if(nanosleep(&timeDelay, &timeDelayR) == -1)
            blog(LOG_WARN, "Sleep not completed entirely");
    }
}
    
void rayMode(long raySpeed, int secInterval, ledColor_t color){
    int             i;
    ledArray_t      buff;
    struct timespec rayDelay;
    struct timespec secDelay;
    
    rayDelay.tv_sec     = 0;
    rayDelay.tv_nsec    = raySpeed;
    
    secDelay.tv_sec  = secInterval;
    secDelay.tv_nsec = 0L;
    
    
    while(1){
        for(i = 0; i < NUM_LEDS; i++){

            // reset buff
            memset(buff, 0, LED_BUFF_SIZE);

            // Set Flash Led
            setLedColor(buff, i, color);
            
            // Send to leds
            showLeds(buff);
            
            // Wait for next led
            if(nanosleep(&rayDelay, NULL) == -1)
                blog(LOG_WARN, "Sleep not completed entirely");
        }
        
        clearLeds();
        
        blog(LOG_INFO, "Waiting next ray .... ");
        if(nanosleep(&secDelay, NULL) == -1)
            blog(LOG_WARN, "Sleep not completed entirely");
    }
}

void rainbowMode(long speed){
    ledArray_t leds;
    ledColor_t color;
    u_int8_t r, g, b;
    
    struct timespec speedDelay;
    
    speedDelay.tv_sec     = 0;
    speedDelay.tv_nsec    = speed;
    
    while(1){
        for(r = 0; r != 254; r++){
            for(g = 0; g != 254; g++){
                for(b = 0; b != 254; b++){
                    color[0] = r;
                    color[1] = g;
                    color[2] = b;
                    fillLeds(leds, color);

                    showLeds(leds);

                    if(nanosleep(&speedDelay, NULL) == -1)
                        blog(LOG_WARN, "Sleep not completed entirely");
                }
            }
        }
    }
}