#include "../includes/config.hpp"

/* This class will search .config for the block "server{}" */
configValues::configValues(std::string &configFile){
	parseConfig(configFile);
}

configValues::~configValues(){}

void configValues::initializeKeyWordsVariables(){
	/* Initiate values (can't have more than 1 per block) */
	_howManyListen = 0;
	_howManyHost = 0;
	_howManyServerName = 0;
	_howManyErrorMessage = 0;
	_howManyClient = 0;
	_howManyRoot = 0;
	_howManyIndex = 0;

	_howManyIndex_location = 0;
	_howManyAllow_methods = 0;
	_howManyUpload_store = 0;
	_howManyCgi_pass = 0;
	_howManyCgi_path = 0;
	_howManyCgi_ext = 0;
	_howManyRoot_location = 0;
	_howManyAutoindex = 0;
}

void configValues::defaultPreConfigs(){
	/* Default configurations */
		_listen = "80"; 					// Default port
    	_host = "0.0.0.0"; 					// All interfaces
    	_serverName = "";
    	_errorPage = "";
    	_clientMaxBodySize = "1024";
    	_root = "./www";
    	_index = "index.html";

    	_location_index = "";
    	_location_allow_methods = "";
    	_location_upload_store = "";
    	_location_cgi_pass = "";
    	_location_cgi_path = "";
    	_location_cgi_ext = "";
    	_location_root = "./www";
    	_location_autoindex = false;      	 // off

		//initializeKeyWordsVariables();
}

void configValues::defaultConfigs(int _howManyListen, int _howManyHost){
	if (_howManyListen == 0)
		_listen = "80";
	if (_listen == "8888" && _howManyHost == 0)
		_host = "0.0.0.0";

	/* Check if there aren't duplicates */
	if (_howManyListen > 1 || _howManyHost > 1 || _howManyServerName > 1 || _howManyErrorMessage > 1 || _howManyClient > 1 || _howManyRoot > 1 || _howManyIndex > 1){
		std::cerr << "There are duplicates keywords in the configuration file" << std::endl;
		throw std::exception();
	}

	if (_howManyIndex_location == 0)
		_location_index = "index.html";
	if (_howManyAllow_methods == 0)
		_location_allow_methods = "GET";
}

void configValues::isKeyWord(std::string statement){
	std::istringstream iss(statement);
	std::string key;
	iss >> key;

	if (key == "listen"){
		iss >> _listen;
		_howManyListen++;
	}
	else if (key == "host"){
		iss >> _host;
		_howManyHost++;
	}
	else if (key == "server_name"){
		while (iss >> key){
			if (!_serverName.empty()) _serverName += " ";
			_serverName += key;
		}
		_howManyServerName++;
	}
	else if (key == "error_page"){
		while (iss >> key){
			if (!_errorPage.empty()) _errorPage += " ";
			_errorPage += key;
		}
		_howManyErrorMessage++;
	}
	else if (key == "client_max_body_size"){
		iss >> _clientMaxBodySize;
		_howManyClient++;
	}
	else if (key == "root"){
		iss >> _root;
		_howManyRoot++;
	}
	else if (key == "index"){
		iss >> _index;
		while (iss >> key){
			if (!_index.empty()) _index += " ";
			_index += key;
		}
		_howManyIndex++;
	}
	else{
		std::cerr << "Invalid keyword in server block: " << statement << std::endl;
		throw std::exception();
	}
}

