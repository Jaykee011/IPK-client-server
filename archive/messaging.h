#include <string>

/*
	REST - RESTful operace
	type - souborový typ
	msgType - požadavek/odpověď
*/
namespace Communication
{
	enum REST 		{PUT, GET, DELETE};
	enum type 		{file, folder};
	enum msgType 	{request, response};
}

/*
	Třída popisující strukturu HTTP zprávy
	Poskytuje metodu compileMessage(), která vytvoří z objektu string obsahující HTTP zprávu v korektním formátu
*/
class Message 
{
	public:
		string version 				= "HTTP/1.1";

		string date;

		string accept;
		string accept_encoding;
		
		string content_type;
		string content_encoding;
		int content_length;

		string path;
		string trailer;

		string code;
		Communication::REST REST;
		Communication::type type;
		Communication::msgType msgType;

		Message();//defaultní konstruktor
		Message(string date, string accept, string accept_encoding, string content_type, int content_length, string path, string trailer, string command);// konstruktor požadavku
		Message(string date, string content_type, int content_length, string content_encoding, string trailer, string HTTP);// konstruktor odpovědi
		string compileMessage();
};

Message::Message()
{

}

/*
	Konstruktor požadavku
	Nastaví hodnoty objektu potřebné pro validní HTTP request
*/
Message::Message(string date, string accept, string accept_encoding, string content_type, int content_length, string path, string trailer, string command)
{
	if (command == "del")
	{
		Message::REST = Communication::DELETE;
		Message::type = Communication::file;
	}
	else if (command == "rmd")
	{
		Message::REST = Communication::DELETE;
		Message::type = Communication::folder;
	}
	else if (command == "lst")
	{
		Message::REST = Communication::GET;
		Message::type = Communication::folder;
	}
	else if (command == "put")
	{
		Message::REST = Communication::PUT;
		Message::type = Communication::file;
	}
	else if (command == "get")
	{
		Message::REST = Communication::GET;
		Message::type = Communication::file;
	}
	else if (command == "mkd")
	{
		Message::REST = Communication::PUT;
		Message::type = Communication::folder;
	}
		

	Message::msgType = Communication::request;
	Message::date = date;
	Message::accept= accept;
	Message::accept_encoding = accept_encoding;
	Message::content_type = content_type;
	Message::content_length = content_length;
	Message::path = path;
	Message::trailer = trailer;
}

/*
	Konstruktor požadavku
	Nastaví hodnoty objektu potřebné pro validní HTTP response
*/
Message::Message(string date, string content_type, int content_length, string content_encoding, string trailer, string HTTP)
{
	Message::msgType = Communication::response;
	Message::date = date;
	Message::content_type = content_type;
	Message::content_length = content_length;
	Message::content_encoding = content_encoding;
	Message::trailer = trailer;
	Message::code = HTTP;
}


/*
	Vytvoří string s HTTP zprávou a vrátí jej.	
*/
string Message::compileMessage()
{
	string message = "";

	if (msgType == Communication::request)
	{
		switch(REST)
		{
			case Communication::PUT:
				message.append("PUT");
				break;
			case Communication::GET:
				message.append("GET");
				break;
			case Communication::DELETE:
				message.append("DELETE");
				break;
		}
		message.append(" ");
		message.append(path);
		message.append("?type=");
		switch(type)
		{
			case Communication::file:
				message.append("file");
				break;
			case Communication::folder:
				message.append("folder");
				break;
		}
		message.append(" ");
		message.append(version);
		message.append("\n");

		message.append("Date: "); 
		message.append(date);
		message.append("\n");

		message.append("Accept: ");
		message.append(accept);
		message.append("\n");

		message.append("Accept_Encoding: ");
		message.append(accept_encoding);
		message.append("\n");

		message.append("Content_Type: ");
		message.append(content_type);
		message.append("\n");

		message.append("Content_Length: ");
		message.append(std::to_string(content_length));
		message.append("\n");

		message.append(trailer);
	}
	else
	{
		message.append(version);
		message.append(" ");
		message.append(code);
		message.append("\n");

		message.append("Date: "); 
		message.append(date); 
		message.append("\n");

		message.append("Content_Type: ");
		message.append(content_type);
		message.append("\n");

		message.append("Content_Length: ");
		message.append(std::to_string(content_length));
		message.append("\n");

		message.append("Content_Encoding: ");
		message.append(content_encoding);
		message.append("\n");

		message.append(trailer);
	}

	return message;
}

/*
	pomocná struktura zprávy
*/
struct parsedMsg{
	string fileType;
	string user;
	string path;
	int content_length;
	string trailer;
	string REST;
	string HTTP;
	string command;
	string content_type;
};