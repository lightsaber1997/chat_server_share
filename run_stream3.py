import requests

def decode_escape(seq: str) -> str:
    r"""Decode JSON escape sequences, including \uXXXX."""
    mapping = {
        '"': '"', '\\': '\\', '/': '/',
        'b': '\b', 'f': '\f', 'n': '\n',
        'r': '\r', 't': '\t'
    }
    if not seq:
        return ""
    if seq.startswith("u") and len(seq) == 5:  # "u" + 4 hex digits
        try:
            return chr(int(seq[1:], 16))
        except ValueError:
            return seq
    return mapping.get(seq, seq)


def stream_keys_progressive_post(url, key, payload):

    buffer1 = ""
    buffer2 = ""

    found_key = False
    found_start_value = False
    int count = 0
    int count_limit = 5
    with requests.post(url, json=payload, stream=True) as r:
        for chunk in r.iter_content(chunk_size=1, decode_unicode=True):
            if not chunk:
                
                continue
            
            
            
            
            buffer1 += chunk
            count += 1

            if (count == count_limit):
                buffer2 += buffer1

                if not found_key:
                    search_str = f'"{key}":'
                    pos = buffer2.find(search_str)
                    if pos != -1:
                        pass
                    else:
                        buffer2 = buffer2[pos+len(search_str):]
                        found_key = True
                else:
                    if not found_start_value:
                        search_str = '"'
                        pos = buffer2.find(search_str)
                        buffer2 = buffer2[pos+len(search_str):]
                        found_start_value = True
                    else:
                        search_str = '"}'
                        pos = buffer2.find(search_str)
                        value = buffer2[pos+len(search_str)]
                        found_start_value = True



                buffer1 = ""
                count = 0


            
                

def stream_keys_progressive_post(url, keys, payload):
    """
    POST request to stream JSON keys progressively.
    Yields (key, chunk, end_of_value).
    """
    targets = {key: f'"{key}":' for key in keys}
    search_buf = {key: "" for key in keys}
    inside_value = {key: False for key in keys}
    escape = {key: False for key in keys}
    escape_buf = {key: "" for key in keys}

    with requests.post(url, json=payload, stream=True) as r:
        yield ("__status__", f"Status: {r.status_code}, Content-Type: {r.headers.get('Content-Type')}", False)

        for chunk in r.iter_content(chunk_size=1, decode_unicode=True):
            if not chunk:
                continue
            print(f"chunk={chunk}")
            breakpoint()
            for key in keys:
                if not inside_value[key]:
                    # accumulate for key detection
                    search_buf[key] += chunk
                    if len(search_buf[key]) > len(targets[key]):
                        search_buf[key] = search_buf[key][-len(targets[key]):]

                    if search_buf[key] == targets[key]:
                        inside_value[key] = True
                        search_buf[key] = ""
                        # wait for the next '"' before value
                        continue

                else:
                    # inside a string value
                    c = chunk
                    if escape[key]:
                        escape_buf[key] += c
                        if escape_buf[key] in ('"', '\\', '/', 'b', 'f', 'n', 'r', 't'):
                            yield (key, decode_escape(escape_buf[key]), False)
                            escape[key] = False
                            escape_buf[key] = ""
                        elif escape_buf[key].startswith("u") and len(escape_buf[key]) == 5:
                            yield (key, decode_escape(escape_buf[key]), False)
                            escape[key] = False
                            escape_buf[key] = ""
                        # else still waiting for full \uXXXX
                    elif c == "\\":
                        escape[key] = True
                        escape_buf[key] = ""
                    elif c == '"':
                        inside_value[key] = False
                        yield (key, "", True)  # signal end of value
                    else:
                        yield (key, c, False)


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
                # optional: show status for debugging
                print(f"\n[{chunk}]\n")
            elif not end:
                print(chunk, end="", flush=True)
            else:
                print("\n--- end of response ---")
