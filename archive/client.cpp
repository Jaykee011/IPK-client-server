#include "client.h"
#include "messaging.h"

/*
	vrací true pokud zadaná cesta míří na soubor, false pokud na složku
*/
int is_regular_file(string path)
{
	struct stat path_stat;
    stat(path.c_str(), &path_stat);
    return S_ISREG(path_stat.st_mode);
}

/*
	Vytvoří socket klienta.
	Vrací -1 při chybě
*/
int socketInit()
{
	int clientSocket;

	if((clientSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Error: SOCKET_CREATE_ERR\n");
		return -1;
	}
	return clientSocket;
}

/*
	pošle zprávu msg socketem socket
*/
void sendMessage(int socket, string msg)
{
	if ((send(socket, msg.c_str(), msg.length(), 0)) == -1)
	{
		fprintf(stderr, "Error: SOCKET_SEND_ERROR\n");
		exit(-1);
	}
}

/*
	pomocí regulárního výrazu zanalyzuje zadané URI a pomocí parametrů hostname serveru, port serveru a cestu
*/
void parseURI(string URI, string* server_name, int* port, string* path)
{

	regex_t compiledRegex;
	const char* regex ="http://([a-zA-Z0-9_-]*):*([[:digit:]]*)/(.*)";
	regcomp (&compiledRegex, regex, REG_EXTENDED);
	regmatch_t matchptr[5];

	if(regexec (&compiledRegex, URI.c_str(), 4, matchptr, REG_NOTEOL|0))
	{
		fprintf(stderr, "Unknown error.\n");
		exit(-1);
	}	

	*server_name = URI.substr(matchptr[1].rm_so,(matchptr[1].rm_eo - matchptr[1].rm_so));
	*port = atoi((URI.substr(matchptr[2].rm_so,(matchptr[2].rm_eo - matchptr[2].rm_so)).c_str()));
	*path = URI.substr(matchptr[3].rm_so,(matchptr[3].rm_eo - matchptr[3].rm_so));

	//cout << *server_name << "\n";
	//cout << *port << "\n";

	regfree(&compiledRegex);
}


/*
	načte soubor do vektoru jako byte string
*/
static std::vector<char> readFile(char const* filename)
{
    ifstream ifs(filename, ios::binary|ios::ate);
    ifstream::pos_type pos = ifs.tellg();

    std::vector<char>  result(pos);

    ifs.seekg(0, ios::beg);
    ifs.read(&result[0], pos);

    return result;
}

/*
	pomocí regulárních výrazů zanalyzuje response od serveru. V řetězci result vrací patičku zprávy.
*/
int parseMsg(string message, struct parsedMsg* request, string* result)
{
	regex_t compiledRegex;
	regmatch_t matchptr[3];

	//matching HTTP
	const char* regex ="^HTTP/1.1 ([0-9][0-9][0-9] [A-Za-z ]*)";
	regcomp (&compiledRegex, regex, REG_EXTENDED);
	if(regexec (&compiledRegex, message.c_str(), 2, matchptr, REG_NOTEOL|0)){fprintf(stderr, "Unknown error.\n");return -1;}	
	request->HTTP = message.substr(matchptr[1].rm_so,(matchptr[1].rm_eo - matchptr[1].rm_so));

	//cout << "\n" << request->HTTP << "\n";

	//matching content_length
	regex ="Content_Length: ([0-9]*)";
	regcomp (&compiledRegex, regex, REG_EXTENDED);
	if(regexec (&compiledRegex, message.c_str(), 2, matchptr, REG_NOTEOL|0)){fprintf(stderr, "Unknown error.\n");return -1;}	
	request->content_length = atoi((message.substr(matchptr[1].rm_so,(matchptr[1].rm_eo - matchptr[1].rm_so))).c_str());

	//cout << "\n" << request->content_length << "\n";

	//matching content_type
	regex ="Content_Type: (.*)\nContent_Length";
	regcomp (&compiledRegex, regex, REG_EXTENDED);
	if(regexec (&compiledRegex, message.c_str(), 2, matchptr, REG_NOTEOL|0)){fprintf(stderr, "Unknown error.\n");return -1;}	
	request->content_type = message.substr(matchptr[1].rm_so,(matchptr[1].rm_eo - matchptr[1].rm_so));

	//cout << "\n" << request->content_type << "\n";

	//matching trailer
	regex ="Encoding: identity\n(.*)";
	regcomp (&compiledRegex, regex, REG_EXTENDED);
	if(regexec (&compiledRegex, message.c_str(), 2, matchptr, REG_NOTEOL|0)){fprintf(stderr, "Unknown error.\n");return -1;}	
	request->trailer = message.substr(matchptr[1].rm_so, request->content_length);

	//cout << "\n" << request->trailer << "\n";

	if (request->HTTP == "404 Not found" || request->HTTP == "400 Bad Request")
	{
		fprintf(stderr, "%s\n", request->trailer.c_str());
		exit(-1);
	}
	else if (request->HTTP == "200 OK")
	{
		*result = request->trailer;
	}

	regfree(&compiledRegex);
}


int main(int argc, char* argv[])
{
	int port = 6677;
	int clientSocket;
	string command;
	string URI;
	string accept = "text/plain";
	string content_type = "text/plain";
	string remote_path;
	string local_path;
	string server_name;
	int content_length = 0;
	string trailer = "";


	if (argc < 3)
	{
		fprintf(stderr, "Error: PARAMETER_ERROR\n");
		exit(-1);
	}

	command = argv[1];
	URI = argv[2];
	if (argc == 4 && (command == "put" || command == "get"))
		local_path	= argv[3];
	else if (command == "put" && argc != 4)
	{
		fprintf(stderr, "Error: PARAMETER_ERROR\n");
		exit(-1);
	}

	/*
		timestamp
	*/
	time_t rawtime;
    tm* timeinfo;
    char timeBuffer [80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timeBuffer,100,"%a, %d %b %Y %H:%M:%S CET",timeinfo);
    /*
    	/timestamp
    */

    parseURI(URI, &server_name, &port, &remote_path);
    if (port == 0)
    	port = 6677;

    /*
		nastavení správné hodnoty accept a content_type
    */
    if (command == "get")
    	accept = "application/octet-stream";
    else if (command == "put")
    {
		if (!is_regular_file(local_path))
		{
			fprintf(stderr, "Not a file.\n");
			exit(-1);
		}
    	content_type = "application/octet-stream";
    	static std::vector<char> bytestring;

    	/*
			načte odesílaný soubor jako byte string a připojí jej k requestu jako patičku
    	*/
    	ifstream myfile;
    	myfile.open (local_path, ios::binary);
    	char character;
    	while ( myfile.get(character) )
	    {
	       bytestring.push_back(character);
	    }
    	//bytestring = readFile(local_path.data());
    	for(std::vector<char>::iterator it = bytestring.begin(); it != bytestring.end(); ++it) 
    	{
		    trailer.push_back(*it);
		    content_length = trailer.size();
		}
		myfile.close();

		//cout << trailer << "\n";
    }
    	
    //compileTrailer(&trailer, &content_length);

    /*
		vytvoření objektu requestu a jeho inicializace
    */
	Message messageObj = Message(timeBuffer, accept, "identity", content_type, content_length, remote_path, trailer, command);
	string message = messageObj.compileMessage();

	string serverResponse;
	struct sockaddr_in sa;

	if ((clientSocket = socketInit()) == -1)
	{
		return -1;
	}


	/*
		Nastavení správné adresy serveru a připojení
	*/
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons(port);

	struct hostent *hptr;

	if((hptr =  gethostbyname(server_name.c_str())) == NULL)
	{
		fprintf(stderr, "Error: HOSTNAME_ERROR: %s\n", server_name.c_str());
		return -1;
	}

	memcpy(&sa.sin_addr, hptr->h_addr, hptr->h_length);

	if(connect(clientSocket, (struct sockaddr *)&sa, sizeof(sa)) < 0 )
	{
		fprintf(stderr, "Error: SOCKET_CONNECT_ERROR\n");
		exit(-1);
	}

	sendMessage(clientSocket, message);

	/*
		přijetí odpovědi serveru
	*/
	char buffer[BUFF_SIZE];
    if( recv(clientSocket , buffer , BUFF_SIZE , 0) < 0)
    {
       	fprintf(stderr, "Error: SERVER_RESPONSE_ERROR\n");
		exit(-1);
    }

   	struct parsedMsg request;
   	string result;

   	/*
		zpracování odpovědi serveru a vytvoření souboru v případě příkazu get
   	*/
	parseMsg(buffer, &request, &result);

	if (request.content_type == "application/octet-stream")
	{
		ofstream outputFile;
		outputFile.open (local_path, ios::binary);
		outputFile << request.trailer;
		outputFile.close();
	}

    //cout << buffer << "\n";

	close(clientSocket);

	return 0;
}