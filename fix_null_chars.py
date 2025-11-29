#!/usr/bin/env python3
with open('app/ui/tabs/tab_crawler.c', 'rb') as f:
    content = f.read()

# Replace the null character bytes
content = content.replace(b"text[0] == '\x00'", b"text[0] == '\\0'")
content = content.replace(b"*endptr != '\x00'", b"*endptr != '\\0'")

with open('app/ui/tabs/tab_crawler.c', 'wb') as f:
    f.write(content)

print('Fixed null character literals')