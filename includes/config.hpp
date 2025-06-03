#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "webserv.hpp"
#include <fstream>
#include <sstream>
#include <map>

class configValues{
	private:
		std::string _listen;
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
    bool _location_autoindex;

    int _howManyListen, _howManyHost, _howManyServerName, _howManyErrorMessage, _howManyClient, _howManyRoot, _howManyIndex; // Check for douplicate
    int _howManyIndex_location, _howManyAllow_methods, _howManyUpload_store, _howManyCgi_pass, _howManyCgi_path, _howManyCgi_ext, _howManyRoot_location, _howManyAutoindex;

		//std::vector<int> _serverFD; //_listen

		void parseConfig(const std::string& configFile);
		void parseLocatePart(std::ifstream &file, std::string &line, std::string locationLine);
    void isKeyWord(std::string statement);
    void isKeyWordLocationPart(std::string statement);

    void defaultPreConfigs(); //Default values or NULL
    void initializeKeyWordsVariables();
    void defaultConfigs(int isThereA_listen, int isThereA_host); // Default values for listen & host. Check if there aren't douplicate keywords

    bool detectServerBlock(std::ifstream& file, std::string& line, bool& insideServerBlock); //Check if it's inside the server
    void LocationPart(); // Parse location{}

	public:
		configValues(std::string &configFile);
		~configValues();
		
		std::string get_listen() const;
		std::string get_host() const;
		std::string get_serverName() const;
		std::string get_errorPage() const;
		std::string get_clientMaxBodySize() const;
		std::string get_root() const;
		std::string get_index() const;

    std::string get_location_index() const;
    std::string get_location_allow_methods() const;
    std::string get_location_upload_store() const;
    std::string get_location_cgi_pass() const;
    std::string get_location_cgi_path() const;
    std::string get_location_cgi_ext() const;
    std::string get_location_root() const;
    bool get_location_autoindex() const;

};

#endif