bool configValues::detectServerBlock(std::istream& file, std::string& line, bool& insideServerBlock){
	if (line.find("server") == 0 && line.find("{") != std::string::npos){
		std::string beforeBrace = line.substr(0, line.find("{"));
		std::string afterBrace = line.substr(line.find("{") + 1);

		size_t end = beforeBrace.find_last_not_of(" \t");
		if (end != std::string::npos)
		    beforeBrace.erase(end + 1);
		else
		    beforeBrace.clear(); // the string is all whitespace

		beforeBrace.erase(beforeBrace.find_last_not_of(" \t") + 1); //problem is here
		afterBrace.erase(0, afterBrace.find_first_not_of(" \t"));
		afterBrace.erase(afterBrace.find_last_not_of(" \t") + 1);

		if (beforeBrace == "server"){
			if (!insideServerBlock){
				insideServerBlock = true;
				if (!afterBrace.empty())
					line = afterBrace;
				else
					line.clear();
				return true;
			}
			else{
				std::cout << "Can't have a server{} inside another server!" << std::endl;
				throw std::exception();
			}
		}
		else{
			std::cout << "Invalid content before '{' in server declaration: " << line << std::endl;
			throw std::exception();
		}
	}
	else if (line.find("server") == 0 && line.find("server_name") != 0){
		std::istringstream iss(line);
		std::string keyword, invalid;
		iss >> keyword >> invalid;

		if (!invalid.empty()){
			std::cout << "Invalid text after server's keyword/s: " << line << std::endl;
			throw std::exception();
		}

		std::string nextLine;
		while (std::getline(file, nextLine)){
			nextLine.erase(0, nextLine.find_first_not_of(" \t"));
			nextLine.erase(nextLine.find_last_not_of(" \t") + 1);

			if (nextLine.empty())
				continue;

			if (nextLine[0] == '{'){
				if (!insideServerBlock){
					insideServerBlock = true;

					std::string restOfLine = nextLine.substr(1);
					restOfLine.erase(0, restOfLine.find_first_not_of(" \t"));
					restOfLine.erase(restOfLine.find_last_not_of(" \t") + 1);

					if (!restOfLine.empty())
						line = restOfLine;
					else
						line.clear();
					return true;
				}
				else{
					std::cout << "Can't have a server block inside a server block: " << line << std::endl;
					throw std::exception();
				}
			}
			throw std::exception();
		}
	}
	return false; // Not a server block
}

void configValues::parseConfig(const std::string& configFile){
    defaultPreConfigs();

    std::ifstream file(configFile.c_str());
    if (!file.is_open()){
        std::cerr << "Error: Unable to open config file: " << configFile << std::endl;
        return ;
    }

    std::string line;
    bool insideServerBlock = false;

    while (std::getline(file, line)){
        // Remove leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Remove comments
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);

        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty())
            continue;

        if (detectServerBlock(file, line, insideServerBlock)){
            if (line.empty())
                continue;
        }

        if (line.find('}') != std::string::npos){
            if (!insideServerBlock){
                std::cerr << "Invalid '}' outside of server block: " << line << std::endl;
                throw std::exception();
            }

            std::string beforeBrace = line.substr(0, line.find('}'));
            std::string afterBrace = line.substr(line.find('}') + 1);

            // Trim both parts
            beforeBrace.erase(0, beforeBrace.find_first_not_of(" \t"));
            beforeBrace.erase(beforeBrace.find_last_not_of(" \t") + 1);
            afterBrace.erase(0, afterBrace.find_first_not_of(" \t"));
            afterBrace.erase(afterBrace.find_last_not_of(" \t") + 1);

            if (!beforeBrace.empty()){
                std::stringstream ss(beforeBrace);
                std::string statement;

                while (std::getline(ss, statement, ';')){
                    statement.erase(0, statement.find_first_not_of(" \t"));
                    statement.erase(statement.find_last_not_of(" \t") + 1);
                    if (statement.empty())
                        continue;

                    isKeyWord(statement);
                }
            }
            insideServerBlock = false; // Only this!
            if (!afterBrace.empty())
                line = afterBrace; // re-parsed
            else
                continue;
        }

        if (!insideServerBlock){
            std::cout << "Invalid text outside server's block: " << line << std::endl;
            throw std::exception();
        }

        // Handle location block
        if (line.find("location") == 0 && line.find("{") != std::string::npos) {
            parseLocatePart(file, line, line);
            continue;
        }

        // Split by ';'
        std::stringstream ss(line);
        std::string statement;
        while (std::getline(ss, statement, ';')) {
            statement.erase(0, statement.find_first_not_of(" \t"));
            statement.erase(statement.find_last_not_of(" \t") + 1);

            if (statement.empty())
                continue;
            isKeyWord(statement);
        }
    }

    // At the end, apply defaults/checks only once for the server block you have
    defaultConfigs(_howManyListen, _howManyHost);

    file.close();
}

