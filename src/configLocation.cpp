#include "config.hpp"

void configValues::isKeyWordLocationPart(std::string statement){
	std::istringstream iss(statement); //splits a line into words/tokens
    std::string key;
    iss >> key;

    if (key == "index"){
    	iss >> _location_index;
		while (iss >> key){
    	    if (!_location_index.empty())if (key == "index"){
    		iss >> _location_index;
			while (iss >> key){
    		    if (!_location_index.empty())
    		        _location_index += " ";
    		    _location_index += key;
    		}
			_howManyIndex_location++;
		}
		else if (key == "allow_methods"){
    		while (iss >> key){
    		    if (!_location_allow_methods.empty())
    		        _location_allow_methods += " ";
    		    _location_allow_methods += key;
    		}
			_howManyAllow_methods++;
		}
		else if (key == "upload_store"){
		    iss >> _location_upload_store;
			_howManyUpload_store++;
		}
		else if (key == "cgi_pass"){
			while (iss >> key){
    		    if (!_location_cgi_pass.empty())
    		        _location_cgi_pass += " ";
    		    _location_cgi_pass += key;
    		}
			_howManyCgi_pass++;
		}
		else if (key == "cgi_path"){
			while (iss >> key){
    		    if (!_location_cgi_path.empty())
    		        _location_cgi_path += " ";
    		    _location_cgi_path += key;
    		}
			_howManyCgi_path++;
		}
		else if (key == "cgi_ext"){
			while (iss >> key){
    		    if (!_location_cgi_ext.empty())
    		        _location_cgi_ext += " ";
    		    _location_cgi_ext += key;
    		}
			_howManyCgi_ext++;
		}
		else if (key == "root"){
		    iss >> _location_root;
			_howManyRoot_location++;
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
			_howManyAutoindex++;
		}
    	        _location_index += " ";
    	    _location_index += key;
    	}
		_howManyIndex_location++;
	}
	else if (key == "allow_methods"){
    	while (iss >> key){
    	    if (!_location_allow_methods.empty())
    	        _location_allow_methods += " ";
    	    _location_allow_methods += key;
    	}
		_howManyAllow_methods++;
	}
	else if (key == "upload_store"){
	    iss >> _location_upload_store;
		_howManyUpload_store++;
	}
	else if (key == "cgi_pass"){
		while (iss >> key){
    	    if (!_location_cgi_pass.empty())
    	        _location_cgi_pass += " ";
    	    _location_cgi_pass += key;
    	}
		_howManyCgi_pass++;
	}
	else if (key == "cgi_path"){
		while (iss >> key){
    	    if (!_location_cgi_path.empty())
    	        _location_cgi_path += " ";
    	    _location_cgi_path += key;
    	}
		_howManyCgi_path++;
	}
	else if (key == "cgi_ext"){
		while (iss >> key){
    	    if (!_location_cgi_ext.empty())
    	        _location_cgi_ext += " ";
    	    _location_cgi_ext += key;
    	}
		_howManyCgi_ext++;
	}
	else if (key == "root"){
	    iss >> _location_root;
		_howManyRoot_location++;
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
		_howManyAutoindex++;
	}
}

bool detectLocationBlock(std::ifstream& file, std::string& line, bool& insideLocationBlock){
	if (line.find("location") == 0 && line.find("{") != std::string::npos){
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

		if (beforeBrace == "location"){
			if (!insideLocationBlock){
				insideLocationBlock = true;
				if (!afterBrace.empty())
					line = afterBrace;
				else
					line.clear();
				return true;
			}
			else{
				std::cout << "Can't have a location{} inside another location!" << std::endl;
				throw std::exception();
			}
		}
		else{
			std::cout << "Invalid content before '{' in location declaration: " << line << std::endl;
			throw std::exception();
		}
	}
	else if (line.find("location") == 0){
		std::istringstream iss(line);
		std::string keyword, invalid;
		iss >> keyword >> invalid;

		if (!invalid.empty()){
			std::cout << "Invalid text after location's keyword/s: " << line << std::endl;
			throw std::exception();
		}

		std::string nextLine;
		while (std::getline(file, nextLine)){
			nextLine.erase(0, nextLine.find_first_not_of(" \t"));
			nextLine.erase(nextLine.find_last_not_of(" \t") + 1);

			if (nextLine.empty())
				continue;

			if (nextLine[0] == '{'){
				if (!insideLocationBlock){
					insideLocationBlock = true;

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
					std::cout << "Can't have a location block inside a location block: " << line << std::endl;
					throw std::exception();
				}
			}
			throw std::exception();
		}
	}
	return false; // Not a location block   
}


