#include <SDL.h>
#include <SDL_net.h>
#include <iostream>
#include <vector>

#include "raylib.h"
//#include <string>
using namespace std;

struct Message
{
	bool fromMe = false;
	string content;
};
vector<Message> mLog{Message{false, "Waiting for someone to talk to..."}};
const int width = 500, height = 750;

int main(int argc, char* argv[]) {

	if (SDLNet_Init() == -1) {
		cerr << "SDLNet_Init error: " << SDLNet_GetError() << endl;
		return 1;
	}
	
	string serverIP;
	int serverPort;
	string pseudo;
	
	cout<<"Server adress: ";
	cin>>serverIP;
	cout<<"Target port: ";
	cin>>serverPort;
	cout<<"Pseudo: ";
	cin>>pseudo;
	
	IPaddress ip;
	if (SDLNet_ResolveHost(&ip, serverIP.c_str() , serverPort) == -1) {
		cerr << "Resolve Host error: " << SDLNet_GetError() << endl;
		SDLNet_Quit();
		return 1;
	}
	
	TCPsocket clientSocket= SDLNet_TCP_Open(&ip);
	if (!clientSocket) {
		cerr << "TCP Open error: " << SDLNet_GetError() << endl;
		SDLNet_Quit();
		return 1;
	}
	
	InitWindow(width, height, "My first chat window!");
	SetTargetFPS(60);

	int bytesSent = SDLNet_TCP_Send(clientSocket, pseudo.c_str(), pseudo.length() + 1);
	if (bytesSent < pseudo.length() + 1) {
		cerr << "SDLNet TCP Send error: " << SDLNet_GetError() << endl;
		SDLNet_TCP_Close(clientSocket);
		SDLNet_Quit();
		return 1;
	}
	
	string typing;
	string prev;
	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(GRAY);
		DrawText("Welcome to ChArtFX!", 220, 15, 25, WHITE);
		DrawRectangle(20, 50, width-40, height -150, DARKGRAY);
		for(int msg = 0; msg < mLog.size(); msg++){
			DrawText(mLog[msg].content.c_str(), 30, 75 + (msg*30), 15, (mLog[msg].fromMe)?SKYBLUE:PURPLE);
		}

		DrawRectangle(20, height - 90, width-40, 50, LIGHTGRAY);
		int inputChar = GetCharPressed();
		if(inputChar != 0){ //A character is pressed on the keyboard
			typing += static_cast<char>(inputChar);
		}
		if(typing.size() > 0){
			if(IsKeyPressed(KEY_BACKSPACE)) typing.pop_back();
			else if (IsKeyPressed(KEY_ENTER)){
				int bytesSent = SDLNet_TCP_Send(clientSocket, typing.c_str(), typing.length() + 1);
				if (bytesSent < typing.length() + 1) {
					cerr << "SDLNet TCP Send error2: " << SDLNet_GetError() << endl;
					SDLNet_TCP_Close(clientSocket);
					SDLNet_Quit();
					return 1;
				}
				if(typing=="EXIT-CHAT"){
					SDLNet_TCP_Close(clientSocket);
					SDLNet_Quit();
					return 0;
				}

				cout << "Sent " << bytesSent << " bytes to the server !" << std::endl;

				prev = typing;
				typing.clear();
			}

			DrawText(typing.c_str(), 30, height - 75, 25, DARKBLUE);
		}
		
		SDLNet_SocketSet socketSet = SDLNet_AllocSocketSet(1);
		SDLNet_AddSocket(socketSet, reinterpret_cast<SDLNet_GenericSocket>(clientSocket));
		if (SDLNet_CheckSockets(socketSet, 0) != 0){
			char recBuffer[1024];
			int bytesRead = SDLNet_TCP_Recv(clientSocket, recBuffer, sizeof(recBuffer));
			auto err = SDLNet_GetError();
			if (std::strlen(err)>0) {
				cerr << "SDLNet TCP Recv error: " << err << endl;
				SDLNet_TCP_Close(clientSocket);
				SDLNet_Quit();
				return 1;
			}
			mLog.push_back(Message{(string(recBuffer)==pseudo+": "+prev), string(recBuffer)});
			if(mLog.size()==19)mLog.erase(mLog.begin());
			if(string(recBuffer)==pseudo+": "+prev) prev.clear();
		}

		//cout << "Incoming response: " << buffer << endl;
		


		EndDrawing();
	}

	cout << "Thank you for using ChArtFX !\n";
	return 0;
}
