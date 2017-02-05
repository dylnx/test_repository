#ifndef WHITELIST_H_JV8B3ZUW
#define WHITELIST_H_JV8B3ZUW


int WhiteListDatabaseInit();
int RefreshWLDatabase(char *data, int count);
int CheckWhiteList(char tid[16],char *carnum);

#endif /* end of include guard: WHITELIST_H_JV8B3ZUW */