/* Take info from .config and store it in this class */
//void configValues::parseConfig(const std::string& configFile){
//	defaultPreConfigs();
//
//    std::ifstream file(configFile.c_str());
//    if (!file.is_open()){
//        std::cerr << "Error: Unable to open config file: " << configFile << std::endl;
//        return ;
//    }
//
//    std::string line;
//    bool insideServerBlock = false;
//
//    while (std::getline(file, line)){
//        /* Remove leading/trailing whitespace */
//        line.erase(0, line.find_first_not_of(" \t"));
//        line.erase(line.find_last_not_of(" \t") + 1);
//
//        /* Skip empty lines and comments */
//		size_t commentPos = line.find('#');
//		if (commentPos != std::string::npos)
//		    line = line.substr(0, commentPos);
//
//		line.erase(0, line.find_first_not_of(" \t"));
//		line.erase(line.find_last_not_of(" \t") + 1);
//
//		// Now skip if empty
//		if (line.empty())
//		    continue;
//
//        /* Detect start of server block */
//		if (detectServerBlock(file, line, insideServerBlock)){
//			if (line.empty())
//				continue;
//		}
//
//		if (line.find('}') != std::string::npos){
//			if (!insideServerBlock){
//				std::cerr << "Invalid '}' outside of server block: " << line << std::endl;
//				throw std::exception();
//			}
//
//			std::string beforeBrace = line.substr(0, line.find('}'));
//			std::string afterBrace = line.substr(line.find('}') + 1);
//		
//			// Trim both parts
//			beforeBrace.erase(0, beforeBrace.find_first_not_of(" \t"));
//			beforeBrace.erase(beforeBrace.find_last_not_of(" \t") + 1);
//			afterBrace.erase(0, afterBrace.find_first_not_of(" \t"));
//			afterBrace.erase(afterBrace.find_last_not_of(" \t") + 1);
//		
//			// First, parse the config that comes before the closing brace
//			if (!beforeBrace.empty()){
//				std::stringstream ss(beforeBrace);
//				std::string statement;
//
//				if (line[line.length() - 2] != ';'){
//					std::cerr << "Missing semicolon in server's block at the end of: " << line << std::endl;
//					throw std::exception();
//				}
//				while (std::getline(ss, statement, ';')){
//					statement.erase(0, statement.find_first_not_of(" \t"));
//					statement.erase(statement.find_last_not_of(" \t") + 1);
//					if (statement.empty())
//						continue;
//				
//					isKeyWord(statement);
//				}
//			}
//			// Finalize current server block
//			defaultConfigs(_howManyListen, _howManyHost);
//			initializeKeyWordsVariables(); // For new block
//			insideServerBlock = false;
//
//			// Continue parsing after the }
//			if (!afterBrace.empty())
//				line = afterBrace; // re-parsed
//			else
//				continue;
//		}
//
//        if (!insideServerBlock){
//			std::cout << "Invalid text outside server's block: " << line << std::endl;
//			throw std::exception();
//		}
//		
//		std::cout << "[DEBUG] Statement: '" << line << "'" << std::endl;
//		if (line.find("location") == 0 && line.find("{") != std::string::npos) {
//		    parseLocatePart(file, line, line);
//		    continue;
//		}
//		// Split by ';'
//
//		while (std::getline(file, line)){
//		    // Remove comments and whitespace as before
//		    line.erase(0, line.find_first_not_of(" \t"));
//		    line.erase(line.find_last_not_of(" \t") + 1);
//
//		    size_t commentPos = line.find('#');
//		    if (commentPos != std::string::npos)
//		        line = line.substr(0, commentPos);
//		    line.erase(0, line.find_first_not_of(" \t"));
//		    line.erase(line.find_last_not_of(" \t") + 1);
//
//		    if (line.empty())
//		        continue;
//		    if (detectServerBlock(file, line, insideServerBlock)) {
//		        if (line.empty())
//		            continue;
//		    }
//		    if (!insideServerBlock){
//		        std::cout << "Invalid text outside server's block: " << line << std::endl;
//		        throw std::exception();
//		    }
//
//		    // 1. **HANDLE LOCATION IMMEDIATELY IF IT APPEARS**
//		    if (line.find("location") == 0 && line.find("{") != std::string::npos) {
//		        parseLocatePart(file, line, line);
//		        continue;
//		    }
//
//		    // 2. **HANDLE CLOSING BRACE**
//		    if (line.find('}') != std::string::npos) {
//		        std::string beforeBrace = line.substr(0, line.find('}'));
//		        std::string afterBrace = line.substr(line.find('}') + 1);
//			
//		        // Remove whitespace
//		        beforeBrace.erase(0, beforeBrace.find_first_not_of(" \t"));
//		        beforeBrace.erase(beforeBrace.find_last_not_of(" \t") + 1);
//		        afterBrace.erase(0, afterBrace.find_first_not_of(" \t"));
//		        afterBrace.erase(afterBrace.find_last_not_of(" \t") + 1);
//			
//		        // ***RECURSIVELY HANDLE WHAT'S BEFORE THE BRACE***
//		        if (!beforeBrace.empty()) {
//		            // If this part is a location, handle it!
//		            if (beforeBrace.find("location") == 0 && beforeBrace.find("{") != std::string::npos) {
//		                parseLocatePart(file, beforeBrace, beforeBrace);
//		            } else {
//		                std::stringstream ss(beforeBrace);
//		                std::string statement;
//		                while (std::getline(ss, statement, ';')) {
//		                    statement.erase(0, statement.find_first_not_of(" \t"));
//		                    statement.erase(statement.find_last_not_of(" \t") + 1);
//		                    if (statement.empty())
//		                        continue;
//		                    isKeyWord(statement);
//		                }
//		            }
//		        }
//			
//		        // Finalize current server block
//		        insideServerBlock = false;
//			
//		        // If there's more after the brace, re-parse the line
//		        if (!afterBrace.empty())
//		            line = afterBrace;
//		        else
//		            continue;
//		    }
//		
//		    // 3. **NORMAL KEYWORD PARSING**
//		    std::stringstream ss(line);
//		    std::string statement;
//		    while (std::getline(ss, statement, ';')) {
//		        statement.erase(0, statement.find_first_not_of(" \t"));
//		        statement.erase(statement.find_last_not_of(" \t") + 1);
//		        if (statement.empty())
//		            continue;
//		        isKeyWord(statement);
//		    }
//		}
//		/* std::stringstream ss(line);
//		std::string statement;
//		while (std::getline(ss, statement, ';')) {
//		    statement.erase(0, statement.find_first_not_of(" \t"));
//		    statement.erase(statement.find_last_not_of(" \t") + 1);
//		
//		    if (statement.empty())
//		        continue;
//		    if (statement.empty() || line.find(statement + ";") == std::string::npos) {
//		        std::cerr << "Missing semicolon in Server's block at the end of: " << statement << std::endl;
//		        throw std::exception();
//		    }
//		    isKeyWord(statement);
//		} */
//    }
//	if (insideServerBlock == 1){
//		std::cout << "Server or location close brackets (}) missing" << std::endl;
//		throw std::exception();
//	}
//    file.close();
//}
//istream can read both files and strings


/* Getters */
std::string configValues::get_listen() const{return _listen;}
std::string configValues::get_host() const{return _host;}
std::string configValues::get_serverName() const{return _serverName;}
std::string configValues::get_errorPage() const{return _errorPage;}
std::string configValues::get_clientMaxBodySize() const{return _clientMaxBodySize;}
std::string configValues::get_root() const{return _root;}
std::string configValues::get_index() const{return _index;}

std::string configValues::get_location_index() const{return _location_index;}
std::string configValues::get_location_allow_methods() const{return _location_allow_methods;}
std::string configValues::get_location_upload_store() const{return _location_upload_store;}
std::string configValues::get_location_cgi_pass() const{return _location_cgi_pass;}
std::string configValues::get_location_cgi_path() const{return _location_cgi_path;}
std::string configValues::get_location_cgi_ext() const{return _location_cgi_ext;}
std::string configValues::get_location_root() const{return _location_root;}
bool configValues::get_location_autoindex() const{return _location_autoindex;}