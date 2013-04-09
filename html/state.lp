<?
  -- Lua server pages have full control over the output, including HTTP
  -- headers they send to the client. Send HTTP headers:
  print('HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n')

json = require("json")

print(json.encode({sensors=sensors, outputs=outputs}))
?>