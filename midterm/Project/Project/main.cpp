#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "BitStream.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <map>
#include <mutex>

static unsigned int SERVER_PORT = 65000;
static unsigned int CLIENT_PORT = 65001;
static unsigned int MAX_CONNECTIONS = 3;
static int index = 0;
static int playrind = 0;
enum NetworkState
{
	NS_Init = 0,
	NS_PendingStart,
	NS_Started,
	NS_Lobby,
	NS_CHOOSECLASS,
	NS_Pending,
	NS_INGAME,
	NS_RealStartwaitgame,
	NS_INPUTCHOICE,
	
};

bool isServer = false;
bool isRunning = true;

RakNet::RakPeerInterface *g_rakPeerInterface = nullptr;
RakNet::SystemAddress g_serverAddress;

std::mutex g_networkState_mutex;
NetworkState g_networkState = NS_Init;

enum {
	ID_THEGAME_LOBBY_READY = ID_USER_PACKET_ENUM,
	ID_PLAYER_READY,
	ID_PLAYER_RENAME,
	ID_CHOOSE_CLASS,
	ID_THEGAME_START,
	ID_CEACK_ALL_READY,
	ID_MAIN_GAME,
	ID_THELOOP,
	ID_healinf,
	ID_deaminf,
	ID_win,
	ID_dead,
};

enum EPlayerClass
{
	Mage = 0,
	Rogue,
	Knight,
	null,
};
enum stats {
	damage = 0,
	heal,
};
struct SPlayer
{
	std::string m_name = "";
	 int m_health = 100;
	EPlayerClass m_class = null;
	int Damge;
	int heal;
	int index = 4;
	stats thestat;
	bool isdealth = false;
	RakNet::RakString targetstring;
	RakNet::SystemAddress playeraddress;
	//function to send a packet with name/health/class etc
	void SendName(RakNet::SystemAddress systemAddress, bool isBroadcast)
	{

	

		RakNet::BitStream writeBs;
		writeBs.Write((RakNet::MessageID)ID_PLAYER_READY);
		RakNet::RakString name(m_name.c_str());
		writeBs.Write(name);
		writeBs.Write(index);
		//returns 0 when something is wrong
		assert(g_rakPeerInterface->Send(&writeBs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, systemAddress, isBroadcast));
	}


	void ChooseClass(RakNet::SystemAddress systemAddress, bool isBroadcast)
	{

		


		RakNet::BitStream writeBs;
		writeBs.Write((RakNet::MessageID)ID_CHOOSE_CLASS);
		

		//returns 0 when something is wrong
		assert(g_rakPeerInterface->Send(&writeBs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, systemAddress, isBroadcast));
	}

	void order(RakNet::SystemAddress systemAddress, bool isBroadcast) {
		RakNet::BitStream writeBs;
		writeBs.Write((RakNet::MessageID)ID_MAIN_GAME);
		RakNet::RakString name(m_name.c_str());
		writeBs.Write(name);
		EPlayerClass sendclassinf = m_class;
		writeBs.Write(sendclassinf);
		int sendhealth = m_health;
		writeBs.Write(sendhealth);
		
		
		//returns 0 when something is wrong
		assert(g_rakPeerInterface->Send(&writeBs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, systemAddress, isBroadcast));
	}

