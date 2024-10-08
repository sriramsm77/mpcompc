/* String-list functions for GNU Mailutils.
   Copyright (C) 2007-2022 Free Software Foundation, Inc.

   Based on slist module from GNU Radius.  Written by Sergey Poznyakoff.
   
   GNU Mailutils is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3, or (at
   your option) any later version.

   GNU Mailutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <mailutils/types.h>
#include <mailutils/alloc.h>
#include <mailutils/opool.h>
#include <mailutils/errno.h>
#include <mailutils/error.h>
#include <mailutils/nls.h>
#include <mailutils/iterator.h>

struct bucket_header
{
  union mu_opool_bucket *next;
  char *buf;
  size_t level;
  size_t size;
};

union mu_opool_bucket
{
  struct bucket_header hdr;
  long double align_double;
  uintmax_t align_uintmax;
  void *align_ptr;
};

struct _mu_opool
{
  int flags;                   /* Flag bits */
  size_t bucket_size;          /* Default bucket size */
  size_t itr_count;            /* Number of iterators created for this pool */
  mu_nonlocal_jmp_t *jmp;      /* Buffer for non-local exit */
  union mu_opool_bucket *bkt_head, *bkt_tail; 
  union mu_opool_bucket *bkt_fini; /* List of finished objects */
};

static union mu_opool_bucket *
alloc_bucket (struct _mu_opool *opool, size_t size)
{
  union mu_opool_bucket *p = malloc (sizeof (*p) + size);
  if (!p)
    {
      if (opool->flags & MU_OPOOL_ENOMEMABRT)
	mu_alloc_die ();
      if (opool->jmp)
	longjmp (opool->jmp->buf, ENOMEM);
    }
  else
    {
      p->hdr.buf = (char*)(p + 1);
      p->hdr.level = 0;
      p->hdr.size = size;
      p->hdr.next = NULL;
    }
  return p;
}

static int
alloc_pool (mu_opool_t opool, size_t size)
{
  union mu_opool_bucket *p = alloc_bucket (opool, opool->bucket_size);
  if (!p)
    return ENOMEM;
  if (opool->bkt_tail)
    opool->bkt_tail->hdr.next = p;
  else
    opool->bkt_head = p;
  opool->bkt_tail = p;
  return 0;
}

static int
copy_chars (mu_opool_t opool, const char *str, size_t n, size_t *psize)
{
  size_t rest;

  if (!opool->bkt_head
      || opool->bkt_tail->hdr.level == opool->bkt_tail->hdr.size)
    if (alloc_pool (opool, opool->bucket_size))
      return ENOMEM;
  rest = opool->bkt_tail->hdr.size - opool->bkt_tail->hdr.level;
  if (n > rest)
    n = rest;
  memcpy (opool->bkt_tail->hdr.buf + opool->bkt_tail->hdr.level, str, n);
  opool->bkt_tail->hdr.level += n;
  *psize = n;
  return 0;
}

int
mu_opool_create (mu_opool_t *pret, int flags)
{
  struct _mu_opool *x = malloc (sizeof (x[0]));
  if (!x)
    {
      if (flags & MU_OPOOL_ENOMEMABRT)
	mu_alloc_die ();
      return ENOMEM;
    }
  x->flags = flags;
  x->bucket_size = MU_OPOOL_BUCKET_SIZE;
  x->itr_count = 0;
  x->bkt_head = x->bkt_tail = x->bkt_fini = NULL;
  x->jmp = NULL;
  *pret = x;
  return 0;
}

void
mu_opool_setjmp (mu_opool_t opool, mu_nonlocal_jmp_t *jmp)
{
  if (jmp)
    {
      jmp->next = opool->jmp;
      opool->jmp = jmp;
    }
  else
    mu_opool_clrjmp (opool);
}

void
mu_opool_clrjmp (mu_opool_t opool)
{
  if (opool->jmp)
    opool->jmp = opool->jmp->next;
}

int
mu_opool_set_bucket_size (mu_opool_t opool, size_t size)
{
  if (!opool)
    return EINVAL;
  opool->bucket_size = size;
  return 0;
}

int
mu_opool_get_bucket_size (mu_opool_t opool, size_t *psize)
{
  if (!opool || !psize)
    return EINVAL;
  *psize = opool->bucket_size;
  return 0;
}

void
mu_opool_clear (mu_opool_t opool)
{
  if (!opool)
    return;
  
  if (opool->bkt_tail)
    {
      opool->bkt_tail->hdr.next = opool->bkt_fini;
      opool->bkt_fini = opool->bkt_head;
      opool->bkt_head = opool->bkt_tail = NULL;
    }
}	

void
mu_opool_less (mu_opool_t opool, size_t sz)
{
  union mu_opool_bucket *p;
  size_t total = 0;
  
  if (!opool)
    return;
  for (p = opool->bkt_head; p; p = p->hdr.next)
    {
      if (total + p->hdr.level >= sz)
	{
	  union mu_opool_bucket *t;
	  p->hdr.level = sz - total;
	  t = p->hdr.next;
	  p->hdr.next = NULL;

	  while (t)
	    {
	      union mu_opool_bucket *next = t->hdr.next;
	      free (t);
	      t = next;
	    }
	  break;
	}
      total += p->hdr.level;
    }
}	    

