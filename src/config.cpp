#include "../includes/config.hpp"

/* This class will search .config for the block "server{}" */
configValues::configValues(std::string &configFile){
	parseConfig(configFile);

    int     yes;


    for (std::vector<ServerBlock>::iterator it = this->_servers.begin(); it != this->_servers.end(); ++it)
{
        yes = 1;
        it->addr.sin_family = AF_INET;
        it->addr.sin_port = htons(std::atoi(it->listen.c_str()));
        it->addr.sin_addr.s_addr = INADDR_ANY;

        it->fd = socket(AF_INET, SOCK_STREAM, 0);
        if (setsockopt(it->fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) // Re-use socket if is already in use
            throw std::runtime_error("config.cpp:18\n");
        if (bind(it->fd, (const struct sockaddr *)&it->addr, sizeof(it->addr)) != 0)
            throw std::runtime_error("config.cpp:20\n");
        if (listen(it->fd, SOMAXCONN) != 0)
            throw std::runtime_error("config.cpp:22\n");
    }
}

configValues::~configValues(){}

void configValues::defaultPreConfigs(){
	_howManyListen = 0;
	_howManyHost = 0;
	_howManyServerName = 0;
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

	
	_numOfLocInSrvBlock = -1;
}

void configValues::defaultConfigs(ServerBlock &srv){
	if (_howManyListen == 0)
		srv.listen = "80";
	if (srv.listen == "8888" && _howManyHost == 0)
		srv.host = "0.0.0.0";

	/* Check if there aren't duplicates */
	if (_howManyListen > 1 || _howManyHost > 1 || _howManyServerName > 1 || /* _howManyErrorMessage > 1 || */ _howManyClient > 1 || _howManyRoot > 1 || _howManyIndex > 1){	
		std::cerr << "There are duplicates keywords in the configuration file" << std::endl;
		throw std::exception();
	}

	if (_howManyIndex_location == 0 && _numOfLocInSrvBlock > -1 && static_cast<size_t>(_numOfLocInSrvBlock) < srv.locations.size())
	    srv.locations[_numOfLocInSrvBlock].index = "index.html";

	if (_howManyAllow_methods == 0 && _numOfLocInSrvBlock > -1 && static_cast<size_t>(_numOfLocInSrvBlock) < srv.locations.size())
	    srv.locations[_numOfLocInSrvBlock].allow_methods = "GET";
}

void configValues::isKeyWord(std::string statement, ServerBlock &srv){
	std::istringstream iss(statement);
	std::string key;
	iss >> key;

	if (key == "listen"){
		iss >> srv.listen;
		_howManyListen++;
	}
	else if (key == "host"){
		iss >> srv.host;
		_howManyHost++;
	}
	else if (key == "server_name"){
        iss >> srv.serverName;
		while (iss >> key){
			if (!srv.serverName.empty()) srv.serverName += " ";
			srv.serverName += key;
		}
		_howManyServerName++;
	}
	/* [0] will be the number(s) (404) [1] will be the path and so foward */         // If I have more than 1 server it will cause a segmentation fault
	else if (key == "error_page"){
	    std::vector<std::string> numError;
	    std::string token;

	    // Gather numError
	    while (iss >> token && token[0] != '/')
	        numError.push_back(token);

	    // Now token is the path
	    if (!numError.empty() && token[0] == '/'){
		    for (size_t i = 0; i < numError.size(); ++i){
		        srv.errorPage.push_back(numError[i] + " " + token);
		    }
		}
		else{
		    std::cerr << "Invalid error_page directive!" << std::endl;
			throw std::exception();
		}
	}
	else if (key == "client_max_body_size"){
		iss >> srv.clientMaxBodySize;
		_howManyClient++;
	}
	else if (key == "root"){
		iss >> srv.root;
		_howManyRoot++;
	}
	else if (key == "index"){
		iss >> srv.index;
		while (iss >> key){
			if (!srv.index.empty()) srv.index += " ";
			srv.index += key;
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
	ServerBlock srvStruct;
	LocationBlock locStruct;

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

        /* Remove comments */
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);

        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty())
            continue;

        if (detectServerBlock(file, line, insideServerBlock)){
			defaultPreConfigs();
			srvStruct = ServerBlock();
            if (line.empty())
                continue;
        }

        // Handle location block
        if (line.find("location") == 0 && line.find("{") != std::string::npos){
            parseLocatePart(file, line, srvStruct, locStruct);
            continue;
        }
        else if (line.find("location") == 0){
            parseLocatePart(file, line, srvStruct, locStruct);
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

                    isKeyWord(statement, srvStruct);
                }
            }
            insideServerBlock = false;
			_servers.push_back(srvStruct);
			srvStruct = ServerBlock();  // This will reset the structure (otherwise it would keep the previous values)
            if (!afterBrace.empty())
                line = afterBrace; // re-parsed the line after }
            else
                continue;
        }

        if (!insideServerBlock){
            std::cout << "Invalid text outside server's block: " << line << std::endl;
            throw std::exception();
        }

        // Split by ';'
        std::stringstream ss(line);
        std::string statement;
        while (std::getline(ss, statement, ';')){
            statement.erase(0, statement.find_first_not_of(" \t"));
            statement.erase(statement.find_last_not_of(" \t") + 1);

            if (statement.empty())
                continue;
            isKeyWord(statement, srvStruct);
        }
    }

    // At the end, apply defaults/checks only once for the server block you have
    defaultConfigs(srvStruct);
    file.close();
}

