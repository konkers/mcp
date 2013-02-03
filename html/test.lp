<?
  -- Lua server pages have full control over the output, including HTTP
  -- headers they send to the client. Send HTTP headers:
  print('HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n')
?>
<html>
  <body>
    <form method="POST" action="/pid_update">
      <table border="1">
	<tr><th>Curent Temp:</th><td><? print(cur_temp) ?></td></tr>
	<tr><th>Set Point:</th><td><? print(set_point) ?></td></tr>
	<tr><th>P term:</th><td>
	    <input type="text" name="p_term" value="<? print(pid_p) ?>"/>
	</td></tr>
	<tr><th>I term:</th><td>
	    <input type="text" name="i_term" value="<? print(pid_i) ?>"/>
	</td></tr>
	<tr><th>D term:</th><td>
	    <input type="text" name="d_term" value="<? print(pid_d) ?>"/>
	</td></tr>
	<tr><th>pkt_1:</th><td><? print(pid_pkt_1) ?></td></tr>
	<tr><th>ekt_1:</th><td><? print(pid_ekt_1) ?></td></tr>
      </table>
      <input type="submit" value="Update"/>
    </form>
  </body>
</html>