void
mu_opool_destroy (mu_opool_t *popool)
{
  union mu_opool_bucket *p;
  if (popool && *popool)
    {
      mu_opool_t opool = *popool;
      mu_opool_clear (opool);
      for (p = opool->bkt_fini; p; )
	{
	  union mu_opool_bucket *next = p->hdr.next;
	  free (p);
	  p = next;
	}
      free (opool);
      *popool = NULL;
    }
}

int
mu_opool_alloc (mu_opool_t opool, size_t size)
{
  while (size)
    {
      size_t rest;

      if (!opool->bkt_head
	  || opool->bkt_tail->hdr.level == opool->bkt_tail->hdr.size)
	if (alloc_pool (opool, opool->bucket_size))
	  return ENOMEM;
      rest = opool->bkt_tail->hdr.size - opool->bkt_tail->hdr.level;
      if (size < rest)
	rest = size;
      opool->bkt_tail->hdr.level += rest;
      size -= rest;
    }
  return 0;
}

int
mu_opool_append (mu_opool_t opool, const void *str, size_t n)
{
  const char *ptr = str;
  while (n)
    {
      size_t s;
      if (copy_chars (opool, ptr, n, &s))
	return ENOMEM;
      ptr += s;
      n -= s;
    }
  return 0;
}

int
mu_opool_append_char (mu_opool_t opool, char c)
{
  return mu_opool_append (opool, &c, 1);
}	

int
mu_opool_appendz (mu_opool_t opool, const char *str)
{
  return mu_opool_append (opool, str, strlen (str));
}

size_t
mu_opool_size (mu_opool_t opool)
{
  size_t size = 0;
  union mu_opool_bucket *p;
  for (p = opool->bkt_head; p; p = p->hdr.next)
    size += p->hdr.level;
  return size;
}

size_t
mu_opool_copy (mu_opool_t opool, void *buf, size_t size)
{
  char *cp = buf;
  size_t total = 0;
  union mu_opool_bucket *p;
  
  for (p = opool->bkt_head; p && total < size; p = p->hdr.next)
    {
      size_t cpsize = size - total;
      if (cpsize > p->hdr.level)
	cpsize = p->hdr.level;
      memcpy (cp, p->hdr.buf, cpsize);
      cp += cpsize;
      total += cpsize;
    }
  return total;
}

int
mu_opool_coalesce (mu_opool_t opool, size_t *psize)
{
  size_t size;

  if (opool->itr_count)
    return MU_ERR_FAILURE;
  if (opool->bkt_head && opool->bkt_head->hdr.next == NULL)
    size = opool->bkt_head->hdr.level;
  else
    {
      union mu_opool_bucket *bucket;
      union mu_opool_bucket *p;

      size = mu_opool_size (opool);
	
      bucket = alloc_bucket (opool, size);
      if (!bucket)
	return ENOMEM;
      for (p = opool->bkt_head; p; )
	{
	  union mu_opool_bucket *next = p->hdr.next;
	  memcpy (bucket->hdr.buf + bucket->hdr.level, p->hdr.buf,
		  p->hdr.level);
	  bucket->hdr.level += p->hdr.level;
	  free (p);
	  p = next;
	}
      opool->bkt_head = opool->bkt_tail = bucket;
    }
  if (psize)
    *psize = size;
  return 0;
}

void *
mu_opool_head (mu_opool_t opool, size_t *psize)
{
  if (psize) 
    *psize = opool->bkt_head ? opool->bkt_head->hdr.level : 0;
  return opool->bkt_head ? opool->bkt_head->hdr.buf : NULL;
}

void *
mu_opool_finish (mu_opool_t opool, size_t *psize)
{
  if (mu_opool_coalesce (opool, psize))
    return NULL;
  mu_opool_clear (opool);
  return opool->bkt_fini->hdr.buf;
}

void *
mu_opool_detach (mu_opool_t opool, size_t *psize)
{
  union mu_opool_bucket *bp;
  
  if (mu_opool_coalesce (opool, psize))
    return NULL;
  mu_opool_clear (opool);

  bp = opool->bkt_fini;
  opool->bkt_fini = bp->hdr.next;
  memmove (bp, bp->hdr.buf, bp->hdr.level);
  return bp;
}

void
mu_opool_free (mu_opool_t pool, void *obj)
{
  if (!pool)
    return;
  if (!obj)
    {
      if (pool->bkt_head)
	mu_opool_finish (pool, NULL);
      while (pool->bkt_fini)
	{
	  union mu_opool_bucket *next = pool->bkt_fini->hdr.next;
	  free (pool->bkt_fini);
	  pool->bkt_fini = next;
	}
    }
  else
    {
      union mu_opool_bucket *bucket = pool->bkt_fini,
	                    **pprev = &pool->bkt_fini;
      while (bucket)
	{
	  if (bucket->hdr.buf == obj)
	    {
	      *pprev = bucket->hdr.next;
	      free (bucket);
	      return;
	    }
	  pprev = &bucket->hdr.next;
	  bucket = bucket->hdr.next;
	}
    }
}

