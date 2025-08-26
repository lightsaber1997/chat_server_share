// ---------------------------------------------------------------------
// ChatManager.hpp
// ---------------------------------------------------------------------
// SPDX-License-Identifier: BSD-3-Clause
// ---------------------------------------------------------------------

#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include "GenieDialog.h"   // Genie SDK types
#include "PromptHandler.hpp"

// ---------------------------------------------------------------------
// GenieChat: wrapper for a single dialog session
// ---------------------------------------------------------------------
class GenieChat {
public:
    GenieDialog_Handle_t m_dialog_handle = nullptr;
    bool is_stateful = false;
    bool is_first_prompt = true;

    GenieChat(GenieDialogConfig_Handle_t config_handle, bool stateful);
    ~GenieChat();
};

// ---------------------------------------------------------------------
// ChatManager: manages multiple GenieChat sessions
// ---------------------------------------------------------------------
class ChatManager {
public:
    using GenieResponseCallback =
        std::function<void(const char*, GenieDialog_SentenceCode_t)>;

    explicit ChatManager(const std::string& config_json);
    ~ChatManager();

    std::string create_new_dialogue(bool is_stateful = false);
    void remove_dialogue(const std::string& dialogue_id);

    /// First-turn query (requires sys + user prompt)
    void query(const std::string& dialogue_id,
               const std::string& sys_prompt,
               const std::string& user_prompt,
               GenieResponseCallback callback);

    /// Subsequent query for stateful dialogues (user only)
    void user_query(const std::string& dialogue_id,
                    const std::string& user_prompt,
                    GenieResponseCallback callback);

private:
    std::shared_ptr<GenieChat> get_dialogue(const std::string& dialogue_id);

    GenieDialogConfig_Handle_t m_config_handle = nullptr;
    std::unordered_map<std::string, std::shared_ptr<GenieChat>> m_sessions;
};
