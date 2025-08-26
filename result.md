Note: The KPIS stdout below is deprecated and will be removed in a future release.
[KPIS]:
Init Time: 0 us
Prompt Processing Time: 142531 us, Prompt Processing Rate : 315.756226 toks/sec
Token Generation Time: 1671492 us, Token Generation Rate: 14.358533 toks/sec
[DEBUG] manager.query (streaming) finished
[DEBUG] POST /chat_stream called
[DEBUG] Raw body: {"sys_prompt": "You are an assistant in a children's story-builder app.\nAnswer ONLY with {\"is_story\":\"true\"} if the user's message is a STORY SENTENCE,\nor {\"is_story\":\"false\"} if it is a QUESTION/CHAT. No extra words.", "user_prompt": "The capital of USA is New York.", "max_new_tokens": 8, "temperature": 0.7, "top_p": 1.0, "stream": true}
[DEBUG] Parsed JSON
[DEBUG] sys_prompt: You are an assistant in a children's story-builder app.
Answer ONLY with {"is_story":"true"} if the user's message is a STORY SENTENCE,
or {"is_story":"false"} if it is a QUESTION/CHAT. No extra words.
[DEBUG] user_prompt: The capital of USA is New York.
[DEBUG] manager.query (streaming) starting
[DEBUG] Stream chunk: "{"" (len=2, code=1)
[DEBUG] Stream chunk: "is" (len=2, code=2)
[DEBUG] Stream chunk: "_story" (len=6, code=2)
[DEBUG] Stream chunk: "":" (len=2, code=2)
[DEBUG] Stream chunk: " "" (len=2, code=2)
[DEBUG] Stream chunk: "false" (len=5, code=2)
[DEBUG] Stream chunk: ""}" (len=2, code=2)
[DEBUG] Stream chunk: "" (len=0, code=3)


Note: The KPIS stdout below is deprecated and will be removed in a future release.
[KPIS]:
Init Time: 0 us
Prompt Processing Time: 159854 us, Prompt Processing Rate : 456.829590 toks/sec
Token Generation Time: 781860 us, Token Generation Rate: 10.232063 toks/sec
[DEBUG] manager.query (streaming) finished
[DEBUG] POST /chat_stream called
[DEBUG] Raw body: {"sys_prompt": "You are a helpful assistant for casual chat.\nRespond with EXACTLY ONE JSON object:\n{\"answer\":\"...\"}", "user_prompt": "The capital of USA is New York.", "max_new_tokens": 120, "temperature": 0.7, "top_p": 1.0, "stream": true}
[DEBUG] Parsed JSON
[DEBUG] sys_prompt: You are a helpful assistant for casual chat.
Respond with EXACTLY ONE JSON object:
{"answer":"..."}
[DEBUG] user_prompt: The capital of USA is New York.
[DEBUG] manager.query (streaming) starting
[DEBUG] Stream chunk: "{"" (len=2, code=1)
[DEBUG] Stream chunk: "answer" (len=6, code=2)
[DEBUG] Stream chunk: "":" (len=2, code=2)
[DEBUG] Stream chunk: " "" (len=2, code=2)
[DEBUG] Stream chunk: "That" (len=4, code=2)
[DEBUG] Stream chunk: "'s" (len=2, code=2)
[DEBUG] Stream chunk: " incorrect" (len=10, code=2)
[DEBUG] Stream chunk: "," (len=1, code=2)
[DEBUG] Stream chunk: " the" (len=4, code=2)
[DEBUG] Stream chunk: " capital" (len=8, code=2)
[DEBUG] Stream chunk: " of" (len=3, code=2)
[DEBUG] Stream chunk: " the" (len=4, code=2)
[DEBUG] Stream chunk: " United" (len=7, code=2)
[DEBUG] Stream chunk: " States" (len=7, code=2)
[DEBUG] Stream chunk: " of" (len=3, code=2)
[DEBUG] Stream chunk: " America" (len=8, code=2)
[DEBUG] Stream chunk: " is" (len=3, code=2)
[DEBUG] Stream chunk: " Washington" (len=11, code=2)
[DEBUG] Stream chunk: "," (len=1, code=2)
[DEBUG] Stream chunk: " D" (len=2, code=2)
[DEBUG] Stream chunk: ".C" (len=2, code=2)
[DEBUG] Stream chunk: "."" (len=2, code=2)
[DEBUG] Stream chunk: "}" (len=1, code=2)
[DEBUG] Stream chunk: "" (len=0, code=3)


Note: The KPIS stdout below is deprecated and will be removed in a future release.
[KPIS]:
Init Time: 0 us
Prompt Processing Time: 147560 us, Prompt Processing Rate : 304.971039 toks/sec
Token Generation Time: 2651618 us, Token Generation Rate: 9.051085 toks/sec
[DEBUG] manager.query (streaming) finished