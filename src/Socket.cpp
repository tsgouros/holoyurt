#include "Socket.h"
#include <iostream>

#ifdef __GNUC__
#define Sleep(x) usleep(x*100)
#endif


std::string Socket::term = "\n0\n";

// get sockaddr, IPv4 or IPv6:
void *Socket::get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

Socket::Socket(const std::string& serverIP, const std::string& serverPort)
{
  _persistentBuffer = "";

	printf("client: connecting...\n");
#ifdef WIN32  // WinSock implementation

	WSADATA wsaData;
	SOCKET sockfd = INVALID_SOCKET;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	rv = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rv != 0) {
		std::cerr << "WSAStartup failed with error: " << rv << std::endl;
		exit(1);
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if ((rv = getaddrinfo(serverIP.c_str(), serverPort.c_str(), &hints, &servinfo)) != 0) {
		std::cerr << "getaddrinfo() failed with error: " << rv << std::endl;
		WSACleanup();
		exit(1);
	}

	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET) {
			std::cerr << "socket() failed with error: " << WSAGetLastError() << std::endl;
			continue;
		}

		if (connect(sockfd, p->ai_addr, (int)p->ai_addrlen) == SOCKET_ERROR) {
			closesocket(sockfd);
			sockfd = INVALID_SOCKET;
			std::cerr << "connect() to server socket failed" << std::endl;
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		//return 2;
		exit(2);
	}

	printf("client: connected\n");
	int iTimeout = 1000;
	int out = setsockopt(sockfd,
		SOL_SOCKET,
		SO_RCVTIMEO,
		/*
		reinterpret_cast<char*>(&tv),
		sizeof(timeval) );
		*/
		(const char *)&iTimeout,
		sizeof(iTimeout));

	freeaddrinfo(servinfo); // all done with this structure

	_socketFD = sockfd;


#else  // BSD sockets implementation


	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(serverIP.c_str(), serverPort.c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		//return 1;
		exit(1);
	}

	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		//return 2;
		exit(2);
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("client: connected to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	_socketFD = sockfd;

#endif
}

Socket::~Socket()
{
#ifdef WIN32 
	closesocket(_socketFD);
	WSACleanup();
#else
	close(_socketFD);
#endif
}

bool Socket::isConnected()
{
#ifdef WIN32
  return _socketFD != INVALID_SOCKET;
#else
  int error = 0;
  socklen_t len = sizeof(error);
  int sockopt = getsockopt(_socketFD, SOL_SOCKET, SO_ERROR, &error, &len);
  return error == 0;
#endif
}

void Socket::sendMessage(std::string  message)
{
  const char * message_char = message.c_str();
  int len = strlen(message_char);
  int total = 0;        // how many bytes we've sent
  int bytesleft = len;  // how many we have left to send
  int n = 0;
  while (total < len) {

    n = send(_socketFD, (char *)(message_char + total), bytesleft, 0);

    if (n == -1) { break; }
    total += n;
    bytesleft -= n;
  }
}

std::string Socket::receiveMessage(int nExpected)
{

  while (_persistentBuffer.length() < nExpected) {
    std::cout << "entering" << _persistentBuffer.length() << nExpected << std::endl;

    std::string reply = recvMessage();
    std::cout << reply << std::endl;
    _persistentBuffer += recvMessage();
  }

  std::string output = _persistentBuffer.substr(0, nExpected);
  _persistentBuffer.erase(0, nExpected);

  return output;
}

std::string Socket::receiveMessage() {

  std::string output = _persistentBuffer + recvMessage();
  _persistentBuffer.clear();

  return output;
}

std::string Socket::recvMessage()
{
  std::string output;
  char buffer[1024];

  int n;
  while ((errno = 0, (n = recv(_socketFD, buffer, sizeof(buffer), 0))>0) ||
	 errno == EINTR) {
    std::cout << "wow" << n<< "," << output.length() << std::endl;
    if (n > 0){
      output.append(buffer, n);

    } else {
    std::cout << "wow!!!" << n<< output.length() << std::endl;
      break;
    }
  }

  return output;
}

std::string Socket::sendMessageGetReply(std::string message, int nExpected) {
  if (message.length() > 0) {

    std::cout << "SENDING>>" << message << "<<" << std::endl;
    sendMessage(message);

  }

  Sleep(500);
  if (nExpected == 0) return(std::string(""));

  std::string out = receiveMessage(nExpected);
  return out;
}


cv::Mat Socket::receiveImage()
{
  //receive data
  std::string reply;
  while (reply.empty()){
    reply = receiveMessage();
  }

  //parse data
  std::vector<std::string> strings;
  std::string::size_type prev_pos = 0, pos = 0;

  pos = reply.find('\n', pos);
  strings.push_back(reply.substr(prev_pos, pos - prev_pos));
  prev_pos = ++pos;

  pos = reply.find('\n', pos);
  strings.push_back(reply.substr(prev_pos, pos - prev_pos));
  prev_pos = ++pos;

  strings.push_back(reply.substr(prev_pos, reply.size() - prev_pos));

  int width = atoi(strings[0].substr(22, 4).c_str());
  int height = atoi(strings[0].substr(27, 4).c_str());
  int datasize = atoi(strings[1].c_str());

  //convert endianess
  char* data = new char[datasize];
  char* dataBigEndian = const_cast<char*> (strings[2].c_str());

  int * ptr_org = (int*)dataBigEndian;
  int * ptr_dest = (int*)data;

  for (int i = 0; i < datasize / 4; i++, ptr_org++, ptr_dest++)
    {
      *ptr_dest = (*ptr_org << 24) |
	((*ptr_org << 8) & 0x00ff0000) |
	((*ptr_org >> 8) & 0x0000ff00) |
	((*ptr_org >> 24) & 0x000000ff);
    }

  cv::Mat image(cv::Size(width, height), CV_32FC1, (float*)data); 
  //we clone so that we can delete the data array
  cv::Mat out_image = image.clone();

  //delete data
  delete[] data;

  return out_image;
}
