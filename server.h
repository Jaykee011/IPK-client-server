#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>

#include <time.h>
#include <regex.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
	chybové konstanty
*/
#define FOLDEREXISTS		1
#define USERNOTFOUND		2
#define DIRNOTFOUND			3
#define DIRNOTEMPTY			4
#define NOTADIR				5
#define NOTAFILE			6
#define FILENOTFOUND		7
#define ERROR				127

#define DEF_PORT 			6677 //implicitní port
#define DEF_PATH			"./" //implicitní cesta
#define BUFF_SIZE			8192 //velikost bufferu
#define MAX_QUEUE			10 //maximální počet na vyřízení čekajících zpráv

using namespace std;
