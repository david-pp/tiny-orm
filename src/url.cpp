#include "url.h"
#include <string.h>
#include <iostream>
#include <sstream>

char *
strndup (const char *s, size_t n)
{
    char *result;
    size_t len = strlen (s);

    if (n < len)
        len = n;

    result = (char *) malloc (len + 1);
    if (!result)
        return 0;

    result[len] = '\0';
    return (char *) memcpy (result, s, len);
}


typedef enum
{
   HOST_IPV4,
   HOST_IPV6,
   HOST_DOMAIN
} host_type_t;

typedef struct _url_field
{
	struct Query {
      char *name;
      char *value;
   };

   host_type_t host_type;
   char *href;
   char *schema;
   char *username;
   char *password;
   char *host;
   char *port;
   char *path;
   int query_num;
   Query *query;
   char *fragment;
} url_field_t;


url_field_t *url_parse(const char *str);

void url_free(url_field_t *url);

void url_field_print(url_field_t *url);

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const char *str_hosttype[] = { "host ipv4", "host ipv6", "host domain", NULL };

#if 0
static char *strndup(const char *str, int n)
{
   char *dst;
   if (!str) return NULL;
   if (n < 0) n = strlen(str);
   if (n == 0) return NULL;
   if ((dst = (char *)malloc(n + 1)) == NULL)
      return NULL;
   memcpy(dst, str, n);
   dst[n] = 0;
   return dst;
}
#endif

static int host_is_ipv4(char *str)
{
   if (!str) return 0;
   while (*str)
   {
      if ((*str >= '0' && *str <= '9') || *str == '.')
         str++;
      else
         return 0;
   }
   return 1;
}

void parse_query(url_field_t *url, char *query)
{
   //int length = strlen(query);
   //int offset = 0;
   char *chr;

   chr = strchr(query, '=');
   while (chr)
   {
      if (url->query)
         url->query = (url_field_t::Query*)realloc(url->query, (url->query_num + 1) * sizeof(url_field_t::Query));
      else
         url->query = (url_field_t::Query*)malloc(sizeof(url_field_t::Query));
      url->query[url->query_num].name = strndup(query, chr - query);
      query = chr + 1;
      chr = strchr(query, '&');
      if (chr)
      {
         url->query[url->query_num].value = strndup(query, chr - query);
         url->query_num++;
         query = chr + 1;
         chr = strchr(query, '=');
      }
      else
      {
         url->query[url->query_num].value = strndup(query, -1);
         url->query_num++;
         break;
      }
   }
}
url_field_t *url_parse (const char *str)
{
   const char *pch;
   char *query;
   url_field_t *url;
   query = NULL;
   if ((url = (url_field_t *)malloc(sizeof(url_field_t))) == NULL)
      return NULL;
   memset(url, 0, sizeof(url_field_t));
   if (str && str[0])
   {
      url->href = strndup(str, -1);
      pch = strchr(str, ':');   /* parse schema */
      if (pch && pch[1] == '/' && pch[2] == '/')
      {
         url->schema = strndup(str, pch - str);
         str = pch + 3;
      }
      else
         goto __fail;
      pch = strchr(str, '@');   /* parse user info */
      if (pch)
      {
         pch = strchr(str, ':');
         if (pch)
         {
            url->username = strndup(str, pch - str);
            str = pch + 1;
            pch = strchr(str, '@');
            if (pch)
            {
               url->password = strndup(str, pch - str);
               str = pch + 1;
            }
            else
               goto __fail;
         }
         else
            goto __fail;
      }
      if (str[0] == '[')        /* parse host info */
      {
         str++;
         pch = strchr(str, ']');
         if (pch)
         {
            url->host = strndup(str, pch - str);
            str = pch + 1;
            if (str[0] == ':')
            {
               str++;
               pch = strchr(str, '/');
               if (pch)
               {
                  url->port = strndup(str, pch - str);
                  str = pch + 1;
               }
               else
               {
                  url->port = strndup(str, -1);
                  str = str + strlen(str);
               }
            }
            url->host_type = HOST_IPV6;
         }
         else
            goto __fail;
      }
      else
      {
         const char *pch_slash;
         pch = strchr(str, ':');
         pch_slash = strchr(str, '/');
         if (pch && (!pch_slash || (pch_slash && pch<pch_slash)))
         {
            url->host = strndup(str, pch - str);
            str = pch + 1;
            pch = strchr(str, '/');
            if (pch)
            {
               url->port = strndup(str, pch - str);
               str = pch + 1;
            }
            else
            {
               url->port = strndup(str, -1);
               str = str + strlen(str);
            }
         }
         else
         {
            pch = strchr(str, '/');
            if (pch)
            {
               url->host = strndup(str, pch - str);
               str = pch + 1;
            }
            else
            {
               url->host = strndup(str, -1);
               str = str + strlen(str);
            }
         }
         url->host_type = host_is_ipv4(url->host) ? HOST_IPV4 : HOST_DOMAIN;
      }
      if (str[0])               /* parse path, query and fragment */
      {
         pch = strchr(str, '?');
         if (pch)
         {
            url->path = strndup(str, pch - str);
            str = pch + 1;
            pch = strchr(str, '#');
            if (pch)
            {
               query = strndup(str, pch - str);
               str = pch + 1;
               url->fragment = strndup(str, -1);
            }
            else
            {
               query = strndup(str, -1);
               str = str + strlen(str);
            }
            parse_query(url, query);
            free(query);
         }
         else
         {
            pch = strchr(str, '#');
            if (pch)
            {
               url->path = strndup(str, pch - str);
               str = pch + 1;
               url->fragment = strndup(str, -1);
               str = str + strlen(str);
            }
            else
            {
               url->path = strndup(str, -1);
               str = str + strlen(str);
            }
         }
      }
   }
   else
   {
__fail:
      url_free(url);
      return NULL;
   }
   return url;
}

