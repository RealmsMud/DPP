/************************************************************************************
 *
 * D++, A Lightweight C++ library for Discord
 *
 * Copyright 2021 Craig Edwards and D++ contributors 
 * (https://github.com/brainboxdotcc/DPP/graphs/contributors)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/

#pragma once
#include <dpp/export.h>
#include <string>
#include <map>
#include <vector>
#include <dpp/json_fwd.hpp>
#include <dpp/wsclient.h>
#include <dpp/dispatcher.h>
#include <dpp/cluster.h>
#include <dpp/discordvoiceclient.h>
#include <dpp/event.h>
#include <queue>
#include <thread>
#include <deque>
#include <mutex>

using json = nlohmann::json;

#define DISCORD_API_VERSION	"9"
#define DEFAULT_GATEWAY		"gateway.discord.gg"
#define API_PATH	        "/api/v" DISCORD_API_VERSION
namespace dpp {

// Forward declarations
class cluster;

/** This is an opaque class containing zlib library specific structures.
 * We define it this way so that the public facing D++ library doesnt require
 * the zlib headers be available to build against it.
 */
class zlibcontext;

/**
 * @brief Represents a connection to a voice channel.
 * A client can only connect to one voice channel per guild at a time, so these are stored in a map
 * in the dpp::discord_client keyed by guild_id.
 */
class DPP_EXPORT voiceconn {
	/**
	 * @brief Owning dpp::discord_client instance
	 */
	class discord_client* creator;
public:
	/**
	 * @brief Voice Channel ID
	 */
	snowflake channel_id;

	/**
	 * @brief Websocket hostname for status
	 */
	std::string websocket_hostname;

	/**
	 * @brief Voice Voice session ID
	 */
	std::string session_id;

	/**
	 * @brief Voice websocket token
	 */
	std::string token;

	/**
	 * @brief voice websocket client
	 */
	class discord_voice_client* voiceclient;

	/**
	 * @brief Construct a new voiceconn object
	 */
	voiceconn() = default;

	/**
	 * @brief Construct a new voiceconn object
	 * 
	 * @param o owner
	 * @param _channel_id voice channel id
	 */
	voiceconn(class discord_client* o, snowflake _channel_id);

	/**
	 * @brief Destroy the voiceconn object
	 */
	~voiceconn();

	/**
	 * @brief return true if the connection is ready to connect
	 * (has hostname, token and session id)
	 * 
	 * @return true if ready to connect
	 */
	bool is_ready();
	
	/**
	 * @brief return true if the connection is active (websocket exists)
	 * 
	 * @return true if has an active websocket
	 */
	bool is_active();

	/**
	 * @brief Create websocket object and connect it.
	 * Needs hosname, token and session_id to be set or does nothing.
	 * 
	 * @param guild_id Guild to connect to the voice channel on
	 */
	void connect(snowflake guild_id);

	/**
	 * @brief Disconnect from the currently connected voice channel
	 */
	void disconnect();
};

/** @brief Implements a discord client. Each discord_client connects to one shard and derives from a websocket client. */
class DPP_EXPORT discord_client : public websocket_client
{
protected:
	/**
	 * @brief Needed so that voice_state_update can call dpp::discord_client::disconnect_voice_internal
	 */
	friend class dpp::events::voice_state_update;

	/**
	 * @brief Needed to allow cluster::set_presence to use the ETF functions
	 */
	friend class dpp::cluster;

	/**
	 * @brief Disconnect from the connected voice channel on a guild
	 * 
	 * @param guild_id The guild who's voice channel you wish to disconnect from
	 * @param send_json True if we shold send a json message confirming we are leaving the VC
	 * Should be set to false if we already receive this message in an event.
	 */
	void disconnect_voice_internal(snowflake guild_id, bool send_json = true);

private:

	/** Mutex for message queue */
	std::mutex queue_mutex;

	/** Queue of outbound messages */
	std::deque<std::string> message_queue;

	/** Thread this shard is executing on */
	std::thread* runner;

	/**
	 * @brief Run shard loop under a thread.
	 * Calls discord_client::Run() from within a std::thread.
	 */
	void ThreadRun();

	/** If true, stream compression is enabled */
	bool compressed;

	/** ZLib decompression buffer */
	unsigned char* decomp_buffer;

	/** Decompressed string */
	std::string decompressed;

	/**
	 * @brief This object contains the various zlib structs which
	 * are not usable by the user of the library directly. They
	 * are wrapped within this opaque object so that this header
	 * file does not bring in a dependency on zlib.h.
	 */
	zlibcontext* zlib;

	/** Total decompressed received bytes */
	uint64_t decompressed_total;

	/** Last connect time of cluster */
	time_t connect_time;

	/** Time last ping sent to websocket, in fractional seconds */
	double ping_start;

	/** ETF parser for when in ws_etf mode */
	class etf_parser* etf;

	/**
	 * @brief Convert a JSON object to string.
	 * In JSON protocol mode, call json.dump(), and in ETF mode,
	 * call etf::build().
	 * 
	 * @param json nlohmann::json object to convert
	 * @return * std::string string output in the correct format
	 */
	std::string jsonobj_to_string(const nlohmann::json& json);

	/**
	 * @brief Initialise ZLib (websocket compression)
	 */
	void SetupZLib();

	/**
	 * @brief Shut down ZLib (websocket compression)
	 */
	void EndZLib();

public:
	/** Owning cluster */
	class dpp::cluster* creator;

	/** Heartbeat interval for sending heartbeat keepalive */
	uint32_t heartbeat_interval;

