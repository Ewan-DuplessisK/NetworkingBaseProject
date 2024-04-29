#include <SDL.h>
#include <SDL_net.h>
#include <iostream>
#include <map>
#include <vector>
#include <string>
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

	vector<TCPsocket*> clients;
	map<Uint32,string> clientPseudos;
	while (true){
		TCPsocket clientSocket;
		clientSocket = SDLNet_TCP_Accept(serverSocket);
		if (clientSocket){
			cout << "A client reached the server!" << endl;
			clients.push_back(&clientSocket);

			char buffer[32];
			int bytesRead = SDLNet_TCP_Recv(clientSocket, buffer, sizeof(buffer));
			if (bytesRead > 0){
				clientPseudos.emplace(SDLNet_TCP_GetPeerAddress(clientSocket)->host,string(buffer));
				string msg = "Send \"EXIT-CHAT\" to disconnect";
				int bytesSent = SDLNet_TCP_Send(clientSocket, msg.c_str(), msg.length() + 1);
				if (bytesSent < msg.length() + 1) {
					cerr << "SDLNet TCP Send error: " << SDLNet_GetError() << endl;
					break;
				}
				
				string answer = string(buffer)+ " connected";
				for(TCPsocket *socket: clients){
					int bytesSent = SDLNet_TCP_Send(*socket, answer.c_str(), answer.length() + 1);
					if (bytesSent < answer.length() + 1) {
						cerr << "SDLNet TCP Send error: " << SDLNet_GetError() << endl;
						break;
					}
				}
			}
		}
		TCPsocket* toDisconnect=nullptr;
		for(TCPsocket *socket: clients){
			char buffer[1024];
			int bytesRead = SDLNet_TCP_Recv(*socket, buffer, sizeof(buffer));
			if (bytesRead > 0) {
			    cout << "Incoming message: " << buffer << endl;
				string answer;
				if(strcmp(buffer,"EXIT-CHAT")==0){
					answer = clientPseudos.at(SDLNet_TCP_GetPeerAddress(*socket)->host) + " has disconnected";
					toDisconnect = socket;
				}
			    else answer = clientPseudos.at(SDLNet_TCP_GetPeerAddress(*socket)->host) + string(buffer);
			    int bytesSent = SDLNet_TCP_Send(*socket, answer.c_str(), answer.length() + 1);
			    if (bytesSent < answer.length() + 1) {
				cerr << "SDLNet TCP Send error: " << SDLNet_GetError() << endl;
				break;
			    }
			}
		}
		if(toDisconnect!=nullptr){
			clients.erase(std::remove(clients.begin(), clients.end(), toDisconnect), clients.end());
		}
	}
	cout << "Thank you for using ChArtFX !\n";
	return 0;
}
