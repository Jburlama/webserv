#include "../includes/config.hpp"

/* This class will search .config for the block "server{}" */
configValues::configValues(std::string &configFile){
	parseConfig(configFile);
}

configValues::~configValues(){}

void configValues::parseLocatePart(std::ifstream &file, std::string &line, const std::string firstLine){
	/* Create a loop that will give this function the line I need */
	bool insideLocationBlock = false;

	/* Detect start of location block: */
	if (firstLine.find("location") == 0 && firstLine.find("{") != std::string::npos){
		std::istringstream iss(firstLine);
		std::string first, second, third;
		iss >> first >> second >> third;
		(void)second;

		if (first == "location" && third == "{"){
			if (insideLocationBlock == false)
				insideLocationBlock = true;
			else {
				std::cout << "Can't have a location{} inside another location!" << line << std::endl;
				throw std::exception();
			}
		}
		else{
			std::cout << "Invalid syntax: unexpected token(s) after 'location'" << line << std::endl;
			throw std::exception();
		}
	}

	while (std::getline(file, line) && insideLocationBlock == true){
        /* Remove leading/trailing whitespace */
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        /* Skip empty lines and comments */
        if (line.empty() || line[0] == '#')
			continue;

		if (line == "}"){
            insideLocationBlock = false;
            break; //Stop parsing after first server block
        }

		std::istringstream iss(line); //splits a line into words/tokens
        std::string key;
        iss >> key;

        if (key == "index"){
    		iss >> _location_index;
			while (iss >> key){
    		    if (!_location_index.empty())
    		        _location_index += " ";
    		    _location_index += key;
    		}
		}
		else if (key == "allow_methods"){
    		while (iss >> key){
    		    if (!_location_allow_methods.empty())
    		        _location_allow_methods += " ";
    		    _location_allow_methods += key;
    		}
		}
		else if (key == "upload_store"){
		    iss >> _location_upload_store;
		}
		else if (key == "cgi_pass"){
			while (iss >> key){
    		    if (!_location_cgi_pass.empty())
    		        _location_cgi_pass += " ";
    		    _location_cgi_pass += key;
    		}
		}
		else if (key == "cgi_path"){
			while (iss >> key){
    		    if (!_location_cgi_path.empty())
    		        _location_cgi_path += " ";
    		    _location_cgi_path += key;
    		}
		}
		else if (key == "cgi_ext"){
			while (iss >> key){
    		    if (!_location_cgi_ext.empty())
    		        _location_cgi_ext += " ";
    		    _location_cgi_ext += key;
    		}
		}
		else if (key == "root"){
		    iss >> _location_root;
		}
		else if (key == "autoindex"){
    		std::string value;
    		iss >> value;
    		if (value == "on;")
    		    _location_autoindex = true;
    		else if (value == "off;")
    		    _location_autoindex = false;
    		else
    		    throw std::runtime_error("Invalid value for location_autoindex: expected 'on' or 'off'");
		}
	}
}

void configValues::defaultPreConfigs(){
	_listen = "80"; 					// Default port
    _host = "0.0.0.0"; 					// All interfaces
    _serverName = "";
    _errorPage = "";
    _clientMaxBodySize = "1024";
    _root = "./www";
    _index = "index.html";

    _location_index = "index.html";
    _location_allow_methods = "GET";
    _location_upload_store = "";
    _location_cgi_pass = "";
    _location_cgi_path = "";
    _location_cgi_ext = "";
    _location_root = "./www";
    _location_autoindex = false;      	 // off
}

void checkIfInvalidAfterKeyWord(std::string invalid){
	if (!invalid.empty())
	{
		std::cout << "Invalid argument after keyword" << std::endl;				
		throw std::exception();;
	}
}

void configValues::defaultConfigs(int isThereA_listen, int isThereA_host){
	if (isThereA_listen == -1)
		_listen = "80";
	if (_listen == "8888" && isThereA_host == -1)
		_host = "0.0.0.0";
}

