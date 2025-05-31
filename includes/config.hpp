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

		//std::vector<int> _serverFD; //_listen

		void parseConfig(const std::string& configFile);
		void parseLocatePart(std::ifstream &file, std::string &line, std::string locationLine);
    	void defaultPreConfigs();
    	void defaultConfigs(int isThereA_listen, int isThereA_host);

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

/* Example
server {
  listen 8001;                        # listening port, mandatory parameter
  host 127.0.0.1;                     # host or 127.0.0.1 by default
  server_name test;                   # specify server_name, need to be added into /etc/hosts to work
  error_page 404 /error/404.html;     # default error page
  client_max_body_size 1024;          # max request body size in bytes
  root docs/fusion_web/;              # root folder of site directory, full or relative path, mandatory parameter
  index index.html;                   # default page when requesting a directory, index.html by default

  location /tours {                   
      root docs/fusion_web;           # root folder of the location, if not specified, taken from the server. 
                                      # EX: - URI /tours           --> docs/fusion_web/tours
                                      #     - URI /tours/page.html --> docs/fusion_web/tours/page.html 
      autoindex on;                   # turn on/off directory listing
      allow_methods POST GET;         # allowed methods in location, GET only by default
      index index.html;               # default page when requesting a directory, copies root index by default
      return abc/index1.html;         # redirection
      alias  docs/fusion_web;         # replaces location part of URI. 
                                      # EX: - URI /tours           --> docs/fusion_web
                                      #     - URI /tours/page.html --> docs/fusion_web/page.html 
  }

  location cgi-bin {
      root ./;                                                 # cgi-bin location, mandatory parameter
      cgi_path /usr/bin/python3 /bin/bash;                     # location of interpreters installed on the current system, mandatory parameter
      cgi_ext .py .sh;                                         # extensions for executable files, mandatory parameter
  }
} */
