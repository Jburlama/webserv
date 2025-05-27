#include "../includes/config.hpp"

/* This class will search .config for the block "server{}" */
configValues::configValues(std::string &configFile){
	parseConfig(configFile);
}

configValues::~configValues(){}

/* Take info from .config and store it in this class */
void configValues::parseConfig(const std::string& configFile){
    std::ifstream file(configFile.c_str());
    if (!file.is_open()){
        std::cerr << "Error: Unable to open config file: " << configFile << std::endl;
        return;
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
        if (line == "server{"){
            insideServerBlock = true;
            continue;
        }	
		else if (line.find("server") == 0 && line.find("{") != std::string::npos) {
            insideServerBlock = true;
            continue;
        }
		else if (line == "server"){
   			/* Look ahead to see if next line is '{' */
   			//std::streampos prevPos = file.tellg(); // save current position
			std::string nextLine;
			while (std::getline(file, nextLine)) {
        		/* Trim whitespace from nextLine */
        		nextLine.erase(0, nextLine.find_first_not_of(" \t"));
        		nextLine.erase(nextLine.find_last_not_of(" \t") + 1);

				if (nextLine != "{" || nextLine != " " || nextLine != "\n" || nextLine != "\t"){
        			if (nextLine == "{") {
        			    insideServerBlock = true;
						break;
        			}
				}
				else{
					throw std::exception &e ;
				}
  			}
			//file.seekg(prevPos); // Reset to previous position
		}

        /* Detect end of server block */                                  /* When I add location{} this will have to change (or loop location{} without reaching this line until it breaks) */
        if (line == "}"){
            insideServerBlock = false;
            break; //Stop parsing after first server block                /* Trying to figure out how to save the next server block data */
        }
        if (!insideServerBlock)
			continue;

        /* Remove trailing semicolon(;) */
        if (!line.empty() && line[line.length() - 1] == ';')
   		line.erase(line.length() - 1);




        std::istringstream iss(line); //splits a line into words/tokens
        std::string key;
        iss >> key;

        if (key == "listen"){
            std::string value;
            iss >> value;
            _listen = atof(value.c_str());
        }
		else if (key == "host"){
            iss >> _host;
        }
		else if (key == "server_name"){
            iss >> _serverName;
        }
		else if (key == "error_page"){
    		std::string restOfLine;
    		std::getline(iss, restOfLine); // rest of the line after key
    		restOfLine.erase(0, restOfLine.find_first_not_of(" \t")); // trim leading spaces
    		_errorPage = restOfLine;
		}
		else if (key == "client_max_body_size"){
            iss >> _clientMaxBodySize;
        }
		else if (key == "root"){
            iss >> _root;
        }
		else if (key == "index"){
            iss >> _index;
        }
    }
    file.close();
}

/* Getters */
double configValues::get_listen() const{
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