void *
mu_opool_dup (mu_opool_t pool, void const *data, size_t size)
{
  if (mu_opool_append (pool, data, size))
    return NULL;
  return mu_opool_finish (pool, NULL);
}

int
mu_opool_union (mu_opool_t *pdst, mu_opool_t *psrc)
{
  mu_opool_t src, dst;
  
  if (!psrc)
    return EINVAL;
  if (!*psrc)
    return 0;
  src = *psrc;
  
  if (!pdst)
    return EINVAL;
  if (!*pdst)
    {
      *pdst = src;
      *psrc = NULL;
      return 0;
    }
  else
    dst = *pdst;

  if (dst->bkt_tail)
    dst->bkt_tail->hdr.next = src->bkt_head;
  else
    dst->bkt_head = src->bkt_head;
  dst->bkt_tail = src->bkt_tail;

  if (src->bkt_fini)
    {
      union mu_opool_bucket *p;

      for (p = src->bkt_fini; p->hdr.next; p = p->hdr.next)
	;
      p->hdr.next = dst->bkt_fini;
      dst->bkt_fini = src->bkt_fini;
    }

  free (src);
  *psrc = NULL;
  return 0;
}


/* Iterator support */
struct opool_iterator
{
  mu_opool_t opool;
  union mu_opool_bucket *cur;
};

static int
opitr_first (void *owner)
{
  struct opool_iterator *itr = owner;
  itr->cur = itr->opool->bkt_head;
  return 0;
}

static int
opitr_next (void *owner)
{
  struct opool_iterator *itr = owner;
  if (itr->cur)
    {
      itr->cur = itr->cur->hdr.next;
      return 0;
    }
  return EINVAL;
}

static int
opitr_getitem (void *owner, void **pret, const void **pkey)
{
  struct opool_iterator *itr = owner;
  if (!itr->cur)
    return MU_ERR_NOENT;

  *pret = itr->cur->hdr.buf;
  if (pkey)
    *(size_t*) pkey = itr->cur->hdr.level;
  return 0;
}

static int
opitr_finished_p (void *owner)
{
  struct opool_iterator *itr = owner;
  return itr->cur == NULL;
}

static int
opitr_delitem (void *owner, void *item)
{
  struct opool_iterator *itr = owner;
  return (itr->cur && itr->cur->hdr.buf == item) ? 
     MU_ITR_DELITEM_NEXT : MU_ITR_DELITEM_NOTHING;
}

static int
opitr_destroy (mu_iterator_t iterator, void *data)
{
  struct opool_iterator *itr = data;
  if (itr->opool->itr_count == 0)
    {
      /* oops! */
      mu_error (_("%s: INTERNAL ERROR: zero reference count"),
		"opool_destroy");
    }
  else
    itr->opool->itr_count--;
  free (data);
  return 0;
}

static int
opitr_data_dup (void **ptr, void *owner)
{
  struct opool_iterator *itr = owner;

  *ptr = malloc (sizeof (struct opool_iterator));
  if (*ptr == NULL)
    return ENOMEM;
  memcpy (*ptr, owner, sizeof (struct opool_iterator));
  itr->opool->itr_count++;
  return 0;
}

static int
opitr_itrctl (void *owner, enum mu_itrctl_req req, void *arg)
{
  struct opool_iterator *itr = owner;
  switch (req)
    {
    case mu_itrctl_count:
      if (!arg)
	return EINVAL;
      else
	{
	  size_t n = 0;
	  union mu_opool_bucket *p;
	  for (p = itr->opool->bkt_head; p; p = p->hdr.next)
	    n++;
	  *(size_t*)arg = n;
	}
      break;

    default:
      return ENOSYS;
    }
  return 0;
}
	  
int
mu_opool_get_iterator (mu_opool_t opool, mu_iterator_t *piterator)
{
  mu_iterator_t iterator;
  int status;
  struct opool_iterator *itr;

  if (!opool)
    return EINVAL;

  itr = calloc (1, sizeof *itr);
  if (!itr)
    return ENOMEM;
  itr->opool = opool;
  itr->cur = opool->bkt_head;
  
  status = mu_iterator_create (&iterator, itr);
  if (status)
    {
      free (itr);
      return status;
    }

  mu_iterator_set_first (iterator, opitr_first);
  mu_iterator_set_next (iterator, opitr_next);
  mu_iterator_set_getitem (iterator, opitr_getitem);
  mu_iterator_set_finished_p (iterator, opitr_finished_p);
  mu_iterator_set_delitem (iterator, opitr_delitem);
  mu_iterator_set_destroy (iterator, opitr_destroy);
  mu_iterator_set_dup (iterator, opitr_data_dup);
  mu_iterator_set_itrctl (iterator, opitr_itrctl);
  
  opool->itr_count++;

  *piterator = iterator;
  return 0;
}




  
  
 
