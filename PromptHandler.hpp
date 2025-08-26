#pragma once

#include <string>

namespace llm::prompt
{

/// Supported LLM model types.
enum class ModelType {
    Llama3,       ///< Meta Llama 3
    Llama3_Taide, ///< TAIDE variant of Llama 3
    Llama2        ///< Meta Llama 2
};

/// Handles prompt formatting across conversation turns.
class PromptHandler
{
  private:
    bool m_is_first_prompt;  ///< Tracks if this is the first prompt
    ModelType m_model;       ///< Selected model type

  public:
    /// Construct a handler for a given model.
    explicit PromptHandler(ModelType model);

    /// Generate a prompt for the user input (includes system prompt if first).
    std::string GetPromptWithTag(const std::string& user_prompt);
};

/// Stateless utilities for prompt construction.
class PromptUtils
{
  private:
    ModelType m_model; ///< Selected model type

  public:
    /// Construct utilities for a given model.
    explicit PromptUtils(ModelType model);

    /// Generate the initial system + user prompt.
    std::string get_prompt_with_tag(
        const std::string& system_prompt,
        const std::string& user_prompt
    );

    /// Generate a prompt for subsequent user messages.
    std::string get_subseq_prompt_with_tag(
        const std::string& user_prompt
    );
};

} // namespace llm::prompt
