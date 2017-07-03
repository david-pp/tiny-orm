#ifndef __COMMON_URL_H
#define __COMMON_URL_H

#include <string>
#include <map>

/*
 * parse url like this
 *
 * schema://username:password@host:port/path?key=value#fragment
 * \____/   \______/ \______/ \__/ \__/ \__/ \_______/ \______/
 *   |         |        |       |    |    |      |         |
 * schema      |     password   |   port  |    query    fragment
 *          username          host      path
 *
 * note:
 *   - username, password, port, path, query, fragment is optional.
 *   - scheme, host must be setting.
 *   - username and password must be paired.
 *
 */

struct URL
{
public:
	typedef std::map<std::string, std::string> QueryMap;

	URL() { reset(); }
	void reset();
	bool parse(const std::string& url);
	std::string make();

public:
	std::string schema;
	std::string username;
	std::string password;
	std::string host;
	int port;
	std::string path;
	QueryMap query;
	std::string fragment;
};

#endif // __COMMON_URL_H