	/** Last heartbeat */
	time_t last_heartbeat;

	/** Shard ID of this client */
	uint32_t shard_id;

	/** Total number of shards */
	uint32_t max_shards;

	/** Thread ID */
	std::thread::native_handle_type thread_id;

	/** Last sequence number received, for resumes and pings */
	uint64_t last_seq;

	/** Discord bot token */
	std::string token;

	/** Privileged gateway intents */
	uint32_t intents;

	/** Discord session id */
	std::string sessionid;

	/** Mutex for voice connections map */
	std::mutex voice_mutex;

	/** Resume count */
	uint32_t resumes;

	/** Reconnection count */
	uint32_t reconnects;

	/** Websocket latency in fractional seconds */
	double websocket_ping;

	/** True if READY or RESUMED has been received */
	bool ready;

	/** Last heartbeat ACK (opcode 11) */
	time_t last_heartbeat_ack;

	/** Current websocket protocol, currently either ETF or JSON */
	websocket_protocol_t protocol;

	/** List of voice channels we are connecting to keyed by guild id */
	std::unordered_map<snowflake, voiceconn*> connecting_voice_channels;

	/** Log a message to whatever log the user is using.
	 * The logged message is passed up the chain to the on_log event in user code which can then do whatever
	 * it wants to do with it.
	 * @param severity The log level from dpp::loglevel
	 * @param msg The log message to output
	 */
	virtual void log(dpp::loglevel severity, const std::string &msg) const;

	/** Handle an event (opcode 0)
	 * @param event Event name, e.g. MESSAGE_CREATE
	 * @param j JSON object for the event content
	 * @param raw Raw JSON event string
	 */
	virtual void HandleEvent(const std::string &event, json &j, const std::string &raw);

	/**
	 * @brief Get the Guild Count for this shard
	 * 
	 * @return uint64_t guild count
	 */
	uint64_t get_guild_count();

	/**
	 * @brief Get the Member Count for this shard
	 * 
	 * @return uint64_t member count
	 */
	uint64_t get_member_count();

	/**
	 * @brief Get the Channel Count for this shard
	 * 
	 * @return uint64_t channel count
	 */
	uint64_t get_channel_count();

	/** Fires every second from the underlying socket I/O loop, used for sending heartbeats */
	virtual void one_second_timer();

	/**
	 * @brief Queue a message to be sent via the websocket
	 * 
	 * @param j The JSON data of the message to be sent
	 * @param to_front If set to true, will place the message at the front of the queue not the back
	 * (this is for urgent messages such as heartbeat, presence, so they can take precedence over
	 * chunk requests etc)
	 */
	void QueueMessage(const std::string &j, bool to_front = false);

	/**
	 * @brief Clear the outbound message queue
	 */
	void ClearQueue();

	/**
	 * @brief Get the size of the outbound message queue
	 * 
	 * @return The size of the queue
	 */
	size_t GetQueueSize();

	/**
	 * @brief Returns true if the shard is connected
	 * 
	 * @return True if connected
	 */
	bool is_connected();

	/**
	 * @brief Returns the connection time of the shard
	 * 
	 * @return dpp::utility::uptime Detail of how long the shard has been connected for
	 */
	dpp::utility::uptime get_uptime();

	/**
	 * @brief Construct a new discord_client object
	 * 
	 * @param _cluster The owning cluster for this shard
	 * @param _shard_id The ID of the shard to start
	 * @param _max_shards The total number of shards across all clusters
	 * @param _token The bot token to use for identifying to the websocket
	 * @param intents Privileged intents to use, a bitmask of values from dpp::intents
	 * @param compressed True if the received data will be gzip compressed
	 * @param ws_protocol Websocket protocol to use for the connection, JSON or ETF
	 */
	discord_client(dpp::cluster* _cluster, uint32_t _shard_id, uint32_t _max_shards, const std::string &_token, uint32_t intents = 0, bool compressed = true, websocket_protocol_t ws_protocol = ws_json);

	/** Destructor */
	virtual ~discord_client();

	/** Get decompressed total bytes received */
	uint64_t get_decompressed_bytes_in();

	/** Handle JSON from the websocket.
	 * @param buffer The entire buffer content from the websocket client
	 * @returns True if a frame has been handled
	 */
	virtual bool HandleFrame(const std::string &buffer);

	/** Handle a websocket error.
	 * @param errorcode The error returned from the websocket
	 */
	virtual void Error(uint32_t errorcode);

	/**
	 * @brief Start and monitor I/O loop.
	 * Note that this is a blocking call and is usually executed within a
	 * thread by whatever creates the object.
	 */
	void Run();

	/**
	 * @brief Connect to a voice channel
	 * 
	 * @param guild_id Guild where the voice channel is
	 * @param channel_id Channel ID of the voice channel
	 * @param self_mute True if the bot should mute itself
	 * @param self_deaf True if the bot should deafen itself
	 */
	void connect_voice(snowflake guild_id, snowflake channel_id, bool self_mute = false, bool self_deaf = false);

	/**
	 * @brief Disconnect from the connected voice channel on a guild
	 * 
	 * @param guild_id The guild who's voice channel you wish to disconnect from
	 */
	void disconnect_voice(snowflake guild_id);

	/**
	 * @brief Get the dpp::voiceconn object for a specific guild on this shard.
	 * 
	 * @param guild_id The guild ID to retrieve the voice connection for
	 * @return voiceconn* The voice connection for the guild, or nullptr if there is no
	 * voice connection to this guild.
	 */
	voiceconn* get_voice(snowflake guild_id);
};

};