	void healinf(RakNet::SystemAddress systemAddress, bool isBroadcast) {
		RakNet::BitStream writeBs;
		writeBs.Write((RakNet::MessageID)ID_healinf);
		RakNet::RakString name(m_name.c_str());
		writeBs.Write(name);
		EPlayerClass sendclassinf = m_class;
		writeBs.Write(sendclassinf);
		int sendhealth = m_health;
		writeBs.Write(sendhealth);

		//returns 0 when something is wrong
		assert(g_rakPeerInterface->Send(&writeBs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, systemAddress, isBroadcast));
	}
	void deaminf(RakNet::SystemAddress systemAddress, bool isBroadcast) {
		RakNet::BitStream writeBs;
		writeBs.Write((RakNet::MessageID)ID_deaminf);
		RakNet::RakString name(m_name.c_str());
		writeBs.Write(name);
		EPlayerClass sendclassinf = m_class;
		writeBs.Write(sendclassinf);
		int sendhealth = m_health;
		writeBs.Write(sendhealth);
		writeBs.Write(targetstring);
		//returns 0 when something is wrong
		assert(g_rakPeerInterface->Send(&writeBs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, systemAddress, isBroadcast));
	}
	void losecondiction(RakNet::SystemAddress systemAddress, bool isBroadcast) {
		RakNet::BitStream writeBs;
		writeBs.Write((RakNet::MessageID)ID_dead);
		RakNet::RakString name(m_name.c_str());
		writeBs.Write(name);
		EPlayerClass sendclassinf = m_class;
		writeBs.Write(sendclassinf);
		int sendhealth = m_health;
		writeBs.Write(sendhealth);
		writeBs.Write(targetstring);
		//returns 0 when something is wrong
		assert(g_rakPeerInterface->Send(&writeBs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, systemAddress, isBroadcast));
	}
	void wincondiction(RakNet::SystemAddress systemAddress, bool isBroadcast) {
		RakNet::BitStream writeBs;
		writeBs.Write((RakNet::MessageID)ID_win);
		RakNet::RakString name(m_name.c_str());
		writeBs.Write(name);
		EPlayerClass sendclassinf = m_class;
		writeBs.Write(sendclassinf);
		int sendhealth = m_health;
		writeBs.Write(sendhealth);
		
		//returns 0 when something is wrong
		assert(g_rakPeerInterface->Send(&writeBs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, systemAddress, isBroadcast));
	}
};

std::map<unsigned long, SPlayer> m_players;
void dead(RakNet::Packet* packet) {
	RakNet::BitStream bs(packet->data, packet->length, false);
	RakNet::MessageID messageId;
	bs.Read(messageId);
	RakNet::RakString userName;
	bs.Read(userName);
	
	std::cout << userName << " is dead."<< std::endl;
}
void win(RakNet::Packet* packet) {
	RakNet::BitStream bs(packet->data, packet->length, false);
	RakNet::MessageID messageId;
	bs.Read(messageId);
	RakNet::RakString userName;
	bs.Read(userName);
	
	std::cout << userName << " is winner." << std::endl;
}
void deamin(RakNet::Packet* packet) {
	RakNet::BitStream bs(packet->data, packet->length, false);
	RakNet::MessageID messageId;
	bs.Read(messageId);
	RakNet::RakString userName;
	bs.Read(userName);
	EPlayerClass sendclassinf;
	bs.Read(sendclassinf);
	int sendhealth;
	bs.Read(sendhealth);
	RakNet::RakString targetstring;
	bs.Read(targetstring);
	std::cout << userName << " hit " << targetstring << " right now." << std::endl;
}
void healin(RakNet::Packet* packet) {
	RakNet::BitStream bs(packet->data, packet->length, false);
	RakNet::MessageID messageId;
	bs.Read(messageId);
	RakNet::RakString userName;
	bs.Read(userName);
	EPlayerClass sendclassinf;
	bs.Read(sendclassinf);
	int sendhealth;
	bs.Read(sendhealth);
	std::cout << userName << " is healing, his HP is " << sendhealth << " right now. " << std::endl;
}
//server
void OnIncomingConnection(RakNet::Packet* packet)
{
	//must be server in order to recieve connection
	assert(isServer);
	//int index = 0;
	m_players.insert(std::make_pair(RakNet::RakNetGUID::ToUint32(packet->guid), SPlayer()));
	for (std::map<unsigned long, SPlayer>::iterator it = m_players.begin(); it != m_players.end(); ++it)
	{
		SPlayer& player = it->second;
		if (player.index == 4)
		{
			player.index = playrind;
			playrind++;
		}
	}
	std::cout << "Total Players: " << m_players.size() << std::endl;
}

//client
void OnConnectionAccepted(RakNet::Packet* packet)
{
	//server should not ne connecting to anybody, 
	//clients connect to server
	assert(!isServer);
	g_networkState_mutex.lock();
	g_networkState = NS_Lobby;
	g_networkState_mutex.unlock();
	g_serverAddress = packet->systemAddress;

}

//this is on the client side
void DisplayPlayerReady(RakNet::Packet* packet)
{
	
	RakNet::BitStream bs(packet->data, packet->length, false);
	RakNet::MessageID messageId;
	bs.Read(messageId);
	RakNet::RakString userName;
	bs.Read(userName);
	int index;
	bs.Read(index);
	if (userName != "")
	{	
		std::cout << userName.C_String() <<"  player"<< index <<" has joined" << std::endl;
		
	}
}

