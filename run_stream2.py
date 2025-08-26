import requests

def decode_escape(seq: str) -> str:
    """Decode simple JSON escape sequences (\" \\ \n \t \r)."""
    if seq == '"':  return '"'
    if seq == '\\': return '\\'
    if seq == 'n':  return '\n'
    if seq == 't':  return '\t'
    if seq == 'r':  return '\r'
    return seq  # fallback


def stream_keys_progressive_post(url, keys, payload):
    """
    POST request to stream JSON keys progressively.
    Yields (key, chunk, end_of_value).
    """
    targets = {key: f'"{key}":"' for key in keys}
    inside_value = {key: False for key in keys}
    escape = {key: False for key in keys}
    key_buffer = {key: "" for key in keys}

    buf = ""

    with requests.post(url, json=payload, stream=True) as r:
        yield ("__status__", f"Status: {r.status_code}, Content-Type: {r.headers.get('Content-Type')}", False)

        for chunk in r.iter_content(chunk_size=None, decode_unicode=True):
            if not chunk:
                continue
            buf += chunk
            # print(chunk)

            # Try to detect each key prefix progressively
            for key in keys:
                if not inside_value[key]:
                    needed = targets[key]
                    for c in buf:
                        key_buffer[key] += c
                        if key_buffer[key].endswith(needed):
                            inside_value[key] = True
                            # Cut off consumed part
                            buf = buf[buf.index(needed) + len(needed):]
                            key_buffer[key] = ""
                            break

                if inside_value[key]:
                    new_buf = ""
                    i = 0
                    while i < len(buf):
                        c = buf[i]
                        if escape[key]:
                            yield (key, decode_escape(c), False)
                            escape[key] = False
                        elif c == "\\":
                            escape[key] = True
                        elif c == '"':
                            inside_value[key] = False
                            yield (key, "", True)
                            new_buf = buf[i+1:]
                            break
                        else:
                            yield (key, c, False)
                        i += 1
                    else:
                        new_buf = ""
                    buf = new_buf


# -------------------------
# Example usage
# -------------------------
if __name__ == "__main__":
    url = "http://localhost:8080/chat_stream"

    print("Streaming Chat Client (type 'exit' to quit)\n")

    while True:
        sys_prompt = input("System prompt: ")
        if sys_prompt.strip().lower() in {"exit", "quit"}:
            break

        user_prompt = input("User prompt: ")
        if user_prompt.strip().lower() in {"exit", "quit"}:
            break

        print("AI: ", end="", flush=True)

        for key, chunk, end in stream_keys_progressive_post(
            url,
            ["output"],
            {"sys_prompt": sys_prompt, "user_prompt": user_prompt}
        ):
            if key == "__status__":
                print(f"\n[{chunk}]\n")
            elif not end:
                print(chunk, end="", flush=True)
            else:
                print("\n--- end of response ---")
