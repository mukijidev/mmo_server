#pragma once
#include "Type.h"


//#define PROFILE

#ifdef PROFILE
#define PRO_BEGIN(TagName) ProfileBegin(TagName)
#define PRO_END(TagName) ProfileEnd(TagName)
#define PRO_SAVE(FileName) ProfileDataOutText(FileName);
#define PRO_RESET() ProfileReset();


#else
#define PRO_BEGIN(TagName)
#define PRO_END(TagName)
#define PRO_SAVE(FileName) ProfileDataOutText(FileName);
#define PRO_RESET()  
#endif

extern LARGE_INTEGER Freq;

void ProfileBegin(const WCHAR* funcName);
void ProfileEnd(const WCHAR* funcName);
void ProfileDataOutText(const WCHAR* fileName);
void ProfileReset();
void ProfileInit();