//rename our player
void Rename(RakNet::Packet* packet) {
	char userInput[255];
	std::cout << "The name already exit, Try again. Enter your name to play or type quit to leave" << std::endl;
	std::cin >> userInput;
	//quitting is not acceptable in our game, create a crash to teach lesson
	assert(strcmp(userInput, "quit"));





	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)ID_THEGAME_LOBBY_READY);
	RakNet::RakString Cname(userInput);

	bs.Write(userInput);

	//returns 0 when something is wrong
	assert(g_rakPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, g_serverAddress, false));

	

	g_networkState_mutex.lock();
	g_networkState = NS_Pending;
	g_networkState_mutex.unlock();
}

void ChooseClass(RakNet::Packet* packet) {

	
	RakNet::BitStream bs(packet->data, packet->length, false);
	RakNet::MessageID messageId;
	bs.Read(messageId);
	
	g_networkState_mutex.lock();
	g_networkState = NS_CHOOSECLASS;
	g_networkState_mutex.unlock();
}
//Player already have the same name
bool PlayerExit(RakNet::Packet* packet)
{
	unsigned long guid = RakNet::RakNetGUID::ToUint32(packet->guid);
	std::map<unsigned long, SPlayer>::iterator it = m_players.find(guid);
	//somehow player didn't connect but now is in lobby ready
	assert(it != m_players.end());

	RakNet::BitStream bs(packet->data, packet->length, false);
	RakNet::MessageID messageId;
	bs.Read(messageId);
	RakNet::RakString userName;

	bs.Read(userName);
	SPlayer& currentPlayer = it->second;
	for (std::map<unsigned long, SPlayer>::iterator it = m_players.begin(); it != m_players.end(); ++it)
	{
		
		//skip over the player who just joined
		if (guid == it->first)
		{
			continue;
		}

		SPlayer& player = it->second;
		if (userName.C_String() == player.m_name)
		{
			
			
			RakNet::BitStream writeBs;
			writeBs.Write((RakNet::MessageID)ID_PLAYER_RENAME);
		
			

			//returns 0 when something is wrong
			assert(g_rakPeerInterface->Send(&writeBs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false));

			return true;
		}
		
	}
	return false;
}
void StartGame(RakNet::Packet* packet) {

	unsigned long guid = RakNet::RakNetGUID::ToUint32(packet->guid);
	std::map<unsigned long, SPlayer>::iterator it = m_players.find(guid);
	//somehow player didn't connect but now is in lobby ready
	assert(it != m_players.end());

	RakNet::BitStream bs(packet->data, packet->length, false);
	RakNet::MessageID messageId;
	bs.Read(messageId);
	EPlayerClass playerc;

	bs.Read(playerc);
	std::cout << playerc << std::endl;


	SPlayer& player = it->second;
	player.m_class = playerc;
	if (playerc == 0)
	{
		player.Damge = 15;
		player.heal = 5;
	}
	else if (playerc == 1)
	{
		player.Damge = 20;
		player.heal = 0;
	}
	else if (playerc == 2) {
		player.Damge = 10;
		player.heal = 10;
	}
	std::cout << player.m_name.c_str() << " class is " << playerc << std::endl;

}

bool checkready(RakNet::Packet* packet)
{
	
	for (std::map<unsigned long, SPlayer>::iterator it = m_players.begin(); it != m_players.end(); ++it)
	{
		//std::cout << m_players.size() << std::endl;
		SPlayer& player = it->second;
		if (player.m_class == 3)
			return false;
		
	}

	return true;
}


