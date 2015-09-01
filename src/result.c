/*
 *  mutt_vc_query - vCard query utility for mutt
 *  Copyright (C) 2003  Andrew Hsu
 * 
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA
 *
 *  $Id: result.c,v 1.3 2003/05/14 03:41:02 ahsu Rel $
 */

#include "result.h"
#include <stdlib.h>
#include <string.h>
#include <vc.h>

#if HAVE_CONFIG_H
#include "config.h"
#endif

struct query_result_tag
{
  char *email;
  char *name;
  char *misc;
  query_result *next;
};

/*** LOCAL PROTOTYPES ***/
static query_result **qr_lltoa (query_result * results, int n);
static query_result *qr_atoll (query_result ** results_a, int n);
static int cmp_results_by_name (const void *a, const void *b);
static int cmp_results_by_email (const void *a, const void *b);
static int cmp_results_by_misc (const void *a, const void *b);
static int strstr_nocase (const char *haystack, const char *needle);
static char *get_misc_value (vc_component * vcard, const char *type_name);
static query_result * add_result (const char *query_string,
                                  const char *name,
                                  const char *misc_value,
                                  const char *email,
                                  query_result * results,
                                  int *rc);

/*** STATIC FUNCTIONS ***/

/***************************************************************************
 */

static query_result **
qr_lltoa (query_result * results, int n)
{
  query_result **results_a = NULL;
  query_result *r = NULL;
  int i = 0;

  results_a = (query_result **) malloc (sizeof (query_result *) * n);

  r = results->next;
  for (i = 0; i < n; i++)
    {
      results_a[i] = r;
      r = r->next;
    }

  return results_a;
}

/***************************************************************************
 */

static query_result *
qr_atoll (query_result ** results_a, int n)
{
  query_result *r = NULL;
  query_result *returning_r = NULL;
  query_result *tmp = NULL;
  int i = 0;

  r = create_query_result ();
  returning_r = r;
  for (i = 0; i < n; i++)
    {
      r->next = results_a[i];
      r = r->next;
      r->next = NULL;
    }

  tmp = returning_r;
  returning_r = returning_r->next;
  delete_query_result (tmp);
  return returning_r;
}

/***************************************************************************
    Compares the query_result names.
 */

static int
cmp_results_by_name (const void *a, const void *b)
{
  int ret_val = 0;
  query_result **qra = NULL;
  query_result **qrb = NULL;

  qra = (query_result **) a;
  qrb = (query_result **) b;

  ret_val = strcmp ((*qra)->name, (*qrb)->name);

  if (0 == ret_val)
    {
      ret_val = strcmp ((*qra)->email, (*qrb)->email);
      if (0 == ret_val)
        {
          ret_val = strcmp ((*qra)->misc, (*qrb)->misc);
        }
    }

  return ret_val;
}

/***************************************************************************
 */

static int
cmp_results_by_email (const void *a, const void *b)
{
  int ret_val = 0;
  query_result **qra = NULL;
  query_result **qrb = NULL;

  qra = (query_result **) a;
  qrb = (query_result **) b;

  ret_val = strcmp ((*qra)->email, (*qrb)->email);

  if (0 == ret_val)
    {
      ret_val = strcmp ((*qra)->name, (*qrb)->name);
      if (0 == ret_val)
        {
          ret_val = strcmp ((*qra)->misc, (*qrb)->misc);
        }
    }

  return ret_val;
}

/***************************************************************************
 */

static int
cmp_results_by_misc (const void *a, const void *b)
{
  int ret_val = 0;
  query_result **qra = NULL;
  query_result **qrb = NULL;

  qra = (query_result **) a;
  qrb = (query_result **) b;

  ret_val = strcmp ((*qra)->misc, (*qrb)->misc);

  if (0 == ret_val)
    {
      ret_val = strcmp ((*qra)->name, (*qrb)->name);
      if (0 == ret_val)
        {
          ret_val = strcmp ((*qra)->email, (*qrb)->email);
        }
    }

  return ret_val;
}

/***************************************************************************
    Searches for the needle in the haystack without worrying about
    the case.  Returns 1 if found, 0 otherwise.
 */

static int
strstr_nocase (const char *haystack, const char *needle)
{
  int result = 0;

  if (NULL != haystack && NULL != needle)
    {
      char tmp_haystack[80];
      char tmp_needle[80];
      int len = 0;
      int i = 0;

      strncpy (tmp_haystack, haystack, sizeof (tmp_haystack) - 1);
      tmp_haystack[sizeof (tmp_haystack) - 1] = '\0';
      strncpy (tmp_needle, needle, sizeof (tmp_needle) - 1);
      tmp_needle[sizeof (tmp_haystack) - 1] = '\0';

      len = strlen (tmp_haystack);
      for (i = 0; i < len; i++)
        {
          tmp_haystack[i] = tolower (tmp_haystack[i]);
        }

      len = strlen (tmp_needle);
      for (i = 0; i < len; i++)
        {
          tmp_needle[i] = tolower (tmp_needle[i]);
        }

      if (NULL != strstr (tmp_haystack, tmp_needle))
        {
          result = 1;
        }
    }

  return result;
}

