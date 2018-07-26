/*
 * Taken from Ioannis Konstantelias @ GitHub
 * https://github.com/gon1332/fort320/blob/master/include/Utils/colors.h
 */
#ifndef COLORS_
#define COLORS_

/* FOREGROUND */
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define bRED  "\x1B[41m"
#define bGRN  "\x1B[42m"
#define bYEL  "\x1B[43m"
#define bBLU  "\x1B[44m"
#define bMAG  "\x1B[45m"
#define bCYN  "\x1B[46m"
#define bWHT  "\x1B[47m"

// BACKGROUNDS
#define BRED(x) bRED + x + RST
#define BGRN(x) bGRN + x + RST
#define BYEL(x) bYEL + x + RST
#define BBLU(x) bBLU + x + RST
#define BMAG(x) bMAG + x + RST
#define BCYN(x) bCYN + x + RST
#define BWHT(x) bWHT + x + RST

// FOREGROUNDS
#define FRED(x) KRED + x + RST
#define FGRN(x) KGRN + x + RST
#define FYEL(x) KYEL + x + RST
#define FBLU(x) KBLU + x + RST
#define FMAG(x) KMAG + x + RST
#define FCYN(x) KCYN + x + RST
#define FWHT(x) KWHT + x + RST

// FOREGROUNDS CONSTANT
#define CRED(x) KRED x RST
#define CGRN(x) KGRN x RST
#define CYEL(x) KYEL x RST
#define CBLU(x) KBLU x RST
#define CMAG(x) KMAG x RST
#define CCYN(x) KCYN x RST
#define CWHT(x) KWHT x RST

#define BOLD(x)	"\x1B[1m" x RST
#define UNDL(x)	"\x1B[4m" x RST


#endif	/* _COLORS_ */
