#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <fstream>
#include <netinet/in.h>
#include <sstream>
#include <map>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <set>

#include <unistd.h>
#include <cerrno>
#include <cstring>

struct LocationBlock{
  std::string path;
  std::string index;											// The default file to serve if a directory is requested. (Example / serves indexABC.html)
  std::string allow_methods;									// HTTP methods allowed for this location (GET, POST, etc.)
  std::string upload_store;										// Directory to store uploaded files for this path
  std::string cgi_pass;											// File extension(s) that should be handled by a CGI (Common Gateway Interface) handler
  std::string cgi_path;											// The actual program(s) to run for CGI scripts. (Example /usr/bin/python3 for Python)
  std::string cgi_ext;											// File extensions that should trigger the CGI, (Example .py, .sh)
  std::string root;												// Filesystem path that corresponds to this URL path (e.g., where to look for content)
  std::string try_files;										// If on, show a directory listing if no index file is found; if off, do not.
  std::string returnLoc;
  //std::string redirect;
  bool autoindex;												// If on, show a directory listing if no index file is found; if off, do not.

  LocationBlock() {};
};

struct ServerBlock{
    int                        fd;
    struct sockaddr_in         addr;
    std::string                listen;
    std::string                host;
    std::string                serverName;
	std::map<int, std::string> errorPage;
    std::string                clientMaxBodySize;
    std::string                root;
    std::string                index;
    std::vector<LocationBlock> locations;

    ServerBlock():listen("8000"),serverName("localhost"),root("/home/jhonas/42/webserv/content/html"),
                index("default.html"){}
};

class configValues{
	private:
    std::vector<ServerBlock> _servers;
    int _numOfLocInSrvBlock;

    int _howManyListen, _howManyHost, _howManyServerName, _howManyErrorMessage, _howManyClient, _howManyRoot, _howManyIndex; // Check for douplicate
    int _howManyIndex_location, _howManyAllow_methods, _howManyUpload_store, _howManyCgi_pass, _howManyCgi_path, _howManyCgi_ext, _howManyRoot_location, _howManyAutoindex, _howManyReturn, _howManyTry_files;

	void parseConfig(const std::string& configFile);
    void parseLocatePart(std::istream &file, std::string &statement, ServerBlock &srv, LocationBlock &loc);
    void isKeyWord(std::string statement, ServerBlock &srv);
    void isKeyWordLocationPart(std::string statement, LocationBlock &loc);

    void defaultPreConfigs(); //Initialize values to 0 at every server block start
    void defaultConfigs(ServerBlock &srv); // Default values for listen & host. Check if there aren't douplicate keywords
    void resetLocationCounters(); //Reset locations counters for duplicate keywords

    bool detectServerBlock(std::istream& file, std::string& line, bool& insideServerBlock); //Check if it's a server block
    bool detectLocationBlock(std::istream& file, std::string& line, bool& insideServerBlock, LocationBlock &loc); //Check if it's a location block

	bool last7Equals(const std::string &configFile);

	public:
		configValues(std::string &configFile);
		~configValues();
		
    // Getters for server's block
	std::string get_listen(int i) const;
    std::string get_host(int i) const;
    std::string get_serverName(int i) const;
    const std::vector<std::string>& get_errorPage(int i) const;
    std::string get_clientMaxBodySize(int i) const;
    std::string get_root(int i) const;
    std::string get_index(int i) const;
    std::vector<ServerBlock> &get_server_blocks() {return this->_servers;};

    // Getters for location's block
	std::string get_location_path(int serverIndex, int locationIndex) const;
    std::string get_location_index(int serverIndex, int locationIndex) const;
    std::string get_location_allow_methods(int serverIndex, int locationIndex) const;
    std::string get_location_upload_store(int serverIndex, int locationIndex) const;
    std::string get_location_cgi_pass(int serverIndex, int locationIndex) const;
    std::string get_location_cgi_path(int serverIndex, int locationIndex) const;
    std::string get_location_cgi_ext(int serverIndex, int locationIndex) const;
    std::string get_location_root(int serverIndex, int locationIndex) const;
    bool get_location_autoindex(int serverIndex, int locationIndex) const;

}; 

#endif
