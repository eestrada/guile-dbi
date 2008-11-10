/* guile-dbi.c - Main source file
 * Copyright (C) 2004, 2005, 2006 Free Software Foundation, Inc.
 * Written by Maurizio Boriani <baux@member.fsf.org>
 *
 * This file is part of the guile-dbi.
 * 
 * The guile-dbi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * The guile-dbi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * Process this file with autoconf to produce a configure script. */


#include <guile-dbi/guile-dbi.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdio.h>


static scm_t_bits g_db_handle_tag;
static SCM mark_db_handle (SCM g_db_handle_smob);
static size_t free_db_handle (SCM g_db_handle_smob);
static int print_db_handle (SCM g_db_handle_smob, SCM port,
			    scm_print_state* pstate);


#define DBI_SMOB_P(obj) ((SCM_NIMP(obj)) && (SCM_TYP16(obj)==g_db_handle_tag))



SCM_DEFINE (make_g_db_handle, "dbi-open", 2, 0, 0,
	    (SCM bcknd, SCM conn_string),
	    "Build db_handle smob.")
#define FUNC_NAME s_make_db_handle
{
  struct g_db_handle *g_db_handle = NULL;
  char* sodbd                     = NULL;
  char* bcknd_str                 = NULL;
  void (*connect)(gdbi_db_handle_t*);

  SCM_ASSERT (scm_is_string (bcknd), bcknd, SCM_ARG1, "make_g_db_handle");
  SCM_ASSERT (scm_is_string (conn_string), bcknd, SCM_ARG2, "make_g_db_handle");

  g_db_handle = (struct g_db_handle*)scm_gc_malloc(sizeof (struct g_db_handle),
						   "g_db_handle");

  g_db_handle->bcknd   = bcknd;
  g_db_handle->constr  = conn_string;
  g_db_handle->handle  = NULL;
  g_db_handle->db_info = NULL;

  bcknd_str = (char*) gh_scm2newstr(bcknd,NULL);

  sodbd=(char*) malloc (sizeof(char)*(strlen("libguile-dbd-") +
				      strlen(bcknd_str) + 10));
  if (sodbd == NULL)
    {
      g_db_handle->status = scm_cons(scm_from_int(errno),
				     scm_makfrom0str(strerror(errno)));
      SCM_RETURN_NEWSMOB (g_db_handle_tag, g_db_handle);
    }
  sprintf(sodbd,"libguile-dbd-%s.so",bcknd_str);

  g_db_handle->handle = dlopen(sodbd,RTLD_NOW);
  if ( g_db_handle->handle == NULL)
    {
      g_db_handle->status =  scm_cons(scm_from_int(1),
			      scm_makfrom0str(dlerror()));      
      SCM_RETURN_NEWSMOB (g_db_handle_tag, g_db_handle);
    }

  __gdbi_dbd_wrap(g_db_handle,(char*) __FUNCTION__,(void**) &connect);  
  if (scm_equal_p (SCM_CAR(g_db_handle->status),scm_from_int(0)) == SCM_BOOL_F)
    {
      SCM_RETURN_NEWSMOB (g_db_handle_tag, g_db_handle);
    }
  
  (*connect)(g_db_handle);

  if (bcknd_str != NULL)
    {
      free(bcknd_str);
    }

  if (sodbd != NULL)
    {
      free(sodbd);
    }

  SCM_RETURN_NEWSMOB (g_db_handle_tag, g_db_handle);
}
#undef FUNC_NAME



static SCM 
mark_db_handle (SCM g_db_handle_smob)
{
  struct g_db_handle* g_db_handle = (struct g_db_handle*)
    SCM_SMOB_DATA(g_db_handle_smob);

  scm_gc_mark(g_db_handle->bcknd);
  scm_gc_mark(g_db_handle->constr);
  scm_gc_mark(g_db_handle->status);
  scm_gc_mark(g_db_handle->closed);

  return SCM_UNSPECIFIED;
}



/* fix this using bcknd function to print */
static int 
print_db_handle (SCM g_db_handle_smob, SCM port,
		 scm_print_state* pstate)
{
  struct g_db_handle* g_db_handle = (struct g_db_handle*)
    SCM_SMOB_DATA(g_db_handle_smob);

  scm_puts("#<guile-dbi ",port);
  if (scm_equal_p (g_db_handle->closed, SCM_BOOL_T) == SCM_BOOL_T)
    {
      scm_puts("close ",port);
    }
  else
    {
      scm_puts("open ",port);
    }
  scm_display(g_db_handle->bcknd,port);
  scm_puts(" ",port);
  scm_display(g_db_handle->constr,port);
  scm_puts(" ",port);
  scm_display(g_db_handle->status,port);
  scm_puts(">",port);
  return 1;
}



SCM_DEFINE (close_g_db_handle, "dbi-close", 1, 0, 0,
	    (SCM db_handle),
	    "Close db connection.")
#define FUNC_NAME s_close_db_handle
{
  struct g_db_handle *g_db_handle = NULL;
  void (*dbi_close)(gdbi_db_handle_t*);

  SCM_ASSERT (DBI_SMOB_P(db_handle), db_handle, SCM_ARG1, "close_g_db_handle");
  g_db_handle = (struct g_db_handle*)SCM_SMOB_DATA(db_handle);

  if (scm_equal_p (g_db_handle->closed, SCM_BOOL_T) == SCM_BOOL_T)
    {
      return  SCM_UNSPECIFIED;
    }

  __gdbi_dbd_wrap(g_db_handle,(char*) __FUNCTION__,(void**) &dbi_close);  
  if (scm_equal_p (SCM_CAR(g_db_handle->status),scm_from_int(0)) == SCM_BOOL_F)
    {
      return  SCM_UNSPECIFIED;
    }
  (*dbi_close)(g_db_handle);
  if (g_db_handle->handle)
    {
      dlclose(g_db_handle->handle);
      g_db_handle->handle = NULL;
    }
  return  SCM_UNSPECIFIED;
}
#undef FUNC_NAME



