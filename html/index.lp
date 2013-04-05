<?
  -- Lua server pages have full control over the output, including HTTP
  -- headers they send to the client. Send HTTP headers:
  print('HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n')

print_float = function(f)
	print(string.format("%.1f",f))
end

function add_gauge(name, temp)
	print(string.format("<div id=\"gauge_%s\" class=\"mcp_gauge\"></div>\n", name));
end

?>
<html>
  <head>
    <script src="jquery.js"></script>
    <script src="raphael.js"></script>
    <script src="justgage.js"></script>
    <script src="jquery.bpopup.js"></script>
    <script src="mcpgauge.js"></script>
    <script type='text/javascript'>

    var gauges = {};

    $(document).ready(function() {
    	loadState();
    });

    function updateState() {
    	$.ajax({
    		url: "state.lp",
    		type: "GET",
    		dataType: "json",
    		success: function(data) {
    			$.each(data, function(key, sensor) {
            gauges[sensor.name].update(sensor);
    			});
    			setTimeout(function() { updateState(); }, 1000);
    		},

    		error: function( xhr, status ) {
    		},
    	});
    }

    function loadState() {
    	$.ajax({
    		url: "state.lp",
    		type: "GET",
    		dataType: "json",
    		success: function(data) {
    			$.each(data, function(key, sensor) {
            gauges[sensor.name] = new MCPGauge(sensor);
    			});
    			setTimeout(function() { updateState(); }, 1000);
    		},

    		error: function( xhr, status ) {
    		},
    	});
    }
    function drawGauge(sensor) {
    	var g = new JustGage({
    		id: "gauge_" + sensor.name,
    		title: sensor.name,
    		value: sensor.temp,
    		decimals: 1,
    		min: 0,
    		max: 100,
    		startAnimationTime: 1,
    		refreshAnimationTime: 1,
    		customSectors: [{
    			color : "#ff0000",
    			lo : 0,
    			hi : 61
    		},{
    			color : "#ffff00",
    			lo : 61,
    			hi : 64.5
    		}, {
    			color : "#00ff00",
    			lo : 64.5,
    			hi : 65.5
     		},{
    			color : "#ffff00",
    			lo : 65.5,
    			hi : 69.0
    		}, {
    			color : "#ff0000",
    			lo : 69.0,
    			hi : 100
    		}]
    	});
    	gauges[sensor.name] = g;
      }
    </script>
    <style>
    .mcp_gauge {
      float: left;
      width: 250px;
      height: 200px;

    }
    .gauges {
      clear:both;
    }
    .pid {
      background: #ffffff;
    }
    </style>
  </head>
  <body>
    <div id="gauges" class="gauges">
      <?
      for i, sensor in ipairs(sensors) do
        add_gauge(sensor["name"], sensor["temp"]);
      end ?>
    </div>
    <div>
      <form method="POST" action="/pid_update">
      <pre><? print(err_string) ?></pre>
      <table border="1">
	<tr><th>flow:</th><td>
	    <input type="radio" name="flow" value="recirc" <?
if (not flow) then
   print("checked") end
 ?>>Recirculate</input><br />
	    <input type="radio" name="flow" value="cross" <?
if (flow) then
   print("checked") end
?>>Cross</input>
      </table>
      <input type="submit" value="Update"/>
    </form>
  </div>
	<div id="debug"></div>
  </body>
</html>
