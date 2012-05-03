/**
 * radeon-pm-gui: Power Management GUI for Radeon Graphics Cards in Linux
 * Copyright (C) 2012, Aaron Watry
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Checked DRM headers for something that looked useful, but didn't see anything
//other than possibly using the raw ioctls, which I'm not ready to do.
//#include <drm/radeon_drm.h>

//Maybe the linux hwmon/sysfs headers will work
//#include <linux/hwmon.h>
//#include <linux/hwmon-sysfs.h>

#include "pmlib.h"

pm_profile_t pm_profiles[] = {LOW, MEDIUM, HIGH, AUTO, DEFAULT, PROFILE_UNKNOWN, PROFILE_UNKNOWN+1 };
const char * const pm_profile_names[] = { "low", "medium", "high", "auto", "default", "unknown", NULL };

const pm_method_t pm_methods[] = {PROFILE, DYNPM, METHOD_UNKNOWN, METHOD_UNKNOWN+1};
const char * const pm_method_names[] = { "profile", "dynpm", "unknown", NULL };
const char* DEFAULT_DRM_DIR = "/sys/class/drm";
const char* DEFAULT_METHOD_PATH = "/device/power_method";
const char* DEFAULT_PROFILE_PATH = "/device/power_profile";

const char* DEFAULT_TEMP_PATH = "/device/hwmon/hwmon1/temp1_input";
#define TEMP_UNKNOWN 0

static char *buildPath(const char *baseDir, const char *card, const char *fileLocation);
static char *readLine(const char *fileName, char *dest, int maxLength);
static char *stripNewLine(char *input);
static int writeFile(const char* fileName, const char* contents);

static inline int methodIsValid(pm_method_t method){
    return (method >= 0 && method <= MAX_METHOD);
}

static inline int profileIsValid(pm_profile_t profile){
    return (profile >= 0 && profile <= MAX_PROFILE);
}

int setMethod(char *card, pm_method_t method){
    const char *methodStr = NULL;
    int retVal = PM_FALSE;
    
    if (!methodIsValid(method) || card == NULL)
        return retVal;
        
    methodStr = pm_method_names[method];
    
    char *fileName = buildPath(DEFAULT_DRM_DIR, card, DEFAULT_METHOD_PATH);
    if (fileName == NULL){
        return retVal;
    }
    
    //Open file for writing, write value, close and flush file
    retVal = writeFile(fileName, methodStr);
    free(fileName);
    return retVal;
}

int setProfile(char *card, pm_profile_t profile){
    const char *profileStr = NULL;
    int retVal = PM_FALSE;

    if (!profileIsValid(profile) || card == NULL)
        return retVal;

    profileStr = pm_method_names[profile];
    
    char *fileName = buildPath(DEFAULT_DRM_DIR, card, DEFAULT_PROFILE_PATH);
    if (fileName == NULL){
        return retVal;
    }
    
    //Open file for writing, write value, close and flush file
    retVal = writeFile(fileName, profileStr);
    free(fileName);
    return retVal;

}

pm_method_t getMethod(char *card){
    pm_method_t retVal = METHOD_UNKNOWN;
    int methodStrLen = 20;
    char methodStr[20];
    FILE *fp;
    methodStr[0] = '\0';
    
    if (card == NULL){
        return METHOD_UNKNOWN;
    }
    
    //Build the path for the method file
    char *fileName = buildPath(DEFAULT_DRM_DIR, card, DEFAULT_METHOD_PATH);
    if (fileName == NULL){
        return METHOD_UNKNOWN;
    }
    
    if (stripNewLine(readLine(fileName, methodStr, methodStrLen)) != NULL){
        int idx = 0;
        while (pm_method_names[idx] != NULL){
            if (strstr(methodStr, pm_method_names[idx])){
                retVal = idx;
                break;
            }
            idx++;
        }
    }
    
    free(fileName);
    
    return retVal;
}

pm_profile_t getProfile(char *card){
    pm_profile_t retVal = PROFILE_UNKNOWN;
    int profileStrLen = 20;
    char profileStr[20];
    char *fileName;
    profileStr[0] = '\0';
    
    if (card == NULL){
        g_printerr("Cannot get profile for a null card\n");
        return PROFILE_UNKNOWN;
    }
    
    if (getMethod(card) != PROFILE){
        return PROFILE_UNKNOWN;
    }
    
    //Build the path for the method file
    fileName = buildPath(DEFAULT_DRM_DIR, card, DEFAULT_PROFILE_PATH);
    if (fileName == NULL){
        return PROFILE_UNKNOWN;
    }
    
    if (stripNewLine(readLine(fileName, profileStr, profileStrLen)) != NULL){
        int idx = 0;
        while (pm_profile_names[idx] != NULL){
            if (strstr(profileStr, pm_profile_names[idx])){
                retVal = idx;
                break;
            }
            idx++;
        }
    }

    free(fileName);
    return retVal;    
}

int getTemperature(char *card){
    int retVal = TEMP_UNKNOWN;
    int tempStrLen = 20;
    char tempStr[20];
    FILE *fp;
    tempStr[0] = '\0';
    
    if (card == NULL){
        return TEMP_UNKNOWN;
    }
    
    //Build the path for the method file
    char *fileName = buildPath(DEFAULT_DRM_DIR, card, DEFAULT_TEMP_PATH);
    if (fileName == NULL){
        return TEMP_UNKNOWN;
    }
    
    if (stripNewLine(readLine(fileName, tempStr, tempStrLen)) != NULL){
        retVal = atoi(tempStr);
    }
    
    free(fileName);
    
    return retVal;
}

char** getCards(char *dirName){
    if (dirName == NULL){
        return NULL;
    }
    
    //Open the /sys/class/drm directory and create a string[] of each card* name
    DIR *dir = opendir(dirName);
    if (dir == NULL){
        return NULL;
    }

    char **cards = calloc(1, sizeof(char*));
    int cardsSize = 0;
    
    struct dirent *dirEntry;
    while (dirEntry = readdir(dir)){
        //Check the name and make sure it's a card
        if (!strstr(dirEntry->d_name, "card") || strstr(dirEntry->d_name, "-"))
            continue;
        
        char *name = malloc(strlen(dirEntry->d_name)+1);
        if (name != NULL){
            strcpy(name, dirEntry->d_name);
            cards = realloc(cards, sizeof(char*) * (cardsSize + 1));
            
            if (cards == NULL){
                closedir(dir);
                return NULL;
            }
            
            //Put the card name in the last slot of cards
            cards[cardsSize++] = name;
        }
    }
    //Null terminate the array
    cards = realloc(cards, sizeof(cards) * (cardsSize + 1));
    cards[cardsSize] = NULL;
    
    //Free system resources
    if (closedir(dir)){
        g_printerr("Unable to close %s\n", dirName);
    }
    
    //And return the generated list of cards
    return cards;
}

void freeCards(char **cards){
    if (cards == NULL)
        return;
    
    while (*cards++ != NULL){
        free(*cards);
    }
    
}

uint countCards(char **cards){
    uint count = 0;
    if (cards == NULL)
        return 0;
    
    while (*cards != NULL){
        count++;
    }
    
    return count;
}

static char *stripNewLine(char *input){
    if (input == NULL)
        return NULL;
    
    char *newline;
    //Strip newline character (and anything after it)
    if (newline = strstr(input, "\n"))
        *newline = '\0';

    return input;
}

static char *readLine(const char *fileName, char *dest, int maxLength){
    char *retVal = NULL;
    FILE *fp;
    
    if (fileName == NULL || dest == NULL)
        return NULL;
    
    //Open the file for reading
    fp = fopen(fileName, "r");
    
    if (fp == NULL){
        g_printerr("File failed to open file for read: %s\n", fileName);
        return NULL;
    }
    
    retVal = fgets(dest, maxLength, fp);
    
    fclose(fp);
    
    return retVal;
}

static int writeFile(const char *fileName, const char *contents){
    int retVal = PM_FALSE;
    FILE *fp;
    
    if (fileName == NULL || contents == NULL)
        return retVal;
    
    //Open the file for writing
    fp = fopen(fileName, "w");
    
    if (fp == NULL){
        g_printerr("File failed to open file for write: %s\n", fileName);
        return retVal;
    }
    
    if (fputs(contents, fp) != EOF){
        retVal = PM_TRUE;
    }
    
    fclose(fp);
    
    return retVal;
}

static char *buildPath(const char *baseDir, const char *card, const char *fileLocation){
    char *fileName = malloc(strlen(DEFAULT_DRM_DIR) + 1 + strlen(card) + strlen(DEFAULT_METHOD_PATH) + 1);
    if (fileName == NULL)
        return NULL;
    
    *fileName = '\0';
    strcat(fileName, baseDir);
    strcat(fileName,"/");
    strcat(fileName, card);
    strcat(fileName, fileLocation);
    return fileName;
}

int canModifyPM(){
    __uid_t uid = geteuid();
    
    if (uid == 0){
        return PM_TRUE;
    } else {
        return PM_FALSE;
    }
}