/* Take info from .config and store it in this class */
void configValues::parseConfig(const std::string& configFile){
	defaultPreConfigs();
	int isThereA_listen = -1, isThereA_host = -1;

    std::ifstream file(configFile.c_str());
    if (!file.is_open()){
        std::cerr << "Error: Unable to open config file: " << configFile << std::endl;
        return ;
    }

    std::string line;
    bool insideServerBlock = false;

    while (std::getline(file, line)){
        /* Remove leading/trailing whitespace */
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        /* Skip empty lines and comments */
        if (line.empty() || line[0] == '#')
			continue;

        /* Detect start of server block */
		if (line.find("server") == 0 && line.find("{") != std::string::npos){
			std::string beforeBrace = line.substr(0, line.find("{"));
			std::string afterBrace = line.substr(line.find("{") + 1);

			// Trim spaces before and after '{'
			beforeBrace.erase(beforeBrace.find_last_not_of(" \t") + 1);
			afterBrace.erase(0, afterBrace.find_first_not_of(" \t"));
			afterBrace.erase(afterBrace.find_last_not_of(" \t") + 1);

			// Confirm that it's just "server {"
			if (beforeBrace == "server"){
				if (!insideServerBlock){
					insideServerBlock = true;
				
					if (!afterBrace.empty())
						line = afterBrace;
					else
						continue;
				} 
				else {
					std::cout << "Can't have a server{} inside another server!" << std::endl;
					throw std::exception();
				}
			}
			else {
				std::cout << "Invalid content before '{' in server declaration: " << line << std::endl;
				throw std::exception();
			}
		}
		else if (line.find("server") == 0 && line.find("server_name") != 0){
			std::istringstream iss(line);
			std::string keyword, invalid;
			iss >> keyword >> invalid;

			// Check if line is only "server" with optional whitespace
			if (!invalid.empty()) {
				std::cout << "Invalid text after server's keyword/s: " << line << std::endl;
				throw std::exception();
			}
		
			// Look ahead for '{'
			std::string nextLine;
			while (std::getline(file, nextLine)) {
				nextLine.erase(0, nextLine.find_first_not_of(" \t"));
				nextLine.erase(nextLine.find_last_not_of(" \t") + 1);
			
				if (nextLine.empty())
					continue;
			
				if (nextLine[0] == '{') {
					if (!insideServerBlock) {
						insideServerBlock = true;
					
						/* Check if there's anything else after '{' */
						std::string restOfLine = nextLine.substr(1);
						restOfLine.erase(0, restOfLine.find_first_not_of(" \t"));
						restOfLine.erase(restOfLine.find_last_not_of(" \t") + 1);
					
						if (!restOfLine.empty()) {
							line = restOfLine;
						} else {
							line.clear();
						}
						break;
					}
					else {
						std::cout << "Can't have a server block inside a server block: " << line << std::endl;
						throw std::exception();
					}
				}
				throw std::exception();
			}
		
			if (line.empty()) continue; // nothing left to parse after opening block
		}


        /* Detect end of server block */
        if (line == "}"){
			if (insideServerBlock == true){
            	insideServerBlock = false;
            	continue ;
			}
			else{
				std::cout << "Invalid '}': " << line << std::endl;
				throw std::exception();
			}
        }
        if (!insideServerBlock){
			std::cout << line << std::endl;
			std::cout << "Invalid text outside server's block!" << std::endl;
			throw std::exception();
		}
        /* Remove trailing semicolon(;) */                                /* Change later because I can have multiple elements in the same line */ 
        if (!line.empty() && line[line.length() - 1] == ';')
   			line.erase(line.length() - 1);

        std::istringstream iss(line); //splits a line into words/tokens
        std::string key;
        iss >> key;

		if (key == "location")
		{
			const std::string locationLine = line;
			parseLocatePart(file, line, locationLine);
		}
        else if (key == "listen"){
            iss >> _listen;
			isThereA_listen = 1;
		}
		else if (key == "host"){
            iss >> _host;
			isThereA_host = 1;
        }
		else if (key == "server_name"){
			while (iss >> key){
    		    if (!_serverName.empty())
    		        _serverName += " ";
    		    _serverName += key;
    		}
        }
		else if (key == "error_page"){
			while (iss >> key){
    		    if (!_errorPage.empty())
    		        _errorPage += " ";
    		    _errorPage += key;
    		}
		}
		else if (key == "client_max_body_size"){
            iss >> _clientMaxBodySize;
        }
		else if (key == "root"){
            iss >> _root;
        }
		else if (key == "index"){
            iss >> _index;
			while (iss >> key){
    		    if (!_index.empty())
    		        _index += " ";
    		    _index += key;
    		}
        }
		else{
			std::cout << "Invalid argument within a server block: " << line << std::endl;
			throw std::exception();
		}
    }
	defaultConfigs(isThereA_listen, isThereA_host);
	if (insideServerBlock == 1){
		std::cout << "Server or location close brackets (}) missing" << std::endl;
		throw std::exception();
	}
    file.close();
}

/* Getters */
std::string configValues::get_listen() const{
	return _listen;}

std::string configValues::get_host() const{
	return _host;}

std::string configValues::get_serverName() const{
	return _serverName;}

std::string configValues::get_errorPage() const{
	return _errorPage;}

std::string configValues::get_clientMaxBodySize() const{
	return _clientMaxBodySize;}

std::string configValues::get_root() const{
	return _root;}

std::string configValues::get_index() const{
	return _index;}

std::string configValues::get_location_index() const{return _location_index;}
std::string configValues::get_location_allow_methods() const {return _location_allow_methods;}
std::string configValues::get_location_upload_store() const {return _location_upload_store;}
std::string configValues::get_location_cgi_pass() const{return _location_cgi_pass;}
std::string configValues::get_location_cgi_path() const{return _location_cgi_path;}
std::string configValues::get_location_cgi_ext() const{return _location_cgi_ext;}
std::string configValues::get_location_root() const{return _location_root;}
bool configValues::get_location_autoindex() const{return _location_autoindex;}