void sendOrder(RakNet::Packet* packet) {

	
	unsigned long guid = RakNet::RakNetGUID::ToUint32(packet->guid);
	std::map<unsigned long, SPlayer>::iterator it = m_players.find(guid);
	//somehow player didn't connect but now is in lobby ready
	assert(it != m_players.end());
	
	RakNet::BitStream bs(packet->data, packet->length, false);
	RakNet::MessageID messageId;
	bs.Read(messageId);
	stats currentstats;
	bs.Read(currentstats);
	RakNet::RakString attacktarget;
	bs.Read(attacktarget);
	SPlayer& currentplayer = it->second;
	bool wrongtarget = false;
	int dealthcount = 0;
	if (currentstats == heal)
	{
		if(it->second.m_health < 100)
			currentplayer.m_health += currentplayer.heal;
		currentplayer.healinf(currentplayer.playeraddress, true);
		currentplayer.healinf(currentplayer.playeraddress, false);
	}
	else if (currentstats == damage)
	{

		for (std::map<unsigned long, SPlayer>::iterator its = m_players.begin(); its != m_players.end(); ++its)
		{
			
				if (attacktarget == '0')
				{
					if (its->second.index == 0)
					{
						if (its->second.m_health > 0)
						{
							its->second.m_health -= currentplayer.Damge;
							currentplayer.targetstring = its->second.m_name.c_str();
						}
						else {
							wrongtarget = true;
						}

					}
				}
				else if (attacktarget == '1')
				{
					if (its->second.index == 1)
					{
						if (its->second.m_health > 0)
						{
							its->second.m_health -= currentplayer.Damge;
							currentplayer.targetstring = its->second.m_name.c_str();
						}
						else {
							wrongtarget = true;
						}
					}
				}
				else {
					if (its->second.index == 2)
					{
						if (its->second.m_health > 0)
						{
							its->second.m_health -= currentplayer.Damge;
							currentplayer.targetstring = its->second.m_name.c_str();
						}
						else {
							wrongtarget = true;
						}
					}
				}

			
			 
				
				
			
		}
		if (!wrongtarget)
		{
			
			currentplayer.deaminf(currentplayer.playeraddress, true);
			currentplayer.deaminf(currentplayer.playeraddress, false);
		}
		
	}
	
	
			
			for (std::map<unsigned long, SPlayer>::iterator its = m_players.begin(); its != m_players.end(); ++its)
			{



				SPlayer& player = its->second;

				if (player.m_health <= 0)
				{
					player.isdealth = true;
					player.losecondiction(player.playeraddress, true);
					player.losecondiction(player.playeraddress, false);
					dealthcount++;
				}

			}


			do
			{
				it++;
				if (it == m_players.end())
				{
					it = m_players.begin();
				}
			} while (it->second.isdealth);

			SPlayer& player = it->second;
			if (dealthcount == 2)
			{
				player.wincondiction(player.playeraddress, true);
				player.wincondiction(player.playeraddress, false);

				
			}
			else
			{
				if (wrongtarget) {
					currentplayer.order(currentplayer.playeraddress, false);
				}
				else {
					player.order(player.playeraddress, false);
				}
			}


			
			
		
	
}

void inputorder(RakNet::Packet* packet) {

	


	g_networkState_mutex.lock();
	g_networkState = NS_INPUTCHOICE;
	g_networkState_mutex.unlock();
	
	
	RakNet::BitStream bs(packet->data, packet->length, false);
	RakNet::MessageID messageId;
	bs.Read(messageId);
	RakNet::RakString userName;
	
	bs.Read(userName);
	
	EPlayerClass playerclass;
	bs.Read(playerclass);
	int health;
	bs.Read(health);
	std::cout << "User name: " << userName << std::endl;
	std::cout << "health: " << health << std::endl;
	if (playerclass == 0)
	{
		std::cout << "class: Mage"<< std::endl;

	}
	else if (playerclass == 1)
	{
		std::cout << "class: Rogue" << std::endl;

	}
	else if (playerclass == 2)
	{
		std::cout << "class: Knight" << std::endl;

	}
	
	
}

void loop(RakNet::Packet* packet) {

}
//the server
void OnLobbyReady(RakNet::Packet* packet)
{
	unsigned long guid = RakNet::RakNetGUID::ToUint32(packet->guid);
	std::map<unsigned long, SPlayer>::iterator it = m_players.find(guid);
	//somehow player didn't connect but now is in lobby ready
	assert(it != m_players.end());

	RakNet::BitStream bs(packet->data, packet->length, false);
	RakNet::MessageID messageId;
	bs.Read(messageId);
	RakNet::RakString userName;
	
	bs.Read(userName);
	
	
	
	SPlayer& player = it->second;
	player.m_name = userName;
	player.playeraddress = packet->systemAddress;
	std::cout << userName << " aka " << player.m_name.c_str() << " IS READY!!!!!" << std::endl;
	
	

		//notify all other connected players that this plyer has joined the game
	for (std::map<unsigned long, SPlayer>::iterator it = m_players.begin(); it != m_players.end(); ++it)
	{

		//skip over the player who just joined
		if (guid == it->first)
		{
			continue;
		}

		SPlayer& player = it->second;
		player.SendName(packet->systemAddress, false);
	}


	


			if (m_players.size() == 3)
			{

				//notify all other connected players that this plyer has joined the game
				for (std::map<unsigned long, SPlayer>::iterator it = m_players.begin(); it != m_players.end(); ++it)
				{

					

					SPlayer& player = it->second;
					//player.ChooseClass(g_serverAddress, false);
					player.ChooseClass(packet->systemAddress, true);


				}
				player.ChooseClass(packet->systemAddress, false);
			}
	
			

	player.SendName(packet->systemAddress, true);
	
}

