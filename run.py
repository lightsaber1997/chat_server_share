import requests

URL = "http://localhost:8080/chat"

def generate(sys_prompt: str, user_prompt: str, callback):
    """
    Calls /chat endpoint.
    - Sends JSON request body
    - Expects plain text response
    - Passes the response text to the callback
    """
    try:
        response = requests.post(
            URL,
            json={
                "sys_prompt": sys_prompt,
                "user_prompt": user_prompt
            },
            timeout=120  # allow long responses
        )

        if response.status_code == 200:
            callback(response.text)   # plain text response
        else:
            callback(f"[Error {response.status_code}] {response.text}")

    except Exception as e:
        callback(f"[Request failed] {e}")


# --------------------------
# Example interactive usage
# --------------------------
if __name__ == "__main__":
    def print_output(text):
        print("AI:", text)

    print("Chat client started. Type 'exit' anytime to quit.\n")

    while True:
        sys_prompt = input("System prompt: ")
        if sys_prompt.strip().lower() in {"exit", "quit"}:
            break

        user_prompt = input("User prompt: ")
        if user_prompt.strip().lower() in {"exit", "quit"}:
            break

        generate(sys_prompt, user_prompt, print_output)
