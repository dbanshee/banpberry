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

// Devices
#define RANDOM_DEVICE         "/dev/random"
#define URANDOM_DEVICE        "/dev/urandom"
#define SPI_DEVICE            "/dev/spidev0.0"

// System Params
#define NUM_LEDS              25
#define NUM_BYTES_LED         3

// Colors
#define NUM_PRED_COLORS       144
#define BLACK                 "x00\x00\x00"
#define AQUA                  "x00\xff\xff"
#define AQUAMARINE            "x7f\xff\xd4"
#define AZURE                 "xf0\xff\xff"
#define BEIGE                 "xf5\xf5\xdc"
#define BISQUE                "xff\xe4\xc4"
#define BLANCHEDALMOND        "xff\xeb\xcd"
#define BLUE                  "x00\x00\xff"
#define BLUEVIOLET            "x8a\x2b\xe2"
#define BROWN                 "xa5\x2a\x2a"
#define BURLYWOOD             "xde\xb8\x87"
#define CADETBLUE             "x5f\x9e\xa0"
#define CHARTREUSE            "x7f\xff\x00"
#define CHOCOLATE             "xd2\x69\x1e"
#define CORAL                 "xff\x7f\x50"
#define CORNFLOWERBLUE        "x64\x95\xed"
#define CORNSILK              "xff\xf8\xdc"
#define CRIMSON               "xdc\x14\x3c"
#define CYAN                  "x00\xff\xff"
#define DARKBLUE              "x00\x00\x8b"
#define DARKCYAN              "x00\x8b\x8b"
#define DARKGOLDENROD         "xb8\x86\x0b"
#define DARKGRAY              "xa9\xa9\xa9"
#define DARKGREY              "xa9\xa9\xa9"
#define DARKGREEN             "x00\x64\x00"
#define DARKKHAKI             "xbd\xb7\x6b"
#define DARKMAGENTA           "x8b\x00\x8b"
#define DARKOLIVEGREEN        "x55\x6b\x2f"
#define DARKORANGE            "xff\x8c\x00"
#define DARKORCHID            "x99\x32\xcc"
#define DARKRED               "x8b\x00\x00"
#define DARKSALMON            "xe9\x96\x7a"
#define DARKSEAGREEN          "x8f\xbc\x8f"
#define DARKSLATEBLUE         "x48\x3d\x8b"
#define DARKSLATEGRAY         "x2f\x4f\x4f"
#define DARKSLATEGREY         "x2f\x4f\x4f"
#define DARKTURQUOISE         "x00\xce\xd1"
#define DARKVIOLET            "x94\x00\xd3"
#define DEEPPINK              "xff\x14\x93"
#define DEEPSKYBLUE           "x00\xbf\xff"
#define DIMGRAY               "x69\x69\x69"
#define DIMGREY               "x69\x69\x69"
#define DODGERBLUE            "x1e\x90\xff"
#define FIREBRICK             "xb2\x22\x22"
#define FLORALWHITE           "xff\xfa\xf0"
#define FORESTGREEN           "x22\x8b\x22"
#define FUCHSIA               "xff\x00\xff"
#define GAINSBORO             "xdc\xdc\xdc"
#define GHOSTWHITE            "xf8\xf8\xff"
#define GOLD                  "xff\xd7\x00"
#define GOLDENROD             "xda\xa5\x20"
#define GRAY                  "x80\x80\x80"
#define GREY                  "x80\x80\x80"
#define GREEN                 "x00\x80\x00"
#define GREENYELLOW           "xad\xff\x2f"
#define HONEYDEW              "xf0\xff\xf0"
#define HOTPINK               "xff\x69\xb4"
#define INDIANRED             "xcd\x5c\x5c"
#define INDIGO                "x4b\x00\x82"
#define IVORY                 "xff\xff\xf0"
#define KHAKI                 "xf0\xe6\x8c"
#define LAVENDER              "xe6\xe6\xfa"
#define LAVENDERBLUSH         "xff\xf0\xf5"
#define LAWNGREEN             "x7c\xfc\x00"
#define LEMONCHIFFON          "xff\xfa\xcd"
#define LIGHTBLUE             "xad\xd8\xe6"
#define LIGHTCORAL            "xf0\x80\x80"
#define LIGHTCYAN             "xe0\xff\xff"
#define LIGHTGOLDENRODYELLOW  "xfa\xfa\xd2"
#define LIGHTGRAY             "xd3\xd3\xd3"
#define LIGHTGREY             "xd3\xd3\xd3"
#define LIGHTGREEN            "x90\xee\x90"
#define LIGHTPINK             "xff\xb6\xc1"
#define LIGHTSALMON           "xff\xa0\x7a"
#define LIGHTSEAGREEN         "x20\xb2\xaa"
#define LIGHTSKYBLUE          "x87\xce\xfa"
#define LIGHTSLATEGRAY        "x77\x88\x99"
#define LIGHTSLATEGREY        "x77\x88\x99"
#define LIGHTSTEELBLUE        "xb0\xc4\xde"
#define LIGHTYELLOW           "xff\xff\xe0"
#define LIME                  "x00\xff\x00"
#define LIMEGREEN             "x32\xcd\x32"
#define LINEN                 "xfa\xf0\xe6"
#define MAGENTA               "xff\x00\xff"
#define MAROON                "x80\x00\x00"
#define MEDIUMAQUAMARINE      "x66\xcd\xaa"
#define MEDIUMBLUE            "x00\x00\xcd"
#define MEDIUMORCHID          "xba\x55\xd3"
#define MEDIUMPURPLE          "x93\x70\xd8"
#define MEDIUMSEAGREEN        "x3c\xb3\x71"
#define MEDIUMSLATEBLUE       "x7b\x68\xee"
#define MEDIUMSPRINGGREEN     "x00\xfa\x9a"
#define MEDIUMTURQUOISE       "x48\xd1\xcc"
#define MEDIUMVIOLETRED       "xc7\x15\x85"
#define MIDNIGHTBLUE          "x19\x19\x70"
#define MINTCREAM             "xf5\xff\xfa"
#define MISTYROSE             "xff\xe4\xe1"
#define MOCCASIN              "xff\xe4\xb5"
#define NAVAJOWHITE           "xff\xde\xad"
#define NAVY                  "x00\x00\x80"
#define OLDLACE               "xfd\xf5\xe6"
#define OLIVE                 "x80\x80\x00"
#define OLIVEDRAB             "x6b\x8e\x23"
#define ORANGE                "xff\xa5\x00"
#define ORANGERED             "xff\x45\x00"
#define ORCHID                "xda\x70\xd6"
#define PALEGOLDENROD         "xee\xe8\xaa"
#define PALEGREEN             "x98\xfb\x98"
#define PALETURQUOISE         "xaf\xee\xee"
#define PALEVIOLETRED         "xd8\x70\x93"
#define PAPAYAWHIP            "xff\xef\xd5"
#define PEACHPUFF             "xff\xda\xb9"
#define PERU                  "xcd\x85\x3f"
#define PINK                  "xff\xc0\xcb"
#define PLUM                  "xdd\xa0\xdd"
#define POWDERBLUE            "xb0\xe0\xe6"
#define PURPLE                "x80\x00\x80"
#define RED                   "xff\x00\x00"
#define ROSYBROWN             "xbc\x8f\x8f"
#define ROYALBLUE             "x41\x69\xe1"
#define SADDLEBROWN           "x8b\x45\x13"
#define SALMON                "xfa\x80\x72"
#define SANDYBROWN            "xf4\xa4\x60"
#define SEAGREEN              "x2e\x8b\x57"
#define SEASHELL              "xff\xf5\xee"
#define SIENNA                "xa0\x52\x2d"
#define SILVER                "xc0\xc0\xc0"
#define SKYBLUE               "x87\xce\xeb"
#define SLATEBLUE             "x6a\x5a\xcd"
#define SLATEGRAY             "x70\x80\x90"
#define SLATEGREY             "x70\x80\x90"
#define SNOW                  "xff\xfa\xfa"
#define SPRINGGREEN           "x00\xff\x7f"
#define STEELBLUE             "x46\x82\xb4"
#define TAN                   "xd2\xb4\x8c"
#define TEAL                  "x00\x80\x80"
#define THISTLE               "xd8\xbf\xd8"
#define TOMATO                "xff\x63\x47"
#define TURQUOISE             "x40\xe0\xd0"
#define VIOLET                "xee\x82\xee"
#define WHEAT                 "xf5\xde\xb3"
#define WHITE                 "xff\xff\xff"
#define WHITESMOKE            "xf5\xf5\xf5"
#define YELLOW                "xff\xff\x00"
#define YELLOWGREEN           "x9a\xcd\x32"



