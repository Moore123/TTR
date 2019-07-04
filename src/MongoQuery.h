#include "Rmongo.h"

#define EMPTYROWS 33

typedef struct
{
	int argc;
	char **argv;
} MgPx_arglist;

typedef struct MgPx_Command {
  const char *name;
  const char *help;
  struct MgPx_Command *children;
  int (* func)(MgPx_arglist *,mongoParam *);
  int privilege;
} MgPx_command;

extern MgPx_command CMD[];

extern int setDim(SEXP , int nCol, int nRow);

extern int select_cmd (MgPx_arglist * args, mongoParam * gp);
extern int delete_cmd (MgPx_arglist * args, mongoParam * gp);

extern int zelect_cmd(MgPx_arglist * args, mongoParam * gp);
extern int xelect_cmd(MgPx_arglist * args, mongoParam * gp);

extern int queryRunning(MgPx_arglist * args, mongoParam * gp);
extern int dselect_cmd (MgPx_arglist * args, mongoParam * gp);
extern int z2gridfs(MgPx_arglist * args, mongoParam * gp);
extern int insert_cmd (MgPx_arglist * args, mongoParam * gp);
extern int drop_cmd (MgPx_arglist * args, mongoParam * gp);
extern int mybson_dump_raw (mongoParam *,int depth);
extern int freeQvList (qV *);

