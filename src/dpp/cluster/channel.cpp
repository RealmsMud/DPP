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
#include <dpp/discord.h>
#include <dpp/cluster.h>
#include <dpp/nlohmann/json.hpp>

namespace dpp {

void cluster::channel_create(const class channel &c, command_completion_event_t callback) {
	this->post_rest(API_PATH "/guilds", std::to_string(c.guild_id), "channels", m_post, c.build_json(), [callback](json &j, const http_request_completion_t& http) {
		if (callback) {
			callback(confirmation_callback_t("channel", channel().fill_from_json(&j), http));
		}
	});
}

void cluster::channel_delete_permission(const class channel &c, snowflake overwrite_id, command_completion_event_t callback) {
	this->post_rest(API_PATH "/channels", std::to_string(c.id), "permissions/" + std::to_string(overwrite_id), m_delete, "", [callback](json &j, const http_request_completion_t& http) {
		if (callback) {
			callback(confirmation_callback_t("confirmation", confirmation(), http));
		}
	});
}


void cluster::channel_delete(snowflake channel_id, command_completion_event_t callback) {
	this->post_rest(API_PATH "/channels", std::to_string(channel_id), "", m_delete, "", [callback](json &j, const http_request_completion_t& http) {
		if (callback) {
			callback(confirmation_callback_t("confirmation", confirmation(), http));
		}
	});
}

void cluster::channel_edit_permissions(const class channel &c, snowflake overwrite_id, uint32_t allow, uint32_t deny, bool member, command_completion_event_t callback) {
	json j({ {"allow", std::to_string(allow)}, {"deny", std::to_string(deny)}, {"type", member ? 1 : 0}  });
	this->post_rest(API_PATH "/channels", std::to_string(c.id), "permissions/" + std::to_string(overwrite_id), m_put, j.dump(), [callback](json &j, const http_request_completion_t& http) {
		if (callback) {
			callback(confirmation_callback_t("confirmation", confirmation(), http));
		}
	});
}

void cluster::channel_edit_position(const class channel &c, command_completion_event_t callback) {
	json j({ {"id", c.id}, {"position", c.position}  });
	this->post_rest(API_PATH "/guilds", std::to_string(c.guild_id), "channels/" + std::to_string(c.id), m_patch, j.dump(), [callback](json &j, const http_request_completion_t& http) {
		if (callback) {
			callback(confirmation_callback_t("channel", channel().fill_from_json(&j), http));
		}
	});
}

void cluster::channel_edit(const class channel &c, command_completion_event_t callback) {
	json j = c.build_json(true);
	auto p = j.find("position");
	if (p != j.end()) {
		j.erase(p);
	}
	this->post_rest(API_PATH "/channels", std::to_string(c.id), "", m_patch, j.dump(), [callback](json &j, const http_request_completion_t& http) {
		if (callback) {
			callback(confirmation_callback_t("channel", channel().fill_from_json(&j), http));
		}
	});
}

void cluster::channel_follow_news(const class channel &c, snowflake target_channel_id, command_completion_event_t callback) {
	json j({ {"webhook_channel_id", target_channel_id} });
	this->post_rest(API_PATH "/channels", std::to_string(c.id), "followers", m_post, j.dump(), [callback](json &j, const http_request_completion_t& http) {
		if (callback) {
			callback(confirmation_callback_t("confirmation", confirmation(), http));
		}
	});
}

void cluster::channel_get(snowflake c, command_completion_event_t callback) {
	this->post_rest(API_PATH "/channels", std::to_string(c), "", m_get, "", [callback](json &j, const http_request_completion_t& http) {
		if (callback) {
			callback(confirmation_callback_t("channel", channel().fill_from_json(&j), http));
		}
	});
}

void cluster::channel_invite_create(const class channel &c, const class invite &i, command_completion_event_t callback) {
	this->post_rest(API_PATH "/channels", std::to_string(c.id), "invites", m_post, i.build_json(), [callback](json &j, const http_request_completion_t& http) {
		if (callback) {
			callback(confirmation_callback_t("invite", invite().fill_from_json(&j), http));
		}
	});
}


void cluster::channel_invites_get(const class channel &c, command_completion_event_t callback) {
	this->post_rest(API_PATH "/channels", std::to_string(c.id), "invites", m_get, "", [callback](json &j, const http_request_completion_t& http) {
		invite_map invites;
		for (auto & curr_invite : j) {
			invites[StringNotNull(&curr_invite, "code")] = invite().fill_from_json(&curr_invite);
		}
		if (callback) {
			callback(confirmation_callback_t("invite_map", invites, http));
		}
	});
}

void cluster::channel_typing(const class channel &c, command_completion_event_t callback) {
	this->post_rest(API_PATH "/channels", std::to_string(c.id), "typing", m_post, "", [callback](json &j, const http_request_completion_t& http) {
		if (callback) {
			callback(confirmation_callback_t("confirmation", confirmation(), http));
		}
	});
}

void cluster::channels_get(snowflake guild_id, command_completion_event_t callback) {
	this->post_rest(API_PATH "/guilds", std::to_string(guild_id), "channels", m_get, "", [callback](json &j, const http_request_completion_t& http) {
		if (callback) {
			channel_map channels;
			for (auto & curr_channel: j) {
				channels[SnowflakeNotNull(&curr_channel, "id")] = channel().fill_from_json(&curr_channel);
			}
			callback(confirmation_callback_t("channel_map", channels, http));
		}
	});
}

};