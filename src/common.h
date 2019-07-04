
#include <sys/types.h>

#ifndef GetTTR_COMMON_H
#define GetTTR_COMMON_H

#define MAX32NUM  4294967295LL
#define MAX32LL  4294967296LL
#define MAX64LL   18446744073709551615LL

#define MAX32   0xFFFFFFFF



/* the below is for plus_minus() */
#define PREVIOUS_OP	(-1)

/* parse translation table - table driven parsers can be your FRIEND!
*/
struct SpecialToken
{
  char *name;			/* token name */
  int value;			/* token id */
};

typedef struct
{
  char *data;
  unsigned int len;
} RequestData;

/* leap year -- account for gregorian reformation in 1752 */
#define	leap_year(yr)  	((yr) <= 1752 ? !((yr) % 4) : (!((yr) % 4) && ((yr) % 100)) || !((yr) % 400))

/* number of centuries since 1700, not inclusive */
#define	centuries_since_1700(yr) \
	((yr) > 1700 ? (yr) / 100 - 17 : 0)

/* number of centuries since 1700 whose modulo of 400 is 0 */
#define	quad_centuries_since_1700(yr) \
	((yr) > 1600 ? ((yr) - 1600) / 400 : 0)

/* number of leap years between year 1 and this year, not inclusive */
#define	leap_years_since_year_1(yr) \
	((yr) / 4 - centuries_since_1700(yr) + quad_centuries_since_1700(yr))

/*#define TRUE    1
#define FALSE   0
*/
#define TINYBUF 128

#define	SUCCESS		0

#define	FAIL		(-1)

#define	NOTSUPPORTED	(-2)
#define	NETWORK_ERROR	(-3)
#define	TIMEOUT_ERROR	(-4)
#define	AGENT_ERROR	(-5)
#define NOSUCHMIB  (-6)

#define MAX_BUF_LEN	65000

#define MAX_QUERY_LEN           4096
#define MAX_STRING_LEN          MAX_QUERY_LEN
#define SQL_DOWN                1
#define MAX_COMMUNITY_LEN       50
#define MAX_SQL_SOCKS           256
#define MAX_TABLE_LEN           20
#define MAX_STR_LEN 			1024
#define MIN_STR_LEN 			16
#define STRING_LEN             	128

#define QUERY_WAIT	10000

/* Secure string copy */
#define strscpy(x,y) { strncpy(x,y,sizeof(x)); x[sizeof(x)-1]=0; }

typedef struct Context_Line
{
  int linenumber;		/* the linenumber               */

  char *content;		/* the logline                  */
  struct Context_Line *next;
  struct Context_Line *prev;
} context_line;

typedef struct Sfa_State
{
  unsigned int linenumber;

  unsigned int order;

  char *content;
  struct Sfa_State *next, *prev;
} sfa_state;

typedef struct DumpData
{
  int type;
  int recover;
  int date;
  int ShareName;
  unsigned long start, end, last;
} Dump_Data;

#define NORECOV 1<<0
#define FORWARD 1<<1
#define BACKWARD 1<<2

#define SHDAY 1<<5
#define SZDAY 1<<6
#define SHMIN 1<<7
#define SZMIN 1<<8

typedef struct DZH
{

  unsigned int date;

  unsigned long open;

  unsigned long high;

  unsigned long low;

  unsigned long endp;

  float volumn;

  unsigned long amount;

  unsigned long a;

  unsigned long b;

  unsigned long c;

} dzh;

typedef struct Shase_Day_Header
{

  unsigned char magic[6];
  unsigned int records;
  unsigned short start;
  unsigned short length;
  unsigned short col;

} ShaseDayHeader;

typedef struct Index_Desc
{

  unsigned int length;
  unsigned int RecCol;

  unsigned long ContentStart;


} IndexDesc;

typedef struct Profit_Index_Desc
{

  unsigned char type;
  unsigned char name[9];
  unsigned short space;
  unsigned int order;
  unsigned short col;

} ProfitIndexDesc;

typedef struct TTR_Descript
{

  unsigned long openor, highor, lowor, endor;
  unsigned int date;
  float volumn, amount;

  struct TTR_Descript *prev, *next;

} TTRdesc;

typedef struct TTR_Profit
{

  unsigned int date;

  unsigned char type;		// +  % 
  int base;
  float div;
  float sub;
  float rate;
  float price;

  struct TTR_Profit *prev, *next;
} TProfit;

typedef struct Profit_State_M
{
  int prevstate;
  int nextstate;
  char *smname;
  int smassign;

} ProfitStateM;

/*

"-"

"("
")"
"每十股"
"配股"
"配股价"
"送"
"红利"
"转增"
"股"
"元"

*/

#define ShaseHeaderLen 0x10
#define ProfitHeaderLen sizeof(IndexDesc)
#define ProfitIndexLen 0x12

#define RecordLen sizeof(dzh)

#define BeInit  1<<0
#define BeStart 1<<1
#define BeCont	1<<2
#define BeContPrice 1<<3
#define BeFin   1<<4
#define BeTen   1<<5
#define BeNum   1<<6
#define BeEnd   1<<8

#define PftHongli 1<<0
#define PftHongGu 1<<1
#define PftPeigu  1<<2
#define PftPeiguP 1<<3
#define PftBase	  1<<4

#define PftNothing 1<<6

#define DATELEN 10
#define PftBaseMent 10

#define CNUM 7

#define DIM(x) (sizeof(x)/sizeof(x[0]))

#define ISPRINT(c) (isascii (c) && isprint (c))
#define ISDIGIT(c) (isascii (c) && isdigit (c))
#define ISALNUM(c) (isascii (c) && isalnum (c))
#define ISALPHA(c) (isascii (c) && isalpha (c))
#define ISCNTRL(c) (isascii (c) && iscntrl (c))
#define ISLOWER(c) (isascii (c) && islower (c))
#define ISPUNCT(c) (isascii (c) && ispunct (c))
#define ISSPACE(c) (isascii (c) && isspace (c))
#define ISUPPER(c) (isascii (c) && isupper (c))
#define ISXDIGIT(c) (isascii (c) && isxdigit (c))

#define OPT_DIFF    1
#define OPT_SETSTART 1<<1
#define OPT_SETEND 1<<2
#define OPT_REWARD  1<<3
#define OPT_SUMMARY 1<<4
#define OPT_PERIOD  1<<5

#define OPT_RATE    1<<6
#define OPT_AMOUNT  1<<7

#define OPT_REFERENCE  1<<8

#define OPT_FILES  1<<9
#define OPT_DATES 1<<10
#define OPT_PROFIT 1<<11

typedef struct CONFOPT
{
  char *SetString;
  unsigned int type;
  unsigned int opt;
//      (void *) value;
} ConfOpt;

typedef context_line List4Files;

typedef struct ShareHold_State
{
  char *StrName;
  int order;
} ShareHoldState;

#endif