void configValues::parseLocatePart(std::ifstream &file, std::string &line, const std::string firstLine){
	/* Create a loop that will give this function the line I need */
	bool insideLocationBlock = false;

    /* Skip empty lines and comments */
	size_t commentPos = line.find('#');
	if (commentPos != std::string::npos)
		line = line.substr(0, commentPos);

	line.erase(0, line.find_first_not_of(" \t"));
	line.erase(line.find_last_not_of(" \t") + 1);

	/* Detect start of location block: */
	if (firstLine.find("location") == 0 && firstLine.find("{") != std::string::npos){
		std::istringstream iss(firstLine);
		std::string first, second, third;
		iss >> first >> second >> third;
		(void)second;

		if (first == "location" && third == "{"){
			if (insideLocationBlock == false)
				insideLocationBlock = true;
			else{
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
        /* Skip empty lines and comments */
	    size_t commentPos = line.find('#');
	    if (commentPos != std::string::npos)
	    	line = line.substr(0, commentPos);

	    line.erase(0, line.find_first_not_of(" \t"));
	    line.erase(line.find_last_not_of(" \t") + 1);

		if (line.empty())
		    continue;

        /* Detect start of server block */
		if (detectLocationBlock(file, line, insideLocationBlock)){
			if (line.empty())
				continue;
		}

		if (line.find('}') != std::string::npos){
			if (!insideLocationBlock){
				std::cerr << "Invalid '}' outside of server block: " << line << std::endl;
				throw std::exception();
			}
		
			std::string beforeBrace = line.substr(0, line.find('}'));
			std::string afterBrace = line.substr(line.find('}') + 1);
		
			beforeBrace.erase(0, beforeBrace.find_first_not_of(" \t"));
			beforeBrace.erase(beforeBrace.find_last_not_of(" \t") + 1);
			afterBrace.erase(0, afterBrace.find_first_not_of(" \t"));
			afterBrace.erase(afterBrace.find_last_not_of(" \t") + 1);
		
			// First, parse the config that comes before the closing brace
			if (!beforeBrace.empty()){
				std::stringstream ss(beforeBrace);
				std::string statement;

				if (line[line.length() - 2] != ';'){
					std::cerr << "Missing semicolon at the end of: " << line << std::endl;
					throw std::exception();
				}
				while (std::getline(ss, statement, ';')){
					statement.erase(0, statement.find_first_not_of(" \t"));
					statement.erase(statement.find_last_not_of(" \t") + 1);
					if (statement.empty())
                        continue;
				
					isKeyWord(statement);
				}
			}
			insideLocationBlock = false;

			// Continue parsing after the }
			if (!afterBrace.empty())
				line = afterBrace; // re-parsed
			return ;
		}
        if (!insideLocationBlock){
			std::cout << "Invalid text outside location's block: " << line << std::endl;
			throw std::exception();
		}

		std::istringstream ss(line); //splits a line into words/tokens
        std::string key;
        ss >> key;
        while (std::getline(ss, key, ';')){
		    // Trim whitespace
			if (!line.empty() && line[line.length() - 1] != ';'){
				std::cerr << "Missing semicolon at the end of: " << line << std::endl;
				throw std::exception();
			}
		    key.erase(0, key.find_first_not_of(" \t"));
		    key.erase(key.find_last_not_of(" \t") + 1);
		
		    if (key.empty())
		        continue;
		
		    isKeyWordLocationPart(key);
		}
	}
	if (_howManyIndex_location > 1 || _howManyAllow_methods > 1 || _howManyUpload_store > 1 || _howManyCgi_pass > 1 || _howManyCgi_path > 1 || _howManyCgi_ext > 1 || _howManyRoot_location > 1 || _howManyAutoindex > 1){
		std::cerr << "There are duplicates keywords in the configuration file (within a location block)!" << std::endl;
		throw std::exception();
	}
}