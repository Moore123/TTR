
#ifndef __INDEP_CFG_H
#define __INDEP_CFG_H

#define TYPE_INT        0
#define TYPE_STRING     1

#define PARM_OPT        0
#define PARM_MAND       1

#define MAX_STRING_LEN 1024
#define STR_BUFF_LEN  256

#define FULL_PATH_LEN 64

/*#define  TRUE 1
#define  FALSE 0
#define  FAIL  -1
*/
struct cfg_line 
{
        char    *parameter; 
        void    *variable;
        int     (*function)();
        int     type;
        int     mandatory;
        int     min;
        int     max;
};      
        
        
struct chkplugs_strsdef
{
        int port;
        char *cmdline;
        int     (*function)();
        char *param;
        int ok_loc;
        
};      
        
extern int  parse_cfg_file(char *cfg_file,struct cfg_line *cfg);
extern void init_config(void);      

#ifndef HAVE_GETOPT_LONG
	struct option {
		const char *name;
		int has_arg;
		int *flag;
		int val;
	};
#define  getopt_long(argc, argv, optstring, longopts, longindex) getopt(argc, argv, optstring)
#endif /* ndef HAVE_GETOPT_LONG */

#endif