void url_free(url_field_t *url)
{
   if (!url) return;
   if (url->href) free(url->href);
   if (url->schema) free(url->schema);
   if (url->username) free(url->username);
   if (url->password) free(url->password);
   if (url->host) free(url->host);
   if (url->port) free(url->port);
   if (url->path) free(url->path);
   if (url->query)
   {
      int i;
      for (i = 0; i < url->query_num; i++)
      {
         free(url->query[i].name);
         free(url->query[i].value);
      }
      free(url->query);
   }
   if (url->fragment) free(url->fragment);
   free(url);
}

void url_field_print(url_field_t *url)
{
   if (!url) return;
   fprintf(stdout, "\nurl field:\n");
   fprintf(stdout, "  - href:     '%s'\n", url->href);
   fprintf(stdout, "  - schema:   '%s'\n", url->schema);
   if (url->username)
      fprintf(stdout, "  - username: '%s'\n", url->username);
   if (url->password)
      fprintf(stdout, "  - password: '%s'\n", url->password);
   fprintf(stdout, "  - host:     '%s' (%s)\n", url->host, str_hosttype[url->host_type]);
   if (url->port)
      fprintf(stdout, "  - port:     '%s'\n", url->port);
   if (url->path)
   fprintf(stdout, "  - path:     '%s'\n", url->path);
   if (url->query_num > 0)
   {
      int i;
      fprintf(stdout, "  - query\n");
      for (i = 0; i < url->query_num; i++)
      {
         fprintf(stdout, "    * %s : %s\n", url->query[i].name, url->query[i].value);
      }
   }
   if (url->fragment)
      fprintf(stdout, "  - fragment: '%s'\n", url->fragment);
}


/////////////////////////////////////////////////////

void URL::reset()
{
	schema = "";
	username = "";
	password = "";
	host = "";
	port = 0;
	path = "";
	fragment = "";
	query.clear();
}

bool URL::parse(const std::string& urltext)
{
	reset();

	url_field_t *url = url_parse(urltext.c_str());
	if (url)
	{
		if (url->schema) 
			this->schema = url->schema;
		if (url->username)
			this->username = url->username;
		if (url->password)
			this->password = url->password;
		if (url->host)
			this->host = url->host;
		if (url->port)
			this->port = atol(url->port);
		if (url->path)
			this->path = url->path;
		if (url->fragment)
			this->fragment = url->fragment;

		for (int i = 0; i < url->query_num; i++)
			this->query[url->query[i].name] = url->query[i].value;

	    url_free(url);
	    return true;
	}

	return false;
}

std::string URL::make()
{
	// schema://username:password@host:port/path?key=value#fragment
	std::ostringstream oss;
	oss << schema << "://";
	if (username.size() > 0 && password.size() > 0)
		oss << username << ":" << password << "@";
	oss << host;
	if (port > 0)
		oss << ":" << port;
	oss << "/";
	oss << path;
	if (query.size())
		oss << "?";
	for (QueryMap::iterator it = query.begin(); it != query.end(); ++ it)
		oss << it->first << "=" << it->second << "&";
	if (fragment.size())
		oss << "#" << fragment;

	return oss.str();
}

/////////////////////////////////////////////////////

#ifdef _TEST_FOR_URL

int main(int argc, const char* argv[])
{
	if (argc < 2)
	{
		const char *str[] = {
	      "scheme://0.0.0.0",
	      "http://username:password@[::1]:8080/index.html",
	      "scheme://username:password@www.google.com",
	      "scheme://host:port",
	      "scheme://host:port/path?id=1&method=get#fragment",
	      "scheme://host/path/to/subpath#fragment",
	      "scheme://username:password@host/path?name=test#fragment",
	      "scheme://username:password@host:port/path?name=test#fragment",
	      NULL
	   };

	   for (int i = 0; str[i]; i++)
	   {
		   	URL url;
			if (url.parse(str[i]))
				std::cout << url.make() << std::endl;
	   }
		return 1;
	}

	URL url;
	if (url.parse(argv[1]))
	{
		std::cout << url.make() << std::endl;
	}
}

#endif