static size_t 
free_db_handle (SCM g_db_handle_smob)
{
  struct g_db_handle *g_db_handle = NULL;
  size_t size = sizeof(struct g_db_handle);

  g_db_handle = (struct g_db_handle*)SCM_SMOB_DATA(g_db_handle_smob);
  close_g_db_handle(g_db_handle_smob);

  if (g_db_handle != NULL)
    {
      scm_gc_free(g_db_handle,sizeof (struct g_db_handle),"g_db_handle");
    }

  SCM_SETCDR (g_db_handle_smob, (SCM)NULL);
  return (size);
}



SCM_DEFINE (query_g_db_handle, "dbi-query", 2, 0, 0,
	    (SCM db_handle, SCM query),
	    "Do a query and set status.")
#define FUNC_NAME s_query_db_handle
{
  struct g_db_handle *g_db_handle = NULL;
  char               *query_str   = NULL;

  void (*dbi_query)(gdbi_db_handle_t*,char*);

  SCM_ASSERT (DBI_SMOB_P(db_handle), db_handle, SCM_ARG1, "query_g_db_handle");  
  SCM_ASSERT (scm_is_string (query), query, SCM_ARG2, "query_g_db_handle");

  g_db_handle = (struct g_db_handle*)SCM_SMOB_DATA(db_handle);
  query_str = (char*) gh_scm2newstr(query,NULL);

  __gdbi_dbd_wrap(g_db_handle,(char*) __FUNCTION__,(void**) &dbi_query);  
  if (scm_equal_p (SCM_CAR(g_db_handle->status),scm_from_int(0)) == SCM_BOOL_F)
    {
      return(SCM_UNSPECIFIED);
    }
  
  (*dbi_query)(g_db_handle,query_str);

  return (SCM_UNSPECIFIED);  
}
#undef FUNC_NAME



SCM_DEFINE (getrow_g_db_handle, "dbi-get_row", 1, 0, 0,
	    (SCM db_handle),
	    "Do a query and return a row in form of pair list or false")
#define FUNC_NAME s_getrow_db_handle
{
  struct g_db_handle *g_db_handle = NULL;
  SCM retrow = SCM_EOL;
  SCM (*dbi_getrow)(gdbi_db_handle_t*);

  SCM_ASSERT (DBI_SMOB_P(db_handle), db_handle, SCM_ARG1, "getrow_g_db_handle");  

  g_db_handle = (struct g_db_handle*)SCM_SMOB_DATA(db_handle);

  __gdbi_dbd_wrap(g_db_handle,(char*) __FUNCTION__,(void**) &dbi_getrow);  
  if (scm_equal_p (SCM_CAR(g_db_handle->status),scm_from_int(0)) == SCM_BOOL_F)
    {
      return(retrow);
    }
  
  retrow = (*dbi_getrow)(g_db_handle);

  return(retrow);  
}
#undef FUNC_NAME



SCM_DEFINE (getstat_g_db_handle, "dbi-get_status", 1, 0, 0,
	    (SCM db_handle),
	    "Return status pair, code and msg, from dbi smob.")
#define FUNC_NAME s_getstat_db_handle
{
  struct g_db_handle *g_db_handle = NULL;

  SCM_ASSERT (DBI_SMOB_P(db_handle), db_handle, SCM_ARG1, "getstat_g_db_handle");  

  g_db_handle = (struct g_db_handle*)SCM_SMOB_DATA(db_handle);

  if (g_db_handle != NULL)
    {
      return (g_db_handle->status);
    }

  return (SCM_BOOL_F);  
}
#undef FUNC_NAME



/* module init function */
void 
init_db_handle_type(void)
{
  g_db_handle_tag = scm_make_smob_type("g_db_handle",
				       sizeof (struct g_db_handle));
  scm_set_smob_mark (g_db_handle_tag, mark_db_handle);
  scm_set_smob_free (g_db_handle_tag, free_db_handle);
  scm_set_smob_print (g_db_handle_tag, print_db_handle);
}


void 
init_dbi(void)
{
  init_db_handle_type();

#ifndef SCM_MAGIC_SNARFER
#include "guile-dbi.x"
#endif
}



/* dbd handler */
void
__gdbi_dbd_wrap(gdbi_db_handle_t* dbh, char* function_name,
		void** function_pointer)
{
  char *ret   = NULL;
  char *func  = NULL;
  char *bcknd = NULL;

  bcknd = (char*) gh_scm2newstr(dbh->bcknd,NULL);

  if((func=malloc(sizeof(char)*(strlen(function_name)+
		  20))) == NULL)
    {
      dbh->status = (SCM) scm_cons(scm_from_int(errno),
				   scm_makfrom0str(strerror(errno)));
      return;
    }

  sprintf(func,"__%s_%s",bcknd,function_name);
  *(void **) (function_pointer) = dlsym(dbh->handle,func);
  if((ret = dlerror()) != NULL)
    {
      dbh->status = (SCM) scm_cons(scm_from_int(1),
				   scm_makfrom0str(ret));
      return;
    }

  free(func);
  if (bcknd != NULL)
    {
      free(bcknd);
    }
  /* todo: error msg to be translated */
  dbh->status = (SCM) scm_cons(scm_from_int(0),
			   scm_makfrom0str("symbol loaded"));
  return;
}
