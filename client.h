#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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

#define BUFF_SIZE			8192 //velikost bufferu

using namespace std;