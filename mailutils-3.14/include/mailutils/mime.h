/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2022 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General
   Public License along with this library.  If not, see 
   <http://www.gnu.org/licenses/>. */

#ifndef _MAILUTILS_MIME_H
#define _MAILUTILS_MIME_H

#include <mailutils/types.h>

/* mime flags */
#define MU_MIME_MULTIPART_MIXED	    0x1
#define MU_MIME_MULTIPART_ALT       0x2

#ifdef __cplusplus
extern "C" {
#endif

struct mu_mime_param
{
  char *lang;
  char *cset;
  char *value;
};

#define MU_MIME_CONTENT_TYPE_MULTIPART      "multipart"
#define MU_MIME_CONTENT_SUBTYPE_MIXED       "mixed"
#define MU_MIME_CONTENT_SUBTYPE_ALTERNATIVE "alternative"
#define MU_MIME_CONTENT_SUBTYPE_DIGEST      "digest"
  
int mu_mime_create	(mu_mime_t *pmime, mu_message_t msg, int flags);
int mu_mime_create_multipart (mu_mime_t *pmime, char const *subtype,
			      mu_assoc_t param);
  
void mu_mime_destroy	(mu_mime_t *pmime);
void mu_mime_ref        (mu_mime_t mime);
void mu_mime_unref      (mu_mime_t mime);
	
int mu_mime_is_multipart	(mu_mime_t mime);
int mu_mime_get_num_parts	(mu_mime_t mime, size_t *nparts);

int mu_mime_sget_content_type (mu_mime_t mime, const char **value);
int mu_mime_aget_content_type (mu_mime_t mime, char **value);
int mu_mime_sget_content_subtype (mu_mime_t mime, const char **value);
int mu_mime_aget_content_subtype (mu_mime_t mime, char **value);
int mu_mime_content_type_get_param (mu_mime_t mime, char const *name,
				    const char **value);
int mu_mime_content_type_set_param (mu_mime_t mime, char const *name,
				    const char *value);
  
int mu_mime_get_part	(mu_mime_t mime, size_t part, mu_message_t *msg);

int mu_mime_add_part	(mu_mime_t mime, mu_message_t msg);

int mu_mime_get_message	(mu_mime_t mime, mu_message_t *msg);
int mu_mime_to_message (mu_mime_t mime, mu_message_t *msg);

int mu_rfc2047_decode   (const char *tocode, const char *fromstr, 
                         char **ptostr);

int mu_rfc2047_encode   (const char *charset, const char *encoding, 
			 const char *text, char **result);
int mu_rfc2047_decode_param (const char *tocode, const char *input,
			     struct mu_mime_param **param);
void mu_mime_param_free (struct mu_mime_param *p);

int mu_base64_encode    (const unsigned char *input, size_t input_len,
			 unsigned char **output, size_t * output_len);

int mu_base64_decode    (const unsigned char *input, size_t input_len,
			 unsigned char **output, size_t * output_len);


int mu_mime_param_assoc_create (mu_assoc_t *passoc);
int mu_mime_param_assoc_add (mu_assoc_t assoc, const char *name);
  
int mu_mime_header_parse (const char *text, const char *charset, char **pvalue,
			  mu_assoc_t *paramtab);
int mu_mime_header_parse_subset (const char *text, const char *charset,
				 char **pvalue,
				 mu_assoc_t assoc);

int mu_mime_header_set_w (mu_header_t hdr, const char *name,
			  const char *value, mu_assoc_t params,
			  size_t line_width);
int mu_mime_header_set (mu_header_t hdr, const char *name,
			const char *value, mu_assoc_t params);

#ifdef __cplusplus
}
#endif

#endif /* _MAILUTILS_MIME_H */