// Types
typedef u_int8_t ledColor_t[NUM_BYTES_LED];
typedef u_int8_t ledArray_t[NUM_LEDS * NUM_BYTES_LED];


// Signal Handler
void signalHandler(int sigNum);

// General Functions
void delay(int secs, long nsecs);

  
// SPI Functions
void initSPI();
void closeSPI();
ssize_t spiWriteBuffer(u_int8_t *buff, size_t buffSize);

// Leds Functions
void showLeds(ledArray_t leds);
void setLedColor(ledArray_t leds, int numLed, ledColor_t color);
void clearLeds();
void fillLeds(ledArray_t leds, ledColor_t color);

// Leds Modes
void randomMode(int secDelay, long nSecDelay);
void rayMode(long raySpeed, int secInterval, ledColor_t color);
void rainbowMode(long speed);



// Constants
const u_int8_t    LED_BUFF_SIZE = NUM_LEDS * NUM_BYTES_LED;

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

void delay(int secs, long nsecs){
  
    struct timespec timeDelay;
  
    timeDelay.tv_sec  = secs;
    timeDelay.tv_nsec = nsecs;

    blog(LOG_DEBUG, "Waiting .... ");
    if(nanosleep(&timeDelay, NULL) == -1)
        blog(LOG_WARN, "Sleep not completed entirely");
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

    if((randomFd = open(RANDOM_DEVICE, O_RDONLY)) == -1)
        blog(LOG_ERROR, "Error opening %s", RANDOM_DEVICE);
    
    if((urandomFd = open(URANDOM_DEVICE, O_RDONLY)) == -1)
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
    memcpy(&leds[numLed*NUM_BYTES_LED], color, NUM_BYTES_LED);
    //leds[numLed*NUM_BYTES_LED]           = color[0];
    //leds[(numLed*NUM_BYTES_LED)+1]       = color[1];
    //leds[(numLed*NUM_BYTES_LED)+2]       = color[2];
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
        
        // Read random frame
        read(randomFd, buff, LED_BUFF_SIZE);
        
        if(strcasecmp(buff, buff2) == 0)
            blog(LOG_WARN, "Not random!");
        
        showLeds(buff);
        delay(secDelay, nSecDelay);
    }
}
    
void rayMode(long raySpeed, int secInterval, ledColor_t color){
    int             i;
    ledArray_t      buff;
        
    while(1){
        for(i = 0; i < NUM_LEDS; i++){

            // reset buff
            memset(buff, 0, LED_BUFF_SIZE);

            // Set Flash Led
            setLedColor(buff, i, color);
            
            // Send to leds
            showLeds(buff);
            
            // Wait for next led
            delay(0, raySpeed);
        }
        
        clearLeds();
        
        // Wait for next ray
        delay(secInterval, 0L);
    }
}

void rainbowMode(long speed){
    ledArray_t leds;
    
    //ledColor_t  RAINBOW [1] = { "\x00\x00\x00" };
    u_int8_t*  RAINBOW [NUM_PRED_COLORS] = { BLACK, AQUA };
    
    ledColor_t color;
    u_int8_t r, g, b;
    
    while(1){
        for(r = 0; r != 254; r++){
            for(g = 0; g != 254; g++){
                for(b = 0; b != 254; b++){
                    color[0] = r;
                    color[1] = g;
                    color[2] = b;
                    
                    // Fill entire led with single color.
                    fillLeds(leds, color);
                    showLeds(leds);

                    delay(0, speed);
                }
            }
        }
    }
}