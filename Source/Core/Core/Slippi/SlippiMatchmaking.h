#pragma once

#include "Common/CommonTypes.h"
#include "Common/Thread.h"
#include "Core/Slippi/SlippiNetplay.h"
#include "Core/Slippi/SlippiUser.h"

#ifndef _WIN32
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <unordered_map>
#include <vector>
#include <json.hpp>

<<<<<<< HEAD
=======
using json = nlohmann::json;

>>>>>>> origin/doubles
class SlippiMatchmaking
{
  public:
	SlippiMatchmaking(SlippiUser *user);
	~SlippiMatchmaking();

	enum OnlinePlayMode
	{
		RANKED = 0,
		UNRANKED = 1,
		DIRECT = 2,
		TEAMS = 3,
	};

	enum ProcessState
	{
		IDLE,
		INITIALIZING,
		MATCHMAKING,
		OPPONENT_CONNECTING,
		CONNECTION_SUCCESS,
		ERROR_ENCOUNTERED,
	};

	struct MatchSearchSettings
	{
		OnlinePlayMode mode = OnlinePlayMode::RANKED;
		std::string connectCode = "";
	};

	void FindMatch(MatchSearchSettings settings);
	void MatchmakeThread();
	ProcessState GetMatchmakeState();
	bool IsSearching();
	std::unique_ptr<SlippiNetplayClient> GetNetplayClient();
	std::string GetErrorMessage();
	int LocalPlayerIndex();
	std::vector<SlippiUser::UserInfo> GetPlayerInfo();
	std::string GetPlayerName(u8 port);
	u8 RemotePlayerCount();

  protected:
	const std::string MM_HOST_DEV = "35.197.121.196"; // Dev host
	const std::string MM_HOST_PROD = "35.247.98.48";  // Production host
	const u16 MM_PORT = 43113;

	std::string MM_HOST = "";

	ENetHost *m_client;
	ENetPeer *m_server;

	std::default_random_engine generator;

	bool isMmConnected = false;

	std::thread m_matchmakeThread;

	MatchSearchSettings m_searchSettings;

	ProcessState m_state;
	std::string m_errorMsg = "";

	SlippiUser *m_user;

	int m_isSwapAttempt = false;

	int m_hostPort;
	int m_localPlayerPort;
	std::vector<std::string> m_remoteIps;
	std::vector<SlippiUser::UserInfo> m_playerInfo;
	bool m_joinedLobby;
	bool m_isHost;

	std::unique_ptr<SlippiNetplayClient> m_netplayClient;

	const std::unordered_map<ProcessState, bool> searchingStates = {
	    {ProcessState::INITIALIZING, true},
	    {ProcessState::MATCHMAKING, true},
	    {ProcessState::OPPONENT_CONNECTING, true},
	};

	void disconnectFromServer();
	void terminateMmConnection();
	void sendMessage(json msg);
	int receiveMessage(json &msg, int maxAttempts);

	void sendHolePunchMsg(std::string remoteIp, u16 remotePort, u16 localPort);

	void startMatchmaking();
	void handleMatchmaking();
	void handleConnecting();
};
