#include "../includes/config.hpp"
#include <iostream>

void configValues::isKeyWordLocationPart(std::string statement, LocationBlock &loc) {
	std::istringstream iss(statement);
	std::string key;
	iss >> key;

	if (key == "index") {
		iss >> key;
		while (iss >> key) {
			if (!loc.index.empty()) loc.index += " ";
			loc.index += key;
		}
		_howManyIndex_location++;
	}
	else if (key == "allow_methods") {
		while (iss >> key) {
			if (!loc.allow_methods.empty()) loc.allow_methods += " ";
			loc.allow_methods += key;
		}
		_howManyAllow_methods++;
	}
	else if (key == "upload_store") {
		iss >> loc.upload_store;
		_howManyUpload_store++;
	}
	else if (key == "cgi_pass") {
		while (iss >> key) {
			if (!loc.cgi_pass.empty()) loc.cgi_pass += " ";
			loc.cgi_pass += key;
		}
		_howManyCgi_pass++;
	}
	else if (key == "cgi_path") {
		while (iss >> key) {
			if (!loc.cgi_path.empty()) loc.cgi_path += " ";
			loc.cgi_path += key;
		}
		_howManyCgi_path++;
	}
	else if (key == "cgi_ext") {
		while (iss >> key) {
			if (!loc.cgi_ext.empty()) loc.cgi_ext += " ";
			loc.cgi_ext += key;
		}
		_howManyCgi_ext++;
	}
	else if (key == "root") {
		iss >> loc.root;
		_howManyRoot_location++;
	}
	else if (key == "autoindex") {
		std::string value;
		iss >> value;
		if (value == "on;" || value == "on") {
			loc.autoindex = true;
		}
		else if (value == "off;" || value == "off") {
			loc.autoindex = false;
		}
		else {
			throw std::runtime_error("Invalid value for location_autoindex: expected 'on' or 'off'");
		}
		_howManyAutoindex++;
	}
	else {
		std::cerr << "Invalid keyword in location block: " << statement << std::endl;
		throw std::exception();
	}
}

bool configValues::detectLocationBlock(std::istream& file, std::string& line, bool& insideLocationBlock, LocationBlock &loc){

	if (line.find("location") == 0 && line.find("{") != std::string::npos){
		std::string beforeBrace = line.substr(0, line.find("{"));
		std::string afterBrace = line.substr(line.find("{") + 1);

		size_t end = beforeBrace.find_last_not_of(" \t");
		if (end != std::string::npos)
		    beforeBrace.erase(end + 1);
		else
		    beforeBrace.clear(); // the string is all whitespace

		//beforeBrace.erase(beforeBrace.find_last_not_of(" \t") + 1); //problem is here
		afterBrace.erase(0, afterBrace.find_first_not_of(" \t"));
		afterBrace.erase(afterBrace.find_last_not_of(" \t") + 1);
		
		std::istringstream iss(beforeBrace);
		std::string keyword, second, thrid;
		iss >> keyword >> second >> thrid;

		if (keyword == "location" && !second.empty()){
			loc.path = second;
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

		if (keyword.empty()){
			std::cout << "Missing text after location's keyword/s: " << line << std::endl;
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

					std::cout << nextLine << ";" << std::endl;
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

void configValues::parseLocatePart(std::istream &file, std::string &line, std::string &unmodifiedLine, ServerBlock &srv) {
    bool insideLocationBlock = false;
	LocationBlock loc;

	(void)unmodifiedLine;
    do {
        // Remove comments
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);

        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty())
            continue;

        // Detect the start of the location block
        if (detectLocationBlock(file, line, insideLocationBlock, loc)) {
			_numOfLocInSrvBlock++;
            if (line.empty())
                continue;
        }

        if (!insideLocationBlock) {
            std::cerr << "Invalid text outside Location's block: " << line << std::endl;
            throw std::exception();
        }

        // Handle lines with a closing brace
        if (line.find('}') != std::string::npos) {
            std::string beforeBrace = line.substr(0, line.find('}'));
            // Trim
            beforeBrace.erase(0, beforeBrace.find_first_not_of(" \t"));
            beforeBrace.erase(beforeBrace.find_last_not_of(" \t") + 1);

            if (!beforeBrace.empty()) {
                std::stringstream ss(beforeBrace);
                std::string statement;
                while (std::getline(ss, statement, ';')) {
                    statement.erase(0, statement.find_first_not_of(" \t"));
                    statement.erase(statement.find_last_not_of(" \t") + 1);
                    if (statement.empty())
                        continue;
                    isKeyWordLocationPart(statement, loc);
                }
            }
            insideLocationBlock = false;
			srv.locations.push_back(loc);
            // Done with this block
            return;
        }

        // Split and parse the line by semicolon
        std::stringstream ss(line);
        std::string statement;
        while (std::getline(ss, statement, ';')) {
            statement.erase(0, statement.find_first_not_of(" \t"));
            statement.erase(statement.find_last_not_of(" \t") + 1);
            if (statement.empty())
                continue;
            isKeyWordLocationPart(statement, loc);
        }

    } while (std::getline(file, line));

    if (insideLocationBlock) {
        std::cerr << "Location close brackets (}) missing" << std::endl;
        throw std::exception();
    }
}