unsigned char GetPacketIdentifier(RakNet::Packet *packet)
{
	if (packet == nullptr)
		return 255;

	if ((unsigned char)packet->data[0] == ID_TIMESTAMP)
	{
		RakAssert(packet->length > sizeof(RakNet::MessageID) + sizeof(RakNet::Time));
		return (unsigned char)packet->data[sizeof(RakNet::MessageID) + sizeof(RakNet::Time)];
	}
	else
		return (unsigned char)packet->data[0];
}




bool HandleLowLevelPackets(RakNet::Packet* packet)
{
	bool isHandled = true;
	// We got a packet, get the identifier with our handy function
	unsigned char packetIdentifier = GetPacketIdentifier(packet);

	// Check if this is a network message packet
	switch (packetIdentifier)
	{
	case ID_DISCONNECTION_NOTIFICATION:
		// Connection lost normally
		printf("ID_DISCONNECTION_NOTIFICATION\n");
		break;
	case ID_ALREADY_CONNECTED:
		// Connection lost normally
		printf("ID_ALREADY_CONNECTED with guid %" PRINTF_64_BIT_MODIFIER "u\n", packet->guid);
		break;
	case ID_INCOMPATIBLE_PROTOCOL_VERSION:
		printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
		break;
	case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
		printf("ID_REMOTE_DISCONNECTION_NOTIFICATION\n");
		break;
	case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
		printf("ID_REMOTE_CONNECTION_LOST\n");
		break;
	case ID_NEW_INCOMING_CONNECTION:
		//client connecting to server
		OnIncomingConnection(packet);
		printf("ID_NEW_INCOMING_CONNECTION\n");
		break;
	case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
		OnIncomingConnection(packet);
		printf("ID_REMOTE_NEW_INCOMING_CONNECTION\n");
		break;
	case ID_CONNECTION_BANNED: // Banned from this server
		printf("We are banned from this server.\n");
		break;
	case ID_CONNECTION_ATTEMPT_FAILED:
		printf("Connection attempt failed\n");
		break;
	case ID_NO_FREE_INCOMING_CONNECTIONS:
		// Sorry, the server is full.  I don't do anything here but
		// A real app should tell the user
		printf("ID_NO_FREE_INCOMING_CONNECTIONS\n");
		break;

	case ID_INVALID_PASSWORD:
		printf("ID_INVALID_PASSWORD\n");
		break;

	case ID_CONNECTION_LOST:
		// Couldn't deliver a reliable packet - i.e. the other system was abnormally
		// terminated
		printf("ID_CONNECTION_LOST\n");
		break;

	case ID_CONNECTION_REQUEST_ACCEPTED:
		// This tells the client they have connected
		printf("ID_CONNECTION_REQUEST_ACCEPTED to %s with GUID %s\n", packet->systemAddress.ToString(true), packet->guid.ToString());
		printf("My external address is %s\n", g_rakPeerInterface->GetExternalID(packet->systemAddress).ToString(true));
		OnConnectionAccepted(packet);
		break;
	case ID_CONNECTED_PING:
	case ID_UNCONNECTED_PING:
		printf("Ping from %s\n", packet->systemAddress.ToString(true));
		break;
	default:
		isHandled = false;
		break;
	}
	return isHandled;
}


