#include "server.h"
#include "messaging.h"

int socketInit()
{
	int serverSocket;

	if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Error: SOCKET_CREATE_ERR\n");
		exit(-1);
	}
	return serverSocket;
}

inline bool fileExists (const std::string& name) {
    ifstream f(name.c_str());
    return f.good();
}

void socketBind(int socket, struct sockaddr_in sa)
{
	if ((bind(socket, (struct sockaddr *)&sa, sizeof(sa))) < 0)
	{
		fprintf(stderr, "Error: SOCKET_BIND_ERR\n");
		exit(-1);
	}

}

int is_regular_file(string path)
{
	struct stat path_stat;
    stat(path.c_str(), &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void sendResponse(string result, int socket, struct parsedMsg* request)
{
	if (result == "Not a file." || result == "Not a directory." || result == "Unknown error.")
		request->HTTP = "400 Bad Request";
	else if (result == "File not found." || result == "User Account Not Found." || result == "Directory not found.")
		request->HTTP = "404 Not found";
	else
		request->HTTP = "200 OK";

	string content_type;

	if (request->command == "get")
		content_type = "application/octet-stream";
	else
		content_type = "text/plain";

	int content_length;

	content_length = result.size();

	string content_encoding = "identity";

	time_t rawtime;
    tm* timeinfo;
    char timeBuffer [80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timeBuffer,100,"%a, %d %b %Y %H:%M:%S CET",timeinfo);

	Message msg = Message(timeBuffer, content_type, content_length, content_encoding, result, request->HTTP);

	string message;

	message = msg.compileMessage();

	if ((send(socket, message.c_str(), message.length(), 0)) == -1)
	{
		fprintf(stderr, "Error: RESPONSE_ERROR\n");
		exit(-1);
	}
}

static std::vector<char> readFile(char const* filename)
{
    ifstream ifs(filename, ios::binary|ios::ate);
    ifstream::pos_type pos = ifs.tellg();

    std::vector<char>  result(pos);

    ifs.seekg(0, ios::beg);
    ifs.read(&result[0], pos);

    return result;
}

int parseMsg(string message, struct parsedMsg* request)
{
	regex_t compiledRegex;
	regmatch_t matchptr[3];

	//matching REST
	const char* regex ="^([A-Z]*)";
	regcomp (&compiledRegex, regex, REG_EXTENDED);
	if(regexec (&compiledRegex, message.c_str(), 2, matchptr, REG_NOTEOL|0)){fprintf(stderr, "Unknown error.\n");return ERROR;}	
	request->REST = message.substr(matchptr[1].rm_so,(matchptr[1].rm_eo - matchptr[1].rm_so));

	//cout << "\n" << request->REST << "\n";

	//matching fileType
	regex =".*?type=([a-z]*).*";
	regcomp (&compiledRegex, regex, REG_EXTENDED);
	if(regexec (&compiledRegex, message.c_str(), 2, matchptr, REG_NOTEOL|0)){fprintf(stderr, "Unknown error.\n");return ERROR;}	
	request->fileType = message.substr(matchptr[1].rm_so,(matchptr[1].rm_eo - matchptr[1].rm_so));

	//cout << "\n" << request->fileType << "\n";

	//matching user
	regex ="^[A-Z]* ([A-Za-z0-9_-]*).*";
	regcomp (&compiledRegex, regex, REG_EXTENDED);
	if(regexec (&compiledRegex, message.c_str(), 2, matchptr, REG_NOTEOL|0)){fprintf(stderr, "Unknown error.\n");return ERROR;}	
	request->user = message.substr(matchptr[1].rm_so,(matchptr[1].rm_eo - matchptr[1].rm_so));

	//cout << "\n" << request->user << "\n";

	//matching path
	regex ="^[A-Z]* [A-Za-z0-9_-]*/(.*)\?type.*";
	regcomp (&compiledRegex, regex, REG_EXTENDED);
	if(regexec (&compiledRegex, message.c_str(), 2, matchptr, REG_NOTEOL|0)){request->path = "?";}	
	else request->path = message.substr(matchptr[1].rm_so,(matchptr[1].rm_eo - matchptr[1].rm_so));
	request->path.pop_back();

	//cout << "\n" << request->path << "\n";

	//matching content_length
	regex ="Content_Length: ([0-9]*)";
	regcomp (&compiledRegex, regex, REG_EXTENDED);
	if(regexec (&compiledRegex, message.c_str(), 2, matchptr, REG_NOTEOL|0)){fprintf(stderr, "Unknown error.\n");return ERROR;}	
	request->content_length = atoi((message.substr(matchptr[1].rm_so,(matchptr[1].rm_eo - matchptr[1].rm_so))).c_str());

	//cout << "\n" << request->content_length << "\n";

	//matching trailer
	regex ="Content_Length: [0-9]*\n(.*)";
	regcomp (&compiledRegex, regex, REG_EXTENDED);
	if(regexec (&compiledRegex, message.c_str(), 2, matchptr, REG_NOTEOL|0)){fprintf(stderr, "Unknown error.\n");return ERROR;}	
	request->trailer = message.substr(matchptr[1].rm_so, request->content_length);

	//cout << "\n" << request->trailer << "\n";

	if (request->REST == "PUT")
	{
		if (request->fileType == "file")
			request->command = "put";
		else if (request->fileType == "folder")
			request->command = "mkd";
	}
	else if (request->REST == "GET")
	{
		if (request->fileType == "file")
			request->command = "get";
		else if (request->fileType == "folder")
			request->command = "lst";
	}
	else if (request->REST == "DELETE")
	{
		if (request->fileType == "file")
			request->command = "del";
		else if (request->fileType == "folder")
			request->command = "rmd";
	}

	regfree(&compiledRegex);
}


int execute(struct parsedMsg request, string root, string *result)
{
	//cout << request.user << "\n\n";
	DIR* dir = opendir((root + request.user + "/").c_str()); //kontrola existence uživatele
	if (errno == ENOENT)
	{
		*result = "User Account Not Found.";
		return USERNOTFOUND;
	}
	closedir(dir);
		

	if (request.command == "mkd")
	{
		string fullPath = root + request.user + "/" + request.path;

		if (mkdir(fullPath.c_str(), S_IRWXO))
		{
			if (errno == EEXIST)
			{
				return FOLDEREXISTS;
			}
			else{*result = "Unknown error.";return ERROR;}	
		}
		*result = "OK";
	}

	else if (request.command == "put")
	{
		string fullPath = root + request.user + "/" + request.path;

		ofstream outputFile;
		outputFile.open (fullPath, ios::binary);
		outputFile << request.trailer;
		outputFile.close();
	}

	else if (request.command == "get")
	{
		string fullPath = root + request.user + "/" + request.path;
		if(!fileExists (fullPath))
		{
			*result = "File not found.";
			return FILENOTFOUND;
		}

		if (!is_regular_file(fullPath))
		{
			*result = "Not a file.";
			return NOTAFILE;
		}

		static std::vector<char> bytestring;
    	bytestring = readFile(fullPath.data());
    	for(std::vector<char>::iterator it = bytestring.begin(); it != bytestring.end(); ++it) 
    	{
		    (*result).push_back(*it);
		    //request->content_length = (*result).size();
		}
	}

	else if (request.command == "lst")
	{
		string fullPath = root + request.user + "/" + request.path;

		if(!fileExists (fullPath))
		{
			*result = "Directory not found.";
			return DIRNOTFOUND;
		}

		if (is_regular_file(fullPath))
		{
			*result = "Not a directory.";
			return NOTADIR;
		}

		DIR *dp;
	 	struct dirent *ep;

	 	dp = opendir (fullPath.c_str());
	 	if (dp != NULL)
	   	{
	      	while (ep = readdir (dp))
	      	{
	      		(*result).append(ep->d_name);
	      		(*result).append("\t");
	      	}
	      	(void) closedir (dp);
	    }
	  	else{*result = "Directory not found.";return ERROR;}	
	}

	else if (request.command == "del")
	{
		string fullPath = root + request.user + "/" + request.path;

		if(!fileExists (fullPath))
		{
			*result = "File not found.";
			return FILENOTFOUND;
		}

		if (!is_regular_file(fullPath))
		{
			*result = "Not a file.";
			return NOTAFILE;
		}

		//cout << fullPath << "\n";

		if (unlink(fullPath.c_str()))
		{
			switch (errno)
			{
				case ENOENT:
					*result = "File not found.";
					return FILENOTFOUND;

				default:
					*result = "Unknown error.";
					return ERROR;
			}
		}
		*result = "OK";
	}

	else if (request.command == "rmd")
	{
		string fullPath = root + request.user + "/" + request.path;

		if (request.path == "")
		{
			*result = "Not a directory.";
			return NOTADIR;
		}

		if(!fileExists (fullPath))
		{
			*result = "Directory not found.";
			return DIRNOTFOUND;
		}

		if (is_regular_file(fullPath))
		{
			*result = "Not a directory.";
			return NOTADIR;
		}

		//cout << fullPath << "\n";

		if (rmdir(fullPath.c_str()))
		{
			switch (errno)
			{
				case ENOENT:
					*result = "Directory not found.";
					return DIRNOTFOUND;

				case ENOTEMPTY:
					*result = "Directory not empty.";
					return DIRNOTEMPTY;
				default:
					*result = "Unknown error.";
					return ERROR;
			}
		}
		*result = "OK";
	}

	return 0;
}

int main(int argc, char* argv[])
{
	string response = "hear ya";
	int port = DEF_PORT;
	string root = DEF_PATH;
	int arg;

	if (argc > 1)
		while ((arg = getopt(argc, argv, "p:r:")) != -1)
		{
			switch (arg)
			{
				case 'p':
					port = atoi(optarg);
					break;
				case 'r':
					root = optarg;
					break;
				default:
					fprintf(stderr, "Error: PARAMETER_ERROR\n");
					exit(-1);
			}
		}

	int serverSocket;
	struct sockaddr_in sa, saConn;

	if ((serverSocket = socketInit()) == -1)
	{
		return -1;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons(port);

	socketBind(serverSocket, sa);

	if (listen(serverSocket, MAX_QUEUE) == -1)
	{
		fprintf(stderr,"Error: SOCKET_LISTEN_ERROR");
		return -1;
	}

	while(1) //message accept loop 
	{

		int commSocket;

		int c = sizeof(struct sockaddr_in);

		if ((commSocket = accept(serverSocket, (struct sockaddr*)&saConn, (socklen_t*)&c)) < 0)
			continue;

		int messLength;
		char buffer[BUFF_SIZE];
		string message;

		while(1)
		{
			messLength =  recv(commSocket, buffer, BUFF_SIZE, 0);
			if (messLength == -1)
			{
				fprintf(stderr,"Error: SOCKET_READ_ERROR\n");
				return -1;
			}
			if (messLength == 0)
				break;

			struct parsedMsg request;

			parseMsg(buffer, &request);

			string result;
			execute(request, root, &result);
			message.append(buffer);
			sendResponse(result, commSocket, &request);
		}

		//cout << message << "\n";
		message.erase();
	}
	
	return 0;
}

