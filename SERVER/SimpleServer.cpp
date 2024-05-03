#include <SDL.h>
#include <SDL_net.h>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "raylib.h"
using namespace std;

int main(int argc, char* argv[]) {
	
	if (SDLNet_Init() == -1) {
		cerr << "SDLNet_Init error: " << SDLNet_GetError() << endl;
		return 1;
	}
	IPaddress ip;
	if (SDLNet_ResolveHost(&ip, nullptr, 4242) == -1) {
		cerr << "Resolve Host error: " << SDLNet_GetError() << endl;
		SDLNet_Quit();
		return 1;
	}
	TCPsocket serverSocket = SDLNet_TCP_Open(&ip);
	if (!serverSocket) {
		cerr << "TCP Open error: " << SDLNet_GetError() << endl;
		SDLNet_Quit();
		return 1;
	}else cout<<"Server open"<<endl;

	SDLNet_SocketSet set = SDLNet_AllocSocketSet(8);	//Put the maximum number of sockets
	vector<_TCPsocket*> clients;
	map<_TCPsocket*,string> clientPseudos;
	while (true){
		TCPsocket clientSocket;
		clientSocket = SDLNet_TCP_Accept(serverSocket);
		if (clientSocket){
			cout << "A client reached the server!" << endl;
			clients.push_back(clientSocket);
			SDLNet_AddSocket(set, reinterpret_cast<SDLNet_GenericSocket>(clientSocket));

			char buffer[32];
			int bytesRead = SDLNet_TCP_Recv(clientSocket, buffer, sizeof(buffer));
			if (bytesRead > 0){
				clientPseudos.emplace(clientSocket,string(buffer));
				string msg = "Send \"EXIT-CHAT\" to disconnect";
				int bytesSent = SDLNet_TCP_Send(clientSocket, msg.c_str(), msg.length() + 1);
				if (bytesSent < msg.length() + 1) {
					cerr << "SDLNet TCP Send error: " << SDLNet_GetError() << endl;
					break;
				}
				
				string answer = string(buffer)+ " connected";
				for(_TCPsocket *socket: clients){
					int bytesSent = SDLNet_TCP_Send(socket, answer.c_str(), answer.length() + 1);
					if (bytesSent < answer.length() + 1) {
						cerr << "SDLNet TCP Send error: " << SDLNet_GetError() << endl;
						break;
					}
				}
			}
		}
		_TCPsocket* toDisconnect=nullptr;
		if(SDLNet_CheckSockets(set, 0)!=0){
			for(_TCPsocket *socket: clients){
				char buffer[1024];
				int bytesRead = SDLNet_TCP_Recv(socket, buffer, sizeof(buffer));
				if (bytesRead > 0) {
					cout << "Incoming message: " << buffer << endl;
					string answer;
					if(strcmp(buffer,"EXIT-CHAT")==0){
						answer = clientPseudos.at(socket) + " has disconnected";
						toDisconnect = socket;
					}
					else answer = clientPseudos.at(socket) +": "+ string(buffer);
					for(_TCPsocket *s:clients){
						int bytesSent = SDLNet_TCP_Send(s, answer.c_str(), answer.length() + 1);
						cout<<bytesSent;
						if (bytesSent < answer.length() + 1) {
							cerr << "SDLNet TCP Send error: " << SDLNet_GetError() << endl;
							break;
						}
					}
				}
			}
		}
		if(toDisconnect!=nullptr){
			clients.erase(std::remove(clients.begin(), clients.end(), toDisconnect), clients.end());
			SDLNet_DelSocket(set,reinterpret_cast<SDLNet_GenericSocket>(toDisconnect));
		}
	}
	cout << "Thank you for using ChArtFX !\n";
	return 0;
}