void InputHandler()
{
	while (isRunning)
	{
		char userInput[255];
		char classInput[255];

		if (g_networkState == NS_Init)
		{
			std::cout << "press (s) for server (c) for client" << std::endl;
			std::cin >> userInput;
			while (userInput[0] != 's' && userInput[0] != 'c')
			{
				
				std::cout << "I don't understand please press (s) for server (c) for client " << std::endl;
				std::cin >> userInput;
			}
			
			isServer = (userInput[0] == 's');
			g_networkState_mutex.lock();
			g_networkState = NS_PendingStart;
			g_networkState_mutex.unlock();

		}
		else if (g_networkState == NS_Lobby)
		{

			std::cout << "Enter your name to play or type quit to leave" << std::endl;
			std::cin >> userInput;
			//quitting is not acceptable in our game, create a crash to teach lesson
			assert(strcmp(userInput, "quit"));

			
			
		

			RakNet::BitStream bs;
			bs.Write((RakNet::MessageID)ID_THEGAME_LOBBY_READY);
			RakNet::RakString Cname(userInput);

			bs.Write(userInput);
		
			
			//returns 0 when something is wrong
			assert(g_rakPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, g_serverAddress, false));

			

			
			
			g_networkState_mutex.lock();
			g_networkState = NS_Pending;
			g_networkState_mutex.unlock();
		}
		else if (g_networkState == NS_Pending)
		{
			static bool doOnce = false;
			if (!doOnce)
				std::cout << "pending..." << std::endl;

			doOnce = true;

		}
		else if (g_networkState == NS_CHOOSECLASS)
		{
			static bool doOnce = false;
			if (!doOnce)
			{
				std::cout << "room is full. Now choose your class" << std::endl;
				std::cout << "type one of the classes. 0 represent Mage,1 represent Rogue and 2 represnet Knight." << std::endl;
				std::cin >> classInput;
				EPlayerClass Playerclass;
				while (classInput[0] != '0' && classInput[0] != '1' && classInput[0] != '2')
				{
					std::cout << "type one of the classes. 0 represent Mage,1 represent Rogue and 2 represnet Knight." << std::endl;
					std::cin >> classInput;
				}
				
				
				RakNet::BitStream bs;
				bs.Write((RakNet::MessageID)ID_THEGAME_START);
				
				
				//bs.Write(theaddress);
					if (classInput[0] == '0')
					{
						Playerclass = Mage;
						std::cout << "You are Mage, wait other players" << std::endl;
					}
					else if (classInput[0] == '1')
					{
						Playerclass = Rogue;
						std::cout << "You are Rogue, wait other players" << std::endl;
					}
					else if (classInput[0] == '2') {
						Playerclass = Knight;
						std::cout << "You are Knight, wait other players" << std::endl;
					}
					
				bs.Write(Playerclass);


				//returns 0 when something is wrong
				assert(g_rakPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, g_serverAddress, false));
			}
			
			doOnce = true;
			
			g_networkState_mutex.lock();
			g_networkState = NS_INGAME;
			g_networkState_mutex.unlock();
			
		}
		else if (g_networkState == NS_INGAME) {
			RakNet::BitStream bs;
			bs.Write((RakNet::MessageID)ID_CEACK_ALL_READY);
			//returns 0 when something is wrong
			assert(g_rakPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, g_serverAddress, false));
			g_networkState_mutex.lock();
			g_networkState = NS_RealStartwaitgame;
			g_networkState_mutex.unlock();
		}
		else if (g_networkState == NS_RealStartwaitgame) {
			static bool doOnce = false;
			char statinput[255];
			if (!doOnce)
				std::cout << "waitting..." << std::endl;
			//std::cout << "type f to check stat " << std::endl;
			//std::cin >> statinput;
			doOnce = true;
		}
		else if (g_networkState == NS_INPUTCHOICE)
		{
			std::cout << "It is your turn. input 0 to attack, input 1 to heal and input f to skip" << std::endl;
			RakNet::BitStream bs;
			bs.Write((RakNet::MessageID)ID_CEACK_ALL_READY);
			/*
			RakNet::BitStream bs(packet->data, packet->length, false);
			RakNet::MessageID messageId;
			bs.Read(messageId);
			RakNet::RakString userName;

			bs.Read(userName);
			EPlayerClass playerclass;
			bs.Read(playerclass);
			int health;
			bs.Read(health);
			std::cout << "User name: " << userName << std::endl;
			std::cout << "health: " << health << std::endl;
			std::cout << "class: " << playerclass << std::endl;
			*/
			std::cin >> userInput;
			while (userInput[0] != '0' && userInput[0] != '1'&& userInput[0] != 'f')
			{
				std::cin >> userInput;
			}
			if (userInput[0] == 'f')
			{
				
				//returns 0 when something is wrong
				
			}
			else if (userInput[0] == '1')
			{
				stats a = heal;
				bs.Write(a);
			}
			else if (userInput[0] == '0')
			{
				stats a = damage;
				bs.Write(a);
				char attplayernum[255];
				std::cout << "enter 0, 1, 2 to attack different player, if the target is dead you still hit it, this line will just show to you again." << std::endl;
				std::cin >> attplayernum;
				while (attplayernum[0] != '0' && attplayernum[0] != '1' && attplayernum[0] != '2')
				{
					std::cin >> attplayernum;
				}
				RakNet::RakString Cname(attplayernum);
				bs.Write(attplayernum);
			}
			assert(g_rakPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, g_serverAddress, false));
			g_networkState_mutex.lock();
			g_networkState = NS_RealStartwaitgame;
			g_networkState_mutex.unlock();
		}
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
}

