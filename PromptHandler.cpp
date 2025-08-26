#include "PromptHandler.hpp"


namespace llm::prompt
{

/**
 * @brief Prompt template structure holding delimiters for different roles.
 *
 * Different LLM families (Llama2, Llama3, Llama3-TAIDE, etc.) use different
 * prompt formatting rules. This struct groups together all delimiters
 * required to build a valid prompt sequence.
 */
struct PromptTemplates {
    std::string begin_prompt;      ///< Conversation start marker (e.g., <|begin_of_text|>)
    std::string begin_system;      ///< Marks the beginning of system instructions
    std::string begin_user;        ///< Marks the beginning of a user message
    std::string end_user;          ///< Marks the end of a user message
    std::string begin_assistant;   ///< Marks the beginning of assistant response
    std::string end_assistant;     ///< Marks the end of assistant response
};

/**
 * @brief Retrieve model-specific prompt templates.
 *
 * Provides the correct delimiters depending on the selected model.
 *
 * @param model The target LLM model type.
 * @return PromptTemplates with role-specific delimiters.
 */
static PromptTemplates getTemplates(ModelType model) {
    switch (model) {
        case ModelType::Llama3:
            return {
                "<|begin_of_text|>",
                "<|start_header_id|>system<|end_header_id|>\n\n",
                "<|start_header_id|>user<|end_header_id|>\n\n",
                "<|eot_id|>",
                "<|start_header_id|>assistant<|end_header_id|>\n\n",
                "<|eot_id|>"
            };
        case ModelType::Llama3_Taide:
            return {
                "<|begin_of_text|>",
                "<|start_header_id|>system<|end_header_id|>\n\n",
                "<|start_header_id|>user<|end_header_id|>\n\n",
                "<|eot_id|>",
                "<|start_header_id|>assistant<|end_header_id|>\n\n",
                "<|eot_id|>"
            };
        case ModelType::Llama2:
            return {
                "",
                "<<SYS>>\n",
                "<s>[INST] ",
                " [/INST] ",
                "",
                "\n</s>\n"
            };
    }
    return {};
}

/**
 * @brief Construct a PromptHandler for a specific model.
 *
 * @param model The LLM model type that determines formatting rules.
 */
PromptHandler::PromptHandler(ModelType model)
    : m_is_first_prompt(true), m_model(model)
{
}

/**
 * @brief Build a prompt string for the given user input.
 *
 * If this is the first call, the system prompt and conversation start token
 * are included. For later calls, only user and assistant tags are added.
 *
 * @param user_prompt User-provided text input.
 * @return Formatted prompt string ready for the LLM.
 */
std::string PromptHandler::GetPromptWithTag(const std::string& user_prompt)
{
    auto t = getTemplates(m_model);

    if (m_is_first_prompt) {
        m_is_first_prompt = false;
        return t.begin_prompt + t.begin_system +
               t.begin_user + user_prompt + t.end_user +
               t.begin_assistant;
    }

    return t.end_assistant + t.begin_user + user_prompt + t.end_user + t.begin_assistant;
}

/**
 * @brief Construct a PromptUtils instance for a given model.
 *
 * @param model The LLM model type.
 */
PromptUtils::PromptUtils(ModelType model)
    : m_model(model)
{
}

/**
 * @brief Generate the initial prompt containing system + user messages.
 *
 * This method is used for the conversation's first turn. It includes
 * the system instructions and the first user input, followed by the
 * assistant tag.
 *
 * @param system_prompt Instructions for the system role.
 * @param user_prompt The first user message.
 * @return A formatted string containing system + user + assistant sections.
 */
std::string PromptUtils::get_prompt_with_tag(
    const std::string& system_prompt,
    const std::string& user_prompt
) {
    auto t = getTemplates(m_model);

    return t.begin_prompt + t.begin_system + system_prompt +
           t.begin_user + user_prompt + t.end_user +
           t.begin_assistant;
}

/**
 * @brief Generate a prompt for subsequent user turns in the conversation.
 *
 * Unlike get_prompt_with_tag(), this function omits the system prompt and
 * conversation start token, appending only user and assistant delimiters.
 *
 * @param user_prompt The next user message.
 * @return A formatted string containing user + assistant sections.
 */
std::string PromptUtils::get_subseq_prompt_with_tag(
    const std::string& user_prompt
) {
    auto t = getTemplates(m_model);

    return t.end_assistant + t.begin_user + user_prompt + t.end_user + t.begin_assistant;
}

} // namespace llm::prompt
