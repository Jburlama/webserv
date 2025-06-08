#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "webserv.hpp"
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

struct LocationBlock {
  std::string path; //didn't set a getter for path, SELF REMINDER
  std::string index;
  std::string allow_methods;
  std::string upload_store;
  std::string cgi_pass;
  std::string cgi_path;
  std::string cgi_ext;
  std::string root;
  bool autoindex;

  LocationBlock() : path("/"), index("index.html"), allow_methods("GET"), //This might be like allow_methods = "GET GET POST" I can use a function for the defaults like I use too
                    root("./www"), autoindex(false) {}
};

struct ServerBlock {
  std::string listen;
  std::string host;
  std::string serverName;
  std::string errorPage;
  std::string clientMaxBodySize;
  std::string root;
  std::string index;
  std::vector<LocationBlock> locations;

  ServerBlock() : listen("80"), host("0.0.0.0"), serverName(""), clientMaxBodySize("1024"),
                  root("./www"), index("index.html") {}
};

class configValues{
	private:
		/* std::string _listen;
		std::string _host;
		std::string _serverName;
		std::string _errorPage;
		std::string _clientMaxBodySize;
		std::string _root;
		std::string _index; //hello.html

    std::string _location_index;
    std::string _location_allow_methods;
    std::string _location_upload_store;
    std::string _location_cgi_pass;
    std::string _location_cgi_path;
    std::string _location_cgi_ext;
    std::string _location_root;
    bool _location_autoindex; */
    std::vector<ServerBlock> _servers;
    int _numOfLocInSrvBlock;

    int _howManyListen, _howManyHost, _howManyServerName, _howManyErrorMessage, _howManyClient, _howManyRoot, _howManyIndex; // Check for douplicate
    int _howManyIndex_location, _howManyAllow_methods, _howManyUpload_store, _howManyCgi_pass, _howManyCgi_path, _howManyCgi_ext, _howManyRoot_location, _howManyAutoindex;

		void parseConfig(const std::string& configFile);
    void parseLocatePart(std::istream &file, std::string &statement, std::string &line, ServerBlock &srv);
    void isKeyWord(std::string statement, ServerBlock &srv);
    void isKeyWordLocationPart(std::string statement, LocationBlock &loc);

    void defaultPreConfigs(); //Default values or NULL
    void initializeKeyWordsVariables(); //initialize variables
    void defaultConfigs(ServerBlock srv); // Default values for listen & host. Check if there aren't douplicate keywords

    bool detectServerBlock(std::istream& file, std::string& line, bool& insideServerBlock); //Check if it's a server block
    bool detectLocationBlock(std::istream& file, std::string& line, bool& insideServerBlock, LocationBlock &loc); //Check if it's a location block

	public:
		configValues(std::string &configFile);
		~configValues();
		
    // Getters for server's block
	std::string get_listen(int i) const;
    std::string get_host(int i) const;
    std::string get_serverName(int i) const;
    std::string get_errorPage(int i) const;
    std::string get_clientMaxBodySize(int i) const;
    std::string get_root(int i) const;
    std::string get_index(int i) const;

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