void PacketHandler()
{
	while (isRunning)
	{
		for (RakNet::Packet* packet = g_rakPeerInterface->Receive(); packet != nullptr; g_rakPeerInterface->DeallocatePacket(packet), packet = g_rakPeerInterface->Receive())
		{
			
			if (!HandleLowLevelPackets(packet))
			{
				//our game specific packets
				unsigned char packetIdentifier = GetPacketIdentifier(packet);
				switch (packetIdentifier)
				{
				case ID_THEGAME_LOBBY_READY:
					if(!PlayerExit(packet))
						OnLobbyReady(packet);
					break;
				case ID_PLAYER_READY:
					DisplayPlayerReady(packet);
					break;
				case ID_PLAYER_RENAME:
					Rename(packet);
					break;
				case ID_CHOOSE_CLASS:
					ChooseClass(packet);
					break;
				case ID_THEGAME_START:
					StartGame(packet);
					break;
				case ID_CEACK_ALL_READY:
					if (checkready(packet))
					{
						sendOrder(packet);
					}
						
					break;
				case ID_MAIN_GAME:
					inputorder(packet);
					break;
				case ID_THELOOP:
					break;
				case ID_healinf:
					healin(packet);
					break;
				case ID_deaminf:
					deamin(packet);
					break;
				case ID_win:
					win(packet);
					break;
				case ID_dead:
					dead(packet);
					break;
				default:
					break;
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
}

int main()
{
	g_rakPeerInterface = RakNet::RakPeerInterface::GetInstance();

	std::thread inputHandler(InputHandler);
	std::thread packetHandler(PacketHandler);

	while (isRunning)
	{
		if (g_networkState == NS_PendingStart)
		{
			if (isServer)
			{
				RakNet::SocketDescriptor socketDescriptors[1];
				socketDescriptors[0].port = SERVER_PORT;
				socketDescriptors[0].socketFamily = AF_INET; // Test out IPV4

				bool isSuccess = g_rakPeerInterface->Startup(MAX_CONNECTIONS, socketDescriptors, 1) == RakNet::RAKNET_STARTED;
				assert(isSuccess);
				//ensures we are server
				g_rakPeerInterface->SetMaximumIncomingConnections(MAX_CONNECTIONS);
				std::cout << "server started" << std::endl;
				g_networkState_mutex.lock();
				g_networkState = NS_Started;
				g_networkState_mutex.unlock();
			}
			//client
			else
			{
				RakNet::SocketDescriptor socketDescriptor(CLIENT_PORT, 0);
				socketDescriptor.socketFamily = AF_INET;

				while (RakNet::IRNS2_Berkley::IsPortInUse(socketDescriptor.port, socketDescriptor.hostAddress, socketDescriptor.socketFamily, SOCK_DGRAM) == true)
					socketDescriptor.port++;

				RakNet::StartupResult result = g_rakPeerInterface->Startup(8, &socketDescriptor, 1);
				assert(result == RakNet::RAKNET_STARTED);

				g_networkState_mutex.lock();
				g_networkState = NS_Started;
				g_networkState_mutex.unlock();

				g_rakPeerInterface->SetOccasionalPing(true);
				//"127.0.0.1" = local host = your machines address
				RakNet::ConnectionAttemptResult car = g_rakPeerInterface->Connect("127.0.0.1", SERVER_PORT, nullptr, 0);
				RakAssert(car == RakNet::CONNECTION_ATTEMPT_STARTED);
				std::cout << "client attempted connection..." << std::endl;

			}
		}

	}

	//std::cout << "press q and then return to exit" << std::endl;
	//std::cin >> userInput;

	inputHandler.join();
	packetHandler.join();
	return 0;
}