/***************************************************************************
    Retrieves the value to be used for the misc field.  The return
    value is malloc'ed so the user of this function is responsible
    for freeing the returned pointer.

    TODO: parse the type_name as TYPE_NAME:STRUCT_NUMBER
 */

static char *
get_misc_value (vc_component * vcard, const char *type_name)
{
  char *ret_val = NULL;
  vc_component *tmp_vc = NULL;

  if (NULL == type_name)
    {
      /* retrieve the default value of the address locality */
      tmp_vc = vc_get_next_by_name (vcard, VC_ADDRESS);
      ret_val = get_val_struct_part (vc_get_value (tmp_vc), ADR_LOCALITY);
    }
  else
    {
      tmp_vc = vc_get_next_by_name (vcard, type_name);
      ret_val = vc_get_value (tmp_vc);
    }

  ret_val = ret_val ? strdup (ret_val) : strdup (" ");

  return ret_val;
}

/***************************************************************************
    Adds a result to the results list if it matches
    results arg is where to add the new result to
    return value is a pointer to where to add the next result
 */

static query_result *
add_result (const char *query_string,
            const char *name,
            const char *misc_value,
            const char *email,
            query_result * results,
            int *rc)
  {
    query_result *r = results;
    if (name != NULL && email != NULL)
      {
        /* perform the query using name, email, and misc fields */
        if (strstr_nocase (name, query_string)
            || strstr_nocase (email, query_string)
            || strstr_nocase (misc_value, query_string))
          {
            r->next = create_query_result ();
            r = r->next;
            r->name = strdup (name);
            r->email = strdup (email);
            r->misc = strdup (misc_value);
            (*rc)++; /* increment results counter */
          }
      }
    return r;
  }


/*** EXTERNAL FUNCTIONS ***/

/***************************************************************************
    Creates a query_result data node with member values initialized
    to NULL.
 */

query_result *
create_query_result ()
{
  query_result *ret_val = NULL;

  ret_val = (query_result *) malloc (sizeof (query_result));
  if (NULL == ret_val)
    {
      fprintf (stderr, "Unable to malloc query_result.\n");
      exit (1);
    }

  /* initialize the members to NULL */
  ret_val->name = NULL;
  ret_val->email = NULL;
  ret_val->misc = NULL;
  ret_val->next = NULL;

  return ret_val;
}

/***************************************************************************
 */

void
delete_query_result (query_result * qr)
{
  free (qr->name);
  qr->name = NULL;
  free (qr->email);
  qr->email = NULL;
  free (qr->misc);
  qr->misc = NULL;

  free (qr);
}

/***************************************************************************
 */

void
get_results (FILE * fp,
             const char *query_string,
             const char *misc_field,
             const int all_emails,
             int *searched, query_result * results, int *rc)
{
  vc_component *v = NULL;
  char *s_result = NULL;
  char *email = NULL;
  char *name = NULL;
  char *misc = NULL;
  vc_component *fn = NULL;
  query_result * r = NULL;

  r = results;
  *rc = 0;
  *searched = 0;
  for (v = parse_vcard_file (fp); NULL != v; v = parse_vcard_file (fp))
    {
      (*searched)++;
      fn = vc_get_next_by_name (v, VC_FORMATTED_NAME);
      name = vc_get_value (fn);
      misc = get_misc_value (v, misc_field);
      if (all_emails)
        {
          for (fn = vc_get_next_by_name (v, VC_EMAIL);
               NULL != fn;
               fn = vc_get_next_by_name(fn, VC_EMAIL))
            {
              email = vc_get_value (fn);
              r = add_result(query_string, name, misc, email, r, rc);
            }
        }
      else
        {
          email = vc_get_preferred_email (v);
          r = add_result(query_string, name, misc, email, r, rc);
        }

      free (misc);
      vc_delete_deep (v);
      v = NULL;
    }
}

/***************************************************************************
 */

void
sort_results (query_result * results, int n, int sort_by)
{
  query_result **results_a = NULL;
  int i = 0;

  results_a = qr_lltoa (results, n);

  switch (sort_by)
    {
    case SORT_RESULTS_BY_NAME:
      qsort (results_a, n, sizeof (query_result *), cmp_results_by_name);
      break;
    case SORT_RESULTS_BY_EMAIL:
      qsort (results_a, n, sizeof (query_result *), cmp_results_by_email);
      break;
    case SORT_RESULTS_BY_MISC:
      qsort (results_a, n, sizeof (query_result *), cmp_results_by_misc);
      break;
    default:
      break;
    }

  results->next = qr_atoll (results_a, n);
}

/***************************************************************************
    Display the results of the query to stdout.
 */

void
print_results (const query_result * results)
{
  query_result *r = NULL;

  for (r = results->next; NULL != r; r = r->next)
    {
      fprintf (stdout, "%s\t%s\t%s\n", r->email, r->name, r->misc);
    }
}
