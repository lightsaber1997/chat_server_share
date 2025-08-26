// chat_server.cpp
#include "httplib.h"
#include "ChatManager.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <json.hpp>      // if this fails on your setup, use: #include <nlohmann/json.hpp>
#include <mutex>
#include <cstring>       // strlen

using json = nlohmann::json;

namespace {
constexpr const std::string_view c_option_genie_config = "--genie-config";
constexpr const std::string_view c_option_base_dir    = "--base-dir";
constexpr const std::string_view c_option_help        = "--help";
constexpr const std::string_view c_option_help_short  = "-h";

void PrintHelp(const char* exe) {
    std::cout << "\nUsage:\n"
              << exe << " --genie-config <config.json> --base-dir <dir>\n\n"
              << c_option_genie_config << " <Local file path>: [Required] Path to Genie config JSON\n"
              << c_option_base_dir    << " <Local directory path>: [Required] Working directory\n";
}
} // namespace

int main(int argc, char* argv[]) {
    std::string genie_config_path;
    std::string base_dir;

    // Parse CLI args
    for (int i = 1; i < argc; ++i) {
        if (c_option_genie_config == argv[i] && i + 1 < argc) {
            genie_config_path = argv[++i];
        } else if (c_option_base_dir == argv[i] && i + 1 < argc) {
            base_dir = argv[++i];
        } else if (c_option_help == argv[i] || c_option_help_short == argv[i]) {
            PrintHelp(argv[0]);
            return 0;
        }
    }

    if (genie_config_path.empty() || base_dir.empty()) {
        PrintHelp(argv[0]);
        return 1;
    }

    // Validate paths
    if (!std::filesystem::exists(genie_config_path)) {
        std::cerr << "Config file not found: " << genie_config_path << "\n";
        return 1;
    }
    if (!std::filesystem::exists(base_dir)) {
        std::cerr << "Base dir not found: " << base_dir << "\n";
        return 1;
    }

    // Load config file into string
    std::ifstream config_file(genie_config_path);
    if (!config_file) {
        std::cerr << "Failed to open Genie config file: " << genie_config_path << "\n";
        return 1;
    }
    std::string config((std::istreambuf_iterator<char>(config_file)),
                       std::istreambuf_iterator<char>());

    // Set working dir
    std::filesystem::current_path(base_dir);

    // Init ChatManager (non–thread-safe)
    ChatManager manager(config);

    // Reuse a single dialogue id if that's what you want
    std::string dlg = manager.create_new_dialogue(false);

    // Serialize ALL access to ChatManager
    static std::mutex g_mgr_mu;

    httplib::Server svr;

    // Avoid huge POST bodies nuking memory
    svr.set_payload_max_length(1ULL << 20); // 1 MiB

    // Better top-level error visibility
    svr.set_exception_handler([](const httplib::Request&, httplib::Response& res, std::exception_ptr ep) {
        try { if (ep) std::rethrow_exception(ep); }
        catch (const std::bad_alloc&) {
            res.status = 500;
            res.set_content("Error: out of memory (bad_alloc)", "text/plain");
        }
        catch (const std::exception& e) {
            res.status = 500;
            res.set_content(std::string("Unhandled exception: ") + e.what(), "text/plain");
        }
    });

    // Health check
    svr.Get("/hi", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello from Chat server!", "text/plain");
    });

    // Blocking endpoint: receive JSON, send text
    svr.Post("/chat", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            json body;
            try {
                body = json::parse(req.body);
            } catch (const json::parse_error& e) {
                res.status = 400;
                res.set_content(std::string("JSON parse error: ") + e.what(), "text/plain");
                return;
            }

            std::string sys_prompt = body.value("sys_prompt", "");
            std::string user_prompt = body.value("user_prompt", "");
            std::cerr << "[DEBUG] sys_prompt: " << sys_prompt << "\n";
            std::cerr << "[DEBUG] user_prompt: " << user_prompt << "\n";


            if (sys_prompt.empty() || user_prompt.empty()) {
                res.status = 400;
                res.set_content("Error: sys_prompt and user_prompt required", "text/plain");
                return;
            }

            std::string output;
            try {
                std::lock_guard<std::mutex> lk(g_mgr_mu);  // serialize ChatManager access
                
                std::cerr << "[DEBUG] manager.query starting\n";
                manager.query(dlg, sys_prompt, user_prompt,
                              [&](const char* text, GenieDialog_SentenceCode_t) {
                                  output += text;
                              });
            } catch (const std::bad_alloc&) {
                res.status = 500;
                res.set_content("Error: out of memory (bad_alloc) in manager.query", "text/plain");
                return;
            } catch (const std::exception& e) {
                res.status = 500;
                res.set_content(std::string("Error in manager.query: ") + e.what(), "text/plain");
                return;
            }
            std::cerr << "[DEBUG] output " << output << "\n";


            res.set_content(output, "text/plain");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(std::string("Error: ") + e.what(), "text/plain");
        }
    });

    // Streaming endpoint: receive JSON, stream plain text
    svr.Post("/chat_stream", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            std::cerr << "[DEBUG] POST /chat_stream called\n";
            std::cerr << "[DEBUG] Raw body: " << req.body << "\n";
            json body;
            try {
                body = json::parse(req.body);
                std::cerr << "[DEBUG] Parsed JSON\n";
            } catch (const json::parse_error& e) {
                std::cerr << "[DEBUG] JSON parse error: " << e.what() << "\n";
                res.status = 400;
                res.set_content(std::string("JSON parse error: ") + e.what(), "text/plain");
                return;
            }

            std::string sys_prompt = body.value("sys_prompt", "");
            std::string user_prompt = body.value("user_prompt", "");
            std::cerr << "[DEBUG] sys_prompt: " << sys_prompt << "\n";
            std::cerr << "[DEBUG] user_prompt: " << user_prompt << "\n";
            
            
            if (sys_prompt.empty() || user_prompt.empty()) {
                res.status = 400;
                res.set_content("Error: sys_prompt and user_prompt required", "text/plain");
                return;
            }

            // Capture prompts by value so nothing dangles
            res.set_chunked_content_provider(
                "text/plain",
                [&, sys_prompt = std::move(sys_prompt), user_prompt = std::move(user_prompt)]
                (size_t /*offset*/, httplib::DataSink& sink) {
                    try {
                        std::lock_guard<std::mutex> lk(g_mgr_mu); // serialize ChatManager access
                        std::cerr << "[DEBUG] manager.query (streaming) starting\n";
                        manager.query(
                            dlg, sys_prompt, user_prompt,
                            [&](const char* text, GenieDialog_SentenceCode_t code) {
                                const size_t n = std::strlen(text); // ChatManager must return NUL-terminated chunks
                                if (n) sink.write(text, n);
                                std::cerr << "[DEBUG] Stream chunk: \"" << text << "\" (len=" << n << ", code=" << code << ")\n";
                                
                                if (code == GENIE_DIALOG_SENTENCE_END) {
                                    sink.write("\n", 1); // separate responses
                                }
                            });
                        
                        std::cerr << "[DEBUG] manager.query (streaming) finished\n";
                    } catch (const std::bad_alloc&) {
                        const char* err = "Error: out of memory (bad_alloc) in manager.query\n";
                        sink.write(err, std::strlen(err));
                    } catch (const std::exception& e) {
                        std::string err = std::string("Error in manager.query: ") + e.what() + "\n";
                        sink.write(err.c_str(), err.size());
                    }
                    sink.done(); // close exactly once, after query completes
                    return true;
                });
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content(std::string("Error: ") + e.what(), "text/plain");
        }
    });

    std::cout << "Server running at http://0.0.0.0:8080\n";
    std::cout << " - POST /chat        (receives JSON, returns plain text)\n";
    std::cout << " - POST /chat_stream (receives JSON, streams plain text)\n";

    svr.listen("0.0.0.0", 8080);
    return 0;
}
