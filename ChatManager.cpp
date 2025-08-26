// ---------------------------------------------------------------------
// ChatManager.cpp
// ---------------------------------------------------------------------
// SPDX-License-Identifier: BSD-3-Clause
// ---------------------------------------------------------------------

#include "ChatManager.hpp"
#include <stdexcept>
#include <iostream>

// ---------------------------------------------------------------------
// Helper types (file-private)
// ---------------------------------------------------------------------
namespace {
    struct CallbackWrapper {
        ChatManager::GenieResponseCallback fn;
    };

    void trampoline(const char* response_back,
                    GenieDialog_SentenceCode_t sentence_code,
                    const void* user_data) {
        auto* wrapper = static_cast<const CallbackWrapper*>(user_data);
        if (wrapper && wrapper->fn) {
            wrapper->fn(response_back, sentence_code);
        }
    }
} // namespace

// ---------------------------------------------------------------------
// GenieChat Implementation
// ---------------------------------------------------------------------
GenieChat::GenieChat(GenieDialogConfig_Handle_t config_handle, bool stateful)
    : is_stateful(stateful)
{
    if (GENIE_STATUS_SUCCESS !=
        GenieDialog_create(config_handle, &m_dialog_handle))
    {
        throw std::runtime_error("Failed to create Genie Dialog.");
    }
}

GenieChat::~GenieChat()
{
    if (m_dialog_handle != nullptr)
    {
        if (GENIE_STATUS_SUCCESS != GenieDialog_free(m_dialog_handle)) {
            std::cerr << "Warning: failed to free GenieDialog\n";
        }
        m_dialog_handle = nullptr;
    }
}

// ---------------------------------------------------------------------
// ChatManager Implementation
// ---------------------------------------------------------------------
ChatManager::ChatManager(const std::string& config_json)
{
    if (GENIE_STATUS_SUCCESS !=
        GenieDialogConfig_createFromJson(config_json.c_str(), &m_config_handle))
    {
        throw std::runtime_error("Failed to create Genie Dialog config.");
    }
}

ChatManager::~ChatManager()
{
    m_sessions.clear();

    if (m_config_handle != nullptr)
    {
        if (GENIE_STATUS_SUCCESS != GenieDialogConfig_free(m_config_handle)) {
            std::cerr << "Warning: failed to free GenieDialogConfig\n";
        }
        m_config_handle = nullptr;
    }
}

std::string ChatManager::create_new_dialogue(bool is_stateful) {
    static int counter = 0;
    std::string dialogue_id = "dlg_" + std::to_string(++counter);

    auto chat = std::make_shared<GenieChat>(m_config_handle, is_stateful);
    m_sessions[dialogue_id] = chat;

    return dialogue_id;
}

void ChatManager::remove_dialogue(const std::string& dialogue_id) {
    m_sessions.erase(dialogue_id);
}

void ChatManager::query(const std::string& dialogue_id,
                        const std::string& sys_prompt,
                        const std::string& user_prompt,
                        GenieResponseCallback callback)
{
    auto chat = get_dialogue(dialogue_id);

    llm::prompt::PromptUtils prompt_utils(llm::prompt::ModelType::Llama3);

    std::string tagged_prompt;
    if (chat->is_first_prompt) {
        tagged_prompt = prompt_utils.get_prompt_with_tag(sys_prompt, user_prompt);
        chat->is_first_prompt = false; // mark first turn done
    } else {
        tagged_prompt = prompt_utils.get_subseq_prompt_with_tag(user_prompt);
    }

    CallbackWrapper wrapper{callback};

    Genie_Status_t status = GenieDialog_query(
                                chat->m_dialog_handle,
                                tagged_prompt.c_str(),
                                GENIE_DIALOG_SENTENCE_COMPLETE,
                                trampoline,
                                &wrapper);

    if (status != GENIE_STATUS_SUCCESS) {
        throw std::runtime_error("Failed to get response from GenieDialog.");
    }

    if (!chat->is_stateful) {
        GenieDialog_reset(chat->m_dialog_handle);
        chat->is_first_prompt = true; // reset for next stateless round
    }
}

void ChatManager::user_query(const std::string& dialogue_id,
                             const std::string& user_prompt,
                             GenieResponseCallback callback)
{
    auto chat = get_dialogue(dialogue_id);

    if (!chat->is_stateful) {
        throw std::runtime_error("user_query() is only valid for stateful sessions.");
    }

    if (chat->is_first_prompt) {
        throw std::runtime_error("Must call query() with system prompt before user_query() in stateful mode.");
    }

    llm::prompt::PromptUtils prompt_utils(llm::prompt::ModelType::Llama3);
    std::string tagged_prompt =
        prompt_utils.get_subseq_prompt_with_tag(user_prompt);

    CallbackWrapper wrapper{callback};

    Genie_Status_t status = GenieDialog_query(
                                chat->m_dialog_handle,
                                tagged_prompt.c_str(),
                                GENIE_DIALOG_SENTENCE_COMPLETE,
                                trampoline,
                                &wrapper);

    if (status != GENIE_STATUS_SUCCESS) {
        throw std::runtime_error("Failed to get response from GenieDialog.");
    }
}

std::shared_ptr<GenieChat> ChatManager::get_dialogue(const std::string& dialogue_id) {
    auto it = m_sessions.find(dialogue_id);
    if (it == m_sessions.end()) {
        throw std::runtime_error("Dialogue ID not found: " + dialogue_id);
    }
    return it->second;
}