/* Getters */
std::string configValues::get_listen(int i) const{
    if (i >= 0 && static_cast<size_t>(i) < _servers.size())
        return _servers[i].listen;
    return "";
}
std::string configValues::get_host(int i) const{
    if (i >= 0 && static_cast<size_t>(i) < _servers.size())
        return _servers[i].host;
    return "";
}
std::string configValues::get_serverName(int i) const{
    if (i >= 0 && static_cast<size_t>(i) < _servers.size())
        return _servers[i].serverName;
    return "";
}
/* const std::vector<std::string> &configValues::get_errorPage(int i) const{
    static const std::vector<std::string> empty;
    if (i >= 0 && static_cast<size_t>(i) < _servers.size())
        return _servers[i].errorPage;
    return empty;
} */
/* std::string configValues::get_errorPage(int i) const{
    if (i >= 0 && static_cast<size_t>(i) < _servers.size())
        return _servers[i].errorPage;
    return "";
} */
std::string configValues::get_clientMaxBodySize(int i) const{
    if (i >= 0 && static_cast<size_t>(i) < _servers.size())
        return _servers[i].clientMaxBodySize;
    return "";
}
std::string configValues::get_root(int i) const{
    if (i >= 0 && static_cast<size_t>(i) < _servers.size())
        return _servers[i].root;
    return "";
}
std::string configValues::get_index(int i) const{
    if (i >= 0 && static_cast<size_t>(i) < _servers.size())
        return _servers[i].index;
    return "";
}

std::string configValues::get_location_path(int srvIdx, int locIdx) const{
    if (srvIdx >= 0 && static_cast<size_t>(srvIdx) < _servers.size()){
        const ServerBlock &srv = _servers[srvIdx];
        if (locIdx >= 0 && static_cast<size_t>(locIdx) < srv.locations.size())
            return srv.locations[locIdx].path;
    }
    return "";
}
std::string configValues::get_location_index(int srvIdx, int locIdx) const{
    if (srvIdx >= 0 && static_cast<size_t>(srvIdx) < _servers.size()){
        const ServerBlock &srv = _servers[srvIdx];
        if (locIdx >= 0 && static_cast<size_t>(locIdx) < srv.locations.size())
            return srv.locations[locIdx].index;
    }
    return "";
}
std::string configValues::get_location_allow_methods(int srvIdx, int locIdx) const{
    if (srvIdx >= 0 && static_cast<size_t>(srvIdx) < _servers.size()){
        const ServerBlock &srv = _servers[srvIdx];
        if (locIdx >= 0 && static_cast<size_t>(locIdx) < srv.locations.size())
            return srv.locations[locIdx].allow_methods;
    }
    return "";
}
std::string configValues::get_location_upload_store(int srvIdx, int locIdx) const{
    if (srvIdx >= 0 && static_cast<size_t>(srvIdx) < _servers.size()){
        const ServerBlock &srv = _servers[srvIdx];
        if (locIdx >= 0 && static_cast<size_t>(locIdx) < srv.locations.size())
            return srv.locations[locIdx].upload_store;
    }
    return "";
}
std::string configValues::get_location_cgi_pass(int srvIdx, int locIdx) const{
    if (srvIdx >= 0 && static_cast<size_t>(srvIdx) < _servers.size()){
        const ServerBlock &srv = _servers[srvIdx];
        if (locIdx >= 0 && static_cast<size_t>(locIdx) < srv.locations.size())
            return srv.locations[locIdx].cgi_pass;
    }
    return "";
}
std::string configValues::get_location_cgi_path(int srvIdx, int locIdx) const{
    if (srvIdx >= 0 && static_cast<size_t>(srvIdx) < _servers.size()){
        const ServerBlock &srv = _servers[srvIdx];
        if (locIdx >= 0 && static_cast<size_t>(locIdx) < srv.locations.size())
            return srv.locations[locIdx].cgi_path;
    }
    return "";
}
std::string configValues::get_location_cgi_ext(int srvIdx, int locIdx) const{
    if (srvIdx >= 0 && static_cast<size_t>(srvIdx) < _servers.size()){
        const ServerBlock &srv = _servers[srvIdx];
        if (locIdx >= 0 && static_cast<size_t>(locIdx) < srv.locations.size())
            return srv.locations[locIdx].cgi_ext;
    }
    return "";
}
std::string configValues::get_location_root(int srvIdx, int locIdx) const{
    if (srvIdx >= 0 && static_cast<size_t>(srvIdx) < _servers.size()){
        const ServerBlock &srv = _servers[srvIdx];
        if (locIdx >= 0 && static_cast<size_t>(locIdx) < srv.locations.size())
            return srv.locations[locIdx].root;
    }
    return "";
}
bool configValues::get_location_autoindex(int srvIdx, int locIdx) const{
    if (srvIdx >= 0 && static_cast<size_t>(srvIdx) < _servers.size()){
        const ServerBlock &srv = _servers[srvIdx];
        if (locIdx >= 0 && static_cast<size_t>(locIdx) < srv.locations.size())
            return srv.locations[locIdx].autoindex;
    }
    return false;
}
