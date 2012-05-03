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

/* 
 * File:   pmlib.h
 * Author: Aaron Watry
 *
 * Created on April 21, 2012, 6:31 PM
 */

#ifndef PMLIB_H
#define	PMLIB_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum pm_profile_t { LOW=0, MEDIUM=1, HIGH=2, AUTO=3, DEFAULT=4, PROFILE_UNKNOWN=5 } pm_profile_t;
#define MAX_PROFILE PROFILE_UNKNOWN

typedef enum PM_METHOD { PROFILE=0, DYNPM=1, METHOD_UNKNOWN=2 } pm_method_t;
#define MAX_METHOD METHOD_UNKNOWN

extern pm_profile_t pm_profiles[];
extern const char * const pm_profile_names[];
extern const pm_method_t pm_methods[];
extern const char * const pm_method_names[];
extern const char* DEFAULT_DRM_DIR;
extern const char* DEFAULT_METHOD_PATH;
extern const char* DEFAULT_PROFILE_PATH;

#define PM_TRUE 1
#define PM_FALSE 0

pm_method_t getMethod(char *card);
const char* getMethodName(pm_method_t method);
pm_profile_t getProfile(char *card);
const char* getProfileName(pm_profile_t profile);
int setMethod(char *card, pm_method_t newMethod);
int setProfile(char *card, pm_profile_t newProfile);
int getTemperature(char *card);
char *getFreqInfo(char *card);
char** getCards(char*);
void freeCards(char **cards);
uint countCards(char**);
int canModifyPM();


#ifdef	__cplusplus
}
#endif

#endif	/* PMLIB_H */

