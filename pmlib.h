/* 
 * File:   pmlib.h
 * Author: awatry
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

extern const char * const pm_profile_names[];
extern const char * const pm_method_names[];
extern const char* DEFAULT_DRM_DIR;
extern const char* DEFAULT_METHOD_PATH;
extern const char* DEFAULT_PROFILE_PATH;

#define PM_TRUE 1
#define PM_FALSE 0

pm_method_t getMethod(char *card);
pm_profile_t getProfile(char *card);
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

